#!/bin/sh

# This script is responsible for building the libptp++ shared library.

g++ -shared -fPIC libptp++.cpp -o libptp++.so -lusb-1.0

