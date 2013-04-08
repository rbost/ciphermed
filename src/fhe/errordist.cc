#include <fhe/fhe.hh>
#include <util/util.hh>
#include <vector>

using namespace std;

ErrorDist::ErrorDist(uint lambda, uint sigma, const PolyRing &r)
 : r_(r), gen_(), gauss_(0.0, sigma)
{
}

poly
ErrorDist::sample()
{
    // XXX(stephentu): Not correct discrete gaussian, but OK for now
    // XXX(stephentu): Super slow for now
    poly x;
    x.resize(r_.monomial_degree() + 1);
    for (size_t i = 0; i < r_.monomial_degree() + 1; i++)
        x[i] = mpz_class(gauss_(gen_));
    return r_.reduce(x);
}

/* vim:set shiftwidth=4 ts=4 et: */
