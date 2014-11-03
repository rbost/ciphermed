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
#include <mpc/garbled_comparison.hh>
#include <mpc/rev_enc_comparison.hh>
#include <mpc/enc_comparison.hh>
#include <mpc/linear_enc_argmax.hh>
#include <mpc/tree_enc_argmax.hh>

#include <math/util_gmp_rand.h>

#include <net/net_utils.hh>
#include <net/message_io.hh>
#include <util/util.hh>
#include <util/fhe_util.hh>

#include <net/client.hh>

#include <protobuf/protobuf_conversion.hh>

#include <net/exec_protocol.hh>
#include <net/oblivious_transfer.hh>

using boost::asio::ip::tcp;

using namespace std;

Client::Client(boost::asio::io_service& io_service, gmp_randstate_t state,Key_dependencies_descriptor key_deps_desc, unsigned int keysize, unsigned int lambda)
: socket_(io_service),key_deps_desc_(key_deps_desc), gm_(NULL), paillier_(NULL), server_paillier_(NULL), server_gm_(NULL), fhe_context_(NULL), server_fhe_pk_(NULL), fhe_sk_(NULL), n_threads_(2), lambda_(lambda)
{
    gmp_randinit_set(rand_state_, state);
    
    init_needed_keys(keysize);
    
    ObliviousTransfer::init(OT_SECPARAM);
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
    
    
    // for FHE keys, we need the FHE context from the server
//    if (key_deps_desc_.need_client_fhe) {
//        init_FHE_context();
//        init_FHE_key();
//    }
//    
//    if (key_deps_desc_.need_server_fhe) {
//        init_FHE_context();
//    }
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

void Client::get_fhe_context()
{
    if (fhe_context_) {
        return;
    }
    
    Protobuf::FHE_Context c = readMessageFromSocket<Protobuf::FHE_Context>(socket_);
    cout << "Received FHE Context" << endl;
    fhe_context_ = create_from_message(c);
    
    // we suppose d > 0
    fhe_G_ = makeIrredPoly(FHE_p, FHE_d);
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
    
    if (key_deps_desc_.need_client_gm) {
        send_gm_pk();
    }
    if (key_deps_desc_.need_client_paillier) {
        send_paillier_pk();
    }
    
    if (key_deps_desc_.need_server_fhe ||
        key_deps_desc_.need_client_fhe) {
        // if we use FHE, we need the context from the server before doing anything
        get_fhe_context();
    }
    
    if (key_deps_desc_.need_server_fhe) {
        get_server_pk_fhe();
    }
    if (key_deps_desc_.need_client_fhe) {
        // create the key only now
        init_FHE_key();
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

mpz_class Client::run_garbled_compare_A(GC_Compare_A *comparator)
{
    exec_garbled_compare_A(socket_,comparator);
    return comparator->output();
}



void Client::run_comparison_protocol_B(Comparison_protocol_B *comparator)
{
    exec_comparison_protocol_B(socket_,comparator,n_threads_);
}

void Client::run_lsic_B(LSIC_B *lsic)
{
    exec_lsic_B(socket_,lsic);
}

void Client::run_priv_compare_B(Compare_B *comparator)
{
    exec_priv_compare_B(socket_,comparator,n_threads_);
}

void Client::run_garbled_compare_B(GC_Compare_B *comparator)
{
    exec_garbled_compare_B(socket_,comparator);
}


// we suppose that the client already has the server's public key for Paillier
void Client::rev_enc_comparison(const mpz_class &a, const mpz_class &b, size_t l, COMPARISON_PROTOCOL comparison_prot)
{
    Rev_EncCompare_Owner owner = create_rev_enc_comparator_owner(l, comparison_prot);
    owner.set_input(a,b);
    run_rev_enc_comparison_owner(owner);
}

void Client::run_rev_enc_comparison_owner(Rev_EncCompare_Owner &owner)
{
    exec_rev_enc_comparison_owner(socket_, owner, lambda_, true, n_threads_);
}


bool Client::help_rev_enc_comparison(const size_t &l, COMPARISON_PROTOCOL comparison_prot)
{
    Rev_EncCompare_Helper helper = create_rev_enc_comparator_helper(l, comparison_prot);
    return run_rev_enc_comparison_helper(helper);
}

bool Client::run_rev_enc_comparison_helper(Rev_EncCompare_Helper &helper)
{
    exec_rev_enc_comparison_helper(socket_, helper, true, n_threads_);
    return helper.output();
}

// we suppose that the client already has the server's public key for Paillier
bool Client::enc_comparison(const mpz_class &a, const mpz_class &b, size_t l, COMPARISON_PROTOCOL comparison_prot)
{
    EncCompare_Owner owner = create_enc_comparator_owner(l, comparison_prot);
    owner.set_input(a,b);

    return run_enc_comparison_owner(owner);
}

bool Client::run_enc_comparison_owner(EncCompare_Owner &owner)
{
    exec_enc_comparison_owner(socket_, owner, lambda_, true, n_threads_);
    return owner.output();
}

void Client::help_enc_comparison(const size_t &l, COMPARISON_PROTOCOL comparison_prot)
{
    EncCompare_Helper helper = create_enc_comparator_helper(l, comparison_prot);
    run_enc_comparison_helper(helper);
}

void Client::run_enc_comparison_helper(EncCompare_Helper &helper)
{
    exec_enc_comparison_helper(socket_,helper, true, n_threads_);
}

// same as before, but the result is encrypted under QR
// recall that in this case, the prefix 'rev_' has to be flipped

mpz_class Client::run_rev_enc_comparison_owner_enc_result(Rev_EncCompare_Owner &owner)
{
    exec_rev_enc_comparison_owner(socket_, owner, lambda_, false, n_threads_);
    return owner.encrypted_output();
}

void Client::run_rev_enc_comparison_helper_enc_result(Rev_EncCompare_Helper &helper)
{
    exec_rev_enc_comparison_helper(socket_, helper, false, n_threads_);
}

void Client::run_enc_comparison_owner_enc_result(EncCompare_Owner &owner)
{
    exec_enc_comparison_owner(socket_, owner, lambda_, false, n_threads_);
}

mpz_class Client::run_enc_comparison_helper_enc_result(EncCompare_Helper &helper)
{
    exec_enc_comparison_helper(socket_,helper, false, n_threads_);
    return helper.encrypted_output();
}

// here we keep the old convention as these are the function meant to be called

void Client::rev_enc_comparison_enc_result(const mpz_class &a, const mpz_class &b, size_t l, COMPARISON_PROTOCOL comparison_prot)
{
    EncCompare_Owner owner = create_enc_comparator_owner(l, comparison_prot);
    owner.set_input(a,b);
    run_enc_comparison_owner_enc_result(owner);
}


mpz_class Client::help_rev_enc_comparison_enc_result(const size_t &l, COMPARISON_PROTOCOL comparison_prot)
{
    EncCompare_Helper helper = create_enc_comparator_helper(l, comparison_prot);
    return run_enc_comparison_helper_enc_result(helper);
}


mpz_class Client::enc_comparison_enc_result(const mpz_class &a, const mpz_class &b, size_t l, COMPARISON_PROTOCOL comparison_prot)
{
    Rev_EncCompare_Owner owner = create_rev_enc_comparator_owner(l, comparison_prot);
    owner.set_input(a,b);
    
    return run_rev_enc_comparison_owner_enc_result(owner);
}


void Client::help_enc_comparison_enc_result(const size_t &l, COMPARISON_PROTOCOL comparison_prot)
{
    Rev_EncCompare_Helper helper = create_rev_enc_comparator_helper(l, comparison_prot);
    run_rev_enc_comparison_helper_enc_result(helper);
}

vector<bool> Client::multiple_enc_comparison(const vector<mpz_class> &a, const vector<mpz_class> &b, size_t l, COMPARISON_PROTOCOL comparison_prot)
{
    assert(a.size() == b.size());
    size_t n = a.size();
    vector<EncCompare_Owner*> owners(5);
    
    for (size_t i = 0; i < n; i++) {
        owners[i] = new EncCompare_Owner(create_enc_comparator_owner(l, comparison_prot));
        owners[i]->set_input(a[i],b[i]);
    }

    unsigned int thread_per_job = ceilf(((float)n_threads_)/n);
    multiple_exec_enc_comparison_owner(socket_, owners, lambda_, true, thread_per_job);
    
    vector<bool> results(n);
    
    for (size_t i = 0; i < n; i++) {
        results[i] = owners[i]->output();
        delete owners[i];
    }
    
    return results;
}

void Client::multiple_help_enc_comparison(const size_t n, const size_t &l, COMPARISON_PROTOCOL comparison_prot)
{
    vector<EncCompare_Helper*> helpers(5);
    
    for (size_t i = 0; i < n; i++) {
        helpers[i] = new EncCompare_Helper(create_enc_comparator_helper(l, comparison_prot));
        
    }
   
    unsigned int thread_per_job = ceilf(((float)n_threads_)/n);
    multiple_exec_enc_comparison_helper(socket_, helpers, true, thread_per_job);
    
    for (size_t i = 0; i < n; i++) {
        delete helpers[i];
    }
}

void Client::multiple_rev_enc_comparison(const vector<mpz_class> &a, const vector<mpz_class> &b, size_t l, COMPARISON_PROTOCOL comparison_prot)
{
    assert(a.size() == b.size());
    size_t n = a.size();
    vector<Rev_EncCompare_Owner*> owners(n);
    
    for (size_t i = 0; i < n; i++) {
        owners[i] = new Rev_EncCompare_Owner(create_rev_enc_comparator_owner(l, comparison_prot));
        owners[i]->set_input(a[i],b[i]);
    }
    
    unsigned int thread_per_job = ceilf(((float)n_threads_)/n);
multiple_exec_rev_enc_comparison_owner(socket_, owners, lambda_, true, thread_per_job);
    
    
    for (size_t i = 0; i < n; i++) {
        delete owners[i];
    }
    
}


vector<bool> Client::multiple_help_rev_enc_comparison(const size_t n, const size_t &l, COMPARISON_PROTOCOL comparison_prot)
{
    vector<Rev_EncCompare_Helper*> helpers(n);
    
    for (size_t i = 0; i < n; i++) {
        helpers[i] = new Rev_EncCompare_Helper(create_rev_enc_comparator_helper(l, comparison_prot));
        
    }
    
    unsigned int thread_per_job = ceilf(((float)n_threads_)/n);
    multiple_exec_rev_enc_comparison_helper(socket_, helpers, true, thread_per_job);
    
    vector<bool> results(n);
    for (size_t i = 0; i < n; i++) {
        results[i] = helpers[i]->output();
        delete helpers[i];
    }
    
    return results;
}

size_t Client::run_linear_enc_argmax(Linear_EncArgmax_Owner &owner, COMPARISON_PROTOCOL comparison_prot)
{
    assert(has_paillier_pk());
    assert(has_gm_pk());
    
    size_t nbits = owner.bit_length();
    function<Comparison_protocol_A*()> comparator_creator;
    
    if (comparison_prot == LSIC_PROTOCOL) {
        comparator_creator = [this,nbits](){ return new LSIC_A(0,nbits,*server_gm_); };
    }else if (comparison_prot == DGK_PROTOCOL){
        comparator_creator = [this,nbits](){ return new Compare_A(0,nbits,*server_paillier_,*server_gm_,rand_state_); };
    }else if (comparison_prot == GC_PROTOCOL) {
        comparator_creator = [this,nbits](){ return new GC_Compare_A(0,nbits,*server_gm_, rand_state_); };
    }

    exec_linear_enc_argmax(socket_,owner, comparator_creator, lambda_, n_threads_);
    
    return owner.output();
}

size_t Client::run_tree_enc_argmax(Tree_EncArgmax_Owner &owner, COMPARISON_PROTOCOL comparison_prot)
{
    assert(has_paillier_pk());
    assert(has_gm_pk());
    
    size_t nbits = owner.bit_length();
    function<Comparison_protocol_A*()> comparator_creator;
    
    if (comparison_prot == LSIC_PROTOCOL) {
        comparator_creator = [this,nbits](){ return new LSIC_A(0,nbits,*server_gm_); };
    }else if (comparison_prot == DGK_PROTOCOL){
        comparator_creator = [this,nbits](){ return new Compare_A(0,nbits,*server_paillier_,*server_gm_,rand_state_); };
    }else if (comparison_prot == GC_PROTOCOL) {
        comparator_creator = [this,nbits](){ return new GC_Compare_A(0,nbits,*server_gm_, rand_state_); };
    }

    exec_tree_enc_argmax(socket_,owner, comparator_creator, lambda_, n_threads_);
    
    return owner.output();
}

Ctxt Client::change_encryption_scheme(const vector<mpz_class> &c_gm)
{
    EncryptedArray ea(*fhe_context_, fhe_G_);

    return exec_change_encryption_scheme_slots(socket_, c_gm, *server_gm_ ,*server_fhe_pk_, ea, rand_state_);
}

void Client::run_change_encryption_scheme_slots_helper()
{
    EncryptedArray ea(*fhe_context_, fhe_G_);
    exec_change_encryption_scheme_slots_helper(socket_, *gm_, *fhe_sk_, ea);
}


mpz_class Client::compute_dot_product(const vector<mpz_class> &x)
{
    return exec_compute_dot_product(socket_, x, *server_paillier_);
}

void Client::help_compute_dot_product(const vector<mpz_class> &y, bool encrypted_input)
{
    exec_help_compute_dot_product(socket_, y, *paillier_, encrypted_input);
}


EncCompare_Owner Client::create_enc_comparator_owner(size_t bit_size, COMPARISON_PROTOCOL comparison_prot)
{
    assert(has_paillier_pk());
    assert(gm_!=NULL);

    Comparison_protocol_B *comparator;
    
    if (comparison_prot == LSIC_PROTOCOL) {
        comparator = new LSIC_B(0,bit_size,*gm_);
    }else if (comparison_prot == DGK_PROTOCOL){
        assert(paillier_ != NULL);
        comparator = new Compare_B(0,bit_size,*paillier_,*gm_);
    }else if (comparison_prot == GC_PROTOCOL) {
        new GC_Compare_B(0,bit_size,*gm_, rand_state_);
    }

    return EncCompare_Owner(0,0,bit_size,*server_paillier_,comparator,rand_state_);
}

EncCompare_Helper Client::create_enc_comparator_helper(size_t bit_size, COMPARISON_PROTOCOL comparison_prot)
{
    assert(paillier_ != NULL);

    Comparison_protocol_A *comparator;
    
    if (comparison_prot == LSIC_PROTOCOL) {
        comparator = new LSIC_A(0,bit_size,*server_gm_);
    }else if (comparison_prot == DGK_PROTOCOL){
        comparator = new Compare_A(0,bit_size,*server_paillier_,*server_gm_,rand_state_);
    }else if (comparison_prot == GC_PROTOCOL) {
        comparator = new GC_Compare_A(0,bit_size,*server_gm_, rand_state_);
    }
    
    return EncCompare_Helper(bit_size,*paillier_,comparator);
}

Rev_EncCompare_Owner Client::create_rev_enc_comparator_owner(size_t bit_size, COMPARISON_PROTOCOL comparison_prot)
{
    assert(has_paillier_pk());
    assert(has_gm_pk());

    Comparison_protocol_A *comparator;
    
    if (comparison_prot == LSIC_PROTOCOL) {
        comparator = new LSIC_A(0,bit_size,*server_gm_);
    }else if (comparison_prot == DGK_PROTOCOL){
        comparator = new Compare_A(0,bit_size,*server_paillier_,*server_gm_,rand_state_);
    }else if (comparison_prot == GC_PROTOCOL) {
        comparator = new GC_Compare_A(0,bit_size,*server_gm_, rand_state_);
    }
    
    return Rev_EncCompare_Owner(0,0,bit_size,*server_paillier_,comparator,rand_state_);
}

Rev_EncCompare_Helper Client::create_rev_enc_comparator_helper(size_t bit_size, COMPARISON_PROTOCOL comparison_prot)
{
    assert(gm_!=NULL);
    assert(paillier_ != NULL);

    Comparison_protocol_B *comparator;
    
    if (comparison_prot == LSIC_PROTOCOL) {
        comparator = new LSIC_B(0,bit_size,*gm_);
    }else if (comparison_prot == DGK_PROTOCOL){
        assert(paillier_ != NULL);
        comparator = new Compare_B(0,bit_size,*paillier_,*gm_);
    }else if (comparison_prot == GC_PROTOCOL) {
        comparator = new GC_Compare_B(0,bit_size,*gm_, rand_state_);
    }
    
    return Rev_EncCompare_Helper(bit_size,*paillier_,comparator);
}