#include <protobuf/protobuf_conversion.hh>
#include <gmpxx.h>


int main()
{
    mpz_class v = 50;
    Protobuf::BigInt m = mpz_class_to_bigint_message(v);
    assert(v == bigint_message_to_mpz_class(m));
    
    return 0;
}