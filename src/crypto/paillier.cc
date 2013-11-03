#include <assert.h>
#include <crypto/paillier.hh>
#include <math/util_gmp_rand.h>
#include <math/math_util.hh>
#include <math/num_th_alg.hh>

using namespace std;
using namespace NTL;

/*
 * Public-key operations
 */

Paillier::Paillier(const vector<mpz_class> &pk, gmp_randstate_t state)
    : n(pk[0]), g(pk[1]),
      nbits(mpz_sizeinbase(n.get_mpz_t(),2)), n2(n*n), good_generator(g == n+1)
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

        if (good_generator) {
            // g = n+1 -> we can avoid an exponentiation
            return ((1+plaintext*n)*rn) %n2;
        }
        
        return (mpz_class_powm(g,plaintext,n2) * rn) % n2;
    } else {
        mpz_class r;
        mpz_urandomm(r.get_mpz_t(),_randstate,n.get_mpz_t());

        if (good_generator) {
            r = mpz_class_powm(r,n,n2);
            // g = n+1 -> we can avoid an exponentiation
            return ((1+plaintext*n)*r) %n2;
        }
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
    // here, we should multiply by r when r is coprime with n
    // to save time, as this will not happen with negligible probability,
    // we don't test this property
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
    find_crt_factors();
}

void Paillier_priv::find_crt_factors()
{
    mpz_class d, u_p2,u_q2;
    mpz_gcdext(d.get_mpz_t(),u_p2.get_mpz_t(),u_q2.get_mpz_t(),p2.get_mpz_t(),q2.get_mpz_t());
    
    assert(d == 1);
    e_p2 = (q2*u_q2) %n2;
    e_q2 = (p2*u_p2) %n2;
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
//        do {
//            g++;
//        } while (mpz_class_gcd(L(mpz_class_powm(g, lambda, n*n), n), n) != 1);
        g = n+1; // use a well chosen generator to speed up encryption (approx. x2)
    }

    return { p, q, g, a };
}

mpz_class
Paillier_priv::encrypt(const mpz_class &plaintext)
{
    if (fast) {
        // a != 0
        return encrypt(plaintext);
    }
    mpz_class rn;

    auto i = rqueue.begin();
    if (i != rqueue.end()) {
        rqueue.pop_front();
        rn = *i;
        
        mpz_class c;
        mpz_class c_p;
        mpz_class c_q;
        if (good_generator) {
            // g = n+1 -> we can avoid an exponentiation
            c_p = ((1+plaintext*n)) % p2;
            c_q = ((1+plaintext*n)) % q2;

        }else{
            c_p = mpz_class_powm(g,plaintext, p2);
            c_q = mpz_class_powm(g,plaintext, q2);
        }
        c = mpz_class_crt_2(c_p,c_q,p2,q2);

        return (c*rn) %n2;
    } else {
        mpz_class r;
        mpz_urandomm(r.get_mpz_t(),_randstate,n.get_mpz_t());

        mpz_class r_p,r_q;
        r_p = mpz_class_powm(r,n,p2);
        r_q = mpz_class_powm(r,n,q2);
        mpz_class c_p;
        mpz_class c_q;

        if (good_generator) {
            // g = n+1 -> we can avoid an exponentiation
            c_p = ((1+plaintext*n)*r_p) % p2;
            c_q = ((1+plaintext*n)*r_q) % q2;
            
            
            //            return ((1+plaintext*n)*r) %n2;
        }else{
            c_p = mpz_class_powm(g,plaintext, p2)*r_p %p2;
            c_q = mpz_class_powm(g,plaintext, q2)*r_q %q2;
        }

        // g = n+1 -> we can avoid an exponentiation
        return mpz_class_crt_2(c_p,c_q,p2,q2);
    }

}

mpz_class
Paillier_priv::fast_encrypt_precompute(const mpz_class &plaintext)
{
    if (fast) {
        // a != 0
        return encrypt(plaintext);
    }
    mpz_class rn;
    
    auto i = rqueue.begin();
    if (i != rqueue.end()) {
        rqueue.pop_front();
        rn = *i;
        
        mpz_class c;
        mpz_class c_p;
        mpz_class c_q;
        if (good_generator) {
            // g = n+1 -> we can avoid an exponentiation
            c_p = ((1+plaintext*n)) % p2;
            c_q = ((1+plaintext*n)) % q2;
        }else{
            c_p = mpz_class_powm(g,plaintext, p2);
            c_q = mpz_class_powm(g,plaintext, q2);
        }
        c =  (c_p*e_p2 + c_q*e_q2) % n2;
        
        return (c*rn) %n2;
    } else {
        mpz_class r;
        mpz_urandomm(r.get_mpz_t(),_randstate,n.get_mpz_t());
        
        mpz_class r_p,r_q;
        r_p = mpz_class_powm(r,n,p2);
        r_q = mpz_class_powm(r,n,q2);
        mpz_class c_p;
        mpz_class c_q;
        
        if (good_generator) {
            // g = n+1 -> we can avoid an exponentiation
            c_p = ((1+plaintext*n)*r_p) % p2;
            c_q = ((1+plaintext*n)*r_q) % q2;
            
        }else{
            c_p = mpz_class_powm(g,plaintext, p2)*r_p %p2;
            c_q = mpz_class_powm(g,plaintext, q2)*r_q %q2;
        }
        
        // g = n+1 -> we can avoid an exponentiation
        
        return (c_p*e_p2 + c_q*e_q2) % n2;
    }
    
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

Paillier_priv_fast::Paillier_priv_fast(const std::vector<mpz_class> &sk, gmp_randstate_t state)
: Paillier_priv({sk[0],sk[1],sk[2],0},state), g_star_(sk[3]), phi_n((p-1)*(q-1)), phi_n2(phi_n*n), phi_n2_bits(mpz_sizeinbase(phi_n2.get_mpz_t(),2))
{
    assert(sk.size() == 4);
    precompute_powers();
}

void Paillier_priv_fast::precompute_powers()
{
    // find the number of elements to precompute
    unsigned int bits = max(mpz_sizeinbase(p2.get_mpz_t(),2),mpz_sizeinbase(q2.get_mpz_t(),2));
    g_star_powers_p_ = vector<mpz_class>(phi_n2_bits);
    g_star_powers_p_[0] = g_star_ % p2;
    g_star_powers_q_ = vector<mpz_class>(phi_n2_bits);
    g_star_powers_q_[0] = g_star_ % q2;
    
    for (size_t i = 1; i < bits; i++) {
        g_star_powers_p_[i] = (g_star_powers_p_[i-1]*g_star_powers_p_[i-1]) %p2;
        g_star_powers_q_[i] = (g_star_powers_q_[i-1]*g_star_powers_q_[i-1]) %q2;
    }
    
}

mpz_class Paillier_priv_fast::compute_g_star_power(const mpz_class &x)
{
    mpz_class y_p = x % ((p-1)*p);
    mpz_class y_q = x % ((q-1)*q);
    mpz_class v_p = 1, v_q = 1;

    unsigned int bits = max(mpz_sizeinbase(p2.get_mpz_t(),2),mpz_sizeinbase(q2.get_mpz_t(),2));

    for (size_t i = 0; i < bits; i++) {
        if (mpz_tstbit(y_p.get_mpz_t(), i)) {
            v_p = (v_p * g_star_powers_p_[i]) %p2;
        }
        if (mpz_tstbit(y_q.get_mpz_t(), i)) {
            v_q = (v_q * g_star_powers_q_[i]) %q2;
        }

    }
    
    return mpz_class_crt_2(v_p,v_q,p2,q2);
}

mpz_class Paillier_priv_fast::encrypt(const mpz_class &plaintext)
{
    mpz_class r_prime;
    mpz_urandomm(r_prime.get_mpz_t(),_randstate,phi_n.get_mpz_t());
    
    mpz_class r = compute_g_star_power(r_prime*n);
    
    mpz_class c_p;
    mpz_class c_q;
    
    // g = n+1 -> we can avoid an exponentiation
    c_p = ((1+plaintext*n)) % p2;
    c_q = ((1+plaintext*n)) % q2;
        
    mpz_class c =  mpz_class_crt_2(c_p,c_q,p2,q2);
    
    return (c*r %n2);
}


vector<mpz_class> Paillier_priv_fast::keygen(gmp_randstate_t state, uint nbits)
{
    mpz_class p, q, n, g, g_star;
    int error = 40;
    do {
        gen_germain_prime(p,nbits/2,state, error);
        gen_germain_prime(q,nbits/2,state, error);
        n = p*q;
    } while ((nbits != (uint) mpz_sizeinbase(n.get_mpz_t(),2)) || p == q);
    
    if (p > q)
        swap(p, q);
    
    mpz_class lambda = LCM(p-1, q-1);
    g = n+1;
    
    // find a generator for Z^*_n
    mpz_class g_p, g_q;
    g_p = get_generator_for_cyclic_group(p,state);
    g_q = get_generator_for_cyclic_group(q,state);
    
    g_star = mpz_class_crt_2(g_p,g_q,p,q);
    
    return {p, q, g, g_star};
}
