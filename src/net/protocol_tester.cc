#include <mpc/lsic.hh>
#include <mpc/private_comparison.hh>
#include <mpc/enc_comparison.hh>
#include <mpc/rev_enc_comparison.hh>
#include <mpc/linear_enc_argmax.hh>

#include <net/protocol_tester.hh>

#include <protobuf/protobuf_conversion.hh>
#include <net/message_io.hh>

#include <net/defs.hh>
#include <util/util.hh>

#include <FHE.h>
#include <EncryptedArray.h>




mpz_class Tester_Client::test_lsic(const mpz_class &a, size_t l)
{
    if (!has_gm_pk()) {
        get_server_pk_gm();
    }
    // send the start message
    boost::asio::streambuf out_buff;
    std::ostream output_stream(&out_buff);
    output_stream << START_LSIC << "\n\r\n";
    boost::asio::write(socket_, out_buff);
    
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
    boost::asio::streambuf out_buff;
    std::ostream output_stream(&out_buff);
    output_stream << START_PRIV_COMP << "\n\r\n";
    boost::asio::write(socket_, out_buff);
    
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
    
    //    get_server_pk_gm();
    //    get_server_pk_paillier();
    
    
    boost::asio::streambuf out_buff;
    std::ostream output_stream(&out_buff);
    string line;
    // send the start message
    output_stream << START_ENC_COMPARE << "\n";
    output_stream << "\r\n";
    
    boost::asio::write(socket_, out_buff);
    
    
    // wait for the pk request from the server
    answer_server_pk_request();
    
    mpz_class c_a, c_b;
    
    bool res = run_enc_comparison(server_paillier_->encrypt(a),server_paillier_->encrypt(b),l);
    cout<< "\nResult is " << res << endl;
    cout << "Result should be " << (a < b) << endl;
}

void Tester_Client::test_rev_enc_compare(size_t l)
{
    mpz_class a, b;
    mpz_urandom_len(a.get_mpz_t(), rand_state_, l);
    mpz_urandom_len(b.get_mpz_t(), rand_state_, l);
    
    //    cout << "a = " << a << endl;
    //    cout << "b = " << b << endl;
    
    //    get_server_pk_gm();
    //    get_server_pk_paillier();
    
    boost::asio::streambuf out_buff;
    std::ostream output_stream(&out_buff);
    output_stream << START_REV_ENC_COMPARE << "\n";
    output_stream << "\r\n";
    
    boost::asio::write(socket_, out_buff);
    
    mpz_class c_a, c_b;
    
    run_rev_enc_compare(server_paillier_->encrypt(a),server_paillier_->encrypt(b),l);
    
    cout << "\nResult should be " << (a < b) << endl;
}


void Tester_Client::test_linear_enc_argmax()
{
    //    get_server_pk_gm();
    //    get_server_pk_paillier();
    
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
    
    boost::asio::streambuf out_buff;
    std::ostream output_stream(&out_buff);
    output_stream << TEST_ENC_ARGMAX << "\n";
    output_stream << "\r\n";
    
    boost::asio::write(socket_, out_buff);
    
    
    Linear_EncArgmax_Owner owner(v,nbits,*server_paillier_,rand_state_, lambda_);
    
    ScopedTimer *t = new ScopedTimer("Linear enc argmax");
    
    run_linear_enc_argmax(owner);
    delete t;
    
    size_t mpc_argmax = owner.output();
    assert(real_argmax == mpc_argmax);
    
    cout << "Real argmax = " << real_argmax;
    cout << "\nFound argmax = " << mpc_argmax << endl;
}

void Tester_Client::test_decrypt_gm(const mpz_class &c)
{
    boost::asio::streambuf buff;
    std::ostream buff_stream(&buff);
    
    buff_stream << DECRYPT_GM << "\n"<< c << "\n\r\n";
    boost::asio::write(socket_, buff);
    
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
    
    boost::asio::streambuf buff;
    std::ostream buff_stream(&buff);

    buff_stream << DECRYPT_FHE << "\n\r\n";
    boost::asio::write(socket_, buff);

    Protobuf::FHE_Ctxt m = convert_to_message(c0);
    sendMessageToSocket<Protobuf::FHE_Ctxt>(socket_,m);
}


void Tester_Client::disconnect()
{
    cout << "Disconnect" << endl;
    
    boost::asio::streambuf buff;
    std::ostream buff_stream(&buff);
    buff_stream << DISCONNECT << "\n\r\n";
    boost::asio::write(socket_, buff);
}



Server_session* Tester_Server::create_new_server_session(tcp::socket *socket)
{
    Tester_Server_session *s = new Tester_Server_session(this, rand_state_, n_clients_++, socket);
    return s;
}

void Tester_Server_session::run_session()
{
    cout << id_ << ": Start session" << endl;
    
    // exchange keys
    exchange_all_keys();
    send_fhe_pk();
    
    // main loop to catch requests
    bool should_exit = false;
    try {
        for (;!should_exit; ) {
            
            // wait for a complete request
            boost::asio::read_until(*socket_, input_buf_, "\r\n");
            
            std::istream input_stream(&input_buf_);
            std::string line;
            
            //    std::string s( (std::istreambuf_iterator<char>( input_stream )),
            //                  (std::istreambuf_iterator<char>()) );
            //    cout << s << endl;
            
            // parse the input
            do {
                getline(input_stream,line);
                //            cout << line;
                if (line == "") {
                    continue;
                }
                
                if (line == GET_PAILLIER_PK) {
                    send_paillier_pk();
                }else if(line == GET_GM_PK) {
                    send_gm_pk();
                }else if(line == GET_FHE_PK) {
                    send_fhe_pk();
                }else if(line == START_LSIC) {
                    mpz_class b(20);
                    test_lsic(b,100);
                }else if(line == START_PRIV_COMP) {
                    mpz_class b(20);
                    test_compare(b,100);
                }else if(line == DECRYPT_GM) {
                    mpz_class c;
                    getline(input_stream,line);
                    c.set_str(line,10);
                    decrypt_gm(c);
                }else if(line == DECRYPT_FHE) {
                    decrypt_fhe();
                }else if(line == START_REV_ENC_COMPARE){
                    // get the bit length and launch the helper
                    bool b = run_rev_enc_comparison(0);
                    
                    cout << id_ << ": Rev Enc Compare result: " << b << endl;
                }else if(line == START_ENC_COMPARE){
                    run_enc_comparison(0, client_gm_);
                }else if(line == TEST_ENC_ARGMAX){
                    Linear_EncArgmax_Helper helper(100,5,server_->paillier());
                    run_linear_enc_argmax(helper);
                }else if(line == DISCONNECT){
                    should_exit = true;
                    break;
                }
            } while (!input_stream.eof());
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
