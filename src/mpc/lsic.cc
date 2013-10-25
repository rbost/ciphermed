#include <mpc/lsic.hh>
#include <algorithm>                
#include <assert.h>

using namespace std;
using namespace NTL;


/* LSIC_Packet_A */

LSIC_Packet_A::LSIC_Packet_A(size_t i, const mpz_class &c)
: index(i), tau(c)
{
}

LSIC_Packet_A::LSIC_Packet_A()
: index(0), tau(0)
{
}

/* LSIC_Packet_B */

LSIC_Packet_B::LSIC_Packet_B(size_t i, const mpz_class &c1, const mpz_class &c2)
: index(i), tb(c1), bi(c2)
{
}

/* LSIC_A */

//LSIC_A::LSIC_A(const mpz_class &x,const size_t &l,const vector<mpz_class> &gm_pk, gmp_randstate_t state)
//: a_(x), bit_length_(l), gm_(gm_pk,state), i_(0)
//{
//}

LSIC_A::LSIC_A(const mpz_class &x,const size_t &l,GM &gm)
: a_(x), bit_length_(l), gm_(gm), i_(0)
{
}

void LSIC_A::set_value(const mpz_class &x)
{
    // we must not be able to reset a after the protocol started
    assert(i_ == 0);
    a_ = x;
}

mpz_class LSIC_A::blindingStep_()
{
    c_ = (bool)RandomLen_long(1);
    mpz_class tau;
    
    if (c_) {
        tau = gm_.XOR(gm_.encrypt(1),t_);
    }else{
        tau = t_;
    }
    
    return gm_.reRand(tau);
}

void LSIC_A::updateStep_(const LSIC_Packet_B &pack)
{
    mpz_class tb2;
    
    if (c_) {
        tb2 = gm_.XOR(pack.tb,pack.bi);
    }else{
        tb2 = pack.tb;
    }
    
    if (mpz_tstbit(a_.get_mpz_t(),i_) == 0) {
        t_ = gm_.XOR(t_,gm_.XOR(pack.bi,tb2));
    }else{
        t_ = tb2;
    }
}

LSIC_Packet_A LSIC_A::firstRound_(const LSIC_Packet_B &pack)
{        
    bool a0 = (bool)mpz_tstbit(a_.get_mpz_t(),0);
    
    if (a0) {
        t_ = 1;
    }else{
        t_ = pack.bi;
    }
    i_++;
    
    return LSIC_Packet_A(i_,blindingStep_());
}

LSIC_Packet_A LSIC_A::regularRound_(const LSIC_Packet_B &pack)
{
    updateStep_(pack);
    
    i_++; // at this point t_ encrypts t_[i] (cf. Veugen's paper)
    
    mpz_class tau = blindingStep_();
    
    return LSIC_Packet_A(i_, tau);
}

void LSIC_A::lastRound_(const LSIC_Packet_B &pack)
{
    updateStep_(pack);
    
    i_++; // at this point t_ encrypts t_[i] (cf. Veugen's paper)
}

bool LSIC_A::answerRound(const LSIC_Packet_B &pack, LSIC_Packet_A *outputPacket)
{
    assert (i_ == pack.index);
    
    if (i_ == 0) {
        *outputPacket = firstRound_(pack);
        return false;
    }else if(i_ == bit_length_-1){
        lastRound_(pack);
        return true;
    }else{
        *outputPacket = regularRound_(pack);
        return false;
    }
}

mpz_class LSIC_A::output() const
{
    assert(i_ == bit_length_);
    return t_;
}

/* LSIC_B */

LSIC_B::LSIC_B(const mpz_class &y,const size_t l, GM_priv &gm)
: b_(y), bit_length_(l), gm_(gm), protocol_started_(false)
{
}

void LSIC_B::set_value(const mpz_class &x)
{
    // we must not be able to reset b after the protocol started
    assert(!protocol_started_);
    b_ = x;
}


LSIC_Packet_B LSIC_B::setupRound()
{
    protocol_started_ = true;
    return LSIC_Packet_B(0,0,gm_.encrypt((bool)mpz_tstbit(b_.get_mpz_t(),0)));
}


LSIC_Packet_B LSIC_B::answerRound(const LSIC_Packet_A &pack)
{
    mpz_class tb;
    
    bool bi = (bool)mpz_tstbit(b_.get_mpz_t(),pack.index);
    
    if (bi) {
        tb = pack.tau;
    }else{
        tb = 1;
    }
    
    return LSIC_Packet_B(pack.index,gm_.reRand(tb),gm_.encrypt(bi));
}

void runProtocol(LSIC_A &party_a, LSIC_B &party_b)
{
    LSIC_Packet_A a_packet;
    LSIC_Packet_B b_packet = party_b.setupRound();
    
    bool state;
    
    state = party_a.answerRound(b_packet,&a_packet);
    
    while (!state) {
        b_packet = party_b.answerRound(a_packet);
        state = party_a.answerRound(b_packet, &a_packet);
    }
}