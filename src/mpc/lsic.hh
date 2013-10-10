#pragma once

#include <NTL/ZZ.h>
#include <vector>
#include <array>
#include <list>
#include <utility>
#include <crypto/gm.hh>                          
           

/*
 *  Implementation of Yao's Millionaires' protocol based on the
 *  Lightweight Secure Integer Comparison (LSIC) protocol of the paper
 *  "Comparing encrypted data"
 *  by Thijs Veugen
 */

struct LSIC_Packet_A {
    size_t index;
    mpz_class tau;
    
    LSIC_Packet_A();
    LSIC_Packet_A(size_t i, const mpz_class &c);
};

struct LSIC_Packet_B {
    size_t index;
    mpz_class tb;
    mpz_class bi;
    
    LSIC_Packet_B(size_t i, const mpz_class &c1, const mpz_class &c2);
};

class LSIC_A{
public:
    LSIC_A(const mpz_class &x,const size_t &l,const std::vector<mpz_class> &gm_pk, gmp_randstate_t state);

    void set_value(const mpz_class &x);
    GM gm() const { return gm_; };

    /* Runs the right round according to the current state.
     * Returns true if the last round has been ran. 
     * In this case, you can get the output by calling output()
     * Otherwise, outputPacket contains the next packet to be sent to B
     */
    bool answerRound(const LSIC_Packet_B &pack, LSIC_Packet_A *outputPacket);

    size_t bitLength() const { return bit_length_; }

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

class LSIC_B{
public:
    LSIC_B(const mpz_class &y,const size_t l, const std::vector<mpz_class> &gm_sk, gmp_randstate_t state);
    LSIC_B(const mpz_class &y,const size_t l, gmp_randstate_t state, unsigned int key_size = 1024);
    
	std::vector<mpz_class> privparams() const { return gm_.privkey(); };
	std::vector<mpz_class> pubparams() const { return gm_.pubkey(); };
    
    void set_value(const mpz_class &x);
    GM_priv gm() const { return gm_; };
    
    size_t bitLength() const { return bit_length_; }
    
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

void runProtocol(LSIC_A &party_a, LSIC_B &party_b);