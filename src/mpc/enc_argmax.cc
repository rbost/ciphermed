#include <mpc/enc_argmax.hh>
#include <algorithm>
#include <thread>

EncArgmax_Owner::EncArgmax_Owner(const vector<mpz_class> &a, const size_t &l, const vector<mpz_class> pk_p, const vector<mpz_class> &pk_gm, gmp_randstate_t state)
: k_(a.size()),is_protocol_done_(false)
{    
    perm_ = genRandomPermutation(k_);
    
    comparators_ = vector< vector<Rev_EncCompare_Owner*> >(k_);
     
    for (size_t i = 0; i < k_; i++) {
        comparators_[i] = vector<Rev_EncCompare_Owner*>(i);
        for (size_t j = 0; j < i; j++) {
            size_t p_i = perm_[i], p_j = perm_[j];
            (comparators_[i])[j] = new Rev_EncCompare_Owner(a[p_i],a[p_j],l,pk_p,pk_gm,state);

        }
    }
}

EncArgmax_Owner::~EncArgmax_Owner()
{
    for (size_t i = 0; i < k_; i++) {
        for (size_t j = 0; j < i; j++) {
            delete comparators_[i][j];
        }
    }
}

void EncArgmax_Owner::unpermuteResult(size_t argmax_perm)
{
    map<size_t,size_t>::iterator it;
    it=perm_.find(argmax_perm);
    
    i_0_ = it->second;

    is_protocol_done_ = true;
}




EncArgmax_Helper::EncArgmax_Helper(const size_t &l, const size_t &k,const std::vector<mpz_class> &sk_p, const std::vector<mpz_class> &sk_gm, gmp_randstate_t state)
: k_(k)
{
    comparators_ = vector< vector<Rev_EncCompare_Helper*> >(k_);
    
    for (size_t i = 0; i < k_; i++) {
        comparators_[i] = vector<Rev_EncCompare_Helper*>(i);
        for (size_t j = 0; j < i; j++) {
            comparators_[i][j] = new Rev_EncCompare_Helper(l,sk_p,sk_gm,state);
        }
    }
 
}

EncArgmax_Helper::~EncArgmax_Helper()
{
    for (size_t i = 0; i < comparators_.size(); i++) {
        for (size_t j = 0; j < comparators_[i].size(); j++) {
            delete comparators_[i][j];
        }
    }
}

bool EncArgmax_Helper::canSort() const
{
    for (size_t i = 0; i < comparators_.size(); i++) {
        for (size_t j = 0; j < comparators_[i].size(); j++) {
            if (!comparators_[i][j]->protocol_done()) {
                return false;
            }
        }
    }
    
    return true;
}

bool EncArgmax_Helper::compare(size_t i, size_t j) const
{
//    true if a_p_i < a_p_j
    if(i == j)
    {
        return false;
    }
    
    if (i > j) {
        return comparators_[i][j]->output();
    }
    return comparators_[j][i]->output();
}

void EncArgmax_Helper::sort()
{
    assert(canSort());
    
    order_permuted_ = vector<size_t>(k_);
    
    for (size_t i = 0; i < k_; i++) {
        order_permuted_[i] = i;
    }
    // by the power of closures, SORT!
    std::sort(order_permuted_.begin(),order_permuted_.end(),[this](int i, int j){ return compare(i,j); });

    is_sorted_ = true;
}

size_t EncArgmax_Helper::permuted_argmax() const
{
    assert(is_sorted_);
    return order_permuted_[order_permuted_.size()-1];
}


map<size_t,size_t> genRandomPermutation(const size_t &n)
{
    map<size_t,size_t> perm;
    
    for (size_t i = 0; i < n; i++)
    {
        perm[i] = i;
    }
    
    for (size_t i = 0; i < n; i++)
    {
        unsigned long randomValue = rand()%(n - 1); // we should use a better PRG here!!
        swap(perm[i], perm[randomValue]);
    }
    return perm;
}

void runProtocol(EncArgmax_Owner &owner, EncArgmax_Helper &helper, unsigned int lambda)
{
    size_t k = owner.comparators().size();
    
    for (size_t i = 0; i < k; i++) {
        for (size_t j = 0; j < i; j++) {
            runProtocol(*(owner.comparators()[i][j]), *(helper.comparators()[i][j]), lambda);
        }
    }
    
    helper.sort();
    owner.unpermuteResult(helper.permuted_argmax());
}

void threadCall(const EncArgmax_Owner *owner, const EncArgmax_Helper *helper, unsigned int lambda, size_t i_begin, size_t i_end)
{
    for (size_t i = i_begin; i < i_end; i++) {
        for (size_t j = 0; j < i; j++) {
            runProtocol(*(owner->comparators()[i][j]), *(helper->comparators()[i][j]), lambda);
        }
    }

}

void runProtocol(EncArgmax_Owner &owner, EncArgmax_Helper &helper, unsigned int lambda, unsigned int num_threads)
{
    size_t k = owner.comparators().size();
    
    size_t m = (k-1)*(k-2)/2;
    m = ceilf( ((float)m)/num_threads);
    
    thread threads[num_threads];
    size_t t = 0, c_count = 0, i_begin = 0;
    for (size_t i = 0; i < k; i++) {
        c_count += i;
        if (c_count >= m) {
            threads[t] = thread(threadCall, &owner, &helper, lambda, i_begin,i+1);
            i_begin = i+1;
            c_count = 0;
            t++;
        }
    }
    if (c_count >0) {
        threads[t] = thread(threadCall, &owner, &helper, lambda, i_begin,k);
    }
    
    for (t = 0; t < num_threads; t++) {
        threads[t].join();
    }

    helper.sort();
    owner.unpermuteResult(helper.permuted_argmax());
}