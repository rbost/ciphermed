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


#include <gmpxx.h>
#include <vector>

std::vector<mpz_class> gen_rand_non_increasing_seq(const mpz_class &m, gmp_randstate_t state);
std::vector<mpz_class> extract_prime_seq(const std::vector<mpz_class> &seq, int reps = 25);
std::vector<mpz_class> gen_rand_number_factorization(const mpz_class &m, mpz_class *result, gmp_randstate_t state, int reps = 25);
std::vector<mpz_class> gen_rand_prime_with_factorization(const mpz_class &m, mpz_class *p, gmp_randstate_t state, int reps = 25);
mpz_class simple_safe_prime_gen(size_t n_bits, gmp_randstate_t state, int reps = 25);
void gen_germain_prime(mpz_class& n, long k,gmp_randstate_t state, long err = 80);

// Constructs a generator for the cyclic group \Z^*_p where p is a Sophie Germain prime
mpz_class get_generator_for_cyclic_group(const mpz_class &p, gmp_randstate_t state)
;