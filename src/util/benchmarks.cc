#include <util/benchmarks.hh>

#ifdef BENCHMARK
BenchTimer* BenchTimer::shared_instance__ = NULL;
unsigned long IOBenchmark::byte_count__ = 0;
unsigned long IOBenchmark::interaction_count__ = 0;
#endif