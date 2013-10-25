#pragma once

#include <iostream>

#include <mpc/lsic.hh>
#include <gmpxx.h>

using namespace std;

ostream& operator<<(ostream& out, const LSIC_Packet_A& p);
ostream& operator<<(ostream& out, const LSIC_Packet_B& p);

istream& operator>>(istream& in, LSIC_Packet_A& p);
istream& operator>>(istream& in, LSIC_Packet_B& p);

ostream& operator<<(ostream& out, const vector<mpz_class> &v);
// will only read the v.size() elements of the stream
istream& operator>>(istream& in, vector<mpz_class> &v);

istream& parseInt(istream& in, mpz_class &i, int base);