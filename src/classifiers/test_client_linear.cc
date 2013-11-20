#include <classifiers/linear_classifier.hh>

#include <util/util.hh>
#include <util/benchmarks.hh>
#include <ctime>

static void test_linear_classifier_client(const string &hostname, unsigned int model_size, unsigned int nbits_max)
{
    cout << "Client for linear classifier\n";
    cout << "Model as dimension " << model_size << "\n";
    cout << nbits_max << " bits of precision" << endl;

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

        srand(time(NULL));

        assert(nbits_max > model_size + 1);
        unsigned int nbits = nbits_max - model_size - 1;
        long two_nbits = 1 << nbits;

        vector<mpz_class> values(model_size);
        for (size_t i = 0; i < model_size; i++) {
            values[i] = rand()%two_nbits;
            if (rand()%2) {
                values[i] *= -1;
            }
        }

        Linear_Classifier_Client client(io_service, randstate,1024,100,values,nbits_max);
        
        client.connect(io_service, hostname);
        
        bool result = client.run();
        
//        client.disconnect();
        
        cout << "Result : " << result << endl;
    }
    catch (std::exception& e)
    {
        std::cout << "Exception: " << e.what() << std::endl;
    }
    
}


static void bench_linear_classifier_client(const string &hostname, unsigned int model_size, unsigned int nbits_max, unsigned int nRounds = 10)
{
    cout << "Client for linear classifier\n";
    cout << "Model as dimension " << model_size << "\n";
    cout << nbits_max << " bits of precision" << endl;
    
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
        
        srand(time(NULL));
        
//        assert(nbits_max > model_size + 1);
        unsigned int nbits = nbits_max - model_size - 1;
        long two_nbits = 1 << nbits;
        
        vector<mpz_class> values(model_size);
        for (size_t i = 0; i < model_size; i++) {
            values[i] = rand()%two_nbits;
            if (rand()%2) {
                values[i] *= -1;
            }
        }
        
        Bench_Linear_Classifier_Client client(io_service, randstate,1024,100,values,nbits_max, nRounds);
        
        client.connect(io_service, hostname);
        
        client.run();
        
        //        client.disconnect();
    }
    catch (std::exception& e)
    {
        std::cout << "Exception: " << e.what() << std::endl;
    }
    
}

int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        std::cerr << "Usage: client <host> <model_size>" << std::endl;
        return 1;
    }
    string hostname(argv[1]);
    unsigned int model_size(atoi(argv[2]));

//    test_linear_classifier_client(hostname,model_size);
    bench_linear_classifier_client(hostname,model_size,64,10);
    
    return 0;
}