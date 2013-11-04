#include <mpc/private_comparison.hh>

#include <util/threadpool.hh>
#include <util/util.hh>

using namespace std;

Compare_B::Compare_B(const mpz_class &y, const size_t &l, Paillier_priv_fast &paillier, GM_priv &gm)
: b_(y), bit_length_(l), paillier_(paillier), gm_(gm)
{
    
}

vector<mpz_class> Compare_B::encrypt_bits()
{
    ScopedTimer timer("encrypt_bits");
    
    vector<mpz_class> c_b(bit_length_);
    
    for (size_t i = 0; i < bit_length_; i++) {
        c_b[i] = paillier_.encrypt(mpz_tstbit(b_.get_mpz_t(),i));
    }
    
    return c_b;
}

vector<mpz_class> Compare_B::encrypt_bits_parallel(unsigned int n_threads)
{
    ScopedTimer timer("encrypt_bits_parallel");
    ThreadPool pool(n_threads);

    vector<mpz_class> c_b(bit_length_);
    
    for (size_t i = 0; i < bit_length_; i++) {
        c_b[i] = paillier_.encrypt(mpz_tstbit(b_.get_mpz_t(),i));
    }
    
    Paillier_priv_fast *paillier_ptr = &paillier_;
    vector<mpz_class> *c_b_ptr = &c_b;
    
    for (size_t i = 0; i < bit_length_; i++) {
        auto job =[this,paillier_ptr,i,c_b_ptr]
        {
            (*c_b_ptr)[i] = paillier_ptr->encrypt(mpz_tstbit(b_.get_mpz_t(),i));
        };
        pool.enqueue(job);
    }

    return c_b;
}

mpz_class Compare_B::search_zero(const vector<mpz_class> &c)
{
    ScopedTimer timer("search_zero");
    // can be parallelized
    for (size_t i = 0; i < c.size(); i++) {
        if (paillier_.decrypt(c[i]) == 0) {
//            cout << "Has zero" << endl;
            return gm_.encrypt(true);
        }
    }
  
    return gm_.encrypt(false);
}

Compare_A::Compare_A(const mpz_class &x, const size_t &l, Paillier &paillier, GM &gm, gmp_randstate_t state)
: a_(x), bit_length_(l), paillier_(paillier), gm_(gm)
{
    s_ = 1 - 2*gmp_urandomb_ui(state,1);
    paillier_one_ = paillier_.pubkey()[1]; // g is an encryption of 1
}

vector<mpz_class> Compare_A::compute_w(const std::vector<mpz_class> &c_b)
{
    ScopedTimer timer("compute_w");

    vector<mpz_class> c_w(bit_length_);
    // can be parallelized
    for (size_t i = 0; i < bit_length_; i++) {
        if (mpz_tstbit(a_.get_mpz_t(),i) == 0) {
            c_w[i] = c_b[i];
        }else{
            c_w[i] = paillier_.sub(paillier_one_,c_b[i]);
        }
    }

    return c_w;
}

vector<mpz_class> Compare_A::compute_sums(const std::vector<mpz_class> &c_w)
{
    ScopedTimer timer("compute_sums");

    vector<mpz_class> c_sums(bit_length_);
    c_sums[bit_length_-1] = 1;

    for (size_t i = bit_length_-1; i > 0; i--) {
        c_sums[i-1] = paillier_.add(c_sums[i],c_w[i]);
    }
    
    return c_sums;
}

vector<mpz_class> Compare_A::compute_c(const std::vector<mpz_class> &c_b,const std::vector<mpz_class> &c_sums)
{
    ScopedTimer timer("compute_c");
    
    vector<mpz_class> c(bit_length_);
    
    for (size_t i = 0; i < bit_length_; i++) {
        c[i] = paillier_.constMult(3,c_sums[i]);
        
        c[i] = paillier_.sub(c[i], c_b[i]);
        
        long a_i = mpz_tstbit(a_.get_mpz_t(),i);
        
        switch (a_i+s_) {
            case 1:
            c[i] = paillier_.add(c[i], paillier_one_);
            break;
            
            case 2:
            c[i] = paillier_.add(c[i], paillier_one_*paillier_one_);
            break;
            
            case -1:
            c[i] = paillier_.sub(c[i], paillier_one_);
            break;
            
            default:
            break;
        }
        
    }
    
    // you will have to rerandomize and shuffle c
    return c;
}

vector<mpz_class> Compare_A::rerandomize(const vector<mpz_class> &c)
{
    ScopedTimer timer("rerandomize");
    vector<mpz_class> c_rand(c);
    
    for (size_t i = 0; i < c_rand.size(); i++) {
        c_rand[i] = paillier_.scalarize(c_rand[i]);
    }
    
    return c_rand;
}

vector<mpz_class> Compare_A::rerandomize_parallel(const vector<mpz_class> &c, unsigned int n_threads)
{
    ScopedTimer timer("rerandomize parallel");
 
    ThreadPool pool(n_threads);
    vector<mpz_class> c_rand(c);
    
    Paillier *paillier_ptr = &paillier_;
    vector<mpz_class> *c_rand_ptr = &c_rand;
    
    for (size_t i = 0; i < c_rand.size(); i++) {
        auto job =[paillier_ptr,i,c_rand_ptr]
        {
            (*c_rand_ptr)[i] = paillier_ptr->scalarize((*c_rand_ptr)[i]);
        };
        pool.enqueue(job);
    }

    
    return c_rand;
}

void Compare_A::unblind(const mpz_class &t_prime)
{
    ScopedTimer timer("unblind");

    if (s_ == 1) {
        res_ = t_prime;
    }else{
        res_ = gm_.neg(t_prime);
    }
}

void runProtocol(Compare_A &party_a, Compare_B &party_b, gmp_randstate_t state)
{
    vector<mpz_class> c_b;
    c_b = party_b.encrypt_bits_parallel(2);
    
    vector<mpz_class> c_w = party_a.compute_w(c_b);
    vector<mpz_class> c_sums = party_a.compute_sums(c_w);
    vector<mpz_class> c = party_a.compute_c(c_b,c_sums);
//    vector<mpz_class> c_rand = party_a.rerandomize(c);
    vector<mpz_class> c_rand = party_a.rerandomize_parallel(c,2);
    
    // we have to suffle
    ScopedTimer *timer = new ScopedTimer("shuffle");

    random_shuffle(c_rand.begin(),c_rand.end(),[state](int n){ return gmp_urandomm_ui(state,n); });
    
    delete timer;
    mpz_class t_prime = party_b.search_zero(c_rand);
    party_a.unblind(t_prime);
}