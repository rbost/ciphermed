#!/usr/bin/env python

import numpy as np
import scipy as sp

import matplotlib.pyplot as pl

def hingeloss(l):
  if l < 1.0:
    return 1.0 - l
  return 0.0

def polify(x, k):
  # make x into the k+1 dimensional polynomial:
  # 1 + x + ... + x^k. representation is of form:
  # [1, x, ..., x^k]

  ret = np.ones(k+1)
  for j in xrange(1, k+1):
    ret[j] = x ** j

  #print ret
  return ret

BEGIN=-60.
END=60.

def polysolve(k, p):
  xpts = np.linspace(BEGIN, END, p)
  # inject extra pts to try and weight near the hinge
  xpts = np.hstack((xpts, np.array([hingeloss(0) for _ in xrange(1000)])))
  ypts = np.array([hingeloss(x) for x in xpts])
  X = np.ones((len(xpts), k+1))
  for i, x in enumerate(xpts):
    X[i] = polify(x, k)
  # need to solve y = Xa for a in the least squares sense
  #print X
  #print ypts
  soln, residuals, _, _ = np.linalg.lstsq(X, ypts)
  print residuals
  return soln

if __name__ == '__main__':
  k=5
  p=5000

  a = polysolve(k=k, p=p)

  print a
  print hingeloss(-1.), np.dot(a, polify(-1., k))
  print hingeloss(0.), np.dot(a, polify(0., k))
  print hingeloss(1.), np.dot(a, polify(1., k))

  xpts = np.linspace(BEGIN, END, p)
  yptshinge= np.array([hingeloss(x) for x in xpts])
  yptspoly = np.array([np.dot(a, polify(x, k)) for x in xpts])

  pl.plot(xpts, yptshinge, xpts, yptspoly)
  pl.show()
