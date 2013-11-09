#!/usr/bin/evn python2

import numpy as np
import os
from sklearn import cross_validation, grid_search, \
    metrics, preprocessing, naive_bayes

BREAST_CANCER_DATA='datasets/wdbc/breast-cancer-wisconsin.data'

def load_breast_cancer_data(fname):
  with open(fname, 'r') as infp:
    lines = infp.readlines()

  def smarttoint(s):
    if s == '?':
      return 1
    return int(s)

  lines = [map(smarttoint, l.strip().split(',')[1:]) for l in lines]
  # format is [ID, ATTRIBS ..., DIAG]
  # ignore ID

  def extractlabel(v):
    assert v[-1] == 2 or v[-1] == 4
    return 1 if v[-1] == 4 else 0

  def extractfeatures(v):
    def adj(x):
      assert x >= 1 and x <= 10
      return x - 1
    return map(adj, v[:-1])

  X = np.array(map(extractfeatures, lines))
  Y = np.array(map(extractlabel, lines))

  X_train, X_test, Y_train, Y_test = \
    cross_validation.train_test_split(X, Y, test_size=0.2)

  return X_train, X_test, Y_train, Y_test

if __name__ == '__main__':

  datasets = [
    load_breast_cancer_data(BREAST_CANCER_DATA),
  ]

  np.seterr(all='raise')

  for X_train, X_test, Y_train, Y_test in datasets:
    a_params = [
        {'alpha':[0.0, 0.25, 0.5, 0.75, 1.0, 2.0]}
    ]

    clf = grid_search.GridSearchCV(
        naive_bayes.MultinomialNB(),
        a_params)
    clf.fit(X_train, Y_train)

    print clf.best_estimator_
    #print clf.grid_scores_

    print "metrics on training data"
    print metrics.classification_report(Y_train, clf.predict(X_train))
    print

    print "metrics on testing data"
    print metrics.classification_report(Y_test, clf.predict(X_test))
