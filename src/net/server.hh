#pragma once

#include <gmpxx.h>
#include <vector>
#include <boost/asio.hpp>

#include <FHE.h>

#include <crypto/paillier.hh>
#include <crypto/gm.hh>

#include <net/key_deps_descriptor.hh>

using boost::asio::ip::tcp;

using namespace std;

class Server_session;

class Server {
public:  
    Server(gmp_randstate_t state, Key_dependencies_descriptor key_deps_desc, unsigned int keysize, unsigned int lambda);
    virtual ~Server();
    
    virtual Server_session* create_new_server_session(tcp::socket &socket) = 0;
    void run();
    
    /* Keys management */

    void init_needed_keys(unsigned int keysize);
    void init_GM(unsigned int keysize);
    void init_Paillier(unsigned int keysize);
    void init_FHE_context();
    void init_FHE_key();

    
    Paillier_priv_fast& paillier() { assert(paillier_!=NULL); return *paillier_; };
    vector<mpz_class> paillier_pk() const { assert(paillier_!=NULL); return paillier_->pubkey(); }
    vector<mpz_class> paillier_sk() const { assert(paillier_!=NULL); return paillier_->privkey(); }
    GM_priv& gm() { assert(gm_!=NULL); return *gm_; };
    vector<mpz_class> gm_pk() const { assert(gm_!=NULL); return gm_->pubkey(); }
    vector<mpz_class> gm_sk() const { assert(gm_!=NULL); return {gm_->pubkey()[0],gm_->pubkey()[1],gm_->privkey()[0],gm_->privkey()[1]}; }
    
    const FHESecKey& fhe_sk() const { return *fhe_sk_; } // I don't want anyone to modify the secret key
    const FHEcontext& fhe_context() const { return *fhe_context_; }
    const ZZX& fhe_G() const { return fhe_G_; }
    
    Key_dependencies_descriptor key_deps_desc() const { return key_deps_desc_; }
    
    unsigned int lambda() const { return lambda_; }
protected:
    const Key_dependencies_descriptor key_deps_desc_;

    Paillier_priv_fast *paillier_;
    GM_priv *gm_;
    FHEcontext *fhe_context_;
    FHESecKey *fhe_sk_;
    ZZX fhe_G_;

    gmp_randstate_t rand_state_;
    unsigned int n_clients_;

    
    /* statistical security */
    unsigned int lambda_;
};

class Server_session {
public:
    Server_session(Server *server, gmp_randstate_t state, unsigned int id, tcp::socket &socket);
    virtual ~Server_session();
    
    virtual void run_session() = 0;
    
    void send_paillier_pk();
    void send_gm_pk();
    void send_fhe_pk();
    void get_client_pk_gm();
    void get_client_pk_paillier();
    void get_client_pk_fhe();
    void exchange_keys();

    mpz_class run_comparison_protocol_A(Comparison_protocol_A *comparator);
    mpz_class run_lsic_A(LSIC_A *lsic);
    mpz_class run_priv_compare_A(Compare_A *comparator);

    void run_comparison_protocol_B(Comparison_protocol_B *comparator);
    void run_lsic_B(LSIC_B *lsic);
    void run_priv_compare_B(Compare_B *comparator);

    bool run_enc_comparison_owner(const mpz_class &a, const mpz_class &b, size_t l, bool use_lsic);
    bool run_enc_comparison_owner(EncCompare_Owner &owner);
    void run_enc_comparison_helper(const size_t &l, bool use_lsic);
    void run_enc_comparison_helper(EncCompare_Helper &helper);
    
    void run_rev_enc_comparison_owner(const mpz_class &a, const mpz_class &b, size_t l, bool use_lsic);
    void run_rev_enc_comparison_owner(Rev_EncCompare_Owner &owner);
    bool run_rev_enc_comparison_helper(const size_t &l, bool use_lsic);
    bool run_rev_enc_comparison_helper(Rev_EncCompare_Helper &helper);

    void run_linear_enc_argmax(Linear_EncArgmax_Helper &helper, bool use_lsic);

    Ctxt change_encryption_scheme(const vector<mpz_class> &c_gm);
    void run_change_encryption_scheme_slots_helper();
    
    EncCompare_Owner create_enc_comparator_owner(size_t bit_size, bool use_lsic);
    EncCompare_Helper create_enc_comparator_helper(size_t bit_size, bool use_lsic);
    Rev_EncCompare_Owner create_rev_enc_comparator_owner(size_t bit_size, bool use_lsic);
    Rev_EncCompare_Helper create_rev_enc_comparator_helper(size_t bit_size, bool use_lsic);

    unsigned int id() const {return id_;}
    
protected:
    Server *server_;
    tcp::socket socket_;
    boost::asio::streambuf input_buf_;

    GM *client_gm_;
    Paillier *client_paillier_;
    FHEPubKey *client_fhe_pk_;
    gmp_randstate_t rand_state_;
    
    unsigned int id_;
};