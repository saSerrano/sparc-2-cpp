cmake_minimum_required(VERSION 3.9)
project(sparc2cpp)

##############################
## Compiler/Linker Settings ##
##############################

# Set default cmake build type to release
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING
      "Choose the type of build, options are:
          Debug Release RelWithDebInfo MinSizeRel."
      FORCE)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

##############################
##      Project Start       ##
##############################

#SPARC-2-CPP header files directory
include_directories(${CMAKE_SOURCE_DIR}/include)

#List of SPARC-2-CPP source files
set(SOURCE
${CMAKE_SOURCE_DIR}/source/sparc2cpp.cpp
${CMAKE_SOURCE_DIR}/source/DomainSort.cpp
${CMAKE_SOURCE_DIR}/source/PState.cpp
${CMAKE_SOURCE_DIR}/source/Transition.cpp
${CMAKE_SOURCE_DIR}/source/Action.cpp
${CMAKE_SOURCE_DIR}/source/Pomdp.cpp
)

#Directory where executables are to be found
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

#Example: Get sorts of a description
add_executable(get-sorts ${CMAKE_SOURCE_DIR}/examples/get-sorts.cpp ${SOURCE})




