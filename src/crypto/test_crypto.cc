#include <assert.h>
#include <vector>
#include <crypto/elgamal.hh>
#include <crypto/paillier.hh>
#include <crypto/gm.hh>
#include <NTL/ZZ.h>
#include <gmpxx.h>
#include <math/util_gmp_rand.h>

#include <ctime>

#include<iostream>

using namespace std;
using namespace NTL;


static void
test_elgamal()
{
    cout << "Test ElGamal ..." << flush;
    auto sk = ElGamal_priv::keygen();
    ElGamal_priv pp(sk);
    
    auto pk = pp.pubkey();
    ElGamal p(pk);
    
    ZZ pt0 = RandomLen_ZZ(256);
    ZZ pt1 = RandomLen_ZZ(256);    

    pair<ZZ,ZZ> ct0 = p.encrypt(pt0);
    pair<ZZ,ZZ> ct1 = p.encrypt(pt1);
    pair<ZZ,ZZ> prod = p.mult(ct0, ct1);
	pair<ZZ,ZZ> scal = p.scalarize(p.encrypt(to_ZZ(1)));

    assert(pp.decrypt(ct0) == pt0);
    assert(pp.decrypt(ct1) == pt1);
    assert(pp.decrypt(prod) == (pt0 * pt1));
    assert(pp.decrypt(scal) == to_ZZ(1));
    
    cout << " passed" << endl;
}

static void
test_paillier()
{
    cout << "Test Paillier ..." << flush;
    
    gmp_randstate_t randstate;
    gmp_randinit_default(randstate);
    gmp_randseed_ui(randstate,time(NULL));
    
    auto sk = Paillier_priv::keygen(randstate,600);
    Paillier_priv pp(sk,randstate);
    
    auto pk = pp.pubkey();
    mpz_class n = pk[0];
    Paillier p(pk,randstate);
    
    mpz_class pt0, pt1,m;
    mpz_urandomm(pt0.get_mpz_t(),randstate,n.get_mpz_t());
    mpz_urandomm(pt1.get_mpz_t(),randstate,n.get_mpz_t());
    mpz_urandomm(m.get_mpz_t(),randstate,n.get_mpz_t());
    
    mpz_class ct0 = p.encrypt(pt0);
    mpz_class ct1 = p.encrypt(pt1);
    mpz_class sum = p.add(ct0, ct1);
    mpz_class prod = p.constMult(m,ct0);
    //    mpz_class diff = p.constMult(-1, ct0);
    mpz_class diff = p.sub(ct0, ct1);
    
    assert(pp.decrypt(ct0) == pt0);
    assert(pp.decrypt(ct1) == pt1);
    assert(pp.decrypt(sum) == (pt0+pt1)%n);
    mpz_class d = pt0 - pt1;
    if (d < 0) {
        d += n;
    }
    assert( pp.decrypt(diff) == d);
    assert(pp.decrypt(prod) == (m*pt0)%n);
    
    cout << " passed" << endl;
}


static void
test_paillier_fast()
{
    cout << "Test Paillier Fast..." << flush;
    
    gmp_randstate_t randstate;
    gmp_randinit_default(randstate);
    gmp_randseed_ui(randstate,time(NULL));
    
    auto sk = Paillier_priv_fast::keygen(randstate,600);
    Paillier_priv_fast pp(sk,randstate);
    
    auto pk = pp.pubkey();
    mpz_class n = pk[0];
    Paillier p(pk,randstate);
    
    mpz_class pt0, pt1,m;
    mpz_urandomm(pt0.get_mpz_t(),randstate,n.get_mpz_t());
    mpz_urandomm(pt1.get_mpz_t(),randstate,n.get_mpz_t());
    mpz_urandomm(m.get_mpz_t(),randstate,n.get_mpz_t());
    
    mpz_class ct0 = pp.fast_encrypt(pt0);
    mpz_class ct1 = pp.fast_encrypt(pt1);
    mpz_class sum = p.add(ct0, ct1);
    mpz_class prod = p.constMult(m,ct0);
    //    mpz_class diff = p.constMult(-1, ct0);
    mpz_class diff = p.sub(ct0, ct1);
    
    assert(pp.decrypt(ct0) == pt0);
    assert(pp.decrypt(ct1) == pt1);
    assert(pp.decrypt(sum) == (pt0+pt1)%n);
    mpz_class d = pt0 - pt1;
    if (d < 0) {
        d += n;
    }
    assert( pp.decrypt(diff) == d);
    assert(pp.decrypt(prod) == (m*pt0)%n);
    
    cout << " passed" << endl;
}

static void paillier_perf(unsigned int k, unsigned int a_bits, size_t n_iteration)
{
    cout << "Test Paillier performances ..." << endl;
    
    cout << "k = " << k << "\na_bits = " << a_bits << "\n" << n_iteration << " iterations" << endl;
    
    gmp_randstate_t randstate;
    gmp_randinit_default(randstate);
    gmp_randseed_ui(randstate,time(NULL));
    
    auto sk = Paillier_priv::keygen(randstate,k,a_bits);
    Paillier_priv pp(sk,randstate);
    
    auto pk = pp.pubkey();
    mpz_class n = pk[0];
    Paillier p(pk,randstate);
    
    mpz_class pt0, pt1,m;
    mpz_urandomm(pt0.get_mpz_t(),randstate,n.get_mpz_t());
    mpz_urandomm(pt1.get_mpz_t(),randstate,n.get_mpz_t());
    mpz_urandomm(m.get_mpz_t(),randstate,n.get_mpz_t());
    
    mpz_class ct0 = p.encrypt(pt0);
    mpz_class ct1 = p.encrypt(pt1);
    mpz_class sum = p.add(ct0, ct1);
    mpz_class prod = p.constMult(m,ct0);
    //    mpz_class diff = p.constMult(-1, ct0);
    mpz_class diff = p.sub(ct0, ct1);
    
    assert(pp.decrypt(ct0) == pt0);
    assert(pp.decrypt(ct1) == pt1);
    assert(pp.decrypt(sum) == (pt0+pt1)%n);
    mpz_class d = pt0 - pt1;
    if (d < 0) {
        d += n;
    }
    assert( pp.decrypt(diff) == d);
    assert(pp.decrypt(prod) == (m*pt0)%n);
    
    struct timespec t0,t1;

    vector<mpz_class> ct(n_iteration);
    clock_gettime(CLOCK_THREAD_CPUTIME_ID,&t0);
    for (size_t i = 0; i < n_iteration; i++) {
        mpz_class pt;
        mpz_urandomm(pt.get_mpz_t(),randstate,n.get_mpz_t());
        ct[i] = p.encrypt(pt);
    }
    clock_gettime(CLOCK_THREAD_CPUTIME_ID,&t1);
    uint64_t t = (((uint64_t)t1.tv_sec) - ((uint64_t)t0.tv_sec) )* 1000000000 + (t1.tv_nsec - t0.tv_nsec);
    cerr << "public encryption: "<<  ((double)t/1000000)/n_iteration <<"ms per plaintext" << endl;

    clock_gettime(CLOCK_THREAD_CPUTIME_ID,&t0);
    for (size_t i = 0; i < n_iteration; i++) {
        mpz_class pt;
        mpz_urandomm(pt.get_mpz_t(),randstate,n.get_mpz_t());
        pp.fast_encrypt(pt);
    }
    clock_gettime(CLOCK_THREAD_CPUTIME_ID,&t1);
    t = (((uint64_t)t1.tv_sec) - ((uint64_t)t0.tv_sec) )* 1000000000 + (t1.tv_nsec - t0.tv_nsec);
    cerr << "private encryption: "<<  ((double)t/1000000)/n_iteration <<"ms per plaintext" << endl;

    clock_gettime(CLOCK_THREAD_CPUTIME_ID,&t0);
    for (size_t i = 0; i < n_iteration; i++) {
        mpz_class pt;
        mpz_urandomm(pt.get_mpz_t(),randstate,n.get_mpz_t());
        pp.fast_encrypt_precompute(pt);
    }
    clock_gettime(CLOCK_THREAD_CPUTIME_ID,&t1);
    t = (((uint64_t)t1.tv_sec) - ((uint64_t)t0.tv_sec) )* 1000000000 + (t1.tv_nsec - t0.tv_nsec);
    cerr << "private encryption with precomputation: "<<  ((double)t/1000000)/n_iteration <<"ms per plaintext" << endl;

    clock_gettime(CLOCK_THREAD_CPUTIME_ID,&t0);
    for (size_t i = 0; i < n_iteration; i++) {
        pp.decrypt(ct[i]);
    }
    clock_gettime(CLOCK_THREAD_CPUTIME_ID,&t1);
    t = (((uint64_t)t1.tv_sec) - ((uint64_t)t0.tv_sec) )* 1000000000 + (t1.tv_nsec - t0.tv_nsec);
    cerr << "decryption: "<<  ((double)t/1000000)/n_iteration <<"ms per cyphertext" << endl;

}


static void paillier_fast_perf(unsigned int k, size_t n_iteration)
{
    cout << "Test Paillier Fast performances ..." << endl;
    
    cout << "k = " << k << "\n" << n_iteration << " iterations" << endl;
    
    gmp_randstate_t randstate;
    gmp_randinit_default(randstate);
    gmp_randseed_ui(randstate,time(NULL));
    
    auto sk = Paillier_priv_fast::keygen(randstate,k);
    Paillier_priv_fast pp(sk,randstate);
    
    auto pk = pp.pubkey();
    mpz_class n = pk[0];
//    Paillier p(pk,randstate);
    
    mpz_class pt0, pt1,m;
    mpz_urandomm(pt0.get_mpz_t(),randstate,n.get_mpz_t());
    mpz_urandomm(pt1.get_mpz_t(),randstate,n.get_mpz_t());
    mpz_urandomm(m.get_mpz_t(),randstate,n.get_mpz_t());
    
    struct timespec t0,t1;
    uint64_t t;
    
    vector<mpz_class> ct(n_iteration);

    clock_gettime(CLOCK_THREAD_CPUTIME_ID,&t0);
    for (size_t i = 0; i < n_iteration; i++) {
        mpz_class pt;
        mpz_urandomm(pt.get_mpz_t(),randstate,n.get_mpz_t());
        ct[i] = pp.fast_encrypt(pt);
    }
    clock_gettime(CLOCK_THREAD_CPUTIME_ID,&t1);
    t = (((uint64_t)t1.tv_sec) - ((uint64_t)t0.tv_sec) )* 1000000000 + (t1.tv_nsec - t0.tv_nsec);
    cerr << "private encryption with generator precomputations: "<<  ((double)t/1000000)/n_iteration <<"ms per plaintext" << endl;
    
    clock_gettime(CLOCK_THREAD_CPUTIME_ID,&t0);
    for (size_t i = 0; i < n_iteration; i++) {
        pp.decrypt(ct[i]);
    }
    clock_gettime(CLOCK_THREAD_CPUTIME_ID,&t1);
    t = (((uint64_t)t1.tv_sec) - ((uint64_t)t0.tv_sec) )* 1000000000 + (t1.tv_nsec - t0.tv_nsec);
    cerr << "decryption: "<<  ((double)t/1000000)/n_iteration <<"ms per cyphertext" << endl;
    
}
static void
test_gm()
{
    cout << "Test GM ..." << flush;
    
    gmp_randstate_t randstate;
    gmp_randinit_default(randstate);
    gmp_randseed_ui(randstate,time(NULL));

    auto sk = GM_priv::keygen(randstate);
    GM_priv pp(sk,randstate);
    
    auto pk = pp.pubkey();
    GM p(pk,randstate);
    
    bool b0 = true; //(bool)RandomBits_long(1);
    bool b1 = false; //(bool)RandomBits_long(1);
    
    mpz_class ct0 = p.encrypt(b0);
    mpz_class ct1 = p.encrypt(b1);
    mpz_class XOR = p.XOR(ct0, ct1);
    mpz_class rerand = p.reRand(ct0);
    
    assert(pp.decrypt(pk[1]) == true);
    assert(pp.decrypt(ct0) == b0);
    assert(pp.decrypt(ct1) == b1);
    assert(pp.decrypt(XOR) == (b0 xor b1));
    assert(pp.decrypt(rerand) == b0);

    cout << " passed" << endl;
}

int
main(int ac, char **av)
{
    SetSeed(to_ZZ(time(NULL)));
//    test_elgamal();
	test_paillier();
	test_paillier_fast();
	test_gm();

    
    unsigned int k = 1024;
    unsigned int a_bits = 256;
    size_t n_iteration = 500;
    cout << endl;
    
//    paillier_perf(k,a_bits,n_iteration);
//    cout << endl;
    
    a_bits = 0;
    paillier_perf(k,a_bits,n_iteration);

    cout << endl;

    paillier_fast_perf(k, n_iteration);
    
    return 0;
}
