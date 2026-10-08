set(build_python FALSE)

# CMake's Boost.Python detector is completely and utterly
# broken. We have to do this manually.
#
# Upstream bug:
# http://public.kitware.com/Bug/view.php?id=12955
find_file(BP_HEADER boost/python.hpp)

if(use_python2)
  pkg_search_module(PYTHONLIBS python)
else()
  pkg_search_module(PYTHONLIBS python3)
endif()

if(NOT BP_HEADER)
  message(STATUS "Boost.Python not found, not building Python bindings.")
else()
  if(NOT PYTHONLIBS_FOUND)
    message(STATUS "Python dev libraries not found, not building Python bindings.")
  else()
    string(SUBSTRING ${PYTHONLIBS_VERSION} 0 1 PYTHON_MAJOR)
    string(SUBSTRING ${PYTHONLIBS_VERSION} 2 1 PYTHON_MINOR)
    message(STATUS "Found Python version ${PYTHON_MAJOR}.${PYTHON_MINOR}.")
    if(NOT use_python2)
      execute_process(COMMAND ${CMAKE_SOURCE_DIR}/cmake/pysoabi.py OUTPUT_VARIABLE pysoabi OUTPUT_STRIP_TRAILING_WHITESPACE)
    endif()
    find_library(BOOST_PYTHON_HACK boost_python-py${PYTHON_MAJOR}${PYTHON_MINOR})

    if(NOT BOOST_PYTHON_HACK)
      message(STATUS "Boost.Python hack library not found, not building Python bindings")
    else()
      set(build_python TRUE)
      message(STATUS "Building Python bindings.")
    endif()
  endif()
endif()
