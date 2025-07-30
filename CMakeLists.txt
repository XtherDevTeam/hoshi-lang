cmake_minimum_required(VERSION 3.20)
project(hoshi_lang)

set(CMAKE_CXX_STANDARD 26)

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")

message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})

add_definitions(${LLVM_DEFINITIONS})

message(STATUS "LLVM_INCLUDE_DIRS: ${LLVM_INCLUDE_DIRS}")

execute_process(COMMAND llvm-config --libs all
                OUTPUT_VARIABLE llvm_libraries
                OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(COMMAND llvm-config --libdir
                OUTPUT_VARIABLE llvm_libdir
                OUTPUT_STRIP_TRAILING_WHITESPACE
)

# print LLVM components
message(STATUS "LLVM components: ${llvm_libraries}")

#set(CMAKE_BUILD_TYPE "Release")
set(CMAKE_CXX_FLAGS_RELEASE  "-O3")

#include_directories(/Library/Developer/CommandLineTools/SDKs/MacOSX15.0.sdk/usr/include/c++/v1/)

include_directories(${PROJECT_SOURCE_DIR})

file (GLOB HOSHI_LANG_INCLUDE
        compiler/*.cpp
        compiler/frontend/*.cpp
        compiler/visitor/*.cpp
        compiler/ir/*.cpp
        compiler/llvmCodegen/*.cpp
        share/*.cpp
        pass/*.cpp)

file (GLOB HOSHI_LANG_RUNTIME_INCLUDE
        runtime/*.cpp
        runtime/memory/*.cpp
        runtime/debug/*.cpp
)

add_executable(hoshi_lang main.cpp ${HOSHI_LANG_INCLUDE})

add_library(elysia_runtime STATIC runtime/runtime.cpp ${HOSHI_LANG_RUNTIME_INCLUDE})

target_link_libraries(elysia_runtime ${CMAKE_DL_LIBS} pthread)

target_link_directories(hoshi_lang PRIVATE ${llvm_libdir})

target_link_libraries(hoshi_lang ${llvm_libraries})
