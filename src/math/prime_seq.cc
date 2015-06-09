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

#include <math/prime_seq.hh>
#include <cstdlib>
#include <cmath>


class PrimeSeq_memory_exception: public std::exception
{
    virtual const char* what() const throw()
    {
        return "PrimeSeq: not enough memory";
    }
} ps_memory_exception;



PrimeSeq::PrimeSeq()
{
    movesieve = 0;
    movesieve_mem = 0;
    pshift = -1;
    pindex = -1;
    exhausted = 0;
}

PrimeSeq::~PrimeSeq()
{
    if (movesieve_mem)
    free(movesieve_mem);
}

long PrimeSeq::next()
{
    if (exhausted) {
        return 0;
    }
    
    if (pshift < 0) {
        shift(0);
        return 2;
    }
    
    for (;;) {
        char *p = movesieve;
        long i = pindex;
        
        while ((++i) < PRIME_BND) {
            if (p[i]) {
                pindex = i;
                return pshift + 2 * i + 3;
            }
        }
        
        long newshift = pshift + 2*PRIME_BND;
        
        if (newshift > 2 * PRIME_BND * (2 * PRIME_BND + 1)) {
            /* end of the road */
            exhausted = 1;
            return 0;
        }
        
        shift(newshift);
    }
}

static char *lowsieve = 0;

void PrimeSeq::shift(long newshift)
{
    long i;
    long j;
    long jstep;
    long jstart;
    long ibound;
    char *p;
    
    if (!lowsieve)
    start();
    
    pindex = -1;
    exhausted = 0;
    
    if (newshift < 0) {
        pshift = -1;
        return;
    }
    
    if (newshift == pshift) return;
    
    pshift = newshift;
    
    if (pshift == 0) {
        movesieve = lowsieve;
    }
    else {
        if (!movesieve_mem) {
//            movesieve_mem = (char *) NTL_MALLOC(PRIME_BND, 1, 0);
            movesieve_mem = (char *) malloc(PRIME_BND);
            if (!movesieve_mem)
            throw ps_memory_exception;
        }
        
        p = movesieve = movesieve_mem;
        for (i = 0; i < PRIME_BND; i++)
        p[i] = 1;
        
        jstep = 3;
        ibound = pshift + 2 * PRIME_BND + 1;
        for (i = 0; jstep * jstep <= ibound; i++) {
            if (lowsieve[i]) {
                if (!((jstart = (pshift + 2) / jstep + 1) & 1))
                jstart++;
                if (jstart <= jstep)
                jstart = jstep;
                jstart = (jstart * jstep - pshift - 3) / 2;
                for (j = jstart; j < PRIME_BND; j += jstep)
                p[j] = 0;
            }
            jstep += 2;
        }
    }
}


void PrimeSeq::start()
{
    long i;
    long j;
    long jstep;
    long jstart;
    long ibnd;
    char *p;
    
    p = lowsieve = (char *) malloc(PRIME_BND);
    if (!p)
    throw ps_memory_exception;
    
    for (i = 0; i < PRIME_BND; i++)
    p[i] = 1;
    
    jstep = 1;
    jstart = -1;
    ibnd = (floor(sqrt(2 * PRIME_BND + 1)) - 3) / 2;
    for (i = 0; i <= ibnd; i++) {
        jstart += 2 * ((jstep += 2) - 1);
        if (p[i])
        for (j = jstart; j < PRIME_BND; j += jstep)
        p[j] = 0;
    }
}

void PrimeSeq::reset(long b)
{
    if (b > (2*PRIME_BND+1)*(2*PRIME_BND+1)) {
        exhausted = 1;
        return;
    }
    
    if (b <= 2) {
        shift(-1);
        return;
    }
    
    if ((b & 1) == 0) b++;
    
    shift(((b-3) / (2*PRIME_BND))* (2*PRIME_BND));
    pindex = (b - pshift - 3)/2 - 1;
}