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

class Client {
public:
    Client(boost::asio::io_service& io_service, gmp_randstate_t state,Key_dependencies_descriptor key_deps_desc, unsigned int keysize, unsigned int lambda);
    ~Client();
    
    void connect(boost::asio::io_service& io_service, const string& hostname);

    tcp::socket& socket() { return socket_; }
    
    /* Keys management */
    void init_needed_keys(unsigned int keysize);
    void init_GM(unsigned int keysize);
    void init_Paillier(unsigned int keysize);
    void init_FHE_context();
    void init_FHE_key();

    const GM_priv& gm() { assert(gm_!=NULL); return *gm_; };
    vector<mpz_class> gm_pk() const { assert(gm_!=NULL); return gm_->pubkey(); }
    vector<mpz_class> gm_sk() const { assert(gm_!=NULL); return {gm_->pubkey()[0],gm_->pubkey()[1],gm_->privkey()[0],gm_->privkey()[1]}; }
    
    bool has_paillier_pk() const { return (server_paillier_ != NULL); }
    bool has_gm_pk() const { return (server_gm_ != NULL); }
    bool has_fhe_pk() const { return (server_fhe_pk_ != NULL); }
    void get_server_pk_gm();
    void get_server_pk_paillier();
    void get_fhe_context();
    void get_server_pk_fhe();
    
    void send_gm_pk();
    void send_paillier_pk();
    void send_fhe_pk();

    void exchange_keys();

    mpz_class run_comparison_protocol_A(Comparison_protocol_A *comparator);
    mpz_class run_lsic_A(LSIC_A *lsic);
    mpz_class run_priv_compare_A(Compare_A *comparator);

    void run_comparison_protocol_B(Comparison_protocol_B *comparator);
    void run_lsic_B(LSIC_B *lsic);
    void run_priv_compare_B(Compare_B *comparator);

    /* calls for (reversed) comparison over encrypted data */
    bool enc_comparison(const mpz_class &a, const mpz_class &b, size_t l, bool use_lsic);
    void help_enc_comparison(const size_t &l, bool use_lsic);

    void rev_enc_comparison(const mpz_class &a, const mpz_class &b, size_t l, bool use_lsic);
    bool help_rev_enc_comparison(const size_t &l, bool use_lsic);

    void rev_enc_comparison_enc_result(const mpz_class &a, const mpz_class &b, size_t l, bool use_lsic);
    
    mpz_class help_rev_enc_comparison_enc_result(const size_t &l, bool use_lsic);
    
    mpz_class enc_comparison_enc_result(const mpz_class &a, const mpz_class &b, size_t l, bool use_lsic);
    void help_enc_comparison_enc_result(const size_t &l, bool use_lsic);
    
    vector<bool> multiple_enc_comparison(const vector<mpz_class> &a, const vector<mpz_class> &b, size_t l, bool use_lsic);
    void multiple_help_enc_comparison(const size_t n, const size_t &l, bool use_lsic);

    void multiple_rev_enc_comparison(const vector<mpz_class> &a, const vector<mpz_class> &b, size_t l, bool use_lsic);
    vector<bool> multiple_help_rev_enc_comparison(const size_t n, const size_t &l, bool use_lsic);

    /* other protocols */

    Ctxt change_encryption_scheme(const vector<mpz_class> &c_gm);
    void run_change_encryption_scheme_slots_helper();

    mpz_class compute_dot_product(const vector<mpz_class> &x);
    void help_compute_dot_product(const vector<mpz_class> &y, bool encrypted_input = false);
    
    /* calls to the comparison owner and helper objects */
    
    bool run_enc_comparison_owner(EncCompare_Owner &owner);
    void run_enc_comparison_helper(EncCompare_Helper &helper);
    void run_rev_enc_comparison_owner(Rev_EncCompare_Owner &owner);
    bool run_rev_enc_comparison_helper(Rev_EncCompare_Helper &helper);

    mpz_class run_rev_enc_comparison_owner_enc_result(Rev_EncCompare_Owner &owner);
    void run_rev_enc_comparison_helper_enc_result(Rev_EncCompare_Helper &helper);
    void run_enc_comparison_owner_enc_result(EncCompare_Owner &owner);
    mpz_class run_enc_comparison_helper_enc_result(EncCompare_Helper &helper);

    size_t run_linear_enc_argmax(Linear_EncArgmax_Owner &owner, bool use_lsic);

    /* to build comparators */
    EncCompare_Owner create_enc_comparator_owner(size_t bit_size, bool use_lsic);
    EncCompare_Helper create_enc_comparator_helper(size_t bit_size, bool use_lsic);
    Rev_EncCompare_Owner create_rev_enc_comparator_owner(size_t bit_size, bool use_lsic);
    Rev_EncCompare_Helper create_rev_enc_comparator_helper(size_t bit_size, bool use_lsic);

    unsigned int n_threads() const { return n_threads_; }
    void set_n_threads(unsigned int n) { assert(n > 0); n_threads_ = n; }
protected:
    tcp::socket socket_;
    
    const Key_dependencies_descriptor key_deps_desc_;
    GM_priv *gm_;
    Paillier_priv_fast *paillier_;
    
    Paillier *server_paillier_;
    GM *server_gm_;
    
    FHEcontext *fhe_context_;
    FHEPubKey *server_fhe_pk_;
    FHESecKey *fhe_sk_;
    ZZX fhe_G_;

    gmp_randstate_t rand_state_;
    
    boost::asio::streambuf input_buf_;
    
    unsigned int n_threads_;

    /* statistical security */
    unsigned int lambda_;

};