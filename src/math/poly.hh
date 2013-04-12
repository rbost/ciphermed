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

    // zero polynomial, but allocate zeros ahead of time
    explicit poly(unsigned long sz) : coeffs_(sz) {}

    explicit poly(const std::vector<mpz_class> &coeffs)
        : coeffs_(coeffs) {}

    explicit poly(std::vector<mpz_class> &&coeffs)
        : coeffs_(std::move(coeffs)) {}

    poly(const poly &p) = default;
    poly(poly &&p) = default;
    poly &operator=(const poly &p) = default;

    // accesses the i-th coefficient, where the 0-th coefficient is the
    // scalar coefficient. safe regardless of i, but larger i's will cause
    // the internal representation to grow
    inline mpz_class &
    operator[](size_t i)
    {
        if (i >= coeffs_.size())
            coeffs_.resize(i + 1);
        return coeffs_[i];
    }

    // precondition: i < size()
    inline const mpz_class &
    operator[](size_t i) const
    {
        assert(i < coeffs_.size());
        return coeffs_[i];
    }

    // safe even if i >= size()
    inline const mpz_class &
    element(size_t i) const
    {
        static const mpz_class s_zero;
        if (i >= coeffs_.size())
            return s_zero;
        return coeffs_[i];
    }

    inline size_t
    deg() const
    {
        ssize_t i = coeffs_.size() - 1;
        while (i >= 0 && coeffs_[i] == 0)
            i--;
        return (i == -1) ? 0 : i;
    }

    inline bool
    empty() const
    {
        return coeffs_.empty();
    }

    // size doesn't account for leading zeros (size() >= deg())
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

    inline void
    reserve(size_t sz)
    {
        coeffs_.reserve(sz);
    }

    inline void
    resize(size_t sz)
    {
        coeffs_.resize(sz);
    }

    mpz_class
    eval(const mpz_class &x) const;

    inline mpz_class
    operator()(const mpz_class &x) const
    {
        return eval(x);
    }

    // P mod q - note that we shift into the range (-q/2, q/2] when q is
    // even, and range [-(q-1)/2, (q-1)/2] when q is odd.
    //
    // precondition: q > 0
    poly modshift(const mpz_class &q) const;

    // coefficient-wise nearest_div
    poly nearest_div(const mpz_class &q) const;

    // compound operators -- not efficient

    poly &operator+=(const poly &q);
    poly &operator*=(const poly &q);

private:
    // first coefficient is for power 0
    std::vector<mpz_class> coeffs_;
};

poly karatsuba(const poly & P, const poly & Q);

poly karatsuba2(const poly &p, const poly &q);

//P+Q
poly operator+(const poly & P, const poly & Q);
//TODO: these operators cause another copyuing of a poly in R = P + Q; if it
//matters, define add(R, P, Q);

//P*Q
poly operator*(const poly & P, const poly & Q);

// P*q
poly operator*(const poly &P, const mpz_class &q);

// p*Q
inline poly
operator*(const mpz_class &p, const poly &Q)
{
    return operator*(Q, p);
}

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

/* vim:set shiftwidth=4 ts=4 sts=4 et: */
