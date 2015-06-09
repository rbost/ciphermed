/*
 * Copyright 2013-2015 Raphael Bost
 *
 * This file is part of ciphermed.

 *  ciphermed is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 * 
 *  ciphermed is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 * 
 *  You should have received a copy of the GNU General Public License
 *  along with ciphermed.  If not, see <http://www.gnu.org/licenses/>. 2
 *
 */

#pragma once

#include <NTL/ZZ.h>
#include <vector>
#include <array>
#include <list>
#include <utility>
#include <crypto/gm.hh>                          
           
#include <mpc/comparison_protocol.hh>

/*
 *  Implementation of Yao's Millionaires' protocol based on the
 *  Lightweight Secure Integer Comparison (LSIC) protocol of the paper
 *  "Comparing encrypted data"
 *  by Thijs Veugen
 */

struct LSIC_Packet_A {
    size_t index;
    mpz_class tau;
    
    LSIC_Packet_A() : index(0), tau(0) {};
    LSIC_Packet_A(size_t i, const mpz_class &c) : index(i), tau(c) {};
};

struct LSIC_Packet_B {
    size_t index;
    mpz_class tb;
    mpz_class bi;
    
    LSIC_Packet_B() : index(0), tb(0), bi(0) {};
    LSIC_Packet_B(size_t i, const mpz_class &c1, const mpz_class &c2) : index(i), tb(c1), bi(c2){};
};

class LSIC_A : public Comparison_protocol_A{
public:
    LSIC_A(const mpz_class &x,const size_t &l,GM &gm);

    void set_value(const mpz_class &x);
    GM gm() const { return gm_; };

    /* Runs the right round according to the current state.
     * Returns true if the last round has been ran. 
     * In this case, you can get the output by calling output()
     * Otherwise, outputPacket contains the next packet to be sent to B
     */
    bool answerRound(const LSIC_Packet_B &pack, LSIC_Packet_A *outputPacket);

    size_t bitLength() const { return bit_length_; }
    void set_bit_length(size_t l);

    /* Returns the output of the protocol if the last round has been ran.
     *  An assert will fail otherwise
     */
    mpz_class output() const;
    
protected:
    mpz_class a_;
    size_t bit_length_; // bit length of the numbers to compare
	GM gm_;
    
    /* internal state */
    bool c_; // the fair coin 'c' in the paper
    size_t i_;
    mpz_class t_;
    
    /* private functions */
    
    /* Runs lines 2 to 6 to get t_1 and then runs the blinding step */
    LSIC_Packet_A firstRound_(const LSIC_Packet_B &pack);
    
    /* Runs the update step, increments i_ and then runs a new bliding step */
    LSIC_Packet_A regularRound_(const LSIC_Packet_B &pack);
    /* Only runs the update step */
    void lastRound_(const LSIC_Packet_B &pack);
    
    /* Lines 10 to 17 in the paper */
    mpz_class blindingStep_();
    /* Lines 27 to 35 in the paper */
    void updateStep_(const LSIC_Packet_B &pack);
};

class LSIC_B : public Comparison_protocol_B{
public:
    LSIC_B(const mpz_class &y,const size_t l, GM_priv &gm);
    
	std::vector<mpz_class> privparams() const { return gm_.privkey(); };
	std::vector<mpz_class> pubparams() const { return gm_.pubkey(); };
    
    void set_value(const mpz_class &x);
    GM_priv gm() const { return gm_; };
    
    size_t bitLength() const { return bit_length_; }
    void set_bit_length(size_t l) { bit_length_ = l; };

    /* The beginning of the protocol - line 1 in the paper */
    LSIC_Packet_B setupRound();
    /* Lines 18 to 25 in the paper */
    LSIC_Packet_B answerRound(const LSIC_Packet_A &pack);
    
protected:
    mpz_class b_;
    size_t bit_length_; // bit length of the numbers to compare
    GM_priv gm_;
    bool protocol_started_;
};

void runProtocol(LSIC_A &party_a, LSIC_B &party_b, gmp_randstate_t state);
inline void runProtocol(LSIC_A *party_a, LSIC_B *party_b, gmp_randstate_t state)
{
    runProtocol(*party_a,*party_b,state);
}
