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

#define BIT_SIZE_DEFAULT 64
#define ITERATIONS_DEFAULT 10

class  Bench_Server : public Server{
    public:
    Bench_Server(gmp_randstate_t state, unsigned int keysize, unsigned int lambda)
    : Server(state,Bench_Server::key_deps_descriptor(), keysize, lambda) {};
    
    Server_session* create_new_server_session(tcp::socket &socket);
    
    static Key_dependencies_descriptor key_deps_descriptor()
    {
        return Key_dependencies_descriptor(true,true,true,true,true,true);
    }
    
};

class Bench_Client : public Client{
    public:
    Bench_Client(boost::asio::io_service& io_service, gmp_randstate_t state, unsigned int keysize, unsigned int lambda)
    : Client(io_service,state,Bench_Server::key_deps_descriptor(),keysize,lambda) {};
    
    void send_test_query(enum Test_Request_Request_Type type, unsigned int bit_size = BIT_SIZE_DEFAULT, unsigned int iterations = ITERATIONS_DEFAULT, bool use_lsic = false, unsigned int argmax_elements = 0);

    void bench_lsic(size_t bit_size, unsigned int iterations);
    void bench_compare(size_t bit_size, unsigned int iterations);
    void bench_enc_compare(size_t bit_size, unsigned int iterations, bool use_lsic);
    void bench_rev_enc_compare(size_t bit_size, unsigned int iterations, bool use_lsic);
    void bench_linear_enc_argmax(size_t n_elements, size_t bit_size,unsigned int iterations, bool use_lsic);
    void bench_tree_enc_argmax(size_t n_elements, size_t bit_size,unsigned int iterations, bool use_lsic);
    void bench_change_es(unsigned int iterations);
    void bench_ot(size_t n_elements ,unsigned int iterations);
    
    void disconnect();
    
    protected:
    size_t bit_size_;
    vector<mpz_class> values_;
    vector<mpz_class> model_;
};

class  Bench_Server_session : public Server_session{
    public:
    
    Bench_Server_session(Bench_Server *server, gmp_randstate_t state, unsigned int id, tcp::socket &socket)
    : Server_session(server,state,id,socket){};
    
    void run_session();
    enum Test_Request_Request_Type get_test_query(unsigned int &bit_size, unsigned int &iterations, bool &use_lsic, unsigned int &argmax_elements);

    /* Test functions */
    void bench_lsic(size_t bit_size, unsigned int iterations);
    void bench_compare(size_t bit_size, unsigned int iterations);
    void bench_enc_compare(size_t bit_size, unsigned int iterations, bool use_lsic);
    void bench_rev_enc_compare(size_t bit_size, unsigned int iterations, bool use_lsic);
    void bench_linear_enc_argmax(size_t n_elements, size_t bit_size,unsigned int iterations, bool use_lsic);
    void bench_tree_enc_argmax(size_t n_elements, size_t bit_size,unsigned int iterations, bool use_lsic);
    void bench_change_es(unsigned int iterations);
    void bench_ot(size_t n_elements ,unsigned int iterations);

};
