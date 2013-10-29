
#ifndef NETUTILS_ENCODE__INCLUDED
#define NETUTILS_ENCODE__INCLUDED


const unsigned HEADER_SIZE = 4;
typedef unsigned char byte;

template <class CharContainer>
std::string show_hex(const CharContainer& c)
{
    std::string hex;
    char buf[16];
    typename CharContainer::const_iterator i;
    for (i = c.begin(); i != c.end(); ++i) {
        std::sprintf(buf, "%02X ", static_cast<unsigned>(*i) & 0xFF);
        hex += buf;
    }
    return hex;
}

unsigned decode_header(const vector<byte>& buf)
{
    if (buf.size() < HEADER_SIZE)
    return 0;
    unsigned msg_size = 0;
    for (unsigned i = 0; i < HEADER_SIZE; ++i)
    msg_size = msg_size * 256 + (static_cast<unsigned>(buf[i]) & 0xFF);
    return msg_size;
}

void encode_header(vector<byte>& buf, unsigned size)
{
    assert(buf.size() >= HEADER_SIZE);
    buf[0] = static_cast<boost::uint8_t>((size >> 24) & 0xFF);
    buf[1] = static_cast<boost::uint8_t>((size >> 16) & 0xFF);
    buf[2] = static_cast<boost::uint8_t>((size >> 8) & 0xFF);
    buf[3] = static_cast<boost::uint8_t>(size & 0xFF);
}

template <class T>
T readMessageFromSocket(tcp::socket &socket) {
    vector<byte> m_readbuf;
    m_readbuf.resize(HEADER_SIZE);
    boost::asio::read(socket, boost::asio::buffer(m_readbuf));
//    (cerr << "Got header!\n");
//    (cerr << show_hex(m_readbuf) << endl);
    unsigned msg_len = decode_header(m_readbuf);
    
    m_readbuf.resize(HEADER_SIZE + msg_len);
    
    boost::asio::mutable_buffers_1 buf = boost::asio::buffer(&m_readbuf[HEADER_SIZE], msg_len);
    boost::asio::read(socket, buf);
    
//    (cerr << "Got body!\n");
//    (cerr << show_hex(m_readbuf) << endl);
    
    T m;
    m.ParseFromArray(&m_readbuf[HEADER_SIZE], m_readbuf.size() - HEADER_SIZE);
    
    return m;
}

template <class T>
void sendMessageToSocket(tcp::socket &socket, const T& msg) {
    vector<byte> writebuf;
    unsigned msg_size = msg.ByteSize();
    
    writebuf.resize(HEADER_SIZE + msg_size);
    encode_header(writebuf, msg_size);
    
    if (!msg.SerializeToArray(&writebuf[HEADER_SIZE], msg_size)) {
        cerr << "Error when serializing" << endl;
        return;
    }
    boost::asio::write(socket, boost::asio::buffer(writebuf));
}

#endif