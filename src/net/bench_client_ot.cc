#include <mpc/lsic.hh>
#include <mpc/private_comparison.hh>
#include <mpc/rev_enc_comparison.hh>
#include <mpc/enc_comparison.hh>
#include <mpc/linear_enc_argmax.hh>
#include <mpc/tree_enc_argmax.hh>

#include <net/client.hh>
#include <net/protocol_bench.hh>

#include <util/util.hh>

#include <util/benchmarks.hh>

static void bench_client_ot(const string &hostname, unsigned int nOTs, unsigned int iterations)
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
        
        Bench_Client client(io_service, randstate,1024,100);
//        client.set_n_threads(1);
        
        client.connect(io_service, hostname);
        
        client.exchange_keys();

        cout << "\n\n\n";

        client.bench_ot(nOTs, iterations);
        
        client.disconnect();
        
    }
    catch (std::exception& e)
    {
        std::cout << "Exception: " << e.what() << std::endl;
    }
}

int main(int argc, char* argv[])
{
    if (argc != 4)
    {
        std::cerr << "Usage: bench_client_change <host> <nOTs> <iterations>" << std::endl;
        return 1;
    }
    string hostname(argv[1]);
    unsigned int nOTs = atoi(argv[2]);
    unsigned int iterations = atoi(argv[3]);

    bench_client_ot(hostname, nOTs, iterations);
    
    return 0;
}