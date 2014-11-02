//
//  oblivious_transfer.h
//  ciphermed-proj
//
//  Created by Raphael Bost on 31/10/2014.
//
//

#ifndef __ciphermed_proj__oblivious_transfer__
#define __ciphermed_proj__oblivious_transfer__

#include <stdio.h>

#include <boost/asio.hpp>
using boost::asio::ip::tcp;

#include <gmpxx.h>


struct security_parameters {
    /* The field size in bytes */
    int field_size;
    mpz_t p;
    mpz_t g;
    mpz_t q;
    gmp_randstate_t	rnd_state;
};


#include <openssl/evp.h>
#include <openssl/sha.h>
//#include <openssl/aes.h>

#define AES_KEY_CTX EVP_CIPHER_CTX
#define HASH_INIT(sha) SHA_Init(sha)
#define HASH_UPDATE(sha, buf, bufsize) SHA_Update(sha, buf, bufsize)
#define HASH_FINAL(sha, sha_buf) SHA_Final(sha_buf, sha)

#define SHA1_BYTES				20


typedef struct security_parameters NPState;

class ObliviousTransfer {
    NPState m_NPState;
    int m_SecParam;

    bool GMP_Init(int secparam);
    bool GMP_Cleanup();   
    
    void mpz_export_padded(char* pBufIdx, int field_size, mpz_t to_export);

public:
    ObliviousTransfer(int secparam){ GMP_Init(secparam); };
    
    bool sender(int nOTs, char *messages, tcp::socket &socket);
    bool receiver(int nOTs, int *choices, char *ret, tcp::socket &socket);
};

#endif /* defined(__ciphermed_proj__oblivious_transfer__) */
