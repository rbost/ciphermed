#include <iostream>
#include <string>
#include <vector>
#include <boost/asio.hpp>
#include <gmpxx.h>
#include <thread>

#include <FHE.h>
#include <EncryptedArray.h>
#include <util/fhe_util.hh>

#include <net/defs.hh>

#include <crypto/paillier.hh>
#include <mpc/lsic.hh>
#include <mpc/private_comparison.hh>
#include <mpc/enc_comparison.hh>
#include <mpc/rev_enc_comparison.hh>
#include <mpc/linear_enc_argmax.hh>

#include <net/server.hh>
#include <net/net_utils.hh>
#include <net/message_io.hh>

#include <protobuf/protobuf_conversion.hh>

using boost::asio::ip::tcp;

using namespace std;

Server::Server(gmp_randstate_t state, Key_dependencies_descriptor key_deps_desc, unsigned int keysize, unsigned int lambda)
: key_deps_desc_(key_deps_desc), paillier_(NULL), gm_(NULL), fhe_context_(NULL), fhe_sk_(NULL), n_clients_(0), lambda_(lambda)
{
    gmp_randinit_set(rand_state_, state);

    init_needed_keys(keysize);
}

Server::~Server()
{
    delete fhe_sk_;
    delete fhe_context_;
}

void Server::init_needed_keys(unsigned int keysize)
{
    if (key_deps_desc_.need_server_gm) {
        init_GM(keysize);
    }
    if (key_deps_desc_.need_server_paillier) {
        init_Paillier(keysize);
    }
    if (key_deps_desc_.need_server_fhe) {
        init_FHE_context();
        init_FHE_key();
    }
    
    
    if (key_deps_desc_.need_client_fhe) {
        init_FHE_context();
    }

}

void Server::init_GM(unsigned int keysize)
{
    if (gm_ != NULL) {
        return;
    }
    gm_ = new GM_priv(GM_priv::keygen(rand_state_,keysize),rand_state_);
}

void Server::init_Paillier(unsigned int keysize)
{
    if (paillier_ != NULL) {
        return;
    }
    
    paillier_ = new Paillier_priv_fast(Paillier_priv_fast::keygen(rand_state_,keysize), rand_state_);
        
}

void Server::init_FHE_context()
{
    if (fhe_context_) {
        return;
    }
    // generate a context. This one should be consisten with the server's one
    // i.e. m, p, r must be the same
    
    fhe_context_ = create_FHEContext(FHE_p,FHE_r,FHE_d,FHE_c,FHE_L,FHE_s,FHE_k,FHE_m);
    // we suppose d > 0
    fhe_G_ = makeIrredPoly(FHE_p, FHE_d);
}
void Server::init_FHE_key()
{
    if (fhe_sk_) {
        return;
    }

    fhe_sk_ = new FHESecKey(*fhe_context_);
    fhe_sk_->GenSecKey(FHE_w); // A Hamming-weight-w secret key
}

void Server::run()
{
    try
    {
        boost::asio::io_service io_service;
        
        tcp::endpoint endpoint(tcp::v4(), PORT);
        tcp::acceptor acceptor(io_service, endpoint);
        
        for (;;)
        {
            tcp::socket *socket = new tcp::socket(io_service);
            acceptor.accept(*socket);
            
            Server_session *c = create_new_server_session(socket);
            
            cout << "Start new connexion: " << c->id() << endl;
            thread t (&Server_session::run_session,c);
            t.detach();
        }
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
}


Server_session::Server_session(Server *server, gmp_randstate_t state, unsigned int id, tcp::socket *socket)
: server_(server), socket_(socket), client_gm_(NULL), client_paillier_(NULL), client_fhe_pk_(NULL), id_(id)
{
    gmp_randinit_set(rand_state_, state);
}

Server_session::~Server_session()
{
    if (client_gm_) {
        delete client_gm_;
    }
}

void Server_session::send_paillier_pk()
{
    boost::asio::streambuf buff;
    std::ostream buff_stream(&buff);
    
    cout << id_ << ": Send Paillier PK" << endl;
    Protobuf::Paillier_PK pk_message = get_pk_message(&(server_->paillier()));
    
    sendMessageToSocket<Protobuf::Paillier_PK>(*socket_,pk_message);
}

void Server_session::send_gm_pk()
{
    boost::asio::streambuf buff;
    std::ostream buff_stream(&buff);
    
    cout << id_ << ": Send GM PK" << endl;
    Protobuf::GM_PK pk_message = get_pk_message(&(server_->gm()));
    
    sendMessageToSocket<Protobuf::GM_PK>(*socket_,pk_message);
}

void Server_session::send_fhe_pk()
{
    const FHEPubKey& publicKey = server_->fhe_sk(); // cast so we only send the public informations
    cout << id_ << ": Send FHE PK" << endl;

    Protobuf::FHE_PK pk_message = get_pk_message(publicKey);
    
    sendMessageToSocket<Protobuf::FHE_PK>(*socket_,pk_message);

}

void Server_session::get_client_pk_gm()
{
    if (client_gm_) {
        return;
    }

    Protobuf::GM_PK pk = readMessageFromSocket<Protobuf::GM_PK>(*socket_);
    cout << id_ << ": Received GM PK" << endl;
    client_gm_ = create_from_pk_message(pk,rand_state_);
}

void Server_session::get_client_pk_paillier()
{
    if (client_paillier_) {
        return;
    }

    Protobuf::Paillier_PK pk = readMessageFromSocket<Protobuf::Paillier_PK>(*socket_);
    cout << id_ << ": Received Paillier PK" << endl;
    client_paillier_ = create_from_pk_message(pk,rand_state_);
}


void Server_session::get_client_pk_fhe()
{
    if (client_fhe_pk_) {
        return;
    }
    
    Protobuf::FHE_PK pk = readMessageFromSocket<Protobuf::FHE_PK>(*socket_);
    cout << id_ << ": Received FHE PK" << endl;
    client_fhe_pk_ = create_from_pk_message(pk,server_->fhe_context());
}


void Server_session::exchange_keys()
{
    Key_dependencies_descriptor key_deps_desc = server_->key_deps_desc();
    if (key_deps_desc.need_server_gm) {
        send_gm_pk();
    }
    if (key_deps_desc.need_server_paillier) {
        send_paillier_pk();
    }
    if (key_deps_desc.need_server_fhe) {
        send_fhe_pk();
    }

    
    if (key_deps_desc.need_client_gm) {
        get_client_pk_gm();
    }
    if (key_deps_desc.need_client_paillier) {
        get_client_pk_paillier();
    }
    if (key_deps_desc.need_client_fhe) {
        get_client_pk_fhe();
    }

}

void Server_session::run_comparison_protocol_B(Comparison_protocol_B *comparator)
{
    if(typeid(*comparator) == typeid(LSIC_B)) {
        run_lsic_B(reinterpret_cast<LSIC_B*>(comparator));
    }else if(typeid(*comparator) == typeid(Compare_B)){
        run_priv_compare_B(reinterpret_cast<Compare_B*>(comparator));
    }
}

void Server_session::run_lsic_B(LSIC_B *lsic)
{
    cout << id_ << ": Start LSIC B" << endl;
    boost::asio::streambuf output_buf;
    std::ostream output_stream(&output_buf);
    std::string line;
    
    LSIC_Packet_A a_packet;
    LSIC_Packet_B b_packet = lsic->setupRound();
    Protobuf::LSIC_A_Message a_message;
    Protobuf::LSIC_B_Message b_message;
    
    b_message = convert_to_message(b_packet);
    sendMessageToSocket(*socket_, b_message);
    
    cout << "LSIC setup sent" << endl;
    
    // wait for packets
    
    for (;b_packet.index < lsic->bitLength()-1; ) {
        a_message = readMessageFromSocket<Protobuf::LSIC_A_Message>(*socket_);
        a_packet = convert_from_message(a_message);

        b_packet = lsic->answerRound(a_packet);
        
        b_message = convert_to_message(b_packet);
        sendMessageToSocket(*socket_, b_message);
    }
    
    cout << id_ << ": LSIC B Done" << endl;

}

void Server_session::run_priv_compare_B(Compare_B *comparator)
{
    boost::asio::streambuf output_buf;
    std::ostream output_stream(&output_buf);
    std::istream input_stream(&input_buf_);
    std::string line;
    
    vector<mpz_class> c(comparator->bit_length());
    
    
    // send the encrypted bits
    Protobuf::BigIntArray c_b_message = convert_to_message(comparator->encrypt_bits());
    sendMessageToSocket(*socket_, c_b_message);

    // wait for the answer from the client
    Protobuf::BigIntArray c_message = readMessageFromSocket<Protobuf::BigIntArray>(*socket_);
    c = convert_from_message(c_message);
    
    
//    input_stream >> c;
    
    mpz_class c_t_prime = comparator->search_zero(c);
    
    // send the blinded result
    Protobuf::BigInt c_t_prime_message = convert_to_message(c_t_prime);
    sendMessageToSocket(*socket_, c_t_prime_message);
    
}

mpz_class Server_session::run_comparison_protocol_A(Comparison_protocol_A *comparator)
{
    if(typeid(*comparator) == typeid(LSIC_A)) {
        run_lsic_A(reinterpret_cast<LSIC_A*>(comparator));
    }else if(typeid(*comparator) == typeid(Compare_A)){
        run_priv_compare_A(reinterpret_cast<Compare_A*>(comparator));
    }
    
    return comparator->output();
}

mpz_class Server_session::run_lsic_A(LSIC_A *lsic)
{
    
    LSIC_Packet_A a_packet;
    LSIC_Packet_B b_packet;
    Protobuf::LSIC_A_Message a_message;
    Protobuf::LSIC_B_Message b_message;
    
    bool state;
    
    
    boost::asio::streambuf out_buff;
    std::ostream output_stream(&out_buff);
    string line;
    
    
    // response-request
    for (; ; ) {
        b_message = readMessageFromSocket<Protobuf::LSIC_B_Message>(*socket_);
        b_packet = convert_from_message(b_message);
        
        state = lsic->answerRound(b_packet,&a_packet);
        
        if (state) {
            return lsic->output();
        }
        
        a_message = convert_to_message(a_packet);
        sendMessageToSocket(*socket_, a_message);
    }
}

mpz_class Server_session::run_priv_compare_A(Compare_A *comparator)
{
    boost::asio::streambuf out_buff;
    std::ostream output_stream(&out_buff);
    string line;
    std::istream input_stream(&input_buf_);
    
    vector<mpz_class> c_b(comparator->bit_length());
    
    // first get encrypted bits
    
    Protobuf::BigIntArray c_b_message = readMessageFromSocket<Protobuf::BigIntArray>(*socket_);
    c_b = convert_from_message(c_b_message);
    
    vector<mpz_class> c_w = comparator->compute_w(c_b);
    vector<mpz_class> c_sums = comparator->compute_sums(c_w);
    vector<mpz_class> c = comparator->compute_c(c_b,c_sums);
    vector<mpz_class> c_rand = comparator->rerandomize(c);
    
    // we have to suffle
    random_shuffle(c_rand.begin(),c_rand.end(),[this](int n){ return gmp_urandomm_ui(rand_state_,n); });
    
    // send the result
    
    Protobuf::BigIntArray c_rand_message = convert_to_message(c_rand);
    sendMessageToSocket(*socket_, c_rand_message);
    
    // wait for the encrypted result
    mpz_class c_t_prime;
    
    Protobuf::BigInt c_t_prime_message = readMessageFromSocket<Protobuf::BigInt>(*socket_);
    c_t_prime = convert_from_message(c_t_prime_message);
    
    comparator->unblind(c_t_prime);
    
    return comparator->output();
}


bool Server_session::run_rev_enc_comparison(const size_t &l)
{
//    LSIC_B comparator(0,l,server_->gm());
    Compare_B comparator(0,l,server_->paillier(),server_->gm());
    
    Rev_EncCompare_Helper helper(l,server_->paillier(),&comparator);
    return run_rev_enc_comparison(helper);
}

bool Server_session::run_rev_enc_comparison(Rev_EncCompare_Helper &helper)
{
    boost::asio::streambuf output_buf;
    std::ostream output_stream(&output_buf);
    std::istream input_stream(&input_buf_);
    std::string line;

    
    // setup the helper if necessary
    if (!helper.is_set_up()) {
        Protobuf::Enc_Compare_Setup_Message setup_message = readMessageFromSocket<Protobuf::Enc_Compare_Setup_Message>(*socket_);
        if (setup_message.has_bit_length()) {
            helper.set_bit_length(setup_message.bit_length());
        }
        mpz_class c_z = convert_from_message(setup_message);

        helper.setup(c_z);
    }

    // now, we need to run the LSIC protocol
    run_comparison_protocol_B(helper.comparator());
    
    
    mpz_class c_z_l(helper.get_c_z_l());
    
    Protobuf::BigInt c_z_l_message = convert_to_message(c_z_l);
    sendMessageToSocket(*socket_, c_z_l_message);

    // wait for the answer of the owner
    Protobuf::BigInt c_t_message = readMessageFromSocket<Protobuf::BigInt>(*socket_);
    mpz_class c_t = convert_from_message(c_t_message);
    helper.decryptResult(c_t);
    return helper.output();

}

void Server_session::run_enc_comparison(const size_t &l, GM *gm)
{
    LSIC_A lsic(0,l,*gm);
    EncCompare_Helper helper(l,server_->paillier(),&lsic);
    run_enc_comparison(helper);
}

void Server_session::run_enc_comparison(EncCompare_Helper &helper)
{
    boost::asio::streambuf output_buf;
    std::ostream output_stream(&output_buf);
    std::istream input_stream(&input_buf_);
    std::string line;
    
    
    // setup the helper if necessary
    if (!helper.is_set_up()) {
        Protobuf::Enc_Compare_Setup_Message setup_message = readMessageFromSocket<Protobuf::Enc_Compare_Setup_Message>(*socket_);
        if (setup_message.has_bit_length()) {
            helper.set_bit_length(setup_message.bit_length());
        }
        mpz_class c_z = convert_from_message(setup_message);
        
        helper.setup(c_z);
    }
    
    // now, we need to run the comparison protocol
    run_comparison_protocol_A(helper.comparator());
    
    Protobuf::BigInt c_r_l_message = readMessageFromSocket<Protobuf::BigInt>(*socket_);
    mpz_class c_r_l = convert_from_message(c_r_l_message);

    mpz_class c_t = helper.concludeProtocol(c_r_l);

    // send the last message to the server
    Protobuf::BigInt c_t_message = convert_to_message(c_t);
    sendMessageToSocket(*socket_, c_t_message);
}


void Server_session::run_linear_enc_argmax(Linear_EncArgmax_Helper &helper)
{
    size_t k = helper.elements_number();
    size_t nbits = helper.bit_length();
    //    auto party_a_creator = [gm_ptr,p_ptr,nbits,randstate_ptr](){ return new Compare_A(0,nbits,*p_ptr,*gm_ptr,*randstate_ptr); };
    
    for (size_t i = 0; i < k - 1; i++) {
//        cout << "Round " << i << endl;
        Compare_B comparator(0,nbits,server_->paillier(),server_->gm());
//        LSIC_B comparator(0,nbits,server_->gm());

        Rev_EncCompare_Helper rev_enc_helper = helper.rev_enc_compare_helper(&comparator);
        
        run_rev_enc_comparison(rev_enc_helper);
        
        mpz_class randomized_enc_max, randomized_value;
        
        // read the values sent by the client
        randomized_enc_max = readIntFromSocket(*socket_);
        randomized_value = readIntFromSocket(*socket_);
        
        // and send the server's response
        mpz_class new_enc_max, x, y;
        helper.update_argmax(rev_enc_helper.output(), randomized_enc_max, randomized_value, i+1, new_enc_max, x, y);

        sendIntToSocket(*socket_,new_enc_max);
        sendIntToSocket(*socket_,x);
        sendIntToSocket(*socket_,y);
    }
    
    cout << "Send result" << endl;
    mpz_class permuted_argmax = helper.permuted_argmax();
    sendIntToSocket(*socket_, permuted_argmax);
}


EncCompare_Owner Server_session::create_enc_comparator_owner(size_t bit_size, bool use_lsic)
{
    Comparison_protocol_B *comparator;
    
    if (use_lsic) {
        comparator = new LSIC_B(0,bit_size,server_->gm());
    }else{
        comparator = new Compare_B(0,bit_size,server_->paillier(),server_->gm());
    }
    
    return EncCompare_Owner(0,0,bit_size,*client_paillier_,comparator,rand_state_);
}

EncCompare_Helper Server_session::create_enc_comparator_helper(size_t bit_size, bool use_lsic)
{

    Comparison_protocol_A *comparator;
    
    if (use_lsic) {
        comparator = new LSIC_A(0,bit_size,*client_gm_);
    }else{
        comparator = new Compare_A(0,bit_size,*client_paillier_,*client_gm_,rand_state_);
    }
    
    return EncCompare_Helper(bit_size,server_->paillier(),comparator);
}

Rev_EncCompare_Owner Server_session::create_rev_enc_comparator_owner(size_t bit_size, bool use_lsic)
{
    Comparison_protocol_A *comparator;
    
    if (use_lsic) {
        comparator = new LSIC_A(0,bit_size,*client_gm_);
    }else{
        comparator = new Compare_A(0,bit_size,*client_paillier_,*client_gm_,rand_state_);
    }
    
    return Rev_EncCompare_Owner(0,0,bit_size,*client_paillier_,comparator,rand_state_);
}


Rev_EncCompare_Helper Server_session::create_rev_enc_comparator_helper(size_t bit_size, bool use_lsic)
{
    Comparison_protocol_B *comparator;
    
    if (use_lsic) {
        comparator = new LSIC_B(0,bit_size,server_->gm());
    }else{
        comparator = new Compare_B(0,bit_size,server_->paillier(),server_->gm());
    }

    return Rev_EncCompare_Helper(bit_size,server_->paillier(),comparator);
}




void Server_session::decrypt_gm(const mpz_class &c)
{
    bool b = (server_->gm()).decrypt(c);
    cout << id_ << ": Decryption result = " << b << endl;
}

void Server_session::decrypt_fhe()
{
    Protobuf::FHE_Ctxt m = readMessageFromSocket<Protobuf::FHE_Ctxt>(*socket_);
    Ctxt c = convert_from_message(m, server_->fhe_sk());

    EncryptedArray ea(server_->fhe_sk().getContext(), server_->fhe_G());
    PlaintextArray pp0(ea);
    ea.decrypt(c, server_->fhe_sk(), pp0);
    cout << id_ << ": Decryption result = " << endl;
    pp0.print(cout);
    cout << endl;
}

