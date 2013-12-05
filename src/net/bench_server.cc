#include <mpc/lsic.hh>
#include <mpc/private_comparison.hh>
#include <mpc/rev_enc_comparison.hh>
#include <mpc/enc_comparison.hh>
#include <mpc/linear_enc_argmax.hh>

#include <net/server.hh>
#include <net/protocol_bench.hh>

#include <util/benchmarks.hh>

static void bench_server(unsigned int key_size, unsigned int n_threads)
{
#ifdef BENCHMARK
    cout << "BENCHMARK flag set" << endl;
    BENCHMARK_INIT
#endif
    
    gmp_randstate_t randstate;
    gmp_randinit_default(randstate);
    gmp_randseed_ui(randstate,time(NULL));
    
    cout << "Init server" << endl;
    Bench_Server server(randstate,key_size,100);
    server.set_threads_per_session(n_threads);
    
    cout << "Start server" << endl;
    server.run();
}

int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        std::cerr << "Usage: bench_server <key_size> <n_threads>" << std::endl;
        return 1;
    }
    
    unsigned int key_size = atoi(argv[1]);
    unsigned int n_threads = atoi(argv[2]);

    bench_server(key_size, n_threads);
        
    return 0;
}