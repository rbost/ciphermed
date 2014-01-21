#include <mpc/lsic.hh>
#include <mpc/private_comparison.hh>
#include <mpc/enc_comparison.hh>
#include <mpc/rev_enc_comparison.hh>
#include <mpc/linear_enc_argmax.hh>

#include <classifiers/nb_classifier.hh>

#include <protobuf/protobuf_conversion.hh>
#include <net/message_io.hh>
#include <util/util.hh>


Naive_Bayes_Classifier_Server::Naive_Bayes_Classifier_Server(gmp_randstate_t state, unsigned int keysize, unsigned int lambda, const vector<vector<vector<double>>> &conditionals_vec, const vector<double> &prior_vec)
: Server(state, Naive_Bayes_Classifier_Server::key_deps_descriptor(), keysize, lambda)
{
    // initialize the model arrays
    enc_prior_vec_ = vector<mpz_class>(prior_vec.size());
    enc_conditionals_vec_ =     vector<vector<vector<mpz_class>>>(conditionals_vec.size());

    for (size_t i = 0; i < enc_conditionals_vec_.size(); i++) {
        enc_conditionals_vec_[i] = vector<vector<mpz_class>>(conditionals_vec[i].size());
        
        for (size_t j = 0; j < enc_conditionals_vec_[i].size(); j++) {
            enc_conditionals_vec_[i][j] = vector<mpz_class>(conditionals_vec[i][j].size());

        }
    }
}

void Naive_Bayes_Classifier_Server::prepare_model(const vector<vector<vector<double>>> &conditionals_vec, const vector<double> &prior_vec)
{
    // first find the max exponent
    int e_max = INT_MIN;
    int e;
    
    for (size_t i = 0; i < prior_vec.size(); i++) {
        frexp(prior_vec[i], &e);
        e_max = max<int>(e,e_max);
    }
    
    for (size_t i = 0; i < conditionals_vec.size(); i++) {
        for (size_t j = 0; j < conditionals_vec[i].size(); j++) {
            for (size_t k = 0; k < conditionals_vec[i][j].size(); k++) {
                frexp(conditionals_vec[i][j][k], &e);
                e_max = max<int>(e,e_max);
            }
        }
    }
    
    // now, convert everything into multi-precision integers
    mpf_class temp;
    
    for (size_t i = 0; i < prior_vec.size(); i++) {
        temp = prior_vec[i];
        mpf_mul_2exp(temp.get_mpf_t(),temp.get_mpf_t(),e_max+52);
        enc_prior_vec_[i] = trunc(temp);
    }
    
    for (size_t i = 0; i < conditionals_vec.size(); i++) {
        for (size_t j = 0; j < conditionals_vec[i].size(); j++) {
            for (size_t k = 0; k < conditionals_vec[i][j].size(); k++) {

                temp = conditionals_vec[i][j][k];
                mpf_mul_2exp(temp.get_mpf_t(),temp.get_mpf_t(),e_max+52);
                
                enc_conditionals_vec_[i][j][k] = trunc(temp);

            }
        }
    }
}


void Naive_Bayes_Classifier_Server::encrypt_model()
{
    for (size_t i = 0; i < enc_prior_vec_.size(); i++) {
        enc_prior_vec_[i] = paillier_->encrypt(enc_prior_vec_[i]);
    }
    
    for (size_t i = 0; i < enc_conditionals_vec_.size(); i++) {
        for (size_t j = 0; j < enc_conditionals_vec_[i].size(); j++) {
            for (size_t k = 0; k < enc_conditionals_vec_[i][j].size(); k++) {
                enc_conditionals_vec_[i][j][k] = paillier_->encrypt(enc_conditionals_vec_[i][j][k]);
            }
        }
    }
}

Server_session* Naive_Bayes_Classifier_Server::create_new_server_session(tcp::socket &socket)
{
    return new Naive_Bayes_Classifier_Server_session(this, rand_state_, n_clients_++, socket);
}

void Naive_Bayes_Classifier_Server_session::run_session()
{
    try {
        exchange_keys();
        
        ScopedTimer *t;
        RESET_BYTE_COUNT
        RESET_BENCHMARK_TIMER

        // send the encrypted probabilities
        Protobuf::BigIntArray prior_prob_message = convert_to_message(nb_server_->enc_prior_prob());
        sendMessageToSocket(socket_, prior_prob_message);

        Protobuf::BigIntMatrix_Collection cond_prob_message = convert_to_message(nb_server_->enc_cond_prob());
        sendMessageToSocket(socket_, cond_prob_message);
        
        // help for the encrypted argmax
        Tree_EncArgmax_Helper helper(54,nb_server_->categories_count(),server_->paillier());
        run_tree_enc_argmax(helper,false);
        

#ifdef BENCHMARK
        cout << "Benchmark: " << GET_BENCHMARK_TIME << " ms" << endl;
        cout << IOBenchmark::byte_count() << " exchanged bytes" << endl;
        cout << IOBenchmark::interaction_count() << " interactions" << endl;
#endif

    } catch (std::exception& e) {
        std::cout << "Exception: " << e.what() << std::endl;
    }
    
    delete this;
}


Naive_Bayes_Classifier_Client::Naive_Bayes_Classifier_Client(boost::asio::io_service& io_service, gmp_randstate_t state, unsigned int keysize, unsigned int lambda, const vector<mpz_class> &vals, size_t bit_size)
: Client(io_service,state,Naive_Bayes_Classifier_Server::key_deps_descriptor(),keysize,lambda), bit_size_(bit_size),values_(vals)
{
    
}

bool Naive_Bayes_Classifier_Client::run()
{
    // get public keys
    RESET_BYTE_COUNT
    exchange_keys();
#ifdef BENCHMARK
    const double to_kB = 1 << 10;
    cout << "Key exchange: " <<  (IOBenchmark::byte_count()/to_kB) << " kB" << endl;
    cout << IOBenchmark::interaction_count() << " interactions" << endl;
#endif
    
    ScopedTimer *t;
    RESET_BYTE_COUNT
    RESET_BENCHMARK_TIMER
    // prepare data
//    vector <mpz_class> x = values_;
//    x.push_back(-1);
    
//    t = new ScopedTimer("Client: Compute dot product");
//    // compute the dot product
//    mpz_class v = compute_dot_product(x);
//    mpz_class w = 1; // encryption of 0
//    delete t;
//
//    t = new ScopedTimer("Client: Compare enc data");
//    // build the comparator over encrypted data
//    bool result = enc_comparison(v,w,bit_size_,false);
//    delete t;
#ifdef BENCHMARK
    cout << "Benchmark: " << GET_BENCHMARK_TIME << " ms" << endl;
    cout << (IOBenchmark::byte_count()/to_kB) << " exchanged kB" << endl;
    cout << IOBenchmark::interaction_count() << " interactions" << endl;
#endif
    return true;
}