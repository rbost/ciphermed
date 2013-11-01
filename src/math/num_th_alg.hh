#pragma once


#include <gmpxx.h>
#include <vector>

std::vector<mpz_class> gen_rand_non_increasing_seq(const mpz_class &m, gmp_randstate_t state);
std::vector<mpz_class> extract_prime_seq(const std::vector<mpz_class> &seq, int reps = 25);
std::vector<mpz_class> gen_rand_number_factorization(const mpz_class &m, mpz_class *result, gmp_randstate_t state, int reps = 25);
std::vector<mpz_class> gen_rand_prime_with_factorization(const mpz_class &m, mpz_class *p, gmp_randstate_t state, int reps = 25);
mpz_class simple_safe_prime_gen(size_t n_bits, gmp_randstate_t state, int reps = 25);
void gen_germain_prime(mpz_class& n, long k,gmp_randstate_t state, long err = 80);