#pragma once

#include <gmp.h>

extern "C"
{
// Generates a random integer of exactly n bits, uniformly distributed in the range 2^(n-1), 2^n - 1
inline void mpz_urandom_len(mpz_t rop, gmp_randstate_t state, mp_bitcnt_t n)
    {
        mpz_urandomb(rop,state,n-1);
        mpz_setbit(rop,n-1);
    }

// Generates a random prime of exactly n bits, testing primality using reps Miller-Rabin primality tests
inline void mpz_random_prime_len(mpz_t rop, gmp_randstate_t state, mp_bitcnt_t n, int reps)
    {
        do {
            mpz_urandom_len(rop,state,n);
        } while (mpz_probab_prime_p(rop,reps) == 0);
    }

}