#pragma once


#include <gmpxx.h>
#include <vector>

std::vector<mpz_class> gen_rand_non_increasing_seq(const mpz_class &m, gmp_randstate_t state);
std::vector<mpz_class> extract_prime_seq(const std::vector<mpz_class> &seq, int reps = 25);
std::vector<mpz_class> gen_rand_number_factorization(const mpz_class &m, mpz_class *result, gmp_randstate_t state, int reps = 25);
std::vector<mpz_class> gen_rand_prime_with_factorization(const mpz_class &m, mpz_class *p, gmp_randstate_t state, int reps = 25);
