#include <cassert>
#include <iostream>
#include <fhe/she.hh>

using namespace std;

int
main(int argc, char **argv)
{
    DefaultSHE sh;
    auto sk = sh.SKKeyGen();
    auto pk = sh.PKKeyGen(sk);
    //cout << "sk: " << sk << endl;

    mpz_class m(12345);
    auto ct = sh.encrypt(pk, m);
    //auto m0 = sh.decrypt(sk, ct);
    //assert(m == m0);
    return 0;
}

/* vim:set shiftwidth=4 ts=4 et: */
