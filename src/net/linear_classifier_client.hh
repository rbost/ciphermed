#pragma once

#include <vector>
#include <mpc/lsic.hh>
#include <mpc/private_comparison.hh>
#include <mpc/enc_comparison.hh>
#include <mpc/rev_enc_comparison.hh>
#include <mpc/linear_enc_argmax.hh>

#include <net/client.hh>

using namespace std;
class Linear_Classifier_Client : public Client{
public:
    Linear_Classifier_Client(boost::asio::io_service& io_service, gmp_randstate_t state, unsigned int nbits_gm, unsigned int lambda, const vector<mpz_class> &vals, size_t bit_size);

    void get_model();
    bool run();
    
protected:
    size_t bit_size_;
    vector<mpz_class> values_;
    vector<mpz_class> model_;
};
