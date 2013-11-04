
#include <net/net_utils.hh>
#include <gmpxx.h>
#include <string>

#include <net/defs.hh>

#include <boost/asio.hpp>
#include <net/message_io.hh>

#include <protobuf/protobuf_conversion.hh>

using namespace std;

ostream& operator<<(ostream& out, const LSIC_Packet_A& p)
{
    out << p.index << "\n" << p.tau.get_str(BASE) << "\n";
//    out << p.index << "\n" << p.tau << "\n";
    return out;
}

ostream& operator<<(ostream& out, const LSIC_Packet_B& p)
{
    out << p.index << "\n" << p.tb.get_str(BASE) << "\n" << p.bi.get_str(BASE) << "\n";
//    out << p.index << "\n" << p.tb << "\n" << p.bi << "\n";
    return out;
}


istream& operator>>(istream& in, LSIC_Packet_A& p)
{
//    in >> p.index;
//    in >> p.tau;

    std::string line;
    
    in >> line;
    p.index = strtoul (line.c_str(), NULL, 0);
//    in >> line;
//    p.tau.set_str(line,BASE);
    parseInt(in,p.tau,BASE);

    return in;
}

istream& operator>>(istream& in, LSIC_Packet_B& p)
{
//    in >> p.index;
//    in >> p.tb;
//    in >> p.bi;
    std::string line;

    in >> line;
    p.index = strtoul (line.c_str(), NULL, 0);
    
    parseInt(in,p.tb,BASE);
    parseInt(in,p.bi,BASE);
//    in >> line;
//    p.tb.set_str(line,BASE);
//    in >> line;
//    p.bi.set_str(line,BASE);

    
    return in;
}

ostream& operator<<(ostream& out, const vector<mpz_class> &v)
{
    for (auto it = v.begin(); it != v.end(); it++) {
        out << *it << "\n";
    }
    return out;
}

istream& operator>>(istream& in, vector<mpz_class> &v)
{
    for (auto it = v.begin(); it != v.end(); it++) {
        in >> (*it);
    }
    return in;
}


istream& parseInt(istream& in, mpz_class &i, int base)
{
    std::string line;
    in >> line;
    i.set_str(line,base);
    return in;
}

void sendIntToSocket(boost::asio::ip::tcp::socket &socket, const mpz_class& m)
{
    Protobuf::BigInt msg = convert_to_message(m);
    sendMessageToSocket(socket,msg);
}

mpz_class readIntFromSocket(boost::asio::ip::tcp::socket &socket)
{
    Protobuf::BigInt msg = readMessageFromSocket<Protobuf::BigInt>(socket);
    return convert_from_message(msg);
}
