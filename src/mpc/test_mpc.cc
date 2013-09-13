#include <assert.h>
#include <vector>
#include <mpc/millionaire.hh>
#include <mpc/lsic.hh>
#include <crypto/gm.hh>
#include <mpc/svm_classifier.hh>
#include <NTL/ZZ.h>

#include<iostream>

using namespace std;
using namespace NTL;
                                            

static void test_millionaire()
{                     
	unsigned int nbits = 256;
	
	Millionaire_Alice alice;
	Millionaire_Bob bob(alice.pubparams());
	
	ZZ x = RandomLen_ZZ(nbits);
    ZZ y = RandomLen_ZZ(nbits);
    
	vector< array <pair<ZZ,ZZ>,2> >T = alice.genTable(nbits,x);
	vector< pair<ZZ,ZZ> >C = bob.encryptRound(T,nbits,y);
	bool b = alice.decryptRound(C);
	
	assert( b == (x > y));
}

static void test_simple_svm(bool useSmallError = true, unsigned int m_size = 10, size_t nQueries = 5)
{
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
}

static void test_lsic(unsigned int nbits = 256)
{
	ZZ a = RandomLen_ZZ(nbits);
    ZZ b = RandomLen_ZZ(nbits);

    LSIC_B party_b(b, nbits);
    LSIC_A party_a(a, nbits, party_b.pubparams());
    
    LSIC_Packet_A a_packet;
    LSIC_Packet_B b_packet = party_b.setupRound();
    
    bool state;
    
    state = party_a.answerRound(b_packet,&a_packet);
    while (!state) {
        b_packet = party_b.answerRound(a_packet);
        state = party_a.answerRound(b_packet, &a_packet);
    }
    
    bool result = party_b.gm().decrypt(party_a.output());
    
    assert( result == (a > b));

}

int main(int ac, char **av)
{            
    SetSeed(to_ZZ(time(NULL)));

//	test_millionaire();
//	test_simple_svm();
    test_lsic();

	return 0;
}