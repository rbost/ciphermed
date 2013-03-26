#pragma once

#include <stdint.h>
#include <assert.h>
#include <string>
#include <vector>

#include <stdio.h>	
#include <gmpxx.h>

#include <math/poly.hh>

class RLWEField {
    RLWEField(mpz_class _q, int _degree): q(_q), degree(_degree) {}

    add(poly a, poly b);
    mul(poly a, poly b);
    

private:
    mpz_class q;
    int degree;
}

