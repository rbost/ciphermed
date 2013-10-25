#pragma once

#include <vector>
#include <gmpxx.h>
#include <crypto/paillier.hh>
#include <crypto/gm.hh>

class Compare_A {
public:
    Compare_A(const mpz_class &x, const size_t &l, Paillier_priv &paillier, GM_priv &gm);
    
    std::vector<mpz_class> encrypt_bits();
    std::vector<mpz_class> encrypt_bits_fast();
    std::vector<mpz_class> encrypt_bits_fast_precompute();
    mpz_class search_zero(const std::vector<mpz_class> &c);
    
protected:
    mpz_class a_;
    size_t bit_length_; // bit length of the numbers to compare
    Paillier_priv paillier_;
    GM_priv gm_;
};

class Compare_B {
public:
    Compare_B(const mpz_class &y, const size_t &l, Paillier &paillier, GM &gm, gmp_randstate_t state);

    std::vector<mpz_class> compute_w(const std::vector<mpz_class> &c_a);
    std::vector<mpz_class> compute_sums(const std::vector<mpz_class> &c_w);
    std::vector<mpz_class> compute_c(const std::vector<mpz_class> &c_a,const std::vector<mpz_class> &c_sums);
    std::vector<mpz_class> rerandomize(const std::vector<mpz_class> &c);
    mpz_class unblind(const mpz_class &t_prime);
    
protected:
    mpz_class b_;
    long s_;
    size_t bit_length_; // bit length of the numbers to compare
    Paillier paillier_;
    GM gm_;

    mpz_class paillier_one_;
};

mpz_class runProtocol(Compare_A &party_a, Compare_B &party_b, gmp_randstate_t state);