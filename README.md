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
The classes provided by this library should be simple to use, and hide any 
functions that aren't needed by outside programs.  Additionally, this library 
should ultimately hide from developers its dependency on libusb.

How to build
============

Unfortunately, I've never done anything with Makefiles and the like before,
so the build process is quite primitive.  To build, simply run build.sh and 
you'll notice the built library -- libptp++.so.

For convenience, I've also added an install.sh which will copy the library to
/usr/lib, and all headers to /usr/include/libptp++/ .  In your project:
`#include <libptp++/libptp++.hpp>` to use the library.

License
=======

Although not noted anywhere in the source, I am releasing this library under
the LGPLv3 license.