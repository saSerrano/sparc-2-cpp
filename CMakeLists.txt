cmake_minimum_required(VERSION 2.8.12.2)
project(sparc2cpp)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "-std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "-std=c++0x")
else()
    message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

#Directory where the header files are
include_directories(${CMAKE_SOURCE_DIR}/include)

#List of source files
set(SOURCE
${CMAKE_SOURCE_DIR}/source/sparc2cpp.cpp
${CMAKE_SOURCE_DIR}/source/DomainSort.cpp
${CMAKE_SOURCE_DIR}/source/PState.cpp
${CMAKE_SOURCE_DIR}/source/Transition.cpp
${CMAKE_SOURCE_DIR}/source/Action.cpp
)

#Directory where executables are to be found
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

#Example: Get sorts of a description
add_executable(get-sorts ${CMAKE_SOURCE_DIR}/examples/get-sorts.cpp ${SOURCE})
