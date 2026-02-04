include_directories(${Boost_INCLUDE_DIR})
include_directories(${PYTHONLIBS_INCLUDE_DIRS})

if(use_python2)
  set(python_lib_name "columbus")
else()
  set(python_lib_name "columbus.${pysoabi}")
endif()

add_library(columbus_ext SHARED columbus.cc)
target_link_libraries(columbus_ext ${COL_LIB_BASENAME} ${BOOST_PYTHON_HACK} ${PYTHONLIBS_LIBRARIES})
set_target_properties(columbus_ext PROPERTIES OUTPUT_NAME ${python_lib_name} PREFIX "")
set_target_properties(columbus_ext PROPERTIES COMPILE_FLAGS "-DBOOST_PYTHON_USE_GCC_SYMBOL_VISIBILITY=1")

add_pch(pch/colpython_pch.hh columbus_ext)

install(TARGETS columbus_ext DESTINATION ${PYTHONDIR})
