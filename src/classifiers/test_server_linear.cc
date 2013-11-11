#include <classifiers/linear_classifier.hh>
#include <util/benchmarks.hh>

static void test_linear_classifier_server()
{
#ifdef BENCHMARK
    cout << "BENCHMARK flag set" << endl;
    BENCHMARK_INIT
#endif
    
    gmp_randstate_t randstate;
    gmp_randinit_default(randstate);
    gmp_randseed_ui(randstate,time(NULL));
    
    
    vector<mpz_class> model(4);
    model[0] = 1;
    model[1] = 1;
    model[2] = 1;
    model[3] = 0;
    
    cout << "Init server" << endl;
    Linear_Classifier_Server server(randstate,1024,100,model,64);
    
    cout << "Start server" << endl;
    server.run();
}

int main()
{    
    test_linear_classifier_server();
    
    return 0;
}