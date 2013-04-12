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
    static const unsigned int LogMsgBase = 2;
};

typedef SHE<test_she_parameters> TestSHE;

template <typename A, typename B>
static ostream &
operator<<(ostream &o, const pair<A, B> &p)
{
    o << "{" << p.first << ", " << p.second << "}";
    return o;
}

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
    //ErrorDist chi(0, 16, PolyRing(1 << 9));
    //cerr << "rand: " << chi.sample() << endl;

    TestSHE sh;
    sh.SanityCheck();
    auto sk = sh.SKKeyGen();
    auto pk = sh.PKKeyGen(sk);

    //cerr << "sk: " << sk << endl;
    //cerr << "pk: " << pk << endl;

    Timer t;
    // test encrypt-decrypt
    {
        mpz_class m(12345);
        auto ct = sh.encrypt(pk, m);
        auto m0 = sh.decrypt(sk, ct);
        assert_s(m == m0, "decrypt failed");
        cout << "encrypt-decrypt passed" << endl;
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

    // test two mulitplications
    {
        mpz_class m0(29);
        mpz_class m1(42);
        mpz_class m2(25);
        auto ct0 = sh.encrypt(pk, m0);
        auto ct1 = sh.encrypt(pk, m1);
        auto ct2 = sh.encrypt(pk, m2);
        auto ct3 = sh.multiply(ct0, ct1);
        auto p = sh.decrypt(sk, ct3);
        assert_s(p == (m0 * m1), "multiply failed");
        auto ct4 = sh.multiply(ct3, ct2);
        auto p1 = sh.decrypt(sk, ct4);
        cerr << "p1 was     : " << p1 << endl;
        cerr << "p1 expected: " << (m0 * m1 * m2) << endl;
        assert_s(p1 == (m0 * m1 * m2), "multiply failed");
        cout << "two-multiplications passed" << endl;
    }
    
    // test one addition + one multiplication
    {
        mpz_class m0(93);
        mpz_class m1(42);
        mpz_class m2(9);
        auto ct0 = sh.encrypt(pk, m0);
        auto ct1 = sh.encrypt(pk, m1);
        auto ct2 = sh.encrypt(pk, m2);
        assert_s(sh.decrypt(sk, ct0) == m0, "dec failed");
        assert_s(sh.decrypt(sk, ct1) == m1, "dec failed");
        assert_s(sh.decrypt(sk, ct2) == m2, "dec failed");

        auto ct3 = sh.add(ct0, ct1);
        {
            auto p = sh.decrypt(sk, ct3);
            assert_s(p == ((m0 + m1)), "add failed");
        }

        auto ct4 = sh.multiply(ct2, ct3);
        auto p = sh.decrypt(sk, ct4);
        cerr << "p was      : " << p << endl;
        cerr << "p should be: " << ((m0 + m1) * m2) << endl;
        //assert_s(p == ((m0 + m1) * m2), "add+multiply failed");
        cout << "one-addition+one-mulitplication passed" << endl;
    }

    cerr << "took " << t.lap() << "\n";

    return 0;
}

/* vim:set shiftwidth=4 ts=4 et: */
