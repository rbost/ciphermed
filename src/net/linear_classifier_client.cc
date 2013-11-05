#include <mpc/lsic.hh>
#include <mpc/private_comparison.hh>
#include <mpc/enc_comparison.hh>
#include <mpc/rev_enc_comparison.hh>
#include <mpc/linear_enc_argmax.hh>

#include <net/linear_classifier_client.hh>

#include <protobuf/protobuf_conversion.hh>
#include <net/message_io.hh>



Linear_Classifier_Client::Linear_Classifier_Client(boost::asio::io_service& io_service, gmp_randstate_t state, unsigned int nbits_gm, unsigned int lambda, const vector<mpz_class> &vals, size_t bit_size)
: Client(io_service,state,nbits_gm,lambda), bit_size_(bit_size),values_(vals)
{
    
}

void Linear_Classifier_Client::get_model()
{
    Protobuf::BigIntArray model_message = readMessageFromSocket<Protobuf::BigIntArray>(socket_);
    model_ = convert_from_message(model_message);
}

bool Linear_Classifier_Client::run()
{
    // get public keys
    exchange_all_keys();
    
    // get the model
    get_model();
    
    // compute the encrypted dot product
    mpz_class v = 1;
    
    for (size_t i = 0; i < values_.size(); i++) {
        v = server_paillier_->add(v, server_paillier_->constMult(values_[i],model_[i]));
    }
    
    // build the comparator over encrypted data
    EncCompare_Owner owner = create_enc_comparator_owner(bit_size_,false);
    owner.set_input(v, model_[model_.size()-1]);
    
    bool result = run_enc_comparison(owner);
    
    return result;
}
