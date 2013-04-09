#include <cassert>
#include <iostream>
#include <fhe/she.hh>

using namespace std;

// not cryptographically secure parameters
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
    //poly s({1, 1, 1, 1, 0});
    //poly a({1, 0, 1, 0, 0});
    //auto q = 16;
    //poly sa = s * a;
    //cerr << "s * a: " << sa << endl;
    //PolyRing R(4); // Z[x]/(x^4 + 1)
    //poly sa_r = R.reduce(sa);
    //cerr << "s * a mod (x^4 + 1): " << sa_r << endl;
    //poly sa_rq = sa_r.modshift(q);
    //cerr << "s * a mod (x^4 + 1) mod 16: " << sa_rq << endl;

    TestSHE sh;
    sh.SanityCheck();
    auto sk = sh.SKKeyGen();
    auto pk = sh.PKKeyGen(sk);

    // test encrypt-decrypt
    {
        mpz_class m(12345);
        auto ct = sh.encrypt(pk, m);
        auto m0 = sh.decrypt(sk, ct);
        assert_s(m == m0, "decrypt failed");
        cout << "encrypt-decrypt passed" << endl;
    }

    // test one mulitplication
    {
        mpz_class m0(123);
        mpz_class m1(5235);
        auto ct0 = sh.encrypt(pk, m0);
        auto ct1 = sh.encrypt(pk, m1);
        auto ct2 = sh.multiply(ct0, ct1);
        auto p = sh.decrypt(sk, ct2);
        assert_s(p == (m0 * m1), "multiply failed");
        cout << "one-multiplication passed" << endl;
    }

    // test one addition
    {
        mpz_class m0(12345);
        mpz_class m1(54321);
        auto ct0 = sh.encrypt(pk, m0);
        auto ct1 = sh.encrypt(pk, m1);
        auto ct2 = sh.add(ct0, ct1);
        auto p = sh.decrypt(sk, ct2);
        assert_s(p == (m0 + m1), "add failed");
        cout << "one-addition passed" << endl;
    }

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
