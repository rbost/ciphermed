#pragma once

#include <cassert>
#include <cstdint>
#include <string>
#include <utility>

#include <math/field.hh>
#include <math/util.hh>
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
    static const unsigned int LogD       = Parameters::LogD;
    static const unsigned int LogMsgBase = Parameters::LogMsgBase;

    typedef poly SK;
    typedef std::pair<poly, poly> PK;
    typedef std::vector<poly> CT;

    SHE()
        : q_(mpz_class(1) << LogQ),
          t_(mpz_class(1) << LogT),
          d_(1UL << LogD),
          r_(d_),
          rq_(q_, d_),
          rt_(t_, d_),
          delta_(mpz_class(1) << (LogQ - LogT)),
          chi_(0 /* XXX: security parameter? */, Sigma, r_),
          g_(gmp_randinit_default)
    {}

    // keygen interface
    inline SK SKKeyGen();
    inline PK PKKeyGen(const SK &sk);

    // basic interface
    CT encrypt(const PK &pk, const mpz_class &m);
    mpz_class decrypt(const SK &sk, const CT &ct) const;

    // homomorphic interface
    CT add(const CT &ct0, const CT &ct1) const;
    CT multiply(const CT &ct0, const CT &ct1) const;

    void SanityCheck();

private:

    // sanity checks
    static_assert(LogQ > LogT, "XX");
    static_assert(LogD <= sizeof(unsigned long) * 8, "YY");
    static_assert(LogT >= LogMsgBase, "ZZ");

    // encode message as a element of R_t.  the coefficients of the message
    // polynomial are simply the base 2^LogMsgBase representation of m (ML
    // Confidential uses LogMsgBase = 1, ie the binary representation, but this
    // seems wasteful)
    poly encode(const mpz_class &m) const;
    mpz_class decode(const poly &p) const;

    mpz_class q_;
    mpz_class t_;
    unsigned long d_;

    PolyRing r_;   // Z[x]/f(x)
    RLWEField rq_; // Z_q[x]/f(x)
    RLWEField rt_; // Z_t[x]/f(x) [for message]
    mpz_class delta_; // floor(q/t)

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
SHE<P>::encrypt(const PK &pk, const mpz_class &m)
{
    poly mz = encode(m);
    poly u = chi_.sample();
    poly f = chi_.sample();
    poly g = chi_.sample();
    poly c0 = rq_.reduce(pk.first * u + g + delta_ * mz);
    poly c1 = rq_.reduce(pk.second * u + f);
    return {c0, c1};
}

template <typename P>
mpz_class
SHE<P>::decrypt(const SK &sk, const CT &ct) const
{
    poly top = t_ * rq_.reduce(naive_polyeval(ct, sk));
    return decode(rt_.reduce(top.nearest_div(q_)));
}

template <typename P>
typename SHE<P>::CT
SHE<P>::add(const CT &ct0, const CT &ct1) const
{
    CT ret(ct0);
    ret.resize(std::max(ct0.size(), ct1.size()));
    for (size_t i = 0; i < ct1.size(); i++)
        ret[i] = rq_.reduce(ret[i] + ct1[i]);
    return ret;
}

template <typename P>
typename SHE<P>::CT
SHE<P>::multiply(const CT &ct0, const CT &ct1) const
{
    CT prod = naive_multiply(ct0, ct1);
    for (size_t i = 0; i < prod.size(); i++) {
        poly p = prod[i] * t_;
        prod[i] = p.nearest_div(q_);
    }
    return prod;
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

template <typename P>
mpz_class
SHE<P>::decode(const poly &p) const
{
    // reduce the polynomial in R_t to a message in Z
    mpz_class ret;
    mpz_class x(1);
    for (size_t i = 0; i < p.size(); i++, x <<= LogMsgBase)
        ret += x * p[i];
    return ret;
}

template <typename P>
void
SHE<P>::SanityCheck()
{
    mpz_class m0(123456789);
    auto mz0 = encode(m0);
    auto md0 = decode(mz0);
    assert_s(m0 == md0, "encode/decode failed");
}

struct default_she_parameters {
    static const unsigned int LogQ       = 128;
    static const unsigned int LogT       = 15;
    static const unsigned int Sigma      = 16;
    static const unsigned int LogD       = 12;
    static const unsigned int LogMsgBase = 8;
};

typedef SHE<default_she_parameters> DefaultSHE;

/* vim:set shiftwidth=4 ts=4 sts=4 et: */
