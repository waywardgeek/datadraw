MACRO(IS_BUILD_DATADRAW)
  FIND_PROGRAM(
    DATADRAW
    datadraw
    PATHS
      ${PROJECT_SOURCE_DIR}/${CMAKE_FILES_DIRECTORY}/CMakeTmp
      ${PROJECT_SOURCE_DIR}/${CMAKE_FILES_DIRECTORY}/CMakeTmp/bin
      ${PROJECT_SOURCE_DIR}/bin
      ${PROJECT_SOURCE_DIR}/src/bin
    PATH_SUFFIXES
      Debug
      debug
      Release
      release
    NO_DEFAULT_PATH
  )
ENDMACRO(IS_BUILD_DATADRAW)


IS_BUILD_DATADRAW()


IF(NOT DATADRAW)

  MESSAGE(STATUS "Building datadraw...")
  FILE(MAKE_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
  TRY_COMPILE(
      DATADRAW_OK
      ${PROJECT_SOURCE_DIR}/${CMAKE_FILES_DIRECTORY}/CMakeTmp
      ${PROJECT_SOURCE_DIR}/src
      datadraw
  )

  IS_BUILD_DATADRAW()

  IF( NOT DATADRAW )
    MESSAGE(FATAL_ERROR "datadraw failed to build. This is a needed file for the database precompilation!")
  ENDIF( NOT DATADRAW )

ENDIF(NOT DATADRAW)
