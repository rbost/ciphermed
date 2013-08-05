#include <assert.h>
#include <crypto/elgamal.hh>

#include <iostream>

using namespace std;
using namespace NTL;

/*
 * Public-key operations
 */

ElGamal::ElGamal(const std::vector<NTL::ZZ> &pk)
    : p(pk[0]), g(pk[1]), h(pk[2]), q((pk[0]-1)/2), qbits(NumBits(q))
{
    assert(pk.size() == 3);
}

void
ElGamal::rand_gen(size_t niter, size_t nmax)
{
    if (rqueue.size() >= nmax)
        niter = 0;
    else
        niter = min(niter, nmax - rqueue.size());
    
    for (size_t i = 0; i < niter; i++) {
        ZZ r = RandomLen_ZZ(qbits) % (q-1) + 1;
        ZZ t = PowerMod(g, r, p);
        ZZ s = PowerMod(h, r, p);
        rqueue.push_back(pair<ZZ,ZZ>(t,s));
    }
}


pair<ZZ,ZZ> ElGamal::encrypt(const ZZ &plaintext)
{
    auto i = rqueue.begin();
    if (i != rqueue.end()) {
        pair<ZZ,ZZ> rn = *i;
        rqueue.pop_front();
        
        ZZ c1 = get<0>(rn);
        ZZ c2 = MulMod(plaintext, get<1>(rn), p);
        
        return pair<ZZ,ZZ>(c1,c2);
    } else {
        ZZ r = RandomLen_ZZ(qbits) % (q-1) + 1;
        
        ZZ c1 = PowerMod(g,r,p);
        ZZ c2 = MulMod(plaintext,PowerMod(h,r,p), p);
        
        return pair<ZZ,ZZ>(c1,c2);
    }
}

pair<ZZ,ZZ> ElGamal::randEncrypt()
{
    ZZ rnd = RandomBnd(q-1) + 1; // 0 is not in the message space. 
	return encrypt(rnd);
}

pair<ZZ,ZZ> ElGamal::encrypt1()
{
    auto i = rqueue.begin();
    if (i != rqueue.end()) {
        pair<ZZ,ZZ> rn = *i;
        rqueue.pop_front();
        
        return rn;
    } else {
        ZZ r = RandomLen_ZZ(qbits) % (q-1) + 1;
        
        ZZ c1 = PowerMod(g,r,p);
        ZZ c2 = PowerMod(h,r,p);
        
        return pair<ZZ,ZZ>(c1,c2);
    }
}

pair<ZZ,ZZ> ElGamal::mult(const pair<ZZ,ZZ> &c0, const pair<ZZ,ZZ> &c1) const
{
    ZZ m1 = MulMod(get<0>(c0),get<0>(c1),p);
    ZZ m2 = MulMod(get<1>(c0),get<1>(c1),p);
    
    return pair<ZZ,ZZ> (m1,m2);
}

pair<ZZ,ZZ> ElGamal::scalarize(const pair<ZZ,ZZ> &c) const
{                 
	ZZ k = RandomLen_ZZ(qbits) % q;
    ZZ m1 = PowerMod(get<0>(c),k,p);
    ZZ m2 = PowerMod(get<1>(c),k,p);
    
    return pair<ZZ,ZZ> (m1,m2);
}

ElGamal_priv::ElGamal_priv(const std::vector<NTL::ZZ> &sk)
    : ElGamal({sk[0],sk[1],sk[2]}), x(sk[3])
{
    assert(sk.size() == 4);
}

ZZ ElGamal_priv::decrypt(const pair<ZZ,ZZ> &ciphertext) const
{
    ZZ exp = SubMod(q, x, p);
    ZZ invS = PowerMod(get<0>(ciphertext), exp, p);
    
    return MulMod(get<1>(ciphertext), invS, p);
}

vector<ZZ> ElGamal_priv::keygen(unsigned int pbits)
{
    ZZ p, g, h, x;
    
    p = GenGermainPrime_ZZ(pbits);
    
    ZZ q = (p - 1)/2;
    
    // find a generator for ZZ_p
    // Shoup's algorithm

    ZZ alpha, beta;
    do {
        alpha = RandomLen_ZZ(pbits) % (p-1) + 1;
        beta = PowerMod(alpha, q, p);
    } while (beta == 1);
    
    g = beta;

    // get an integer between 2 and p-2 (included)
    alpha = RandomLen_ZZ(pbits)% (p-3) + 2;    
    beta = SqrMod(alpha,p);
    
    g = MulMod(g, beta, p);
    
    // to get a generator of QR_p, square the generator of ZZ_p
    g = SqrMod(g, p);
    
    do {
        x = RandomLen_ZZ(pbits-1) % q + 1;
    } while ((pbits-1 != (unsigned int) NumBits(x)));
             
    h = PowerMod(g, x, p);
    
    return { p, g, h, x};
}
