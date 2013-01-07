#include <math/matrix.hh>

#include <util/util.hh>

#include <iostream>
using namespace std;

static poly
make_poly(vector<mpz_class> v) {
    return poly(&v, true);
}

int
main(int ac, char **av)
{
    // test poly's

    poly a = make_poly({1, 8});
    poly b = make_poly({2, 3});

    mpz_class q = 9; 

    cerr << "a is " << a << "\n b is " << b << "\n q is " << q << "\n";
    
    poly c = (a+b) % q;

    cerr << "sum is " << c << "\n";
    assert_s(c == make_poly({3, 2}), "incorrect sum");

    cerr << "after sum a is " << a << "\n";

    poly d = a * b % q;
    cerr << "mult is (mod q) " << d << "\n";
    assert_s(d == make_poly({2, 1, 6}), "incorrect multiply");

    cerr << "after d, a is " << a << "\n";
	
    poly e = modpoly(d, 2);
    cerr << "mult is (mod q) mod n^2+1" << e << "\n";
    assert_s(e == make_poly({-4, 1}), "incorrect modpoly");

    cerr << "after e, a is " << a << "\n";
    
    poly f = modpoly(d, 1);
    cerr << "d=" << d << " mod x+1 is " << f << "\n";
    assert_s(f == make_poly({7}), "incorrect modpoly");
    
    cerr << "after f, a is " << a << "\n";
    
    //test vecs

    vec v = {a, b};
    vec w = {c, d};

    cerr << "poly a is " << a << " and b " << b << "\n";
    cerr << "vec v is " << v << "\n";
    cerr << "vec w is " << w << "\n";
    cerr << "poly c is " << c << "\n";

    cerr << "v + w is " << (v+w) << "\n";
    cerr << "v * c is " << (v*c) << "\n";
    cerr << "w * c is " << (w*c) << "\n";


    cerr << "v dot w is " << dot(v, w) << "\n";
    cerr << "v dot w mod x^4+1" << modpoly(dot(v, w), 4) << " and mod q" << modpoly(dot(v, w), 4) % q << "\n"; 

    cerr << "OK\n";
}
