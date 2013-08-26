#include <assert.h>
#include <vector>
#include <crypto/elgamal.hh>
#include <crypto/paillier.hh>
#include <NTL/ZZ.h>

#include<iostream>

using namespace std;
using namespace NTL;


static void
test_elgamal()
{
    auto sk = ElGamal_priv::keygen();
    ElGamal_priv pp(sk);
    
    auto pk = pp.pubkey();
    ElGamal p(pk);
    
    ZZ pt0 = RandomLen_ZZ(256);
    ZZ pt1 = RandomLen_ZZ(256);    

    pair<ZZ,ZZ> ct0 = p.encrypt(pt0);
    pair<ZZ,ZZ> ct1 = p.encrypt(pt1);
    pair<ZZ,ZZ> prod = p.mult(ct0, ct1);
	pair<ZZ,ZZ> scal = p.scalarize(p.encrypt(to_ZZ(1)));

    assert(pp.decrypt(ct0) == pt0);
    assert(pp.decrypt(ct1) == pt1);
    assert(pp.decrypt(prod) == (pt0 * pt1));
    assert(pp.decrypt(scal) == to_ZZ(1));
}

static void
test_paillier()
{
    auto sk = Paillier_priv::keygen();
    Paillier_priv pp(sk);

    auto pk = pp.pubkey();
    ZZ n = pk[0];
    Paillier p(pk);

    ZZ pt0 = RandomBnd(n);
    ZZ pt1 = RandomBnd(n);
    ZZ m = RandomBnd(n);

    ZZ ct0 = p.encrypt(pt0);
    ZZ ct1 = p.encrypt(pt1);
    ZZ sum = p.add(ct0, ct1);
    ZZ prod = p.constMult(m,ct0);
    
    assert(pp.decrypt(ct0) == pt0);
    assert(pp.decrypt(ct1) == pt1);
    assert(pp.decrypt(sum) == AddMod(pt0, pt1,n));
    assert(pp.decrypt(prod) == MulMod(m,pt0,n));
}

int
main(int ac, char **av)
{
    SetSeed(to_ZZ(time(NULL)));
    test_elgamal();
	test_paillier();
	
    return 0;
}
