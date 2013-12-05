#!/usr/bin/evn python2

import numpy as np
import os
from naive_bayes import *
from sklearn import cross_validation, grid_search, \
    metrics, preprocessing, naive_bayes
from loaders import *

if __name__ == '__main__':

  datasets = [
    load_simple_breast_cancer_data(SIMPLE_BREAST_CANCER_DATA),
    #load_audiology_data(AUDIOLOGY_TRAIN_DATA, AUDIOLOGY_TEST_DATA),
    load_nursery_data(NURSERY_DATA),
  ]

  np.seterr(all='raise')

  for X_train, X_test, Y_train, Y_test in datasets:
    best = discrete_nb_clf()
    best.fit(X_train, Y_train)

    print "metrics on training data"
    print metrics.classification_report(Y_train, best.predict(X_train))
    print

    print "metrics on testing data"
    print metrics.classification_report(Y_test, best.predict(X_test))
