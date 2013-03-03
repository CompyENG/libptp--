#!/bin/sh

# This script is responsible for building the libptp++ shared library.

g++ -shared -fPIC CameraBase.cpp CHDKCamera.cpp LVData.cpp PTPCamera.cpp PTPContainer.cpp -o libptp++.so -lusb-1.0

