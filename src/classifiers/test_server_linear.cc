#include <classifiers/linear_classifier.hh>
#include <util/benchmarks.hh>
#include <ctime>

static void test_linear_classifier_server(unsigned int model_size, unsigned int nbits_max)
{
#ifdef BENCHMARK
    cout << "BENCHMARK flag set" << endl;
    BENCHMARK_INIT
#endif
    
    gmp_randstate_t randstate;
    gmp_randinit_default(randstate);
    gmp_randseed_ui(randstate,time(NULL));
    
    srand(time(NULL));
    
    assert(nbits_max > model_size + 1);
    unsigned int nbits = nbits_max - model_size - 1;
    
    long two_nbits = 1 << nbits;
    
    vector<mpz_class> model(model_size+1);
    for (size_t i = 0; i <= model_size; i++) {
        model[i] = rand()%two_nbits;
        if (rand()%2) {
            model[i] *= -1;
        }
    }
    
    cout << "Server for linear classifier\n";
    cout << "Model as dimension " << model_size << "\n";
    cout << nbits_max << " bits of precision" << endl;
    cout << "Init server" << endl;
    Linear_Classifier_Server server(randstate,1024,100,model,nbits_max);
    
    cout << "Start server" << endl;
    server.run();
}

int main()
{    
    test_linear_classifier_server(30, 64);
    
    return 0;
}