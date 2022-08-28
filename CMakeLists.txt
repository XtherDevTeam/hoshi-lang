cmake_minimum_required(VERSION 3.20)
project(hoshi_lang)

set(CMAKE_CXX_STANDARD 20)

#set(CMAKE_BUILD_TYPE "Release")
set(CMAKE_CXX_FLAGS_RELEASE  "-O3")

include_directories(ThirdPartyLibraries Share Frontend Backend)

file (GLOB HOSHI_LANG_INCLUDE
        Frontend/*.cpp
        Frontend/Parsers/*.cpp
        Share/*.cpp
        Share/Exceptions/*.cpp
        Backend/*.cpp
        Backend/IR/*.cpp
        Backend/Codegen/*.cpp
        Backend/Optimizer/*.cpp)

add_executable(hoshi_lang main.cpp ${HOSHI_LANG_INCLUDE})
