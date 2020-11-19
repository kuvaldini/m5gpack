CMAKE_MINIMUM_REQUIRED(VERSION 3.10)

enable_testing()

execute_process(
    COMMAND sh -c "git -C ${CMAKE_CURRENT_LIST_DIR}/googletest rev-parse \
       || git clone https://github.com/google/googletest ${CMAKE_CURRENT_LIST_DIR}/googletest --single-branch --depth 1"
    RESULTS_VARIABLE STATUS
    ## TODO with CMake version 3.19 COMMAND_ERROR_IS_FATAL ANY
)
if(STATUS AND NOT ${STATUS} EQUAL 0)
    message(FATAL_ERROR "Failed to clone googletest")
endif()
add_subdirectory(googletest)

add_executable(gtest-m5gpack
    gtest-m5gpack.cpp
    )
target_compile_features(gtest-m5gpack PRIVATE cxx_std_17)
target_link_libraries(gtest-m5gpack PRIVATE gtest_main gtest m5gpack)
add_test(NAME gtest-m5gpack COMMAND gtest-m5gpack)
# target_include_directories(gtest-m5gpack PRIVATE ${CMAKE_BINARY_DIR}/..) ## For rev_label.hpp

##maybe todo for CTest
#include(GoogleTest)
#gtest_discover_tests(gtest-m5gpack)
