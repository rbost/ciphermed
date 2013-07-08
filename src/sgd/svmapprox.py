#!/usr/bin/env python

"""
A (plaintext) evaluation of approximating hinge-loss SVM
with more FHE friendly operations
"""

from sklearn.datasets import load_digits
from sklearn.linear_model import SGDClassifier
from sklearn.cross_validation import train_test_split
from sklearn.metrics import classification_report
from sklearn.preprocessing import StandardScaler

import numpy as np

class SimpleSGD(object):
  def __init__(self, alpha, eta, niters):
    self._alpha = alpha
    self._eta = eta
    self._niters = niters

  def _loss(self, y, w, x, j, state):
    """
    Overridden by subclasses

    returns (loss, state)
    """
    raise Exception("need to override")

  def fit(self, X, Y):
    Ycopy = Y.copy()
    Ycopy[np.where(Ycopy == 0)] = -1.0
    Y = Ycopy
    n, d = X.shape
    w = np.random.normal(size=d)
    for t in xrange(self._niters):
      # shuffle X, Y
      p = np.random.permutation(len(X))
      Xp, Yp = X[p], Y[p]
      for x, y in zip(Xp, Yp):
        state = None
        for j in xrange(d):
          g = self._alpha * w[j] / n
          l, state = self._loss(y, w, x, j, state)
          w[j] -= self._eta * (l + g)
      self._eta *= 0.9
    self._w = w
    return self

  def predict(self, X):
    evals = (X * self._w).sum(axis=1)
    ret = np.ones(X.shape[0])
    ret[np.where(evals < 0.0)] = 0
    return ret

class HingeSVM(SimpleSGD):
  def _loss(self, y, w, x, j, state):
    if state is None:
      state = y * np.dot(w, x)
    if state < 1.0:
      return -(y * x[j]), state
    return 0.0, state

class ApproxHingeSVM(SimpleSGD):
  def _loss(self, y, w, x, j, state):
    # see polysolve.py - this is a degree 5
    # least-squares approximation to hinge loss
    # in a bounded region [-60, 60]
    a0 = 2.58472098e+00
    a1 = -5.40711736e-01
    a2 = 1.55222467e-02
    a3 = 2.55123532e-05
    a4 = -2.36061625e-06
    a5 = -4.90336040e-09

    if state is None:
      state = y * np.dot(w, x)

    t1 = a1
    t2 = 2. * a2 * (state)
    t3 = 3. * a3 * (state ** 2)
    t4 = 4. * a4 * (state ** 3)
    t5 = 5. * a5 * (state ** 4)

    return y * x[j] * (t1 + t2 + t3 + t4 + t5), state

def evaluate(clf, Xall, Yall):
  Xtrain, Xtest, Ytrain, Ytest = train_test_split(Xall, Yall, test_size=0.25)
  scaler = StandardScaler()
  scaler.fit(Xtrain)
  Xtrain = scaler.transform(Xtrain)
  Xtest = scaler.transform(Xtest)
  clf.fit(Xtrain, Ytrain)
  Ytrue, Ypred = Ytest, clf.predict(Xtest)
  print classification_report(Ytrue, Ypred)
  print

if __name__ == '__main__':

  d = load_digits(2)
  Xall, Yall = d['data'], d['target']

  #clf = SGDClassifier(loss='hinge', penalty='l2')
  #evaluate(clf, Xall, Yall)

  clf = HingeSVM(alpha=0.0001, eta=0.001, niters=15)
  evaluate(clf, Xall, Yall)

  clf = ApproxHingeSVM(alpha=0.0001, eta=0.001, niters=15)
  evaluate(clf, Xall, Yall)
