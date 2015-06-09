/*
 * Copyright 2013-2015 Raphael Bost, Raluca Ada Popa
 *
 * This file is part of ciphermed.

 *  ciphermed is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 * 
 *  ciphermed is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 * 
 *  You should have received a copy of the GNU General Public License
 *  along with ciphermed.  If not, see <http://www.gnu.org/licenses/>. 2
 *
 */

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




FixedPointExp::FixedPointExp(mpz_t& g, mpz_t& p, int fieldsize)
{
    mpz_init(m_g);
    mpz_init(m_p);
    mpz_set(m_g, g);
    mpz_set(m_p, p);
    
    
    m_isInitialized = false;
    m_numberOfElements = fieldsize;
    m_table = NULL;
    init();
}

FixedPointExp::~FixedPointExp() {
    if (m_isInitialized) {
        delete[] m_table;
    }
}

void FixedPointExp::init() {
    
    m_table = (mpz_t*) malloc(sizeof(mpz_t) * m_numberOfElements);
    for(int i = 0; i < m_numberOfElements; i++)
    {
        mpz_init(m_table[i]);
    }
    
    // m_table[0] = m_g;
    mpz_set(m_table[0], m_g);
    for (unsigned u=1; u<m_numberOfElements; ++u) {
        mpz_mul(m_table[u], m_table[u-1], m_table[u-1]);
        mpz_mod(m_table[u], m_table[u], m_p);
        //mpz_powm_ui(m_table[u], m_table[u-1], 2, m_p);
        //SqrMod(m_table[u], m_table[u-1], m_p);
    }
    m_isInitialized = true;
    
    //   for (unsigned u=0; u<m_numberOfElements; ++u) {
    //     cout << "table[" << u << "] = " << m_table[u] << endl;
    //     ZZ res;
    //     ZZ ex = power_ZZ(2,u);
    //     PowerMod(res, m_g, ex, m_p);
    //     cout << "    (Should be = " << res << ")" << endl;
    //   }
}

void FixedPointExp::powerMod(mpz_t& result, mpz_t& e) {
    mpz_set_ui(result, 1);
    for (unsigned u=0; u<m_numberOfElements; u++) {
        //if (bit(e,u)) {
        if(mpz_tstbit(e, u))
        {
            mpz_mul(result, result, m_table[u]);
            mpz_mod(result, result, m_p);
        }
        //MulMod(result, result, m_table[u], m_p);
    }
}

