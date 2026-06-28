cmake_minimum_required(VERSION 3.16)

project(rg_etc1 CXX)

add_library(${PROJECT_NAME} rg-etc1-true/rg_etc1.h rg-etc1-true/rg_etc1.cpp rg_etc1_c.cpp)
