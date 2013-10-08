#include <tree/m_variate_poly.hh>

Ctxt evalTerm_FHE(const Term< vector<long> > &term, const vector<Ctxt> &vals, const EncryptedArray &ea)
{
    // we want at least one value (otherwise, all of this is useless ...)
    assert(vals.size() > 0);
    
    const FHEPubKey &pk = vals[0].getPubKey();
    
    
    if (term.variables().size() == 0) {
        Ctxt c = Ctxt(pk);
        ea.encrypt(c,pk,term.coefficient());
        return c;
    }
    
    Ctxt c = vals[term.variables()[0]];
    
    for (size_t i = 1; i < term.variables().size(); i++) {
        //        v *= vals[term.variables()[i]];
        c.multiplyBy(vals[term.variables()[i]]);
    }
    
    ZZX coeffPoly;
    ea.encode(coeffPoly,term.coefficient());
    
    c.multByConstant(coeffPoly);
    
    //    v*= term.coefficient();
    
    return c;
}

Ctxt evalPoly_FHE(const Multivariate_poly< vector<long> > &poly, const vector<Ctxt> &vals, const EncryptedArray &ea)
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
    
    Ctxt c = evalTerm_FHE(poly.terms()[0], vals,ea);
    
    for (size_t i = 1; i < s; i++) {
        c.addCtxt(evalTerm_FHE(poly.terms()[i], vals,ea));
    }

    return c;
}