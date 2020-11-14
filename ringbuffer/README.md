[![Build Status](https://travis-ci.org/sledgeh/RingBuffer.svg?branch=master)](https://travis-ci.org/sledgeh/RingBuffer) 
[![build status](https://gitlab.com/kyb/RingBuffer/badges/master/build.svg)](https://gitlab.com/kyb/RingBuffer/commits/master)
[![coverage report](https://gitlab.com/kyb/RingBuffer/badges/master/coverage.svg)](https://gitlab.com/kyb/RingBuffer/commits/master)


RingBuffer
==========
This is a header-only implementation of ring (or circular) buffer in C++11. It depends only on few stdlib definitions.
Read more about [Circular buffer](https://en.wikipedia.org/wiki/Circular_buffer) on wikipedia.

See usage examples in folder [`test`](test/)


To do
-----
- Better compability with `std::` containers
- Iterators
- Memory reallocation
- Some functional extensions


Build and run tests
-------------
```
$ cd path/to/workspace
$ git clone https://gitlab.com/kyb/RingBuffer.git --recurce-submodules
$ cd ./RingBuffer
$ mkdir ./build  &&  cd ./build
$ cmake -G"MinGW Makefiles" ../
$ cmake --build ./
$ cd ./test  &&  ctest
```
"MinGW Makefiles" can be "Unix Makefiles", "Ninja", or any supported by CMake.
