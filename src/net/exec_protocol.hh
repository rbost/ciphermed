#pragma once

#include <gmpxx.h>

#include <crypto/paillier.hh>
#include <crypto/gm.hh>

#include <boost/asio.hpp>
#include <net/message_io.hh>

#include <mpc/lsic.hh>
#include <mpc/private_comparison.hh>
#include <mpc/rev_enc_comparison.hh>
#include <mpc/enc_comparison.hh>
#include <mpc/linear_enc_argmax.hh>

#include <math/util_gmp_rand.h>

#include <FHE.h>
#include <EncryptedArray.h>

#include <functional>

using boost::asio::ip::tcp;


void exec_comparison_protocol_A(tcp::socket &socket, Comparison_protocol_A *comparator, unsigned int n_threads = 2);
void exec_lsic_A(tcp::socket &socket, LSIC_A *lsic);
void exec_priv_compare_A(tcp::socket &socket, Compare_A *comparator, unsigned int n_threads);

void exec_comparison_protocol_B(tcp::socket &socket, Comparison_protocol_B *comparator);
void exec_lsic_B(tcp::socket &socket, LSIC_B *lsic);
void exec_priv_compare_B(tcp::socket &socket, Compare_B *comparator);

void exec_enc_comparison_owner(tcp::socket &socket, EncCompare_Owner &owner, unsigned int lambda);
void exec_enc_comparison_helper(tcp::socket &socket, EncCompare_Helper &helper);

void exec_rev_enc_comparison_owner(tcp::socket &socket, Rev_EncCompare_Owner &owner, unsigned int lambda);
void exec_rev_enc_comparison_helper(tcp::socket &socket, Rev_EncCompare_Helper &helper);


void exec_linear_enc_argmax(tcp::socket &socket, Linear_EncArgmax_Owner &owner, function<Comparison_protocol_A*()> comparator_creator, unsigned int lambda);
void exec_linear_enc_argmax(tcp::socket &socket, Linear_EncArgmax_Helper &helper, function<Comparison_protocol_B*()> comparator_creator);


Ctxt exec_change_encryption_scheme_slots(tcp::socket &socket, const vector<mpz_class> &c_gm, GM &gm, const FHEPubKey& publicKey, const EncryptedArray &ea, gmp_randstate_t randstate);
void exec_change_encryption_scheme_slots_helper(tcp::socket &socket, GM_priv &gm, const FHEPubKey &publicKey, const EncryptedArray &ea);

mpz_class exec_compute_dot_product(tcp::socket &socket, const vector<mpz_class> &x, Paillier &p);
void exec_help_compute_dot_product(tcp::socket &socket, const vector<mpz_class> &y, Paillier_priv &pp, bool encrypted_input);
