#pragma once

#include <proto_src/proto_headers.hh>

#include <gmpxx.h>

#include <crypto/gm.hh>
#include <crypto/paillier.hh>
#include <mpc/lsic.hh>

#include <FHE.h>
#include <FHEContext.h>

/* multiple precision integers and arrays */
mpz_class convert_from_message(const Protobuf::BigInt &m);
Protobuf::BigInt convert_to_message(const mpz_class &v);

std::vector<mpz_class> convert_from_message(const Protobuf::BigIntArray &m);
Protobuf::BigIntArray convert_to_message(const std::vector<mpz_class> &v);

std::vector< std::vector <mpz_class> > convert_from_message(const Protobuf::BigIntMatrix &m);
Protobuf::BigIntMatrix convert_to_message(const std::vector< std::vector <mpz_class> > &v);

std::vector< std::vector< std::vector <mpz_class> >> convert_from_message(const Protobuf::BigIntMatrix_Collection &m);
Protobuf::BigIntMatrix_Collection convert_to_message(const std::vector< std::vector< std::vector <mpz_class> >> &v);

/* LSIC Packets */
LSIC_Packet_A convert_from_message(const Protobuf::LSIC_A_Message &m);
LSIC_Packet_B convert_from_message(const Protobuf::LSIC_B_Message &m);
Protobuf::LSIC_A_Message convert_to_message(const LSIC_Packet_A &p);
Protobuf::LSIC_B_Message convert_to_message(const LSIC_Packet_B &p);

/* Setup messages for comparison over encrypted data */
mpz_class convert_from_message(const Protobuf::Enc_Compare_Setup_Message &m);
Protobuf::Enc_Compare_Setup_Message convert_to_message_partial(const mpz_class &c_z);
Protobuf::Enc_Compare_Setup_Message convert_to_message(const mpz_class &c_z, size_t bit_length);

/* GM and Paillier key exchanges */

GM* create_from_pk_message(const Protobuf::GM_PK &m_pk, gmp_randstate_t state);
Paillier* create_from_pk_message(const Protobuf::Paillier_PK &m_pk, gmp_randstate_t state);

Protobuf::GM_PK get_pk_message(const GM *gm);
Protobuf::Paillier_PK get_pk_message(const Paillier *paillier);

/* FHE context, key and cyphertext */

FHEPubKey* create_from_pk_message(const Protobuf::FHE_PK &m_pk, const FHEcontext &fhe_context);
Protobuf::FHE_PK get_pk_message(const FHEPubKey& pubKey);

Ctxt convert_from_message(const Protobuf::FHE_Ctxt &m, const FHEPubKey &pubkey);
Protobuf::FHE_Ctxt convert_to_message(const Ctxt &c);

FHEcontext* create_from_message(const Protobuf::FHE_Context &m);
Protobuf::FHE_Context convert_to_message(const FHEcontext &c);