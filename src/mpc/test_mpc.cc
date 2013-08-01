#include <assert.h>
#include <vector>
#include <mpc/millionaire.hh>
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

int main(int ac, char **av)
{            
	test_millionaire();
	
	return 0;
}