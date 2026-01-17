cmake_minimum_required(VERSION 3.20)
project(hoshi_lang)

set(CMAKE_CXX_STANDARD 23)

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")

message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})

add_definitions(${LLVM_DEFINITIONS})

message(STATUS "LLVM_INCLUDE_DIRS: ${LLVM_INCLUDE_DIRS}")

string(REPLACE "." ";" LLVM_PACKAGE_VERSION_LIST "${LLVM_PACKAGE_VERSION}")
list(GET LLVM_PACKAGE_VERSION_LIST 0 LLVM_MAJOR_VERSION)

find_program(LLVM_CONFIG_EXECUTABLE
             NAMES llvm-config llvm-config-${LLVM_MAJOR_VERSION}
             DOC "Path to the llvm-config executable"
             REQUIRED # Make this step fail if neither is found
)
message(STATUS "Using llvm-config executable: ${LLVM_CONFIG_EXECUTABLE}")

set(ADDITIONAL_LLVM_CONFIG_FLAGS --link-static)

# 4. Execute process using the detected LLVM_CONFIG_EXECUTABLE
execute_process(COMMAND ${LLVM_CONFIG_EXECUTABLE} --libs core support native linker lto ${ADDITIONAL_LLVM_CONFIG_FLAGS}
                OUTPUT_VARIABLE llvm_components
                OUTPUT_STRIP_TRAILING_WHITESPACE
                RESULT_VARIABLE LLVM_LIBS_RESULT
)

execute_process(COMMAND ${LLVM_CONFIG_EXECUTABLE} --system-libs core support native linker lto ${ADDITIONAL_LLVM_CONFIG_FLAGS}
                OUTPUT_VARIABLE llvm_system_libs
                OUTPUT_STRIP_TRAILING_WHITESPACE
)

string(REPLACE " " ";" llvm_components_list "${llvm_components}")
string(REPLACE " " ";" llvm_system_libs_list "${llvm_system_libs}")

execute_process(COMMAND ${LLVM_CONFIG_EXECUTABLE} --libdir
                OUTPUT_VARIABLE llvm_libdir
                OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(COMMAND ${LLVM_CONFIG_EXECUTABLE} --ldflags ${ADDITIONAL_LLVM_CONFIG_FLAGS}
                OUTPUT_VARIABLE LLVM_LDFLAGS
                OUTPUT_STRIP_TRAILING_WHITESPACE
)

string(REPLACE "\n" " " LLVM_LDFLAGS "${LLVM_LDFLAGS}")
string(REPLACE " " ";" LLVM_LDFLAGS_LIST "${LLVM_LDFLAGS}")

# print LLVM components
message(STATUS "LLVM libdir: ${llvm_libdir}")
message(STATUS "LLVM components: ${llvm_components}")
message(STATUS "LLVM system libs: ${llvm_system_libs}")

set(CMAKE_CXX_FLAGS_RELEASE  "-O3")
set(CMAKE_CXX_FLAGS_DEBUG    "-g -O0")

include_directories(${PROJECT_SOURCE_DIR})

file (GLOB HOSHI_LANG_INCLUDE
        compiler/*.cpp
        compiler/frontend/*.cpp
        compiler/visitor/*.cpp
        compiler/ir/*.cpp
        compiler/llvmCodegen/*.cpp
        compiler/objectLinker/*.cpp
        share/*.cpp
        pass/*.cpp)

file (GLOB HOSHI_LANG_RUNTIME_INCLUDE
        runtime/*.cpp
        runtime/memory/*.cpp
        runtime/string/*.cpp
        runtime/debug/*.cpp
        runtime/threading/*.cpp
        runtime/rtti/*.cpp
        runtime/random/*.cpp
        runtime/signal/*.cpp
        runtime/time/*.cpp
        runtime/fs/*.cpp
        runtime/hperf/*.cpp
)

add_executable(hoshi_lang main.cpp ${HOSHI_LANG_INCLUDE})

add_library(elysia_runtime STATIC runtime/runtime.cpp ${HOSHI_LANG_RUNTIME_INCLUDE})

# add -fno-builtin to avoid linking with system libraries for elysia_runtime
set_target_properties(elysia_runtime PROPERTIES COMPILE_FLAGS "-fno-builtin -static")

target_link_options(hoshi_lang PRIVATE ${LLVM_LDFLAGS_LIST})

target_link_libraries(elysia_runtime ${CMAKE_DL_LIBS} pthread)

target_link_libraries(hoshi_lang PRIVATE ${llvm_components_list})

if (WIN32)
    
else()
    target_link_libraries(hoshi_lang PRIVATE ${llvm_system_libs_list})
endif()

target_link_directories(hoshi_lang PRIVATE ${llvm_libdir})

if (WIN32)
    set(WINDOWS_SYSTEM_LIBS ntdll.lib shell32.lib user32.lib advapi32.lib)
    target_link_libraries(hoshi_lang PRIVATE ${WINDOWS_SYSTEM_LIBS} ${llvm_components_list})
else()
    target_link_libraries(hoshi_lang PRIVATE ${llvm_components_list})
endif()

# if build in release mode, add CXX marco definition ELYSIA_RUNTIME_BUILD_TYPE_RELEASE
if (CMAKE_BUILD_TYPE STREQUAL "RELEASE" OR CMAKE_BUILD_TYPE STREQUAL "Release")
    message(STATUS "Build in Release mode")
    add_definitions(-DELYSIA_RUNTIME_BUILD_TYPE_RELEASE)
endif()

# If CI presents, set ELYSIA_RUNTIME_DISABLE_BUILTIN_MEMORY_LEAK_DETECTOR
if("$ENV{CI}")
    set(IS_CI TRUE)
    message(STATUS "CI detected, disable memory leak detector")
    add_definitions(-DELYSIA_RUNTIME_DISABLE_BUILTIN_MEMORY_LEAK_DETECTOR)
endif()

set_target_properties(elysia_runtime PROPERTIES POSITION_INDEPENDENT_CODE ON)
