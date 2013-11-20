#!/usr/bin/evn python2

import numpy as np
import os
from sklearn import cross_validation, grid_search, \
    metrics, preprocessing, svm
from loaders import *
#from util import *

if __name__ == '__main__':

  datasets = [
    ('wdbc', load_wdbc_breast_cancer_data(WDBC_BREAST_CANCER_DATA)),
    ('har', load_har_data(HAR_DATA_BASEFOLDER)),
    ('credit', load_credit_data(CREDIT_DATA)),
  ]

  np.seterr(all='raise')

  for name, (X_train, X_test, Y_train, Y_test) in datasets:

    c_params = [
        {'C':[1e-4,1e-3,1e-2,1e-1,1,10,100,1000]}
    ]

    clf = grid_search.GridSearchCV(
        svm.LinearSVC(class_weight='auto'),
        c_params)
    clf.fit(X_train, Y_train)

    print "metadata"
    print "data matrix shape:", X_train.shape
    print "estimator coef shape:", clf.best_estimator_.coef_.shape

    print clf.best_estimator_
    #print clf.best_estimator_.coef_      # the weights
    #print clf.best_estimator_.intercept_ # the intercept
    #print clf.grid_scores_

    print "metrics on training data"
    print metrics.classification_report(Y_train, clf.predict(X_train))
    print

    print "metrics on testing data"
    print metrics.classification_report(Y_test, clf.predict(X_test))

    with open(os.path.join('out', name + '.model'), 'w+') as fp:
      coef_shape = clf.best_estimator_.coef_.shape
      assert coef_shape[0] == 1
      for w in clf.best_estimator_.coef_[0]:
        print >>fp, w
      assert clf.best_estimator_.intercept_.shape == (1,)
      print >>fp, clf.best_estimator_.intercept_[0]
