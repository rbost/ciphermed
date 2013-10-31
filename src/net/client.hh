#pragma once

#include <gmpxx.h>
#include <vector>
#include <boost/asio.hpp>

#include <FHE.h>

#include <crypto/paillier.hh>
#include <crypto/gm.hh>

using boost::asio::ip::tcp;

using namespace std;

class Client {
public:
    Client(boost::asio::io_service& io_service, gmp_randstate_t state, unsigned int nbits_gm, unsigned int lambda);
    ~Client();
    
    void connect(boost::asio::io_service& io_service, const string& hostname);
    
    tcp::socket& socket() { return socket_; }
    
    const GM_priv& gm() { return gm_; };
    vector<mpz_class> gm_pk() const { return gm_.pubkey(); }
    vector<mpz_class> gm_sk() const { return {gm_.pubkey()[0],gm_.pubkey()[1],gm_.privkey()[0],gm_.privkey()[1]}; }
    
    bool has_paillier_pk() const { return (server_paillier_ != NULL); }
    bool has_gm_pk() const { return (server_gm_ != NULL); }
    bool has_fhe_pk() const { return (server_fhe_pk_ != NULL); }
    void get_server_pk_gm();
    void get_server_pk_paillier();
    void get_server_pk_fhe();
    void answer_server_pk_request();

    mpz_class run_comparison_protocol_A(Comparison_protocol_A *comparator);
    mpz_class run_lsic_A(LSIC_A *lsic);
    mpz_class run_priv_compare_A(Compare_A *comparator);

    void run_comparison_protocol_B(Comparison_protocol_B *comparator);
    void run_lsic_B(LSIC_B *lsic);
    void run_priv_compare_B(Compare_B *comparator);

    mpz_class test_lsic(const mpz_class &a, size_t l);
    mpz_class test_compare(const mpz_class &b, size_t l);

    void run_enc_comparison(EncCompare_Owner &owner);

    void test_rev_enc_compare(size_t l);
    void run_rev_enc_compare(const mpz_class &a, const mpz_class &b, size_t l);
    void run_rev_enc_comparison(Rev_EncCompare_Owner &owner);

    void test_decrypt_gm(const mpz_class &c);
    void test_fhe();

    void disconnect();
protected:
    tcp::socket socket_;
    
    GM_priv gm_;
    
    Paillier *server_paillier_;
    GM *server_gm_;
    FHEcontext *fhe_context_;
    FHEPubKey *server_fhe_pk_;

    ZZX fhe_G_;

    gmp_randstate_t rand_state_;
    
    boost::asio::streambuf input_buf_;

    /* statistical security */
    unsigned int lambda_;

};