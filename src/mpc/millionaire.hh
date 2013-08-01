#include <NTL/ZZ.h>
#include <vector>
#include <array>
#include <list>
#include <utility>
#include <crypto/elgamal.hh>                          
           

/*
 *  Implementation of Yao's Millionaires' protocol based on the paper
 *  "An efficient Solution to The Millionaires' Problem Based on Homomorphic Encryption"
 *  by Lin and Tweng
 */

class Millionaire_Alice{         
public:
	Millionaire_Alice();
	    
	std::vector<NTL::ZZ> pubparams() const { return eg_.pubkey(); };
	      
	std::vector< std::array<std::pair<NTL::ZZ,NTL::ZZ>,2> > genTable(unsigned int nbits, const NTL::ZZ &x);
	bool decryptRound(const std::vector< std::pair<NTL::ZZ,NTL::ZZ> > &c) const;
	
protected:
	ElGamal_priv eg_;

};

class Millionaire_Bob{
public:
	Millionaire_Bob(const std::vector<NTL::ZZ> &pp);

	std::vector< std::pair<NTL::ZZ,NTL::ZZ> > encryptRound(const std::vector< std::array<std::pair<NTL::ZZ,NTL::ZZ>,2> > &T,unsigned int nbits, const NTL::ZZ &y);
	
protected:
	ElGamal eg_;
	
};