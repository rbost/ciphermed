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
                mpz_init(store[i]);

		// FAST:
		// if(mpz_set_str(store[i], "1340710934871093470931470134", 10) == -1) cout << "ERROR" << endl;

		// SLOW:
		// mpz_t v;
		// if(mpz_init_set_str(v, "1340710934871093470931470134", 10) == -1) cout << "ERROR" << endl;
		// mpz_set(store[i], v);

		// ALMOST AS FAST AGAIN:
		mpz_t v;
		if(mpz_init_set_str(v, "1340710934871093470931470134", 10) == -1) cout << "ERROR" << endl;
		_mpz_realloc(store[i], 3);
		mpz_set(store[i], v);
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
		_mpz_realloc(store[i], 4);
	}

	//Time multiplication
	gettimeofday(&begin_mult_time, 0);
	for ( int i=0 ; i< trial_run; i++ ){
		mpz_mul(store[i], store[i], store[i]);
	}
	gettimeofday(&end_mult_time, 0);

        float mult_time = end_mult_time.tv_sec - begin_mult_time.tv_sec + \
        (end_mult_time.tv_usec*1.0)/(1000000.0) - (begin_mult_time.tv_usec*1.0)/(1000000.0);

	//Make sure compiler doesn't optimize
	for ( int i=0; i < trial_run; i++ ){
		mpz_class tmp(store[i]);
		sum += tmp;
	}
	cout << sum << endl;
	cout << "mpz_t add: " <<add_time << " : " <<  add_time/(trial_run *1.0) << endl;
	cout << "mpz_t mult: " << mult_time << " : " << mult_time/(trial_run *1.0) << endl;

	return 0;
}
