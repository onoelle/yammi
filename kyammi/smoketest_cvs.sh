#!/bin/bash

# smoketest to verify that fresh checkout from cvs (via anonymous) compiles without errors
rm -r smoketestCvs
mkdir smoketestCvs
cd smoketestCvs
cvs -z3 -d:pserver:anonymous@cvs.sourceforge.net:/cvsroot/yammi co kyammi
cd kyammi
./configure
make

echo "finished!"

