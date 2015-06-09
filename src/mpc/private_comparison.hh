/*
 * Copyright 2013-2015 Raphael Bost
 *
 * This file is part of ciphermed.

 *  ciphermed is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 * 
 *  ciphermed is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 * 
 *  You should have received a copy of the GNU General Public License
 *  along with ciphermed.  If not, see <http://www.gnu.org/licenses/>. 2
 *
 */

#pragma once

#include <vector>
#include <gmpxx.h>
#include <crypto/paillier.hh>
#include <crypto/gm.hh>

#include <mpc/comparison_protocol.hh>

class Compare_A : public Comparison_protocol_A {
public:
    Compare_A(const mpz_class &x, const size_t &l, Paillier &paillier, GM &gm, gmp_randstate_t state);
    
    void set_value(const mpz_class &x) { a_ = x; };

    std::vector<mpz_class> compute(const std::vector<mpz_class> &c_b, unsigned int n_threads = 4);
    
    std::vector<mpz_class> compute_w(const std::vector<mpz_class> &c_b);
    std::vector<mpz_class> compute_sums(const std::vector<mpz_class> &c_w);
    
    std::vector<mpz_class> compute_c(const std::vector<mpz_class> &c_a,const std::vector<mpz_class> &c_sums, std::vector<size_t> &rerand_indexes);
    
    std::vector<mpz_class> rerandomize(const std::vector<mpz_class> &c, const std::vector<size_t> &rerand_indexes);
    std::vector<mpz_class> rerandomize_parallel(const std::vector<mpz_class> &c, const std::vector<size_t> &rerand_indexes, unsigned int n_threads = 4);
    
    void shuffle(std::vector<mpz_class> &c);

    void unblind(const mpz_class &t_prime);
    
    GM gm() const { return gm_; }
    size_t bit_length() const { return bit_length_; }
    virtual void set_bit_length(size_t l) {bit_length_ = l;}

    mpz_class output() const { return res_; }
    
protected:
    mpz_class a_;
    long s_;
    size_t bit_length_; // bit length of the numbers to compare
    Paillier paillier_;
    GM gm_;
    
    mpz_class res_;
    mpz_class paillier_one_;
    
    gmp_randstate_t randstate_;
};

class Compare_B : public Comparison_protocol_B {
public:
    Compare_B(const mpz_class &y, const size_t &l, Paillier_priv_fast &paillier, GM_priv &gm);
    
    virtual void set_value(const mpz_class &x) { b_ = x; };

    std::vector<mpz_class> encrypt_bits();
    std::vector<mpz_class> encrypt_bits_parallel(unsigned int n_threads = 4);
    
    mpz_class search_zero(const std::vector<mpz_class> &c);
    
    GM_priv gm() const { return gm_; };
    size_t bit_length() const { return bit_length_; }
    virtual void set_bit_length(size_t l) {bit_length_ = l;}

protected:
    mpz_class b_;
    size_t bit_length_; // bit length of the numbers to compare
    Paillier_priv_fast paillier_;
    GM_priv gm_;
};

void threadCall(Paillier &paillier, std::vector<mpz_class> &c_rand, std::vector<size_t> &rerand_indexes, size_t i_start, size_t i_end);


void runProtocol(Compare_A &party_a, Compare_B &party_b, gmp_randstate_t state);
inline void runProtocol(Compare_A *party_a, Compare_B *party_b, gmp_randstate_t state)
{
    runProtocol(*party_a,*party_b, state);
}