cmake_minimum_required (VERSION 2.6)
project (leptjson_test C)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -ansi -pedantic -Wall")

add_library(leptjson leptjson.c)
add_executable(leptjson_test test.c)
target_link_libraries(leptjson_test leptjson)
