#include <math/matrix.hh>

#include <util/util.hh>

#include <iostream>
#include <sys/time.h>

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
//		cerr << "polyA: " << ma << endl;
//		cerr << "polyB: " << mb << endl;
//		cerr << "calculated poly = " << mc << endl;
//		cerr << "correct poly = " << mcorrect << endl;
		cerr <<" karatsuba time: " << k_time <<" normal time: " << norm_time << endl;

	}else{
		cerr << "karatsuba is correct for degree " << deg << endl;
		cerr <<" karatsuba time: " << k_time <<" normal time: " << norm_time << endl;
	}


    }
}
