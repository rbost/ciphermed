#include <mpc/linear_enc_argmax.hh>
#include <mpc/enc_argmax.hh>
#include <algorithm>


Linear_EncArgmax_Owner::Linear_EncArgmax_Owner(const vector<mpz_class> &a, const size_t &l, Paillier &p, gmp_randstate_t state, unsigned int lambda)
: a_(a), k_(a.size()), round_count_(0), lambda_(lambda), bit_length_(l), paillier_(p), is_protocol_done_(false)
{
    assert(k_ > 0);
    gmp_randinit_set(randstate_, state);
    
    for (size_t i = 0; i < k_; i++) {
        perm_[i] = k_ -1 - i;
    }

    perm_ = genRandomPermutation(k_,randstate_);
    
    enc_max_ = a_[perm_[0]];
}

Rev_EncCompare_Owner Linear_EncArgmax_Owner::create_current_round_rev_enc_compare_owner(function<Comparison_protocol_A*()> comparator_creator)
{
    return Rev_EncCompare_Owner(enc_max_, a_[perm_[round_count_+1]],bit_length_,paillier_,comparator_creator(),randstate_);
}

void Linear_EncArgmax_Owner::next_round(mpz_class &randomized_enc_max, mpz_class &randomized_value)
{
    assert(round_count_ < (k_-1));

    mpz_urandomb(rand_x_.get_mpz_t(), randstate_, lambda_+bit_length_);
    mpz_urandomb(rand_y_.get_mpz_t(), randstate_, lambda_+bit_length_);

    randomized_enc_max = paillier_.add(enc_max_,paillier_.encrypt(rand_x_));
    randomized_value = paillier_.add(a_[perm_[round_count_+1]],paillier_.encrypt(rand_y_));
    
    round_count_++;
}


void Linear_EncArgmax_Owner::update_enc_max(const mpz_class &new_enc_max, const mpz_class &x, const mpz_class &y)
{
    enc_max_ = new_enc_max;
    mpz_class r_x = paillier_.constMult(rand_x_,x);
    mpz_class r_y = paillier_.constMult(rand_y_,y);
    
    enc_max_ = paillier_.sub(enc_max_,r_x);
    enc_max_ = paillier_.sub(enc_max_,r_y);
}

void Linear_EncArgmax_Owner::unpermuteResult(size_t argmax_perm)
{
    map<size_t,size_t>::iterator it;
    it=perm_.find(argmax_perm);
    
    i_0_ = it->second;
    
    is_protocol_done_ = true;
}

Linear_EncArgmax_Helper::Linear_EncArgmax_Helper(const size_t &l, const size_t &k,Paillier_priv_fast &pp)
: k_(k), round_count_(0), bit_length_(l), argmax_perm_(0), paillier_(pp)
{
    assert(k_ > 0);
        
}

size_t Linear_EncArgmax_Helper::permuted_argmax() const
{
    assert(round_count_ < (k_-1));
    return argmax_perm_;
}

void Linear_EncArgmax_Helper::update_argmax(bool comp, const mpz_class &old_enc_max, const mpz_class &v, size_t index, mpz_class &new_enc_max, mpz_class &x, mpz_class &y)
{
    mpz_class zero = paillier_.encrypt(0);
    mpz_class one = paillier_.encrypt(1);
    
    if (comp) {
        new_enc_max = v;
        argmax_perm_ = index;
        x = zero;
        y = one;
    }else{
        new_enc_max = old_enc_max;
        x = one;
        y = zero;
    }
    
    paillier_.refresh(new_enc_max);
}

Rev_EncCompare_Helper Linear_EncArgmax_Helper::rev_enc_compare_helper(function<Comparison_protocol_B*()> comparator_creator)
{
    return Rev_EncCompare_Helper(bit_length_,paillier_,comparator_creator());
}

void runProtocol(Linear_EncArgmax_Owner &owner, Linear_EncArgmax_Helper &helper,function<Comparison_protocol_A*()> comparator_creator_A, function<Comparison_protocol_B*()> comparator_creator_B, gmp_randstate_t state, unsigned int lambda)
{
    size_t k = owner.elements_number();
    
    for (size_t i = 0; i < k - 1; i++) {
        Rev_EncCompare_Owner rev_enc_owner = owner.create_current_round_rev_enc_compare_owner(comparator_creator_A);
        
        Rev_EncCompare_Helper rev_enc_helper = helper.rev_enc_compare_helper(comparator_creator_B);
        
        runProtocol(rev_enc_owner,rev_enc_helper,state, lambda);
        
        mpz_class randomized_enc_max, randomized_value;
        owner.next_round(randomized_enc_max, randomized_value);
        
        mpz_class new_enc_max, x, y;
        helper.update_argmax(rev_enc_helper.output(), randomized_enc_max, randomized_value, i+1, new_enc_max, x, y);
        
        owner.update_enc_max(new_enc_max, x, y);
    }
    
    owner.unpermuteResult(helper.permuted_argmax());
}





