########################################################
# Files

SET(SPLASH_SRCS splash.cpp)

IF (WIN32)
  IF (MSVC)
    ADD_DEFINITIONS("-DSPLASH_EXPORT=__declspec(dllexport)")
  ELSE (MSVC)
    ADD_DEFINITIONS("-USPLASH_EXPORT \"-DSPLASH_EXPORT=__declspec(dllexport)\"")
  ENDIF (MSVC)
ENDIF (WIN32)

ADD_LIBRARY(qgis_splash SHARED ${SPLASH_SRCS})

INSTALL(TARGETS qgis_splash
  RUNTIME DESTINATION ${QGIS_BIN_DIR}
  LIBRARY DESTINATION ${QGIS_LIB_DIR}
  ARCHIVE DESTINATION ${QGIS_LIB_DIR}
  )
