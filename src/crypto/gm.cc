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
#include <crypto/gm.hh>
#include <math/util_gmp_rand.h>

#include <iostream>

using namespace std;

GM::GM(const vector<mpz_class> &pk, gmp_randstate_t state) : N(pk[0]), y(pk[1])
{
    assert(pk.size() == 2);
    gmp_randinit_set(_randstate, state);
}

void
GM::rand_gen(size_t niter, size_t nmax)
{
    if (rqueue.size() >= nmax)
        niter = 0;
    else
        niter = min(niter, nmax - rqueue.size());
    
    for (size_t i = 0; i < niter; i++) {
        
        mpz_class r;
        
        do {
            mpz_urandomm(r.get_mpz_t(),_randstate,N.get_mpz_t());
        } while (mpz_class_gcd(r,N) != 1);
        
        rqueue.push_back(mpz_class_powm_ui(r,2,N));
    }
}

mpz_class GM::encrypt(const bool &bit)
{
    mpz_class r2;
    auto i = rqueue.begin();
    if (i != rqueue.end()) {
        r2 = *i;
        rqueue.pop_front();
        
    } else {
        mpz_class r;
        do {
            mpz_urandomm(r.get_mpz_t(),_randstate,N.get_mpz_t());
        } while (mpz_class_gcd(r,N) != 1);
        
        r2 = mpz_class_powm_ui(r,2,N);
    }
    
    if (bit) {
        return (r2 * y)%N;
    }
    
    return r2;
}

mpz_class GM::reRand(const mpz_class &c)
{
    mpz_class r2;
    auto i = rqueue.begin();
    if (i != rqueue.end()) {
        r2 = *i;
        rqueue.pop_front();
        
    } else {
        mpz_class r;
        do {
            mpz_urandomm(r.get_mpz_t(),_randstate,N.get_mpz_t());
        } while (mpz_class_gcd(r,N) != 1);
        
        r2 = mpz_class_powm_ui(r,2,N);
    }
    
    return (r2 * c)%N;
}

mpz_class GM::XOR(const mpz_class &c1, const mpz_class &c2)
{
    return (c1 * c2)%N;
}
mpz_class GM::neg(const mpz_class &c)
{
    return (c*y)%N;
}

GM_priv::GM_priv(const vector<mpz_class> &sk, gmp_randstate_t state) : GM({sk[0],sk[1]},state), p(sk[2]), q(sk[3]), pMinOneBy2((p-1)/2), qMinOneBy2((q-1)/2)
{
    assert(sk.size() == 4);
}

bool GM_priv::decrypt_fast(const mpz_class &ciphertext) const
{
    mpz_class cp = ciphertext % p;
    return ( mpz_class_powm(cp,pMinOneBy2,p) == 1);
}

bool GM_priv::decrypt(const mpz_class &ciphertext) const
{
    mpz_class cp = ciphertext % p;
    mpz_class cq = ciphertext % q;
    
    return ( mpz_class_powm(cp,pMinOneBy2,p) != 1)&&( mpz_class_powm(cq,pMinOneBy2,q) != 1);
}


vector<mpz_class> GM_priv::keygen(gmp_randstate_t randstate, unsigned int nbits)
{
    mpz_class p,q;
    
    mpz_random_prime_len(p.get_mpz_t(),randstate,nbits/2,40);
    mpz_random_prime_len(q.get_mpz_t(),randstate,nbits/2,40);
    mpz_class N = p*q;
    
    mpz_class pMinOneBy2 = (p-1)/2;
    mpz_class qMinOneBy2 = (q-1)/2;
    
    mpz_class y,rp, rq;
    
    do {
        mpz_urandomm(y.get_mpz_t(),randstate,N.get_mpz_t());
        rp = mpz_class_powm(y%p,pMinOneBy2,p);
        rq = mpz_class_powm(y%q,qMinOneBy2,q);
    }while(rp == 0 || rp == 1 || rq == 0 || rq == 1);
    
    return {N,y,p,q};
}

