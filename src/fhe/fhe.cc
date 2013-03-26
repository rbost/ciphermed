#include <fhe/fhe.hh>
#include <util/util.hh>

#include <vector>


/*
 * FHE implementation RLWE of BGV12 n = 1
 *
 */

using namespace std;


//TODO: currently I set the params at random! need to figure what security needs
//TODO: setup should run based on depth wanted, current setting can support one multiplication
void FHE::Setup(){
    lambda = 100;
    d = 100;
    q = 1048576;
    N = ceil(3 * log q);
    Rq = RLWEField(q, d);
    mu = 100;
    chi = ErrorDist(lambda, mu, Rq);
    
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



