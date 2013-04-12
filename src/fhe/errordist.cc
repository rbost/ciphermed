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
    // TODO: can optimize?
    poly x;
    x.resize(r_.monomial_degree());
    for (size_t i = 0; i < r_.monomial_degree(); i++)
        x[i] = mpz_class(gauss_(gen_));
    assert_s(x == r_.reduce(x), "it needs reduction");
    return x;
}

/* vim:set shiftwidth=4 ts=4 et: */
