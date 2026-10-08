configure_file(ColumbusCore.hh.in ColumbusCore.hh)
install(FILES
${CMAKE_CURRENT_BINARY_DIR}/ColumbusCore.hh
Matcher.hh
MatchResults.hh
columbus.h
columbus.hh
Word.hh
WordList.hh
Corpus.hh
ErrorValues.hh
Document.hh
ColumbusHelpers.hh
IndexWeights.hh
DESTINATION include/${COL_LIB_BASENAME}${SO_VERSION_MAJOR})

# Build and install a pkg-config file
set(prefix ${CMAKE_INSTALL_PREFIX})
set(exec_prefix ${prefix}/bin)
set(libdir ${prefix}/${LIBDIR})
set(includedir ${prefix}/include/${COL_LIB_BASENAME}${SO_VERSION_MAJOR})
set(pkg-name "lib${COL_LIB_BASENAME}")
configure_file(libcolumbus.pc.in libcolumbus.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libcolumbus.pc DESTINATION ${LIBDIR}/pkgconfig)
