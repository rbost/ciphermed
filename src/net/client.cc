#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <gmpxx.h>

#include <net/defs.hh>

#include <crypto/paillier.hh>

using boost::asio::ip::tcp;

using namespace std;

void send_mpz_class(const mpz_class &v, tcp::socket &socket)
{
    boost::asio::streambuf buff;
    std::ostream buff_stream(&buff);
    
    buff_stream << v << "\n";
    
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

        // generate random cyphertext
        gmp_randstate_t randstate;
        gmp_randinit_default(randstate);
        gmp_randseed_ui(randstate,time(NULL));

        mpz_class v;
        mpz_urandomm(v.get_mpz_t(),randstate,n.get_mpz_t());

        Paillier p({n,g},randstate);
        
        mpz_class c_v = p.encrypt(v);

        
        // send it to the server
        send_mpz_class(c_v, socket);
        cout << v << endl;
    }
    catch (std::exception& e)
    {
        std::cout << "Exception: " << e.what() << std::endl;
    }
    
    return 0;
}
