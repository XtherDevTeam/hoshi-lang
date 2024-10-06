cmake_minimum_required(VERSION 3.20)
project(hoshi_lang)

set(CMAKE_CXX_STANDARD 20)

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")

message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})

add_definitions(${LLVM_DEFINITIONS})

llvm_map_components_to_libnames(llvm_libs support core irreader)

#set(CMAKE_BUILD_TYPE "Release")
set(CMAKE_CXX_FLAGS_RELEASE  "-O3")

#include_directories(/Library/Developer/CommandLineTools/SDKs/MacOSX15.0.sdk/usr/include/c++/v1/)

include_directories(${PROJECT_SOURCE_DIR})

file (GLOB HOSHI_LANG_INCLUDE
        compiler/*.cpp
        compiler/frontend/*.cpp
        compiler/visitor/*.cpp
        compiler/ir/*.cpp
        runtime/*.cpp
        share/*.cpp
        pass/*.cpp)

add_executable(hoshi_lang main.cpp ${HOSHI_LANG_INCLUDE})

target_link_libraries(hoshi_lang ${llvm_libs})