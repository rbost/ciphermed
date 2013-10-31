#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <gmpxx.h>

#include <FHE.h>
#include <EncryptedArray.h>

#include <net/defs.hh>

#include <crypto/gm.hh>
#include <mpc/lsic.hh>
#include <mpc/private_comparison.hh>
#include <mpc/rev_enc_comparison.hh>
#include <mpc/enc_comparison.hh>

#include <math/util_gmp_rand.h>

#include <net/net_utils.hh>
#include <net/message_io.hh>
#include <util/util.hh>

#include <net/client.hh>

#include <protobuf/protobuf_conversion.hh>

using boost::asio::ip::tcp;

using namespace std;

static ZZX makeIrredPoly(long p, long d)
{
    assert(d >= 1);
    assert(ProbPrime(p));
    
    if (d == 1) return ZZX(1, 1); // the monomial X
    
    zz_pBak bak; bak.save();
    zz_p::init(p);
    return to_ZZX(BuildIrred_zz_pX(d));
}

Client::Client(boost::asio::io_service& io_service, gmp_randstate_t state, unsigned int nbits_gm, unsigned int lambda)
: socket_(io_service), gm_(GM_priv::keygen(state,nbits_gm),state), server_paillier_(NULL), server_gm_(NULL), server_fhe_pk_(NULL), lambda_(lambda)
{
    gmp_randinit_set(rand_state_, state);
    
    // generate a context. This one should be consisten with the server's one
    // i.e. m, p, r must be the same
    long p = FHE_p;
    long r = FHE_r
    long d = FHE_d;
    long c = FHE_c;
    long L = FHE_L;
    long w = FHE_w;
    long s = FHE_s;
    long k = FHE_k;
    long chosen_m = FHE_m;
    
    long m = FindM(k, L, c, p, d, s, chosen_m, true);
    fhe_context_ = new FHEcontext(m, p, r);
    buildModChain(*fhe_context_, L, c);

    // we suppose d > 0
    fhe_G_ = makeIrredPoly(FHE_p, FHE_d);
    
}

Client::~Client()
{
    if (server_fhe_pk_ != NULL) {
        delete server_fhe_pk_;
    }
    delete fhe_context_;
}


void Client::connect(boost::asio::io_service& io_service, const string& hostname)
{
    tcp::resolver resolver(io_service);
    tcp::resolver::query query(hostname, to_string( PORT ));
    tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
    boost::asio::connect(socket_, endpoint_iterator);
}


void Client::get_server_pk_gm()
{
    if (server_gm_) {
        return;
    }
    cout << "Request server's pubkey for GM" << endl;
    boost::asio::streambuf buff;
    std::ostream buff_stream(&buff);
    buff_stream << GET_GM_PK <<"\n\r\n";
    boost::asio::write(socket_, buff);
    
    Protobuf::GM_PK pk = readMessageFromSocket<Protobuf::GM_PK>(socket_);
    cout << "Received PK" << endl;
    server_gm_ = create_from_pk_message(pk,rand_state_);
}


void Client::get_server_pk_paillier()
{
    if (server_paillier_) {
        return;
    }
    cout << "Request server's pubkey for Paillier" << endl;
    boost::asio::streambuf buff;
    std::ostream buff_stream(&buff);
    buff_stream << GET_PAILLIER_PK <<"\n\r\n";
    boost::asio::write(socket_, buff);
    
    Protobuf::Paillier_PK pk = readMessageFromSocket<Protobuf::Paillier_PK>(socket_);
    cout << "Received PK" << endl;
    server_paillier_ = create_from_pk_message(pk,rand_state_);
}

void Client::get_server_pk_fhe()
{
    if (server_fhe_pk_) {
        return;
    }
    cout << "Request server's pubkey for FHE" << endl;
    boost::asio::streambuf buff;
    std::ostream buff_stream(&buff);
    buff_stream <<  GET_FHE_PK <<"\n\r\n";
    boost::asio::write(socket_, buff);
    string line;

    boost::asio::read_until(socket_, input_buf_, END_FHE_PK);
    std::istream input_stream(&input_buf_);
    
    do {
        getline(input_stream,line);
    } while (line != FHE_PK);
    // get the public key
    


    server_fhe_pk_ = new FHEPubKey(*fhe_context_);
    input_stream >> *server_fhe_pk_;
}


mpz_class Client::run_comparison_protocol_A(Comparison_protocol_A *comparator)
{
    if(typeid(*comparator) == typeid(LSIC_A)) {
        run_lsic_A(reinterpret_cast<LSIC_A*>(comparator));
    }else if(typeid(*comparator) == typeid(Compare_A)){
        run_priv_compare_A(reinterpret_cast<Compare_A*>(comparator));
    }
    
    return comparator->output();
}

mpz_class Client::run_lsic_A(LSIC_A *lsic)
{
    
    LSIC_Packet_A a_packet;
    LSIC_Packet_B b_packet;
    Protobuf::LSIC_A_Message a_message;
    Protobuf::LSIC_B_Message b_message;
    
    bool state;
    
    
    boost::asio::streambuf out_buff;
    std::ostream output_stream(&out_buff);
    string line;
    
    
    // response-request
    for (; ; ) {
        b_message = readMessageFromSocket<Protobuf::LSIC_B_Message>(socket_);
        b_packet = convert_from_message(b_message);

        state = lsic->answerRound(b_packet,&a_packet);
                
        if (state) {
            return lsic->output();
        }
        
        a_message = convert_to_message(a_packet);
        sendMessageToSocket(socket_, a_message);
    }
 }

mpz_class Client::run_priv_compare_A(Compare_A *comparator)
{
    boost::asio::streambuf out_buff;
    std::ostream output_stream(&out_buff);
    string line;
    std::istream input_stream(&input_buf_);
    
    vector<mpz_class> c_b(comparator->bit_length());
    
    // first get encrypted bits
    
    Protobuf::BigIntArray c_b_message = readMessageFromSocket<Protobuf::BigIntArray>(socket_);
    c_b = convert_from_message(c_b_message);
    
    vector<mpz_class> c_w = comparator->compute_w(c_b);
    vector<mpz_class> c_sums = comparator->compute_sums(c_w);
    vector<mpz_class> c = comparator->compute_c(c_b,c_sums);
    vector<mpz_class> c_rand = comparator->rerandomize(c);
    
    // we have to suffle
    random_shuffle(c_rand.begin(),c_rand.end(),[this](int n){ return gmp_urandomm_ui(rand_state_,n); });
    
    // send the result
    
    Protobuf::BigIntArray c_rand_message = convert_to_message(c_rand);
    sendMessageToSocket(socket_, c_rand_message);
    
    // wait for the encrypted result
    mpz_class c_t_prime;

    Protobuf::BigInt c_t_prime_message = readMessageFromSocket<Protobuf::BigInt>(socket_);
    c_t_prime = convert_from_message(c_t_prime_message);

    comparator->unblind(c_t_prime);

    return comparator->output();

    
}



void Client::run_comparison_protocol_B(Comparison_protocol_B *comparator)
{
    if(typeid(*comparator) == typeid(LSIC_B)) {
        run_lsic_B(reinterpret_cast<LSIC_B*>(comparator));
    }else if(typeid(*comparator) == typeid(Compare_B)){
        run_priv_compare_B(reinterpret_cast<Compare_B*>(comparator));
    }
}

void Client::run_lsic_B(LSIC_B *lsic)
{
    cout << "Start LSIC B" << endl;
    boost::asio::streambuf output_buf;
    std::ostream output_stream(&output_buf);
    std::string line;
    
    LSIC_Packet_A a_packet;
    LSIC_Packet_B b_packet = lsic->setupRound();
    Protobuf::LSIC_A_Message a_message;
    Protobuf::LSIC_B_Message b_message;
    
    b_message = convert_to_message(b_packet);
    sendMessageToSocket(socket_, b_message);
    
    cout << "LSIC setup sent" << endl;
    
    // wait for packets
    
    for (;b_packet.index < lsic->bitLength()-1; ) {
        a_message = readMessageFromSocket<Protobuf::LSIC_A_Message>(socket_);
        a_packet = convert_from_message(a_message);
        
        b_packet = lsic->answerRound(a_packet);
        
        b_message = convert_to_message(b_packet);
        sendMessageToSocket(socket_, b_message);
    }
    
    cout << "LSIC B Done" << endl;
}

void Client::run_priv_compare_B(Compare_B *comparator)
{
//    boost::asio::streambuf output_buf;
//    std::ostream output_stream(&output_buf);
//    std::istream input_stream(&input_buf_);
//    std::string line;
    
    vector<mpz_class> c(comparator->bit_length());
    
    
    // send the encrypted bits
    Protobuf::BigIntArray c_b_message = convert_to_message(comparator->encrypt_bits_fast());
    sendMessageToSocket(socket_, c_b_message);
    
    // wait for the answer from the client
    Protobuf::BigIntArray c_message = readMessageFromSocket<Protobuf::BigIntArray>(socket_);
    c = convert_from_message(c_message);
    
    
    //    input_stream >> c;
    
    mpz_class c_t_prime = comparator->search_zero(c);
    
    // send the blinded result
    Protobuf::BigInt c_t_prime_message = convert_to_message(c_t_prime);
    sendMessageToSocket(socket_, c_t_prime_message);
    
}

mpz_class Client::test_lsic(const mpz_class &a, size_t l)
{
    if (!has_gm_pk()) {
        get_server_pk_gm();
    }
    // send the start message
    boost::asio::streambuf out_buff;
    std::ostream output_stream(&out_buff);
    output_stream << START_LSIC << "\n\r\n";
    boost::asio::write(socket_, out_buff);
    
    LSIC_A lsic(a,l,*server_gm_);
    return run_lsic_A(&lsic);
}

mpz_class Client::test_compare(const mpz_class &b, size_t l)
{
    if (!has_gm_pk()) {
        get_server_pk_gm();
    }
    if (!has_paillier_pk()) {
        get_server_pk_paillier();
    }
    // send the start message
    boost::asio::streambuf out_buff;
    std::ostream output_stream(&out_buff);
    output_stream << START_PRIV_COMP << "\n\r\n";
    boost::asio::write(socket_, out_buff);
    
    Compare_A comparator(b,l,*server_paillier_,*server_gm_,rand_state_);
    return run_priv_compare_A(&comparator);
}


void Client::test_rev_enc_compare(size_t l)
{
    mpz_class a, b;
    mpz_urandom_len(a.get_mpz_t(), rand_state_, l);
    mpz_urandom_len(b.get_mpz_t(), rand_state_, l);
    
//    cout << "a = " << a << endl;
//    cout << "b = " << b << endl;

    get_server_pk_gm();
    get_server_pk_paillier();
    
    mpz_class c_a, c_b;

    run_rev_enc_compare(server_paillier_->encrypt(a),server_paillier_->encrypt(b),l);
    
    cout << "\nResult should be " << (a < b) << endl;
}

// we suppose that the client already has the server's public key for Paillier
void Client::run_rev_enc_compare(const mpz_class &a, const mpz_class &b, size_t l)
{
    assert(has_paillier_pk());
    assert(has_gm_pk());
    
    LSIC_A lsic(0,l,*server_gm_);
    Rev_EncCompare_Owner owner(a,b,l,*server_paillier_,&lsic,rand_state_);
    run_rev_enc_comparison(owner);
}

void Client::run_rev_enc_comparison(Rev_EncCompare_Owner &owner)
{
    assert(has_paillier_pk());
    assert(has_gm_pk());
 
    size_t l = owner.bit_length();
    mpz_class c_z(owner.setup(lambda_));
    cout << "l = " << l << endl;

    
    boost::asio::streambuf out_buff;
    std::ostream output_stream(&out_buff);
    string line;
    
    // send the start message
    output_stream << START_REV_ENC_COMPARE << "\n";
    output_stream << "\r\n";

    boost::asio::write(socket_, out_buff);
    Protobuf::Enc_Compare_Setup_Message setup_message = convert_to_message(c_z,l);
    sendMessageToSocket(socket_, setup_message);

    // the server does some computation, we just have to run the lsic
    
    run_comparison_protocol_A(owner.comparator());
    
    Protobuf::BigInt c_z_l_message = readMessageFromSocket<Protobuf::BigInt>(socket_);
    mpz_class c_z_l = convert_from_message(c_z_l_message);


    mpz_class c_t = owner.concludeProtocol(c_z_l);
    
    // send the last message to the server
    Protobuf::BigInt c_t_message = convert_to_message(c_t);
    sendMessageToSocket(socket_, c_t_message);
}

void Client::disconnect()
{
    cout << "Disconnect" << endl;
    
    boost::asio::streambuf buff;
    std::ostream buff_stream(&buff);
    buff_stream << DISCONNECT << "\n\r\n";
    boost::asio::write(socket_, buff);
}



void Client::test_decrypt_gm(const mpz_class &c)
{
    boost::asio::streambuf buff;
    std::ostream buff_stream(&buff);
    
    buff_stream << DECRYPT_GM << "\n"<< c << "\n\r\n";
    boost::asio::write(socket_, buff);
    
}

void Client::test_fhe()
{
    get_server_pk_fhe();
    
    EncryptedArray ea(server_fhe_pk_->getContext(), fhe_G_);
    PlaintextArray p0(ea);
    p0.encode(0);
    p0.print(cout);

    Ctxt c0(*server_fhe_pk_);
    ea.encrypt(c0, *server_fhe_pk_, p0);
    
    boost::asio::streambuf buff;
    std::ostream buff_stream(&buff);
    
    buff_stream << DECRYPT_FHE << "\n"<< c0 << "\n\r\n";
    boost::asio::write(socket_, buff);
}


int main(int argc, char* argv[])
{
    try
    {
        if (argc != 2)
        {
            std::cerr << "Usage: client <host>" << std::endl;
            return 1;
        }
        
        boost::asio::io_service io_service;

        gmp_randstate_t randstate;
        gmp_randinit_default(randstate);
        gmp_randseed_ui(randstate,time(NULL));
        

        Client client(io_service, randstate,1024,100);

        string hostname(argv[1]);
        client.connect(io_service, hostname);

        // server has b = 20
/*
        ScopedTimer *t_lsic = new ScopedTimer("LSIC");
        mpz_class res_lsic = client.test_lsic(40,100);
        delete t_lsic;
        client.test_decrypt_gm(res_lsic);

        ScopedTimer *t_comp = new ScopedTimer("Comp");
        mpz_class res_comp = client.test_compare(40,100);
        delete t_comp;
        client.test_decrypt_gm(res_comp);
        
*/
        
        
        
        client.test_rev_enc_compare(5);
        
//        client.test_fhe();
        
        client.disconnect();
    
    }
    catch (std::exception& e)
    {
        std::cout << "Exception: " << e.what() << std::endl;
    }
    
    return 0;
}