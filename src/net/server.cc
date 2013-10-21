#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <gmpxx.h>

#include <net/defs.hh>

#include <crypto/paillier.hh>

using boost::asio::ip::tcp;

void loop(tcp::socket &socket, Paillier_priv &pp)
{
    auto pk = pp.pubkey();
    boost::asio::streambuf buff;
    std::ostream buff_stream(&buff);
    
    buff_stream << "PAILLIER PK\n";
    buff_stream << pk[0] << "\n" << pk[1] << "\n";

    buff_stream << "END PAILLIER PK\n";
        
    boost::asio::write(socket, buff);

    // wait for a cyphertext to decrypt
    boost::asio::streambuf response;
    boost::asio::read_until(socket, response, "\n");
    std::istream response_stream(&response);
    std::string line;
    getline(response_stream,line);

    mpz_class c_v;
    c_v.set_str(line,10);

    
    std::cout << pp.decrypt(c_v) << std::endl;
}

int main()
{    
    gmp_randstate_t randstate;
    gmp_randinit_default(randstate);
    gmp_randseed_ui(randstate,time(NULL));
    
    auto sk = Paillier_priv::keygen(randstate,600);
    Paillier_priv pp(sk,randstate);

//    std::string pk_string;
    
    try
    {
        boost::asio::io_service io_service;
        
        tcp::endpoint endpoint(tcp::v4(), PORT);
        tcp::acceptor acceptor(io_service, endpoint);
        
        for (;;)
        {
            tcp::socket socket(io_service);
            acceptor.accept(socket);
            
            loop(socket,pp);
        }
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
    
    return 0;
}