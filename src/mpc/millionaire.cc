/*
 * Copyright 2013-2015 Raphael Bost
 *
 * This file is part of ciphermed.

 *  ciphermed is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 * 
 *  ciphermed is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 * 
 *  You should have received a copy of the GNU General Public License
 *  along with ciphermed.  If not, see <http://www.gnu.org/licenses/>. 2
 *
 */

#include <mpc/millionaire.hh>
#include <algorithm>                

using namespace std;
using namespace NTL;

Millionaire_Alice::Millionaire_Alice(const vector<ZZ> &elgamal_sk)
: eg_(elgamal_sk)
{
    
}

Millionaire_Alice::Millionaire_Alice()
: eg_(ElGamal_priv::keygen())
{                  

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

void Millionaire_Alice::prepareRandomness(size_t nmax)
{
    eg_.rand_gen(nmax,nmax);
}


Millionaire_Bob::Millionaire_Bob(const vector<ZZ> &pp)
: eg_(pp)
{
	
}
int rndGen (int n) { return RandomBnd(n);} 
  
vector< pair<ZZ,ZZ> > Millionaire_Bob::encryptRound(const vector< array<pair<ZZ,ZZ>,2> > &T, unsigned int nbits,const ZZ &y) 
{        
	vector< pair<ZZ,ZZ> >C(nbits);  
	   
	pair<ZZ,ZZ> buffer = eg_.encrypt1();
	
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

void Millionaire_Bob::prepareRandomness(size_t nmax)
{
    eg_.rand_gen(nmax,nmax);
}

