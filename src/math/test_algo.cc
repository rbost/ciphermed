#include <iostream>
#include <math/num_th_alg.hh>
#include <util/util.hh>

using namespace std;

static void test_algo(size_t m_bits)
{
    mpz_class m = 0;
    mpz_setbit(m.get_mpz_t(),m_bits);

    
    mpz_class p;

    gmp_randstate_t randstate;
    gmp_randinit_default(randstate);
    gmp_randseed_ui(randstate,time(NULL));

    ScopedTimer *t = new ScopedTimer("Test algo");
    std::vector<mpz_class> fact = gen_rand_prime_with_factorization(m,&p,randstate,25);
    delete t;
    
    cout << "Prime generated is \n" << p << endl;
    cout << "Factors:\n";
    
    for (size_t i = 0; i < fact.size(); i++) {
        cout << fact[i] << endl;
    }
    
    cout << "\n" << m_bits << " bits queried\n";
    cout << "p is " << mpz_sizeinbase(p.get_mpz_t(),2) << " bits" << endl;
}


int main()
{
    test_algo(512);
    
    return 0;
}