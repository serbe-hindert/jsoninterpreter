cmake_minimum_required(VERSION 3.27)
project(jsoninterpreter C)

set(CMAKE_C_STANDARD 23)

add_library(jsoninterpreter STATIC jsoninterpreter.c)
