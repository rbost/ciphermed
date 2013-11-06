#pragma once

#include <vector>
#include <mpc/lsic.hh>
#include <mpc/private_comparison.hh>
#include <mpc/enc_comparison.hh>
#include <mpc/rev_enc_comparison.hh>
#include <mpc/linear_enc_argmax.hh>

#include <net/client.hh>
#include <net/server.hh>

#include <tree/tree.hh>
#include <tree/m_variate_poly.hh>

using namespace std;

#define N_LEVELS 3

class Decision_tree_Classifier_Server : public Server {
public:
    Decision_tree_Classifier_Server(gmp_randstate_t state, unsigned int keysize, const Tree<long> &model, unsigned int n_variables);
  
    Server_session* create_new_server_session(tcp::socket &socket);

    static Key_dependencies_descriptor key_deps_descriptor()
    {
        return Key_dependencies_descriptor(false,false,false,false,false,true);
    }

    Multivariate_poly< vector<long> > model_poly() const { return model_poly_; }
    unsigned int n_variables() const { return n_variables_; }
protected:
    Multivariate_poly< vector<long> > model_poly_;
    const unsigned int n_variables_;
};


class  Decision_tree_Classifier_Server_session : public Server_session{
public:
    
    Decision_tree_Classifier_Server_session(Decision_tree_Classifier_Server *server, gmp_randstate_t state, unsigned int id, tcp::socket &socket)
    : Server_session(server,state,id,socket), tree_server_(server) {};
    
    void run_session();
    
protected:
    Decision_tree_Classifier_Server *tree_server_;
};

class Decision_tree_Classifier_Client : public Client{
public:
    Decision_tree_Classifier_Client(boost::asio::io_service& io_service, gmp_randstate_t state, unsigned int keysize, vector<long> &query);
    
    void run();
    
protected:
    vector<long> query_;
};
