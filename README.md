[![pipeline status](https://gitlab.com/kyb/m5gpack/badges/master/pipeline.svg)](https://gitlab.com/kyb/m5gpack/pipelines?scope=branches) 
[![releases](https://img.shields.io/badge/m5gpack-releases-green.svg?style=flat)](https://gitlab.com/kyb/m5gpack/-/releases) 
[![Gitpod.ready-to-code](https://img.shields.io/badge/Gitpod-ready--to--code-blue?logo=gitpod)](https://gitpod.io/#https://gitlab.com/kyb/m5gpack) 

MessagePack for C++
===================
[![GitLab|origin](https://img.shields.io/badge/GitLab-origin-darkorange?logo=gitlab)](https://gitlab.com/kyb/m5gpack) 
[![GitHub|mirror](https://img.shields.io/badge/GitHub-mirror-blue?logo=github)](https://github.com/kuvaldini/m5gpack)

According to [MessagePack specification](https://github.com/msgpack/msgpack/blob/master/spec.md)

**m5gpack** is a family of C++ types and functions to provide next functionality:
- serialize C++ data structures to MessagePack sequence of bytes
- deserialize series of bytes to C++ data tuples

A list of types:
```cpp
namespace m5g {
   using value = ...;  /// The MessagePack value can contain 
                       /// nil, boolean, integer, float-point, 
                       /// blob, array, map and timestamp
   using nil_t = ...;
   constexpr auto nil = nil_t{};
   using bin   = ...;  /// Growable array of bytes (std::vector<uint8_t>)
   using array = ...;  /// Growable array of m5g::value`s (std::vector<m5g::value>)
   using arr   = array;
   using map   = ...;  /// Growable assosiative array of m5g::value`s
   using ext   = ...;  /// see MessagePack specification
   using timestamp = ...;   /// Timestamp representation seconds+nanoseconds
   using stream = ...; /// Stream of bytes. To be stored or transferred.
                       /// m5g::value could be serialized to 
                       /// and deserialized from m5g::stream 
                       /// using operators << and >>
   using byte_stream = stream;
}
```
`m5g::value` is extended `std::variant<nil_t,bin,arr,map,ext,timestamp>`. 
`m5g::value` is constructable from any of MessagePack types.
Everything applicable to `std::variant<>` is also applicable to `m5g::value`, i.e `std::visit()`.
`m5g::value` could be serialized to `m5g::stream` and deserialized from it. 
`m5g::stream` in its turn is an extension of `std::vector<uint8_t>`, all 
functions applicable to `std::vector<uint8_t>` are also applicable to `m5g::stream`.



## Example of operation
Lets consider example program `m5gpack-demo.cpp`
```cpp
ToDo add demo program source
```

This is an output from `./m5gpack-demo`
```
ToDo add output
```

Build and run it with:
```
cmake -Bbuild  &&  cmake --build build  &&  ./build/m5gpack-demo 
```


Integrate to user project **[TODO validate and fix]**
-----
### 1. The most straightforward way is to include this project's directory.
I suggest to use [git subrepo](https://github.com/ingydotnet/git-subrepo)
instead of git subtree and git submodule.
```
git subrepo clone git@gitlab.com:kyb/m5gpack.git m5gpack
```
Then add m5gpack project directory to includes.

### 2. Using the only header file – may be the most convenient way
Download amalgama header from the project's [releases page](https://gitlab.com/kyb/m5gpack/-/releases)
```
cd your/project/include_dir
curl -fsSL https://gitlab.com/kyb/m5gpack/raw/artifacts/master/$(curl https://gitlab.com/kyb/m5gpack/raw/artifacts/master/m5gpack.hh -fsSL) -o m5gpack.hh
```

### 3. Using CMake `target_link_libraries` with interface library
*Assuming the repo has been already cloned as sugested in (1)*  
Since `m5gpack` this is header-only library there is no need to link with it.
But CMake has feature INTERFACE LIBRARIES. m5gpack provides two interface 
libraries: `m5gpack` and `m5gpack_amalgama`. Linking against them addes 
include directory to the target.
1. `m5gpack`
    ```
    target_link_libraries(your_executable PRIVATE m5gpack)
    ```
    ```
    #include "m5gpack.hpp"
    #include "byte_range.hpp"
    #include "byte_range_ascii.hpp"
    #include "byte_range_hex.hpp"
    ```
2. `m5gpack_amalgama`
    ```
    target_link_libraries(your_executable PRIVATE m5gpack_amalgama)
    ```
    ```
    #include "m5gpack.hh"
    ```
