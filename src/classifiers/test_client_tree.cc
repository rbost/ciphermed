#include <classifiers/decision_tree_classifier.hh>

#include <util/util.hh>

static void test_tree_classifier_client(const string &hostname)
{
    try
    {
        
        boost::asio::io_service io_service;
        
        gmp_randstate_t randstate;
        gmp_randinit_default(randstate);
        gmp_randseed_ui(randstate,time(NULL));


        long query = rand() % ((1<<n_levels) - 1);
        
        Decision_tree_Classifier_Client client(io_service, randstate,1024,query);
        
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