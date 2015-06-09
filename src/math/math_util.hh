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

#pragma once

#include <vector>

#include <gmpxx.h>

inline int mpz_class_probab_prime_p(const mpz_class &n, int reps)
{
    return mpz_probab_prime_p(n.get_mpz_t(),reps);
}

// v: residues, m: moduli, s: number of elements
void mpz_t_crt(mpz_t x, const mpz_ptr *v, const mpz_ptr *m, size_t s);

mpz_class mpz_class_crt(const std::vector<mpz_class> &v, const std::vector<mpz_class> &m);

inline mpz_class mpz_class_crt_2(const mpz_class &v1, const mpz_class &v2, const mpz_class &m1, const mpz_class &m2)
{
    return mpz_class_crt({v1,v2},{m1,m2});
}

class FixedPointExp {
public:
    
    FixedPointExp(mpz_t& g, mpz_t& p, int fieldsize);
    ~FixedPointExp();
    
public:
    void powerMod(mpz_t& result, mpz_t& e);
    
private:
    //create table
    void init();
    
private:
    mpz_t m_p;
    mpz_t m_g;
    bool m_isInitialized;
    unsigned m_numberOfElements;
    mpz_t* m_table;
};


