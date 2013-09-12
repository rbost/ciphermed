#include <assert.h>
#include <crypto/gm.hh>

#include <iostream>

using namespace std;
using namespace NTL;

GM::GM(const vector<ZZ> &pk) : N(pk[0]), y(pk[1])
{
    assert(pk.size() == 2);
}

void
GM::rand_gen(size_t niter, size_t nmax)
{
    if (rqueue.size() >= nmax)
        niter = 0;
    else
        niter = min(niter, nmax - rqueue.size());
    
    for (size_t i = 0; i < niter; i++) {
        
        ZZ r;
        
        do {
            r = RandomBnd(N);
        } while (GCD(r,N) != to_ZZ(1));
        
        rqueue.push_back(SqrMod(r,N));
    }
}

ZZ GM::encrypt(const bool &bit)
{
    ZZ r2;
    auto i = rqueue.begin();
    if (i != rqueue.end()) {
        r2 = *i;
        rqueue.pop_front();
        
    } else {
        ZZ r;
        do {
            r = RandomBnd(N);
        } while (GCD(r,N) != to_ZZ(1));
        
        r2 = SqrMod(r,N);
    }
    
    if (bit) {
        return MulMod(r2,y,N);
    }
    
    return r2;
}

ZZ GM::reRand(const ZZ &c)
{
    ZZ r2;
    auto i = rqueue.begin();
    if (i != rqueue.end()) {
        r2 = *i;
        rqueue.pop_front();
        
    } else {
        ZZ r;
        do {
            r = RandomBnd(N);
        } while (GCD(r,N) != to_ZZ(1));
        
        r2 = SqrMod(r,N);
    }
    
    return MulMod(r2,c,N);
}

ZZ GM::XOR(const ZZ &c1, const ZZ &c2)
{
    return MulMod(c1,c2,N);
}

GM_priv::GM_priv(const vector<ZZ> &sk) : GM({sk[0],sk[1]}), p(sk[2]), q(sk[3]), pMinOneBy2((p-1)/2), qMinOneBy2((q-1)/2)
{
    assert(sk.size() == 4);
}

bool GM_priv::decrypt_fast(const ZZ &ciphertext) const
{
    ZZ cp = ciphertext % p;
    return ( PowerMod(cp,pMinOneBy2,p) == to_ZZ(1));
}

bool GM_priv::decrypt(const ZZ &ciphertext) const
{
    ZZ cp = ciphertext % p;
    ZZ cq = ciphertext % q;
    
    return ( PowerMod(cp,pMinOneBy2,p) == to_ZZ(1))&&( PowerMod(cq,pMinOneBy2,q) == to_ZZ(1));
}


vector<ZZ> GM_priv::keygen(unsigned int nbits)
{
    ZZ p = RandomPrime_ZZ(nbits/2);
    ZZ q = RandomPrime_ZZ(nbits/2);
    ZZ N = p*q;
    
    ZZ pMinOneBy2 = (p-1)/2;
    ZZ qMinOneBy2 = (q-1)/2;
    
    ZZ y,rp, rq;
    
    do {
        y = RandomBnd(N);
        rp = PowerMod(y%p,pMinOneBy2,p);
        rq = PowerMod(y%q,pMinOneBy2,q);
    } while (rp == to_ZZ(p-1) && rq == to_ZZ(q-1));
    
    return {N,y,p,q};
}

