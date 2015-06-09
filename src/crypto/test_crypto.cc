/*
 * Copyright 2013-2015 Raphael Bost
 *
 * This file is part of ciphermed.

 *  ciphermed is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 * 
 *  ciphermed is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 * 
 *  You should have received a copy of the GNU General Public License
 *  along with ciphermed.  If not, see <http://www.gnu.org/licenses/>. 2
 *
 */

#include <assert.h>
#include <vector>
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
    
    mpz_class ct0 = pp.encrypt(pt0);
    mpz_class ct1 = pp.encrypt(pt1);
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
        pp.encrypt(pt);
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
        ct[i] = pp.encrypt(pt);
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
