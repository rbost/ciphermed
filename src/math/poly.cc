#include "math/poly.hh"

#include <sstream>
#include <iostream>
#include <algorithm>
#include <util/util.hh>

using namespace std;

static vector<mpz_class> *
newvec(uint sz) {
    vector<mpz_class> * res = new vector<mpz_class>(sz);
    for (auto it = res->begin(); it != res->end(); it++) {
	*it = 0;
    }
    return res;
}

static vector<mpz_class> *
copy(vector<mpz_class> * x) {
    uint sz = x->size();
    vector<mpz_class> * r = newvec(sz);
    
    for (uint i = 0; i < sz; i++) {
	r->at(i) = x->at(i);
    }

    return r;
}

poly
poly::zero() {
    return poly(new vector<mpz_class>({0}));
}

poly::poly(std::vector<mpz_class> * c, bool makecopy) {
    if (!makecopy) {
	coeffs = c;
    } else {
	coeffs = ::copy(c);
    }
}

poly::poly(const poly & P) {
    coeffs = ::copy(P.coeffs);
}

void
poly::operator=(const poly & P) {
    coeffs = ::copy(P.coeffs);
}


poly::~poly() {
    coeffs->clear();
    delete coeffs;
}


mpz_class &
poly::operator[](uint i) const {
    if(i > deg() ) return *(new mpz_class(0));
    return coeffs->at(i);
}

// add polynomial P and Q mod q
poly operator+(const poly & P, const poly & Q){
    uint max_sz = max(P.size(),Q.size());
    uint min_sz = min(P.size(), Q.size());
    
    vector<mpz_class> * res = newvec(max_sz);


    for (uint i = 0; i < min_sz; i++) {
	res->at(i) = P[i] + Q[i];
    }

    if (P.size() > min_sz) {
	for (uint i = min_sz; i < P.size(); i++) {
	    res->at(i) = P[i];
	}
    } else {
	for (uint i = min_sz; i < Q.size(); i++) {
	    res->at(i) = Q[i];
	}
    }

    return poly(res);
}

//attempt at karatsuba poly multiplication
poly karatsuba(const poly & P, const poly & Q){

	uint num_mult = 0;

//	assert_s(P.deg()==Q.deg(), "Trying to use karatsuba with polynomials of different degrees");
	
	uint deg = P.deg();
	uint n = deg+1;

	vector<mpz_class> di(n);
	for(uint i=0; i <= deg; i++){
		di[i] = P[i]*Q[i];
	}
	
	num_mult += deg;
	
	uint limit = 2*n - 1;
	vector<mpz_class> * dst = newvec(limit);

	(*dst)[0] = di[0];
	(*dst)[limit-1] = di[deg];

	if(limit<2)
		return poly(dst);

	bool odd = true;
	for(uint i=1; i < n; i++){
		for(uint s = 0; s <= i/2; s++){
			uint t = i-s;
//			cerr<<"i: "<<i<<" s: "<<s<<" t: "<<t<<" n: "<<n<<endl;
			num_mult+=1;
			if(t>s && t < n){
//				cerr <<" ENTERED "<<endl;
				(*dst)[i] += (P[s]+P[t])*(Q[s]+Q[t]) - di[s] - di[t];
			}
		}
		if(odd) odd = false;
		else {
			(*dst)[i] += di[i>>1];
			odd = true;
		}
		
	}
//multiples of 2 still enter loop extra time
	for(uint i=n; i <= limit-2; i++){
		for(uint s = i-n+1; s <= i/2; s++){
			uint t = i-s;
			num_mult+=1;
//			cerr<<"i: "<<i<<" s: "<<s<<" t: "<<t<<" n: "<<n<<endl;
			if(t>s && t < n){
//				cerr <<" ENTERED "<<endl;
				(*dst)[i] += (P[s]+P[t])*(Q[s]+Q[t]) - di[s] - di[t];
			}
		}
		if (odd) odd = false;
		else {
			(*dst)[i] += di[i>>1];
			odd = true;
		}
		
	}

//	cerr << " karatsuba takes " << num_mult <<" mults." << endl;
	return poly(dst);
}

// multiply poly P and Q mod q
// some efficient way of doing it?
poly operator*(const poly & P, const poly & Q) {

    uint num_mult = 0;

    vector<mpz_class> * res = newvec(P.deg() + Q.deg() + 1);

    for (uint i = 0; i < P.size(); i++) {
	for (uint j = 0; j < Q.size(); j++) {
	    num_mult += 1;
	    res->at(i+j) = res->at(i+j) + (P[i] * Q[j]);
	}
    }
    cerr << " txtbk mult takes " << num_mult <<" mults."<<endl;

    return poly(res); 
}

poly operator%(const poly & P, const mpz_class & q) {
    vector<mpz_class> * res = newvec(P.size());

    for (uint i = 0; i < P.size(); i++) {
	res->at(i) = P[i] % q;
    }

    return poly(res);
}

static void
subtract_monomial(poly & res, const mpz_class & c, uint n, uint delta) {
    res[delta] = res[delta] - c;
    res.coeffs->resize(res.coeffs->size() - 1);
}


poly
poly::copy() const {
    return poly(::copy(coeffs));
}

poly
modpoly(const poly & P, uint n) {
    //we only divide by poly of the form x^n + 1
    // TODO: this could be optimized 

    poly res = P.copy();
        
    while (res.deg() >= n) {
	// the coeff for largest power
	mpz_class c = res[res.size()-1];
	subtract_monomial(res, c, n, res.deg()-n);
    }

    return res;
}



ostream&
operator<<(ostream & res, const poly & P) {

    res << "{";

    for (uint i = 0; i < P.coeffs->size(); i++) {
	res << P.coeffs->at(i);
	if (i != P.coeffs->size()-1)
	    res << ", ";
    }
    
    res << "}";

    return res;
}

bool
operator==(const poly & P, const poly & Q) {
    if (P.deg() != Q.deg()) {
	return false;
    }

    for (uint i = 0; i < P.size(); i++) {
	if (P[i] != Q[i]) {
	    return false;
	}
    }

    return true;
}
