cmake_minimum_required(VERSION 3.10)

project(AUTOM CXX)

set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_STANDARD 17)

file(GLOB SRCS "./src/*.cpp" "./src/*.h" "./src/bridge/*.cpp" "./src/gen/*.cpp" "src/parser/*.cpp" "src/parser/*.h")

add_executable("autom" "src/driver/main.cpp" ${SRCS})