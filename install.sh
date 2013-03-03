#!/bin/sh

# Note: you'll probably need to run this as root.

cp libptp++.so /usr/lib/
mkdir /usr/include/libptp++/
cp *.hpp /usr/include/libptp++/
mkdir /usr/include/libptp++/chdk/
cp chdk/* /usr/include/libptp++/chdk/
