#include <mpc/private_comparison.hh>

#include <util/util.hh>

using namespace std;

Compare_A::Compare_A(const mpz_class &x, const size_t &l, Paillier_priv &paillier, GM_priv &gm)
: a_(x), bit_length_(l), paillier_(paillier), gm_(gm)
{
    
}

vector<mpz_class> Compare_A::encrypt_bits()
{
    ScopedTimer timer("encrypt_bits");

    vector<mpz_class> c_a(bit_length_);
    
    for (size_t i = 0; i < bit_length_; i++) {
        c_a[i] = paillier_.encrypt(mpz_tstbit(a_.get_mpz_t(),i));
//        c_a[i] = mpz_tstbit(a_.get_mpz_t(),i);
    }
    
    return c_a;
}

mpz_class Compare_A::search_zero(const vector<mpz_class> &c)
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

Compare_B::Compare_B(const mpz_class &y, const size_t &l, Paillier &paillier, GM &gm, gmp_randstate_t state)
: b_(y), bit_length_(l), paillier_(paillier), gm_(gm)
{
    s_ = 1 - 2*gmp_urandomb_ui(state,1);
    paillier_one_ = paillier_.pubkey()[1]; // g is an encryption of 1
}

vector<mpz_class> Compare_B::compute_w(const std::vector<mpz_class> &c_a)
{
    ScopedTimer timer("compute_w");

    vector<mpz_class> c_w(bit_length_);
    // can be parallelized
    for (size_t i = 0; i < bit_length_; i++) {
        if (mpz_tstbit(b_.get_mpz_t(),i) == 0) {
            c_w[i] = c_a[i];
        }else{
            c_w[i] = paillier_.sub(paillier_one_,c_a[i]);
        }
    }

    return c_w;
}

vector<mpz_class> Compare_B::compute_sums(const std::vector<mpz_class> &c_w)
{
    ScopedTimer timer("compute_sums");

    vector<mpz_class> c_sums(bit_length_);
    c_sums[bit_length_-1] = 1;

    for (size_t i = bit_length_-1; i > 0; i--) {
        c_sums[i-1] = paillier_.add(c_sums[i],c_w[i]);
    }
    
    return c_sums;
}

vector<mpz_class> Compare_B::compute_c(const std::vector<mpz_class> &c_a,const std::vector<mpz_class> &c_sums)
{
    ScopedTimer timer("compute_c");

    vector<mpz_class> c(bit_length_);

    for (size_t i = 0; i < bit_length_; i++) {
        c[i] = paillier_.constMult(3,c_sums[i]);
    
        c[i] = paillier_.sub(c[i], c_a[i]);
      
        long b_i = mpz_tstbit(b_.get_mpz_t(),i);
        
        switch (b_i+s_) {
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

vector<mpz_class> Compare_B::rerandomize(const vector<mpz_class> &c)
{
    ScopedTimer timer("rerandomize");
    vector<mpz_class> c_rand(c);
    
    for (size_t i = 0; i < c_rand.size(); i++) {
        c_rand[i] = paillier_.scalarize(c_rand[i]);
    }
    
    return c_rand;
}

mpz_class Compare_B::unblind(const mpz_class &t_prime)
{
    ScopedTimer timer("unblind");

    if (s_ == -1) {
        return t_prime;
    }
    
    return gm_.neg(t_prime);
}

mpz_class runProtocol(Compare_A &party_a, Compare_B &party_b, gmp_randstate_t state)
{
    vector<mpz_class> c_a = party_a.encrypt_bits();
    
    vector<mpz_class> c_w = party_b.compute_w(c_a);
    vector<mpz_class> c_sums = party_b.compute_sums(c_w);
    vector<mpz_class> c = party_b.compute_c(c_a,c_sums);
    vector<mpz_class> c_rand = party_b.rerandomize(c);
    
    // we have to suffle
    ScopedTimer *timer = new ScopedTimer("shuffle");

    random_shuffle(c_rand.begin(),c_rand.end(),[state](int n){ return gmp_urandomm_ui(state,n); });
    
    delete timer;
    mpz_class t_prime = party_a.search_zero(c_rand);
    return party_b.unblind(t_prime);
}
