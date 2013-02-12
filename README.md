libptp++
========

A library to communicate with cameras running CHDK in C++

The API is based on the PyPTP2 project, which provides a nice wrapper around
PyUSB to facilitate easy communication with CHDK cameras.  Similarly, this 
library will wrap around libusb(x) to provide easy communication with CHDK
cameras.

This library aims to be speedy and simple to use.  For developers, this library
aims to be unintrusive and let them simply interact with the camera without 
worrying about how PTP commands work, or what commands they are able to send.
The classes provided by this camera should be simple to use, and hide any 
functions that aren't needed by outside programs.  Additionally, this library 
should ultimately hide from developers its dependency on libusb.
