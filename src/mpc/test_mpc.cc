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
#include <mpc/private_comparison.hh>
#include <functional>

#include <FHE.h>

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

    auto sk_gm = GM_priv::keygen(randstate);
    GM_priv gm_priv(sk_gm,randstate);
    GM gm(gm_priv.pubkey(),randstate);
    
    LSIC_B party_b(b, nbits, gm_priv);
    LSIC_A party_a(a, nbits, gm);
    
    delete t;
    
    t = new ScopedTimer("LSIC execution");

    runProtocol(party_a, party_b, randstate);
    
    bool result = party_b.gm().decrypt(party_a.output());
    
    delete t;
    
    assert( result == (a < b));
    
    cout << "Test LSIC passed" << endl;
}

static void test_compare(unsigned int nbits = 256)
{
    cout << "Test compare ..." << endl;
    ScopedTimer timer("Compare");
    
    ScopedTimer *t;
    t = new ScopedTimer("Compare init");
    
    
    gmp_randstate_t randstate;
    gmp_randinit_default(randstate);
    gmp_randseed_ui(randstate,time(NULL));
    
    auto sk_p = Paillier_priv::keygen(randstate,1024,0);
    Paillier_priv pp(sk_p,randstate);
    Paillier p(pp.pubkey(),randstate);
    
    auto sk_gm = GM_priv::keygen(randstate);
    GM_priv gm_priv(sk_gm,randstate);
    GM gm(gm_priv.pubkey(),randstate);

    mpz_class a, b;
    mpz_urandom_len(a.get_mpz_t(), randstate, nbits);
    mpz_urandom_len(b.get_mpz_t(), randstate, nbits);
    
//    cout << "a = " << a.get_str(2) << endl;
//    cout << "b = " << b.get_str(2) << endl;
    
    Compare_A party_a(a, nbits, p, gm, randstate);
    Compare_B party_b(b, nbits, pp, gm_priv);
    
    delete t;
    
    t = new ScopedTimer("Compare execution");
    
    runProtocol(party_a, party_b,randstate);
    
    bool result = party_b.gm().decrypt(party_a.output());
    
    delete t;
    
    assert( result == (a < b));
    
    cout << "Test Compare passed" << endl;
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
    
    auto sk_p = Paillier_priv::keygen(randstate,1024,0);
    Paillier_priv pp(sk_p,randstate);
    Paillier p(pp.pubkey(),randstate);
    
    auto sk_gm = GM_priv::keygen(randstate);
    GM_priv gm_priv(sk_gm,randstate);
    GM gm(gm_priv.pubkey(),randstate);

    mpz_class a, b;
    mpz_urandom_len(a.get_mpz_t(), randstate, nbits);
    mpz_urandom_len(b.get_mpz_t(), randstate, nbits);
    
    LSIC_A party_a(0, nbits, gm);
    LSIC_B party_b(0, nbits, gm_priv);

    EncCompare_Owner client(pp.encrypt(a),pp.encrypt(b), nbits, p, &party_b, randstate);
    EncCompare_Helper server(nbits,pp, &party_a);
    
    delete t;
    
    t = new ScopedTimer("Running protocol");

    runProtocol(client,server,randstate,lambda);

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
    
    auto sk_p = Paillier_priv::keygen(randstate,1024,0);
    Paillier_priv pp(sk_p,randstate);
    Paillier p(pp.pubkey(),randstate);
    
    auto sk_gm = GM_priv::keygen(randstate);
    GM_priv gm_priv(sk_gm,randstate);
    GM gm(gm_priv.pubkey(),randstate);
    
    LSIC_A party_a(0, nbits, gm);
    LSIC_B party_b(0, nbits, gm_priv);

    mpz_class a, b;
    mpz_urandom_len(a.get_mpz_t(), randstate, nbits);
    mpz_urandom_len(b.get_mpz_t(), randstate, nbits);
    
    Rev_EncCompare_Helper server(nbits,pp,&party_b);
    Rev_EncCompare_Owner client(pp.encrypt(a),pp.encrypt(b), nbits, p,&party_a, randstate);
    
    delete t;
    
    t = new ScopedTimer("Running protocol");

    runProtocol(client,server,randstate,lambda);
    
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

    auto sk_p = Paillier_priv::keygen(randstate,1024,0);
    Paillier_priv pp(sk_p,randstate);
    Paillier p(pp.pubkey(),randstate);
    
    auto sk_gm = GM_priv::keygen(randstate);
    GM_priv gm_priv(sk_gm,randstate);
    GM gm(gm_priv.pubkey(),randstate);
    
    size_t real_argmax = 0;
    for (size_t i = 0; i < k; i++) {
        mpz_urandom_len(v[i].get_mpz_t(), randstate, nbits);
        if (v[i] > v[real_argmax]) {
            real_argmax = i;
        }
    }

    for (size_t i = 0; i < k; i++) {
        v[i] = pp.encrypt(v[i]);
    }
    GM *gm_ptr = &gm;
    GM_priv *gm_priv_ptr = &gm_priv;
    Paillier *p_ptr = &p;
    Paillier_priv *pp_ptr = &pp;
    gmp_randstate_t *randstate_ptr = &randstate;

    auto party_a_creator = [gm_ptr,p_ptr,nbits,randstate_ptr](){ return new Compare_A(0,nbits,*p_ptr,*gm_ptr,*randstate_ptr); };
    auto party_b_creator = [gm_priv_ptr,pp_ptr,nbits](){ return new Compare_B(0,nbits,*pp_ptr,*gm_priv_ptr); };
//    auto party_a_creator = [gm_ptr,nbits](){ return new LSIC_A(0,nbits,*gm_ptr); };
//    auto party_b_creator = [gm_priv_ptr,nbits](){ return new LSIC_B(0,nbits,*gm_priv_ptr); };
   
    EncArgmax_Owner client(v,nbits,p,party_a_creator,randstate);
    EncArgmax_Helper server(nbits,k,pp,party_b_creator);
    
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
        runProtocol(client,server,randstate,lambda,num_threads);
    }else{
        runProtocol(client,server,randstate,lambda);
    }
    delete t;
    
    vector<mpz_class>::iterator argmax;

    size_t mpc_argmax = client.output();

    assert(real_argmax == mpc_argmax);
}

void usage(char *prog)
{
    cerr << "Usage: "<<prog<<" [ optional parameters ]...\n";
    cerr << "  optional parameters have the form 'attr1=val1 attr2=val2 ...'\n";
    cerr << "  e.g, 'lambda=100'\n\n";
    cerr << "  lambda is the parameter for statistical indistinguishability [default=100]\n";
    cerr << "  l is the number of bits of tested integers [default=256]\n";
    cerr << "  n is the number of elements in the argmax test [default=10]\n";
    cerr << "  t is the number of threads used by the protocols (only for argmax test) [default=1]\n";
    cerr << endl;
    exit(0);
}


int main(int ac, char **av)
{            
    argmap_t argmap;
    argmap["lambda"] = "100";
    argmap["l"] = "256";
    argmap["n"] = "10";
    argmap["t"] = "1";
    
    if (!parseArgs(ac, av, argmap)) usage(av[0]);
    
    unsigned int lambda = atoi(argmap["lambda"]);
    unsigned int l = atoi(argmap["l"]);
    unsigned int n = atoi(argmap["n"]);
    unsigned int t = atoi(argmap["t"]);

    SetSeed(to_ZZ(time(NULL)));
    srand(time(NULL));
    

    test_lsic(l);
//    test_compare(l);

    cout << "\n\n";
    
    
    test_enc_compare(l,lambda);
    cout << "\n\n";
    test_rev_enc_compare(l,lambda);

    cout << "\n\n";
    test_enc_argmax(n,l,lambda,t);

	return 0;
}