cmake_minimum_required(VERSION 3.1)
project(bent VERSION 0.2.0)

## test

enable_testing()

aux_source_directory(test TEST_LIST)
add_executable(bent_test ${TEST_LIST})
set_property(TARGET bent_test PROPERTY CXX_STANDARD 11)
set_property(TARGET bent_test PROPERTY CXX_STANDARD_REQUIRED ON)
target_include_directories(bent_test PRIVATE ./include)

add_test(test_all bent_test)

## install

install(DIRECTORY include/bent DESTINATION include)
