#pragma once

#include <gmpxx.h>
#include <vector>
#include <boost/asio.hpp>


#include <crypto/paillier.hh>
#include <crypto/gm.hh>

using boost::asio::ip::tcp;

using namespace std;

class Client {
public:
    Client(boost::asio::io_service& io_service, gmp_randstate_t state, unsigned int nbits_gm, unsigned int lambda);

    void connect(boost::asio::io_service& io_service, const string& hostname);
    
    tcp::socket& socket() { return socket_; }
    
    const GM_priv& gm() { return gm_; };
    vector<mpz_class> gm_pk() const { return gm_.pubkey(); }
    vector<mpz_class> gm_sk() const { return {gm_.pubkey()[0],gm_.pubkey()[1],gm_.privkey()[0],gm_.privkey()[1]}; }

    bool has_paillier_pk() const { return (server_paillier_ != NULL); }
    bool has_gm_pk() const { return (server_gm_ != NULL); }
    void get_server_pk_gm();
    void get_server_pk_paillier();
    
    mpz_class run_lsic(const mpz_class &a, size_t l);
    mpz_class run_lsic_A(LSIC_A &lsic);
    void run_rev_enc_compare(const mpz_class &a, const mpz_class &b, size_t l);

    void test_rev_enc_compare(size_t l);

    void disconnect();
protected:
    tcp::socket socket_;
    
    GM_priv gm_;
    
//    bool has_paillier_pk_;
    Paillier *server_paillier_;
//    bool has_gm_pk_;
    GM *server_gm_;
    
    gmp_randstate_t rand_state_;
    
    boost::asio::streambuf input_buf_;

    /* statistical security */
    unsigned int lambda_;

};