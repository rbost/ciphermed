#include <net/linear_classifier_server.hh>
#include <protobuf/protobuf_conversion.hh>
#include <net/message_io.hh>


Linear_Classifier_Server::Linear_Classifier_Server(gmp_randstate_t state, unsigned int keysize, unsigned int lambda, const vector<mpz_class> &model, size_t bit_size)
: Server(state, keysize, lambda), model_(model), bit_size_(bit_size)
{

}

Server_session* Linear_Classifier_Server::create_new_server_session(tcp::socket *socket)
{
    return new Linear_Classifier_Server_session(this, rand_state_, n_clients_++, socket);
}

void Linear_Classifier_Server_session::run_session()
{
    exchange_all_keys();
    
    send_model();
    
    EncCompare_Helper helper = create_enc_comparator_helper(linear_server_->bit_size(), false);
    run_enc_comparison(helper);
}


void Linear_Classifier_Server_session::send_model()
{
    Protobuf::BigIntArray model_message = convert_to_message(linear_server_->model());
    sendMessageToSocket(*socket_, model_message);
}