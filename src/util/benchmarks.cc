#include <util/benchmarks.hh>

BenchTimer* BenchTimer::shared_instance__ = NULL;
unsigned long IOBenchmark::byte_count__ = 0;