#include <mpc/lsic.hh>
#include <mpc/private_comparison.hh>
#include <mpc/rev_enc_comparison.hh>
#include <mpc/enc_comparison.hh>
#include <mpc/linear_enc_argmax.hh>

#include <net/server.hh>
#include <net/linear_classifier_server.hh>

static void test_basic_server()
{
    gmp_randstate_t randstate;
    gmp_randinit_default(randstate);
    gmp_randseed_ui(randstate,time(NULL));
    
    
    cout << "Init server" << endl;
    Server server(randstate,1024,0,1024,100);
    
    cout << "Start server" << endl;
    server.run();
}


static void test_linear_classifier_server()
{
    gmp_randstate_t randstate;
    gmp_randinit_default(randstate);
    gmp_randseed_ui(randstate,time(NULL));
    
    
    vector<mpz_class> model(4);
    model[0] = 1;
    model[1] = 1;
    model[2] = 1;
    model[3] = 0;
    
    cout << "Init server" << endl;
    Linear_Classifier_Server server(randstate,1024,0,1024,100,model,64);
    
    cout << "Start server" << endl;
    server.run();
}

int main()
{
//    test_basic_server();
    
    test_linear_classifier_server();
    
    return 0;
}