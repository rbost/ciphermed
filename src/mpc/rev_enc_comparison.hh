#pragma once


#include <vector>
#include <crypto/paillier.hh>
#include <mpc/lsic.hh>


// We use the following naming convention:
// - the OWNER is the owner of the encrypted data
// - the HELPER is the party that helps the owner to compare its data and has the secret key of the cyphertext

// - variables prefixed by c_ are GM cyphertexts
// - variables prefixed by pk_ (resp. sk_) are public (resp. secret) keys: sk_p secret key for Paillier, pk_gm public key for GM

class Rev_EncCompare_Owner {
public:
//    Rev_EncCompare_Owner(const mpz_class &v_a, const mpz_class &v_b, const size_t &l, const std::vector<mpz_class> pk_p, const std::vector<mpz_class> &pk_gm, gmp_randstate_t state);
    Rev_EncCompare_Owner(const mpz_class &v_a, const mpz_class &v_b, const size_t &l, Paillier &p,Comparison_protocol_A* comparator, gmp_randstate_t state);
    
    mpz_class setup(unsigned int lambda); // lambda is the parameter for statistical security. r <- [0, 2^{l+lambda}[ \cap \Z
    mpz_class concludeProtocol(const mpz_class &c_r_l_);

    Paillier paillier() const { return paillier_; }
    Comparison_protocol_A* comparator() { return comparator_; };
    mpz_class get_c_r_l() const { return c_r_l_; };
    
protected:
    mpz_class a_,b_;
    size_t bit_length_;
    Paillier paillier_;
    Comparison_protocol_A *comparator_;
    gmp_randstate_t randstate_;
    
    /* intermediate values */
    mpz_class c_r_l_; // encryption of the l-th bit of r

    /* cached values */
    mpz_class two_l_; // 2^l = 1 << bit_length_
};

class Rev_EncCompare_Helper {
public:
    Rev_EncCompare_Helper(const size_t &l, Paillier_priv &pp, Comparison_protocol_B *comparator);

    void setup(const mpz_class &c_z);
    void decryptResult(const mpz_class &c_t);
    inline bool protocol_done() { return is_protocol_done_; }
    inline bool output() const { assert(is_protocol_done_);  return t_; }

    Paillier_priv paillier() const { return paillier_; }
    Comparison_protocol_B* comparator() { return comparator_; };
    mpz_class get_c_z_l() const { return c_z_l_; };

protected:
    size_t bit_length_;
    Paillier_priv paillier_;
    Comparison_protocol_B *comparator_;
    
    /* intermediate values */
    mpz_class c_z_l_; // encryption of the l-th bit of r
    
    /* cached values */
    mpz_class two_l_; // 2^l = 1 << bit_length_
    
    /* final output */
    bool is_protocol_done_;
    bool t_;
};

void runProtocol(Rev_EncCompare_Owner &owner, Rev_EncCompare_Helper &helper, gmp_randstate_t state, unsigned int lambda = 100);