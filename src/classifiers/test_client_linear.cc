#include <classifiers/linear_classifier.hh>

#include <util/util.hh>

static void test_linear_classifier_client(const string &hostname)
{
    try
    {
        
        boost::asio::io_service io_service;
        
        gmp_randstate_t randstate;
        gmp_randinit_default(randstate);
        gmp_randseed_ui(randstate,time(NULL));

        vector<mpz_class> values(4);
        values[0] = -1;
        values[1] = -1;
        values[2] = -1;

        Linear_Classifier_Client client(io_service, randstate,1024,100,values,64);
        
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

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: client <host>" << std::endl;
        return 1;
    }
    string hostname(argv[1]);

    test_linear_classifier_client(hostname);
    
    return 0;
}