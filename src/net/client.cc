#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <gmpxx.h>

#include <net/defs.hh>

#include <crypto/gm.hh>
#include <mpc/lsic.hh>
#include <mpc/rev_enc_comparison.hh>

#include <math/util_gmp_rand.h>

#include <net/net_utils.hh>

#include <net/client.hh>

using boost::asio::ip::tcp;

using namespace std;

Client::Client(boost::asio::io_service& io_service, gmp_randstate_t state, unsigned int nbits_gm, unsigned int lambda)
: socket_(io_service), gm_(GM_priv::keygen(state,nbits_gm),state), server_paillier_(NULL), server_gm_(NULL), lambda_(lambda)
{
    gmp_randinit_set(rand_state_, state);
}


void Client::connect(boost::asio::io_service& io_service, const string& hostname)
{
    tcp::resolver resolver(io_service);
    tcp::resolver::query query(hostname, to_string( PORT ));
    tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
    boost::asio::connect(socket_, endpoint_iterator);
}


void Client::get_server_pk_gm()
{
    if (server_gm_) {
        return;
    }
    cout << "Request server's pubkey for GM" << endl;
    
    boost::asio::streambuf buff;
    std::ostream buff_stream(&buff);
    buff_stream << GET_GM_PK <<"\n\r\n";
    boost::asio::write(socket_, buff);
    
    
    boost::asio::read_until(socket_, input_buf_, END_GM_PK);
    std::istream input_stream(&input_buf_);
    string line;
    
    do {
        getline(input_stream,line);
    } while (line != GM_PK);
    // get the public key
    mpz_class N,y;
    getline(input_stream,line);
    N.set_str(line,10);
    getline(input_stream,line);
    y.set_str(line,10);
    
    server_gm_ = new GM({N,y},rand_state_);
}

void Client::get_server_pk_paillier()
{
    if (server_paillier_) {
        return;
    }
    cout << "Resquest server's pubkey for Paillier" << endl;

    boost::asio::streambuf buff;
    std::ostream buff_stream(&buff);
    buff_stream <<  GET_PAILLIER_PK <<"\n\r\n";
    boost::asio::write(socket_, buff);
    string line;
    
    
    boost::asio::read_until(socket_, input_buf_, END_PAILLIER_PK);
    std::istream input_stream(&input_buf_);
    
    do {
        getline(input_stream,line);
    } while (line != PAILLIER_PK);
    // get the public key
    mpz_class n,g;
    getline(input_stream,line);
    n.set_str(line,10);
    getline(input_stream,line);
    g.set_str(line,10);
    
    server_paillier_ = new Paillier({n,g},rand_state_);
}

mpz_class Client::run_lsic(const mpz_class &a, size_t l)
{
    if (!has_gm_pk()) {
        get_server_pk_gm();
    }
    // send the start message
    boost::asio::streambuf out_buff;
    std::ostream output_stream(&out_buff);
    output_stream << START_LSIC << "\n\r\n";
    boost::asio::write(socket_, out_buff);

    LSIC_A lsic(a,l,server_gm_->pubkey(),rand_state_);
    return run_lsic_A(lsic);
}

mpz_class Client::run_lsic_A(LSIC_A &lsic)
{
    
    LSIC_Packet_A a_packet;
    LSIC_Packet_B b_packet;
    bool state;
    
    
    boost::asio::streambuf out_buff;
    std::ostream output_stream(&out_buff);
    string line;
    
    
    // first get the setup round
    
    boost::asio::read_until(socket_, input_buf_, "\r\n");
    
    // parse the input - discard all the messages before the setup message
    bool received_setup = false;
    while (!received_setup) {
        std::istream input_stream(&input_buf_);

        while (!input_stream.eof()) {
            getline(input_stream,line);
            if (line == "") {
                continue;
            }
            
            if(line == LSIC_SETUP) {
                //            cout << "LSIC setup received" << endl;
                input_stream >> b_packet;
                
                state = lsic.answerRound(b_packet,&a_packet);
                
                if (state) {
                    return lsic.output();
                }
                
                output_stream << LSIC_PACKET << "\n";
                output_stream << a_packet;
                output_stream << "\r\n";
                boost::asio::write(socket_, out_buff);
                received_setup = true;
//                cout << "First packet sent to server" << endl;
                break;
            }
        } ;
        boost::asio::read_until(socket_, input_buf_, "\r\n");
    }
    
    // response-request
    for (; ; ) {
        boost::asio::read_until(socket_, input_buf_, "\r\n");
        std::istream input_stream(&input_buf_);
        
        // parse the input
        do {
            getline(input_stream,line);
            //            cout << line;
            if (line == "") {
                continue;
            }
            
            if(line == LSIC_PACKET) {
                input_stream >> b_packet;
                
                state = lsic.answerRound(b_packet,&a_packet);
                
                if (state) {
                    output_stream << LSIC_END << "\n";
                    output_stream << "\r\n";
                    boost::asio::write(socket_, out_buff);
                    
                    return lsic.output();
                }
                
                output_stream << LSIC_PACKET << "\n";
                output_stream << a_packet;
                output_stream << "\r\n";
                boost::asio::write(socket_, out_buff);
                
            }
        } while (!input_stream.eof());
    }
}

void Client::test_rev_enc_compare(size_t l)
{
    mpz_class a, b;
    mpz_urandom_len(a.get_mpz_t(), rand_state_, l);
    mpz_urandom_len(b.get_mpz_t(), rand_state_, l);
    
//    cout << "a = " << a << endl;
//    cout << "b = " << b << endl;

    get_server_pk_gm();
    get_server_pk_paillier();
    
    mpz_class c_a, c_b;

    run_rev_enc_compare(server_paillier_->encrypt(a),server_paillier_->encrypt(b),l);
    
    cout << "\nResult should be " << (a < b) << endl;
}

// we suppose that the client already has the server's public key for Paillier
void Client::run_rev_enc_compare(const mpz_class &a, const mpz_class &b, size_t l)
{
    assert(has_paillier_pk());
    assert(has_gm_pk());

    Rev_EncCompare_Owner owner(a,b,l,server_paillier_->pubkey(),server_gm_->pubkey(),rand_state_);
    
    mpz_class c_z(owner.setup(lambda_));

    
    boost::asio::streambuf out_buff;
    std::ostream output_stream(&out_buff);
    string line;
    
    // send the start message
    output_stream << START_REV_ENC_COMPARE << "\n";
    output_stream << l << "\n";
    output_stream << c_z << "\n\r\n";
    boost::asio::write(socket_, out_buff);
    
    // the server does some computation, we just have to run the lsic
    
    run_lsic_A(owner.lsic());
    
    // wait for the conlude message
    
    for (; ; ) {
        boost::asio::read_until(socket_, input_buf_, "\r\n");
        std::istream input_stream(&input_buf_);
        // parse the input
        do {
            getline(input_stream,line);
            //            cout << line;
            if (line == "") {
                continue;
            }
            
            if (line == REV_ENC_COMPARE_CONCLUDE) {
                mpz_class c_z_l;
                input_stream >> c_z_l;
                
                mpz_class c_t = owner.concludeProtocol(c_z_l);
                
                // send the last message to the server
                output_stream << REV_ENC_COMPARE_RESULT << "\n";
                output_stream << c_t << "\n\r\n";
                boost::asio::write(socket_, out_buff);

                return;
            }
        } while (!input_stream.eof());
    }    

    
}
void Client::disconnect()
{
    cout << "Disconnect" << endl;
    
    boost::asio::streambuf buff;
    std::ostream buff_stream(&buff);
    buff_stream << DISCONNECT << "\n\r\n";
    boost::asio::write(socket_, buff);
}



void decrypt_gm(tcp::socket &socket,const mpz_class &c)
{
    boost::asio::streambuf buff;
    std::ostream buff_stream(&buff);
    
    buff_stream << "DECRYPT GM\n"<< c << "\n\r\n";
    boost::asio::write(socket, buff);
    
}



int main(int argc, char* argv[])
{
    try
    {
        if (argc != 2)
        {
            std::cerr << "Usage: client <host>" << std::endl;
            return 1;
        }
        
        boost::asio::io_service io_service;

        gmp_randstate_t randstate;
        gmp_randinit_default(randstate);
        gmp_randseed_ui(randstate,time(NULL));
        

        Client client(io_service, randstate,1024,100);

        string hostname(argv[1]);
        client.connect(io_service, hostname);

        // server has b = 20
//        mpz_class res = client.run_lsic(40,10);
//        decrypt_gm(client.socket(),res);

        client.test_rev_enc_compare(5);
        client.disconnect();
    
    }
    catch (std::exception& e)
    {
        std::cout << "Exception: " << e.what() << std::endl;
    }
    
    return 0;
}