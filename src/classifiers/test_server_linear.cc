/*
 * Copyright 2013-2015 Raphael Bost
 *
 * This file is part of ciphermed.

 *  ciphermed is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 * 
 *  ciphermed is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 * 
 *  You should have received a copy of the GNU General Public License
 *  along with ciphermed.  If not, see <http://www.gnu.org/licenses/>. 2
 *
 */

#include <classifiers/linear_classifier.hh>
#include <util/benchmarks.hh>
#include <ctime>
#include <fstream>

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


static void bench_linear_classifier_server(const vector<mpz_class> &model, unsigned int nbits_max, unsigned int nRounds = 10)
{
#ifdef BENCHMARK
    cout << "BENCHMARK flag set" << endl;
    BENCHMARK_INIT
#endif
    unsigned int model_size = model.size();

    cout << "Server for linear classifier\n";
    cout << "Model as dimension " << model_size << "\n";
    cout << nbits_max << " bits of precision" << endl;

    gmp_randstate_t randstate;
    gmp_randinit_default(randstate);
    gmp_randseed_ui(randstate,time(NULL));
    
//    srand(time(NULL));
//    
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
    
//    assert(nbits_max > model_size + 1);

    
    cout << "Init server" << endl;
    Bench_Linear_Classifier_Server server(randstate,1024,100,model,nbits_max, nRounds);
    server.set_threads_per_session(2);
    
    cout << "Start server" << endl;
    server.run();
}

static void bench_linear_classifier_server(unsigned int model_size, unsigned int nbits_max, unsigned int nRounds = 10)
{
    srand(time(NULL));
    
    assert(nbits_max > model_size + 1);
    unsigned int nbits = nbits_max - model_size - 1;
    
    long two_nbits = (1 << nbits);
    
    vector<mpz_class> model(model_size+1);
    for (size_t i = 0; i <= model_size; i++) {
        model[i] = rand()%two_nbits;
        if (rand()%2) {
            model[i] *= -1;
        }
    }
    
    bench_linear_classifier_server(model, nbits_max, nRounds);
}

static vector<mpz_class> read_model(string file)
{
    std::ifstream infile(file);
    double v;
    vector<mpz_class> values;
    while (infile >> v){
        long v_int = v * 1e13;
        values.push_back(v_int);
    }
    
    return values;
}

int main(int argc, char* argv[])
{
    if (argc < 2){
        // randomly generate model
        cout << "Randomly generate model\n";

        //    test_linear_classifier_server(30, 64);
        bench_linear_classifier_server(30, 64,10);
    }else{
        cout << "Read model file " << argv[1] << endl;

        vector<mpz_class> model = read_model(argv[1]);
        bench_linear_classifier_server(model,64,10);
    }

    
    
    return 0;
}