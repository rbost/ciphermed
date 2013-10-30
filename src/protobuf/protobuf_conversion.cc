#include <gmpxx.h>
#include <string>

#include <protobuf/protobuf_conversion.hh>

#include <crypto/gm.hh>
#include <crypto/paillier.hh>

using namespace std;

mpz_class convert_from_message(const Protobuf::BigInt &m)
{
    if (!m.has_data()) {
        return 0;
    }
    
    mpz_class v;
    std::string str = m.data();
    mpz_import(v.get_mpz_t(),str.length(),1,sizeof(char),1,0,str.data());
    
    return v;
}

Protobuf::BigInt convert_to_message(const mpz_class &v)
{
    Protobuf::BigInt m;
    void *data;
    size_t data_count;
    
    data = mpz_export(NULL,&data_count,1,sizeof(char),1,0,v.get_mpz_t());
    m.set_data(data,data_count);
    
    return m;
}


std::vector<mpz_class> convert_from_message(const Protobuf::BigIntArray &m)
{
    size_t n = m.values_size();
    std::vector<mpz_class> v(n);
    
    for (size_t i = 0; i < n; i++) {
        v[i] = convert_from_message(m.values(i));
    }
    
    return v;
}

Protobuf::BigIntArray convert_to_message(const std::vector<mpz_class> &v)
{
    Protobuf::BigIntArray m;
    
    for (size_t i = 0; i < v.size(); i++) {
        Protobuf::BigInt *bi_ptr = m.add_values();
        *bi_ptr = convert_to_message(v[i]);
    }
    
    return m;
}

GM* create_from_pk_message(const Protobuf::GM_PK &m_pk, gmp_randstate_t state)
{
    mpz_class n(convert_from_message(m_pk.n()));
    mpz_class y(convert_from_message(m_pk.y()));

    return new GM({n,y},state);
}

Paillier* create_from_pk_message(const Protobuf::Paillier_PK &m_pk, gmp_randstate_t state)
{
    mpz_class n(convert_from_message(m_pk.n()));
    mpz_class g(convert_from_message(m_pk.g()));

    return new Paillier({n,g},state);
}


Protobuf::GM_PK get_pk_message(const GM *gm)
{
    std::vector<mpz_class> pk = gm->pubkey();
    Protobuf::GM_PK pk_message;

    *(pk_message.mutable_n()) = convert_to_message(pk[0]);
    *(pk_message.mutable_y()) = convert_to_message(pk[1]);
    
    return pk_message;
}

Protobuf::Paillier_PK get_pk_message(const Paillier *paillier)
{
    std::vector<mpz_class> pk = paillier->pubkey();
    Protobuf::Paillier_PK pk_message;
    
    *(pk_message.mutable_n()) = convert_to_message(pk[0]);
    *(pk_message.mutable_g()) = convert_to_message(pk[1]);
    
    return pk_message;
}