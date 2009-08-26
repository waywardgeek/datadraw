

set(SRCS ddutil.h util.c utmanage.c utmem.c utmem.h utpersist.c utpersist.h utrand.c uttypes.h utoslayer.c)


set_directory_properties(PROPERTIES COMPILE_DEFINITIONS_DEBUG "DD_DEBUG")


add_library(
  ddutil
  STATIC

  ${SRCS} utdatabase.c utdatabase.h
)
set_target_properties(ddutil PROPERTIES COMPILE_DEFINITIONS UT_USE_UTDATABASE_H DEBUG_POSTFIX "_dbg")

add_library(
  ddutilp
  STATIC

  ${SRCS} utdatabasep.c utdatabasep.h
)
set_target_properties(ddutil PROPERTIES COMPILE_DEFINITIONS UT_USE_UTDATABASEP_H DEBUG_POSTFIX "_dbg")

add_library(
  ddutilu
  STATIC

  ${SRCS} utdatabaseu.c utdatabaseu.h
)
set_target_properties(ddutil PROPERTIES COMPILE_DEFINITIONS UT_USE_UTDATABASEU_H DEBUG_POSTFIX "_dbg")

add_library(
  ddutilup
  STATIC

  ${SRCS} utdatabaseup.c utdatabaseup.h
)
set_target_properties(ddutil PROPERTIES COMPILE_DEFINITIONS UT_USE_UTDATABASEUP_H DEBUG_POSTFIX "_dbg")

install(
  TARGETS
    ddutil ddutilp ddutilu ddutilup
  ARCHIVE DESTINATION lib
)

install(
  FILES
    ddutil.h
    utdatabase.h
    utdatabasep.h
    utdatabaseu.h
    utdatabaseup.h
    utmem.h
    utpersist.h
    uttypes.h
  DESTINATION
    include
)
