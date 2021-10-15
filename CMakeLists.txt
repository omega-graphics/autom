cmake_minimum_required(VERSION 3.10)

project(AUTOM C CXX)

set(CMAKE_C_STANDARD_REQUIRED TRUE)
set(CMAKE_C_STANDARD 11)

set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_STANDARD 17)

file(GLOB SRCS "./src/*.cpp" "./src/*.h" "./src/bridge/*.cpp" "./src/gen/*.cpp" "src/engine/*.cpp" "src/parser/*.h")

if(NOT WIN32)
    # Required on Linux because some standard libs are not compiled with -fPIC.
    add_library("autom-lib" SHARED ${SRCS})
    set_target_properties("autom-lib" PROPERTIES LIBRARY_OUTPUT_NAME "autom")
else()
    add_library("autom-lib" STATIC ${SRCS})
    set_target_properties("autom-lib" PROPERTIES ARCHIVE_OUTPUT_NAME "autom")
	if(NOT WIN32)
		set_target_properties("autom-lib" PROPERTIES PREFIX "lib")
	endif()
endif()


if(NOT APPLE AND UNIX)
    target_link_libraries("autom-lib" PRIVATE dl)
endif()

add_executable("autom" "src/driver/main.cpp")
set_target_properties("autom" PROPERTIES INSTALL_RPATH @loader_path/../lib)
target_link_libraries("autom" PRIVATE "autom-lib")

macro(add_autom_test _NAME)
    set(_SOURCES ${ARGN})
    add_executable(${_NAME} ${_SOURCES})
    target_link_libraries(${_NAME} PRIVATE "autom-lib")
    set_target_properties(${_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tests")
endmacro()

add_executable("autom-install" "src/tools/install-main.cpp")
set_target_properties("autom-install" PROPERTIES INSTALL_RPATH @loader_path/../lib)
target_link_libraries("autom-install" PRIVATE "autom-lib")

add_autom_test("lex-test" "src/tests/lex-test.cpp")
#add_autom_test("run-test" "src/tests/run-test.cpp")

include_directories("deps/rapidjson/include" "src")

set(STDLIB_MODULES)

macro(add_stdlib_module _NAME DIR)
    file(GLOB _SRCS "${DIR}/*.cpp" "${DIR}/*.c" "${DIR}/*.h")
    add_library(${_NAME} MODULE ${_SRCS})
    list(APPEND STDLIB_MODULES ${_NAME})
    set_target_properties(${_NAME} PROPERTIES PREFIX "" SUFFIX ".aext" RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/stdlib" LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/stdlib")
    target_link_libraries(${_NAME} PRIVATE "autom-lib")
endmacro()

add_stdlib_module("fs" "modules/fs")
add_stdlib_module("bridge" "modules/bridge")

file(GLOB STDLIB_MODS "${CMAKE_SOURCE_DIR}/modules/*.autom")
if(WIN32)
	file(GLOB PYTHON_TOOLS ${CMAKE_SOURCE_DIR}/tools/*.py ${CMAKE_SOURCE_DIR}/tools/*.bat)
else()
	file(GLOB PYTHON_TOOLS ${CMAKE_SOURCE_DIR}/tools/*.py ${CMAKE_SOURCE_DIR}/tools/autom-*)
endif()
set(DEFAULT_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/tools/default_toolchains.json")

install(FILES ${STDLIB_MODS} DESTINATION "modules")

install(FILES ${PYTHON_TOOLS} ${DEFAULT_TOOLCHAIN_FILE} DESTINATION "bin"  PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
                    GROUP_EXECUTE GROUP_READ)


install(TARGETS autom autom-install DESTINATION "bin")
install(TARGETS "autom-lib" ARCHIVE DESTINATION "lib")
install(TARGETS ${STDLIB_MODULES} RUNTIME DESTINATION "modules" LIBRARY DESTINATION "modules")