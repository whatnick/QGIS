
FILE (GLOB IMAGES *.png)
INSTALL (FILES ${IMAGES}
         DESTINATION ${QGIS_DATA_DIR}/images/icons)

# OS X app/doc icons
IF (APPLE)
  ADD_SUBDIRECTORY(mac)
ENDIF (APPLE)
