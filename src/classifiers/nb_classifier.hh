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

class  Naive_Bayes_Classifier_Server : public Server{
    public:
    Naive_Bayes_Classifier_Server(gmp_randstate_t state, unsigned int keysize, unsigned int lambda, const vector<vector<vector<double>>> &conditionals_vec, const vector<double> &prior_vec);
    
    Server_session* create_new_server_session(tcp::socket &socket);
    
    size_t categories_count() const { return enc_conditionals_vec_.size(); }
    
    const vector<mpz_class>& enc_prior_prob() const { return enc_prior_vec_; }
    const vector<vector<vector<mpz_class>>>& enc_cond_prob() const { return enc_conditionals_vec_; }
    
    
    static Key_dependencies_descriptor key_deps_descriptor()
    {
        return Key_dependencies_descriptor(false,true,false,true,true,false);
    }
    
    protected:
    void prepare_model(const vector<vector<vector<double>>> &conditionals_vec, const vector<double> &prior_vec);
    void encrypt_model();

    vector<mpz_class> enc_prior_vec_;
    vector<vector<vector<mpz_class>>> enc_conditionals_vec_;
};


class  Naive_Bayes_Classifier_Server_session : public Server_session{
    public:
    
    Naive_Bayes_Classifier_Server_session(Naive_Bayes_Classifier_Server *server, gmp_randstate_t state, unsigned int id, tcp::socket &socket)
    : Server_session(server,state,id,socket), nb_server_(server) {};
    
    void run_session();
    
    protected:
    Naive_Bayes_Classifier_Server *nb_server_;
};


class Naive_Bayes_Classifier_Client : public Client{
public:
    Naive_Bayes_Classifier_Client(boost::asio::io_service& io_service, gmp_randstate_t state, unsigned int keysize, unsigned int lambda, const vector<unsigned int> &features_value);

    bool run();
    vector<mpz_class> cat_probabilities() const;

protected:
    size_t bit_size_;
    const vector<unsigned int> features_value_;
    vector<mpz_class> enc_prior_vec_;
    vector<vector<vector<mpz_class>>> enc_conditionals_vec_;
};