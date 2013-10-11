#include <assert.h>
#include <vector>
#include <mpc/millionaire.hh>
#include <mpc/lsic.hh>
#include <mpc/enc_comparison.hh>
#include <mpc/rev_enc_comparison.hh>
#include <mpc/enc_argmax.hh>
#include <crypto/gm.hh>
#include <mpc/svm_classifier.hh>
#include <NTL/ZZ.h>
#include <util/util.hh>
#include <math/util_gmp_rand.h>

#include<iostream>

using namespace std;
using namespace NTL;
                                            

static void test_millionaire()
{                     
    cout << "Test Millionaire ..." << flush;
    ScopedTimer timer("Millionaire");
    unsigned int nbits = 256;
	
	Millionaire_Alice alice;
	Millionaire_Bob bob(alice.pubparams());
	
	ZZ x = RandomLen_ZZ(nbits);
    ZZ y = RandomLen_ZZ(nbits);
    
	vector< array <pair<ZZ,ZZ>,2> >T = alice.genTable(nbits,x);
	vector< pair<ZZ,ZZ> >C = bob.encryptRound(T,nbits,y);
	bool b = alice.decryptRound(C);
	
	assert( b == (x > y));
    cout << " passed" << endl;
}

/*
static void test_simple_svm(bool useSmallError = true, unsigned int m_size = 10, size_t nQueries = 5)
{
    cout << "Test SVM classifier ..." << flush;
    ScopedTimer timer("SVM classifier");
    
    SimpleClassifier_Client client;

    srand(time(NULL));
    vector<long> model(m_size);
    vector<ZZ> test(m_size);
    
    // we don't need a good quality of randomness for testing -> use rand()
    for (size_t i = 0; i < m_size; i++) {
        model[i] = ((i % 2)? 1 : -1)*rand();
        test[i] = to_ZZ(rand());
    }
    
    ZZ v;
    
    for (size_t i = 0; i < model.size(); i++) {
        v += model[i]*test[i];
    }
    
    cout << "\n\nv = " << v << "\n\n";

    SimpleClassifier_Server server(model);
    cout << "Start encryption ... " << flush;
    vector<ZZ> enc = client.encryptVector(test);
    cout << " done"<<endl;
    
    cout << "Computing randomized dot products ... " << endl;
    
    vector<pair<size_t,ZZ> > randomized_results;
    if (useSmallError) {
        cout << "Use small error" << endl;
        randomized_results = server.randomizedDotProduct_smallError(enc, nQueries, client.paillierPubKey(),to_ZZ(0));
    }else{
        randomized_results = server.randomizedDotProduct(enc, nQueries, client.paillierPubKey());
    }

    cout << "... done\n"<<endl;

    size_t posCount = 0, negCount = 0;
    for (size_t i = 0; i < nQueries; i++) {
        size_t index = get<0>(randomized_results[i]);
        ZZ rand_val = get<1>(randomized_results[i]);
        
        cout << "Starting millionaires' protocol " << i << endl;
        vector<array <pair<ZZ,ZZ>,2> >tmp_mil1 = client.decryptAndStartMillionaire(rand_val);
        
        cout << "Server's turn" <<endl;
        vector< pair<ZZ,ZZ> >tmp_mil2 = server.compareRandomness(tmp_mil1,client.millionairePubParam(),index);
        
        cout << "Client's final round"<<endl;
        bool mpc_sign = client.compareResult(tmp_mil2);
        cout << "Protocol completed \n" << endl;
        
        mpc_sign ? posCount++ : negCount++;
    }
    
    cout << posCount << " positive results\n";
    cout << negCount << " negative results\n";
    cout << "Real result is " << ((v>0)? "positive" : "negative") << endl;
    
    assert((posCount >= negCount) == (v > 0));
    cout << " passed" << endl;
}
*/

static void test_lsic(unsigned int nbits = 256)
{
    cout << "Test LSIC ..." << endl;
    ScopedTimer timer("LSIC");
    
    ScopedTimer *t;
    t = new ScopedTimer("LSIC init");


    gmp_randstate_t randstate;
    gmp_randinit_default(randstate);
    gmp_randseed_ui(randstate,time(NULL));

    mpz_class a, b;
    mpz_urandom_len(a.get_mpz_t(), randstate, nbits);
    mpz_urandom_len(b.get_mpz_t(), randstate, nbits);

    LSIC_B party_b(b, nbits, randstate);
    LSIC_A party_a(a, nbits, party_b.pubparams(), randstate);
    
    delete t;
    
    t = new ScopedTimer("LSIC execution");

    runProtocol(party_a, party_b);
    
    bool result = party_b.gm().decrypt(party_a.output());
    
    delete t;
    
    assert( result == (a < b));
    
    cout << "Test LSIC passed" << endl;
}

static void test_enc_compare(unsigned int nbits = 256,unsigned int lambda = 100)
{
    cout << "Test comparison over encrypted data ..." << endl;
    ScopedTimer timer("Enc. Compare");
    
    ScopedTimer *t;
    t = new ScopedTimer("Protocol init");
    
    gmp_randstate_t randstate;
    gmp_randinit_default(randstate);
    gmp_randseed_ui(randstate,time(NULL));
    
    auto sk_p = Paillier_priv::keygen(randstate);
    Paillier_priv paillier(sk_p,randstate);
    
    mpz_class a, b;
    mpz_urandom_len(a.get_mpz_t(), randstate, nbits);
    mpz_urandom_len(b.get_mpz_t(), randstate, nbits);
    
    EncCompare_Owner client(paillier.encrypt(a),paillier.encrypt(b), nbits, paillier.pubkey(), randstate);
    auto pk_gm = client.lsic().gm().pubkey();
    EncCompare_Helper server(nbits,sk_p,pk_gm,randstate);
    
    delete t;
    
    t = new ScopedTimer("Running protocol");

    runProtocol(client,server,lambda);

    delete t;

    bool result = client.output();
    
    assert( result == (a <= b));
    
    cout << "Test passed" << endl;
}

static void test_rev_enc_compare(unsigned int nbits = 256,unsigned int lambda = 100)
{
    cout << "Test reverse comparison over encrypted data ..." << endl;
    ScopedTimer timer("Enc. Compare");
    
    ScopedTimer *t;
    t = new ScopedTimer("Protocol init");
    
    gmp_randstate_t randstate;
    gmp_randinit_default(randstate);
    gmp_randseed_ui(randstate,time(NULL));
    
    auto sk_p = Paillier_priv::keygen(randstate);
    Paillier_priv paillier(sk_p,randstate);
    
    auto sk_gm = GM_priv::keygen(randstate);
    
    mpz_class a, b;
    mpz_urandom_len(a.get_mpz_t(), randstate, nbits);
    mpz_urandom_len(b.get_mpz_t(), randstate, nbits);
    
    Rev_EncCompare_Helper server(nbits,sk_p,sk_gm,randstate);
    auto pk_gm = server.lsic().gm().pubkey();
    Rev_EncCompare_Owner client(paillier.encrypt(a),paillier.encrypt(b), nbits, paillier.pubkey(),pk_gm, randstate);
    
    delete t;
    
    t = new ScopedTimer("Running protocol");

    runProtocol(client,server,lambda);
    
    delete t;
    
    bool result = server.output();
    
    assert( result == (a <= b));
    
    cout << "Test passed" << endl;
}

static void test_enc_argmax(unsigned int k = 5, unsigned int nbits = 256,unsigned int lambda = 100, unsigned int num_threads = 1)
{
    cout << "Test argmax over encrypted data ..." << endl;
    cout << k << " integers of " << nbits << " bits, " << lambda << " bits of security\n";
    ScopedTimer timer("Enc. Argmax");

    vector<mpz_class> v(k);
    
    ScopedTimer *t;
    t = new ScopedTimer("Protocol init");

    gmp_randstate_t randstate;
    gmp_randinit_default(randstate);
    gmp_randseed_ui(randstate,time(NULL));

    
    size_t real_argmax = 0;
    for (size_t i = 0; i < k; i++) {
        mpz_urandom_len(v[i].get_mpz_t(), randstate, nbits);
        if (v[i] > v[real_argmax]) {
            real_argmax = i;
        }
    }

    vector<mpz_class> sk_p = Paillier_priv::keygen(randstate);
    vector<mpz_class> pk_p = {sk_p[0]*sk_p[1], sk_p[2]};
    vector<mpz_class> sk_gm = GM_priv::keygen(randstate);
    vector<mpz_class> pk_gm = {sk_gm[0],sk_gm[1]};

    Paillier_priv paillier(sk_p,randstate);

    for (size_t i = 0; i < k; i++) {
        v[i] = paillier.encrypt(v[i]);
    }
    
    EncArgmax_Owner client(v,nbits,pk_p,pk_gm,randstate);
    EncArgmax_Helper server(nbits,k,sk_p,sk_gm,randstate);
    
    delete t;
    
    t = new ScopedTimer("Running comparisons");
//
//    for (size_t i = 0; i < k; i++) {
//        for (size_t j = 0; j < i; j++) {
//            runProtocol(*(client.comparators()[i][j]), *(server.comparators()[i][j]), lambda);
//        }
//    }
//    
//    delete t;
//    
//    t = new ScopedTimer("Sorting & un-permuting");
//    server.sort();
//    client.unpermuteResult(server.permuted_argmax());

    
    if (num_threads > 1) {
        runProtocol(client,server,lambda,num_threads);
    }else{
        runProtocol(client,server,lambda);
    }
    delete t;
    
    vector<mpz_class>::iterator argmax;

    size_t mpc_argmax = client.output();

    assert(real_argmax == mpc_argmax);
}

int main(int ac, char **av)
{            
    SetSeed(to_ZZ(time(NULL)));
    srand(time(NULL));
    
//	test_millionaire();

//    test_lsic(256);
//    cout << "\n\n";
    
    
//    test_enc_compare(256,100);
//    cout << "\n\n";
//    test_rev_enc_compare(256,100);
    //	test_simple_svm();

//    cout << "\n\n";
    test_enc_argmax(10,256,100);
    
	return 0;
}