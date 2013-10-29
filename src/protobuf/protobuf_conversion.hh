#pragma once

#include <gmpxx.h>
#include <proto_src/bigint.pb.h>

mpz_class bigint_message_to_mpz_class(const Protobuf::BigInt &m);
Protobuf::BigInt mpz_class_to_bigint_message(const mpz_class &v);
