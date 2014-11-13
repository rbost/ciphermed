#include <mpc/lsic.hh>
#include <mpc/private_comparison.hh>
#include <mpc/garbled_comparison.hh>
#include <mpc/enc_comparison.hh>
#include <mpc/rev_enc_comparison.hh>
#include <mpc/linear_enc_argmax.hh>
#include <mpc/tree_enc_argmax.hh>

#include <net/protocol_bench.hh>

#include <protobuf/protobuf_conversion.hh>
#include <net/message_io.hh>

#include <net/defs.hh>
#include <net/net_utils.hh>
#include <net/oblivious_transfer.hh>
#include <util/util.hh>

#include <FHE.h>
#include <EncryptedArray.h>

#define OT_BLOCK_SIZE 16

static inline std::string protocol_string(COMPARISON_PROTOCOL comparison_prot)
{
    switch (comparison_prot) {
        case LSIC_PROTOCOL:
            return "LSIC";
            break;
            
        case DGK_PROTOCOL:
            return "DGK";
            break;
            
        case GC_PROTOCOL:
            return "Garbled Comparison";
            break;
            
        default:
            return "??";
            break;
    }
    return "??";
}

void Bench_Client::send_test_query(enum Test_Request_Request_Type type, unsigned int bit_size, unsigned int iterations, COMPARISON_PROTOCOL comparison_prot, unsigned int argmax_elements)
{
    Test_Request request;
    request.set_type(type);
    request.set_bit_size(bit_size);
    request.set_iterations(iterations);
    request.set_comparison_protocol(comparison_prot);
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
    
    double cpu_time = 0., total_time = 0.;
    Timer t;

    
    RESET_BYTE_COUNT

    for (unsigned int i = 0; i < iterations; i++) {
        mpz_urandom_len(a.get_mpz_t(), rand_state_, bit_size);
        
        RESET_BENCHMARK_TIMER
        t.lap(); // reset timer

        LSIC_A lsic(a,bit_size,*server_gm_);
        run_lsic_A(&lsic);
      
        cpu_time += GET_BENCHMARK_TIME;
        total_time += t.lap_ms();
    }

    cout << "Party A LSIC bench for " << iterations << " rounds, bit size=" << bit_size << endl;
    cout << "CPU time: " << cpu_time/iterations << endl;
    cout << "Total time: " << total_time/iterations << endl;
#ifdef BENCHMARK
    cout << (IOBenchmark::byte_count()/((double)iterations)) << " exchanged bytes per iteration" << endl;
    cout << (IOBenchmark::interaction_count()/((double)iterations)) << " interactions per iteration\n\n" << endl;
#endif
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
    
    double cpu_time = 0., total_time = 0.;
    Timer t;
    
    RESET_BYTE_COUNT
    
    for (unsigned int i = 0; i < iterations; i++) {
        mpz_urandom_len(b.get_mpz_t(), rand_state_, bit_size);
        
        RESET_BENCHMARK_TIMER
        t.lap(); // reset timer
        
        Compare_A comparator(b,bit_size,*server_paillier_,*server_gm_,rand_state_);
        run_priv_compare_A(&comparator);
        
        cpu_time += GET_BENCHMARK_TIME;
        total_time += t.lap_ms();
    }
    
    cout << "Party A DGK bench for " << iterations << " rounds, bit size=" << bit_size << endl;
    cout << "CPU time: " << cpu_time/iterations << endl;
    cout << "Total time: " << total_time/iterations << endl;
#ifdef BENCHMARK
    cout << (IOBenchmark::byte_count()/((double)iterations)) << " exchanged bytes per iteration" << endl;
    cout << (IOBenchmark::interaction_count()/((double)iterations)) << " interactions per iteration\n\n" << endl;
#endif
}

void Bench_Client::bench_garbled_compare(size_t bit_size, unsigned int iterations)
{
    if (!has_gm_pk()) {
        get_server_pk_gm();
    }
    // send the start message
    send_test_query(Test_Request_Request_Type_TEST_GARBLED_COMPARE, bit_size, iterations);
    
    mpz_class b;
    
    double cpu_time = 0., total_time = 0.;
    Timer t;
    
    RESET_BYTE_COUNT
    
    for (unsigned int i = 0; i < iterations; i++) {
        mpz_urandom_len(b.get_mpz_t(), rand_state_, bit_size);
        
        RESET_BENCHMARK_TIMER
        t.lap(); // reset timer
        
        GC_Compare_A comparator(b,bit_size,*server_gm_,rand_state_);
        run_garbled_compare_A(&comparator);
        
        cpu_time += GET_BENCHMARK_TIME;
        total_time += t.lap_ms();
    }
    
    cout << "Party A Garbled Comparison bench for " << iterations << " rounds, bit size=" << bit_size << endl;
    cout << "CPU time: " << cpu_time/iterations << endl;
    cout << "Total time: " << total_time/iterations << endl;
#ifdef BENCHMARK
    cout << (IOBenchmark::byte_count()/((double)iterations)) << " exchanged bytes per iteration" << endl;
    cout << (IOBenchmark::interaction_count()/((double)iterations)) << " interactions per iteration\n\n" << endl;
#endif
}

void Bench_Client::bench_enc_compare(size_t bit_size, unsigned int iterations, COMPARISON_PROTOCOL comparison_prot)
{
    
    send_test_query(Test_Request_Request_Type_TEST_ENC_COMPARE, bit_size, iterations, comparison_prot);

    mpz_class a, b;

    double cpu_time = 0., total_time = 0.;
    Timer t;
    
    RESET_BYTE_COUNT

    for (unsigned int i = 0; i < iterations; i++) {
        mpz_urandom_len(a.get_mpz_t(), rand_state_, bit_size);
        mpz_urandom_len(b.get_mpz_t(), rand_state_, bit_size);
        
        mpz_class c_a, c_b;
        c_a = server_paillier_->encrypt(a);
        c_b = server_paillier_->encrypt(b);
        
        RESET_BENCHMARK_TIMER
        t.lap(); // reset timer

        enc_comparison(c_a,c_b,bit_size, comparison_prot);

        cpu_time += GET_BENCHMARK_TIME;
        total_time += t.lap_ms();
    }
    
    cout << "Owner Enc Compare bench for " << iterations << " rounds, bit size=" << bit_size << " using " << protocol_string(comparison_prot) << endl;
    cout << "CPU time: " << cpu_time/iterations << endl;
    cout << "Total time: " << total_time/iterations << endl;
#ifdef BENCHMARK
    cout << (IOBenchmark::byte_count()/((double)iterations)) << " exchanged bytes per iteration" << endl;
    cout << (IOBenchmark::interaction_count()/((double)iterations)) << " interactions per iteration\n\n" << endl;
#endif
}

void Bench_Client::bench_rev_enc_compare(size_t bit_size, unsigned int iterations, COMPARISON_PROTOCOL comparison_prot)
{
    send_test_query(Test_Request_Request_Type_TEST_REV_ENC_COMPARE, bit_size, iterations, comparison_prot);

    mpz_class a, b;
    
    double cpu_time = 0., total_time = 0.;
    Timer t;

    RESET_BYTE_COUNT
    for (unsigned int i = 0; i < iterations; i++) {
        mpz_urandom_len(a.get_mpz_t(), rand_state_, bit_size);
        mpz_urandom_len(b.get_mpz_t(), rand_state_, bit_size);
        
        mpz_class c_a, c_b;
        c_a = server_paillier_->encrypt(a);
        c_b = server_paillier_->encrypt(b);

        RESET_BENCHMARK_TIMER
        t.lap(); // reset timer

        rev_enc_comparison(c_a,c_b,bit_size, comparison_prot);
        
        cpu_time += GET_BENCHMARK_TIME;
        total_time += t.lap_ms();
    }
    cout << "Owner Rev Enc Compare bench for " << iterations << " rounds, bit size=" << bit_size << " using " << protocol_string(comparison_prot) << endl;
    cout << "CPU time: " << cpu_time/iterations << endl;
    cout << "Total time: " << total_time/iterations << endl;
#ifdef BENCHMARK
    cout << (IOBenchmark::byte_count()/((double)iterations)) << " exchanged bytes per iteration" << endl;
    cout << (IOBenchmark::interaction_count()/((double)iterations)) << " interactions per iteration\n\n" << endl;
#endif
}


void Bench_Client::bench_linear_enc_argmax(size_t n_elements, size_t bit_size,unsigned int iterations, COMPARISON_PROTOCOL comparison_prot)
{
    size_t k = n_elements;
    size_t nbits = bit_size;
    send_test_query(Test_Request_Request_Type_TEST_LINEAR_ENC_ARGMAX, nbits, iterations, comparison_prot, n_elements);
    
    vector<mpz_class> v(k);
    
    double cpu_time = 0., total_time = 0.;
    Timer t;
    
    RESET_BYTE_COUNT

    for (unsigned int j = 0; j < iterations; j++) {
        for (size_t i = 0; i < k; i++) {
            mpz_urandom_len(v[i].get_mpz_t(), rand_state_, nbits);
        }
        for (size_t i = 0; i < k; i++) {
            v[i] = server_paillier_->encrypt(v[i]);
        }
        
        
        Linear_EncArgmax_Owner owner(v,nbits,*server_paillier_,rand_state_, lambda_);
        
        RESET_BENCHMARK_TIMER
        t.lap(); // reset timer
        
        run_linear_enc_argmax(owner,comparison_prot);
        
        cpu_time += GET_BENCHMARK_TIME;
        total_time += t.lap_ms();
    }
    
    cout << "Owner Enc Argmax bench for " << n_elements << " elements, " << iterations << " rounds, bit size=" << bit_size << " using " <<  protocol_string(comparison_prot) << endl;
    cout << "CPU time: " << cpu_time/iterations << endl;
    cout << "Total time: " << total_time/iterations << endl;
#ifdef BENCHMARK
    cout << (IOBenchmark::byte_count()/((double)iterations)) << " exchanged bytes per iteration" << endl;
    cout << (IOBenchmark::interaction_count()/((double)iterations)) << " interactions per iteration\n\n" << endl;
#endif
}

void Bench_Client::bench_tree_enc_argmax(size_t n_elements, size_t bit_size,unsigned int iterations, COMPARISON_PROTOCOL comparison_prot)
{
    size_t k = n_elements;
    size_t nbits = bit_size;
    send_test_query(Test_Request_Request_Type_TEST_TREE_ENC_ARGMAX, nbits, iterations, comparison_prot, n_elements);
    
    vector<mpz_class> v(k);
    
    double cpu_time = 0., total_time = 0.;
    Timer t;
    
    RESET_BYTE_COUNT

    for (unsigned int j = 0; j < iterations; j++) {
        for (size_t i = 0; i < k; i++) {
            mpz_urandom_len(v[i].get_mpz_t(), rand_state_, nbits);
        }
        for (size_t i = 0; i < k; i++) {
            v[i] = server_paillier_->encrypt(v[i]);
        }
        
        
        Tree_EncArgmax_Owner owner(v,nbits,*server_paillier_,rand_state_, lambda_);
        
        RESET_BENCHMARK_TIMER
        t.lap(); // reset timer
        
        run_tree_enc_argmax(owner,comparison_prot);
        
        cpu_time += GET_BENCHMARK_TIME;
        total_time += t.lap_ms();
    }
    
    cout << "Owner Tree Enc Argmax bench for " << n_elements << " elements, " << iterations << " rounds, bit size=" << bit_size << " using " <<  protocol_string(comparison_prot) << endl;
    cout << "CPU time: " << cpu_time/iterations << endl;
    cout << "Total time: " << total_time/iterations << endl;
#ifdef BENCHMARK
    cout << (IOBenchmark::byte_count()/((double)iterations)) << " exchanged bytes per iteration" << endl;
    cout << (IOBenchmark::interaction_count()/((double)iterations)) << " interactions per iteration\n\n" << endl;
#endif
}

void Bench_Client::bench_change_es(unsigned int iterations)
{
    send_test_query(Test_Request_Request_Type_TEST_CHANGE_ES,BIT_SIZE_DEFAULT, iterations);


    EncryptedArray ea(*fhe_context_, fhe_G_);
    
    size_t n_slots = ea.size();
    vector<long> bits_query(n_slots);
    
    vector<mpz_class> c_gm(bits_query.size());
    
    double cpu_time = 0., total_time = 0.;
    Timer t;
    
    RESET_BYTE_COUNT

    for (unsigned int j = 0; j < iterations; j++) {

        for (size_t i = 0; i < c_gm.size(); i++) {
            bits_query[i] = gmp_urandomb_ui(rand_state_,1);
            c_gm[i] = server_gm_->encrypt(bits_query[i]);
        }
        
        RESET_BENCHMARK_TIMER
        t.lap(); // reset timer

        Ctxt c_fhe = change_encryption_scheme(c_gm);
        
        cpu_time += GET_BENCHMARK_TIME;
        total_time += t.lap_ms();
    }
    
    cout << "Owner Change ES bench for " << iterations << " rounds" << endl;
    cout << "CPU time: " << cpu_time/iterations << endl;
    cout << "Total time: " << total_time/iterations << endl;
#ifdef BENCHMARK
    cout << (IOBenchmark::byte_count()/((double)iterations)) << " exchanged bytes per iteration" << endl;
    cout << (IOBenchmark::interaction_count()/((double)iterations)) << " interactions per iteration\n\n" << endl;
#endif
}


void Bench_Client::bench_ot(size_t n_elements ,unsigned int iterations)
{
    send_test_query(Test_Request_Request_Type_TEST_OT, BIT_SIZE_DEFAULT, iterations, GC_PROTOCOL, n_elements);

    const int nOTs = n_elements;
    int choices[n_elements];
    
    double cpu_time = 0., total_time = 0.;
    Timer t;
    
    RESET_BYTE_COUNT

    for (unsigned int k = 0; k < iterations; k++) {

        for (size_t i = 0; i < nOTs; i++) {
            choices[i] = gmp_urandomb_ui(rand_state_,1);
        }
        
        char *messages = new char[nOTs*OT_BLOCK_SIZE];
        
        
        RESET_BENCHMARK_TIMER
        t.lap(); // reset timer
        
        ObliviousTransfer::receiver(nOTs, choices, messages, socket_);
        
        cpu_time += GET_BENCHMARK_TIME;
        total_time += t.lap_ms();
    }
    
    cout << "OT receiver for " << iterations << " iterations" << endl;
    cout << "CPU time: " << cpu_time/iterations << endl;
    cout << "Total time: " << total_time/iterations << endl;
#ifdef BENCHMARK
    cout << (IOBenchmark::byte_count()/((double)iterations)) << " exchanged bytes per iteration" << endl;
    cout << (IOBenchmark::interaction_count()/((double)iterations)) << " interactions per iteration\n\n" << endl;
#endif

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

enum Test_Request_Request_Type Bench_Server_session::get_test_query(unsigned int &bit_size, unsigned int &iterations, COMPARISON_PROTOCOL &comparison_prot, unsigned int &argmax_elements)
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
    
    if (request.has_comparison_protocol()) {
        comparison_prot = (COMPARISON_PROTOCOL)request.comparison_protocol();
    }else{
        comparison_prot = GC_PROTOCOL;
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
            COMPARISON_PROTOCOL comparison_prot;
            Test_Request_Request_Type request_type = get_test_query(bit_size, iterations, comparison_prot, argmax_elements);
            
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
                    
                case Test_Request_Request_Type_TEST_GARBLED_COMPARE:
                {
                    cout << id_ << ": Bench Garbled Compare" << endl;
                    bench_garbled_compare(bit_size, iterations);
                }
                    break;
                    
                case Test_Request_Request_Type_TEST_ENC_COMPARE:
                {
                    cout << id_ << ": Bench Enc Compare" << endl;
                    bench_enc_compare(bit_size, iterations, comparison_prot);
                }
                    break;
                    
                case Test_Request_Request_Type_TEST_REV_ENC_COMPARE:
                {
                    cout << id_ << ": Bench Rev Enc Compare" << endl;
                    bench_rev_enc_compare(bit_size, iterations, comparison_prot);
                }
                    break;
                    
                case Test_Request_Request_Type_TEST_LINEAR_ENC_ARGMAX:
                {
                    cout << id_ << ": Bench Linear Enc Argmax" << endl;
                    bench_linear_enc_argmax(argmax_elements, bit_size, iterations, comparison_prot);
                }
                    break;

                case Test_Request_Request_Type_TEST_TREE_ENC_ARGMAX:
                {
                    cout << id_ << ": Bench Tree Enc Argmax" << endl;
                    bench_tree_enc_argmax(argmax_elements, bit_size, iterations, comparison_prot);
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
                case Test_Request_Request_Type_TEST_OT:
                {
                    cout << id_ << ": Bench OT" << endl;
                    bench_ot(argmax_elements,iterations);
                }
                    break;
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
    
    double cpu_time = 0.;

    for (unsigned int i = 0; i < iterations; i++) {
        mpz_urandom_len(b.get_mpz_t(), rand_state_, bit_size);
        
        RESET_BENCHMARK_TIMER
        
        LSIC_B lsic(b,bit_size, server_->gm());
        run_lsic_B(&lsic);
        
        cpu_time += GET_BENCHMARK_TIME;
    }
    
    cout << id_  << ": Party B LSIC bench for " << iterations << " rounds, bit size=" << bit_size << endl;
    cout << id_  << ": CPU time: " << cpu_time/iterations << endl;

}

void Bench_Server_session::bench_compare(size_t bit_size, unsigned int iterations)
{
    mpz_class a;

    double cpu_time = 0.;

    for (unsigned int i = 0; i < iterations; i++) {
        mpz_urandom_len(a.get_mpz_t(), rand_state_, bit_size);

        RESET_BENCHMARK_TIMER

        Compare_B comparator(a,bit_size,server_->paillier(),server_->gm());
        run_priv_compare_B(&comparator);

        cpu_time += GET_BENCHMARK_TIME;
    }

    cout << id_  << ": Party B DGK bench for " << iterations << " rounds, bit size=" << bit_size << endl;
    cout << id_  << ": CPU time: " << cpu_time/iterations << endl;

}

void Bench_Server_session::bench_garbled_compare(size_t bit_size, unsigned int iterations)
{
    mpz_class a;
    
    double cpu_time = 0.;
    
    for (unsigned int i = 0; i < iterations; i++) {
        mpz_urandom_len(a.get_mpz_t(), rand_state_, bit_size);
        
        RESET_BENCHMARK_TIMER
        
        GC_Compare_B comparator(a,bit_size,server_->gm(), rand_state_);
        run_garbled_compare_B(&comparator);
        
        cpu_time += GET_BENCHMARK_TIME;
    }
    
    cout << id_  << ": Party B Garbled Compare bench for " << iterations << " rounds, bit size=" << bit_size << endl;
    cout << id_  << ": CPU time: " << cpu_time/iterations << endl;
    
}

void Bench_Server_session::bench_enc_compare(size_t bit_size, unsigned int iterations, COMPARISON_PROTOCOL comparison_prot)
{
    double cpu_time = 0.;

    for (unsigned int i = 0; i < iterations; i++) {
        RESET_BENCHMARK_TIMER

        help_enc_comparison(bit_size,comparison_prot);
        
        cpu_time += GET_BENCHMARK_TIME;
    }
    cout << id_  << ": Helper Enc Compare bench for " << iterations << " rounds, bit size=" << bit_size << " using " <<  protocol_string(comparison_prot) << endl;
    cout << id_  << ": CPU time: " << cpu_time/iterations << endl;
}

void Bench_Server_session::bench_rev_enc_compare(size_t bit_size, unsigned int iterations, COMPARISON_PROTOCOL comparison_prot)
{
    double cpu_time = 0.;

    for (unsigned int i = 0; i < iterations; i++) {
        RESET_BENCHMARK_TIMER
        help_rev_enc_comparison(bit_size,comparison_prot);
        cpu_time += GET_BENCHMARK_TIME;
    }
    cout << id_  << ": Helper Rev Enc Compare bench for " << iterations << " rounds, bit size=" << bit_size << " using " <<  protocol_string(comparison_prot) << endl;
    cout << id_  << ": CPU time: " << cpu_time/iterations << endl;
}

void Bench_Server_session::bench_linear_enc_argmax(size_t n_elements, size_t bit_size,unsigned int iterations, COMPARISON_PROTOCOL comparison_prot)
{
    cout << id_  << ": Protocol:" <<  protocol_string(comparison_prot) << endl;
    
    double cpu_time = 0.;
    
    for (unsigned int i = 0; i < iterations; i++) {
        Linear_EncArgmax_Helper helper(bit_size,n_elements,server_->paillier());
        RESET_BENCHMARK_TIMER
        run_linear_enc_argmax(helper,comparison_prot);
        cpu_time += GET_BENCHMARK_TIME;
    }
    cout << id_  << ": Helper Enc Argmax bench for " << n_elements << " elements, " << iterations << " rounds, bit size=" << bit_size << " using " <<  protocol_string(comparison_prot) << endl;
    cout << id_  << ": CPU time: " << cpu_time/iterations << endl;
}

void Bench_Server_session::bench_tree_enc_argmax(size_t n_elements, size_t bit_size,unsigned int iterations, COMPARISON_PROTOCOL comparison_prot)
{
    
    double cpu_time = 0.;
    
    for (unsigned int i = 0; i < iterations; i++) {
        Tree_EncArgmax_Helper helper(bit_size,n_elements,server_->paillier());
        RESET_BENCHMARK_TIMER
        run_tree_enc_argmax(helper,comparison_prot);
        cpu_time += GET_BENCHMARK_TIME;
    }
    cout << id_  << ": Helper Tree Enc Argmax bench for " << n_elements << " elements, " << iterations << " rounds, bit size=" << bit_size << " using " <<  protocol_string(comparison_prot) << endl;
    cout << id_  << ": CPU time: " << cpu_time/iterations << endl;
}

void Bench_Server_session::bench_change_es(unsigned int iterations)
{
    double cpu_time = 0.;
    for (unsigned int i = 0; i < iterations; i++) {
        RESET_BENCHMARK_TIMER
        run_change_encryption_scheme_slots_helper();
        cpu_time += GET_BENCHMARK_TIME;
    }
    cout << id_  << ": Helper Change ES bench for " << iterations << " rounds" << endl;
    cout << id_  << ": CPU time: " << cpu_time/iterations << endl;
}


void Bench_Server_session::bench_ot(size_t n_elements, unsigned int iterations)
{
    const int nOTs = n_elements;
    char *messages = new char [2*nOTs*OT_BLOCK_SIZE];
    double cpu_time = 0.;

    for (unsigned int k = 0; k < iterations; k++) {

        for (size_t i = 0; i < 2*nOTs; i++) {
            for (size_t j = 0; j < SHA1_BYTES; j++) {
                messages[i*SHA1_BYTES + j] = gmp_urandomb_ui(rand_state_,8);
            }
        }

        RESET_BENCHMARK_TIMER
        ObliviousTransfer::sender(nOTs, messages, socket_);
        cpu_time += GET_BENCHMARK_TIME;
    }
    cout << id_  << ": Sender OT bench for " << iterations << " iterations" << endl;
    cout << id_  << ": CPU time: " << cpu_time/iterations << endl;

}
