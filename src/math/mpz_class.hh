#pragma once

#include <gmpxx.h>

// this exists so we can spice up mpz_class
// the interface given by gmpxx is quite limited
namespace {

inline mpz_class
operator-(const mpz_class &z)
{
    mpz_class neg;
    mpz_neg(neg.get_mpz_t(), z.get_mpz_t());
    return neg;
}

inline mpz_class
operator<<(const mpz_class &z, unsigned long shift)
{
    mpz_class ret;
    mpz_mul_2exp(ret.get_mpz_t(), z.get_mpz_t(), shift);
    return ret;
}

inline mpz_class
mpz_class_abs(const mpz_class &z)
{
	mpz_class a;
	mpz_abs(a.get_mpz_t(), z.get_mpz_t());
	return a;
}

} // empty namespace
