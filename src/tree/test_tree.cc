#include <iostream>
#include <cassert>
#include <cstdlib>

#include <tree/tree.hh>
#include <tree/m_variate_poly.hh>
#include <tree/util.hh>
#include <tree/util_poly.hh>

#include <FHE.h>
#include <EncryptedArray.h>
#include <NTL/lzz_pXFactoring.h>

#include <util/util.hh>

using namespace std;
//std::vector<long> bitDecomp(long x, size_t n);

static void test_tree()
{
    // basic decision trees
    
    Tree<int> *t = new Node<int>(0, new Leaf<int>(1), new Leaf<int>(2));

    assert(t->decision({true}) == 1);
    assert(t->decision({false}) == 2);

    cout << t->to_polynomial() << endl;

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
    Term<int> t2(3,{0});
    Term<int> t = t1*t2;

    cout << "Lex Compare: " << compareVars(t1,t2) << endl;
    vector<int> vals = {1,1};
    
    cout << t1 << " eval " << evalTerm(t1,vals) << endl;
//    cout << t2 << " eval " << t2.eval(vals) << endl;
//    cout << t << " eval " << t.eval(vals) << endl;
    
    Multivariate_poly<int> p = t1*(t1 + t2 + t);
//    cout << p << endl;
    
    
    Term<int> x(1,{0});
    Term<int> one(1);
    p = x + one;
    p *= p;
    cout << p << endl;
    cout << "Eval : " << evalPoly(p,vals) << endl;


}


struct configuration {
    long p;
    long r;
    long d;
    long c;
    long L;
    long w;
    long s;
    long k;
    long chosen_m;
};

static inline ostream &
operator<<(ostream &o, const configuration &cfg)
{
    o << "{p=" << cfg.p
    << ",r=" << cfg.r
    << ",d=" << cfg.d
    << ",c=" << cfg.c
    << ",L=" << cfg.L
    << ",w=" << cfg.w
    << ",s=" << cfg.s
    << ",k=" << cfg.k
    << ",chosen_m=" << cfg.chosen_m
    << "}";
    return o;
}

static void fun_with_fhe()
{
    long p = 2;
    long r = 1;
    long d = 1;
    long c = 2;
    long L = 5;
    long w = 64;
    long s = 1;
    long k = 80;
    long chosen_m = 0; // XXX: check?

    long m = FindM(k, L, c, p, d, s, chosen_m, true);

    
    FHEcontext context(m, p, r);
    buildModChain(context, L, c);
    FHESecKey secretKey(context);
    const FHEPubKey& publicKey = secretKey;
    secretKey.GenSecKey(w); // A Hamming-weight-w secret key
    
    ZZX G;
    
    if (d == 0)
        G = context.alMod.getFactorsOverZZ()[0];
    else
        G = makeIrredPoly(p, d);

    cerr << "G = " << G << "\n";
//    cerr << "generating key-switching matrices... ";
//    addSome1DMatrices(secretKey); // compute key-switching matrices that we need
//    addSome1DMatrices(secretKey,1); // compute key-switching matrices that we need
//    cerr << "done\n";

    
    EncryptedArray ea(context, G);
    
    PlaintextArray p0(ea);
    PlaintextArray p1(ea);
    PlaintextArray p2(ea);
    PlaintextArray p3(ea);
    
    vector<long> values = bitDecomp(3,ea.size());
    cerr << "\n";
    for (size_t i = 0; i < values.size(); i++) {
        cerr << values[i] << ", ";
    }
    cerr<<endl;

    p0.random();
    p1.random();
    p2.random();
//    p3.random();
    p3.encode(values);
    
    Ctxt c0(publicKey), c1(publicKey), c2(publicKey), c3(publicKey);
    ea.encrypt(c0, publicKey, p0);
    ea.encrypt(c1, publicKey, p1);
    ea.encrypt(c2, publicKey, p2);
    ea.encrypt(c3, publicKey, p3);

    p0.mul(p1); c0*=c1; //c0.multiplyBy(c1); //c0.reLinearize(1);
//    p0.mul(p3); c0.multiplyBy(c3);
    p2.mul(p3); c2.multiplyBy(c3);
//    p0.mul(p2); c0.multiplyBy(c2);
    
    cerr << "Levels, KeyId :\n";
    cerr << "c0 -> " << c0.findBaseLevel() << " , " << c0.getKeyID() << "\n";
    cerr << "c1 -> " << c1.findBaseLevel() << " , " << c1.getKeyID() << "\n";
    cerr << "c2 -> " << c2.findBaseLevel() << " , " << c2.getKeyID() << "\n";
    cerr << "c3 -> " << c3.findBaseLevel() << " , " << c3.getKeyID() << "\n";
    
    PlaintextArray pp0(ea);
    PlaintextArray pp1(ea);
    PlaintextArray pp2(ea);
    PlaintextArray pp3(ea);
    
    ea.decrypt(c0, secretKey, pp0);
    ea.decrypt(c1, secretKey, pp1);
    ea.decrypt(c2, secretKey, pp2);
    ea.decrypt(c3, secretKey, pp3);
    
    cerr << "\n";
    
    if (!pp0.equals(p0)) cerr << "oops 0\n";
    if (!pp1.equals(p1)) cerr << "oops 1\n";
    if (!pp2.equals(p2)) cerr << "oops 2\n";
    if (!pp3.equals(p3)) cerr << "oops 3\n";
    
    vector<long> v;
    pp3.decode(v);
//
    pp3.print(cerr);
    
//    cerr << "\n";
//    for (size_t i = 0; i < v.size(); i++) {
//        cerr << v[i] << ", ";
//    }
//    cerr<<endl;
    
    cerr << "Sizes:\n";
    cerr << "ea.size()=" << ea.size() << endl;
    cerr << "ea.dimension()=" << ea.dimension() << endl;
    cerr << "v.size()=" << v.size() << endl;

    
    
    Ctxt b(publicKey);
    p0.encode(1);
    ea.encrypt(b,publicKey,p0);
    Ctxt b_neg = ctxt_neg(b,ea);
    ea.decrypt(b_neg, secretKey, pp0);
    pp0.print(cerr);

    
}

static void test_selector(size_t n_levels = 3, bool useShallowCircuit = true)
{
    ScopedTimer *timer;
    
    long p = 2;
    long r = 1;
    long d = 1;
    long c = 2;
    
    long L;
    /* We have to understand how to choose the number of primes in the moduli chain */
    if(useShallowCircuit) L = 2;//= 2*log(n_levels)+1; // this seems to work with the shallow multiplication
    else L = n_levels; // this seems to work with the deep multiplication
    
    
    long w = 64;
    long s = 1;
    long k = 80;
    long chosen_m = 0; // XXX: check?
    
    timer = new ScopedTimer("Setup SHE scheme");
    
    long m = FindM(k, L, c, p, d, s, chosen_m, true);
    
    FHEcontext context(m, p, r);
    buildModChain(context, L, c);
    FHESecKey secretKey(context);
    const FHEPubKey& publicKey = secretKey;
    secretKey.GenSecKey(w); // A Hamming-weight-w secret key
    cerr << "Chosen level=" << L << endl;
    
    ZZX G;
    
    if (d == 0)
        G = context.alMod.getFactorsOverZZ()[0];
    else
        G = makeIrredPoly(p, d);
    
    //    addSome1DMatrices(secretKey); // compute key-switching matrices that we need
    
    EncryptedArray ea(context, G);
    delete timer;
    
    Tree<long> *t;
    
    timer = new ScopedTimer("Build tree & polynomial");
    
    t = binaryRepTree(n_levels);
    
    Multivariate_poly< vector<long> > selector = t->to_polynomial_with_slots(ea.size());
    
    delete timer;
    
    long query = rand() % ((1<<n_levels) - 1);
    vector<long> bits_query = bitDecomp(query, n_levels);
    
    timer = new ScopedTimer("Encode query");
    
    vector<PlaintextArray> b(n_levels,PlaintextArray(ea));
    vector<Ctxt> c_b(n_levels,Ctxt(publicKey));
    
    
    for (size_t i = 0; i < n_levels; i++) {
        b[i].encode(bits_query[i]);
        ea.encrypt(c_b[i],publicKey,b[i]);
    }
    
    delete timer;
    
    cerr << endl;
    
    cerr << "selector: " << selector.termsCount() << " terms, degree " << selector.degree() << ", sum of degrees " << selector.sumOfDegrees() <<endl;
    //
    //    timer = new ScopedTimer("Eval polynomial - not regrouped");
    //    evalPoly_FHE(selector, c_b,ea,useShallowCircuit);
    //    delete timer;
    //
    //    cerr << endl;
    
    timer = new ScopedTimer("Regroup terms");
    
    selector = mergeRegroup(selector);
    cerr << endl;
    
    delete timer;
    cerr << "selector regrouped: " << selector.termsCount() << " terms, degree " << selector.degree() << ", sum of degrees " << selector.sumOfDegrees() <<endl;
    
    timer = new ScopedTimer("Eval polynomial - regrouped");
    Ctxt c_r = evalPoly_FHE(selector, c_b,ea,useShallowCircuit);
    delete timer;
    
    cerr << endl;
    
    cerr << "Level of final cyphertext: " << c_r.findBaseLevel() << endl;
    vector<long> res_bits;
    
    timer = new ScopedTimer("Decrypt");
    ea.decrypt(c_r, secretKey, res_bits);
    delete timer;
    
    long res = bitDecomp_inv(res_bits);
    cerr << "query=" << query << endl;
    cerr << "result=" << res << endl;
    
    assert(query == res);
}


static void test_selector_tree(size_t n_levels = 3)
{
    ScopedTimer *timer;
    
    long p = 2;
    long r = 1;
    long d = 1;
    long c = 2;
    
    long L;
    /* We have to understand how to choose the number of primes in the moduli chain */
    L = n_levels-1;
    
    
    long w = 64;
    long s = 1;
    long k = 80;
    long chosen_m = 0; // XXX: check?
    
    timer = new ScopedTimer("Setup SHE scheme");
    
    long m = FindM(k, L, c, p, d, s, chosen_m, true);
    
    FHEcontext context(m, p, r);
    buildModChain(context, L, c);
    FHESecKey secretKey(context);
    const FHEPubKey& publicKey = secretKey;
    secretKey.GenSecKey(w); // A Hamming-weight-w secret key
    cerr << "Chosen level=" << L << endl;
    
    ZZX G;
    
    if (d == 0)
        G = context.alMod.getFactorsOverZZ()[0];
    else
        G = makeIrredPoly(p, d);
    
    //    addSome1DMatrices(secretKey); // compute key-switching matrices that we need
    
    EncryptedArray ea(context, G);
    delete timer;
    
    Tree<long> *t;
    
    timer = new ScopedTimer("Build tree");
    
    t = binaryRepTree(n_levels);
    
    delete timer;
    
    long query = rand() % ((1<<n_levels) - 1);
    vector<long> bits_query = bitDecomp(query, n_levels);
    
    timer = new ScopedTimer("Encode query");
    
    vector<PlaintextArray> b(n_levels,PlaintextArray(ea));
    vector<Ctxt> c_b(n_levels,Ctxt(publicKey));
    
    
    for (size_t i = 0; i < n_levels; i++) {
        b[i].encode(bits_query[i]);
        ea.encrypt(c_b[i],publicKey,b[i]);
    }
    
    delete timer;
    
    cerr << endl;
 
    timer = new ScopedTimer("Eval tree");
    Ctxt c_r = evalNode_FHE(*(Node<long> *)t,c_b,ea);
    delete timer;
    
    cerr << endl;
    
    cerr << "Level of final cyphertext: " << c_r.findBaseLevel() << endl;
    vector<long> res_bits;
    
    timer = new ScopedTimer("Decrypt");
    ea.decrypt(c_r, secretKey, res_bits);
    delete timer;
    
    long res = bitDecomp_inv(res_bits);
    cerr << "query=" << query << endl;
    cerr << "result=" << res << endl;
    
    assert(query == res);
}

static void usage(char *prog)
{
    cerr << "Usage: "<<prog<<" [ optional parameters ]...\n";
    cerr << "  optional parameters have the form 'attr1=val1 attr2=val2 ...'\n";
    cerr << "  e.g, 'n=4 s=1 k=80'\n\n";
    cerr << "  n is the depth of the test tree [default=5]\n";
    cerr << "  shallow is the flag to turn of/on the shallow circuit for multiplication [default=1]" << endl;
    exit(0);
}

int
main(int ac, char **av)
{
    argmap_t argmap;
    argmap["n"] = "5";
    argmap["shallow"] = "1";

    if (!parseArgs(ac, av, argmap)) usage(av[0]);

    long n = atoi(argmap["n"]);
    bool shallow = atoi(argmap["shallow"]);
   
    srand(time(NULL));
    
//    test_tree();
//    test_poly();
//    fun_with_fhe();
    
    cout << "Test selector with polynomial" << endl;
    test_selector(n,shallow);
    
    cout << "\n\n\n";
    cout << "Test selector without polynomial" << endl;
    test_selector_tree(n);
    
    return 0;
}

