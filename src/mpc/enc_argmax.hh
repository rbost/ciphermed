#pragma once

#include <mpc/rev_enc_comparison.hh>
#include <vector>
#include <map>
#include <cstddef>

using namespace std;

class EncArgmax_Owner {
public:
    EncArgmax_Owner(const vector<mpz_class> &a, const size_t &l, const vector<mpz_class> pk_p, const vector<mpz_class> &pk_gm, gmp_randstate_t state);
    ~EncArgmax_Owner();
    
    vector< vector<Rev_EncCompare_Owner*> >comparators() const { return comparators_; }
    
    void unpermuteResult(size_t argmax_perm);
    size_t output() const { assert(is_protocol_done_); return i_0_;}
    
    
protected:
    map<size_t,size_t> perm_;
    vector< vector<Rev_EncCompare_Owner*> >comparators_;
    
    size_t k_;
    bool is_protocol_done_;
    size_t i_0_;
};

class EncArgmax_Helper {
public:
    EncArgmax_Helper(const size_t &l, const size_t &k,const std::vector<mpz_class> &sk_p, const std::vector<mpz_class> &sk_gm, gmp_randstate_t state);
    ~EncArgmax_Helper();
    
    vector< vector<Rev_EncCompare_Helper*> >comparators() const { return comparators_; }
    
    bool canSort() const;
    void sort();
    size_t permuted_argmax() const;

protected:
    bool compare(size_t i, size_t j) const;
    
    vector< vector<Rev_EncCompare_Helper*> >comparators_;
    vector<size_t> order_permuted_;
    
    size_t k_;
    bool is_sorted_;
};

map<size_t,size_t> genRandomPermutation(const size_t &n);
void runProtocol(EncArgmax_Owner &owner, EncArgmax_Helper &helper, unsigned int lambda = 100);
