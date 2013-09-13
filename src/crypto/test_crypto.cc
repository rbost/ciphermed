#include <assert.h>
#include <vector>
#include <crypto/elgamal.hh>
#include <crypto/paillier.hh>
#include <crypto/gm.hh>
#include <NTL/ZZ.h>

#include<iostream>

using namespace std;
using namespace NTL;


static void
test_elgamal()
{
    cout << "Test ElGamal ..." << flush;
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
    
    cout << " passed" << endl;
}

static void
test_paillier()
{
    cout << "Test Paillier ..." << flush;
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

    cout << " passed" << endl;
}

static void
test_gm()
{
    cout << "Test GM ..." << flush;
    auto sk = GM_priv::keygen();
    GM_priv pp(sk);
    
    auto pk = pp.pubkey();
    GM p(pk);
    
    bool b0 = true; //(bool)RandomBits_long(1);
    bool b1 = false; //(bool)RandomBits_long(1);
    
    ZZ ct0 = p.encrypt(b0);
    ZZ ct1 = p.encrypt(b1);
    ZZ XOR = p.XOR(ct0, ct1);
    ZZ rerand = p.reRand(ct0);
    
    assert(pp.decrypt(pk[1]) == true);
    assert(pp.decrypt(ct0) == b0);
    assert(pp.decrypt(ct1) == b1);
    assert(pp.decrypt(XOR) == (b0 xor b1));
    assert(pp.decrypt(rerand) == b0);

    cout << " passed" << endl;
}

int
main(int ac, char **av)
{
    SetSeed(to_ZZ(time(NULL)));
//    test_elgamal();
//	test_paillier();
	test_gm();
    
    return 0;
}
