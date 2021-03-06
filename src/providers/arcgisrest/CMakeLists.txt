IF(NOT ENABLE_QT5)
FIND_PACKAGE(QJSON REQUIRED)
ENDIF(NOT ENABLE_QT5)

INCLUDE_DIRECTORIES(
  ../../core
  ../../core/auth
  ../../core/geometry
  ../../core/raster
  ../../gui
  ../../gui/auth
  ${GEOS_INCLUDE_DIR}
  ${QJSON_INCLUDE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}/../../ui
)

###############################################################################

SET (AFS_SRCS
  qgsafsdataitems.cpp
  qgsafsfeatureiterator.cpp
  qgsafsprovider.cpp
  qgsafsproviderextern.cpp
  qgsafssourceselect.cpp
)
SET (AFS_MOC_HDRS
  qgsafsdataitems.h
  qgsafsfeatureiterator.h
  qgsafsprovider.h
  qgsafssourceselect.h
)

QT4_WRAP_CPP (AFS_MOC_SRCS ${AFS_MOC_HDRS})

ADD_LIBRARY(arcgisfeatureserverprovider MODULE ${AFS_SRCS} ${AFS_MOC_SRCS})

TARGET_LINK_LIBRARIES(arcgisfeatureserverprovider
  qgis_core
  qgis_gui
  ${QJSON_LIBRARIES}
)

INSTALL (TARGETS arcgisfeatureserverprovider
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})

###############################################################################

SET (AMS_SRCS
  qgsamsdataitems.cpp
  qgsamsprovider.cpp
  qgsamsproviderextern.cpp
  qgsamssourceselect.cpp
)
SET (AMS_MOC_HDRS
  qgsamsdataitems.h
  qgsamsprovider.h
  qgsamssourceselect.h
)

QT4_WRAP_CPP (AMS_MOC_SRCS ${AMS_MOC_HDRS})

ADD_LIBRARY(arcgismapserverprovider MODULE ${AMS_SRCS} ${AMS_MOC_SRCS})

TARGET_LINK_LIBRARIES(arcgismapserverprovider
  qgis_core
  qgis_gui
  ${QJSON_LIBRARIES}
  ${qjson_LIBRARIES}
)

INSTALL (TARGETS arcgismapserverprovider
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
