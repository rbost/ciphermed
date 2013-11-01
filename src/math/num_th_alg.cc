#include <math/num_th_alg.hh>
#include <math/math_util.hh>


/* The algorithms here are from Victor Shoup's Book
 * A Computational Introduction to Number Theory and Algebra
 * or Shoup's NTL library (for Sophie Germain primes)
 */

std::vector<mpz_class> gen_rand_non_increasing_seq(const mpz_class &m, gmp_randstate_t state)
{
    std::vector<mpz_class> seq;
    
    mpz_class n = m;
    mpz_class new_n;
    do {
        // pick a new n between 1...n
        mpz_urandomm(new_n.get_mpz_t(),state,n.get_mpz_t());
        n = new_n + 1;
        seq.push_back(n);
    } while (n != 1);
    
    return seq;
}

std::vector<mpz_class> extract_prime_seq(const std::vector<mpz_class> &seq, int reps)
{
    std::vector<mpz_class> primes;
    for (size_t i = 0; i < seq.size(); i++) {
        if (mpz_class_probab_prime_p(seq[i],reps) != 0) {
            primes.push_back(seq[i]);
        }
    }
    
    return primes;
}

// generates a random integer with its factorization (returns the factorization)
std::vector<mpz_class> gen_rand_number_factorization(const mpz_class &m, mpz_class *result, gmp_randstate_t state, int reps)
{
    for (; ; ) {
        std::vector<mpz_class> seq = gen_rand_non_increasing_seq(m,state);
        std::vector<mpz_class> primes = extract_prime_seq(seq, reps);
        
        mpz_class y = 1;
        for (size_t i = 0; i < primes.size(); i++) {
            y *= primes[i];
            if (y > m) {
                break;
            }
        }
        if (y > m) {
            continue;
        }
        
        mpz_class x;
        mpz_urandomm(x.get_mpz_t(),state,m.get_mpz_t());
        x += 1;
        
        if (x <= y) {
            if (result) {
                *result = y;
            }
            return primes;
        }
    }
}

// generates a random prime p with the factorization of p-1 (returns the factorization)
std::vector<mpz_class> gen_rand_prime_with_factorization(const mpz_class &m, mpz_class *p, gmp_randstate_t state, int reps)
{
    for (; ; ) {
        mpz_class n;
        std::vector<mpz_class> factorization = gen_rand_number_factorization(m,&n,state,reps);
        
        if (mpz_class_probab_prime_p(n+1, reps) != 0) {
            if (p) {
                *p = n+1;
            }
            return factorization;
        }
    }
}
