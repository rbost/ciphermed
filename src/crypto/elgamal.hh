#include <NTL/ZZ.h>
#include <vector>
#include <list>
#include <utility>

class ElGamal {
public:
    ElGamal(const std::vector<NTL::ZZ> &pk);
    std::vector<NTL::ZZ> pubkey() const { return { p, g, h }; }
    
    std::pair<NTL::ZZ,NTL::ZZ> encrypt(const NTL::ZZ &plaintext);
    std::pair<NTL::ZZ,NTL::ZZ> mult(const std::pair<NTL::ZZ,NTL::ZZ> &c0, const std::pair<NTL::ZZ,NTL::ZZ> &c1) const;

    void rand_gen(size_t niter = 100, size_t nmax = 1000);

protected:
    /* Public key */
    const NTL::ZZ p, g, h;

    /* Cached values */
    const NTL::ZZ q;
    const uint qbits;
    
    /* Pre-computed randomness */
    std::list< std::pair<NTL::ZZ,NTL::ZZ> > rqueue;
};

class ElGamal_priv : public ElGamal {    
public:
    ElGamal_priv(const std::vector<NTL::ZZ> &sk);
    std::vector<NTL::ZZ> privkey() const { return { q, g, h, x }; }
    
    NTL::ZZ decrypt(const std::pair<NTL::ZZ,NTL::ZZ> &ciphertext) const;

    static std::vector<NTL::ZZ> keygen(unsigned int qbits = 1024);

protected:
    /* Private key */
    const NTL::ZZ x;
};