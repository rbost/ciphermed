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
//    EncArgmax_Owner(const vector<mpz_class> &a, const size_t &l, Paillier &p, Comparison_protocol_A* comparator, gmp_randstate_t state);
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
//    EncArgmax_Helper(const size_t &l, const size_t &k,Paillier_priv &pp, Comparison_protocol_B* comparator, gmp_randstate_t state);
    EncArgmax_Helper(const size_t &l, const size_t &k,Paillier_priv &pp, function<Comparison_protocol_B*()> comparator_creator);

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

// generate a random permutation using rand() <-- BAD !!
map<size_t,size_t> genRandomPermutation(const size_t &n);

void runProtocol(EncArgmax_Owner &owner, EncArgmax_Helper &helper, gmp_randstate_t state, unsigned int lambda = 100);

// Parallelization
void threadCall(const EncArgmax_Owner *owner, const EncArgmax_Helper *helper, gmp_randstate_t state, unsigned int lambda, size_t i_begin, size_t i_end);
void runProtocol(EncArgmax_Owner &owner, EncArgmax_Helper &helper, gmp_randstate_t state, unsigned int lambda, unsigned int num_threads);
