#include <mpc/lsic.hh>
#include <mpc/private_comparison.hh>
#include <mpc/enc_comparison.hh>
#include <mpc/rev_enc_comparison.hh>
#include <mpc/linear_enc_argmax.hh>

#include <net/protocol_bench.hh>

#include <protobuf/protobuf_conversion.hh>
#include <net/message_io.hh>

#include <net/defs.hh>
#include <net/net_utils.hh>
#include <util/util.hh>

#include <FHE.h>
#include <EncryptedArray.h>

void Bench_Client::send_test_query(enum Test_Request_Request_Type type, unsigned int bit_size, unsigned int iterations, bool use_lsic, unsigned int argmax_elements)
{
    Test_Request request;
    request.set_type(type);
    request.set_bit_size(bit_size);
    request.set_iterations(iterations);
    request.set_use_lsic(use_lsic);
    request.set_argmax_elements(argmax_elements);
    sendMessageToSocket<Test_Request>(socket_,request);
}

void Bench_Client::bench_lsic(size_t bit_size, unsigned int iterations)
{
    if (!has_gm_pk()) {
        get_server_pk_gm();
    }
    // send the start message
    send_test_query(Test_Request_Request_Type_TEST_LSIC, bit_size, iterations);
    
    mpz_class a;
    for (unsigned int i = 0; i < iterations; i++) {
        mpz_urandom_len(a.get_mpz_t(), rand_state_, bit_size);
        LSIC_A lsic(a,bit_size,*server_gm_);
        run_lsic_A(&lsic);
    }
}

void Bench_Client::bench_compare(size_t bit_size, unsigned int iterations)
{
    if (!has_gm_pk()) {
        get_server_pk_gm();
    }
    if (!has_paillier_pk()) {
        get_server_pk_paillier();
    }
    // send the start message
    send_test_query(Test_Request_Request_Type_TEST_COMPARE, bit_size, iterations);
    
    mpz_class b;
    
    for (unsigned int i = 0; i < iterations; i++) {
        mpz_urandom_len(b.get_mpz_t(), rand_state_, bit_size);
        Compare_A comparator(b,bit_size,*server_paillier_,*server_gm_,rand_state_);
        run_priv_compare_A(&comparator);
    }
}

void Bench_Client::bench_enc_compare(size_t bit_size, unsigned int iterations, bool use_lsic)
{
    
    send_test_query(Test_Request_Request_Type_TEST_ENC_COMPARE, bit_size, iterations, use_lsic);

    mpz_class a, b;

    for (unsigned int i = 0; i < iterations; i++) {
        mpz_urandom_len(a.get_mpz_t(), rand_state_, bit_size);
        mpz_urandom_len(b.get_mpz_t(), rand_state_, bit_size);
        
        mpz_class c_a, c_b;
        c_a = server_paillier_->encrypt(a);
        c_b = server_paillier_->encrypt(b);
        
        enc_comparison(c_a,c_b,bit_size, use_lsic);
    }
}

void Bench_Client::bench_rev_enc_compare(size_t bit_size, unsigned int iterations, bool use_lsic)
{
    send_test_query(Test_Request_Request_Type_TEST_REV_ENC_COMPARE, bit_size, iterations, use_lsic);

    mpz_class a, b;
    
    for (unsigned int i = 0; i < iterations; i++) {
        mpz_urandom_len(a.get_mpz_t(), rand_state_, bit_size);
        mpz_urandom_len(b.get_mpz_t(), rand_state_, bit_size);
        
        mpz_class c_a, c_b;
        c_a = server_paillier_->encrypt(a);
        c_b = server_paillier_->encrypt(b);

        rev_enc_comparison(c_a,c_b,bit_size, use_lsic);
    }
}


void Bench_Client::bench_linear_enc_argmax(size_t n_elements, size_t bit_size,unsigned int iterations, bool use_lsic)
{
    size_t k = n_elements;
    size_t nbits = bit_size;
    send_test_query(Test_Request_Request_Type_TEST_LINEAR_ENC_ARGMAX, nbits, iterations, use_lsic, n_elements);

    vector<mpz_class> v(k);
    
    for (unsigned int j = 0; j < iterations; j++) {
        for (size_t i = 0; i < k; i++) {
            mpz_urandom_len(v[i].get_mpz_t(), rand_state_, nbits);
        }
        for (size_t i = 0; i < k; i++) {
            v[i] = server_paillier_->encrypt(v[i]);
        }
        
        
        Linear_EncArgmax_Owner owner(v,nbits,*server_paillier_,rand_state_, lambda_);
        
        run_linear_enc_argmax(owner,use_lsic);
    }
}

void Bench_Client::bench_change_es(unsigned int iterations)
{
    send_test_query(Test_Request_Request_Type_TEST_CHANGE_ES,BIT_SIZE_DEFAULT, iterations);


    EncryptedArray ea(*fhe_context_, fhe_G_);
    
    size_t n_slots = ea.size();
    vector<long> bits_query(n_slots);
    
    vector<mpz_class> c_gm(bits_query.size());
    
    for (unsigned int j = 0; j < iterations; j++) {

        for (size_t i = 0; i < c_gm.size(); i++) {
            bits_query[i] = gmp_urandomb_ui(rand_state_,1);
            c_gm[i] = server_gm_->encrypt(bits_query[i]);
        }
        
        Ctxt c_fhe = change_encryption_scheme(c_gm);
    }
}

void Bench_Client::disconnect()
{
    cout << "Disconnect" << endl;
    
    send_test_query(Test_Request_Request_Type_DISCONNECT);

}



Server_session* Bench_Server::create_new_server_session(tcp::socket &socket)
{
    Bench_Server_session *s = new Bench_Server_session(this, rand_state_, n_clients_++, socket);
    return s;
}

enum Test_Request_Request_Type Bench_Server_session::get_test_query(unsigned int &bit_size, unsigned int &iterations, bool &use_lsic, unsigned int &argmax_elements)
{
    Test_Request request = readMessageFromSocket<Test_Request>(socket_);
    
    if (request.has_bit_size()) {
        bit_size = request.bit_size();
    }else{
//        cout << "bit_size field not set, setting bit_size to be default value: " << BIT_SIZE_DEFAULT << endl;
        bit_size = BIT_SIZE_DEFAULT;
    }

    if (request.has_iterations()) {
        iterations = request.iterations();
    }else{
//        cout << "iterations field not set, setting iterations to be default value: " << ITERATIONS_DEFAULT << endl;
        iterations = ITERATIONS_DEFAULT;
    }
    
    if (request.has_use_lsic()) {
        use_lsic = request.use_lsic();
    }else{
        use_lsic = false;
    }
        
    if (request.has_argmax_elements()) {
        argmax_elements = request.argmax_elements();
    }else{
        argmax_elements = 0;
    }
    
    
    return request.type();
}

void Bench_Server_session::run_session()
{
    cout << id_ << ": Start session" << endl;
    
    // exchange keys
    exchange_keys();
    
    // main loop to catch requests
    bool should_exit = false;
    try {
        for (;!should_exit; ) {

            // get the request
            unsigned int bit_size, iterations, argmax_elements;
            bool use_lsic;
            Test_Request_Request_Type request_type = get_test_query(bit_size, iterations, use_lsic, argmax_elements);
            
            switch (request_type) {
                case Test_Request_Request_Type_TEST_LSIC:
                {
                    cout << id_ << ": Bench LSIC" << endl;
                    bench_lsic(bit_size, iterations);
                }
                    break;

                case Test_Request_Request_Type_TEST_COMPARE:
                {
                    cout << id_ << ": Bench DGK" << endl;
                    bench_compare(bit_size, iterations);
                }
                    break;

                case Test_Request_Request_Type_TEST_ENC_COMPARE:
                {
                    cout << id_ << ": Bench Enc Compare" << endl;
                    bench_enc_compare(bit_size, iterations, use_lsic);
                }
                    break;
                    
                case Test_Request_Request_Type_TEST_REV_ENC_COMPARE:
                {
                    cout << id_ << ": Bench Rev Enc Compare" << endl;
                    bench_rev_enc_compare(bit_size, iterations, use_lsic);
                }
                    break;
                    
                case Test_Request_Request_Type_TEST_LINEAR_ENC_ARGMAX:
                {
                    cout << id_ << ": Bench Linear Enc Argmax" << endl;
                    bench_linear_enc_argmax(argmax_elements, bit_size, iterations, use_lsic);
                }
                    break;
                    
                case Test_Request_Request_Type_TEST_FHE:
                {
                    cout << id_ << ": Cannot Bench FHE" << endl;
                }
                    break;
                    
                case Test_Request_Request_Type_DISCONNECT:
                {
                    cout << id_ << ": Disconnect" << endl;
                    should_exit = true;
                }
                    break;

                case Test_Request_Request_Type_TEST_CHANGE_ES:
                {
                    cout << id_ << ": Bench Change ES" << endl;
                    bench_change_es(iterations);
                }
                default:
                {
                    cout << id_ << ": Bad Request " << request_type << endl;
//                    should_exit = true;
                }
                    break;
            }
        }
        cout << id_ << ": Disconnected" << endl;
        
        
    } catch (std::exception& e) {
        std::cout << "Exception: " << e.what() << std::endl;
    }
    
    // we are done, delete ourself
    delete this;
}

/* BENCH CALLS */

void Bench_Server_session::bench_lsic(size_t bit_size, unsigned int iterations)
{
    mpz_class b;

    for (unsigned int i = 0; i < iterations; i++) {
        mpz_urandom_len(b.get_mpz_t(), rand_state_, bit_size);
        LSIC_B lsic(b,bit_size, server_->gm());
        run_lsic_B(&lsic);
    }
}

void Bench_Server_session::bench_compare(size_t bit_size, unsigned int iterations)

{

    mpz_class a;
    for (unsigned int i = 0; i < iterations; i++) {
        mpz_urandom_len(a.get_mpz_t(), rand_state_, bit_size);
        Compare_B comparator(a,bit_size,server_->paillier(),server_->gm());
        run_priv_compare_B(&comparator);
    }
}

void Bench_Server_session::bench_enc_compare(size_t bit_size, unsigned int iterations, bool use_lsic)
{
    for (unsigned int i = 0; i < iterations; i++) {
        help_enc_comparison(bit_size,use_lsic);
    }
}

void Bench_Server_session::bench_rev_enc_compare(size_t bit_size, unsigned int iterations, bool use_lsic)
{
    for (unsigned int i = 0; i < iterations; i++) {
        help_rev_enc_comparison(bit_size,use_lsic);
    }
}

void Bench_Server_session::bench_linear_enc_argmax(size_t n_elements, size_t bit_size,unsigned int iterations, bool use_lsic)
{

    for (unsigned int i = 0; i < iterations; i++) {
        Linear_EncArgmax_Helper helper(bit_size,n_elements,server_->paillier());
        run_linear_enc_argmax(helper,use_lsic);
    }
}

void Bench_Server_session::bench_change_es(unsigned int iterations)
{
    for (unsigned int i = 0; i < iterations; i++) {
        run_change_encryption_scheme_slots_helper();
    }
}
