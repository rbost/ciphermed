#pragma once

/*
 * Distribution for sampling random errors.
 *
 */

#include <stdint.h>
#include <string>
#include <math/matrix.hh>
#include <math/field.hh>
#include <random>

/**
 * Implements a zero-mean gaussian distribution over R=Z[x]/(f(x)), with
 * standard deviation sigma. Assumes that f(x) = x^(2^d) + 1 for some d.
 */
class ErrorDist {
public:
    /* lambda = sec parameter lambda and mu, and the field Rq*/
    ErrorDist(uint lambda, uint sigma, const PolyRing &r);
    poly sample();

private:
    PolyRing r_;
    std::default_random_engine gen_; // not cryptographically secure
    std::normal_distribution<double> gauss_; // not discrete gaussian
};

