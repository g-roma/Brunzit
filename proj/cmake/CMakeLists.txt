cmake_minimum_required( VERSION 3.10 FATAL_ERROR )
set( CMAKE_VERBOSE_MAKEFILE ON )

project( brunzit )

include(FetchContent)

set(FLUID_BRANCH "origin/main" CACHE STRING "Branch to pull flucoma dependencies from")
set(FLUID_PATH "" CACHE PATH "Optional path to the Fluid Decomposition repo")
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

FetchContent_Declare(
  HISSTools
  GIT_REPOSITORY https://github.com/AlexHarker/HISSTools_Library
  GIT_PROGRESS TRUE
  GIT_TAG f3292ad 
)


FetchContent_Declare(
  Eigen
  GIT_SHALLOW TRUE
  GIT_REPOSITORY https://gitlab.com/libeigen/eigen.git
  GIT_PROGRESS TRUE
  GIT_BRANCH "3.4"
  GIT_TAG "3.4.0"
  #https://stackoverflow.com/questions/77210209/how-to-prevent-eigen-targets-to-show-up-in-the-main-app-in-a-cmake-project
  SOURCE_SUBDIR cmake 

)

FetchContent_Declare(
  Spectra
  GIT_SHALLOW TRUE
  GIT_REPOSITORY https://github.com/yixuan/spectra
  GIT_PROGRESS TRUE
  GIT_BRANCH "master"
  GIT_TAG "v1.0.1"
)

FetchContent_Declare(
  Memory
  GIT_SHALLOW TRUE  
  GIT_REPOSITORY https://github.com/foonathan/memory.git
  GIT_PROGRESS TRUE
  GIT_TAG main
)


FetchContent_Declare(
   flucoma-core
   GIT_REPOSITORY https://github.com/flucoma/flucoma-core.git
   GIT_PROGRESS TRUE
   GIT_TAG ${FLUID_BRANCH}
)


FetchContent_MakeAvailable(HISSTools)
FetchContent_MakeAvailable(Eigen)
FetchContent_MakeAvailable(Memory)
FetchContent_MakeAvailable(Spectra)
FetchContent_MakeAvailable(flucoma-core)

include(flucoma_version)
include(flucoma-buildtools)
include(flucoma-buildtype)

# change to Cinder source directory
get_filename_component( CINDER_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../../../Cinder/" ABSOLUTE )
get_filename_component( APP_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../" ABSOLUTE )

include( "${CINDER_PATH}/proj/cmake/modules/cinderMakeApp.cmake" )

set( SRC_FILES
	${APP_PATH}/src/Brunzit.cpp
  ${APP_PATH}/src/Actions.hpp
  ${APP_PATH}/src/Agent.hpp
  ${APP_PATH}/src/Corpus.hpp
  ${APP_PATH}/src/Extractor.hpp
  ${APP_PATH}/src/Flock.hpp
  ${APP_PATH}/src/Parser.hpp
  ${APP_PATH}/src/Runtime.hpp
  ${APP_PATH}/src/Sound.hpp
  ${APP_PATH}/src/Synth.hpp
)

if (APPLE)
  set(CMAKE_XCODE_GENERATE_SCHEME ON)
  set(CMAKE_OSX_DEPLOYMENT_TARGET "13.8")
  message("apple")
endif()


ci_make_app(
	APP_NAME    Brunzit
	SOURCES     ${SRC_FILES}
	INCLUDES    
        ${APP_PATH}/include
	${hisstools_SOURCE_DIR}/include
	${eigen_SOURCE_DIR}
	${memory_SOURCE_DIR}/include/foonathan
	${memory_BINARY_DIR}/src
	${spectra_SOURCE_DIR}/include
	${flucoma-core_SOURCE_DIR}/include/flucoma
	CINDER_PATH ${CINDER_PATH}
	LIBRARIES 	foonathan_memory
        ASSETS_PATH ${APP_PATH}/assets
)


