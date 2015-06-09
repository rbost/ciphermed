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

#include <classifiers/decision_tree_classifier.hh>
#include <util/benchmarks.hh>


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


static Tree<long>* model_ecg(vector<pair <vector<long>,long> > &criteria )
{
    criteria = vector<pair <vector<long>,long> >(6);
    
    criteria[0] = make_pair<vector<long>,long>({1,0,0,0,0},0);
    criteria[1] = make_pair<vector<long>,long>({0,1,0,0,0},0);
    criteria[2] = make_pair<vector<long>,long>({0,0,1,0,0},0);
    criteria[3] = make_pair<vector<long>,long>({0,0,0,1,0},0);
    criteria[4] = make_pair<vector<long>,long>({0,0,0,0,1},0);
    criteria[5] = make_pair<vector<long>,long>({0,0,0,0,0},0);
    
#define     VF  0
#define     VT  1
#define     SVT  2
#define     PVC  3
#define     NSR  4
#define     APC  5
    
    
    Node<long> *n_left = new Node<long>(4,new Leaf<long>(PVC), new Leaf<long>(NSR));
    Node<long> *n_right = new Node<long>(5,new Leaf<long>(NSR), new Leaf<long>(APC));
   
    n_right = new Node<long>(3, n_left, n_right);
    n_right = new Node<long>(2, new Leaf<long>(SVT), n_right);
    
    n_left = new Node<long>(1,new Leaf<long>(VF), new Leaf<long>(VT));
    
    return new Node<long>(0, n_left, n_right);
}

static void test_tree_classifier_server()
{
    gmp_randstate_t randstate;
    gmp_randinit_default(randstate);
    gmp_randseed_ui(randstate,time(NULL));
    
    Tree<long> *t;
    vector<pair <vector<long>,long> > criteria;
    unsigned int n_nodes;
//    t = binaryRepTree(N_LEVELS);
    
   t = model_nursery(criteria);
   n_nodes = 4;
    
    
    // t = model_ecg(criteria);
    // n_nodes = 6;
    
    
#ifdef BENCHMARK
    cout << "BENCHMARK flag set" << endl;
    BENCHMARK_INIT
#endif

    cout << "Init server" << endl;
    Decision_tree_Classifier_Server server(randstate,1248,*t,n_nodes, criteria);
    
    cout << "Start server" << endl;
    server.run();
}

int main()
{    
    test_tree_classifier_server();
    
    return 0;
}