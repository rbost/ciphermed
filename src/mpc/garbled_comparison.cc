//
//  garbled_comparison.cc
//  ciphermed-proj
//
//  Created by Raphael Bost on 27/10/2014.
//
//

#include "garbled_comparison.hh"

GarbledCircuit* create_comparison_circuit(GarblingContext *garblingContext, size_t l, OutputMap *om)
{
    GarbledCircuit * gc = (GarbledCircuit *)malloc(sizeof(GarbledCircuit));
    
    int n = 2;
    int m = 1;
    int q = 10;
    int r = 5 * n;
    
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
    ANDCircuit(gc, garblingContext, n, inp, outputs);
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
    
    int inputs[2] = {0,1};
    
    int n = 2;
    int m = 1;
    block extractedLabels[n];
    block computedOutputMap[m];
    int outputVals[m];

    extractLabels(extractedLabels, gc_a->inputLabels, inputs, n);
    evaluate(gc_a, extractedLabels, computedOutputMap);
    
    mapOutputs(party_a.get_output_map(), computedOutputMap, outputVals, m);

    printf("%d & %d = %d\n", inputs[0], inputs[1], outputVals[0]);
}