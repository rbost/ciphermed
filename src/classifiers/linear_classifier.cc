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
        
        ScopedTimer *t;
        RESET_BYTE_COUNT
        RESET_BENCHMARK_TIMER

        t = new ScopedTimer("Server: Compute dot product");
        help_compute_dot_product(linear_server_->enc_model(),true);
        delete t;
        
        t = new ScopedTimer("Server: Compare enc data");
        run_enc_comparison_helper(linear_server_->bit_size(), false);
        delete t;

#ifdef BENCHMARK
        cout << "Benchmark: " << GET_BENCHMARK_TIME << " ms" << endl;
        cout << IOBenchmark::byte_count() << " exchanged bytes" << endl;
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
    RESET_BYTE_COUNT
    exchange_keys();
#ifdef BENCHMARK
    const double to_kB = 1 << 10;
    cout << "Key exchange: " <<  (IOBenchmark::byte_count()/to_kB) << " kB" << endl;
#endif
    
    ScopedTimer *t;
    RESET_BYTE_COUNT
    RESET_BENCHMARK_TIMER
    // prepare data
    vector <mpz_class> x = values_;
    x.push_back(-1);
    
    t = new ScopedTimer("Client: Compute dot product");
    // compute the dot product
    mpz_class v = compute_dot_product(x);
    mpz_class w = 1; // encryption of 0
    delete t;

    t = new ScopedTimer("Client: Compare enc data");
    // build the comparator over encrypted data
    bool result = run_enc_comparison_owner(v,w,bit_size_,false);
    delete t;
#ifdef BENCHMARK
    cout << "Benchmark: " << GET_BENCHMARK_TIME << " ms" << endl;
    cout << (IOBenchmark::byte_count()/to_kB) << " exchanged kB" << endl;
#endif
    return result;
}
