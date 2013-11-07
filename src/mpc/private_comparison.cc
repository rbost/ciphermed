#include <mpc/private_comparison.hh>

#include <thread>
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
    vector<mpz_class> c_b(bit_length_);
    
    
    size_t n = bit_length_;
    size_t m = ceilf( ((float)n)/n_threads);
    thread threads[n_threads];

    auto job =[this,&c_b](size_t i_start,size_t i_end)
    {
        for (size_t i = i_start; i < i_end; i++) {
            c_b[i] = paillier_.encrypt(mpz_tstbit(b_.get_mpz_t(),i));
        }
    };

    size_t t = 0, i_start = 0;
    
    for (t = 0; t < n_threads; t++) {
        threads[t] = thread(job,i_start,min<size_t>(i_start+m,n));
        i_start += m;
    }
    
    
    size_t t_max = t;
    for (t = 0; t < t_max; t++) {
        threads[t].join();
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
    gmp_randinit_set(randstate_, state);

}


std::vector<mpz_class> Compare_A::compute(const std::vector<mpz_class> &c_b, unsigned int n_threads)
{
    vector<mpz_class> c = compute_w(c_b);
    c = compute_sums(c);
    
    vector<size_t> rerand_indexes(0);
    c = compute_c(c_b,c,rerand_indexes);
    
    c = rerandomize_parallel(c,rerand_indexes,n_threads);
    shuffle(c);
    
    
    return c;
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

vector<mpz_class> Compare_A::compute_c(const std::vector<mpz_class> &c_b,const std::vector<mpz_class> &c_sums, std::vector<size_t> &rerand_indexes)
{
    ScopedTimer timer("compute_c");
    
    vector<mpz_class> c(bit_length_);
    long delta = (1-s_)/2;
    
    
    for (size_t i = 0; i < bit_length_; i++) {
        long a_i = mpz_tstbit(a_.get_mpz_t(),i);

        if (a_i != delta) {
            // a_i != delta => c_i > 0
            // avoid computations, generate a random element
            // the decryption of c[i] can be 0 but this happens with neg. prob.
            c[i] = paillier_.random_encryption();
            continue;
        }
        c[i] = paillier_.constMult(3,c_sums[i]);
        
        c[i] = paillier_.sub(c[i], c_b[i]);
        
        
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
        
        rerand_indexes.push_back(i);
        
    }
    
    // you will have to rerandomize and shuffle c
    return c;
}

vector<mpz_class> Compare_A::rerandomize(const vector<mpz_class> &c, const std::vector<size_t> &rerand_indexes)
{
    ScopedTimer timer("rerandomize");
    vector<mpz_class> c_rand(c);
    
    for (size_t i = 0; i < rerand_indexes.size(); i++) {
        c_rand[rerand_indexes[i]] = paillier_.scalarize(c_rand[rerand_indexes[i]]);
    }
    
    return c_rand;
}

void threadCall(Paillier &paillier, vector<mpz_class> &c_rand, vector<size_t> &rerand_indexes, size_t i_start, size_t i_end)
{
    for (size_t i = i_start; i < i_end; i++) {
        c_rand[rerand_indexes[i]] = paillier.scalarize(c_rand[rerand_indexes[i]]);
    }

}

void call_from_thread(int tid) {
    std::cout << "Launched by thread " << tid << std::endl;
}

vector<mpz_class> Compare_A::rerandomize_parallel(const vector<mpz_class> &c, const std::vector<size_t> &rerand_indexes, unsigned int n_threads)
{
    ScopedTimer timer("rerandomize parallel");
 
    vector<mpz_class> c_rand(c);
    
    size_t n = rerand_indexes.size();
    size_t m = ceilf( ((float)n)/n_threads);
    thread threads[n_threads];
    
    auto job =[this,&c_rand,&rerand_indexes](size_t i_start,size_t i_end)
    {
        for (size_t i = i_start; i < i_end; i++) {
            c_rand[rerand_indexes[i]] = paillier_.scalarize(c_rand[rerand_indexes[i]]);
        }
    };

    size_t t = 0, i_start = 0;
    
    for (t = 0; t < n_threads; t++) {
        threads[t] = thread(job,i_start,min<size_t>(i_start+m,n));
        i_start += m;
    }
    
    
    size_t t_max = t;
    for (t = 0; t < t_max; t++) {
        threads[t].join();
    }
    
    return c_rand;
}

void Compare_A::shuffle(std::vector<mpz_class> &c)
{
    random_shuffle(c.begin(),c.end(),[this](int n){ return gmp_urandomm_ui(randstate_,n); });
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
//    c_b = party_b.encrypt_bits();
    c_b = party_b.encrypt_bits_parallel(2);

    vector<mpz_class> c = party_a.compute(c_b,2);

    
//    delete timer;
    mpz_class t_prime = party_b.search_zero(c);
    party_a.unblind(t_prime);
}