add_library(${COL_LIB_BASENAME} SHARED
LevenshteinIndex.cc
IndexMatches.cc
ErrorValues.cc
Word.cc
ColumbusHelpers.cc
ColumbusSlow.cc
WordList.cc
Document.cc
Corpus.cc
Matcher.cc
MatchResults.cc
IndexWeights.cc
MatcherStatistics.cc
WordStore.cc
ColumbusCAPI.cc
ErrorMatrix.cc
ResultFilter.cc
Trie.cc
SearchParameters.cc
)

if(ICONV_LIBRARIES)
  target_link_libraries(${COL_LIB_BASENAME} ${ICONV_LIBRARIES})
endif()
target_link_libraries(${COL_LIB_BASENAME} ${ICU_LIBRARIES})

set(symbol_map "${CMAKE_CURRENT_SOURCE_DIR}/libcolumbus.map")
set_target_properties(${COL_LIB_BASENAME} PROPERTIES VERSION ${SO_VERSION} SOVERSION ${ABI_VERSION})
set_target_properties(${COL_LIB_BASENAME} PROPERTIES
    LINK_FLAGS "${ldflags} -Wl,--version-script,${symbol_map}")
set_target_properties(${COL_LIB_BASENAME} PROPERTIES LINK_DEPENDS ${symbol_map})
add_pch(pch/columbus_pch.hh ${COL_LIB_BASENAME})

install(
  TARGETS ${COL_LIB_BASENAME}
  ARCHIVE DESTINATION ${LIBDIR}
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION ${LIBDIR}
)
