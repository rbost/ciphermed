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
    return 0;
}

/* vim:set shiftwidth=4 ts=4 et: */
