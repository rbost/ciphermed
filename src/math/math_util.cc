#include <gmpxx.h>
#include <math/mpz_class.hh>
#include <math/math_util.hh>
#include <vector>

using namespace std;

// Garner's Algorithm for CRT, cf. Handbook of Applied Cryptography - 14.71
void mpz_t_crt(mpz_t x, const mpz_ptr *v, const mpz_ptr *m, size_t s)
{
    mpz_t u;
    mpz_t C[s];
    size_t i, j;
    
    mpz_init(u);
    for (i=1; i<s; i++) {
        mpz_init(C[i]);
        mpz_set_ui(C[i], 1);
        for (j=0; j<i; j++) {
            mpz_invert(u, m[j], m[i]);
            mpz_mul(C[i], C[i], u);
            mpz_mod(C[i], C[i], m[i]);
        }
    }
    mpz_set(u, v[0]);
    mpz_set(x, u);
    for (i=1; i<s; i++) {
        mpz_sub(u, v[i], x);
        mpz_mul(u, u, C[i]);
        mpz_mod(u, u, m[i]);
        for (j=0; j<i; j++) {
            mpz_mul(u, u, m[j]);
        }
        mpz_add(x, x, u);
    }
    
    for (i=1; i<s; i++) mpz_clear(C[i]);
    mpz_clear(u);
}


mpz_class mpz_class_crt(const vector<mpz_class> &v, const vector<mpz_class> &m)
{
    mpz_class u,x;
    size_t s = m.size();
    vector<mpz_class> C(s);
    size_t i, j;
    
    for (i=1; i<s; i++) {
        C[i] = 1;
        for (j=0; j<i; j++) {
            mpz_class_invert(u,m[j],m[i]);
            C[i] *= u;
            C[i] %= m[i];
        }
    }
    u = v[0];
    x = u;
    
    for (i=1; i<s; i++) {
        u = v[i] - x;
        u *= C[i];
        u %= m[i];
        
        for (j=0; j<i; j++) {
            u *= m[j];
        }
        x+= u;
    }
    return x;
}
