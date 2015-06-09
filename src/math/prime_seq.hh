/*
 * Copyright 2013-2015 Raphael Bost
 *
 * This file is part of ciphermed.

 *  ciphermed is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 * 
 *  ciphermed is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 * 
 *  You should have received a copy of the GNU General Public License
 *  along with ciphermed.  If not, see <http://www.gnu.org/licenses/>. 2
 *
 */

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

