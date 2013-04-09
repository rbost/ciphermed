#include "math/poly.hh"

#include <sstream>
#include <iostream>
#include <algorithm>
#include <util/util.hh>

using namespace std;

static inline vector<mpz_class>
zerovec(size_t sz)
{
    vector<mpz_class> ret(sz);
    return ret;
}

// add polynomial P and Q
poly
operator+(const poly &P, const poly &Q)
{
    const size_t max_sz = max(P.size(),Q.size());
    const size_t min_sz = min(P.size(), Q.size());
    vector<mpz_class> res = zerovec(max_sz);
    for (size_t i = 0; i < min_sz; i++)
        res[i] = P[i] + Q[i];
    if (P.size() > min_sz) {
        for (size_t i = min_sz; i < P.size(); i++)
            res[i] = P[i];
    } else {
        for (size_t i = min_sz; i < Q.size(); i++)
            res[i] = Q[i];
    }
    return poly(move(res));
}

//attempt at karatsuba poly multiplication
poly
karatsuba(const poly & P, const poly & Q)
{
	uint num_mult = 0;

//	assert_s(P.deg()==Q.deg(), "Trying to use karatsuba with polynomials of different degrees");

	uint deg = P.deg();
	uint n = deg+1;

	vector<mpz_class> di(n);
	for(uint i=0; i <= deg; i++){
		_mpz_realloc(di[i].get_mpz_t(), 4);
		di[i] = P[i]*Q[i];
	}

	num_mult += deg;

	uint limit = 2*n - 1;
	vector<mpz_class> dst = zerovec(limit);

        mpz_class tmp1("0",10), tmp2("0", 10), tmp3 ("0", 10), mult("0", 10);
        _mpz_realloc(tmp1.get_mpz_t(), 3);
        _mpz_realloc(tmp2.get_mpz_t(), 3);
        _mpz_realloc(tmp3.get_mpz_t(), 3);
        _mpz_realloc(mult.get_mpz_t(), 4);

        for(uint i = 0; i < limit; i++){
                _mpz_realloc(dst[i].get_mpz_t(), 4);
        }

	dst[0] = di[0];
	dst[limit-1] = di[deg];

	if(limit<2)
		return poly(move(dst));

	bool odd = true;
	uint t = 0, s = 0;
	for(uint i=1; i < n; i++){
		for(s = 0; s <= i/2; s++){
			t = i-s;
//			cerr<<"i: "<<i<<" s: "<<s<<" t: "<<t<<" n: "<<n<<endl;
			num_mult+=1;
			if(t>s && t < n){
//				cerr <<" ENTERED "<<endl;
//				(*dst)[i] += (P[s]+P[t])*(Q[s]+Q[t]) - di[s] - di[t];

                                tmp1 = P[s]+P[t];
                                tmp2 = Q[s]+Q[t];
                                tmp3 = di[s]+di[t];
                                mult = tmp1*tmp2;

                                dst[i] += mult;
                                dst[i] -= tmp3;
			}
		}
		if(odd) odd = false;
		else {
			dst[i] += di[i>>1];
			odd = true;
		}

	}
//multiples of 2 still enter loop extra time
	for(uint i=n; i <= limit-2; i++){
		for(s = i-n+1; s <= i/2; s++){
			t = i-s;
			num_mult+=1;
//			cerr<<"i: "<<i<<" s: "<<s<<" t: "<<t<<" n: "<<n<<endl;
			if(t>s && t < n){
//				cerr <<" ENTERED "<<endl;
//				(*dst)[i] += (P[s]+P[t])*(Q[s]+Q[t]) - di[s] - di[t];

                                tmp1 = P[s]+P[t];
                                tmp2 = Q[s]+Q[t];
                                tmp3 = di[s]+di[t];
                                mult = tmp1*tmp2;

                                dst[i] += mult;
                                dst[i] -= tmp3;

			}
		}
		if (odd) odd = false;
		else {
			dst[i] += di[i>>1];
			odd = true;
		}

	}

//	cerr << " karatsuba takes " << num_mult <<" mults." << endl;
	return poly(move(dst));
}

// multiply poly P and Q
// some efficient way of doing it?
poly
operator*(const poly & P, const poly & Q)
{
    uint num_mult = 0;
    vector<mpz_class> res = zerovec(P.deg() + Q.deg() + 1);

    for (uint i = 0; i < P.size(); i++) {
        for (uint j = 0; j < Q.size(); j++) {
            num_mult += 1;
            res[i+j] = res[i+j] + (P[i] * Q[j]);
        }
    }
    cerr << " txtbk mult takes " << num_mult <<" mults."<<endl;

    return poly(move(res));
}

poly
operator*(const poly &p, const mpz_class &q)
{
    poly res = p;
    for (size_t i = 0; i < res.size(); i++)
        res[i] *= q;
    return res;
}

mpz_class
poly::eval(const mpz_class &x) const
{
    mpz_class ret, xv;
    if (coeffs_.empty())
        return ret;
    ret = coeffs_[0];
    xv = x;
    for (size_t i = 1; i < coeffs_.size(); i++, xv *= x)
        ret += coeffs_[i] * xv;
    return ret;
}

poly
poly::modshift(const mpz_class &q) const
{
    assert(mpz_sgn(q.get_mpz_t()) == 1);
    mpz_class upper;
    if (mpz_odd_p(q.get_mpz_t()))
        upper = (q - 1) >> 1;
    else
        upper = q >> 1;
    vector<mpz_class> res = zerovec(size());
    for (uint i = 0; i < size(); i++) {
        // XXX: make more efficient
        mpz_class c = mpz_class_mod(coeffs_[i], q);
        if (c > upper)
            c -= q;
        res[i] = c;
    }
    return poly(move(res));
}

poly
poly::nearest_div(const mpz_class &q) const
{
    vector<mpz_class> copy(coeffs_);
    for (auto &c : copy)
        c = mpz_class_nearest_div(c, q);
    return poly(move(copy));
}

poly
operator%(const poly & P, const mpz_class & q)
{
    vector<mpz_class> res = zerovec(P.size());
    for (uint i = 0; i < P.size(); i++)
        res[i] = mpz_class_mod(P[i], q);
    return poly(move(res));
}

poly
operator-(const poly &P)
{
    poly ret = P;
    for (size_t i = 0; i < ret.size(); i++)
        ret[i] = -ret[i];
    return ret;
}

static void
subtract_monomial(poly &res, const mpz_class &c, uint delta)
{
    res[delta] = res[delta] - c;
    res.unsafe().resize(res.unsafe().size() - 1);
}

poly
modpoly(const poly & P, uint n)
{
    //we only divide by poly of the form x^n + 1
    // TODO: this could be optimized
    poly res = P;
    while (res.deg() >= n) {
        // the coeff for largest power
        mpz_class &c = res[res.size() - 1];
        subtract_monomial(res, c, res.deg() - n);
    }
    return res;
}

ostream&
operator<<(ostream & res, const poly & P)
{
    res << "{";
    for (uint i = 0; i < P.view().size(); i++) {
        res << P.view()[i];
        if (i != P.view().size()-1)
            res << ", ";
    }
    res << "}";
    return res;
}

bool
operator==(const poly & P, const poly & Q) {
    if (P.deg() != Q.deg())
        return false;
    for (uint i = 0; i < P.size(); i++)
        if (P[i] != Q[i])
            return false;
    return true;
}

/* vim:set shiftwidth=4 ts=4 sts=4 et: */
