#pragma once

#include <exception>

// primes are generated in sequence, starting at 2,
// and up until (2*PRIME_BND+1)^2, which is less than NBITS_MAX.

#define NBITS_MAX 50
#if (NBITS_MAX > 30)
#define PRIME_BND ((1L << 14) - 1)
#else
#define PRIME_BND ((1L << (NBITS_MAX/2-1)) - 1)
#endif
class PrimeSeq {
    
    
    char *movesieve;
    char *movesieve_mem;
    long pindex;
    long pshift;
    long exhausted;
    
    public:
    
    PrimeSeq();
    ~PrimeSeq();
    
    long next();
    // returns next prime in the sequence.
    // returns 0 if list of small primes is exhausted.
    
    void reset(long b);
    // resets generator so that the next prime in the sequence
    // is the smallest prime >= b.
    
    private:
    
    PrimeSeq(const PrimeSeq&);        // disabled
    void operator=(const PrimeSeq&);  // disabled
    
    // auxilliary routines
    
    void start();
    void shift(long);
    
};

