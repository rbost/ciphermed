#pragma once

#include <algorithm>
#include <cassert>
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
    
inline mpz_class mpz_class_gcd(const mpz_class &p, const mpz_class &q)
{
    mpz_class d;
    mpz_gcd (d.get_mpz_t(), p.get_mpz_t(), q.get_mpz_t());
    return d;
}

inline void mpz_class_gcd(mpz_class &d, const mpz_class &p, const mpz_class &q)
{
    mpz_gcd (d.get_mpz_t(), p.get_mpz_t(), q.get_mpz_t());
}
    
inline mpz_class mpz_class_powm(const mpz_class &base, const mpz_class &exp, const mpz_class &mod)
{
    mpz_class rop;
    mpz_powm(rop.get_mpz_t(),base.get_mpz_t(),exp.get_mpz_t(),mod.get_mpz_t());
    return rop;
}

inline void mpz_class_powm(mpz_class &rop, const mpz_class &base, const mpz_class &exp, const mpz_class &mod)
{
    mpz_powm(rop.get_mpz_t(),base.get_mpz_t(),exp.get_mpz_t(),mod.get_mpz_t());
}

inline mpz_class mpz_class_powm_ui(const mpz_class &base, unsigned long int exp, const mpz_class &mod)
{
    mpz_class rop;
    mpz_powm_ui(rop.get_mpz_t(),base.get_mpz_t(),exp,mod.get_mpz_t());
    return rop;
}

inline void mpz_class_powm_ui(mpz_class &rop, const mpz_class &base, unsigned long int exp, const mpz_class &mod)
{
    mpz_powm_ui(rop.get_mpz_t(),base.get_mpz_t(),exp,mod.get_mpz_t());
}

inline mpz_class mpz_class_pow_ui(const mpz_class &base, unsigned long int exp)
{
    mpz_class rop;
    mpz_pow_ui(rop.get_mpz_t(),base.get_mpz_t(),exp);
    return rop;
}

inline void mpz_class_pow_ui(mpz_class &rop, const mpz_class &base, unsigned long int exp)
{
    mpz_pow_ui(rop.get_mpz_t(),base.get_mpz_t(),exp);
}

inline mpz_class mpz_class_ui_pow_ui(unsigned long int base, unsigned long int exp)
{
    mpz_class rop;
    mpz_ui_pow_ui(rop.get_mpz_t(),base,exp);
    return rop;
}

inline void mpz_class_ui_pow_ui(mpz_class &rop, unsigned long int base, unsigned long int exp)
{
    mpz_ui_pow_ui(rop.get_mpz_t(),base,exp);
}

inline mpz_class mpz_class_invert (const mpz_class &op1, const mpz_class &op2)
{
    mpz_class rop;
    assert(mpz_invert(rop.get_mpz_t(),op1.get_mpz_t(),op2.get_mpz_t()) != 0);
    
    return rop;
}

inline int mpz_class_invert (mpz_class &rop, const mpz_class &op1, const mpz_class &op2)
{
    return mpz_invert(rop.get_mpz_t(),op1.get_mpz_t(),op2.get_mpz_t());
}

} // empty namespace

// specialize std::swap()
namespace std {
    template <>
    inline void
    swap(mpz_class &a, mpz_class &b)
    {
        std::swap(*a.get_mpz_t(), *b.get_mpz_t());
    }
}

/* vim:set shiftwidth=4 ts=4 sts=4 et: */
