#pragma once


#include <vector>
#include <crypto/paillier.hh>
#include <mpc/lsic.hh>


// We use the following naming convention:
// - the OWNER is the owner of the encrypted data
// - the HELPER is the party that helps the owner to compare its data and has the secret key of the cyphertext


class EncCompare_Owner {
public:
    EncCompare_Owner(const mpz_class &v_a, const mpz_class &v_b, const size_t &l, const std::vector<mpz_class> pk_p, const std::vector<mpz_class> &sk_gm, gmp_randstate_t state);
    EncCompare_Owner(const mpz_class &v_a, const mpz_class &v_b, const size_t &l, const std::vector<mpz_class> pk_p, gmp_randstate_t state, unsigned int key_size = 1024);
    
    mpz_class setup(unsigned int lambda);
    bool decryptResult(const mpz_class &c_t);

    Paillier paillier() const { return paillier_; }
    LSIC_B& lsic() { return lsic_; };
    mpz_class get_c_r_l() const { return c_r_l_; };
    
protected:
    mpz_class a_,b_;
    size_t bit_length_;
    Paillier paillier_;
    LSIC_B lsic_;
    gmp_randstate_t randstate_;
    
    /* intermediate values */
    mpz_class c_r_l_;

    /* cached values */
    mpz_class two_l_;
};

class EncCompare_Helper {
public:
    EncCompare_Helper(const size_t &l, const std::vector<mpz_class> sk_p, const std::vector<mpz_class> &pk_gm, gmp_randstate_t state);
    EncCompare_Helper(const size_t &l, const std::vector<mpz_class> &pk_gm, gmp_randstate_t state, unsigned int key_size = 1024);

    void setup(const mpz_class &c_z);
    mpz_class concludeProtocol(const mpz_class &c_r_l_);

    Paillier_priv paillier() const { return paillier_; }
    LSIC_A& lsic() { return lsic_; };
    
protected:
    size_t bit_length_;
    Paillier_priv paillier_;
    LSIC_A lsic_;
    gmp_randstate_t randstate_;
    
    /* intermediate values */
    mpz_class c_z_l_;
    
    /* cached values */
    mpz_class two_l_;
};