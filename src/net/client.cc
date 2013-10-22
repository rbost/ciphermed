#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <gmpxx.h>

#include <net/defs.hh>

#include <crypto/gm.hh>
#include <mpc/lsic.hh>
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
    tcp::resolver::query query(hostname, "1990");
    tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
    boost::asio::connect(socket_, endpoint_iterator);
}


void Client::get_server_pk_gm()
{
    cout << "Resquest server's pubkey for GM" << endl;
    
    boost::asio::streambuf buff;
    std::ostream buff_stream(&buff);
    buff_stream << "GET PK GM\n\r\n";
    boost::asio::write(socket_, buff);
    
    
    boost::asio::read_until(socket_, input_buf_, "END GM PK\n");
    std::istream input_stream(&input_buf_);
    string line;
    
    do {
        getline(input_stream,line);
    } while (line != "GM PK");
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
    cout << "Resquest server's pubkey for Paillier" << endl;

    boost::asio::streambuf buff;
    std::ostream buff_stream(&buff);
    buff_stream << "GET PK PAILLIER\n\r\n";
    boost::asio::write(socket_, buff);
    string line;
    
    
    boost::asio::read_until(socket_, input_buf_, "END PAILLIER PK\n");
    std::istream input_stream(&input_buf_);
    
    do {
        getline(input_stream,line);
    } while (line != "PAILLIER PK");
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
    
    LSIC_A lsic(a,l,server_gm_->pubkey(),rand_state_);
    
    LSIC_Packet_A a_packet;
    LSIC_Packet_B b_packet;
    bool state;
    
    
    boost::asio::streambuf out_buff;
    std::ostream output_stream(&out_buff);
    string line;
    
    // send the start message
    output_stream << "START LSIC\n\r\n";
    boost::asio::write(socket_, out_buff);
    
    // first get the setup round
    
    boost::asio::read_until(socket_, input_buf_, "\r\n");
    std::istream input_stream(&input_buf_);
    
    // parse the input
    do {
        getline(input_stream,line);
        if (line == "") {
            continue;
        }
        
        if(line == "LSIC SETUP") {
            //            cout << "LSIC setup received" << endl;
            input_stream >> b_packet;
            
            state = lsic.answerRound(b_packet,&a_packet);
            
            if (state) {
                return lsic.output();
            }
            
            output_stream << "LSIC PACKET\n";
            output_stream << a_packet;
            output_stream << "\r\n";
            boost::asio::write(socket_, out_buff);
            //            cout << "First packet sent to server" << endl;
            break;
        }
    } while (!input_stream.eof());
    
    // response-resquest
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
            
            if(line == "LSIC PACKET") {
                input_stream >> b_packet;
                
                state = lsic.answerRound(b_packet,&a_packet);
                
                if (state) {
                    output_stream << "LSIC END\n";
                    output_stream << "\r\n";
                    boost::asio::write(socket_, out_buff);
                    
                    return lsic.output();
                }
                
                output_stream << "LSIC PACKET\n";
                output_stream << a_packet;
                output_stream << "\r\n";
                boost::asio::write(socket_, out_buff);
                
            }
        } while (!input_stream.eof());
    }      
}


void Client::disconnect()
{
    cout << "Disconnect" << endl;
    
    boost::asio::streambuf buff;
    std::ostream buff_stream(&buff);
    buff_stream << "DISCONNECT\n\r\n";
    boost::asio::write(socket_, buff);
}



//void decrypt_gm(tcp::socket &socket, mpz_class c)
//{
//    boost::asio::streambuf buff;
//    std::ostream buff_stream(&buff);
//    
//    buff_stream << "DECRYPT GM\n"<< c << "\n\r\n";
//    boost::asio::write(socket, buff);
//    
//}



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

        mpz_class res = client.run_lsic(15,5);

        client.disconnect();
    
//        decrypt_gm(client.socket,res);
    }
    catch (std::exception& e)
    {
        std::cout << "Exception: " << e.what() << std::endl;
    }
    
    return 0;
}