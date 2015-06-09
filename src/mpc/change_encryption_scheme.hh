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

#include <gmpxx.h>

#include <crypto/gm.hh>

#include <FHE.h>
#include <EncryptedArray.h>

#include <vector>

using namespace std;

class Change_ES_FHE_to_GM_A {
public:
//    Change_ES_FHE_to_GM_A()

    mpz_class blind(const mpz_class &c, GM &gm, gmp_randstate_t state);
    Ctxt unblind(const Ctxt &c, const FHEPubKey& publicKey, const EncryptedArray &ea);
protected:
    bool coin_;
};

class Change_ES_FHE_to_GM_B {
    public:
    static Ctxt decrypt_encrypt(const mpz_class &c, GM_priv &gm, const FHEPubKey &publicKey, const EncryptedArray &ea);
};



class Change_ES_FHE_to_GM_slots_A {
    public:
    //    Change_ES_FHE_to_GM_A()
    
    vector<mpz_class> blind(const vector<mpz_class> &c, GM &gm, gmp_randstate_t state, unsigned long n_slots);
    Ctxt unblind(const Ctxt &c, const FHEPubKey& publicKey, const EncryptedArray &ea);
    protected:
    vector<long> coins_;
};

class Change_ES_FHE_to_GM_slots_B {
    public:
    static Ctxt decrypt_encrypt(const vector<mpz_class> &c, GM_priv &gm, const FHEPubKey &publicKey, const EncryptedArray &ea);
};
