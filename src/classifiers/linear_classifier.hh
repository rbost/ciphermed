#pragma once

#include <vector>
#include <mpc/lsic.hh>
#include <mpc/private_comparison.hh>
#include <mpc/enc_comparison.hh>
#include <mpc/rev_enc_comparison.hh>
#include <mpc/linear_enc_argmax.hh>
#include <mpc/tree_enc_argmax.hh>

#include <net/client.hh>
#include <net/server.hh>

using namespace std;

class  Linear_Classifier_Server : public Server{
    public:
    Linear_Classifier_Server(gmp_randstate_t state, unsigned int keysize, unsigned int lambda, const vector<mpz_class> &model, size_t bit_size);
    
    Server_session* create_new_server_session(tcp::socket &socket);
    
    vector<mpz_class> enc_model() const { return enc_model_; }
    size_t bit_size() const { return bit_size_; }
    
    static Key_dependencies_descriptor key_deps_descriptor()
    {
        return Key_dependencies_descriptor(false,true,false,true,true,false);
    }

    protected:
    vector<mpz_class> enc_model_;
    size_t bit_size_;
};


class  Linear_Classifier_Server_session : public Server_session{
    public:
    
    Linear_Classifier_Server_session(Linear_Classifier_Server *server, gmp_randstate_t state, unsigned int id, tcp::socket &socket)
    : Server_session(server,state,id,socket), linear_server_(server) {};
    
    void run_session();
    
    protected:
    Linear_Classifier_Server *linear_server_;
};


class Linear_Classifier_Client : public Client{
public:
    Linear_Classifier_Client(boost::asio::io_service& io_service, gmp_randstate_t state, unsigned int keysize, unsigned int lambda, const vector<mpz_class> &vals, size_t bit_size);

    bool run();
    
protected:
    size_t bit_size_;
    vector<mpz_class> values_;
    vector<mpz_class> model_;
};


class  Bench_Linear_Classifier_Server : public Server{
public:
    Bench_Linear_Classifier_Server(gmp_randstate_t state, unsigned int keysize, unsigned int lambda, const vector<mpz_class> &model, size_t bit_size, unsigned int nRounds = 10);
    
    Server_session* create_new_server_session(tcp::socket &socket);
    
    vector<mpz_class> enc_model() const { return enc_model_; }
    size_t bit_size() const { return bit_size_; }
    unsigned int nRounds() const { return nRounds_; }
    
    static Key_dependencies_descriptor key_deps_descriptor()
    {
        return Key_dependencies_descriptor(false,true,false,true,true,false);
    }
    
protected:
    vector<mpz_class> enc_model_;
    size_t bit_size_;
    unsigned int nRounds_;
};


class  Bench_Linear_Classifier_Server_session : public Server_session{
public:
    
    Bench_Linear_Classifier_Server_session(Bench_Linear_Classifier_Server *server, gmp_randstate_t state, unsigned int id, tcp::socket &socket)
    : Server_session(server,state,id,socket), linear_server_(server) {};
    
    void run_session();
    
protected:
    Bench_Linear_Classifier_Server *linear_server_;
};


class Bench_Linear_Classifier_Client : public Client{
public:
    Bench_Linear_Classifier_Client(boost::asio::io_service& io_service, gmp_randstate_t state, unsigned int keysize, unsigned int lambda, const vector<mpz_class> &vals, size_t bit_size, unsigned int nRounds = 10);
    
    void run();
    
protected:
    size_t bit_size_;
    vector<mpz_class> values_;
    vector<mpz_class> model_;
    unsigned int nRounds_;
};
