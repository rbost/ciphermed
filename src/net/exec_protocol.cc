#include <net/exec_protocol.hh>

#include <protobuf/protobuf_conversion.hh>

void exec_comparison_protocol_A(tcp::socket &socket, Comparison_protocol_A *comparator, unsigned int n_threads)
{
    if(typeid(*comparator) == typeid(LSIC_A)) {
        exec_lsic_A(socket, reinterpret_cast<LSIC_A*>(comparator));
    }else if(typeid(*comparator) == typeid(Compare_A)){
        exec_priv_compare_A(socket, reinterpret_cast<Compare_A*>(comparator),n_threads);
    }
}

void exec_lsic_A(tcp::socket &socket, LSIC_A *lsic)
{
    LSIC_Packet_A a_packet;
    LSIC_Packet_B b_packet;
    Protobuf::LSIC_A_Message a_message;
    Protobuf::LSIC_B_Message b_message;
    
    bool state;
   
    // response-request
    for (; ; ) {
        b_message = readMessageFromSocket<Protobuf::LSIC_B_Message>(socket);
        b_packet = convert_from_message(b_message);
        
        state = lsic->answerRound(b_packet,&a_packet);
        
        if (state) {
            return;
        }
        
        a_message = convert_to_message(a_packet);
        sendMessageToSocket(socket, a_message);
    }
}

void exec_priv_compare_A(tcp::socket &socket, Compare_A *comparator, unsigned int n_threads)
{
    vector<mpz_class> c_b(comparator->bit_length());
    
    // first get encrypted bits
    
    Protobuf::BigIntArray c_b_message = readMessageFromSocket<Protobuf::BigIntArray>(socket);
    c_b = convert_from_message(c_b_message);
    
    vector<mpz_class> c_w = comparator->compute_w(c_b);
    vector<mpz_class> c_sums = comparator->compute_sums(c_w);
    vector<mpz_class> c = comparator->compute_c(c_b,c_sums);
    vector<mpz_class> c_rand = comparator->rerandomize_parallel(c,n_threads);
    
    // we have to suffle
    comparator->shuffle(c_rand);
    
    // send the result
    
    Protobuf::BigIntArray c_rand_message = convert_to_message(c_rand);
    sendMessageToSocket(socket, c_rand_message);
    
    // wait for the encrypted result
    mpz_class c_t_prime;
    
    Protobuf::BigInt c_t_prime_message = readMessageFromSocket<Protobuf::BigInt>(socket);
    c_t_prime = convert_from_message(c_t_prime_message);
    
    comparator->unblind(c_t_prime);
}



void exec_comparison_protocol_B(tcp::socket &socket, Comparison_protocol_B *comparator)
{
    if(typeid(*comparator) == typeid(LSIC_B)) {
        exec_lsic_B(socket, reinterpret_cast<LSIC_B*>(comparator));
    }else if(typeid(*comparator) == typeid(Compare_B)){
        exec_priv_compare_B(socket, reinterpret_cast<Compare_B*>(comparator));
    }
}

void exec_lsic_B(tcp::socket &socket, LSIC_B *lsic)
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
    sendMessageToSocket(socket, b_message);
    
    cout << "LSIC setup sent" << endl;
    
    // wait for packets
    
    for (;b_packet.index < lsic->bitLength()-1; ) {
        a_message = readMessageFromSocket<Protobuf::LSIC_A_Message>(socket);
        a_packet = convert_from_message(a_message);
        
        b_packet = lsic->answerRound(a_packet);
        
        b_message = convert_to_message(b_packet);
        sendMessageToSocket(socket, b_message);
    }
    
    cout << "LSIC B Done" << endl;
}

void exec_priv_compare_B(tcp::socket &socket, Compare_B *comparator)
{
    //    boost::asio::streambuf output_buf;
    //    std::ostream output_stream(&output_buf);
    //    std::istream input_stream(&input_buf_);
    //    std::string line;
    
    vector<mpz_class> c(comparator->bit_length());
    
    
    // send the encrypted bits
    Protobuf::BigIntArray c_b_message = convert_to_message(comparator->encrypt_bits());
    sendMessageToSocket(socket, c_b_message);
    
    // wait for the answer from the client
    Protobuf::BigIntArray c_message = readMessageFromSocket<Protobuf::BigIntArray>(socket);
    c = convert_from_message(c_message);
    
    
    //    input_stream >> c;
    
    mpz_class c_t_prime = comparator->search_zero(c);
    
    // send the blinded result
    Protobuf::BigInt c_t_prime_message = convert_to_message(c_t_prime);
    sendMessageToSocket(socket, c_t_prime_message);
    
}


