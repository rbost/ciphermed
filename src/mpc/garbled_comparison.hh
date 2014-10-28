//
//  garbled_comparison.h
//  ciphermed-proj
//
//  Created by Raphael Bost on 27/10/2014.
//
//

#ifndef __ciphermed_proj__garbled_comparison__
#define __ciphermed_proj__garbled_comparison__

#include <vector>
#include <gmpxx.h>

#include <crypto/gm.hh>

#include <mpc/comparison_protocol.hh>

extern "C" {
    #include <justGarble.h>
}

class GC_Compare_A : public Comparison_protocol_A {
public:
    
    GC_Compare_A(const mpz_class &x, const size_t &l, GM &gm, gmp_randstate_t state);
    void set_value(const mpz_class &x) { a_ = x; };

    void prepare_circuit();
    GarbledTable* get_garbled_table(){ return gc_->garbledTable; };
    OutputMap get_output_map(){ return outputMap_; };
    InputLabels get_input_labels(){ return gc_->inputLabels; };
    
    
    GarbledCircuit* get_garbled_circuit(){ return gc_; };

    
    
    GM gm() const { return gm_; }
    size_t bit_length() const { return bit_length_; }
    virtual void set_bit_length(size_t l) {bit_length_ = l;}
    
    mpz_class output() const { return res_; }
    

    mpz_class a_;
protected:
    long s_;
    size_t bit_length_; // bit length of the numbers to compare
    
    GarbledCircuit *gc_;
    OutputMap outputMap_;
    
    GM gm_;
    gmp_randstate_t randstate_;
    
    mpz_class res_;

};



class GC_Compare_B : public Comparison_protocol_B {
public:
    
    GC_Compare_B(const mpz_class &y, const size_t &l, GM_priv &gm);
    void set_value(const mpz_class &y) { b_ = y; };
    
    void prepare_circuit();
    void set_garbled_table(GarbledTable* gt){ gc_->garbledTable = gt; };
    void evaluateGC(InputLabels inputLabels, OutputMap outputMap);
    
    GarbledCircuit* get_garbled_circuit(){ return gc_; };

    
    GM_priv gm() const { return gm_; };
    size_t bit_length() const { return bit_length_; }
    virtual void set_bit_length(size_t l) {bit_length_ = l;}
    

    mpz_class b_;
protected:
    size_t bit_length_; // bit length of the numbers to compare
    GarbledCircuit *gc_;
    GM_priv gm_;
};

int CompareCircuit(GarbledCircuit *gc, GarblingContext *garblingContext, int n,               int* inputs, int* outputs);

void runProtocol(GC_Compare_A &party_a, GC_Compare_B &party_b, gmp_randstate_t state);
inline void runProtocol(GC_Compare_A *party_a, GC_Compare_B *party_b, gmp_randstate_t state)
{
    runProtocol(*party_a,*party_b, state);
}
#endif /* defined(__ciphermed_proj__garbled_comparison__) */
