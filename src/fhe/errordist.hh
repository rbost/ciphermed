#pragma once

/*
 * Distribution for sampling random errors.
 *
 */

#include <stdint.h>
#include <string>
#include <math/matrix.hh>
#include <math/field.hh>


class ErrorDist {
public:
    /* lambda = sec parameter lambda and mu, and the field Rq*/
    ErrorDist(uint lambda, uint mu, const RLWEField & Rq);
    poly sample();

private:
    mpz_class B;

};

