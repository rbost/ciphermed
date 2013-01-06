#pragma once

#include <stdint.h>
#include <assert.h>
#include <string>
#include <vector>

#include <gmpxx.h>


struct poly {

    poly(std::vector<mpz_class> * c, bool makecopy = false);
    poly() {coeffs = NULL;};
    poly(const poly & p);
    
    // first coefficient is for power 0
    std::vector<mpz_class> * coeffs;
    mpz_class & operator[](uint i) const;
     
    uint deg() const {return coeffs->size()-1;}
    uint size() const {return coeffs->size();}

    poly copy() const;
    
    ~poly();
};


//P+Q
poly operator+(const poly & P, const poly & Q);

//P*Q
poly operator*(const poly & P, const poly & Q);

// computes P mod x^n + 1
poly modpoly(const poly & P, uint n);

// P mod scalar q
poly operator%(const poly & P, const mpz_class & q);

// P == Q
bool operator==(const poly &P, const poly & Q);

std::ostream&
operator<<(std::ostream&, const poly & P);

//TODO: one way to speed up performance for some operations may be to have them
//do mod automatically so you work with small numbers (also avoid looping a
//second time for %q)
// also some values do not have to be mpz
// Not clear what optimizations matter right now.

