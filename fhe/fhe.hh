#pragma once

#include <stdint.h>
#include <string>
#include <math/matrix.hh>

//distribution from which we sample random errors
class ErrorDist {
public:
    ErrorDist(uint lambda, uint mu, mpz_class q);
    poly sample();

private:
    mpz_class B;
    
};

struct PK {
    mpz_class b;
    mpz_class aa;
};

struct SK {
    std::vector<poly> s;
};

class FHE {
public:
    void Setup();

    PK PKKeyGen();
    SK SKKeyGen();
    


private:
    uint lambda;
    uint d; //degree of poly
    mpz_class q;
    uint N;
    uint mu;
    ErrorDist chi;
};



