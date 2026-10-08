project(columbus C CXX)
cmake_minimum_required(VERSION 2.8.9)

if(PROJECT_BINARY_DIR STREQUAL PROJECT_SOURCE_DIR)
   message(FATAL_ERROR "In-tree build attempt detected, aborting. Set your build dir outside your source dir, delete CMakeCache.txt from source root and try again.")
endif()

option(enable_tests "Enable tests." ON)
option(enable_scalability_tests "Additional scalability tests that are potentially very slow to run." OFF)
option(full_warnings "All possible compiler warnings." OFF)
option(debug_messages "Print debug messages.")
option(full_unicode "Enable full Unicode support (takes lots of memory).")
option(use_python2 "Build Python bindings against Python 2 (UNSUPPORTED)." OFF)

if(use_python2)
  message(WARNING "Python 2 bindings are NOT SUPPORTED! If they break, you get to keep both pieces.")
endif()

include(FindPkgConfig)
include(cmake/pch.cmake)
include(cmake/python.cmake)
include(cmake/coverage.cmake)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -pedantic -Wextra")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -Wextra")
if(${full_warnings})
# C does not have any more warning flags.
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Weffc++")
endif()

if(${debug_messages})
  add_definitions(-DDEBUG_MESSAGES)
endif()

# Symbol visibility
add_definitions(-DBUILDING_COLUMBUS)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden -fvisibility-inlines-hidden")

set(SO_VERSION_MAJOR "1")
set(SO_VERSION_MINOR "1")
set(SO_VERSION_PATCH "0")

set(SO_VERSION "${SO_VERSION_MAJOR}.${SO_VERSION_MINOR}.${SO_VERSION_PATCH}")

set(COL_LIB_BASENAME "columbus")

# Increment this manually whenever breaking ABI.
# http://tldp.org/HOWTO/Program-Library-HOWTO/shared-libraries.html#AEN135
set(ABI_VERSION 1)

include(GNUInstallDirs)
set(LIBDIR ${CMAKE_INSTALL_LIBDIR})
# Set as cache variable so packaging can override.
set(PYTHONDIR "lib/python3/dist-packages" CACHE PATH "Destination install dir for Python module")

include(TestBigEndian)
TEST_BIG_ENDIAN(IS_BIG_ENDIAN)

if(full_unicode)
  if(${IS_BIG_ENDIAN})
    set(INTERNAL_ENCODING "UTF-32BE") # Iconv encoding string.
  else()
    set(INTERNAL_ENCODING "UTF-32LE")
  endif()
  set(LETTER_TYPE "uint32_t")
else()
  if(${IS_BIG_ENDIAN})
    set(INTERNAL_ENCODING "UCS-2BE//IGNORE") # Drop everything outside Base Multilingual Plane.
  else()
    set(INTERNAL_ENCODING "UCS-2LE//IGNORE")
  endif()
  set(LETTER_TYPE "uint16_t")
endif()  

find_file(HAS_SPARSE_HASH "google/sparse_hash_map")
if(HAS_SPARSE_HASH)
  message(STATUS "Using sparse hash.")
else()
  message(STATUS "Sparse hash not found, using regular hash.")
endif()

# Only required on some platforms.
# If not found, that's ok.
find_library(ICONV_LIBRARIES iconv)

pkg_search_module(GTK3 gtk+-3.0)

if(GTK3_FOUND)
  message(STATUS "GTK+ 3.0 found, building GUI apps.")
else()
  message(STATUS "GTK+ 3.0 not found, not building GUI apps.")
endif()

include_directories(include)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/include)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

pkg_search_module(ICU icu-uc)

# Quantal and earlier do not have pkg-config files for icu.
# Find it manually.
if(NOT ICU_FOUND)
  find_library(icu1 icuuc)
  find_library(icu2 icudata)
  if(NOT icu1)
    message(FATAL_ERROR "Libicu not found. Please install it and rerun CMake.")
  endif()
  if(NOT icu2)
    message(FATAL_ERROR "Libicu not found. Please install it and rerun CMake.")
  endif()
  set(ICU_LIBRARIES ${icu1} ${icu2})
endif()

add_subdirectory(include)
add_subdirectory(src)
add_subdirectory(tools)
add_subdirectory(share)

if(build_python)
  add_subdirectory(python)
endif()

if(${enable_tests})
  enable_testing()
  add_subdirectory(test)
endif()
