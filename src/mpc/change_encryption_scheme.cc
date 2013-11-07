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
        
        PlaintextArray array(ea);
        array.encode(1);
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
    PlaintextArray array(ea);
    array.encode(b);
    
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
    
    PlaintextArray array(ea);
    array.encode(coins_);
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
    
    PlaintextArray array(ea);
    array.encode(v);
    
    Ctxt c0(publicKey);
    ea.encrypt(c0, publicKey, array);
    
    return c0;

}