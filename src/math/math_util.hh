#pragma once

#include <vector>

#include <gmpxx.h>

inline int mpz_class_probab_prime_p(const mpz_class &n, int reps)
{
    return mpz_probab_prime_p(n.get_mpz_t(),reps);
}

// v: residues, m: moduli, s: number of elements
void mpz_t_crt(mpz_t x, const mpz_ptr *v, const mpz_ptr *m, size_t s);

mpz_class mpz_class_crt(const std::vector<mpz_class> &v, const std::vector<mpz_class> &m);

inline mpz_class mpz_class_crt_2(const mpz_class &v1, const mpz_class &v2, const mpz_class &m1, const mpz_class &m2)
{
    return mpz_class_crt({v1,v2},{m1,m2});
}
