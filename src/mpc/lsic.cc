#include <mpc/lsic.hh>
#include <algorithm>                
#include <assert.h>

using namespace std;
using namespace NTL;


/* LSIC_Packet_A */

LSIC_Packet_A::LSIC_Packet_A(size_t i, const ZZ &c)
: index(i), tau(c)
{
}

LSIC_Packet_A::LSIC_Packet_A()
: index(0), tau(ZZ::zero())
{
}

/* LSIC_Packet_B */

LSIC_Packet_B::LSIC_Packet_B(size_t i, const ZZ &c1, const ZZ &c2)
: index(i), tb(c1), bi(c2)
{
}

/* LSIC_A */

LSIC_A::LSIC_A(const ZZ &x,const size_t &l,const vector<ZZ> &gm_pk)
: a_(x), bit_length_(l), gm_(gm_pk), i_(0)
{
}

ZZ LSIC_A::blindingStep_()
{
    c_ = (bool)RandomLen_long(1);
    ZZ tau;
    
    if (c_) {
        tau = gm_.XOR(gm_.encrypt(1),t_);
    }else{
        tau = t_;
    }
    
    return gm_.reRand(tau);
}

void LSIC_A::updateStep_(const LSIC_Packet_B &pack)
{
    ZZ tb2;
    
    if (c_) {
        tb2 = gm_.XOR(pack.tb,pack.bi);
    }else{
        tb2 = pack.tb;
    }
    
    if (bit(a_,i_) == 0) {
        t_ = gm_.XOR(t_,gm_.XOR(pack.bi,tb2));
    }else{
        t_ = tb2;
    }
}

LSIC_Packet_A LSIC_A::firstRound_(const LSIC_Packet_B &pack)
{        
    bool a0 = (bool)bit(a_,0);
    
    if (a0) {
        t_ = to_ZZ(1);
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
    
    ZZ tau = blindingStep_();
    
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
        updateStep_(pack);
        *outputPacket = LSIC_Packet_A(i_,blindingStep_());
        return false;
    }
}

ZZ LSIC_A::output() const
{
    assert(i_ == bit_length_);
    return t_;
}

/* LSIC_B */

LSIC_B::LSIC_B(const ZZ &y,const size_t l,const vector<ZZ> &gm_sk)
: b_(y), bit_length_(l), gm_(gm_sk)
{
}

LSIC_B::LSIC_B(const ZZ &y,const size_t l, unsigned int key_size)
: b_(y), bit_length_(l), gm_(GM_priv::keygen(key_size))
{
}

LSIC_Packet_B LSIC_B::setupRound()
{
    return LSIC_Packet_B(0,ZZ::zero(),gm_.encrypt((bool)bit(b_,0)));
}


LSIC_Packet_B LSIC_B::answerRound(const LSIC_Packet_A &pack)
{
    ZZ tb;
    
    bool bi = (bool)bit(b_,pack.index);
    
    if (bi) {
        tb = pack.tau;
    }else{
        tb = to_ZZ(1);
    }
    
    return LSIC_Packet_B(pack.index,gm_.reRand(tb),gm_.encrypt(bi));
}