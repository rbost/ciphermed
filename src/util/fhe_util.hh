static ZZX makeIrredPoly(long p, long d)
{
    assert(d >= 1);
    assert(ProbPrime(p));
    
    if (d == 1) return ZZX(1, 1); // the monomial X
    
    zz_pBak bak; bak.save();
    zz_p::init(p);
    return to_ZZX(BuildIrred_zz_pX(d));
}

static FHEcontext* create_FHEContext(long p, long r, long d, long c, long L, long s, long k, long chosen_m )
{
    long m = FindM(k, L, c, p, d, s, chosen_m, true);
    FHEcontext *fhe_context = new FHEcontext(m, p, r);
    buildModChain(*fhe_context, L, c);
    return fhe_context;
}