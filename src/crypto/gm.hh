#pragma once

#include <NTL/ZZ.h>
#include <vector>
#include <list>
#include <utility>

class GM {
public:
    GM(const std::vector<NTL::ZZ> &pk);
    std::vector<NTL::ZZ> pubkey() const { return {N, y}; }
    
    NTL::ZZ encrypt(const bool &bit);
    NTL::ZZ reRand(const NTL::ZZ &c);
    NTL::ZZ XOR(const NTL::ZZ &c1, const NTL::ZZ &c2);
    
    void rand_gen(size_t niter = 100, size_t nmax = 1000);

protected:
    /* Public key */
    const NTL::ZZ N, y;
    
    /* Pre-computed randomness */
    std::list< NTL::ZZ > rqueue;
};

class GM_priv : public GM {
public:
    GM_priv(const std::vector<NTL::ZZ> &sk);
    std::vector<NTL::ZZ> privkey() const { return { p, q }; }
    
    bool decrypt_fast(const NTL::ZZ &ciphertext) const;
    bool decrypt(const NTL::ZZ &ciphertext) const;

    static std::vector<NTL::ZZ> keygen(unsigned int nbits = 1024);

protected:
    /* Private key */
    const NTL::ZZ p,q;
    
    /* Cached values */
    const NTL::ZZ pMinOneBy2, qMinOneBy2;
};