#include <classifiers/decision_tree_classifier.hh>

static void test_tree_classifier_server()
{
    gmp_randstate_t randstate;
    gmp_randinit_default(randstate);
    gmp_randseed_ui(randstate,time(NULL));
    
    Tree<long> *t;
    t = binaryRepTree(n_levels);

    cout << "Init server" << endl;
    Decision_tree_Classifier_Server server(randstate,1024,*t);
    
    cout << "Start server" << endl;
    server.run();
}

int main()
{    
    test_tree_classifier_server();
    
    return 0;
}