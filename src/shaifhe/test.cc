#include <iostream>

#include <FHE.h>
#include <EncryptedArray.h>
#include <NTL/lzz_pXFactoring.h>

#include <util/util.hh>

using namespace std;
using namespace NTL;

static ZZX makeIrredPoly(long p, long d)
{
  assert(d >= 1);
  assert(ProbPrime(p));

  if (d == 1) return ZZX(1, 1); // the monomial X

  zz_pBak bak; bak.save();
  zz_p::init(p);
  return to_ZZX(BuildIrred_zz_pX(d));
}

struct configuration {
  long p;
  long r;
  long d;
  long c;
  long L;
  long w;
  long s;
  long k;
  long chosen_m;
};

struct results {
  double encrypt_rate_ms; // per op
  double decrypt_rate_ms; // per op

  unsigned int batchsize;
  double add_rate_batch_ms; // per batch
  double mult_rate_batch_ms; // per batch

  inline double add_rate_per_op() const { return add_rate_batch_ms / double(batchsize); }
  inline double mult_rate_per_op() const { return mult_rate_batch_ms / double(batchsize); }
};

static void
test(const configuration &config, results &res)
{
  long p = config.p;
  long r = config.r;
  long d = config.d;
  long c = config.c;
  long L = config.L;
  long w = config.w;
  long s = config.s;
  long k = config.k;
  long chosen_m = config.chosen_m;
  long m = FindM(k, L, c, p, d, s, chosen_m, true);

  FHEcontext context(m, p, r);
  buildModChain(context, L, c);

  //context.zMStar.printout();
  //cerr << endl;

  FHESecKey secretKey(context);
  const FHEPubKey& publicKey = secretKey;
  secretKey.GenSecKey(w); // A Hamming-weight-w secret key

  ZZX G;
  if (r > 1)
    G = context.alMod.getFactorsOverZZ()[0];
  else if (r == 1)
    G = makeIrredPoly(p, d);
  else
    assert(false);

  //cerr << "G = " << G << "\n";
  //cerr << "generating key-switching matrices... ";
  addSome1DMatrices(secretKey); // compute key-switching matrices that we need
  //cerr << "done\n";

  //cerr << "computing masks and tables for rotation...";
  EncryptedArray ea(context, G);
  //cerr << "done\n";

  long nslots = ea.size();
  //cerr << "nslots=" << nslots << endl;
  res.batchsize = nslots;

  PlaintextArray p0(ea);
  PlaintextArray p1(ea);
  PlaintextArray p2(ea);
  PlaintextArray pp0(ea);

  p0.random(); // accumulator
  p1.random(); // addition
  p2.encode(3); // multiplier

  Ctxt c0(publicKey); Ctxt c1(publicKey); Ctxt c2(publicKey);

  Timer timer;
  ea.encrypt(c0, publicKey, p0);
  ea.encrypt(c1, publicKey, p1);
  ea.encrypt(c2, publicKey, p2);
  res.encrypt_rate_ms = timer.lap_ms() / 3.;

  const size_t niters = 10;
  for (size_t i = 0; i < niters; i++)
    c0.addCtxt(c1);
  const double t0 = timer.lap_ms();
  res.add_rate_batch_ms = (t0 / double(niters));

  timer.lap_ms();
  for (size_t i = 0; i < 1; i++)
    c2.multiplyBy(c1);
  const double t1 = timer.lap_ms();
  res.mult_rate_batch_ms = (t1);

  timer.lap_ms();
  ea.decrypt(c0, secretKey, pp0);
  res.decrypt_rate_ms = timer.lap_ms();
}

int
main(int argc, char **argv)
{

  configuration c;

  c.p = 2;
  c.r = 16;
  //c.r = 32;
  c.d = 1; // XXX: check?
  c.c = 2;
  c.L = 4;
  c.w = 64;
  c.s = 0; // XXX: check?
  c.k = 80;
  c.chosen_m = 0; // XXX: check?

  //long m = FindM(k, L, c, p, d, s, chosen_m, true);

  results r;
  test(c, r);

  cout << "encrypt_rate_ms=" << r.encrypt_rate_ms << endl;
  cout << "decrypt_rate_ms=" << r.decrypt_rate_ms << endl;
  cout << "add_rate_batch_ms=" << r.add_rate_batch_ms << endl;
  cout << "add_rate_per_op=" << r.add_rate_per_op() << endl;
  cout << "mult_rate_batch_ms=" << r.mult_rate_batch_ms << endl;
  cout << "mult_rate_per_op=" << r.mult_rate_per_op() << endl;
  return 0;
}
