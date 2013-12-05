import os
import errno

def mkdirp(path):
  # worst idiom ever
  try:
    os.makedirs(path)
  except OSError as exc:
    if exc.errno == errno.EEXIST:
      pass
    else:
      raise
