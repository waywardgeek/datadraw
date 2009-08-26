

add_custom_command(
  OUTPUT dwdatabase.c
  COMMAND ${DATADRAW} -I ${CMAKE_CURRENT_SOURCE_DIR}/../src Dataview.dd
  COMMENT "Pre-compiling Dataview.dd"
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)


include_directories(
  ..
  ../util
  ../src
)


add_executable(
  dataview

  dwdatabase.c
  dwdatabase.h

  ../src/dvparse.c
  ../src/dvparse.h

  ../src/dvscan.c

  ../src/dvdatabase.c
  ../src/dvdatabase.h

  dw.h
  dwanneal.c
  dwmain.c
  dwplace.c
  dwpostscript.c
  dwroute.c

  ../src/dvbuild.c
  ../src/dvlexwrap.c
  ../src/dvread.c
  ../src/dvutil.c
)

set_source_files_properties(dwdatabase.c dwdatabase.h ../src/dvparse.c ../src/dvparse.h ../src/dvscan.c ../src/dvdatabase.c ../src/dvdatabase.h PROPERTIES GENERATED TRUE)

target_link_libraries(dataview ddutil m)

install(
  TARGETS dataview
  RUNTIME DESTINATION bin
)
