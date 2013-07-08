#!/usr/bin/env python

import sys
import subprocess
import itertools
import multiprocessing
import pickle

BINARY='./obj/shaifhe/test'

def async_run_config(config):
  """
  config is dict obj
  """

  cmd = [BINARY] + list(itertools.chain.from_iterable((['-' + k, str(v)] for k, v in config.iteritems())))
  proc = subprocess.Popen(
      cmd,
      stdin=open('/dev/null', 'r'),
      stdout=subprocess.PIPE,
      stderr=open('/dev/null', 'w'))
  return proc

def block_config_run(proc):
  results = proc.stdout.readlines()
  ret = proc.wait()
  assert ret == 0

  resmap = {}
  for line in results:
    k, v = line.strip().split('=')
    resmap[k] = float(v)
  return resmap

if __name__ == '__main__':
  _, outfile = sys.argv
  ncpus = multiprocessing.cpu_count()

  r_values = (1, 2, 4, 8, 16)
  L_values = (1, 2, 4, 8, 16)

  all_configs = \
    [{'r':r} for r in r_values] + \
    [{'L':L} for L in L_values]

  procs = [async_run_config(cfg) for cfg in all_configs]
  results = [block_config_run(proc) for proc in procs]

  with open(outfile, 'w') as fp:
    pickle.dump({'configs':all_configs, 'results':results}, fp)
