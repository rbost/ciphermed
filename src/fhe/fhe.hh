#pragma once

#include <stdint.h>
#include <string>
#include <math/matrix.hh>
#include <fhe/errordist.hh>

struct PK {
    mpz_class b;
    mpz_class ap;
};

struct SK {
    std::vector<poly> s;
};

class FHE {
public:
    void Setup();

    PK PKKeyGen();
    SK SKKeyGen();

    static vec Enc(PK pk, int m);
    static int Dec(SK sk, vec);

private:
    uint N;
    ErrorDist chi;
    RLWEField Rq;
};



