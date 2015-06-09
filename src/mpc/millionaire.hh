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

#pragma once

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
	Millionaire_Alice(const std::vector<NTL::ZZ> &elgamal_sk);
	Millionaire_Alice();
	   
	std::vector<NTL::ZZ> pubparams() const { return eg_.pubkey(); };
	      
	std::vector< std::array<std::pair<NTL::ZZ,NTL::ZZ>,2> > genTable(unsigned int nbits, const NTL::ZZ &x);
	bool decryptRound(const std::vector< std::pair<NTL::ZZ,NTL::ZZ> > &c) const;
	void prepareRandomness(size_t nmax = 1000);
    
protected:
	ElGamal_priv eg_;

};

class Millionaire_Bob{
public:
	Millionaire_Bob(const std::vector<NTL::ZZ> &pp);

	std::vector< std::pair<NTL::ZZ,NTL::ZZ> > encryptRound(const std::vector< std::array<std::pair<NTL::ZZ,NTL::ZZ>,2> > &T,unsigned int nbits, const NTL::ZZ &y);
    void prepareRandomness(size_t nmax = 1000);

protected:
	ElGamal eg_;
	
};