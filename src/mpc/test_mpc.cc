#include <assert.h>
#include <vector>
#include <mpc/millionaire.hh>
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

void test_simple_svm()
{
    SimpleClassifier_Client client;

    unsigned int nbits = 256;
    unsigned int m_size = 20;

    srand(time(NULL));
    vector<long> model(m_size);
    vector<ZZ> test(m_size);
    
    // we don't need a good quality of randomness for testing -> use rand()
    for (size_t i = 0; i < model.size(); i++) {
        model[i] = rand();
        test[i] = to_ZZ(rand());
    }

    SimpleClassifier_Server server(model);    
    
    vector<ZZ> enc = client.encryptVector(test);
    
    size_t index;
    ZZ randomized_result = server.randomizedDotProduct(enc,client.paillierPubKey(),&index);
    vector<array <pair<ZZ,ZZ>,2> >tmp_mil1 = client.decryptAndStartMillionaire(randomized_result);
        
    vector< pair<ZZ,ZZ> >tmp_mil2 = server.compareRandomness(tmp_mil1,client.millionairePubParam(),index);
    
    bool mpc_sign = client.compareResult(tmp_mil2);
    
    ZZ v;
    
    for (size_t i = 0; i < model.size(); i++) {
        v += model[i]*test[i];
    }
    
    assert(sign == (v > 0));
}

int main(int ac, char **av)
{            
    SetSeed(to_ZZ(time(NULL)));

	test_millionaire();
	test_simple_svm();

	return 0;
}