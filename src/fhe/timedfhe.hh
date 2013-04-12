#pragma once

#include <util/util.hh>
#include <math/mpz_class.hh>

// for debugging purposes
// assumes Impl has no-arg ctor
template <typename Impl>
class TimedFHE {
public:
    typedef typename Impl::SK SK;
    typedef typename Impl::PK PK;
    typedef typename Impl::CT CT;

    inline SK SKKeyGen() { return impl_.SKKeyGen(); }
    inline PK PKKeyGen(const SK &sk) { return impl_.PKKeyGen(sk); }

    CT
    encrypt(const PK &pk, const mpz_class &m)
    {
        ScopedTimer t("encrypt");
        return impl_.encrypt(pk, m);
    }

    mpz_class
    decrypt(const SK &sk, const CT &ct) const
    {
        ScopedTimer t("decrypt");
        return impl_.decrypt(sk, ct);
    }

    CT
    add(const CT &ct0, const CT &ct1) const
    {
        ScopedTimer t("add");
        return impl_.add(ct0, ct1);
    }

    CT multiply(const CT &ct0, const CT &ct1) const
    {
        ScopedTimer t("multiply");
        return impl_.multiply(ct0, ct1);
    }

    Impl &underlying() { return impl_; }

private:
    Impl impl_;
};

/* vim:set shiftwidth=4 ts=4 sts=4 et: */
