static FHEcontext* create_FHEContext(long p, long r, long d, long c, long L, long s, long k, long chosen_m )
{
    long m = FindM(k, L, c, p, d, s, chosen_m, true);
    FHEcontext *fhe_context = new FHEcontext(m, p, r);
    buildModChain(*fhe_context, L, c);
    return fhe_context;
}
