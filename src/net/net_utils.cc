
#include <net/net_utils.hh>
#include <gmpxx.h>

using namespace std;

ostream& operator<<(ostream& out, const LSIC_Packet_A& p)
{
    out << p.index << "\n" << p.tau << "\n";
    return out;
}

ostream& operator<<(ostream& out, const LSIC_Packet_B& p)
{
    out << p.index << "\n" << p.tb << "\n" << p.bi << "\n";
    return out;
}


istream& operator>>(istream& in, LSIC_Packet_A& p)
{
    in >> p.index;
    in >> p.tau;
    return in;
}

istream& operator>>(istream& in, LSIC_Packet_B& p)
{
    in >> p.index;
    in >> p.tb;
    in >> p.bi;
    return in;
}
