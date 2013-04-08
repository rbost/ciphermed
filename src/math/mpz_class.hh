#pragma once

#include <gmpxx.h>

namespace {

inline mpz_class
mpz_class_abs(const mpz_class &z)
{
	mpz_class a;
	mpz_abs(a.get_mpz_t(), z.get_mpz_t());
	return a;
}

} // empty namespace
