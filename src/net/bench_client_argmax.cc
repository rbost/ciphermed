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

static void bench_linear_lsic(Bench_Client &client, unsigned int bit_size, unsigned int iterations, unsigned int n_elts_min, unsigned int n_elts_max, unsigned int step)
{
    unsigned int n_elts;
    cout << "===================================\n";
    cout << "LINEAR USING LSIC\n\n";
    
    for (n_elts = n_elts_min; n_elts <= n_elts_max; n_elts += step) {
        cout << "\n\n\n";
        
        client.bench_linear_enc_argmax(n_elts, bit_size, iterations, true);
    }
}

static void bench_linear_dgk(Bench_Client &client, unsigned int bit_size, unsigned int iterations, unsigned int n_elts_min, unsigned int n_elts_max, unsigned int step)
{
    unsigned int n_elts;
    cout << "===================================\n";
    cout << "LINEAR USING DGK\n\n";
    
    for (n_elts = n_elts_min; n_elts <= n_elts_max; n_elts += step) {
        cout << "\n\n\n";
        
        client.bench_linear_enc_argmax(n_elts, bit_size, iterations, false);
    }
}

static void bench_tree_lsic(Bench_Client &client, unsigned int bit_size, unsigned int iterations, unsigned int n_elts_min, unsigned int n_elts_max, unsigned int step)
{
    unsigned int n_elts;
    cout << "===================================\n";
    cout << "TREE USING LSIC\n\n";
    
    for (n_elts = n_elts_min; n_elts <= n_elts_max; n_elts += step) {
        cout << "\n\n\n";
        
        client.bench_tree_enc_argmax(n_elts, bit_size, iterations, true);
    }
}

static void bench_tree_dgk(Bench_Client &client, unsigned int bit_size, unsigned int iterations, unsigned int n_elts_min, unsigned int n_elts_max, unsigned int step)
{
    unsigned int n_elts;
    cout << "===================================\n";
    cout << "TREE USING DGK\n\n";
    
    for (n_elts = n_elts_min; n_elts <= n_elts_max; n_elts += step) {
        cout << "\n\n\n";
        
        client.bench_tree_enc_argmax(n_elts, bit_size, iterations, false);
    }
}

static void bench_client_argmax(const string &hostname, unsigned int key_size, unsigned int bit_size, unsigned int iterations, unsigned int n_threads, unsigned int n_elts_min, unsigned int n_elts_max, unsigned int step)
{
    assert(n_elts_min <= n_elts_max);
    assert(step > 0);
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
        
        bench_linear_lsic(client, bit_size, iterations, n_elts_min, n_elts_max, step);
        bench_linear_dgk(client, bit_size, iterations, n_elts_min, n_elts_max, step);
        bench_tree_lsic(client, bit_size, iterations, n_elts_min, n_elts_max, step);
        bench_tree_dgk(client, bit_size, iterations, n_elts_min, n_elts_max, step);
        
        client.disconnect();
        
    }
    catch (std::exception& e)
    {
        std::cout << "Exception: " << e.what() << std::endl;
    }
}

int main(int argc, char* argv[])
{
    if (argc < 8)
    {
        std::cerr << "Usage: bench_client <host> <key_size> <bit_size> <iterations> <n_threads> <n_elts_min> <n_elts_max> <step - optional>" << std::endl;
        return 1;
    }
    string hostname(argv[1]);
    unsigned int key_size = atoi(argv[2]);
    unsigned int bit_size = atoi(argv[3]);
    unsigned int iterations = atoi(argv[4]);
    unsigned int n_threads = atoi(argv[5]);
    unsigned int n_elts_min = atoi(argv[6]);
    unsigned int n_elts_max = atoi(argv[7]);
    unsigned int step = 1;
    
    if (argc >= 9) {
        step = atoi(argv[8]);
    }
    
    bench_client_argmax(hostname, key_size, bit_size, iterations, n_threads, n_elts_min, n_elts_max, step);
    
    return 0;
}