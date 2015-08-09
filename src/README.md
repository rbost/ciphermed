# ciphermed
Implementation of "Machine Learning Classification over Encrypted Data" by Raphael Bost, Raluca Ada Popa, Stephen Tu and Shafi Goldwasser

The code has been mostly written by Raphael Bost, with some lines from Raluca Ada Popa and Stephen Tu, and fixes by [nescio007](https://github.com/nescio007).
 
It is available under the General Public License (GPL) version 3.
 
## Compilation & Prerequisites

Ciphermed builds on [JustGarble](http://cseweb.ucsd.edu/groups/justgarble/) which needs AES-NI enable on your CPU. Try `less /proc/cpuinfo | grep aes` to ensure these instructions are available. If not, you might have to disable all the garbled circuit-based code, or replace AES-NI calls by software AES.

Ciphermed directly uses GMP, HELib, boost, and others. You will need to install these and their own dependencies.

### [GMP](https://gmplib.org)

The C++ support of GMP is used in Ciphermed, so be sure to use the option `--enable-cxx` when calling the `configure` script.

### HELib

[HELib](https://github.com/shaih/HElib) is an experimental Level Homomorphic Encryption library by Shai Halevi and Victor Shoup that builds on NTL (and GMP). So be sure to have NTL installed.

Note that when building NTL, be sure to make GMP the underlying large integer library by calling `./configure NTL_GMP_LIP=on`.
For better performances, you might also want to use [GF2X](https://gforge.inria.fr/projects/gf2x/) (downloadable [here](https://gforge.inria.fr/frs/download.php/file/30873/gf2x-1.1.tar.gz)). 
In this case, call `./configure NTL_GMP_LIP=on NTL_GF2X_LIB=on`.
You will also need to change HELib's makefile (see later).


To be properly used in Ciphermed, HELib's makefile needs to be modified: be sure to add `-fPIC` to `CFLAGS`:

``CFLAGS = -g -O2 -fPIC -std=c++11``

If you decided to use gf2x in NTL, to pass checks, don't forget to add `-lfg2x` in `LDLIBS`. Test programs won't compile otherwise. 

### Other dependencies

Ciphermed uses the following external libraries:

* boost (for the sockets)
* msg_pack (needed by JustGarble)
* OpenSSL (idem)
* Google's protocol buffers and the protoc compiler (for serialization)
* JsonCpp (for ML models I/O)


On Ubuntu, these are available as apt packages and can be installed using the command
``sudo apt-get install libboost-system-dev libmsgpack-dev libssl-dev libprotoc-dev protobuf-compiler libjsoncpp-dev
``

## Building on Mac OS X

gcc's version installed on current versions of Mac OS is completely outdated.
So if you want to build Ciphermed on your Mac OS X machine, you have two solutions:

* build and install and new version of gcc (painful)
* use clang instead

Unfortunately, Ciphermed's Makefile relies on gcc-only options, and is not immediately portable to clang. 
However, a `clang` branch of Ciphermed exists and should work properly. This branch might not necessarily include all the changes/fixes of the master branch, and you might have to debug it.


## Building models for tests

Ciphermed codebase includes some Python scripts by Stephen Tu in the `src/ml` directory.
These scripts are known to be working with Python 2.7.6 and use the `numpy` and `sklearn` modules.

You can install them on Ubuntu/Debian by entering in your terminal
``sudo apt-get install python-numpy python-sklearn
``

