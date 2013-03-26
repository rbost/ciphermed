#include <fhe/fhe.hh>
#include <util/util.hh>

#include <vector>


using namespace std;

// TODO: need to implement the actual distribution
// now it samples randomly
ErrorDist::ErrorDist(uint lambda, uint mu, mpz_class q) {
    B = 10;
    //seed it 
}

poly
ErrorDist::sample() {
    //a poly with small coeffs: TODO this is a placeholder

    vector<mpz_class> * c = new vector<mpz_class>(Rq.deg+1);
    for (uint i = 0; i< Rq.deg+1; i++) {
	c->at(i) = 1;
    }
    return poly(c);
}

