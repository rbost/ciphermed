#include <mpc/lsic.hh>
#include <mpc/private_comparison.hh>
#include <mpc/garbled_comparison.hh>
#include <mpc/rev_enc_comparison.hh>
#include <mpc/enc_comparison.hh>
#include <mpc/linear_enc_argmax.hh>
#include <mpc/tree_enc_argmax.hh>

#include <net/client.hh>
#include <net/protocol_bench.hh>

#include <util/util.hh>

#include <util/benchmarks.hh>

static void bench_client(const string &hostname, unsigned int key_size, unsigned int bit_size, unsigned int iterations, unsigned int n_threads)
{
    try
    {
#ifdef BENCHMARK
        cout << "BENCHMARK flag set" << endl;
        BENCHMARK_INIT
#endif

        boost::asio::io_service io_service;
        
        gmp_randstate_t randstate;
        gmp_randinit_default(randstate);
        gmp_randseed_ui(randstate,time(NULL));
        
        Bench_Client client(io_service, randstate,key_size,100);
        client.set_n_threads(n_threads);
        
        client.connect(io_service, hostname);
        
        client.exchange_keys();

        cout << "\n\n\n";
        
//        client.bench_lsic(bit_size, iterations);
//        client.bench_compare(bit_size, iterations);
        client.bench_garbled_compare(bit_size, iterations);
        
//        client.bench_enc_compare(bit_size, iterations, LSIC_PROTOCOL);
//        client.bench_enc_compare(bit_size, iterations, DGK_PROTOCOL);
        client.bench_enc_compare(bit_size, iterations, GC_PROTOCOL);
        
//        client.bench_rev_enc_compare(bit_size, iterations, LSIC_PROTOCOL);
//        client.bench_rev_enc_compare(bit_size, iterations, DGK_PROTOCOL);
        client.bench_rev_enc_compare(bit_size, iterations, GC_PROTOCOL);

        client.disconnect();
        
    }
    catch (std::exception& e)
    {
        std::cout << "Exception: " << e.what() << std::endl;
    }
}

int main(int argc, char* argv[])
{
    if (argc != 6)
    {
        std::cerr << "Usage: bench_client <host> <key_size> <bit_size> <iterations> <n_threads>" << std::endl;
        return 1;
    }
    string hostname(argv[1]);
    unsigned int key_size = atoi(argv[2]);
    unsigned int bit_size = atoi(argv[3]);
    unsigned int iterations = atoi(argv[4]);
    unsigned int n_threads = atoi(argv[5]);

    bench_client(hostname, key_size, bit_size, iterations, n_threads);
    
    return 0;
}
