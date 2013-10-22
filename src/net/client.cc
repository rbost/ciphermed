#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <gmpxx.h>

#include <net/defs.hh>

#include <crypto/paillier.hh>
#include <mpc/lsic.hh>
#include <net/net_utils.hh>

using boost::asio::ip::tcp;

using namespace std;


void send_mpz_class(const mpz_class &v, tcp::socket &socket)
{
    boost::asio::streambuf buff;
    std::ostream buff_stream(&buff);
    
    buff_stream << v << "\n";
    
    boost::asio::write(socket, buff);
}

void send_pk_paillier_request(tcp::socket &socket)
{
    boost::asio::streambuf buff;
    std::ostream buff_stream(&buff);
        
    buff_stream << "GET PK PAILLIER\n\r\n";
    boost::asio::write(socket, buff);
}

void send_pk_gm_request(tcp::socket &socket)
{
    boost::asio::streambuf buff;
    std::ostream buff_stream(&buff);
    
    cout << "Send PK request" << endl;
    
    buff_stream << "GET PK GM\n\r\n";
    //    buff_stream << "\r\n";
    boost::asio::write(socket, buff);
}

void send_disconnect_request(tcp::socket &socket)
{
    boost::asio::streambuf buff;
    std::ostream buff_stream(&buff);
    
    cout << "Disconnect" << endl;
    
    buff_stream << "DISCONNECT\n\r\n";
    //    buff_stream << "\r\n";
    boost::asio::write(socket, buff);
}

mpz_class run_lsic(tcp::socket &socket,const mpz_class &a,size_t l, const vector<mpz_class> &gm_pk, gmp_randstate_t randstate)
{
    LSIC_A lsic(a,l,gm_pk,randstate);

    LSIC_Packet_A a_packet;
    LSIC_Packet_B b_packet;
    bool state;
    
    
    boost::asio::streambuf out_buff;
    std::ostream output_stream(&out_buff);
    boost::asio::streambuf in_buff;
    string line;
    
    // send the start message
    output_stream << "START LSIC\n\r\n";
    boost::asio::write(socket, out_buff);
    
    // first get the setup round
    
    boost::asio::read_until(socket, in_buff, "\r\n");
    std::istream input_stream(&in_buff);

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
            boost::asio::write(socket, out_buff);
//            cout << "First packet sent to server" << endl;
            break;
        }
    } while (!input_stream.eof());

    // response-resquest
    for (; ; ) {
        boost::asio::read_until(socket, in_buff, "\r\n");
        std::istream input_stream(&in_buff);

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
                    boost::asio::write(socket, out_buff);

                    return lsic.output();
                }

                output_stream << "LSIC PACKET\n";
                output_stream << a_packet;
                output_stream << "\r\n";
                boost::asio::write(socket, out_buff);
                
            }
        } while (!input_stream.eof());
    }    
}

void decrypt_gm(tcp::socket &socket, mpz_class c)
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
        tcp::resolver resolver(io_service);
        tcp::resolver::query query(argv[1], "1990");
        tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
        tcp::socket socket(io_service);
        boost::asio::connect(socket, endpoint_iterator);
        
        
        
        send_pk_paillier_request(socket);
        
        boost::asio::streambuf response;
        boost::asio::read_until(socket, response, "END PAILLIER PK\n");
        
        std::istream response_stream(&response);
        std::string line;

        do {
            getline(response_stream,line);
        } while (line != "PAILLIER PK");
        
        // get the public key
        mpz_class n,g;
        getline(response_stream,line);
        n.set_str(line,10);
        getline(response_stream,line);
        g.set_str(line,10);
        
        send_pk_gm_request(socket);
        boost::asio::read_until(socket, response, "END GM PK\n");
        do {
            getline(response_stream,line);
        } while (line != "GM PK");
        // get the public key
        mpz_class N,y;
        getline(response_stream,line);
        N.set_str(line,10);
        getline(response_stream,line);
        y.set_str(line,10);


        // generate random cyphertext
        gmp_randstate_t randstate;
        gmp_randinit_default(randstate);
        gmp_randseed_ui(randstate,time(NULL));
//
//        mpz_class v;
//        mpz_urandomm(v.get_mpz_t(),randstate,n.get_mpz_t());
//
//        Paillier p({n,g},randstate);
//        
//        mpz_class c_v = p.encrypt(v);

        
        // send it to the server
//        send_mpz_class(c_v, socket);
//        cout << v << endl;
        
        mpz_class c = run_lsic(socket, 15,5,{N,y}, randstate);
        
//        GM gm({N,y},randstate);
        decrypt_gm(socket,c);
        
        send_disconnect_request(socket);
    }
    catch (std::exception& e)
    {
        std::cout << "Exception: " << e.what() << std::endl;
    }
    
    return 0;
}
