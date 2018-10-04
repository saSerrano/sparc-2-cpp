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
# AI-Toolbox settings & deps #
##############################

# For additional Find library scripts
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/Modules/")

set(BOOST_VERSION_REQUIRED 1.54)
set(EIGEN_VERSION_REQUIRED 3.2.92)

find_package(Boost ${BOOST_VERSION_REQUIRED} REQUIRED)
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

find_package(Eigen3 ${EIGEN_VERSION_REQUIRED} REQUIRED)
include_directories(SYSTEM ${EIGEN3_INCLUDE_DIR})

find_package(LpSolve REQUIRED)
include_directories(SYSTEM ${LPSOLVE_INCLUDE_DIR})

##############################
##      Project Start       ##
##############################

#SPARC-2-CPP header files directory
include_directories(${CMAKE_SOURCE_DIR}/include)

#AI-Toolbox header files directory
include_directories("/home/zero/Documents/AI-Toolbox/include")

#Directory where the AI-Toolbox libraries are
link_directories("/home/zero/Documents/AI-Toolbox/build")

#List of SPARC-2-CPP source files
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

#Example: Use SPARC-2-CPP and AI-Toolbox objects
add_executable(s2c-ait ${CMAKE_SOURCE_DIR}/examples/s2c-ait.cpp ${SOURCE})
target_link_libraries(s2c-ait AIToolboxMDP AIToolboxPOMDP)



