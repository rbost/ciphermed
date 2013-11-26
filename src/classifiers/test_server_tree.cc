#include <classifiers/decision_tree_classifier.hh>


static Tree<long>* model_nursery(vector<pair <vector<long>,long> > &criteria )
{
    criteria = vector<pair <vector<long>,long> >(4);
    
    criteria[0] = make_pair<vector<long>,long>({0,0,0,0,0,0,0,2},3);
    criteria[1] = make_pair<vector<long>,long>({0,0,0,0,0,0,0,2},5);
    criteria[2] = make_pair<vector<long>,long>({0,0,0,0,0,0,0,2},7);
    criteria[3] = make_pair<vector<long>,long>({0,0,0,0,0,0,2,0},3);
    
    Node<long> *n = new Node<long>(0,new Leaf<long>(1), new Leaf<long>(2));
    n = new Node<long>(1,n, new Leaf<long>(3));
    n = new Node<long>(2,n, new Leaf<long>(4));
    n = new Node<long>(3,n, new Leaf<long>(0));
    
    return n;
}

static void test_tree_classifier_server()
{
    gmp_randstate_t randstate;
    gmp_randinit_default(randstate);
    gmp_randseed_ui(randstate,time(NULL));
    
    Tree<long> *t;
    vector<pair <vector<long>,long> > criteria;
//    t = binaryRepTree(N_LEVELS);
    t = model_nursery(criteria);

    cout << "Init server" << endl;
    Decision_tree_Classifier_Server server(randstate,1024,*t,4, criteria);
    
    cout << "Start server" << endl;
    server.run();
}

int main()
{    
    test_tree_classifier_server();
    
    return 0;
}