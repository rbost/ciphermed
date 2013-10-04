#include <iostream>
#include <cassert>

#include <tree/tree.hh>
#include <tree/m_variate_poly.hh>


using namespace std;

static void test_tree()
{
    // basic decision trees
    
    Tree<int> *t = new Node<int>(0, new Leaf<int>(1), new Leaf<int>(2));

    assert(t->decision({true}) == 1);
    assert(t->decision({false}) == 2);

    delete t;
    
    t = new Node<int>(0, new Node<int>(1, new Leaf<int>(1), new Leaf<int>(2)), new Leaf<int>(3));
    
    assert(t->decision({true,true}) == 1);
    assert(t->decision({true,false}) == 2);
    assert(t->decision({false,false}) == 3);
    assert(t->decision({false,false}) == 3);
    
    delete t;

}

static void test_poly()
{
    Term<int> t1(2,{0});
    Term<int> t2(3,{1});
    Term<int> t = t1*t2;

    vector<int> vals = {1,1};
    cout << t1 << " eval " << t1.eval(vals) << endl;
    cout << t2 << " eval " << t2.eval(vals) << endl;
    cout << t << " eval " << t.eval(vals) << endl;
    
    Multivariate_poly<int> p = t1*(t1 + t2 + t);
    cout << p << endl;
    
    cout << "Eval : " << p.eval(vals) << endl;
}

int
main(int ac, char **av)
{
    test_tree();
    test_poly();
    
    return 0;
}

