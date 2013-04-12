#pragma once

#include <stdexcept>
#include <assert.h>
#include <iostream>
#include <string>

#include <util/errstream.hh>
#include <util/compiler.hh>
#include <sys/time.h>

class Timer {
 private:
    Timer(const Timer&) = delete;  /* no reason to copy timer objects */
    Timer(Timer &&) = delete;
    Timer &operator=(const Timer &) = delete;

 public:
    Timer() { lap(); }

    //microseconds
    uint64_t lap() {
        uint64_t t0 = start;
        uint64_t t1 = cur_usec();
        start = t1;
        return t1 - t0;
    }

    //milliseconds
    double lap_ms() {
        return ((double)lap()) / 1000.0;
    }

 private:
    static uint64_t cur_usec() {
        struct timeval tv;
        gettimeofday(&tv, 0);
        return ((uint64_t)tv.tv_sec) * 1000000 + tv.tv_usec;
    }

    uint64_t start;
};

class ScopedTimer {
public:
    ScopedTimer(const std::string &m) : m_(m) {}

    ScopedTimer(const ScopedTimer&) = delete;
    ScopedTimer(ScopedTimer &&) = delete;
    ScopedTimer &operator=(const ScopedTimer &) = delete;

    ~ScopedTimer()
    {
        const double e = t_.lap_ms();
        std::cerr << "region " << m_ << " took " << e << " ms" << std::endl;
    }

private:
    std::string m_;
    Timer t_;
};

inline void
assert_s(bool value, const std::string &msg) throw (FHEError)
{
    if (unlikely(!value)) {
        std::cerr << "ERROR: " << msg << std::endl;
        throw FHEError(msg);
    }
}
