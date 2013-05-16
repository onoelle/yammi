#!/bin/bash
# little minimal smoketest for yammi

echo "./configure, make"
echo "###############################################################"
./configure
make

echo "./configure --disable-id3lib"
echo "###############################################################"
./configure --disable-id3lib
make

echo "./configure --disable-ogglibs"
echo "###############################################################"
./configure --disable-ogglibs
make

echo "./configure --disable-xmms"
echo "###############################################################"
./configure --disable-xmms
make


echo "./configure --disable-id3lib --disable-ogglibs"
echo "###############################################################"
./configure --disable-id3lib --disable-ogglibs
make

echo "./configure --disable-id3lib --disable-xmms"
echo "###############################################################"
./configure --disable-id3lib --disable-xmms
make

echo "./configure --disable-ogglibs --disable-xmms"
echo "###############################################################"
./configure --disable-ogglibs --disable-xmms
make

echo "./configure --disable-id3lib --disable-ogglibs --disable-xmms"
echo "###############################################################"
./configure --disable-id3lib --disable-ogglibs --disable-xmms
make


echo "finished!"
