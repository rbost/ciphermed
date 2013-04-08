#pragma once

#include <stdint.h>
#include <string>
#include <utility>
#include <math/matrix.hh>

// An implementation of a homomorphic encryption scheme described by
// Graepel et al - ML Confidential
template <unsigned int LogQ,
          unsigned int LogT,
          unsigned int Sigma,
          unsigned int D>
class SHE {
public:
    typedef poly SK;
    typedef std::pair<poly, poly> PK;

    inline SK
    SKKeyGen() const
    {
        return chi_.sample();
    }

    inline PK
    PKKeyGen(const SK &sk) const
    {
        poly err = chi_.sample();
        poly a1 = rq_.sample(g_);
        poly a0 = rq_.reduce(-(a1 * sk + e))
        return std::make_pair(a0, a1);
    }

private:
    PolyRing r_;   // Z[x]/f(x)
    RLWEField rq_; // Z_q[x]/f(x)

    ErrorDist chi_;
    gmp_randclass g_;
};

typedef SHE<128, 15, 16, 4096> DefaultSHE;

/* vim:set shiftwidth=4 ts=4 et: */
