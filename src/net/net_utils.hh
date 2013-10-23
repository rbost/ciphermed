#pragma once

#include <iostream>

#include <mpc/lsic.hh>
#include <gmpxx.h>

using namespace std;

ostream& operator<<(ostream& out, const LSIC_Packet_A& p);
ostream& operator<<(ostream& out, const LSIC_Packet_B& p);

istream& operator>>(istream& in, LSIC_Packet_A& p);
istream& operator>>(istream& in, LSIC_Packet_B& p);

istream& parseInt(istream& in, mpz_class &i, int base);