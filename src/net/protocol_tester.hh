#pragma once

#pragma once

#include <vector>
#include <mpc/lsic.hh>
#include <mpc/private_comparison.hh>
#include <mpc/enc_comparison.hh>
#include <mpc/rev_enc_comparison.hh>
#include <mpc/linear_enc_argmax.hh>

#include <net/client.hh>
#include <net/server.hh>

#include <proto_src/test_requests.pb.h>

using namespace std;



class  Tester_Server : public Server{
    public:
    Tester_Server(gmp_randstate_t state, unsigned int keysize, unsigned int lambda)
    : Server(state,Tester_Server::key_deps_descriptor(), keysize, lambda) {};
    
    Server_session* create_new_server_session(tcp::socket *socket);
    
    static Key_dependencies_descriptor key_deps_descriptor()
    {
        return Key_dependencies_descriptor(true,true,true,true,true,true);
    }
    
};

class Tester_Client : public Client{
    public:
    Tester_Client(boost::asio::io_service& io_service, gmp_randstate_t state, unsigned int keysize, unsigned int lambda)
    : Client(io_service,state,Tester_Server::key_deps_descriptor(),keysize,lambda) {};
    
    void send_test_query(enum Test_Request_Request_Type type);

    mpz_class test_lsic(const mpz_class &a, size_t l);
    mpz_class test_compare(const mpz_class &b, size_t l);
    void test_rev_enc_compare(size_t l);
    void test_enc_compare(size_t l);
    void test_linear_enc_argmax();
    void test_fhe();
    void disconnect();
    
    protected:
    size_t bit_size_;
    vector<mpz_class> values_;
    vector<mpz_class> model_;
};

class  Tester_Server_session : public Server_session{
    public:
    
    Tester_Server_session(Tester_Server *server, gmp_randstate_t state, unsigned int id, tcp::socket *socket)
    : Server_session(server,state,id,socket){};
    
    void run_session();
    enum Test_Request_Request_Type get_test_query();

    /* Test functions */
    void test_lsic(const mpz_class &b,size_t l);
    void test_compare(const mpz_class &a,size_t l);
};
