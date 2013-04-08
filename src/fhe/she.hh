#pragma once

#include <cassert>
#include <cstdint>
#include <string>
#include <utility>

#include <math/field.hh>
#include <fhe/errordist.hh>
#include <util/util.hh>

// An implementation of a homomorphic encryption scheme described by
// Graepel et al - ML Confidential
template <typename Parameters>
class SHE {
public:

    static const unsigned int LogQ       = Parameters::LogQ;
    static const unsigned int LogT       = Parameters::LogT;
    static const unsigned int Sigma      = Parameters::Sigma;
    static const unsigned int D          = Parameters::D;
    static const unsigned int LogMsgBase = Parameters::LogMsgBase;

    typedef poly SK;
    typedef std::pair<poly, poly> PK;
    typedef std::pair<poly, poly> CT;

    SHE()
        : r_(D),
          rq_(mpz_class(1) << LogQ, D),
          rt_(mpz_class(1) << LogT, D),
          chi_(0 /* XXX: security parameter? */, Sigma, r_),
          g_(gmp_randinit_default)
    {}

    // keygen interface
    inline SK SKKeyGen();
    inline PK PKKeyGen(const SK &sk);

    // basic interface
    CT encrypt(const PK &pk, const mpz_class &m) const;
    mpz_class decrypt(const SK &sk, const CT &ct) const;

    // homomorphic interface
    CT add(const PK &pk, const CT &ct0, const CT &ct1) const;
    CT multiply(const PK &pk, const CT &ct0, const CT &ct1) const;

private:

    // encode message as a element of R_t.  the coefficients of the message
    // polynomial are simply the base 2^LogMsgBase representation of m (ML
    // Confidential uses LogMsgBase = 1, ie the binary representation, but this
    // seems wasteful)
    poly encode(const mpz_class &m) const;

    PolyRing r_;   // Z[x]/f(x)
    RLWEField rq_; // Z_q[x]/f(x)
    RLWEField rt_; // Z_t[x]/f(x) [for message]

    ErrorDist chi_;
    gmp_randclass g_;
};

template <typename P>
typename SHE<P>::SK
SHE<P>::SKKeyGen()
{
    return chi_.sample();
}

template <typename P>
typename SHE<P>::PK
SHE<P>::PKKeyGen(const SK &sk)
{
    poly e  = chi_.sample();
    poly a1 = rq_.sample(g_);
    poly a0 = rq_.reduce(-(a1 * sk + e));
    return std::make_pair(a0, a1);
}

template <typename P>
typename SHE<P>::CT
SHE<P>::encrypt(const PK &pk, const mpz_class &m) const
{
    poly mz = encode(m);
    assert_s(false, "unimpl");
    return CT();
}

template <typename P>
poly
SHE<P>::encode(const mpz_class &m) const
{
    static_assert(LogMsgBase >= 1,  "XX");
    static_assert(LogMsgBase <= 64, "YY");
    const unsigned long base = static_cast<unsigned long>(1) << LogMsgBase;
    const int sgn = mpz_sgn(m.get_mpz_t());
    mpz_class z(mpz_class_abs(m));
    poly ret;
    unsigned int i = 0;
    while (z != 0) {
        const unsigned long rem = mpz_fdiv_ui(z.get_mpz_t(), base);
        ret[i++] = sgn * rem;
        z = z >> LogMsgBase;
    }
    return ret;
}

struct default_she_parameters {
    static const unsigned int LogQ       = 128;
    static const unsigned int LogT       = 15;
    static const unsigned int Sigma      = 16;
    static const unsigned int D          = 4096;
    static const unsigned int LogMsgBase = 8;
};

typedef SHE<default_she_parameters> DefaultSHE;

/* vim:set shiftwidth=4 ts=4 et: */
