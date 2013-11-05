#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <gmpxx.h>

#include <FHE.h>
#include <EncryptedArray.h>

#include <net/defs.hh>

#include <crypto/gm.hh>
#include <mpc/lsic.hh>
#include <mpc/private_comparison.hh>
#include <mpc/rev_enc_comparison.hh>
#include <mpc/enc_comparison.hh>
#include <mpc/linear_enc_argmax.hh>

#include <math/util_gmp_rand.h>

#include <net/net_utils.hh>
#include <net/message_io.hh>
#include <util/util.hh>
#include <util/fhe_util.hh>

#include <net/client.hh>

#include <protobuf/protobuf_conversion.hh>

#include <net/exec_protocol.hh>

using boost::asio::ip::tcp;

using namespace std;

Client::Client(boost::asio::io_service& io_service, gmp_randstate_t state,Key_dependencies_descriptor key_deps_desc, unsigned int keysize, unsigned int lambda)
: socket_(io_service),key_deps_desc_(key_deps_desc), gm_(NULL), paillier_(NULL), server_paillier_(NULL), server_gm_(NULL), fhe_context_(NULL), server_fhe_pk_(NULL), fhe_sk_(NULL), n_threads_(2), lambda_(lambda)
{
    gmp_randinit_set(rand_state_, state);
    
    init_needed_keys(keysize);
}

Client::~Client()
{
    if (server_fhe_pk_ != NULL) {
        delete server_fhe_pk_;
    }
    delete fhe_context_;
}


void Client::connect(boost::asio::io_service& io_service, const string& hostname)
{
    tcp::resolver resolver(io_service);
    tcp::resolver::query query(hostname, to_string( PORT ));
    tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
    boost::asio::connect(socket_, endpoint_iterator);
}

void Client::init_needed_keys(unsigned int keysize)
{
    if (key_deps_desc_.need_client_gm) {
        init_GM(keysize);
    }
    if (key_deps_desc_.need_client_paillier) {
        init_Paillier(keysize);
    }
    if (key_deps_desc_.need_client_fhe) {
        init_FHE_context();
        init_FHE_key();
    }
    
    if (key_deps_desc_.need_server_fhe) {
        init_FHE_context();
    }
}

void Client::init_GM(unsigned int keysize)
{
    if (gm_ != NULL) {
        return;
    }
    gm_ = new GM_priv(GM_priv::keygen(rand_state_,keysize),rand_state_);
}

void Client::init_Paillier(unsigned int keysize)
{
    if (paillier_ != NULL) {
        return;
    }
    paillier_ = new Paillier_priv_fast(Paillier_priv_fast::keygen(rand_state_,keysize), rand_state_);

}

void Client::init_FHE_context()
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
void Client::init_FHE_key()
{
    if (fhe_sk_) {
        return;
    }
    
    fhe_sk_ = new FHESecKey(*fhe_context_);
    fhe_sk_->GenSecKey(FHE_w); // A Hamming-weight-w secret key
}


void Client::get_server_pk_gm()
{
    if (server_gm_) {
        return;
    }

    Protobuf::GM_PK pk = readMessageFromSocket<Protobuf::GM_PK>(socket_);
    cout << "Received GM PK" << endl;
    server_gm_ = create_from_pk_message(pk,rand_state_);
}


void Client::get_server_pk_paillier()
{
    if (server_paillier_) {
        return;
    }

    Protobuf::Paillier_PK pk = readMessageFromSocket<Protobuf::Paillier_PK>(socket_);
    cout << "Received Paillier PK" << endl;
    server_paillier_ = create_from_pk_message(pk,rand_state_);
}

void Client::get_server_pk_fhe()
{
    if (server_fhe_pk_) {
        return;
    }
    
    Protobuf::FHE_PK pk = readMessageFromSocket<Protobuf::FHE_PK>(socket_);
    cout << "Received FHE PK" << endl;
    server_fhe_pk_ = create_from_pk_message(pk,*fhe_context_);
}

void Client::send_gm_pk()
{
    assert(gm_!=NULL);
    Protobuf::GM_PK pk_message = get_pk_message(gm_);
    sendMessageToSocket<Protobuf::GM_PK>(socket_,pk_message);
}

void Client::send_paillier_pk()
{
    assert(paillier_ != NULL);
    Protobuf::Paillier_PK pk_message = get_pk_message(paillier_);
    sendMessageToSocket<Protobuf::Paillier_PK>(socket_,pk_message);
}

void Client::send_fhe_pk()
{
    const FHEPubKey& publicKey = *fhe_sk_; // cast so we only send the public informations
    
    Protobuf::FHE_PK pk_message = get_pk_message(publicKey);
    
    sendMessageToSocket<Protobuf::FHE_PK>(socket_,pk_message);
    
}

void Client::exchange_keys()
{
    if (key_deps_desc_.need_server_gm) {
        get_server_pk_gm();
    }
    if (key_deps_desc_.need_server_paillier) {
        get_server_pk_paillier();
    }
    if (key_deps_desc_.need_server_fhe) {
        get_server_pk_fhe();
    }

    
    if (key_deps_desc_.need_client_gm) {
        send_gm_pk();
    }
    if (key_deps_desc_.need_client_paillier) {
        send_paillier_pk();
    }
    if (key_deps_desc_.need_client_fhe) {
        send_fhe_pk();
    }
}

mpz_class Client::run_comparison_protocol_A(Comparison_protocol_A *comparator)
{
    exec_comparison_protocol_A(socket_,comparator,n_threads_);
    return comparator->output();
}

mpz_class Client::run_lsic_A(LSIC_A *lsic)
{
    exec_lsic_A(socket_,lsic);
    return lsic->output();
}

mpz_class Client::run_priv_compare_A(Compare_A *comparator)
{
    exec_priv_compare_A(socket_,comparator,n_threads_);
    return comparator->output();
}



void Client::run_comparison_protocol_B(Comparison_protocol_B *comparator)
{
    exec_comparison_protocol_B(socket_,comparator);
}

void Client::run_lsic_B(LSIC_B *lsic)
{
    exec_lsic_B(socket_,lsic);
}

void Client::run_priv_compare_B(Compare_B *comparator)
{
    exec_priv_compare_B(socket_,comparator);
}


// we suppose that the client already has the server's public key for Paillier
void Client::run_rev_enc_comparison_owner(const mpz_class &a, const mpz_class &b, size_t l)
{
    assert(has_paillier_pk());
    assert(has_gm_pk());
    
//    LSIC_A comparator(0,l,*server_gm_);
    Compare_A comparator(0,l,*server_paillier_,*server_gm_,rand_state_);

    Rev_EncCompare_Owner owner(a,b,l,*server_paillier_,&comparator,rand_state_);
    run_rev_enc_comparison_owner(owner);
}

void Client::run_rev_enc_comparison_owner(Rev_EncCompare_Owner &owner)
{
    exec_rev_enc_comparison_owner(socket_, owner, lambda_);
}


bool Client::run_rev_enc_comparison_helper(const size_t &l)
{
    assert(has_paillier_pk());
    assert(has_gm_pk());

    //    LSIC_B comparator(0,l,server_->gm());
    Compare_B comparator(0,l,*paillier_,*gm_);
    
    Rev_EncCompare_Helper helper(l,*paillier_,&comparator);
    return run_rev_enc_comparison_helper(helper);
}

bool Client::run_rev_enc_comparison_helper(Rev_EncCompare_Helper &helper)
{
    exec_rev_enc_comparison_helper(socket_, helper);
    return helper.output();
}

// we suppose that the client already has the server's public key for Paillier
bool Client::run_enc_comparison_owner(const mpz_class &a, const mpz_class &b, size_t l)
{
    assert(has_paillier_pk());
    assert(has_gm_pk());
    assert(gm_!=NULL);

    LSIC_B lsic(0,l,*gm_);
    EncCompare_Owner owner(a,b,l,*server_paillier_,&lsic,rand_state_);
    return run_enc_comparison_owner(owner);
}

bool Client::run_enc_comparison_owner(EncCompare_Owner &owner)
{
    exec_enc_comparison_owner(socket_, owner, lambda_);
    return owner.output();
}

void Client::run_enc_comparison_helper(const size_t &l)
{
    assert(server_gm_ != NULL);
#warning WE MUST BE ABLE TO CHOOSE COMPARISON PROTOCOL
    LSIC_A lsic(0,l,*server_gm_);
    EncCompare_Helper helper(l,*paillier_,&lsic);
    run_enc_comparison_helper(helper);
}

void Client::run_enc_comparison_helper(EncCompare_Helper &helper)
{
    exec_enc_comparison_helper(socket_,helper);
}



size_t Client::run_linear_enc_argmax(Linear_EncArgmax_Owner &owner)
{
    assert(has_paillier_pk());
    assert(has_gm_pk());
    
    size_t nbits = owner.bit_length();
    auto comparator_creator = [this,nbits](){ return new Compare_A(0,nbits,*server_paillier_,*server_gm_,rand_state_); };

    exec_linear_enc_argmax(socket_,owner, comparator_creator, lambda_);
    
    return owner.output();
}

EncCompare_Owner Client::create_enc_comparator_owner(size_t bit_size, bool use_lsic)
{
    assert(has_paillier_pk());
    assert(gm_!=NULL);

    Comparison_protocol_B *comparator;
    
    if (use_lsic) {
        comparator = new LSIC_B(0,bit_size,*gm_);
    }else{
        assert(paillier_ != NULL);
        comparator = new Compare_B(0,bit_size,*paillier_,*gm_);
    }

    return EncCompare_Owner(0,0,bit_size,*server_paillier_,comparator,rand_state_);
}

EncCompare_Helper Client::create_enc_comparator_helper(size_t bit_size, bool use_lsic)
{
    assert(paillier_ != NULL);

    Comparison_protocol_A *comparator;
    
    if (use_lsic) {
        comparator = new LSIC_A(0,bit_size,*server_gm_);
    }else{
        comparator = new Compare_A(0,bit_size,*server_paillier_,*server_gm_,rand_state_);
    }
    
    return EncCompare_Helper(bit_size,*paillier_,comparator);
}

Rev_EncCompare_Owner Client::create_rev_enc_comparator_owner(size_t bit_size, bool use_lsic)
{
    assert(has_paillier_pk());
    assert(has_gm_pk());

    Comparison_protocol_A *comparator;
    
    if (use_lsic) {
        comparator = new LSIC_A(0,bit_size,*server_gm_);
    }else{
        comparator = new Compare_A(0,bit_size,*server_paillier_,*server_gm_,rand_state_);
    }
    
    return Rev_EncCompare_Owner(0,0,bit_size,*server_paillier_,comparator,rand_state_);
}

Rev_EncCompare_Helper Client::create_rev_enc_comparator_helper(size_t bit_size, bool use_lsic)
{
    assert(gm_!=NULL);
    assert(paillier_ != NULL);

    Comparison_protocol_B *comparator;
    
    if (use_lsic) {
        comparator = new LSIC_B(0,bit_size,*gm_);
    }else{
        comparator = new Compare_B(0,bit_size,*paillier_,*gm_);
    }
    
    return Rev_EncCompare_Helper(bit_size,*paillier_,comparator);
}