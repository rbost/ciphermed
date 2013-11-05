#pragma once

#include <gmpxx.h>
#include <vector>
#include <boost/asio.hpp>

#include <FHE.h>

#include <crypto/paillier.hh>
#include <crypto/gm.hh>

using boost::asio::ip::tcp;

using namespace std;

class Server_session;

class Server {
public:  
    Server(gmp_randstate_t state, unsigned int nbits_p, unsigned int abits_p, unsigned int nbits_gm, unsigned int lambda);
    virtual ~Server();
    
    virtual Server_session* create_new_server_session(tcp::socket *socket);
    void run();
    
    Paillier_priv_fast& paillier() { return paillier_; };
    vector<mpz_class> paillier_pk() const { return paillier_.pubkey(); }
    vector<mpz_class> paillier_sk() const { return paillier_.privkey(); }
    GM_priv& gm() { return gm_; };
    vector<mpz_class> gm_pk() const { return gm_.pubkey(); }
    vector<mpz_class> gm_sk() const { return {gm_.pubkey()[0],gm_.pubkey()[1],gm_.privkey()[0],gm_.privkey()[1]}; }
    
    const FHESecKey& fhe_sk() const { return *fhe_sk_; } // I don't want anyone to modify the secret key
    const ZZX& fhe_G() const { return fhe_G_; }
    
protected:
    Paillier_priv_fast paillier_;
    GM_priv gm_;
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
    Server_session(Server *server, gmp_randstate_t state, unsigned int id, tcp::socket *socket);
    virtual ~Server_session();
    
    virtual void run_session();
    
    void send_paillier_pk();
    void send_gm_pk();
    void send_fhe_pk();
    void get_client_pk_gm();
    void get_client_pk_paillier();
    void exchange_all_keys();

    mpz_class run_comparison_protocol_A(Comparison_protocol_A *comparator);
    mpz_class run_lsic_A(LSIC_A *lsic);
    mpz_class run_priv_compare_A(Compare_A *comparator);

    void run_comparison_protocol_B(Comparison_protocol_B *comparator);
    void run_lsic_B(LSIC_B *lsic);
    void run_priv_compare_B(Compare_B *comparator);

    void run_enc_comparison(const size_t &l, GM *gm);
    void run_enc_comparison(EncCompare_Helper &helper);
    
    bool run_rev_enc_comparison(const size_t &l);
    bool run_rev_enc_comparison(Rev_EncCompare_Helper &helper);

    void run_linear_enc_argmax(Linear_EncArgmax_Helper &helper);

    
    EncCompare_Helper create_enc_comparator_helper(size_t bit_size, bool use_lsic);
    Rev_EncCompare_Helper create_rev_enc_comparator_helper(size_t bit_size, bool use_lsic);

    void decrypt_gm(const mpz_class &c);
    void decrypt_fhe(const Ctxt &c);

    unsigned int id() const {return id_;}
    
    
    /* Test functions */
    void test_lsic(const mpz_class &b,size_t l);
    void test_compare(const mpz_class &a,size_t l);

protected:
    Server *server_;
    tcp::socket *socket_;
    boost::asio::streambuf input_buf_;

    GM *client_gm_;
    Paillier *client_paillier_;
    gmp_randstate_t rand_state_;
    
    unsigned int id_;
};