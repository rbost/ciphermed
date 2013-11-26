#include <classifiers/decision_tree_classifier.hh>

#include <util/util.hh>
#include <util/benchmarks.hh>

static vector<long> gen_nursery_query()
{
    vector<long> query(8);
    
    query[0] = 1 + (rand() %3);
    query[1] = 1 + (rand() %5);
    query[2] = 1 + (rand() %4);
    query[3] = 1 + (rand() %4);
    query[4] = 1 + (rand() %3);
    query[5] = 1 + (rand() %2);
    query[6] = 1 + (rand() %3);
    query[7] = 1 + (rand() %3);
    
    return query;
}

static void test_tree_classifier_client(const string &hostname)
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


        vector<long> query = gen_nursery_query();
//        vector<long> query_bits = bitDecomp(query, N_LEVELS);
        Decision_tree_Classifier_Client client(io_service, randstate,1024,query,4);
        
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
    if (argc != 2)
    {
        std::cerr << "Usage: client <host>" << std::endl;
        return 1;
    }
    string hostname(argv[1]);
    srand(time(NULL));

    test_tree_classifier_client(hostname);
    
    return 0;
}