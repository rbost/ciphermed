/*
 * Copyright 2013-2015 Raphael Bost
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

#include <iostream>
#include <math/num_th_alg.hh>
#include <util/util.hh>
#include <NTL/ZZ.h>

using namespace std;

static void test_fact_generation(size_t m_bits)
{
    mpz_class m = 0;
    mpz_setbit(m.get_mpz_t(),m_bits);

    
    mpz_class p;

    gmp_randstate_t randstate;
    gmp_randinit_default(randstate);
    gmp_randseed_ui(randstate,time(NULL));

    ScopedTimer *t = new ScopedTimer("Test factorization generation");
    std::vector<mpz_class> fact = gen_rand_prime_with_factorization(m,&p,randstate,25);
    delete t;
    
    cout << "Prime generated is \n" << p << endl;
    cout << "Factors:\n";
    
    for (size_t i = 0; i < fact.size(); i++) {
        cout << fact[i] << endl;
    }
    
    cout << "\n" << m_bits << " bits queried\n";
    cout << "p is " << mpz_sizeinbase(p.get_mpz_t(),2) << " bits" << endl;
}

static void test_simple_safe_prime(size_t n_bits)
{
    gmp_randstate_t randstate;
    gmp_randinit_default(randstate);
    gmp_randseed_ui(randstate,time(NULL));
    ScopedTimer *t;

    /*
    t = new ScopedTimer("Test simple safe prime");
    mpz_class p = simple_safe_prime_gen(n_bits,randstate,25);
    delete t;
    
    cout << "Prime generated is \n" << 2*p+1 << endl;
    cout << "\n" << n_bits << " bits queried\n";
    cout << "p is " << mpz_sizeinbase(p.get_mpz_t(),2) << " bits" << endl;
*/
    cout << "\n\nWith NTL:" << endl;
    
    t = new ScopedTimer("NTL safe prime");
    NTL::ZZ q = NTL::GenGermainPrime_ZZ(n_bits+1);
    delete t;
    cout << "Prime generated is \n" << q << endl;
    cout << "q is " << NTL::NumBits(q) << " bits" << endl;

    mpz_class r;
    t = new ScopedTimer("Our safe prime");
    gen_germain_prime(r,n_bits,randstate);
    delete t;
    cout << "Prime generated is \n" << r << endl;
    cout << "q is " <<mpz_sizeinbase(r.get_mpz_t(),2) << " bits" << endl;

}

int main()
{
//    test_fact_generation(512);
    test_simple_safe_prime(512);
    
    return 0;
}