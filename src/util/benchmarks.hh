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

#ifdef BENCHMARK

#include <util/util.hh>



class BenchTimer : public ResumableTimer {
public:
    static BenchTimer* shared_instance__;

public:
    static void create()
    {
        if(!shared_instance__){
            shared_instance__ = new BenchTimer("Benchmark Timer");
        }
    }

    static BenchTimer* shared_BenchTimer()
    {
        assert(shared_instance__ != NULL);
        return shared_instance__;
    }
    
protected:
    BenchTimer(const std::string &m) : ResumableTimer(m) {};
    
};

class IOBenchmark {
private:
    static unsigned long byte_count__;
    static unsigned long interaction_count__;
    
public:
    static void reset()
    {
        byte_count__ = 0;
        interaction_count__ = 0;
    }
    
    static void exchanged_bytes(unsigned long n)
    {
        byte_count__ += n;
    }
    
    static unsigned long byte_count()
    {
        return byte_count__;
    }
    
    static void interaction()
    {
        interaction_count__++;
    }
    
    static unsigned long interaction_count()
    {
        return interaction_count__;
    }
};

#define BENCHMARK_INIT BenchTimer::create();
#define PAUSE_BENCHMARK BenchTimer::shared_BenchTimer()->pause();
#define RESUME_BENCHMARK BenchTimer::shared_BenchTimer()->resume();
#define GET_BENCHMARK_TIME BenchTimer::shared_BenchTimer()->get_elapsed_time()
#define RESET_BENCHMARK_TIMER BenchTimer::shared_BenchTimer()->restart();

#define RESET_BYTE_COUNT IOBenchmark::reset();
#define EXCHANGED_BYTES(n) IOBenchmark::exchanged_bytes(n);
#define INTERACTION IOBenchmark::interaction();
#else

#define PAUSE_BENCHMARK
#define RESUME_BENCHMARK
#define GET_BENCHMARK_TIME 0
#define RESET_BENCHMARK_TIMER
#define RESET_BYTE_COUNT
#define EXCHANGED_BYTES(n)
#define INTERACTION

#endif