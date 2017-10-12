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

#include <mpc/change_encryption_scheme.hh>
#include <NTL/ZZX.h>

#include <algorithm>

using namespace NTL;

mpz_class Change_ES_FHE_to_GM_A::blind(const mpz_class &c, GM &gm, gmp_randstate_t state)
{
    coin_ = gmp_urandomb_ui(state,1);
    
    if (coin_) {
        return gm.neg(c);
    }
    
    return c;
}

Ctxt Change_ES_FHE_to_GM_A::unblind(const Ctxt &c, const FHEPubKey& publicKey, const EncryptedArray &ea)
{
    if (coin_) {
        Ctxt d(c);
        
        NewPlaintextArray array(ea);
        encode(ea,array,1);
        ZZX poly;
        ea.encode(poly,array);
        
        d.addConstant(poly);
        
        return d;
    }
    
    return c;
}


Ctxt Change_ES_FHE_to_GM_B::decrypt_encrypt(const mpz_class &c, GM_priv &gm, const FHEPubKey &publicKey, const EncryptedArray &ea)
{
    bool b = gm.decrypt(c);
    NewPlaintextArray array(ea);
    encode(ea,array,b);
    
    Ctxt c0(publicKey);
    ea.encrypt(c0, publicKey, array);

    return c0;
}



vector<mpz_class> Change_ES_FHE_to_GM_slots_A::blind(const vector<mpz_class> &c, GM &gm, gmp_randstate_t state, unsigned long n_slots)
{
    size_t n = std::min<size_t>(c.size(),n_slots);
    vector<mpz_class> rand_c(n_slots);
    coins_ = vector<long>(n);
    
    for (size_t i = 0; i < n; i++) {
        coins_[i] = gmp_urandomb_ui(state,1);
        
        if (coins_[i]) {
            rand_c[i] = gm.neg(c[i]);
        }else{
            rand_c[i] = c[i];
        }
    }
    
    return rand_c;
}

Ctxt Change_ES_FHE_to_GM_slots_A::unblind(const Ctxt &c, const FHEPubKey& publicKey, const EncryptedArray &ea)
{
    Ctxt d(c);
    
    NewPlaintextArray array(ea);
    encode(ea,array,coins_);
    ZZX poly;
    ea.encode(poly,array);
    
    d.addConstant(poly);
    
    return d;
}

Ctxt Change_ES_FHE_to_GM_slots_B::decrypt_encrypt(const vector<mpz_class> &c, GM_priv &gm, const FHEPubKey &publicKey, const EncryptedArray &ea)
{
    vector<long> v(c.size());
    
    for (size_t i = 0; i < c.size(); i++) {
        v[i] = gm.decrypt(c[i]);
    }
    
    NewPlaintextArray array(ea);
    encode(ea,array,v);
    
    Ctxt c0(publicKey);
    ea.encrypt(c0, publicKey, array);
    
    return c0;

}