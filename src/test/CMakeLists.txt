# for installing headers

SET(QGIS_TEST_HDRS
  qgstest.h
)

INSTALL(FILES ${QGIS_TEST_HDRS} DESTINATION ${QGIS_INCLUDE_DIR})
