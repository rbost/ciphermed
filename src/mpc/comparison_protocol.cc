
#include <mpc/lsic.hh>
#include <mpc/private_comparison.hh>
#include <typeinfo>

void runProtocol(Comparison_protocol_A *party_a, Comparison_protocol_B *party_b, gmp_randstate_t state)
{
    if(typeid(*party_a) == typeid(LSIC_A)) {
        runProtocol(reinterpret_cast<LSIC_A*>(party_a),reinterpret_cast<LSIC_B*>(party_b),state);
    }else if(typeid(*party_a) == typeid(Compare_A))
    {
        runProtocol(reinterpret_cast<Compare_A*>(party_a),reinterpret_cast<Compare_B*>(party_b),state);
    }

}
