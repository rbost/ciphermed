#include <mpc/lsic.hh>
#include <mpc/private_comparison.hh>
#include <mpc/enc_comparison.hh>
#include <mpc/rev_enc_comparison.hh>
#include <mpc/linear_enc_argmax.hh>

#include <classifiers/linear_classifier.hh>

#include <protobuf/protobuf_conversion.hh>
#include <net/message_io.hh>



Linear_Classifier_Server::Linear_Classifier_Server(gmp_randstate_t state, unsigned int keysize, unsigned int lambda, const vector<mpz_class> &model, size_t bit_size)
: Server(state, Linear_Classifier_Server::key_deps_descriptor(), keysize, lambda), enc_model_(model.size()), bit_size_(bit_size)
{
    for (size_t i = 0; i < enc_model_.size(); i++) {
        enc_model_[i] = paillier_->encrypt(model[i]);
    }
}

Server_session* Linear_Classifier_Server::create_new_server_session(tcp::socket &socket)
{
    return new Linear_Classifier_Server_session(this, rand_state_, n_clients_++, socket);
}

void Linear_Classifier_Server_session::run_session()
{
    try {
        exchange_keys();
        
        RESET_BENCHMARK_TIMER

        help_compute_dot_product(linear_server_->enc_model(),true);
        
        EncCompare_Helper helper = create_enc_comparator_helper(linear_server_->bit_size(), false);
        run_enc_comparison_helper(helper);

#ifdef BENCHMARK
        cout << "Benchmark: " << GET_BENCHMARK_TIME << " ms" << endl;
#endif

    } catch (std::exception& e) {
        std::cout << "Exception: " << e.what() << std::endl;
    }
    
    delete this;
}


Linear_Classifier_Client::Linear_Classifier_Client(boost::asio::io_service& io_service, gmp_randstate_t state, unsigned int keysize, unsigned int lambda, const vector<mpz_class> &vals, size_t bit_size)
: Client(io_service,state,Linear_Classifier_Server::key_deps_descriptor(),keysize,lambda), bit_size_(bit_size),values_(vals)
{
    
}

bool Linear_Classifier_Client::run()
{
    // get public keys
    exchange_keys();
    
    RESET_BENCHMARK_TIMER
    // prepare data
    vector <mpz_class> x = values_;
    x.push_back(-1);
    
    // compute the dot product
    mpz_class v = compute_dot_product(x);
    mpz_class w = 1; // encryption of 0
    

    // build the comparator over encrypted data
    EncCompare_Owner owner = create_enc_comparator_owner(bit_size_,false);
    owner.set_input(v, w);
    
    bool result = run_enc_comparison_owner(owner);
    
#ifdef BENCHMARK
    cout << "Benchmark: " << GET_BENCHMARK_TIME << " ms" << endl;
#endif
    return result;
}
