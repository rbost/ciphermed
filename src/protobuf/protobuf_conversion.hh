#pragma once

#include <proto_src/proto_headers.hh>

#include <gmpxx.h>

#include <crypto/gm.hh>
#include <crypto/paillier.hh>

mpz_class convert_from_message(const Protobuf::BigInt &m);
Protobuf::BigInt convert_to_message(const mpz_class &v);

std::vector<mpz_class> convert_from_message(const Protobuf::BigIntArray &m);
Protobuf::BigIntArray convert_to_message(const std::vector<mpz_class> &v);

GM* create_from_pk_message(const Protobuf::GM_PK &m_pk, gmp_randstate_t state);
Paillier* create_from_pk_message(const Protobuf::Paillier_PK &m_pk, gmp_randstate_t state);

Protobuf::GM_PK get_pk_message(const GM *gm);
Protobuf::Paillier_PK get_pk_message(const Paillier *paillier);