#!/usr/bin/evn python2

import numpy as np
import os
from sklearn import cross_validation, grid_search, \
    metrics, preprocessing, svm

BREAST_CANCER_DATA='datasets/wdbc/wdbc.data'
HAR_DATA_BASEFOLDER='datasets/har/har'

def load_breast_cancer_data(fname):
  with open(fname, 'r') as infp:
    lines = infp.readlines()

  lines = [l.strip().split(',') for l in lines]
  # format is [ID, DIAG, ATTRIBS ...]
  # ignore ID

  def extractlabel(v):
    assert v[1] == 'M' or v[1] == 'B'
    return 1.0 if v[1] == 'M' else 0.0

  def extractfeatures(v):
    return map(float, v[2:])

  yvalues = map(extractlabel, lines)
  xvalues = map(extractfeatures, lines)

  # SVMs are not scale invariant -- scale to 0 mean, 1 variance
  X = preprocessing.scale(xvalues)
  Y = np.array(yvalues)

  X_train, X_test, Y_train, Y_test = \
    cross_validation.train_test_split(X, Y, test_size=0.2)

  return X_train, X_test, Y_train, Y_test

def load_har_data(fname):
  def load_type(typ):
    with open(os.path.join(fname, typ, 'X_%s.txt' % (typ)), 'r') as infp:
      lines = infp.readlines()
    lines = [l.strip().split() for l in lines]
    xvalues = [map(float, l) for l in lines]
    with open(os.path.join(fname, typ, 'y_%s.txt' % (typ)), 'r') as infp:
      lines = infp.readlines()
    lines = [l.strip() for l in lines]
    yvalues = map(int, lines)
    return np.array(xvalues), np.array(yvalues)
  def load_label_map(fname):
    with open(fname, 'r') as infp:
      lines = infp.readlines()
    lines = [l.strip().split() for l in lines]
    return { b : int(a) for a, b in lines }
  label_map = load_label_map(os.path.join(fname, 'activity_labels.txt'))

  X_train, Y_train = load_type('train')
  X_test, Y_test = load_type('test')

  #print X_train.shape
  #print Y_train.shape

  #print X_test.shape
  #print Y_test.shape

  def filter_and_make_binary(X, Y, a, b):
    # for class labels:
    # a -> 0
    # b -> 1
    p = np.logical_or(Y==label_map[a], Y==label_map[b])
    X, Y = X[p], Y[p]
    Y = (Y==label_map[b]).astype(int)
    return X, Y

  # we'll do a binary task-- WALKING vs SITTING
  #A = 'WALKING_UPSTAIRS'
  #B = 'WALKING_DOWNSTAIRS'
  #A = 'SITTING'
  #B = 'LAYING'
  A = 'WALKING'
  B = 'WALKING_DOWNSTAIRS'
  X_train, Y_train = filter_and_make_binary(X_train, Y_train, A, B)
  X_test, Y_test = filter_and_make_binary(X_test, Y_test, A, B)

  print Y_train.shape
  print Y_test.shape

  return X_train, X_test, Y_train, Y_test

if __name__ == '__main__':

  datasets = [
    #load_breast_cancer_data(BREAST_CANCER_DATA),
    load_har_data(HAR_DATA_BASEFOLDER),
  ]

  np.seterr(all='raise')

  for X_train, X_test, Y_train, Y_test in datasets:

    c_params = [
        {'C':[1e-4,1e-3,1e-2,1e-1,1,10,100,1000]}
    ]

    clf = grid_search.GridSearchCV(
        svm.LinearSVC(class_weight='auto'),
        c_params)
    clf.fit(X_train, Y_train)

    print clf.best_estimator_
    print clf.best_estimator_.coef_      # the weights
    print clf.best_estimator_.intercept_ # the intercept
    #print clf.grid_scores_

    print "metrics on training data"
    print metrics.classification_report(Y_train, clf.predict(X_train))
    print

    print "metrics on testing data"
    print metrics.classification_report(Y_test, clf.predict(X_test))
