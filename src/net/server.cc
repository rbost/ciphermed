#include <iostream>
#include <string>
#include <vector>
#include <boost/asio.hpp>
#include <gmpxx.h>
#include <thread>

#include <net/defs.hh>

#include <crypto/paillier.hh>
#include <mpc/lsic.hh>
#include <mpc/rev_enc_comparison.hh>

#include <net/server.hh>
#include <net/net_utils.hh>

using boost::asio::ip::tcp;

using namespace std;

Server::Server(gmp_randstate_t state, unsigned int nbits_p, unsigned int abits_p, unsigned int nbits_gm, unsigned int lambda)
: paillier_(Paillier_priv::keygen(state,nbits_p,abits_p),state), gm_(GM_priv::keygen(state,nbits_gm),state), n_clients_(0), lambda_(lambda)
{
    gmp_randinit_set(rand_state_, state);
//    cout << "SK GM\np = " << gm_.privkey()[0] << "\nq = " << gm_.privkey()[1] << endl;
//    cout << "PK Paillier\nn" << paillier_.pubkey()[0] << "\ng" << paillier_.pubkey()[1] << endl;
}

void Server::run()
{
    try
    {
        boost::asio::io_service io_service;
        
        tcp::endpoint endpoint(tcp::v4(), PORT);
        tcp::acceptor acceptor(io_service, endpoint);
        
        for (;;)
        {
            tcp::socket *socket = new tcp::socket(io_service);
            acceptor.accept(*socket);
            
            Server_session *c = new Server_session(this, rand_state_, n_clients_++, socket);
            
            cout << "Start new connexion: " << c->id() << endl;
            thread t (&Server_session::run_session,c);
            t.detach();
        }
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
}



Server_session::Server_session(Server *server, gmp_randstate_t state, unsigned int id, tcp::socket *socket)
: server_(server), socket_(socket), id_(id)
{
    gmp_randinit_set(rand_state_, state);
}

void Server_session::run_session()
{
    cout << id_ << ": Start session" << endl;
    
    // main loop to catch requests
    bool should_exit = false;
    try {
        for (;!should_exit; ) {
        
        // wait for a complete request
        boost::asio::read_until(*socket_, input_buf_, "\r\n");
        
        std::istream input_stream(&input_buf_);
        std::string line;
        
    //    std::string s( (std::istreambuf_iterator<char>( input_stream )),
    //                  (std::istreambuf_iterator<char>()) );
    //    cout << s << endl;

        // parse the input
        do {
            getline(input_stream,line);
//            cout << line;
            if (line == "") {
                continue;
            }

            if (line == GET_PAILLIER_PK) {
                send_paillier_pk();
            }else if(line == GET_GM_PK) {
                send_gm_pk();
            }else if(line == START_LSIC) {
                mpz_class b(20);
                run_lsic(b,10);
            }else if(line == DECRYPT_GM) {
                mpz_class c(5);
                getline(input_stream,line);
                c.set_str(line,10);
                decrypt_gm(c);
            }else if(line == START_REV_ENC_COMPARE){
                // get the bit length and launch the helper
                getline(input_stream,line);
                size_t l = strtoul (line.c_str(), NULL, 0);
                bool b = run_rev_enc_comparison(l,server_->paillier_sk(),server_->gm_sk());
                
                cout << id_ << ": Rev Enc Compare result: " << b << endl;
            }else if(line == DISCONNECT){
                should_exit = true;
                break;
            }
        } while (!input_stream.eof());
    }
    cout << id_ << ": Disconnected" << endl;

        
    } catch (std::exception& e) {
        std::cout << "Exception: " << e.what() << std::endl;
    }

    // we are done, delete ourself
    delete this;
}

void Server_session::send_paillier_pk()
{
    auto pk = server_->paillier_pk();
    boost::asio::streambuf buff;
    std::ostream buff_stream(&buff);
    
    cout << id_ << ": Send Paillier PK" << endl;
    buff_stream << PAILLIER_PK << "\n";
    buff_stream << pk[0] << "\n" << pk[1] << "\n";
    
    buff_stream << END_PAILLIER_PK << "\n";
    boost::asio::write(*socket_, buff);
}

void Server_session::send_gm_pk()
{
    auto pk = server_->gm_pk();
    boost::asio::streambuf buff;
    std::ostream buff_stream(&buff);
    
    cout << id_ << ": Send GM PK" << endl;
    buff_stream << GM_PK << "\n";
    buff_stream << pk[0] << "\n" << pk[1] << "\n";
    
    buff_stream << END_GM_PK << "\n";
    boost::asio::write(*socket_, buff);
}

void Server_session::run_lsic(const mpz_class &b,size_t l)
{
    cout << id_ << ": Start LSIC" << endl;
    LSIC_B lsic(b,l, server_->gm_sk(),rand_state_);
    run_lsic_B(lsic);
}

void Server_session::run_lsic_B(LSIC_B &lsic)
{
    cout << id_ << ": Start LSIC B" << endl;
    boost::asio::streambuf output_buf;
    std::ostream output_stream(&output_buf);
    std::string line;
        
    LSIC_Packet_A a_packet;
    LSIC_Packet_B b_packet = lsic.setupRound();
    
    output_stream << LSIC_SETUP << "\n";
    output_stream << b_packet;
    output_stream << "\r\n";
    
    boost::asio::write(*socket_, output_buf);
    
    //    cout << "LSIC setup sent" << endl;
    
    // wait for packets
    
    for (; ; ) {
        boost::asio::read_until(*socket_, input_buf_, "\r\n");
        std::istream input_stream(&input_buf_);
        
        //        cout << "Received something" << endl;
        // parse the input
        do {
            getline(input_stream,line);
            //            cout << line;
            if (line == "") {
                continue;
            }
            
            if (line == LSIC_END) {
                cout << id_ << ": LSIC finished" << endl;
                return;
            }else if(line == LSIC_PACKET) {
                //                cout << "New packet" << endl;
                input_stream >> a_packet;
                
                b_packet = lsic.answerRound(a_packet);
                
                boost::asio::streambuf output_buf;
                std::ostream output_stream(&output_buf);
                
                output_stream << LSIC_PACKET << "\n";
                output_stream << b_packet;
                output_stream << "\r\n";
                
                boost::asio::write(*socket_, output_buf);
                
                //                cout << "Sent packet " << b_packet.index << endl;
            }
        } while (!input_stream.eof());
    }
    
}

bool Server_session::run_rev_enc_comparison(const size_t &l, const std::vector<mpz_class> sk_p, const std::vector<mpz_class> &sk_gm)
{
    Rev_EncCompare_Helper helper(l,server_->paillier_sk(),server_->gm_sk(),rand_state_);
    return run_rev_enc_comparison(helper);
}

bool Server_session::run_rev_enc_comparison(Rev_EncCompare_Helper &helper)
{
    boost::asio::streambuf output_buf;
    std::ostream output_stream(&output_buf);
    std::istream input_stream(&input_buf_);
    std::string line;

    
//    size_t l;
//
//    // get the bit length
//    getline(input_stream,line);
//    l = strtoul (line.c_str(), NULL, 0);


    // setup the helper if necessary
    if (!helper.is_set_up()) {
        mpz_class c_z;
        input_stream >> c_z;
        helper.setup(c_z);
    }

    // now, we need to run the LSIC protocol
    run_lsic_B(helper.lsic());
    
    
    mpz_class c_z_l(helper.get_c_z_l());
    
    output_stream << REV_ENC_COMPARE_CONCLUDE << "\n";
    output_stream << c_z_l;
    output_stream << "\r\n";
    
    boost::asio::write(*socket_, output_buf);

    // wait for the answer of the owner
    for (; ; ) {
        boost::asio::read_until(*socket_, input_buf_, "\r\n");
        std::istream input_stream(&input_buf_);
        // parse the input
        do {
            getline(input_stream,line);
            //            cout << line;
            if (line == "") {
                continue;
            }
            
            if (line == REV_ENC_COMPARE_RESULT) {
                cout << id_ << ": Rev encrypted comparison finished" << endl;
                mpz_class c_t;
                input_stream >> c_t;

                helper.decryptResult(c_t);
                
                return helper.output();
            }
        } while (!input_stream.eof());
    }    
}

void Server_session::decrypt_gm(const mpz_class &c)
{
    bool b = (server_->gm()).decrypt(c);
    cout << id_ << ": Decryption result = " << b << endl;
}

int main()
{
    gmp_randstate_t randstate;
    gmp_randinit_default(randstate);
    gmp_randseed_ui(randstate,time(NULL));
    

    cout << "Init server" << endl;
    Server server(randstate,1024,256,1024,100);
    
    cout << "Start server" << endl;
    server.run();
    
    return 0;
}