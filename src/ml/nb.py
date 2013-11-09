#!/usr/bin/evn python2

import numpy as np
import os
from sklearn import cross_validation, grid_search, \
    metrics, preprocessing, naive_bayes

BREAST_CANCER_DATA='datasets/wdbc/breast-cancer-wisconsin.data'
AUDIOLOGY_TRAIN_DATA='datasets/audiology/audiology.standardized.data'
AUDIOLOGY_TEST_DATA='datasets/audiology/audiology.standardized.test'

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

def load_audiology_data(train_fname, test_fname):

  schema = [
    ['f','t'],
    ['mild','moderate','severe','normal','profound'],
    ['f','t'],
    ['normal','elevated','absent'],
    ['normal','absent','elevated'],
    ['mild','moderate','normal','unmeasured'],
    ['f','t'],
    ['normal','degraded'],
    ['f','t'],
    ['f','t'],
    ['f','t'],
    ['f','t'],
    ['f','t'],
    ['f','t'],
    ['f','t'],
    ['f','t'],
    ['f','t'],
    ['f','t'],
    ['f','t'],
    ['f','t'],
    ['f','t'],
    ['f','t'],
    ['f','t'],
    ['f','t'],
    ['f','t'],
    ['f','t'],
    ['f','t'],
    ['f','t'],
    ['f','t'],
    ['f','t'],
    ['f','t'],
    ['f','t'],
    ['f','t'],
    ['f','t'],
    ['f','t'],
    ['f','t'],
    ['f','t'],
    ['f','t'],
    ['f','t'],
    ['f','t'],
    ['f','t'],
    ['f','t'],
    ['f','t'],
    ['f','t'],
    ['f','t'],
    ['f','t'],
    ['f','t'],
    ['f','t'],
    ['f','t'],
    ['f','t'],
    ['f','t'],
    ['f','t'],
    ['f','t'],
    ['f','t'],
    ['f','t'],
    ['f','t'],
    ['f','t'],
    ['f','t'],
    ['normal','elevated','absent'],
    ['normal','absent','elevated'],
    ['f','t'],
    ['f','t'],
    ['f','t'],
    ['normal','good','very_good','very_poor','poor','unmeasured'],
    ['f','t'],
    ['a','as','b','ad','c'],
    ['f','t'],
    ['f','t'],
    ['f','t'],
    ['cochlear_unknown','mixed_cochlear_age_fixation','poss_central',
     'mixed_cochlear_age_otitis_media','mixed_poss_noise_om',
     'cochlear_age','normal_ear','cochlear_poss_noise','cochlear_age_and_noise',
     'acoustic_neuroma','mixed_cochlear_unk_ser_om','conductive_discontinuity',
     'retrocochlear_unknown','conductive_fixation','bells_palsy',
     'cochlear_noise_and_heredity','mixed_cochlear_unk_fixation',
     'otitis_media','possible_menieres','possible_brainstem_disorder',
     'cochlear_age_plus_poss_menieres','mixed_cochlear_age_s_om',
     'mixed_cochlear_unk_discontinuity','mixed_poss_central_om']]

  schema_map = [ { v : k for k, v in enumerate(s) } for s in schema ]
  print schema_map[-1]

  def load_file(fname):
    with open(fname, 'r') as infp:
      lines = infp.readlines()

    # strip out the useless identifier column
    lines = [l.strip().split(',') for l in lines]
    lines = [l[:-2]+l[-1:] for l in lines]

    def extractlabel(l):
      return schema_map[-1][l[-1]]

    def extractfeatures(l):
      return [sm[v] if v != '?' else 0 for sm, v in zip(schema_map, l)]

    Y = np.array(map(extractlabel, lines))
    X = np.array(map(extractfeatures, lines))

    return X, Y

  X_train, Y_train = load_file(train_fname)
  X_test, Y_test = load_file(test_fname)

  return X_train, X_test, Y_train, Y_test

if __name__ == '__main__':

  datasets = [
    load_breast_cancer_data(BREAST_CANCER_DATA),
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
