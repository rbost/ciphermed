#include <assert.h>
#include <crypto/paillier.hh>
#include <math/util_gmp_rand.h>
#include <math/math_util.hh>

using namespace std;
using namespace NTL;

/*
 * Public-key operations
 */

Paillier::Paillier(const vector<mpz_class> &pk, gmp_randstate_t state)
    : n(pk[0]), g(pk[1]),
      nbits(mpz_sizeinbase(n.get_mpz_t(),2)), n2(n*n)
{
    assert(pk.size() == 2);
    gmp_randinit_set(_randstate, state);
}

void
Paillier::rand_gen(size_t niter, size_t nmax)
{
    if (rqueue.size() >= nmax)
        niter = 0;
    else
        niter = min(niter, nmax - rqueue.size());

    mpz_class r;
    for (uint i = 0; i < niter; i++) {
        mpz_urandomm(r.get_mpz_t(),_randstate,n.get_mpz_t());

        rqueue.push_back(mpz_class_powm(g,n*r,n2));
    }
}

mpz_class
Paillier::encrypt(const mpz_class &plaintext)
{
    auto i = rqueue.begin();
    if (i != rqueue.end()) {
        mpz_class rn = *i;
        rqueue.pop_front();

        return (mpz_class_powm(g,plaintext,n2) * rn) % n2;
    } else {
        mpz_class r;
        mpz_urandomm(r.get_mpz_t(),_randstate,n.get_mpz_t());

        return mpz_class_powm(g,plaintext + n*r, n2);
    }
}

mpz_class
Paillier::add(const mpz_class &c0, const mpz_class &c1) const
{
    return (c0*c1) % n2;
}

mpz_class
Paillier::sub(const mpz_class &c0, const mpz_class &c1) const
{
    return add(c0,constMult(-1,c1));
}

mpz_class
Paillier::constMult(const mpz_class &m, const mpz_class &c) const
{
    return mpz_class_powm(c, m, n2);
}

mpz_class
Paillier::constMult(long m, const mpz_class &c) const
{
    return mpz_class_powm(c, m, n2);
}

mpz_class
Paillier::scalarize(const mpz_class &c)
{
    mpz_class r;
    mpz_urandomm(r.get_mpz_t(),_randstate,n.get_mpz_t());
    return constMult(r,c);
}

/*
 * Private-key operations
 */

static inline mpz_class
L(const mpz_class &u, const mpz_class &n)
{
    return (u - 1) / n;
}

static inline mpz_class
Lfast(const mpz_class &u, const mpz_class &ninv, const mpz_class &two_n, const mpz_class &n)
{
    return (((u - 1) * ninv) % two_n) % n;
}

static inline mpz_class
LCM(const mpz_class &a, const mpz_class &b)
{
    return (a * b) / mpz_class_gcd(a, b);
}

Paillier_priv::Paillier_priv(const vector<mpz_class> &sk, gmp_randstate_t state)
    : Paillier({sk[0]*sk[1], sk[2]},state), p(sk[0]), q(sk[1]), a(sk[3]),
      fast(a != 0),
      p2(p * p), q2(q * q),
      two_p(mpz_class_ui_pow_ui(2, mpz_sizeinbase(p.get_mpz_t(),2))),
      two_q(mpz_class_ui_pow_ui(2, mpz_sizeinbase(q.get_mpz_t(),2))),
      pinv(mpz_class_invert(p, two_p)),
      qinv(mpz_class_invert(q, two_q)),
      hp(mpz_class_invert(Lfast(mpz_class_powm(g % p2, fast ? a : (p-1), p2),
                      pinv, two_p, p), p)),
      hq(mpz_class_invert(Lfast(mpz_class_powm(g % q2, fast ? a : (q-1), q2),
                      qinv, two_q, q), q))
{
    assert(sk.size() == 4);                 
}

std::vector<mpz_class>
Paillier_priv::keygen(gmp_randstate_t state, uint nbits, uint abits)
{
    mpz_class p, q, n, g, a;

    mpz_class cp, cq;
    do {
        if (abits) {
            mpz_random_prime_len(a.get_mpz_t(), state, abits,40);

            mpz_urandom_len(cp.get_mpz_t(), state, nbits/2-abits);
            mpz_urandom_len(cq.get_mpz_t(), state, nbits/2-abits);

            p = a * cp + 1;
            while (mpz_probab_prime_p(p.get_mpz_t(),40) == 0)
                p += a;

            q = a * cq + 1;
            while (mpz_probab_prime_p(q.get_mpz_t(),40) == 0)
                q += a;
        } else {
            a = 0;
            mpz_random_prime_len(p.get_mpz_t(), state, nbits/2,40);
            mpz_random_prime_len(q.get_mpz_t(), state, nbits/2,40);
        }
        n = p * q;
    } while ((nbits != (uint) mpz_sizeinbase(n.get_mpz_t(),2)) || p == q);

    if (p > q)
        swap(p, q);

    mpz_class lambda = LCM(p-1, q-1);

    if (abits) {
        g = mpz_class_powm(2, lambda / a, n);
    } else {
        g = 1;
        do {
            g++;
        } while (mpz_class_gcd(L(mpz_class_powm(g, lambda, n*n), n), n) != 1);
    }

    return { p, q, g, a };
}

mpz_class
Paillier_priv::decrypt(const mpz_class &ciphertext) const
{
    mpz_class mp = (Lfast(mpz_class_powm(ciphertext % p2, fast ? a : (p-1), p2),
                   pinv, two_p, p) * hp) % p;
    mpz_class mq = (Lfast(mpz_class_powm(ciphertext % q2, fast ? a : (q-1), q2),
                   qinv, two_q, q) * hq) % q;

    mpz_class m;
    m = mpz_class_crt_2(mp,mq,p,q);
    
    if (m < 0)
        return (m + n);
    
    return m;
}
