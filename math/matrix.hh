#pragma once

#include <stdint.h>
#include <assert.h>
#include <string>
#include <vector>

#include <stdio.h>	
#include <gmpxx.h>

#include <math/poly.hh>

/* This is just a simple vector library [1 x m matrix]
 * Later, if we want to be more general and go to n X m  matrices, we can use blitz which allows templatization.
 *
 * No need for templates so far.
 */

typedef std::vector<poly> vec;

// multiplication by scalar
vec operator*(const vec & v, const poly & s);

// v1+v2
vec operator+(const vec & v1, const vec & v2);

// inner product <v1, v2>
poly dot(const vec & v1, const vec & v2);

vec tensor(const vec & v);

// compute v in Rq, namely v % x^n +1 % q, entrywise
vec modRq(const vec & v, uint n, mpz_class q);

std::ostream &
operator<<(std::ostream & s, const vec & v);

