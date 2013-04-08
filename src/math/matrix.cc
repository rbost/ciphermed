#include "math/matrix.hh"

#include <vector>
#include <sstream>
#include <iostream>

#include <util/util.hh>

using namespace std;


vec
operator*(const vec & v, const poly & s) {
    vec res = vector<poly>(v.size());

    for (uint i = 0; i < v.size(); i++) {
	res[i] = v[i] * s;
    }

    //TODO: avoid copying
    return res;
}

vec
operator+(const vec & v1, const vec & v2) {
    assert_s(v1.size() == v2.size(), "cannot add vectors not of the same size");


    vector<poly> res = vector<poly>(v1.size());
    for (uint i = 0; i < v1.size(); i++) {
	res[i] = v1[i] + v2[i];
    }

    //TODO: could avoid copying on return
    return res;
}

poly
dot(const vec & v1, const vec & v2) {
    assert(v1.size() == v2.size());

    poly res;

    for (uint i = 0; i < v1.size(); i++) {
	res = res + (v1[i] * v2[i]);
    }

    return res;
}

vec
tensor(const vec & v) {

    assert_s(false, "unimplemented");
    return vector<poly>();
    //TODO
}

vec
modRq(const vec & v, uint n, mpz_class q) {
    vec res = vector<poly>(v.size());

    for (uint i = 0; i < v.size(); i++) {
	res[i] = modpoly(v[i], n) % q;
    }

    return res;
}


std::ostream &
operator<<(std::ostream & s, const vec & v) {
    s << "{";

    for (auto it = v.begin(); it != v.end(); it++) {
	s << *it << ", ";
    }
    s << "} ";
    return s;
}
