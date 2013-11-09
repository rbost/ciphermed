#!/usr/bin/evn python2

import numpy as np
import os
from sklearn import cross_validation, grid_search, \
    metrics, preprocessing, naive_bayes
from loaders import *

if __name__ == '__main__':

  datasets = [
    load_simple_breast_cancer_data(SIMPLE_BREAST_CANCER_DATA),
    load_audiology_data(AUDIOLOGY_TRAIN_DATA, AUDIOLOGY_TEST_DATA),
  ]

  np.seterr(all='raise')

  for X_train, X_test, Y_train, Y_test in datasets:
    a_params = [
        {'alpha':[1e-6, 1e-3, 0.25, 0.5, 0.75, 1.0, 2.0]}
    ]

    smallest_class_occur = min(np.bincount(Y_train))
    if smallest_class_occur == 1:
      # cannot do k-fold
      clfs = [naive_bayes.MultinomialNB(alpha=a) for a in a_params[0]['alpha']]
      for clf in clfs:
        clf.fit(X_train, Y_train)
      scores = np.array([metrics.accuracy_score(Y_train, clf.predict(X_train)) for clf in clfs])
      best = clfs[np.argmax(scores)]
    else:
      cv = min(3, smallest_class_occur)
      clf = grid_search.GridSearchCV(
          naive_bayes.MultinomialNB(),
          a_params, cv=cv)
      clf.fit(X_train, Y_train)
      best = clf.best_estimator_

    print "metrics on training data"
    print metrics.classification_report(Y_train, best.predict(X_train))
    print

    print "metrics on testing data"
    print metrics.classification_report(Y_test, best.predict(X_test))
