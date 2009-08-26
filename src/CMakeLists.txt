cmake_minimum_required(VERSION 2.5)


if(NOT MAINBUILD)
  project(datadraw)

  set(CMAKE_MODULE_PATH           ${PROJECT_SOURCE_DIR}/../cmake)
  set(PACKAGE                     ${PROJECT_NAME})
  set(CMAKE_SKIP_BUILD_RPATH      TRUE)
  set(CMAKE_CURRENT_BINARY_DIR    ${PROJECT_SOURCE_DIR}/bin)
  set(EXECUTABLE_OUTPUT_PATH      ${CMAKE_CURRENT_BINARY_DIR})
  set(REAL_EXECUTABLE_OUTPUT_PATH ${EXECUTABLE_OUTPUT_PATH}/${CMAKE_CFG_INTDIR})

  set(CMAKE_VERBOSE_MAKEFILE TRUE CACHE BOOLEAN "" FORCE)

  add_subdirectory(../util ${CMAKE_CURRENT_BINARY_DIR})

  if(MSVC AND NOT MSVC60)
    add_definitions(
      -D_CRT_FAR_MAPPINGS_NO_DEPRECATE
      -D_CRT_IS_WCTYPE_NO_DEPRECATE
      -D_CRT_MANAGED_FP_NO_DEPRECATE
      -D_CRT_NONSTDC_NO_DEPRECATE
      -D_CRT_NONSTDC_NO_WARNINGS
      -D_CRT_SECURE_NO_DEPRECATE
      -D_CRT_SECURE_NO_DEPRECATE_GLOBALS
      -D_CRT_SECURE_NO_WARNINGS
      -D_CRT_SETERRORMODE_BEEP_SLEEP_NO_DEPRECATE
      -D_CRT_TIME_FUNCTIONS_NO_DEPRECATE
      -D_CRT_VCCLRIT_NO_DEPRECATE
      -D_SCL_SECURE_NO_DEPRECATE
    )
  endif(MSVC AND NOT MSVC60)
else(NOT MAINBUILD)
  add_custom_command(
    OUTPUT dvdatabase.c
    COMMAND ${DATADRAW} Database.dd
    COMMENT "Pre-compiling Database.dd"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  )
endif(NOT MAINBUILD)


include(FindBISON)
include(FindFLEX)

set(BISON_FIND_REQUIRED TRUE)
find_package(BISON)

set(FLEX_FIND_REQUIRED TRUE)
find_package(FLEX)

BISON_TARGET(dvParse dvparse.y ${CMAKE_CURRENT_SOURCE_DIR}/dvparse.c COMPILE_FLAGS " -l -d -p dv -b dv ")
FLEX_TARGET(dvScan dvscan.l ${CMAKE_CURRENT_SOURCE_DIR}/dvscan.c COMPILE_FLAGS " -L -f -Pdvlex ")
ADD_FLEX_BISON_DEPENDENCY(dvScan dvParse)


include_directories(
  ..
  ../util
)


if(MSVC)
  include_directories(msw)
endif(MSVC)


add_executable(
  datadraw

  # dvParse
  dvparse.c dvparse.h
  # dvScan
  dvscan.c

  dv.h
  dvadmin.c
  dvbuild.c
  dvdatabase.c
  dvdatabase.h
  dvgenc.c
  dvgenh.c
  dvgenerate.c
  dvlexwrap.c
  dvmain.c
  dvparse.h
  dvread.c
  dvutil.c
)

set_source_files_properties(dvparse.c dvparse.h dvscan.c PROPERTIES GENERATED TRUE)

target_link_libraries(datadraw ddutil)

install(
  TARGETS datadraw
  RUNTIME DESTINATION bin
)
