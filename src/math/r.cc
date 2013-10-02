#include <iostream>
#include <sys/time.h>

#include <math/matrix.hh>
#include <math/mpz_class.hh>
#include <math/math_util.hh>
#include <util/util.hh>

using namespace std;

static poly
make_poly(const vector<mpz_class> &v)
{
    return poly(v);
}

// returns time in millis to do multiplication
// also checks correctness
template <typename MultAlgorithm>
static double
TestMultiplication(const poly &a, const poly &b, const poly &expected,
                   MultAlgorithm algo = MultAlgorithm())
{
    Timer t;
    const poly ans = algo(a, b);
    const double d = t.lap_ms();
    assert_s(ans == expected, "algo failed");
    return d;
}

// various algo functors:
// XXX: should replace w/ C++11 lambdas
namespace algo {
    struct naive {
        inline poly
        operator()(const poly &a, const poly &b) const
        {
            return a * b;
        }
    };
    struct karatsuba {
        inline poly
        operator()(const poly &a, const poly &b) const
        {
            return ::karatsuba(a, b);
        }
    };
    struct karatsuba2 {
        inline poly
        operator()(const poly &a, const poly &b) const
        {
            return ::karatsuba2(a, b);
        }
    };
}

int
main(int ac, char **av)
{
    // test swap
    {
        mpz_class x0(12345);
        mpz_class x1(54321);
        swap(x0, x1);
        assert_s(x0 == 54321, "swap failed");
        assert_s(x1 == 12345, "swap failed");
    }

    // test the nearest_div(p, q), for odd q
    {
        assert_s(mpz_class_nearest_div(3, 3) == 1, "nearest(3/3)");
        assert_s(mpz_class_nearest_div(4, 3) == 1, "nearest(4/3)");
        assert_s(mpz_class_nearest_div(5, 3) == 2, "nearest(5/3)");

        assert_s(mpz_class_nearest_div(2, 3) == 1, "nearest(2/3)");
        assert_s(mpz_class_nearest_div(1, 3) == 0, "nearest(1/3)");
        assert_s(mpz_class_nearest_div(0, 3) == 0, "nearest(0/3)");

        assert_s(mpz_class_nearest_div(-1, 3) == 0, "nearest(-1/3)");
        assert_s(mpz_class_nearest_div(-2, 3) == -1, "nearest(-2/3)");
        assert_s(mpz_class_nearest_div(-3, 3) == -1, "nearest(-3/3)");
        assert_s(mpz_class_nearest_div(-4, 3) == -1, "nearest(-4/3)");
        assert_s(mpz_class_nearest_div(-5, 3) == -2, "nearest(-5/3)");
    }

    // test the nearest_div(p, q) for even q
    {
        assert_s(mpz_class_nearest_div(3, 6) == 1, "nearest(3/6)");
        assert_s(mpz_class_nearest_div(4, 6) == 1, "nearest(4/6)");
        assert_s(mpz_class_nearest_div(5, 6) == 1, "nearest(5/6)");
        assert_s(mpz_class_nearest_div(6, 6) == 1, "nearest(6/6)");
        assert_s(mpz_class_nearest_div(7, 6) == 1, "nearest(7/6)");
        assert_s(mpz_class_nearest_div(8, 6) == 1, "nearest(8/6)");
        assert_s(mpz_class_nearest_div(9, 6) == 2, "nearest(9/6)");

        // XXX: not sure if this behavior is correct
        assert_s(mpz_class_nearest_div(-3, 6) == 0, "nearest(-3/6)");
        assert_s(mpz_class_nearest_div(-4, 6) == -1, "nearest(-4/6)");
        assert_s(mpz_class_nearest_div(-5, 6) == -1, "nearest(-5/6)");
        assert_s(mpz_class_nearest_div(-6, 6) == -1, "nearest(-6/6)");
        assert_s(mpz_class_nearest_div(-7, 6) == -1, "nearest(-7/6)");
        assert_s(mpz_class_nearest_div(-8, 6) == -1, "nearest(-8/6)");
        assert_s(mpz_class_nearest_div(-9, 6) == -1, "nearest(-9/6)");
        assert_s(mpz_class_nearest_div(-10, 6) == -2, "nearest(-10/6)");
    }

    // test the CRT
    {
        
        vector<mpz_class> v_c = {2,3};
        vector<mpz_class> m_c = {5,7};

        mpz_class x = mpz_class_crt(v_c,m_c);
                
        assert(x % 5 == 2);
        assert(x % 7 == 3);
    }
    
    // test modular inversion
    {
        mpz_class a(93), b(100);
        mpz_class c = mpz_class_invert(a,b);
        assert(a*c %b == 1);
    }
    
    // test poly's

    poly a = make_poly({1, 8});
    poly b = make_poly({2, 3});

    mpz_class q = 9;

    cerr << "a is " << a << "\n b is " << b << "\n q is " << q << endl;

    poly c = (a+b) % q;

    cerr << "sum is " << c << endl;
    assert_s(c == make_poly({3, 2}), "incorrect sum");

    cerr << "after sum a is " << a << endl;

    poly d = a * b % q;
    cerr << "mult is (mod q) " << d << endl;
    cerr << "deg(d): " << d.deg() << endl;
    assert_s(d.deg() == 2, "incorrect deg");
    assert_s(d == make_poly({2, 1, 6}), "incorrect multiply");

    cerr << "after d, a is " << a << endl;

    poly e = modpoly(d, 2);
    cerr << "mult is (mod q) mod n^2+1" << e << endl;
    assert_s(e == make_poly({-4, 1}), "incorrect modpoly");

    cerr << "after e, a is " << a << endl;

    poly f = modpoly(d, 1);
    cerr << "d=" << d << " mod x+1 is " << f << endl;
    assert_s(f == make_poly({7}), "incorrect modpoly");

    cerr << "after f, a is " << a << endl;

    poly g = make_poly({3, 15, 20}); // 20x^2 + 15x + 3
    assert_s(mpz_class(3) == g(mpz_class(0)), "p(0) incorrect");
    assert_s(mpz_class(38) == g(mpz_class(1)), "p(1) incorrect");
    assert_s(mpz_class(383) == g(mpz_class(4)), "p(4) incorrect");

    // test modulo q w/ remainder shift into -q/2, q/2 (exact bounds depend
    // on q even or odd)

    poly xx(vector<mpz_class>({-6}));
    assert_s(xx.modshift(7) == poly({1}), "modshift(7) failed");

    // q even case, require (-q/2, q/2]. we test q = 4, so x \in [-1, 0, 1, 2]
    poly h0({0, 1, 2, 3,
             4, 5, 6, 7,
             -1, -2, -3, -4});
    poly hshift0 = h0.modshift(4);
    cerr << "hshift0: " << hshift0 << endl;
    assert_s(hshift0 == poly({0, 1, 2, -1, 0, 1, 2, -1, -1, 2, 1, 0}), "modshift(4) failed");

    // q odd case, require [-(q-1)/2, (q-1)/2]. we test q = 3, so x \in [-1, 0, 1]
    poly h1({0, 1, 2,
             3, 4, 5,
             -1, -2, -3});
    poly hshift1 = h1.modshift(3);
    assert_s(hshift1 == poly({0, 1, -1, 0, 1, -1, -1, 1, 0}), "modshift(3) failed");

    //test vecs

    vec v = {a, b};
    vec w = {c, d};

    cerr << "poly a is " << a << " and b " << b << endl;
    cerr << "vec v is " << v << endl;
    cerr << "vec w is " << w << endl;
    cerr << "poly c is " << c << endl;

    cerr << "v + w is " << (v+w) << endl;
    cerr << "v * c is " << (v*c) << endl;
    cerr << "w * c is " << (w*c) << endl;


    cerr << "v dot w is " << dot(v, w) << endl;
    cerr << "v dot w mod x^4+1" << modpoly(dot(v, w), 4) << " and mod q" << modpoly(dot(v, w), 4) % q << endl;

    cerr << "OK\n";

    {
        // karatsuba simple test
        const poly a({1, 2, 3});
        const poly b({4, 5, 6});
        const poly c = karatsuba2(a, b);
        assert_s(c == poly({4, 13, 28, 27, 18}), "ka failed");

        const poly a1({1, 2, 3, 4});
        const poly b1({3, 4, 5});
        const poly c1 = karatsuba2(a1, b1);
        assert_s(c1 == poly({3, 10, 22, 34, 31, 20}), "ka failed");

        const poly a2({1, 2, 3, 4, 0, 1, 2, 3, 5, 68, 23});
        const poly b2({3, 4, 5, 23, 342, 0, 0, 1, 235});
        const poly c2 = karatsuba2(a2, b2);
        assert_s(c2 == poly({3, 10, 22, 57, 419, 776, 1128, 1391, 297, 1100, 1828, 2513, 3390, 24022, 8339, 710, 1243, 16003, 5405}), "ka, failed");
    }

    //Karatsuba test
    srand(time(NULL));

    uint max_degree = 1000;
    for (uint deg = 999; deg < max_degree; deg++){
        vector<mpz_class> poly1_coeffs(deg+1);
        vector<mpz_class> poly2_coeffs(deg+1);

        for (uint i=0; i < deg+1; i++){
            int coeff1, coeff2;
            do {
                coeff1 = rand();
                coeff2 = rand();
            } while (coeff1==0 || coeff2==0);
            poly1_coeffs[i] = coeff1;
            poly2_coeffs[i] = coeff2;
        }

        const poly ma = make_poly(poly1_coeffs);
        const poly mb = make_poly(poly2_coeffs);
        const poly correct = ma * mb;

        const double naive = TestMultiplication(ma, mb, correct, algo::naive());
        const double karatsuba = TestMultiplication(ma, mb, correct, algo::karatsuba());
        const double karatsuba2 = TestMultiplication(ma, mb, correct, algo::karatsuba2());

        cerr << "time breakdown (milliseconds):"  << endl;
        cerr << "  naive     : " << naive << endl;
        cerr << "  karatsuba : " << karatsuba << endl;
        cerr << "  karatsuba2: " << karatsuba2 << endl;
    }
}

/* vim:set shiftwidth=4 ts=4 sts=4 et: */
