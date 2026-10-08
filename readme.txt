Columbus - a library for fast error-tolerant searching

(C) 2012 Canonical ltd


Compiling

Columbus uses CMake. It also enforces separate build and source directories.
To compile it, simply extract the source, cd into it and run the following
commands:

mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=debug ..
make

Change "debug" to "release" or "relwithdebinfo" depending on your needs.


Testing

Columbus comes with a test suite. It is not enabled by default. To enable it,
simply run 'ccmake .' in your build directory and enable tests in the GUI.

There are additional scalability tests that need to be enabled separately,
since they take quite a lot of time to run. They are enabled in the same way
as regular tests.
