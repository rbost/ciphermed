//
//  garbled_comparison.cc
//  ciphermed-proj
//
//  Created by Raphael Bost on 27/10/2014.
//
//

#include "garbled_comparison.hh"

using namespace std;

#include<iostream>

extern "C"{
    #include "gates.h"
}
int OneBitCompareCircuit(GarbledCircuit *gc, GarblingContext *garblingContext, int* inputs, int* outputs);
GarbledCircuit* create_comparison_circuit(GarblingContext *garblingContext, size_t l, OutputMap *om);

int OneBitCompareCircuit(GarbledCircuit *garbledCircuit, GarblingContext *garblingContext, int* inputs, int* outputs) {
    
    // inputs[0] = x
    // inputs[1] = y
    // inputs[2] = c
    
    int x = inputs[0];
    int y = inputs[1];
    int c = inputs[2];
    
    int x_xor_c;
    int y_xor_c;
    int and_wire;
    int x_xor_and; // output
    
    x_xor_c = getNextWire(garblingContext);
    XORGate(garbledCircuit, garblingContext, x, c, x_xor_c);
    
    y_xor_c = getNextWire(garblingContext);
    XORGate(garbledCircuit, garblingContext, y, c, y_xor_c);
    
    and_wire  = getNextWire(garblingContext);
    ANDGate(garbledCircuit, garblingContext, x_xor_c, y_xor_c,and_wire);
    
    x_xor_and = getNextWire(garblingContext);
    XORGate(garbledCircuit, garblingContext, x, and_wire, x_xor_and);
    
    outputs[0] = x_xor_and;
    
    return 0;
}

int FirstRound_OneBitCompareCircuit(GarbledCircuit *garbledCircuit, GarblingContext *garblingContext, int* inputs, int* outputs) {
    // inputs[0] = x
    // inputs[1] = y
    // Same as the one before (OneBitCompareCircuit) but where c is forced to 0 (
    int x = inputs[0];
    int y = inputs[1];
    
    int x_and_y;
    int x_xor_and; // output
    

    x_and_y  = getNextWire(garblingContext);
    ANDGate(garbledCircuit, garblingContext, x, y,x_and_y);
    
    x_xor_and = getNextWire(garblingContext);
    XORGate(garbledCircuit, garblingContext, x, x_and_y, x_xor_and);
    
    outputs[0] = x_xor_and;
    
    return 0;
}


int CompareCircuit(GarbledCircuit *gc, GarblingContext *garblingContext, int n, int* inputs, int* outputs) {
    // waiting for 2*n+1 inputs where n the bit-length of integers to compare
    // inputs[2*n] is a bit c that we use to tune the circuit
    // x is encoded on even bits (inputs[0],...,inputs[2*n-2]
    // y is encoded on odd bits (inputs[1],...,inputs[2*n-1]
    
    
    FirstRound_OneBitCompareCircuit(gc, garblingContext, inputs, outputs);

    int in[3];
//    outputs[0] = inputs[2*n];
    
    int i = 0;
    for (i = 1; i < n; i++) {
        in[0] = inputs[2*i];
        in[1] = inputs[2*i+1];
        in[2] = outputs[0];
        
        OneBitCompareCircuit(gc, garblingContext, in, outputs);
    }
    
    return 0;
}


GarbledCircuit* create_comparison_circuit(GarblingContext *garblingContext, size_t l, OutputMap *om)
{
    GarbledCircuit * gc = (GarbledCircuit *)malloc(sizeof(GarbledCircuit));
    
    int n = 2*l+1; // # of input bits
    int m = 1; // one output bit : the result
    int q = 4*l-1; // number of gates : 2 (first round) + 4*(l-1) other rounds + 1 (final XOR)
    int r = n+4*l+1; // number of wires
  
    block *labels = (block*) malloc(sizeof(block) * 2 * n);
    block *outputbs = (block*) malloc(sizeof(block) * 2 * m);
    
    int *inp = (int *) malloc(sizeof(int) * n);
    countToN(inp, n);
    int outputs[1];
    
    OutputMap outputMap = outputbs;
    InputLabels inputLabels = labels;
    
    createInputLabels(labels, n);
    createEmptyGarbledCircuit(gc, n, m, q, r, inputLabels);
    
    startBuilding(gc, garblingContext);

    CompareCircuit(gc, garblingContext, l, inp, outputs);
    
    // now XOR the result with an extra bit in order to hide the result
    int out = getNextWire(garblingContext);
    XORGate(gc, garblingContext, inp[2*l], outputs[0], out);
    
    outputs[0] = out;
    
    finishBuilding(gc, garblingContext, outputMap, outputs);
    
    if (om) {
        *om = (OutputMap)outputMap;
    }
    
    gc->inputLabels = inputLabels;
    return gc;
}

GC_Compare_A::GC_Compare_A(const mpz_class &x, const size_t &l, GM &gm, gmp_randstate_t state)
: a_(x), bit_length_(l), gm_(gm)
{
    s_ = 1 - 2*gmp_urandomb_ui(state,1);
    gmp_randinit_set(randstate_, state);
}

void GC_Compare_A::prepare_circuit()
{
    GarblingContext garblingContext;
    
    gc_ = create_comparison_circuit(&garblingContext, bit_length_,&outputMap_);
    
    createInputLabels(gc_->inputLabels,gc_->n);
    
    garbleCircuit(gc_, gc_->inputLabels, outputMap_);
}


GC_Compare_B::GC_Compare_B(const mpz_class &y, const size_t &l, GM_priv &gm)
: b_(y), bit_length_(l), gm_(gm)
{
    
}


void GC_Compare_B::prepare_circuit()
{
    GarblingContext garblingContext;
    
    gc_ = create_comparison_circuit(&garblingContext, bit_length_, NULL);
}

void GC_Compare_B::evaluateGC(InputLabels extractedLabels, OutputMap outputMap)
{
    int n = gc_->n;
    int m = gc_->m;
    block computedOutputMap[m];
    int outputVals[m];

    evaluate(gc_, extractedLabels, computedOutputMap);
    mapOutputs(outputMap, computedOutputMap, outputVals, m);
    
}

void runProtocol(GC_Compare_A &party_a, GC_Compare_B &party_b, gmp_randstate_t state)
{
    party_a.prepare_circuit();
    party_b.prepare_circuit();
    
    GarbledTable *gt = party_a.get_garbled_table();
    party_b.set_garbled_table(gt);
    
    GarbledCircuit *gc_a = party_a.get_garbled_circuit();
    GarbledCircuit *gc_b = party_b.get_garbled_circuit();
    
    gc_b->globalKey = gc_a->globalKey;
    
    int l = party_a.bit_length();
    int n = 2*l+1;
    int m = 1;

    int *inputs = (int *)malloc(n*sizeof(int));
    
    char *bits_a = mpz_get_str(NULL,2,party_a.a_.get_mpz_t());
    char *bits_b =mpz_get_str(NULL,2,party_b.b_.get_mpz_t());
                              
    mpz_class a = party_a.a_;
    mpz_class b = party_b.b_;
    
    size_t i;
    for (i = 0; i < l; i++) {
        inputs[2*i] = mpz_tstbit(a.get_mpz_t(), i);
        inputs[2*i+1] = mpz_tstbit(b.get_mpz_t(), i);
    }

    for (i = 0; i < l; i++) {
        cout << inputs[2*i];
    }
    cout << endl;
    for (i = 0; i < l; i++) {
        cout << inputs[2*i+1];
    }
    cout << endl;

    inputs[2*l] = 0; // set to one to reverse the comparison result
    
    block extractedLabels[n];
    block computedOutputMap[m];
    int outputVals[m];

    extractLabels(extractedLabels, gc_a->inputLabels, inputs, n);
    evaluate(gc_b, extractedLabels, computedOutputMap);
    
    mapOutputs(party_a.get_output_map(), computedOutputMap, outputVals, m);

    cout << a << " LES " << b << " = " << outputVals[0] << endl;
    cout << a << " < " << b << " = " << (a > b) << endl;
}