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
    ~Server();
    
    
    void run();
    
    const Paillier_priv& paillier() { return paillier_; };
    vector<mpz_class> paillier_pk() const { return paillier_.pubkey(); }
    vector<mpz_class> paillier_sk() const { return paillier_.privkey(); }
    const GM_priv& gm() { return gm_; };
    vector<mpz_class> gm_pk() const { return gm_.pubkey(); }
    vector<mpz_class> gm_sk() const { return {gm_.pubkey()[0],gm_.pubkey()[1],gm_.privkey()[0],gm_.privkey()[1]}; }
    
    const FHESecKey& fhe_sk() const { return *fhe_sk_; } // I don't want anyone to modify the secret key
    const ZZX& fhe_G() const { return fhe_G_; }
    
protected:
    Paillier_priv paillier_;
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
    
    void run_session();
    
    void send_paillier_pk();
    void send_gm_pk();
    void send_fhe_pk();
    
    void run_lsic(const mpz_class &b,size_t l);
    void run_lsic_B(LSIC_B &lsic);
    
    bool run_rev_enc_comparison(const size_t &l, const std::vector<mpz_class> sk_p, const std::vector<mpz_class> &sk_gm);
    bool run_rev_enc_comparison(Rev_EncCompare_Helper &helper);

    void decrypt_gm(const mpz_class &c);
    void decrypt_fhe(const Ctxt &c);

    unsigned int id() const {return id_;}
    
protected:
    Server *server_;
    tcp::socket *socket_;
    boost::asio::streambuf input_buf_;

    
    std::vector<mpz_class> gm_pk_;
    gmp_randstate_t rand_state_;
    
    unsigned int id_;
};