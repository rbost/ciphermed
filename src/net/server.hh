#pragma once

#include <gmpxx.h>
#include <vector>
#include <boost/asio.hpp>


#include <crypto/paillier.hh>
#include <crypto/gm.hh>

using boost::asio::ip::tcp;

using namespace std;

class Server_session;

class Server {
public:  
    Server(gmp_randstate_t state, unsigned int nbits_p, unsigned int abits_p, unsigned int nbits_gm, unsigned int lambda);
    void run();
    
    const Paillier_priv& paillier() { return paillier_; };
    vector<mpz_class> paillier_pk() const { return paillier_.pubkey(); }
    vector<mpz_class> paillier_sk() const { return paillier_.privkey(); }
    const GM_priv& gm() { return gm_; };
    vector<mpz_class> gm_pk() const { return gm_.pubkey(); }
    vector<mpz_class> gm_sk() const { return {gm_.pubkey()[0],gm_.pubkey()[1],gm_.privkey()[0],gm_.privkey()[1]}; }
    
protected:
    Paillier_priv paillier_;
    GM_priv gm_;
    
    gmp_randstate_t rand_state_;
    
//    vector<Server_session> clients_;
    
    /* statistical security */
    unsigned int lambda_;
    unsigned int n_clients_;
};

class Server_session {
public:
    Server_session(Server *server, gmp_randstate_t state, unsigned int id, tcp::socket *socket);
    
    void run_session();
    void send_paillier_pk();
    void send_gm_pk();
    void run_lsic(const mpz_class &b,size_t l);
    void decrypt_gm(const mpz_class &c);
    
    unsigned int id() const {return id_;}
    
protected:
    Server *server_;
    tcp::socket *socket_;
    boost::asio::streambuf input_buf_;

    
    std::vector<mpz_class> gm_pk_;
    gmp_randstate_t rand_state_;
    
    unsigned int id_;
};