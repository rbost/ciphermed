#include <classifiers/nb_classifier.hh>
#include <util/benchmarks.hh>
#include <ctime>
#include <fstream>

static void test_nb_classifier_server()
{
#ifdef BENCHMARK
    cout << "BENCHMARK flag set" << endl;
    BENCHMARK_INIT
#endif
    
    gmp_randstate_t randstate;
    gmp_randinit_default(randstate);
    gmp_randseed_ui(randstate,time(NULL));
    
    srand(time(NULL));
    
//    assert(nbits_max > model_size + 1);
//    unsigned int nbits = nbits_max - model_size - 1;
//    
//    long two_nbits = 1 << nbits;
//    
//    vector<mpz_class> model(model_size+1);
//    for (size_t i = 0; i <= model_size; i++) {
//        model[i] = rand()%two_nbits;
//        if (rand()%2) {
//            model[i] *= -1;
//        }
//    }
    
    cout << "Server for linear classifier\n";
    cout << "Init server" << endl;
    vector<vector<vector<double>>> conditionals_vec;
    vector<double> prior_vec;
    
    Naive_Bayes_Classifier_Server server(randstate,1024,100,conditionals_vec,prior_vec);
    
    cout << "Start server" << endl;
    server.run();
}


int main(int argc, char* argv[])
{
    test_nb_classifier_server();
    
    return 0;
}