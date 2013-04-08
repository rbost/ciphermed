#include <cassert>
#include <iostream>
#include <fhe/she.hh>

using namespace std;

struct test_she_parameters {
    static const unsigned int LogQ       = 128;
    static const unsigned int LogT       = 15;
    static const unsigned int Sigma      = 16;
    static const unsigned int LogD       = 12;
    static const unsigned int LogMsgBase = 8;
};

typedef SHE<test_she_parameters> TestSHE;

int
main(int argc, char **argv)
{
    TestSHE sh;
    sh.SanityCheck();
    auto sk = sh.SKKeyGen();
    auto pk = sh.PKKeyGen(sk);
    mpz_class m(12345);
    auto ct = sh.encrypt(pk, m);
    auto m0 = sh.decrypt(sk, ct);
    assert(m == m0);

    //DefaultSHE sh;
    //auto sk = sh.SKKeyGen();
    //auto pk = sh.PKKeyGen(sk);
    //cout << "sk: " << sk << endl;

    //mpz_class m(12345);
    //auto ct = sh.encrypt(pk, m);
    //auto m0 = sh.decrypt(sk, ct);
    //assert(m == m0);

    return 0;
}

/* vim:set shiftwidth=4 ts=4 et: */
