cmake_minimum_required(VERSION 3.10)

project(AUTOM CXX)

set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_STANDARD 17)

file(GLOB SRCS "./src/*.cpp" "./src/*.h" "./src/bridge/*.cpp" "./src/gen/*.cpp" "src/engine/*.cpp" "src/parser/*.h")

if(NOT APPLE AND UNIX)
    # Required, because some standard libs are not compiled with -fPIC.
    add_library("autom-lib" SHARED ${SRCS})
    set_target_properties("autom-lib" PROPERTIES LIBRARY_OUTPUT_NAME "autom")
else()
    add_library("autom-lib" STATIC ${SRCS})
    set_target_properties("autom-lib" PROPERTIES ARCHIVE_OUTPUT_NAME "autom")
endif()

# If using LLVM on Linux, prefer linking to libc++
#if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
#    if(NOT APPLE AND UNIX)
#        set(CMAKE_CXX_FLAGS "-stdlib=libc++")
#    endif()
#endif()

if(NOT APPLE AND UNIX)
    target_link_libraries("autom-lib" PRIVATE dl)
endif()

add_executable("autom" "src/driver/main.cpp")
target_link_libraries("autom" PRIVATE "autom-lib")

macro(add_autom_test _NAME)
    set(_SOURCES ${ARGN})
    add_executable(${_NAME} ${_SOURCES})
    target_link_libraries(${_NAME} PRIVATE "autom-lib")
    set_target_properties(${_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tests")
endmacro()

add_autom_test("lex-test" "src/tests/lex-test.cpp")
add_autom_test("run-test" "src/tests/run-test.cpp")

include_directories("deps/rapidjson/include" "src")

set(STDLIB_MODULES)

macro(add_stdlib_module _NAME DIR)
    file(GLOB _SRCS "${DIR}/*.cpp" "${DIR}/*.cc" "${DIR}/*.h")
    add_library(${_NAME} SHARED ${_SRCS})
    list(APPEND STDLIB_MODULES ${_NAME})
    set_target_properties(${_NAME} PROPERTIES PREFIX "" SUFFIX ".aext" RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/stdlib" LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/stdlib")
    target_link_libraries(${_NAME} PRIVATE "autom-lib")
endmacro()

add_stdlib_module("fs" "stdlib/fs")
add_stdlib_module("bridge" "stdlib/bridge")

file(GLOB STDLIB_MODS "${CMAKE_SOURCE_DIR}/stlib/*.autom")

install(FILES ${STDLIB_MODS} DESTINATION "mods")

install(TARGETS "autom" DESTINATION "bin")
install(TARGETS "autom-lib" ARCHIVE DESTINATION "lib")
install(TARGETS ${STDLIB_MODULES} RUNTIME DESTINATION "stdlib" LIBRARY DESTINATION "stdlib")