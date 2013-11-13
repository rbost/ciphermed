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
    
public:
    static void reset()
    {
        byte_count__ = 0;
    }
    
    static void exchanged_bytes(unsigned long n)
    {
        byte_count__ += n;
    }
    
    static unsigned long byte_count()
    {
        return byte_count__;
    }
};

#define BENCHMARK_INIT BenchTimer::create();
#define PAUSE_BENCHMARK BenchTimer::shared_BenchTimer()->pause();
#define RESUME_BENCHMARK BenchTimer::shared_BenchTimer()->resume();
#define GET_BENCHMARK_TIME BenchTimer::shared_BenchTimer()->get_elapsed_time()
#define RESET_BENCHMARK_TIMER BenchTimer::shared_BenchTimer()->restart();

#define RESET_BYTE_COUNT IOBenchmark::reset();
#define EXCHANGED_BYTES(n) IOBenchmark::exchanged_bytes(n);

#else

#define PAUSE_BENCHMARK
#define RESUME_BENCHMARK
#define GET_BENCHMARK_TIME 0
#define RESET_BENCHMARK_TIMER
#define EXCHANGED_BYTES(n)

#endif