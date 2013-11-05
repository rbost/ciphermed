#pragma once

#include <iostream>
#include <vector>

#include <boost/asio.hpp>

#include <mpc/lsic.hh>
#include <gmpxx.h>

#include <FHE.h>

using namespace std;

ostream& operator<<(ostream& out, const LSIC_Packet_A& p);
ostream& operator<<(ostream& out, const LSIC_Packet_B& p);

istream& operator>>(istream& in, LSIC_Packet_A& p);
istream& operator>>(istream& in, LSIC_Packet_B& p);

ostream& operator<<(ostream& out, const vector<mpz_class> &v);
// will only read the v.size() elements of the stream
istream& operator>>(istream& in, vector<mpz_class> &v);

istream& parseInt(istream& in, mpz_class &i, int base);


void sendIntToSocket(boost::asio::ip::tcp::socket &socket, const mpz_class& m);
mpz_class readIntFromSocket(boost::asio::ip::tcp::socket &socket);


void send_int_array_to_socket(boost::asio::ip::tcp::socket &socket, const vector<mpz_class>& m);
vector<mpz_class> read_int_array_from_socket(boost::asio::ip::tcp::socket &socket);


void send_fhe_ctxt_to_socket(boost::asio::ip::tcp::socket &socket, const Ctxt &c);
Ctxt read_fhe_ctxt_from_socket(boost::asio::ip::tcp::socket &socket, const FHEPubKey &pubkey);