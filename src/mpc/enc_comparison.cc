#include <vector>
#include <gmpxx.h>

#include <mpc/enc_comparison.hh>

using namespace std;


EncCompare_Owner::EncCompare_Owner(const mpz_class &v_a, const mpz_class &v_b, const size_t &l, const vector<mpz_class> pk_p, const vector<mpz_class> &sk_gm, gmp_randstate_t state)
: a_(v_a), b_(v_b), bit_length_(l), paillier_(pk_p,state), lsic_(0,bit_length_,sk_gm,state), two_l_(0), is_protocol_done_(false)
{
    gmp_randinit_set(randstate_, state);
    mpz_setbit(two_l_.get_mpz_t(),bit_length_);
}

EncCompare_Owner::EncCompare_Owner(const mpz_class &v_a, const mpz_class &v_b, const size_t &l, const vector<mpz_class> pk_p, gmp_randstate_t state, unsigned int key_size)
: a_(v_a), b_(v_b), bit_length_(l), paillier_(pk_p,state), lsic_(0,bit_length_,state,key_size), two_l_(0), is_protocol_done_(false)
{
    gmp_randinit_set(randstate_, state);
    mpz_setbit(two_l_.get_mpz_t(),bit_length_);
}

mpz_class EncCompare_Owner::setup(unsigned int lambda)
{
    mpz_class x, r, z, c;
    
    x = paillier_.add(b_,paillier_.encrypt(two_l_));
    x = paillier_.sub(x,a_);
    
    mpz_urandomb(r.get_mpz_t(), randstate_, lambda+bit_length_);
    z = paillier_.add(x,paillier_.encrypt(r));

    c = r % two_l_;
    lsic_.set_value(c);
    
    bool r_l = (bool)mpz_tstbit(r.get_mpz_t(),bit_length_);
    c_r_l_ = lsic_.gm().encrypt(r_l);

    
//    cout << "l = " << bit_length_ << endl;
//    cout << "Owner setup: \nr = " << r << "\t" << r.get_str(2) << "\nr_l = " << r_l << "\nc = " << c << endl;
//    cout << "2^l = " << two_l_ << endl;
    return z;
}

void EncCompare_Owner::decryptResult(const mpz_class &c_t)
{
    is_protocol_done_ = true;
    t_ = lsic_.gm().decrypt(c_t);
}


EncCompare_Helper::EncCompare_Helper(const size_t &l, const std::vector<mpz_class> sk_p, const std::vector<mpz_class> &pk_gm, gmp_randstate_t state)
: bit_length_(l), paillier_(sk_p,state), lsic_(0,bit_length_,pk_gm,state), two_l_(0)
{
    gmp_randinit_set(randstate_, state);
    mpz_setbit(two_l_.get_mpz_t(),bit_length_);
}

EncCompare_Helper::EncCompare_Helper(const size_t &l, const std::vector<mpz_class> &pk_gm, gmp_randstate_t state, unsigned int key_size)
: bit_length_(l), paillier_(Paillier_priv::keygen(state,key_size),state), lsic_(0,bit_length_,pk_gm,state), two_l_(0)
{
    gmp_randinit_set(randstate_, state);
    mpz_setbit(two_l_.get_mpz_t(),bit_length_);
}

void EncCompare_Helper::setup(const mpz_class &c_z)
{
    mpz_class z = paillier_.decrypt(c_z);
    mpz_class d = z % two_l_;
    lsic_.set_value(d);
    
    bool z_l = (bool)mpz_tstbit(z.get_mpz_t(),bit_length_);
    c_z_l_ = lsic_.gm().encrypt(z_l);
    
//    cout << "Helper setup: \nz = " << z << "\t" << z.get_str(2)<< "\nz_l = " << z_l << "\nd = " << d << endl;

}

mpz_class EncCompare_Helper::concludeProtocol(const mpz_class &c_r_l)
{
    mpz_class c_t_prime = lsic_.output();
    mpz_class c_t = lsic_.gm().XOR(c_t_prime,c_r_l);
    c_t = lsic_.gm().XOR(c_t,c_z_l_);

    return c_t;
}

