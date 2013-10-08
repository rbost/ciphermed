#include <tree/m_variate_poly.hh>

Ctxt evalTerm_FHE(const Term< vector<long> > &term, const vector<Ctxt> &vals, const EncryptedArray &ea, bool useShallowCircuit)
{
    // we want at least one value (otherwise, all of this is useless ...)
    assert(vals.size() > 0);
    
    const FHEPubKey &pk = vals[0].getPubKey();
    
    
    if (term.variables().size() == 0) {
        Ctxt c = Ctxt(pk);
        ea.encrypt(c,pk,term.coefficient());
        return c;
    }
    
    Ctxt c(pk);
    if (useShallowCircuit) {
        vector<Ctxt> operands;
        operands.reserve(term.variables().size());
        
        for (size_t i = 0; i < term.variables().size(); i++) {
            operands.insert(operands.end(),vals[term.variables()[i]]);
        }
        
        c = shallowMultiplication(operands,ea);

    }else{
        c = vals[term.variables()[0]];

        for (size_t i = 1; i < term.variables().size(); i++) {
//            c.multiplyBy(vals[term.variables()[i]]);
            c*= vals[term.variables()[i]];
        }
    }

    
    ZZX coeffPoly;
    ea.encode(coeffPoly,term.coefficient());
    
    c.multByConstant(coeffPoly);
        
    return c;
}

Ctxt shallowMultiplication(const vector<Ctxt> &terms, const EncryptedArray &ea)
{
    assert(terms.size() > 0);
    
    if (terms.size() == 1) {
        return terms[0];
    }
    
    if (terms.size() == 2) {
        Ctxt c = terms[0];
        c*= terms[1];
        
        return c;
    }
    size_t n_size = terms.size()/2;
    vector<Ctxt> newTerms;
    newTerms.reserve(n_size + (terms.size() %2));
    
    for (size_t i = 0; i < n_size; i++) {
        Ctxt c = terms[2*i];
        c*= terms[2*i+1];
//        c.multiplyBy(terms[2*i+1]);

        newTerms.insert(newTerms.end(),c);
    }
    if (terms.size() & 1) {
        newTerms.insert(newTerms.end(),terms[2*n_size]);
    }
    
    return shallowMultiplication(newTerms,ea);
}

Ctxt evalPoly_FHE(const Multivariate_poly< vector<long> > &poly, const vector<Ctxt> &vals, const EncryptedArray &ea, bool useShallowCircuit)
{
    assert(vals.size() > 0);

    const FHEPubKey &pk = vals[0].getPubKey();
    size_t s = poly.terms().size();

    if (s == 0) {
        Ctxt c = Ctxt(pk);
        PlaintextArray pa(ea);
        pa.encode(0);
        ea.encrypt(c,pk,pa);
        return c;
    }
    
    Ctxt c = evalTerm_FHE(poly.terms()[0], vals,ea,useShallowCircuit);
    
    for (size_t i = 1; i < s; i++) {
        c.addCtxt(evalTerm_FHE(poly.terms()[i], vals,ea,useShallowCircuit));
    }

    return c;
}