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

#include <map>

#include <mpc/rev_enc_comparison.hh>

#include <cstddef>
#include <functional>
#include <math/util_gmp_rand.h>

using namespace  std;

class Tree_EncArgmax_Owner {
public:
    Tree_EncArgmax_Owner(const vector<mpz_class> &a, const size_t &l, Paillier &p, gmp_randstate_t state, unsigned int lambda = 100);
    
    void unpermuteResult(size_t argmax_perm);
    size_t output() const { assert(is_protocol_done_); return i_0_;}

    vector<Rev_EncCompare_Owner*> create_current_round_rev_enc_compare_owners(function<Comparison_protocol_A*()> comparator_creator);
    vector<mpz_class> next_round();
    void update_local_max(const vector<mpz_class> &rand_local_max, const vector<mpz_class> &x, const vector<mpz_class> &y);
    
    size_t bit_length() const { return bit_length_; }
    size_t elements_number() const { return k_; }
    
    bool new_round_needed() const { return local_max_.size() != 1; };
protected:
    vector<mpz_class> a_;
    map<size_t,size_t> perm_; // the permutation used to hide the real order
    
    size_t k_; // number of elements
    vector<mpz_class> local_max_;
    size_t round_count_;

    unsigned int lambda_;
    size_t bit_length_;
    gmp_randstate_t randstate_;
    Paillier paillier_;

    /* intermediate value */
    vector<mpz_class> noise_;
    
    /* final output */
    bool is_protocol_done_;
    size_t i_0_;

};


class Tree_EncArgmax_Helper {
public:
    Tree_EncArgmax_Helper(const size_t &l, const size_t &k,Paillier_priv_fast &pp);
        
    void update_argmax(vector<bool> comp, const vector<mpz_class> &old_enc_max, vector<mpz_class> &new_enc_max, vector<mpz_class> &x, vector<mpz_class> &y);
    
    vector<Rev_EncCompare_Helper*> create_current_round_rev_enc_compare_helpers(function<Comparison_protocol_B*()> comparator_creator);

    size_t permuted_argmax() const;
    size_t elements_number() const { return k_; }
    size_t bit_length() const { return bit_length_; }
    
    bool new_round_needed() const { return local_argmax_.size() != 1; };

protected:
    size_t k_; // number of elements
    vector<size_t> local_argmax_;
    size_t round_count_;
    size_t bit_length_;

    size_t argmax_perm_;
    Paillier_priv_fast paillier_;
    
};

void runProtocol(Tree_EncArgmax_Owner &owner, Tree_EncArgmax_Helper &helper,function<Comparison_protocol_A*()> comparator_creator_A, function<Comparison_protocol_B*()> comparator_creator_B, gmp_randstate_t state, unsigned int lambda = 100);