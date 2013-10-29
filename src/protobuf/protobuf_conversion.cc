#include <gmpxx.h>
#include <string>

#include <protobuf/protobuf_conversion.hh>

using namespace std;

mpz_class bigint_message_to_mpz_class(const Protobuf::BigInt &m)
{
    if (!m.has_data()) {
        return 0;
    }
    
    mpz_class v;
    std::string str = m.data();
    mpz_import(v.get_mpz_t(),str.length(),1,sizeof(char),1,0,str.data());
    
    return v;
}

Protobuf::BigInt mpz_class_to_bigint_message(const mpz_class &v)
{
    Protobuf::BigInt m;
    void *data;
    size_t data_count;
    
    data = mpz_export(NULL,&data_count,1,sizeof(char),1,0,v.get_mpz_t());
    m.set_data(data,data_count);
    
    return m;
}
