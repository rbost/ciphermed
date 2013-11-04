#pragma once

#include <map>

#include <mpc/rev_enc_comparison.hh>

#include <cstddef>
#include <functional>
#include <math/util_gmp_rand.h>

using namespace  std;

class Linear_EncArgmax_Owner {
public:
    Linear_EncArgmax_Owner(const vector<mpz_class> &a, const size_t &l, Paillier &p, gmp_randstate_t state, unsigned int lambda = 100);
//    ~Linear_EncArgmax_Owner();
    
    void unpermuteResult(size_t argmax_perm);
    size_t output() const { assert(is_protocol_done_); return i_0_;}

    Rev_EncCompare_Owner create_current_round_rev_enc_compare_owner(function<Comparison_protocol_A*()> comparator_creator);
    void next_round(mpz_class &randomized_enc_max, mpz_class &randomized_value);
    void update_enc_max(const mpz_class &new_enc_max, const mpz_class &x, const mpz_class &y);
    
    size_t elements_number() const { return a_.size(); }
protected:
    vector<mpz_class> a_;
    map<size_t,size_t> perm_; // the permutation used to hide the real order
    
    size_t k_; // number of elements
    size_t round_count_;

    unsigned int lambda_;
    size_t bit_length_;
    gmp_randstate_t randstate_;
    Paillier paillier_;

    /* intermediate value */
    mpz_class enc_max_;
    mpz_class rand_x_, rand_y_;
    
    /* final output */
    bool is_protocol_done_;
    size_t i_0_;

};


class Linear_EncArgmax_Helper {
public:
    Linear_EncArgmax_Helper(const size_t &l, const size_t &k,Paillier_priv_fast &pp);
    
//    ~Linear_EncArgmax_Helper();
    
    void update_argmax(bool comp, const mpz_class &old_enc_max, const mpz_class &v, size_t index, mpz_class &new_enc_max, mpz_class &x, mpz_class &y);
    Rev_EncCompare_Helper rev_enc_compare_helper(function<Comparison_protocol_B*()> comparator_creator);

    size_t permuted_argmax() const;
protected:
    size_t k_; // number of elements
    size_t round_count_;
    size_t bit_length_;

    size_t argmax_perm_;
    Paillier_priv_fast paillier_;
    
};

void runProtocol(Linear_EncArgmax_Owner &owner, Linear_EncArgmax_Helper &helper,function<Comparison_protocol_A*()> comparator_creator_A, function<Comparison_protocol_B*()> comparator_creator_B, gmp_randstate_t state, unsigned int lambda = 100);