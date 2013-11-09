#!/usr/bin/evn python2

import numpy as np
import os
from sklearn import cross_validation, grid_search, \
    metrics, preprocessing, tree
from loaders import *

if __name__ == '__main__':

  datasets = [
    load_simple_breast_cancer_data(SIMPLE_BREAST_CANCER_DATA),
    load_audiology_data(AUDIOLOGY_TRAIN_DATA, AUDIOLOGY_TEST_DATA),
    load_nursery_data(NURSERY_DATA),
  ]

  np.seterr(all='raise')

  PRINT_TREES=False
  if PRINT_TREES:
    import StringIO
    import pydot

  for idx, (X_train, X_test, Y_train, Y_test) in enumerate(datasets):

    clf = tree.DecisionTreeClassifier()
    clf.fit(X_train, Y_train)

    print "metrics on training data"
    print metrics.classification_report(Y_train, clf.predict(X_train))
    print

    print "metrics on testing data"
    print metrics.classification_report(Y_test, clf.predict(X_test))

    if PRINT_TREES:
      dot_data = StringIO.StringIO()
      tree.export_graphviz(clf, out_file=dot_data)
      graph = pydot.graph_from_dot_data(dot_data.getvalue())
      graph.write_pdf("dtree-%d.pdf" % (idx))
