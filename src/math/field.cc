#include "math/field.hh"

#include <vector>
#include <sstream>
#include <iostream>

#include <util/util.hh>

using namespace std;


RLWEField::RLWEField(mpz_class _q, int degree) : q(_q), deg(degree) {

    // construct the monic polynomial
    vector<mpz_class> * c = new vector<mpz_class>(deg+1);
    for (uint i = 0 ; i < deg+1; i++) {
	c->at(i) = 0;
    }
    c->at(0) = 1;
    c->at(deg) = 1;

    //monic = poly(c);

}

// OPT: all the below could be made faster

RLWEField::add(poly a, poly b) {
    return modpoly(a+b, monic) % q;
}

RLWEField::mul(poly a, poly b) {
    return modpoly(a*b, monic) % q;
}
