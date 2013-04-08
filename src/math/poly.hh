#pragma once

#include <stdint.h>
#include <assert.h>
#include <string>
#include <vector>

#include <math/mpz_class.hh>

class poly {
public:

    // default ctor is for zero polynomial
    poly() : coeffs_() {}
    explicit poly(const std::vector<mpz_class> &coeffs)
        : coeffs_(coeffs) {}
    explicit poly(std::vector<mpz_class> &&coeffs)
        : coeffs_(std::move(coeffs)) {}

    poly(const poly &p) = default;
    poly(poly &&p) = default;
    poly &operator=(const poly &p) = default;

    // accesses the i-th coefficient, where the 0-th coefficient is the
    // scalar coefficient
    inline mpz_class &
    operator[](size_t i)
    {
        if (i >= coeffs_.size())
            coeffs_.resize(i + 1);
        return coeffs_[i];
    }

    // unchecked
    inline const mpz_class &
    operator[](size_t i) const
    {
        return coeffs_[i];
    }

    inline size_t
    deg() const
    {
        if (coeffs_.empty())
            return 0;
        // XXX: check for non-zero degree?
        return coeffs_.size() - 1;
    }

    inline size_t
    size() const
    {
        return coeffs_.size();
    }

    inline const std::vector<mpz_class> &
    view() const
    {
        return coeffs_;
    }

    inline std::vector<mpz_class> &
    unsafe()
    {
        return coeffs_;
    }

    // P mod scalar q - note that we shift into the range (-q/2, q/2] when q is
    // even, and range [-(q-1)/2, (q-1)/2] when q is odd.
    poly modshift(const mpz_class &q) const;

private:
    // first coefficient is for power 0
    std::vector<mpz_class> coeffs_;
};

poly karatsuba(const poly & P, const poly & Q);

//P+Q
poly operator+(const poly & P, const poly & Q);
//TODO: these operators cause another copyuing of a poly in R = P + Q; if it
//matters, define add(R, P, Q);

//P*Q
poly operator*(const poly & P, const poly & Q);

// computes P mod x^n + 1
poly modpoly(const poly & P, uint n);

// P mod scalar q
poly operator%(const poly & P, const mpz_class & q);

poly operator-(const poly &P);

// P == Q
bool operator==(const poly &P, const poly & Q);

inline bool
operator!=(const poly &P, const poly &Q)
{
  return !operator==(P, Q);
}

std::ostream&
operator<<(std::ostream&, const poly & P);

//TODO: one way to speed up performance for some operations may be to have them
//do mod automatically so you work with small numbers (also avoid looping a
//second time for %q)
// also some values do not have to be mpz
// Not clear what optimizations matter right now.

/* vim:set shiftwidth=4 ts=4 et: */
