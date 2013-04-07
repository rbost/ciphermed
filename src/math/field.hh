#pragma once

#include <stdint.h>
#include <assert.h>
#include <string>
#include <vector>

#include <stdio.h>
#include <gmpxx.h>

#include <math/poly.hh>

class RLWEField {
    RLWEField(mpz_class _q, int degree): q(_q), deg(degree) {}

    static poly add(poly a, poly b);
    static poly mul(poly a, poly b);

private:
    mpz_class q;
    int deg;
};

