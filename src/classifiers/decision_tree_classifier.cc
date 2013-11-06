#include <classifiers/decision_tree_classifier.hh>

#include <protobuf/protobuf_conversion.hh>
#include <net/message_io.hh>
#include <net/net_utils.hh>

#include <tree/util.hh>
#include <tree/util_poly.hh>

Decision_tree_Classifier_Server::Decision_tree_Classifier_Server(gmp_randstate_t state, unsigned int keysize, const Tree<long> &model, unsigned int n_variables)
: Server(state, Decision_tree_Classifier_Server::key_deps_descriptor(), keysize, 0), n_variables_(n_variables)
{
    EncryptedArray ea(*fhe_context_, fhe_G_);
    model_poly_ = model.to_polynomial_with_slots(ea.size());
    model_poly_ = mergeRegroup(model_poly_);

}

Server_session* Decision_tree_Classifier_Server::create_new_server_session(tcp::socket *socket)
{
    return new Decision_tree_Classifier_Server_session(this, rand_state_, n_clients_++, socket);
}

void Decision_tree_Classifier_Server_session::run_session()
{
    try {
        exchange_keys();
                
        // get the query
        vector<Ctxt> query;
        for (size_t i = 0; i < tree_server_->n_variables() ; i++) {
            query.push_back(read_fhe_ctxt_from_socket(*socket_, *client_fhe_pk_));
        }


    
        bool useShallowCircuit = true;
        EncryptedArray ea(server_->fhe_context(), server_->fhe_G());

        // evaluate the polynomial
        Ctxt c_r = evalPoly_FHE(tree_server_->model_poly(), query,ea,useShallowCircuit);

        // send the result back to the client
        send_fhe_ctxt_to_socket(*socket_, c_r);
    
    } catch (std::exception& e) {
        std::cout << "Exception: " << e.what() << std::endl;
    }
    
    delete this;
}

Decision_tree_Classifier_Client::Decision_tree_Classifier_Client(boost::asio::io_service& io_service, gmp_randstate_t state, unsigned int keysize, vector<long> &query)
: Client(io_service,state,Decision_tree_Classifier_Server::key_deps_descriptor(),keysize,0), query_(query)
{
    
}

void Decision_tree_Classifier_Client::run()
{
    exchange_keys();
    
    EncryptedArray ea(*fhe_context_, fhe_G_);

    vector<PlaintextArray> b(query_.size(),PlaintextArray(ea));
    vector<Ctxt> c_b(query_.size(),Ctxt(*fhe_sk_));
    
    for (size_t i = 0; i < query_.size(); i++) {
        b[i].encode(query_[i]);
        ea.encrypt(c_b[i],*fhe_sk_,b[i]);
    }

    // send the query to the server ...
    for (size_t i = 0; i < query_.size(); i++) {
        send_fhe_ctxt_to_socket(socket_, c_b[i]);
    }
    
    // ... and wait for the result
    Ctxt c_r = read_fhe_ctxt_from_socket(socket_,*fhe_sk_);
    
    // decrypt and test
    vector<long> res_bits;
    ea.decrypt(c_r, *fhe_sk_, res_bits);
    
    for (size_t i = 0; i < query_.size(); i++) {
        assert(res_bits[i] == query_[i]);
    }
    
    cout << "Test passed!" << endl;

}




