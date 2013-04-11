#include <iostream>
#include <sys/time.h>

#include <math/matrix.hh>
#include <math/mpz_class.hh>
#include <util/util.hh>

using namespace std;

static poly
make_poly(const vector<mpz_class> &v)
{
    return poly(v);
}

int
main(int ac, char **av)
{
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

    poly g = make_poly({3, 15, 20}); // 20x^2 + 15x + 3
    assert_s(mpz_class(3) == g(mpz_class(0)), "p(0) incorrect");
    assert_s(mpz_class(38) == g(mpz_class(1)), "p(1) incorrect");
    assert_s(mpz_class(383) == g(mpz_class(4)), "p(4) incorrect");

    // test modulo q w/ remainder shift into -q/2, q/2 (exact bounds depend
    // on q even or odd)

    poly xx({-6});
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

    //Karatsuba test
    srand(time(NULL));

    uint max_degree = 1000;

    for(uint deg = 999; deg < max_degree; deg++){
        vector<mpz_class> poly1_coeffs(deg+1);
        vector<mpz_class> poly2_coeffs(deg+1);

        for(uint i=0; i < deg+1; i++){
            int coeff1, coeff2;
            do{
                coeff1 = rand();
                coeff2 = rand();
            }while(coeff1==0 || coeff2==0);
            poly1_coeffs[i] = coeff1;
            poly2_coeffs[i] = coeff2;

        }

        poly ma = make_poly(poly1_coeffs);
        poly mb = make_poly(poly2_coeffs);

        struct timeval begin_k_time, end_k_time, begin_norm_time, end_norm_time;

        gettimeofday(&begin_k_time, 0);
        poly mc = karatsuba(ma, mb);
        gettimeofday(&end_k_time, 0);
        float k_time = end_k_time.tv_sec - begin_k_time.tv_sec + \
                       (end_k_time.tv_usec*1.0)/(1000000.0) - (begin_k_time.tv_usec*1.0)/(1000000.0);

        gettimeofday(&begin_norm_time, 0);
        poly mcorrect = ma*mb;
        gettimeofday(&end_norm_time, 0);
        float norm_time = end_norm_time.tv_sec - begin_norm_time.tv_sec + \
                          (end_norm_time.tv_usec*1.0)/(1000000.0) - (begin_norm_time.tv_usec*1.0)/(1000000.0);

        if( !(mc == mcorrect) ){
            cerr << "karatsuba is wrong!" << endl;
            //        cerr << "polyA: " << ma << endl;
            //        cerr << "polyB: " << mb << endl;
            //        cerr << "calculated poly = " << mc << endl;
            //        cerr << "correct poly = " << mcorrect << endl;
            cerr <<" karatsuba time: " << k_time <<" normal time: " << norm_time << endl;

        }else{
            cerr << "karatsuba is correct for degree " << deg << endl;
            cerr <<" karatsuba time: " << k_time <<" normal time: " << norm_time << endl;
        }


    }
}

/* vim:set shiftwidth=4 ts=4 sts=4 et: */
