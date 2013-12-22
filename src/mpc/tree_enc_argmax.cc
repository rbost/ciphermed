#include <mpc/tree_enc_argmax.hh>
#include <mpc/enc_argmax.hh>
#include <algorithm>


Tree_EncArgmax_Owner::Tree_EncArgmax_Owner(const vector<mpz_class> &a, const size_t &l, Paillier &p, gmp_randstate_t state, unsigned int lambda)
: a_(a), k_(a.size()), local_max_(k_), round_count_(0), lambda_(lambda), bit_length_(l), paillier_(p), is_protocol_done_(false)
{
    assert(k_ > 0);
    gmp_randinit_set(randstate_, state);
    
    for (size_t i = 0; i < k_; i++) {
        perm_[i] = i;
    }

    perm_ = genRandomPermutation(k_,randstate_);

    for (size_t i = 0; i < k_; i++) {
        local_max_[i] = a_[perm_[i]];
    }
}

vector<Rev_EncCompare_Owner*> Tree_EncArgmax_Owner::create_current_round_rev_enc_compare_owners(function<Comparison_protocol_A*()> comparator_creator)
{
    size_t n = local_max_.size()/2;
    vector<Rev_EncCompare_Owner*> owners(n);
    
    for (size_t i = 0; i < n; i++) {
        owners[i] = new  Rev_EncCompare_Owner(local_max_[2*i], local_max_[2*i+1],bit_length_,paillier_,comparator_creator(),randstate_);
    }
    
    return owners;
}

vector<mpz_class> Tree_EncArgmax_Owner::next_round()
{
    assert(local_max_.size() > 1);

    size_t n = local_max_.size();
    // we don't care about the last value if n is odd
    n = n - (n%2);
    vector<mpz_class> randomized_values(n);
    noise_ = vector<mpz_class>(n);
    
    for (size_t i = 0; i<n; i++) {
        mpz_urandomb(noise_[i].get_mpz_t(), randstate_, lambda_+bit_length_);
        randomized_values[i] = paillier_.add(local_max_[i],paillier_.encrypt(noise_[i]));
    }
    
    return randomized_values;
}


void Tree_EncArgmax_Owner::update_local_max(const vector<mpz_class> &rand_local_max, const vector<mpz_class> &x, const vector<mpz_class> &y)
{
    size_t n = local_max_.size();
    size_t new_size = n/2 + n%2;
    vector<mpz_class> new_local_max(new_size); // size of vector is ceil(n/2)
    
    for (size_t i = 0; i < n/2; i++) {
        new_local_max[i] = rand_local_max[i];
        mpz_class r_x = paillier_.constMult(noise_[2*i],x[i]);
        mpz_class r_y = paillier_.constMult(noise_[2*i+1],y[i]);
        
        new_local_max[i] = paillier_.sub(new_local_max[i],r_x);
        new_local_max[i] = paillier_.sub(new_local_max[i],r_y);
    }
    
    if (n%2 == 1) {
        new_local_max[new_size-1] = local_max_[local_max_.size()-1];
    }
    
    local_max_ = new_local_max;
}

void Tree_EncArgmax_Owner::unpermuteResult(size_t argmax_perm)
{
    map<size_t,size_t>::iterator it;
    it=perm_.find(argmax_perm);
    
    i_0_ = it->second;
    
    is_protocol_done_ = true;
}

Tree_EncArgmax_Helper::Tree_EncArgmax_Helper(const size_t &l, const size_t &k,Paillier_priv_fast &pp)
: k_(k), local_argmax_(k), round_count_(0), bit_length_(l), argmax_perm_(0), paillier_(pp)
{
    assert(k_ > 0);
    
    for (size_t i = 0; i < k_; i++) {
        local_argmax_[i] = i;
    }
}

size_t Tree_EncArgmax_Helper::permuted_argmax() const
{
    assert(local_argmax_.size() == 1);
    return local_argmax_[0];
}

void Tree_EncArgmax_Helper::update_argmax(vector<bool> comp, const vector<mpz_class> &old_enc_max, vector<mpz_class> &new_enc_max, vector<mpz_class> &x, vector<mpz_class> &y)
{
    size_t n = comp.size();
    new_enc_max = vector<mpz_class>(n);
    x = vector<mpz_class>(n);
    y = vector<mpz_class>(n);
    
    vector<size_t> new_local_argmax(n);
    
    for (size_t i = 0; i<n; i++) {
        
        // we need to generate fresh encryptions of 0 and 1 to ensure security
        mpz_class zero = paillier_.encrypt(0);
        mpz_class one = paillier_.encrypt(1);

        if (comp[i]) {
            new_enc_max[i] = old_enc_max[2*i+1];
            new_local_argmax[i] = local_argmax_[2*i+1];
            x[i] = zero;
            y[i] = one;
        }else{
            new_enc_max[i] = old_enc_max[2*i];
            new_local_argmax[i] = local_argmax_[2*i];
            x[i] = one;
            y[i] = zero;
        }
        
        paillier_.refresh(new_enc_max[i]);
    }
    
    if (local_argmax_.size()%2 == 1) {
        new_local_argmax.push_back(local_argmax_[local_argmax_.size()-1]);
    }
    
    local_argmax_ = new_local_argmax;
}

vector<Rev_EncCompare_Helper*> Tree_EncArgmax_Helper::create_current_round_rev_enc_compare_helpers(function<Comparison_protocol_B*()> comparator_creator)
{
    size_t n = local_argmax_.size()/2;
    vector<Rev_EncCompare_Helper*> helpers(n);
    
    for (size_t i = 0; i < n; i++) {
        helpers[i] = new  Rev_EncCompare_Helper(bit_length_,paillier_,comparator_creator());
    }
    
    return helpers;
}

void runProtocol(Tree_EncArgmax_Owner &owner, Tree_EncArgmax_Helper &helper,function<Comparison_protocol_A*()> comparator_creator_A, function<Comparison_protocol_B*()> comparator_creator_B, gmp_randstate_t state, unsigned int lambda)
{
    size_t k = owner.elements_number();
    
    while (owner.new_round_needed()) {
        
        vector<Rev_EncCompare_Owner*> rev_enc_owners = owner.create_current_round_rev_enc_compare_owners(comparator_creator_A);
        
        vector<Rev_EncCompare_Helper*> rev_enc_helpers = helper.create_current_round_rev_enc_compare_helpers(comparator_creator_B);

        vector<bool> results (rev_enc_owners.size());
        for (size_t i = 0; i < rev_enc_owners.size(); i++) {
            runProtocol(*rev_enc_owners[i],*rev_enc_helpers[i],state, lambda);
            results[i] = rev_enc_helpers[i]->output();
            delete rev_enc_owners[i];
            delete rev_enc_helpers[i];
        }
        
        vector<mpz_class> randomized_enc_max = owner.next_round();
        
        vector<mpz_class> new_enc_max, x, y;
        
        
        helper.update_argmax(results, randomized_enc_max, new_enc_max, x, y);
        
        owner.update_local_max(new_enc_max, x, y);
    }
    
    owner.unpermuteResult(helper.permuted_argmax());
}





