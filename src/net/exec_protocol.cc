#include <net/exec_protocol.hh>

#include <protobuf/protobuf_conversion.hh>
#include <net/net_utils.hh>

#include <mpc/change_encryption_scheme.hh>


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

void exec_rev_enc_comparison_owner(tcp::socket &socket, Rev_EncCompare_Owner &owner, unsigned int lambda)
{
    size_t l = owner.bit_length();
    mpz_class c_z(owner.setup(lambda));
    
    Protobuf::Enc_Compare_Setup_Message setup_message = convert_to_message(c_z,l);
    sendMessageToSocket(socket, setup_message);
    
    // the other party does some computation, we just have to run the comparator
    
    exec_comparison_protocol_A(socket, owner.comparator());
    
    Protobuf::BigInt c_z_l_message = readMessageFromSocket<Protobuf::BigInt>(socket);
    mpz_class c_z_l = convert_from_message(c_z_l_message);
    
    
    mpz_class c_t = owner.concludeProtocol(c_z_l);
    
    // send the last message to the server
    Protobuf::BigInt c_t_message = convert_to_message(c_t);
    sendMessageToSocket(socket, c_t_message);
}

void exec_rev_enc_comparison_helper(tcp::socket &socket, Rev_EncCompare_Helper &helper)
{
    // setup the helper if necessary
    if (!helper.is_set_up()) {
        Protobuf::Enc_Compare_Setup_Message setup_message = readMessageFromSocket<Protobuf::Enc_Compare_Setup_Message>(socket);
        if (setup_message.has_bit_length()) {
            helper.set_bit_length(setup_message.bit_length());
        }
        mpz_class c_z = convert_from_message(setup_message);
        
        helper.setup(c_z);
    }
    
    // now, we need to run the comparison protocol
    exec_comparison_protocol_B(socket, helper.comparator());
    
    
    mpz_class c_z_l(helper.get_c_z_l());
    
    Protobuf::BigInt c_z_l_message = convert_to_message(c_z_l);
    sendMessageToSocket(socket, c_z_l_message);
    
    // wait for the answer of the owner
    Protobuf::BigInt c_t_message = readMessageFromSocket<Protobuf::BigInt>(socket);
    mpz_class c_t = convert_from_message(c_t_message);
    helper.decryptResult(c_t);
}

void exec_enc_comparison_owner(tcp::socket &socket, EncCompare_Owner &owner, unsigned int lambda)
{
    // now run the protocol itself
    size_t l = owner.bit_length();
    mpz_class c_z(owner.setup(lambda));
    
    Protobuf::Enc_Compare_Setup_Message setup_message = convert_to_message(c_z,l);
    sendMessageToSocket(socket, setup_message);
    
    // the server does some computation, we just have to run the lsic
    
    exec_comparison_protocol_B(socket, owner.comparator());
    
    mpz_class c_r_l(owner.get_c_r_l());
    Protobuf::BigInt c_r_l_message = convert_to_message(c_r_l);
    sendMessageToSocket(socket, c_r_l_message);
    
    // wait for the answer of the owner
    Protobuf::BigInt c_t_message = readMessageFromSocket<Protobuf::BigInt>(socket);
    mpz_class c_t = convert_from_message(c_t_message);
    
    owner.decryptResult(c_t);
}

void exec_enc_comparison_helper(tcp::socket &socket, EncCompare_Helper &helper)
{
    // setup the helper if necessary
    if (!helper.is_set_up()) {
        Protobuf::Enc_Compare_Setup_Message setup_message = readMessageFromSocket<Protobuf::Enc_Compare_Setup_Message>(socket);
        if (setup_message.has_bit_length()) {
            helper.set_bit_length(setup_message.bit_length());
        }
        mpz_class c_z = convert_from_message(setup_message);
        
        helper.setup(c_z);
    }
    
    // now, we need to run the comparison protocol
    exec_comparison_protocol_A(socket, helper.comparator());
    
    Protobuf::BigInt c_r_l_message = readMessageFromSocket<Protobuf::BigInt>(socket);
    mpz_class c_r_l = convert_from_message(c_r_l_message);
    
    mpz_class c_t = helper.concludeProtocol(c_r_l);
    
    // send the last message to the server
    Protobuf::BigInt c_t_message = convert_to_message(c_t);
    sendMessageToSocket(socket, c_t_message);
}

void exec_linear_enc_argmax(tcp::socket &socket, Linear_EncArgmax_Owner &owner, function<Comparison_protocol_A*()> comparator_creator, unsigned int lambda)
{
    size_t k = owner.elements_number();
    for (size_t i = 0; i < (k-1); i++) {
        Comparison_protocol_A *comparator = comparator_creator();
        
        Rev_EncCompare_Owner rev_enc_owner = owner.create_current_round_rev_enc_compare_owner(comparator);
        
        exec_rev_enc_comparison_owner(socket, rev_enc_owner, lambda);
        
        mpz_class randomized_enc_max, randomized_value;
        owner.next_round(randomized_enc_max, randomized_value);
        
        // send the randomizations to the server
        sendIntToSocket(socket,randomized_enc_max);
        sendIntToSocket(socket,randomized_value);
        
        // get the server's response
        mpz_class new_enc_max, x, y;
        new_enc_max = readIntFromSocket(socket);
        x = readIntFromSocket(socket);
        y = readIntFromSocket(socket);
        
        owner.update_enc_max(new_enc_max, x, y);
    }
    
    mpz_class permuted_argmax;
    permuted_argmax = readIntFromSocket(socket);
    
    owner.unpermuteResult(permuted_argmax.get_ui());
}

void exec_linear_enc_argmax(tcp::socket &socket, Linear_EncArgmax_Helper &helper, function<Comparison_protocol_B*()> comparator_creator)
{
    size_t k = helper.elements_number();
    //    auto party_a_creator = [gm_ptr,p_ptr,nbits,randstate_ptr](){ return new Compare_A(0,nbits,*p_ptr,*gm_ptr,*randstate_ptr); };
    
    for (size_t i = 0; i < k - 1; i++) {
        //        cout << "Round " << i << endl;
//        Compare_B comparator(0,nbits,server_->paillier(),server_->gm());
        //        LSIC_B comparator(0,nbits,server_->gm());
        Comparison_protocol_B *comparator = comparator_creator();

        Rev_EncCompare_Helper rev_enc_helper = helper.rev_enc_compare_helper(comparator);
        
        exec_rev_enc_comparison_helper(socket, rev_enc_helper);
        
        mpz_class randomized_enc_max, randomized_value;
        
        // read the values sent by the client
        randomized_enc_max = readIntFromSocket(socket);
        randomized_value = readIntFromSocket(socket);
        
        // and send the server's response
        mpz_class new_enc_max, x, y;
        helper.update_argmax(rev_enc_helper.output(), randomized_enc_max, randomized_value, i+1, new_enc_max, x, y);
        
        sendIntToSocket(socket,new_enc_max);
        sendIntToSocket(socket,x);
        sendIntToSocket(socket,y);
    }
    
    cout << "Send result" << endl;
    mpz_class permuted_argmax = helper.permuted_argmax();
    sendIntToSocket(socket, permuted_argmax);
}

Ctxt exec_change_encryption_scheme_slots(tcp::socket &socket, const vector<mpz_class> &c_gm, GM &gm, const FHEPubKey& publicKey, const EncryptedArray &ea, gmp_randstate_t randstate)
{
    Change_ES_FHE_to_GM_slots_A switcher;
    vector<mpz_class> c_gm_blinded = switcher.blind(c_gm,gm,randstate, ea.size());
    
    send_int_array_to_socket(socket, c_gm_blinded);
    
    Ctxt c_blinded_fhe = read_fhe_ctxt_from_socket(socket, publicKey);
    
    
    Ctxt c_fhe = switcher.unblind(c_blinded_fhe,publicKey,ea);

    return c_fhe;
}

void exec_change_encryption_scheme_slots_helper(tcp::socket &socket, GM_priv &gm, const FHEPubKey &publicKey, const EncryptedArray &ea)
{
    vector<mpz_class> c_gm_blinded = read_int_array_from_socket(socket);
    Ctxt c_blinded_fhe = Change_ES_FHE_to_GM_slots_B::decrypt_encrypt(c_gm_blinded,gm,publicKey,ea);
    
    send_fhe_ctxt_to_socket(socket, c_blinded_fhe);
}
