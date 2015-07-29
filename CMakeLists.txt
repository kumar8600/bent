cmake_minimum_required(VERSION 3.1)
project(bent VERSION 0.1.0)

include_directories(include)

## lib

aux_source_directory(lib LIB_LIST)
add_library(bent ${LIB_LIST})
set_property(TARGET bent PROPERTY CXX_STANDARD 14)
set_property(TARGET bent PROPERTY CXX_STANDARD_REQUIRED ON)
target_include_directories(bent PUBLIC ./include)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(bent Threads::Threads)

## test

enable_testing()

aux_source_directory(test TEST_LIST)
add_executable(bent_test ${TEST_LIST})
set_property(TARGET bent_test PROPERTY CXX_STANDARD 14)
set_property(TARGET bent_test PROPERTY CXX_STANDARD_REQUIRED ON)
target_link_libraries(bent_test bent)
target_include_directories(bent_test PRIVATE ./include)

add_test(test_all bent_test)

