#pragma once

#include <stdint.h>
#include <assert.h>
#include <string>
#include <vector>

#include <stdio.h>
#include <gmpxx.h>

#include <math/poly.hh>

// Represents a polynomial ring R = Z[x]/(f(x)), where f(x) is the commonly
// chosen monic irreducible polynomial f(x) = x^n + 1.
//
// It is suggested that n be a power of 2 [makes discrete gaussian sampling
// over R easier]
class PolyRing {
public:
    PolyRing(uint deg) : deg_(deg) {}

    inline poly
    add(poly a, poly b) const
    {
      return reduce(a + b);
    }

    inline poly
    mul(poly a, poly b) const
    {
      return reduce(a * b);
    }

    inline poly
    reduce(poly a) const
    {
      return modpoly(a, deg_);
    }

    inline unsigned int
    monomial_degree() const
    {
      return deg_;
    }

private:
    unsigned int deg_; // n
};

// Represents a polynomial ring R_q = Z_q[x]/(f(x)), where
// f(x) = x^n + 1.
class RLWEField {
public:
    RLWEField(mpz_class q, unsigned int deg)
        : q_(q), deg_(deg) {}

    // OPT: all the below could be made faster

    inline poly
    add(poly a, poly b) const
    {
        return reduce(a + b);
    }

    inline poly
    mul(poly a, poly b) const
    {
        return reduce(a * b);
    }

    inline poly
    reduce(poly a) const
    {
        return modpoly(a, deg_) % q_;
    }

    inline const mpz_class &
    modulus() const
    {
        return q_;
    }

    inline unsigned int
    monomial_degree() const
    {
        return deg_;
    }

    poly
    sample(gmp_randclass &g) const
    {
        poly p;
        for (unsigned int i = 0; i < deg_ + 1; i++)
            p[i] = g.get_z_range(q_);
        return p;
    }

private:
    mpz_class q_;
    unsigned int deg_; // n
};

/* vim:set shiftwidth=4 ts=4 et: */
