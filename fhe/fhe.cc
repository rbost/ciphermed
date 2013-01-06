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
    //a poly with small coeffs
    return poly();
}

//TODO: currently I set the params at random! need to figure what security needs
//TODO: setup should run based on depth wanted, current setting can support one multiplication
void FHE::Setup(){
    lambda = 100;
    d = 100;
    q = 1048576;
    N = 3*20; //TODO: ceil 3 log q
    mu = 100;
    chi = ErrorDist(lambda, mu, q);
    
}

SK FHE::SKKeyGen() {
    SK sk = SK();
    sk.s = vector<poly>(2);
    sk.s[0] = poly(new vector<mpz_class>({1}));
    sk.s[1] = chi.sample();
    return sk;
}

PK FHE::PKKeyGen() {
    assert_s(false, "unimpl");
    return PK();
    
    
}
