#pragma once

#include <cassert>
#include <cstdint>
#include <string>
#include <utility>
#include <tuple>

#include <math/field.hh>
#include <math/util.hh>
#include <fhe/errordist.hh>
#include <util/util.hh>

// An implementation of a homomorphic encryption scheme described by
// Graepel et al - ML Confidential. This implementation performs
// relinearization of ciphertexts after mulitplication.
template <typename Parameters>
class SHERelin {
public:

    static const unsigned int LogP           = Parameters::LogP;
    static const unsigned int LogQ           = Parameters::LogQ;
    static const unsigned int LogT           = Parameters::LogT;
    static const unsigned int Sigma          = Parameters::Sigma;
    static const constexpr double SigmaPrime = Parameters::SigmaPrime;
    static const unsigned int LogD           = Parameters::LogD;
    static const unsigned int LogMsgBase     = Parameters::LogMsgBase;

    typedef poly SK;
    typedef std::tuple<poly, poly, poly, poly> PK;
    typedef std::pair<poly, poly> CT;

    SHERelin()
        : p_(mpz_class(1) << LogP),
          q_(mpz_class(1) << LogQ),
          pq_(p_ * q_),
          t_(mpz_class(1) << LogT),
          d_(1UL << LogD),
          r_(d_),
          rpq_(pq_, d_),
          rq_(q_, d_),
          rt_(t_, d_),
          r2_(mpz_class(2), d_),
          delta_(mpz_class(1) << (LogQ - LogT)),
          chi_(0 /* XXX: security parameter? */, Sigma, r_),
          //where is the security parameter in chi?
          //also it should get q or a bound as a paramters?
          chiprime_(0 /* XXX: security parameter? */, SigmaPrime, r_),
          g_(gmp_randinit_default)
    {
        const double b(Sigma);
        const double bk(SigmaPrime);
        const double alpha(3.758);
        const double k(double(LogP)/double(LogQ) + 1.0);
        const double sqrtk = sqrt(k);
        const double rhs = pow(alpha, 1.0 - sqrtk) * pow(pow(2.0, LogQ), k - sqrtk) * pow(b, sqrtk);
        if (bk <= rhs)
            std::cerr << "{WARNING}: B_k=" << bk << ", rhs=" << rhs << std::endl;
    }

    // keygen interface
    inline SK SKKeyGen();
    inline PK PKKeyGen(const SK &sk);

    // basic interface
    CT encrypt(const PK &pk, const mpz_class &m);
    mpz_class decrypt(const SK &sk, const CT &ct) const;

    // homomorphic interface
    CT add(const PK &pk, const CT &ct0, const CT &ct1) const;
    CT multiply(const PK &pk, const CT &ct0, const CT &ct1) const;

    void SanityCheck();

private:

    // sanity checks
    static_assert(LogP >= (3 * LogQ), "XX");
    static_assert(LogQ > LogT, "XX");
    static_assert(LogD <= sizeof(unsigned long) * 8, "YY");
    static_assert(LogT >= LogMsgBase, "ZZ");
    static_assert(double(Sigma) != SigmaPrime, "bad security");

    // encode message as a element of R_t.  the coefficients of the message
    // polynomial are simply the base 2^LogMsgBase representation of m (ML
    // Confidential uses LogMsgBase = 1, ie the binary representation, but this
    // seems wasteful)
    poly encode(const mpz_class &m) const;
    mpz_class decode(const poly &p) const;
    mpz_class p_;
    mpz_class q_;
    mpz_class pq_;
    mpz_class t_;
    unsigned long d_;

    PolyRing r_;   // Z[x]/f(x)
    RLWEField rpq_; // Z_{pq}[x]/f(x)
    RLWEField rq_; // Z_q[x]/f(x)
    RLWEField rt_; // Z_t[x]/f(x) [for message]
    RLWEField r2_; // Z_2[x]/f(x)
    mpz_class delta_; // floor(q/t)

    ErrorDist chi_;
    ErrorDist chiprime_;
    gmp_randclass g_;
};

template <typename P>
typename SHERelin<P>::SK
SHERelin<P>::SKKeyGen()
{
    return r2_.sample(g_);
}

template <typename P>
typename SHERelin<P>::PK
SHERelin<P>::PKKeyGen(const SK &sk)
{
    poly e  = chi_.sample();
    poly a1 = rq_.sample(g_);
    poly a0 = rq_.reduce(-(a1 * sk + e));

    poly a3 = rpq_.sample(g_);
    poly e1 = chiprime_.sample();
    poly a2 = rpq_.reduce(-(a3 * sk + e1) + p_ * (sk * sk));
    return std::make_tuple(a0, a1, a2, a3);
}

template <typename P>
typename SHERelin<P>::CT
SHERelin<P>::encrypt(const PK &pk, const mpz_class &m)
{
    poly mz = encode(m);
    poly u = r2_.sample(g_);
    poly f = chi_.sample();
    poly g = chi_.sample();
    poly c0 = rq_.reduce(std::get<0>(pk) * u + g + delta_ * mz);
    poly c1 = rq_.reduce(std::get<1>(pk) * u + f);
    return std::make_pair(c0, c1);
}

template <typename P>
mpz_class
SHERelin<P>::decrypt(const SK &sk, const CT &ct) const
{
    poly top = t_ * rq_.reduce(ct.first + ct.second * sk);
    return decode(rt_.reduce(top.nearest_div(q_)));
}

template <typename P>
typename SHERelin<P>::CT
SHERelin<P>::add(const PK &pk, const CT &ct0, const CT &ct1) const
{
    return std::make_pair(ct0.first + ct1.first, ct0.second + ct1.second);
}

template <typename P>
typename SHERelin<P>::CT
SHERelin<P>::multiply(const PK &pk, const CT &ct0, const CT &ct1) const
{
    poly c0top = t_ * (ct0.first * ct1.first);
    poly c0 = rq_.reduce(c0top.nearest_div(q_));

    poly c1top = t_ * (ct0.first * ct1.second + ct0.second * ct1.first);
    poly c1 = rq_.reduce(c1top.nearest_div(q_));

    poly c2top = t_ * (ct0.second * ct1.second);
    poly c2 = rq_.reduce(c2top.nearest_div(q_));

    poly c20 = rq_.reduce((c2 * std::get<2>(pk)).nearest_div(p_));
    poly c21 = rq_.reduce((c2 * std::get<3>(pk)).nearest_div(p_));

    return std::make_pair(rq_.reduce(c0 + c20), rq_.reduce(c1 + c21));
}

template <typename P>
poly
SHERelin<P>::encode(const mpz_class &m) const
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
        assert(rem <= (t_/2));
        ret[i++] = sgn * rem;
        z = z >> LogMsgBase;
    }
    assert(ret.deg() <= d_);
    return ret;
}

template <typename P>
mpz_class
SHERelin<P>::decode(const poly &p) const
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
SHERelin<P>::SanityCheck()
{
    mpz_class m0(123456789);
    auto mz0 = encode(m0);
    auto md0 = decode(mz0);
    assert_s(m0 == md0, "encode/decode failed");
}

struct default_she_relin_parameters {
    static const unsigned int LogP           = 384;
    static const unsigned int LogQ           = 128;
    static const unsigned int LogT           = 15;
    static const unsigned int Sigma          = 16;
    static constexpr const double SigmaPrime = 1e79;
    static const unsigned int LogD           = 12;
    static const unsigned int LogMsgBase     = 2;
};

typedef SHERelin<default_she_relin_parameters> DefaultSHERelin;

/* vim:set shiftwidth=4 ts=4 sts=4 et: */
