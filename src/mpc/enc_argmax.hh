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

#include <mpc/rev_enc_comparison.hh>
#include <vector>
#include <map>
#include <cstddef>
#include <functional>

using namespace std;

// We use the following naming convention:
// - the OWNER is the owner of the encrypted data
// - the HELPER is the party that helps the owner to compare its data and has the secret key of the cyphertext

class EncArgmax_Owner {
public:
    EncArgmax_Owner(const vector<mpz_class> &a, const size_t &l, Paillier &p, function<Comparison_protocol_A*()> comparator_creator, gmp_randstate_t state);
    ~EncArgmax_Owner();
    
    vector< vector<Rev_EncCompare_Owner*> >comparators() const { return comparators_; }
    
    void unpermuteResult(size_t argmax_perm);
    size_t output() const { assert(is_protocol_done_); return i_0_;}
    
    
protected:
    map<size_t,size_t> perm_; // the permutation used to hide the real order
    vector< vector<Rev_EncCompare_Owner*> >comparators_;
    
    size_t k_; // number of elements
    
    /* final output */
    bool is_protocol_done_;
    size_t i_0_;
};

class EncArgmax_Helper {
public:
    EncArgmax_Helper(const size_t &l, const size_t &k,Paillier_priv_fast &pp, function<Comparison_protocol_B*()> comparator_creator);

    ~EncArgmax_Helper();
    
    vector< vector<Rev_EncCompare_Helper*> >comparators() const { return comparators_; }
    
    bool canSort() const;
    void sort();
    size_t permuted_argmax() const; // returns the argmax using the permuted indices

protected:
    bool compare(size_t i, size_t j) const;
    
    vector< vector<Rev_EncCompare_Helper*> >comparators_;
    vector<size_t> order_permuted_; // order of the elements using permuted indices
    
    size_t k_; // number of elements
    bool is_sorted_;
};

map<size_t,size_t> genRandomPermutation(const size_t &n, gmp_randstate_t state);

void runProtocol(EncArgmax_Owner &owner, EncArgmax_Helper &helper, gmp_randstate_t state, unsigned int lambda = 100);

// Parallelization
void threadCall(const EncArgmax_Owner *owner, const EncArgmax_Helper *helper, gmp_randstate_t state, unsigned int lambda, size_t i_begin, size_t i_end);
void runProtocol(EncArgmax_Owner &owner, EncArgmax_Helper &helper, gmp_randstate_t state, unsigned int lambda, unsigned int num_threads);
