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
#include <list>
#include <utility>

class ElGamal {
public:
    ElGamal(const std::vector<NTL::ZZ> &pk);
    std::vector<NTL::ZZ> pubkey() const { return { p, g, h }; }
    
    std::pair<NTL::ZZ,NTL::ZZ> encrypt(const NTL::ZZ &plaintext);
    std::pair<NTL::ZZ,NTL::ZZ> encrypt1();
    std::pair<NTL::ZZ,NTL::ZZ> randEncrypt();
    std::pair<NTL::ZZ,NTL::ZZ> mult(const std::pair<NTL::ZZ,NTL::ZZ> &c0, const std::pair<NTL::ZZ,NTL::ZZ> &c1) const;
	std::pair<NTL::ZZ,NTL::ZZ> scalarize(const std::pair<NTL::ZZ,NTL::ZZ> &c) const;

    void rand_gen(size_t niter = 100, size_t nmax = 1000);

protected:
    /* Public key */
    const NTL::ZZ p, g, h;

    /* Cached values */
    const NTL::ZZ q;
    const uint qbits;
    
    /* Pre-computed randomness */
    std::list< std::pair<NTL::ZZ,NTL::ZZ> > rqueue;
};

class ElGamal_priv : public ElGamal {    
public:
    ElGamal_priv(const std::vector<NTL::ZZ> &sk);
    std::vector<NTL::ZZ> privkey() const { return { q, g, h, x }; }
    
    NTL::ZZ decrypt(const std::pair<NTL::ZZ,NTL::ZZ> &ciphertext) const;

    static std::vector<NTL::ZZ> keygen(unsigned int qbits = 1024);

protected:
    /* Private key */
    const NTL::ZZ x;
};