#include <mpc/lsic.hh>
#include <mpc/private_comparison.hh>
#include <mpc/enc_comparison.hh>
#include <mpc/rev_enc_comparison.hh>
#include <mpc/linear_enc_argmax.hh>
#include <mpc/tree_enc_argmax.hh>

#include <net/protocol_tester.hh>

#include <protobuf/protobuf_conversion.hh>
#include <net/message_io.hh>

#include <net/defs.hh>
#include <net/net_utils.hh>
#include <util/util.hh>

#include <FHE.h>
#include <EncryptedArray.h>

const bool use_lsic__ = true;

void Tester_Client::send_test_query(enum Test_Request_Request_Type type)
{
    Test_Request request;
    request.set_type(type);
    sendMessageToSocket<Test_Request>(socket_,request);
}

mpz_class Tester_Client::test_lsic(const mpz_class &a, size_t l)
{
    if (!has_gm_pk()) {
        get_server_pk_gm();
    }
    // send the start message
    send_test_query(Test_Request_Request_Type_TEST_LSIC);
    LSIC_A lsic(a,l,*server_gm_);
    return run_lsic_A(&lsic);
}

mpz_class Tester_Client::test_compare(const mpz_class &b, size_t l)
{
    if (!has_gm_pk()) {
        get_server_pk_gm();
    }
    if (!has_paillier_pk()) {
        get_server_pk_paillier();
    }
    // send the start message
    send_test_query(Test_Request_Request_Type_TEST_COMPARE);
    
    Compare_A comparator(b,l,*server_paillier_,*server_gm_,rand_state_);
    return run_priv_compare_A(&comparator);
}

void Tester_Client::test_enc_compare(size_t l)
{
    mpz_class a, b;
    mpz_urandom_len(a.get_mpz_t(), rand_state_, l);
    mpz_urandom_len(b.get_mpz_t(), rand_state_, l);
    
    //    cout << "a = " << a << endl;
    //    cout << "b = " << b << endl;
    
    mpz_class c_a, c_b;

    send_test_query(Test_Request_Request_Type_TEST_ENC_COMPARE);

    bool res = enc_comparison(server_paillier_->encrypt(a),server_paillier_->encrypt(b),l, use_lsic__);
    cout<< "\nResult is " << res << endl;
    cout << "Result should be " << (a < b) << endl;
}

void Tester_Client::test_multiple_enc_compare(size_t l)
{
    size_t n = 5;
    vector<mpz_class> a(n), b(n);
    
    for (size_t i = 0; i < n; i++) {
        mpz_urandom_len(a[i].get_mpz_t(), rand_state_, l);
        mpz_urandom_len(b[i].get_mpz_t(), rand_state_, l);
    }

    vector<mpz_class> c_a(n), c_b(n);
    for (size_t i = 0; i < n; i++) {
        c_a[i] = server_paillier_->encrypt(a[i]);
        c_b[i] = server_paillier_->encrypt(b[i]);
    }
    
    send_test_query(Test_Request_Request_Type_TEST_MULTIPLE_COMPARE);

    vector<bool> results = multiple_enc_comparison(c_a, c_b, l, use_lsic__);
    
    for (size_t i = 0; i < n; i++) {
        if ((a[i] < b[i]) != results[i]) {
            cout << "Result " << i << " is false" << endl;
        }
    }
}

void Tester_Client::test_rev_enc_compare(size_t l)
{
    mpz_class a, b;
    mpz_urandom_len(a.get_mpz_t(), rand_state_, l);
    mpz_urandom_len(b.get_mpz_t(), rand_state_, l);
    
    //    cout << "a = " << a << endl;
    //    cout << "b = " << b << endl;

    mpz_class c_a, c_b;

    send_test_query(Test_Request_Request_Type_TEST_REV_ENC_COMPARE);

    rev_enc_comparison(server_paillier_->encrypt(a),server_paillier_->encrypt(b),l, use_lsic__);
    
    cout << "\nResult should be " << (a < b) << endl;
}


void Tester_Client::test_linear_enc_argmax()
{
    size_t k = 5;
    size_t nbits = 100;
    
    vector<mpz_class> v(k);
    size_t real_argmax = 0;
    for (size_t i = 0; i < k; i++) {
        mpz_urandom_len(v[i].get_mpz_t(), rand_state_, nbits);
        //        v[i] = i;
        if (v[i] > v[real_argmax]) {
            real_argmax = i;
        }
    }
    for (size_t i = 0; i < k; i++) {
        v[i] = server_paillier_->encrypt(v[i]);
    }
    
    send_test_query(Test_Request_Request_Type_TEST_LINEAR_ENC_ARGMAX);
    
    Linear_EncArgmax_Owner owner(v,nbits,*server_paillier_,rand_state_, lambda_);
    
    ScopedTimer *t = new ScopedTimer("Linear enc argmax");
    
    run_linear_enc_argmax(owner,use_lsic__);
    delete t;
    
    size_t mpc_argmax = owner.output();
    assert(real_argmax == mpc_argmax);
    
    cout << "Real argmax = " << real_argmax;
    cout << "\nFound argmax = " << mpc_argmax << endl;
}

void Tester_Client::test_tree_enc_argmax()
{
    size_t k = 5;
    size_t nbits = 100;
    
    vector<mpz_class> v(k);
    size_t real_argmax = 0;
    for (size_t i = 0; i < k; i++) {
        mpz_urandom_len(v[i].get_mpz_t(), rand_state_, nbits);
        //        v[i] = i;
        if (v[i] > v[real_argmax]) {
            real_argmax = i;
        }
    }
    for (size_t i = 0; i < k; i++) {
        v[i] = server_paillier_->encrypt(v[i]);
    }
    
    send_test_query(Test_Request_Request_Type_TEST_TREE_ENC_ARGMAX);
    
    Tree_EncArgmax_Owner owner(v,nbits,*server_paillier_,rand_state_, lambda_);
    
    ScopedTimer *t = new ScopedTimer("Tree enc argmax");
    
    run_tree_enc_argmax(owner,use_lsic__);
    delete t;
    
    size_t mpc_argmax = owner.output();
    assert(real_argmax == mpc_argmax);
    
    cout << "Real argmax = " << real_argmax;
    cout << "\nFound argmax = " << mpc_argmax << endl;
}

void Tester_Client::test_fhe()
{
    get_server_pk_fhe();
    
    EncryptedArray ea(server_fhe_pk_->getContext(), fhe_G_);
    
    vector<long> bits(ea.size());
    for (size_t i = 0; i < bits.size(); i++) {
        bits[i] = gmp_urandomb_ui(rand_state_,1);
    }
    
    PlaintextArray p0(ea);
    p0.encode(bits);
    p0.print(cout);
    
    Ctxt c0(*server_fhe_pk_);
    ea.encrypt(c0, *server_fhe_pk_, p0);
    
    send_test_query(Test_Request_Request_Type_TEST_FHE);

    Protobuf::FHE_Ctxt m = convert_to_message(c0);
    sendMessageToSocket<Protobuf::FHE_Ctxt>(socket_,m);
}


void Tester_Client::test_change_es()
{
    send_test_query(Test_Request_Request_Type_TEST_CHANGE_ES);


    EncryptedArray ea(*fhe_context_, fhe_G_);
    
    size_t n_slots = ea.size();
    vector<long> bits_query(n_slots);
    
    vector<mpz_class> c_gm(bits_query.size());
    
    for (size_t i = 0; i < c_gm.size(); i++) {
        bits_query[i] = gmp_urandomb_ui(rand_state_,1);
        c_gm[i] = server_gm_->encrypt(bits_query[i]);
    }
    
    Ctxt c_fhe = change_encryption_scheme(c_gm);
    
    send_fhe_ctxt_to_socket(socket_, c_fhe);
    for (size_t i = 0; i < bits_query.size(); i++) {
        cout << "[" << bits_query[i] << "]";
    }
    cout << endl;
}

void Tester_Client::disconnect()
{
    cout << "Disconnect" << endl;
    
    send_test_query(Test_Request_Request_Type_DISCONNECT);

}



Server_session* Tester_Server::create_new_server_session(tcp::socket &socket)
{
    Tester_Server_session *s = new Tester_Server_session(this, rand_state_, n_clients_++, socket);
    return s;
}

enum Test_Request_Request_Type Tester_Server_session::get_test_query()
{
    Test_Request request = readMessageFromSocket<Test_Request>(socket_);
    
    return request.type();
}

void Tester_Server_session::run_session()
{
    cout << id_ << ": Start session" << endl;
    
    // exchange keys
    exchange_keys();
    
    // main loop to catch requests
    bool should_exit = false;
    try {
        for (;!should_exit; ) {

            // get the request
            Test_Request_Request_Type request_type = get_test_query();
            
            
            switch (request_type) {
                case Test_Request_Request_Type_TEST_LSIC:
                {
                    cout << id_ << ": Test LSIC" << endl;
                    mpz_class b(20);
                    test_lsic(b,100);
                }
                    break;

                case Test_Request_Request_Type_TEST_COMPARE:
                {
                    cout << id_ << ": Test Compare" << endl;
                    mpz_class b(20);
                    test_compare(b,100);
                }
                    break;

                case Test_Request_Request_Type_TEST_ENC_COMPARE:
                {
                    cout << id_ << ": Test Enc Compare" << endl;
                    help_enc_comparison(0,use_lsic__);
                }
                    break;
                    
                case Test_Request_Request_Type_TEST_REV_ENC_COMPARE:
                {
                    cout << id_ << ": Test Rev Enc Compare" << endl;
                    bool b = help_rev_enc_comparison(0,use_lsic__);
                    cout << id_ << ": Rev Enc Compare result: " << b << endl;
                }
                    break;
                    
                case Test_Request_Request_Type_TEST_LINEAR_ENC_ARGMAX:
                {
                    cout << id_ << ": Test Linear Enc Argmax" << endl;
                    Linear_EncArgmax_Helper helper(100,5,server_->paillier());
                    run_linear_enc_argmax(helper,use_lsic__);
                }
                    break;
                    
                case Test_Request_Request_Type_TEST_FHE:
                {
                    cout << id_ << ": Test FHE" << endl;
                    decrypt_fhe();
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
                    cout << id_ << ": Change ES" << endl;
                    test_change_es();
                }
                    break;
                    
                case Test_Request_Request_Type_TEST_MULTIPLE_COMPARE:
                {
                    cout << id_ << ": Test Multiple Enc Compare" << endl;
                    multiple_help_enc_comparison(5, 0,use_lsic__);
                }
                    break;
                 
                case Test_Request_Request_Type_TEST_TREE_ENC_ARGMAX:
                {
                    cout << id_ << ": Test Tree Enc Argmax" << endl;
                    Tree_EncArgmax_Helper helper(100,5,server_->paillier());
                    run_tree_enc_argmax(helper,use_lsic__);
                }
                    break;
                case Test_Request_Request_Type_TEST_OT:
                {
                    cout << id_ << ": Test OT" << endl;
                    test_ot();
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

/* TESTS */

void Tester_Server_session::test_lsic(const mpz_class &b,size_t l)
{
    cout << id_ << ": Start LSIC" << endl;
    LSIC_B lsic(b,l, server_->gm());
    run_lsic_B(&lsic);
}

void Tester_Server_session::test_compare(const mpz_class &a,size_t l)
{
    cout << id_ << ": Test compare" << endl;
    Compare_B comparator(a,l,server_->paillier(),server_->gm());
    run_priv_compare_B(&comparator);
}

void Tester_Server_session::test_change_es()
{
    run_change_encryption_scheme_slots_helper();
    
    // get the encryption from the client
    Ctxt c = read_fhe_ctxt_from_socket(socket_, server_->fhe_sk());
    
    EncryptedArray ea(server_->fhe_sk().getContext(), server_->fhe_G());
    PlaintextArray pp0(ea);
    ea.decrypt(c, server_->fhe_sk(), pp0);
    cout << id_ << ": Decryption result = " << endl;
    pp0.print(cout);
    cout << endl;
}

void Tester_Server_session::decrypt_gm(const mpz_class &c)
{
    bool b = (server_->gm()).decrypt(c);
    cout << id_ << ": Decryption result = " << b << endl;
}

void Tester_Server_session::decrypt_fhe()
{
    Protobuf::FHE_Ctxt m = readMessageFromSocket<Protobuf::FHE_Ctxt>(socket_);
    Ctxt c = convert_from_message(m, server_->fhe_sk());
    
    EncryptedArray ea(server_->fhe_sk().getContext(), server_->fhe_G());
    PlaintextArray pp0(ea);
    ea.decrypt(c, server_->fhe_sk(), pp0);
    cout << id_ << ": Decryption result = " << endl;
    pp0.print(cout);
    cout << endl;
}







void Tester_Client::test_ot()
{
    const int nOTs = 2;
    int choices[nOTs] = {0,1};
    
    char *messages = new char[nOTs*SHA1_BYTES];
    
    
    send_test_query(Test_Request_Request_Type_TEST_OT);

    ot_->receiver(nOTs, choices, messages, socket_);
    
    
    for (size_t i = 0; i < nOTs; i++) {
        for (size_t j = 0; j < SHA1_BYTES; j++) {
            cout << (int)messages[i*SHA1_BYTES + j];
        }
    }
    cout << endl;

}

void Tester_Server_session::test_ot()
{
    const int nOTs = 2;
    char *messages = new char [2*nOTs*SHA1_BYTES];
    
    for (size_t i = 0; i < 2*nOTs; i++) {
        for (size_t j = 0; j < SHA1_BYTES; j++) {
            messages[i*SHA1_BYTES + j] = i;
        }
    }
    
    for (size_t i = 0; i < nOTs; i++) {
        for (size_t j = 0; j < SHA1_BYTES; j++) {
            cout << (int) messages[2*i*SHA1_BYTES + j];
        }
        cout << ";";
    }
    cout << endl;
    for (size_t i = 0; i < nOTs; i++) {
        for (size_t j = 0; j < SHA1_BYTES; j++) {
            cout << (int) messages[(2*i+1)*SHA1_BYTES + j];
        }
        cout << ";";
    }
    cout << endl;

    ot_->sender(nOTs, messages, socket_);
}
