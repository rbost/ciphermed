#pragma once

#include <NTL/ZZ.h>
#include <math/mpz_class.hh>

#include <vector>
#include <list>
#include <utility>

class GM {
public:
    GM(const std::vector<mpz_class> &pk, gmp_randstate_t state);
    std::vector<mpz_class> pubkey() const { return {N, y}; }
    
    mpz_class encrypt(const bool &bit);
    mpz_class reRand(const mpz_class &c);
    mpz_class XOR(const mpz_class &c1, const mpz_class &c2);
    
    void rand_gen(size_t niter = 100, size_t nmax = 1000);

protected:
    /* Public key */
    const mpz_class N, y;
    
    /* Randomness state */
    gmp_randstate_t _randstate;
    
    /* Pre-computed randomness */
    std::list< mpz_class > rqueue;
};

class GM_priv : public GM {
public:
    GM_priv(const std::vector<mpz_class> &sk, gmp_randstate_t state);
    std::vector<mpz_class> privkey() const { return { p, q }; }
    
    bool decrypt_fast(const mpz_class &ciphertext) const;
    bool decrypt(const mpz_class &ciphertext) const;

    static std::vector<mpz_class> keygen(gmp_randstate_t randstate, unsigned int nbits = 1024);

protected:
    /* Private key */
    const mpz_class p,q;
    
    /* Cached values */
    const mpz_class pMinOneBy2, qMinOneBy2;
};