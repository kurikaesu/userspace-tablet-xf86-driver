cmake_minimum_required(VERSION 3.21)
project(xf86_input_kuriuserspace C)

set(CMAKE_C_STANDARD 11)

add_library(xf86_input_kuriuserspace SHARED src/kuriuserspace.c)
