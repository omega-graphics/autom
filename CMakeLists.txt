cmake_minimum_required(VERSION 3.9)

project(autom CXX)

file(GLOB AUTOM_C_SRCS "${CMAKE_SOURCE_DIR}/cc/automc/*.cpp")
file(GLOB AUTOM_C_DEFS "${CMAKE_SOURCE_DIR}/cc/automc/*.def")
file(GLOB AUTOM_C_HEADERS "${CMAKE_SOURCE_DIR}/cc/automc/*.h")

add_library("automc" SHARED ${AUTOM_C_SRCS} ${AUTOM_C_DEFS} ${AUTOM_C_HEADERS})