#include <mpc/millionaire.hh>
#include <algorithm>                

using namespace std;
using namespace NTL;


Millionaire_Alice::Millionaire_Alice()
: eg_(ElGamal_priv::keygen())
{                  
//	assert( n >= NumBits(x))
	// eg_ = ElGamal_priv(ElGamal_priv::keygen());
}

vector< array <pair<ZZ,ZZ>,2> > Millionaire_Alice::genTable(unsigned int nbits, const NTL::ZZ &x)
{
	vector< array < pair<ZZ,ZZ>,2> >T(nbits,array<pair<ZZ,ZZ>,2>());
	
	for(unsigned int i = 0; i < nbits; i++){ 
		long b = bit(x,i);
		T[i][b] = eg_.encrypt(to_ZZ(1));
		T[i][1-b] = eg_.randEncrypt();
	}
	
	return T; 
}

bool Millionaire_Alice::decryptRound(const vector< pair<ZZ,ZZ> > &c) const
{
	for(unsigned int i = 0; i < c.size(); i++)
	{
		if(eg_.decrypt(c[i]) == 1)
			return true;
	}                   
	return false;
}             



Millionaire_Bob::Millionaire_Bob(const vector<ZZ> &pp)
: eg_(pp)
{
	
}
int rndGen (int n) { return RandomBnd(n);} 
  
vector< pair<ZZ,ZZ> > Millionaire_Bob::encryptRound(const vector< array<pair<ZZ,ZZ>,2> > &T, unsigned int nbits,const ZZ &y) 
{        
	vector< pair<ZZ,ZZ> >C(nbits);  
	   
	pair<ZZ,ZZ> buffer = eg_.encrypt(to_ZZ(1));
	
	for(unsigned int i = 0; i < nbits; i++){ 
		long b = bit(y,nbits-1-i);
		pair<ZZ,ZZ> nBuffer =  eg_.mult(buffer, T[nbits-1-i][b]);
		if(b == 0){        
			C[i] = eg_.mult(buffer, T[nbits-1-i][1]);
			C[i] = eg_.scalarize(C[i]);
		}else{
			C[i] = eg_.randEncrypt();		
		}
		buffer = nBuffer;
	}
	    
	random_shuffle(C.begin(),C.end(),rndGen);

	return C;
}
