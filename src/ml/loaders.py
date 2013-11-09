import numpy as np
import os
from sklearn import cross_validation, preprocessing

# path constants
WDBC_BREAST_CANCER_DATA='datasets/wdbc/wdbc.data'

SIMPLE_BREAST_CANCER_DATA='datasets/wdbc/breast-cancer-wisconsin.data'

HAR_DATA_BASEFOLDER='datasets/har/har'

AUDIOLOGY_TRAIN_DATA='datasets/audiology/audiology.standardized.data'
AUDIOLOGY_TEST_DATA='datasets/audiology/audiology.standardized.test'

NURSERY_DATA='datasets/nursery/nursery.data'

def load_wdbc_breast_cancer_data(fname):
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

def load_simple_breast_cancer_data(fname):
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

def load_nursery_data(fname):
  with open(fname, 'r') as infp:
    lines = infp.readlines()

  schema = [
    ['usual','pretentious','great_pret'],
    ['proper','less_proper','improper','critical','very_crit'],
    ['complete','completed','incomplete','foster'],
    ['1','2','3','more'],
    ['convenient','less_conv','critical'],
    ['convenient','inconv'],
    ['nonprob','slightly_prob','problematic'],
    ['recommended','priority','not_recom'],
    ['not_recom', 'recommend', 'very_recom', 'priority', 'spec_prior'],
  ]

  schema_map = [ { v : k for k, v in enumerate(s) } for s in schema ]

  lines = [l.strip().split(',') for l in lines]

  def extractlabel(l):
    return schema_map[-1][l[-1]]

  def extractfeatures(l):
    return [sm[v] if v != '?' else 0 for sm, v in zip(schema_map, l)]

  Y = np.array(map(extractlabel, lines))
  X = np.array(map(extractfeatures, lines))

  X_train, X_test, Y_train, Y_test = \
    cross_validation.train_test_split(X, Y, test_size=0.2)

  return X_train, X_test, Y_train, Y_test
