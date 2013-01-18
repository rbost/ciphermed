#include <iostream>
#include <vector>
#include <sys/time.h>
#include <gmpxx.h>

using namespace std;

int main(){

        struct timeval begin_add_time, end_add_time,
			begin_mult_time, end_mult_time;

	int trial_run = 100000;

	mpz_t store[trial_run];

	//Init w/ large string
	for(int i=0; i< trial_run; i++){
		if(mpz_init_set_str(store[i], "1340710934871093470931470134", 10) == -1) cout << "ERROR" << endl;
	}

	//Time addition
	gettimeofday(&begin_add_time, 0);
	for ( int i=0 ; i< trial_run; i++ ){
		mpz_add(store[i], store[i], store[i]);
	}
	gettimeofday(&end_add_time, 0);

        float add_time = end_add_time.tv_sec - begin_add_time.tv_sec + \
        (end_add_time.tv_usec*1.0)/(1000000.0) - (begin_add_time.tv_usec*1.0)/(1000000.0);
	//Make sure compiler doesn't optimize
	mpz_class sum = 0;
	for ( int i=0; i < trial_run; i++ ){
		mpz_class tmp(store[i]);
		sum += tmp;
	}
	cout << sum << endl;

	//Reset
	for(int i=0; i< trial_run; i++){
		if(mpz_init_set_str(store[i], "1340710934871093470931470134", 10) == -1) cout << "ERROR" << endl;

	}

	//Time multiplication
	gettimeofday(&begin_mult_time, 0);
	for ( int i=0 ; i< trial_run; i++ ){
		mpz_mul(store[i], store[i], store[i]);
	}
	gettimeofday(&end_mult_time, 0);

        float mult_time = end_mult_time.tv_sec - begin_mult_time.tv_sec + \
        (end_mult_time.tv_usec*1.0)/(1000000.0) - (begin_mult_time.tv_usec*1.0)/(1000000.0);
	for ( int i=0; i < trial_run; i++ ){
		mpz_class tmp(store[i]);
		sum += tmp;
	}
	cout << sum << endl;

	cout << sum << endl;
	cout << "mpz_t add: " <<add_time << " : " <<  add_time/(trial_run *1.0) << endl;
	cout << "mpz_t mult: " << mult_time << " : " << mult_time/(trial_run *1.0) << endl;

	return 0;
}
