#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <sys/types.h>

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

static inline ostream &
operator<<(ostream &o, const configuration &cfg)
{
  o << "{p=" << cfg.p
    << ",r=" << cfg.r
    << ",d=" << cfg.d
    << ",c=" << cfg.c
    << ",L=" << cfg.L
    << ",w=" << cfg.w
    << ",s=" << cfg.s
    << ",k=" << cfg.k
    << ",chosen_m=" << cfg.chosen_m
    << "}";
  return o;
}

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
test(const configuration &config)
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
  cerr << "nslots=" << nslots << endl;

  PlaintextArray p0(ea);
  PlaintextArray p1(ea);
  PlaintextArray p2(ea);
  PlaintextArray p3(ea);

  PlaintextArray pp0(ea);
  PlaintextArray pp2(ea);

  p0.encode({0, 0, 1});
  p0.print(cerr); cerr << endl;

  p1.encode({0, 0, 1});

  //p0.random(); // add-accumulator
  //p1.random(); // addition
  //p2.encode(2); // mult-accumulator
  //p2.negate();
  //p3.encode(3); // multiplier
  //p3.negate();

  Ctxt c0(publicKey); Ctxt c1(publicKey);
  Ctxt c2(publicKey); Ctxt c3(publicKey);

  ea.encrypt(c0, publicKey, p0);
  ea.encrypt(c1, publicKey, p1);

  ea.rotate(c0, -2);
  //c0.multiplyBy(c1);

  ea.decrypt(c0, secretKey, pp0);
  //ea.decrypt(c2, secretKey, pp2);
  vector<long> vls;
  pp0.decode(vls);
  cerr << vls.front() << endl;

  pp0.print(cerr); cerr << endl;
}

int
main(int argc, char **argv)
{
  cerr << "pid=" << getpid() << endl;

  int ch;

  long p = 2; // plaintext base
  long r = 1; // lifting
  long d = 1; // XXX: check?
  long c = 2;
  long L = 4;
  long w = 64;
  long s = 0; // XXX: check?
  long k = 80;
  long m = 0; // XXX: check?

  opterr = 0;
  while ((ch = getopt(argc, argv, "p:r:d:c:L:w:s:k:m:")) != -1) {
    switch (ch) {

#define CASE_X(c) \
    case (#c)[0]: \
      c = atol(optarg); \
      break;

    CASE_X(p)
    CASE_X(r)
    CASE_X(d)
    CASE_X(c)
    CASE_X(L)
    CASE_X(w)
    CASE_X(s)
    CASE_X(k)
    CASE_X(m)

    case '?':
    default:
      return 1;
    }
  }

  configuration cfg;

  cfg.p = p;
  cfg.r = r;
  cfg.d = d;
  cfg.c = c;
  cfg.L = L;
  cfg.w = w;
  cfg.s = s;
  cfg.k = k;
  cfg.chosen_m = m;
  cerr << cfg << endl;

  test(cfg);

  return 0;
}
