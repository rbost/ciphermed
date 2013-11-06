#include <mpc/lsic.hh>
#include <mpc/private_comparison.hh>
#include <mpc/rev_enc_comparison.hh>
#include <mpc/enc_comparison.hh>
#include <mpc/linear_enc_argmax.hh>

#include <net/server.hh>
#include <net/protocol_tester.hh>

static void test_basic_server()
{
    gmp_randstate_t randstate;
    gmp_randinit_default(randstate);
    gmp_randseed_ui(randstate,time(NULL));
    
    cout << "Init server" << endl;
    Tester_Server server(randstate,1024,100);
    
    cout << "Start server" << endl;
    server.run();
}

int main()
{
    test_basic_server();
        
    return 0;
}