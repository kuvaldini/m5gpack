cmake_minimum_required(VERSION 3.16)

project(m5gpack LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

################## byte_vector ##################
### Interface library user project to be linked with
### to include byte_vector.hpp and other headers
add_library(m5gpack INTERFACE)
target_include_directories(m5gpack INTERFACE include)
##################

# ################## byte_vector_amalgamate ##################
# ### Squash all source headers in one file byte_vector.hh
# add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/byte_vector.hh
#    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
#    COMMAND ${CMAKE_CURRENT_LIST_DIR}/amalgamate.sh ${CMAKE_CURRENT_BINARY_DIR}/byte_vector.hh
#    DEPENDS
#       byte_vector.hpp
#       byte_range.hpp
#       byte_range_ascii.hpp
#       byte_range_hex.hpp
# )
# add_custom_target(byte_vector_amalgamate ALL SOURCES ${CMAKE_CURRENT_BINARY_DIR}/byte_vector.hh)
# ##################

# ################## byte_vector_amalgama ##################
# ### Interface library user project to be linked with to include the only header byte_vector.hh
# add_library(byte_vector_amalgama INTERFACE)
# target_include_directories(byte_vector_amalgama INTERFACE ${CMAKE_CURRENT_BINARY_DIR})
# add_dependencies(byte_vector_amalgama byte_vector_amalgamate)
# ##################

# ################## byte_vector-demo ##################
# ### Demonstration executable
# add_executable(byte_vector-demo byte_vector-demo.cpp)
# target_compile_features(byte_vector-demo PRIVATE cxx_std_17)
# target_link_libraries(byte_vector-demo PRIVATE byte_vector_amalgama)
# ##################

# ################## Tests ##################
include(CTest)
if(BUILD_TESTING) ## By default CMake provides ON
   add_subdirectory(tests)
endif()
# ##################


add_executable(main main.cpp)
target_include_directories(main PRIVATE include .)
target_compile_features(main PRIVATE cxx_std_20)


# add_executable(temp-test temp-test.cpp)
# target_include_directories(temp-test PRIVATE include .)
# target_compile_features(temp-test PRIVATE cxx_std_20)
# # target_compile_options(temp-test PRIVATE -fconcepts-diagnostics-depth=4)
