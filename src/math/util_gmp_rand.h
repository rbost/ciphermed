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