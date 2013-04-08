#pragma once

#include <gmpxx.h>

namespace {

// computes |z|
inline mpz_class
mpz_class_abs(const mpz_class &z)
{
    mpz_class a;
    mpz_abs(a.get_mpz_t(), z.get_mpz_t());
    return a;
}

// computes nearest(p/q)
inline mpz_class
mpz_class_nearest_div(const mpz_class &p, const mpz_class &q)
{
    // XXX: slow
    if (mpz_divisible_p(p.get_mpz_t(), q.get_mpz_t()))
        return p/q;
    mpz_class ret;
    mpz_class pplusq2 = p + (q/2);
    mpz_fdiv_q(ret.get_mpz_t(), pplusq2.get_mpz_t(), q.get_mpz_t());
    return ret;
}

// precondition: q is positive
//
// computes p mod q, handling negative values of p such that the return value r is
// always positive (0 <= r < q)
inline mpz_class
mpz_class_mod(const mpz_class &p, const mpz_class &q)
{
    mpz_class r;
    mpz_mod(r.get_mpz_t(), p.get_mpz_t(), q.get_mpz_t());
    return r;
}

} // empty namespace

/* vim:set shiftwidth=4 ts=4 sts=4 et: */
