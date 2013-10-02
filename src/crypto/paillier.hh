#pragma once

#include <list>
#include <vector>
#include <NTL/ZZ.h>
#include <math/mpz_class.hh>

class Paillier {
 public:
    Paillier(const std::vector<mpz_class> &pk, gmp_randstate_t state);
    std::vector<mpz_class> pubkey() const { return { n, g }; }

    mpz_class encrypt(const mpz_class &plaintext);
    mpz_class add(const mpz_class &c0, const mpz_class &c1) const;
    mpz_class constMult(const mpz_class &m, const mpz_class &c) const;
    mpz_class constMult(long m, const mpz_class &c) const;
    mpz_class constMult(const mpz_class &c, long m) const { return constMult(m,c); };

    void rand_gen(size_t niter = 100, size_t nmax = 1000);

 protected:
    /* Public key */
    const mpz_class n, g;

    /* Randomness state */
    gmp_randstate_t _randstate;

    /* Cached values */
    const uint nbits;
    const mpz_class n2;

    /* Pre-computed randomness */
    std::list<mpz_class> rqueue;
};

class Paillier_priv : public Paillier {
 public:
    Paillier_priv(const std::vector<mpz_class> &sk, gmp_randstate_t state);
    std::vector<mpz_class> privkey() const { return { p, q, g, a }; }

    mpz_class decrypt(const mpz_class &ciphertext) const;
    static std::vector<mpz_class> keygen(gmp_randstate_t state, uint nbits = 1024, uint abits = 256);


 private:
    /* Private key, including g from public part; n=pq */
    const mpz_class p, q;
    const mpz_class a;      /* non-zero for fast mode */

    /* Cached values */
    const bool fast;
    const mpz_class p2, q2;
    const mpz_class two_p, two_q;
    const mpz_class pinv, qinv;
    const mpz_class hp, hq;
};
