#include <gmpxx.h>
#include <string>
#include <sstream>

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

LSIC_Packet_A convert_from_message(const Protobuf::LSIC_A_Message &m)
{
    LSIC_Packet_A p;
    p.index = m.index();
    p.tau = convert_from_message(m.tau());
    
    return p;
}

LSIC_Packet_B convert_from_message(const Protobuf::LSIC_B_Message &m)
{
    LSIC_Packet_B p;
    p.index = m.index();
    p.tb = convert_from_message(m.tb());
    p.bi = convert_from_message(m.bi());
    
    return p;
}

Protobuf::LSIC_A_Message convert_to_message(const LSIC_Packet_A &p)
{
    Protobuf::LSIC_A_Message m;
    m.set_index(p.index);
    *(m.mutable_tau()) = convert_to_message(p.tau);
    
    return m;
}

Protobuf::LSIC_B_Message convert_to_message(const LSIC_Packet_B &p)
{
    Protobuf::LSIC_B_Message m;
    m.set_index(p.index);
    *(m.mutable_tb()) = convert_to_message(p.tb);
    *(m.mutable_bi()) = convert_to_message(p.bi);
    
    return m;
}

mpz_class convert_from_message(const Protobuf::Enc_Compare_Setup_Message &m)
{
    return convert_from_message(m.c_z());
}

Protobuf::Enc_Compare_Setup_Message convert_to_message_partial(const mpz_class &c_z)
{
    Protobuf::Enc_Compare_Setup_Message m;
    *(m.mutable_c_z()) = convert_to_message(c_z);
    
    return m;
}
Protobuf::Enc_Compare_Setup_Message convert_to_message(const mpz_class &c_z, size_t bit_length)
{
    Protobuf::Enc_Compare_Setup_Message m = convert_to_message_partial(c_z);
    m.set_bit_length(bit_length);
    
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

FHEPubKey* create_from_pk_message(const Protobuf::FHE_PK &m_pk, const FHEcontext &fhe_context)
{
    FHEPubKey *fhe_pk = new FHEPubKey(fhe_context);
    
    std::istringstream stream(m_pk.content());
    stream >> (*fhe_pk);
    
    return fhe_pk;
}

Protobuf::FHE_PK get_pk_message(const FHEPubKey& pubKey)
{
    Protobuf::FHE_PK pk_message;

    std::ostringstream stream;
    stream << pubKey;
    
    pk_message.set_content(stream.str().c_str());
    
    return pk_message;
}


Ctxt convert_from_message(const Protobuf::FHE_Ctxt &m, const FHEPubKey &pubkey)
{
    Ctxt c(pubkey);
    
    std::istringstream stream(m.content());
    stream >> c;
    
    return c;
}

Protobuf::FHE_Ctxt convert_to_message(const Ctxt &c)
{
    Protobuf::FHE_Ctxt m;
    std::ostringstream stream;
    stream << c;
    m.set_content(stream.str().c_str());
    
    return m;

}

