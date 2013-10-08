#include <iostream>
#include <cassert>
#include <cstdlib>

#include <tree/tree.hh>
#include <tree/m_variate_poly.hh>
#include <tree/util.hh>

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
    Term<int> t2(3,{1});
    Term<int> t = t1*t2;

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

static ZZX makeIrredPoly(long p, long d)
{
    assert(d >= 1);
    assert(ProbPrime(p));
    
    if (d == 1) return ZZX(1, 1); // the monomial X
    
    zz_pBak bak; bak.save();
    zz_p::init(p);
    return to_ZZX(BuildIrred_zz_pX(d));
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
//    long p = 2;
//    long r = 16;
//    long d = 1; // XXX: check?
//    long c = 2;
//    long L = 10;
//    long w = 64;
//    long s = 4; // XXX: check?
//    long k = 80;
//    long chosen_m = 0; // XXX: check?

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
    cerr << "c0 -> " << c0.getLevel() << " , " << c0.getKeyID() << "\n";
    cerr << "c1 -> " << c1.getLevel() << " , " << c1.getKeyID() << "\n";
    cerr << "c2 -> " << c2.getLevel() << " , " << c2.getKeyID() << "\n";
    cerr << "c3 -> " << c3.getLevel() << " , " << c3.getKeyID() << "\n";
    
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

}

static void test_selector(size_t n_levels = 3, bool useShallowCircuit = true)
{
    ScopedTimer *timer;
    
    long p = 2;
    long r = 1;
    long d = 1;
    long c = 2;
//    long L = 6;
    long L = 2*n_levels;
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
    
    ZZX G;
    
    if (d == 0)
        G = context.alMod.getFactorsOverZZ()[0];
    else
        G = makeIrredPoly(p, d);

    addSome1DMatrices(secretKey); // compute key-switching matrices that we need

    EncryptedArray ea(context, G);
    delete timer;

    Tree<long> *t;
//    t = new Node<int>(0, new Node<int>(1, new Leaf<int>(1), new Leaf<int>(2)), new Leaf<int>(3));
//    t = balancedBinaryTree(4);
    
    timer = new ScopedTimer("Build tree & polynomial");

    t = binaryRepTree(n_levels);
    
    Multivariate_poly< vector<long> > selector = t->to_polynomial_with_slots(ea.size());

    delete t;

//    cerr << t->to_polynomial() << endl;
    

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

    timer = new ScopedTimer("Eval polynomial");
    Ctxt c_r = evalPoly_FHE(selector, c_b,ea,useShallowCircuit);
    delete timer;

    vector<long> res_bits;
    
    timer = new ScopedTimer("Decrypt result");
    ea.decrypt(c_r, secretKey, res_bits);
    delete timer;

    long res = bitDecomp_inv(res_bits);
    cerr << "query=" << query << endl;
    cerr << "result=" << res << endl;

    assert(query == res);
}

int
main(int ac, char **av)
{
    srand(time(NULL));
//    test_tree();
//    test_poly();
//    fun_with_fhe();
    test_selector(5,false);
    
    return 0;
}

