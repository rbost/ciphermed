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

#include <net/exec_protocol.hh>

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


mpz_class Server_session::run_comparison_protocol_A(Comparison_protocol_A *comparator)
{
    exec_comparison_protocol_A(*socket_,comparator,1);
    return comparator->output();
}

mpz_class Server_session::run_lsic_A(LSIC_A *lsic)
{
    exec_lsic_A(*socket_,lsic);
    return lsic->output();
}
mpz_class Server_session::run_priv_compare_A(Compare_A *comparator)
{
    exec_priv_compare_A(*socket_,comparator,1);
    return comparator->output();
}

void Server_session::run_comparison_protocol_B(Comparison_protocol_B *comparator)
{
    exec_comparison_protocol_B(*socket_,comparator);
}

void Server_session::run_lsic_B(LSIC_B *lsic)
{
    exec_lsic_B(*socket_,lsic);
}

void Server_session::run_priv_compare_B(Compare_B *comparator)
{
    exec_priv_compare_B(*socket_,comparator);
}

// we suppose that the client already has the server's public key for Paillier
void Server_session::run_rev_enc_comparison_owner(const mpz_class &a, const mpz_class &b, size_t l)
{
    //    LSIC_A comparator(0,l,*server_gm_);
    Compare_A comparator(0,l,*client_paillier_,*client_gm_,rand_state_);
    
    Rev_EncCompare_Owner owner(a,b,l,*client_paillier_,&comparator,rand_state_);
    run_rev_enc_comparison_owner(owner);
}

void Server_session::run_rev_enc_comparison_owner(Rev_EncCompare_Owner &owner)
{
    exec_rev_enc_comparison_owner(*socket_, owner, server_->lambda());
}

bool Server_session::run_rev_enc_comparison_helper(const size_t &l)
{
//    LSIC_B comparator(0,l,server_->gm());
    Compare_B comparator(0,l,server_->paillier(),server_->gm());
    
    Rev_EncCompare_Helper helper(l,server_->paillier(),&comparator);
    return run_rev_enc_comparison_helper(helper);
}

bool Server_session::run_rev_enc_comparison_helper(Rev_EncCompare_Helper &helper)
{
    exec_rev_enc_comparison_helper(*socket_, helper);
    return helper.output();
}

bool Server_session::run_enc_comparison_owner(const mpz_class &a, const mpz_class &b, size_t l)
{
    assert(client_paillier_ != NULL);

#warning WE MUST BE ABLE TO CHOOSE COMPARISON PROTOCOL

    LSIC_B lsic(0,l,server_->gm());
    EncCompare_Owner owner(a,b,l,*client_paillier_,&lsic,rand_state_);
    return run_enc_comparison_owner(owner);
}

bool Server_session::run_enc_comparison_owner(EncCompare_Owner &owner)
{
    exec_enc_comparison_owner(*socket_, owner, server_->lambda());
    return owner.output();
}

void Server_session::run_enc_comparison_helper(const size_t &l)
{
    assert(client_gm_ != NULL);
    
#warning WE MUST BE ABLE TO CHOOSE COMPARISON PROTOCOL
    LSIC_A lsic(0,l,*client_gm_);
    EncCompare_Helper helper(l,server_->paillier(),&lsic);
    run_enc_comparison_helper(helper);
}

void Server_session::run_enc_comparison_helper(EncCompare_Helper &helper)
{
    exec_enc_comparison_helper(*socket_,helper);
}


void Server_session::run_linear_enc_argmax(Linear_EncArgmax_Helper &helper)
{
    size_t nbits = helper.bit_length();

    auto comparator_creator = [this,nbits](){ return new Compare_B(0,nbits,server_->paillier(),server_->gm()); };

    exec_linear_enc_argmax(*socket_, helper, comparator_creator);
}

Ctxt Server_session::change_encryption_scheme(const vector<mpz_class> &c_gm)
{
    EncryptedArray ea(server_->fhe_context(), server_->fhe_G());
    
    return exec_change_encryption_scheme_slots(*socket_, c_gm, *client_gm_ ,*client_fhe_pk_, ea, rand_state_);
}


void Server_session::run_change_encryption_scheme_slots_helper()
{
    EncryptedArray ea(server_->fhe_context(), server_->fhe_G());
    exec_change_encryption_scheme_slots_helper(*socket_, server_->gm(), server_->fhe_sk(), ea);
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

