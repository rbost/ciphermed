#include <protobuf/protobuf_conversion.hh>
#include <gmpxx.h>


int main()
{
    mpz_class v = 50;
    Protobuf::BigInt m = convert_to_message(v);
    assert(v == convert_from_message(m));
    
    gmp_randstate_t randstate;
    gmp_randinit_default(randstate);
    gmp_randseed_ui(randstate,time(NULL));
    
    auto sk = Paillier_priv::keygen(randstate,2048,0);
    Paillier_priv pp(sk,randstate);

    Protobuf::Paillier_PK paillier_pk = get_pk_message(&pp);
    
    Paillier *p = create_from_pk_message(paillier_pk,randstate);
    mpz_class  n = (p->pubkey())[0];
    mpz_class pt0;
    mpz_urandomm(pt0.get_mpz_t(),randstate,n.get_mpz_t());
    mpz_class ct0 = p->encrypt(pt0);
    assert(pp.decrypt(ct0) == pt0);

    return 0;
}