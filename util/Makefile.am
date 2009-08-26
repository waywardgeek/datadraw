## -*- Makefile -*-
##
##

lib_LIBRARIES=	 libddutil.a libddutilp.a libddutilu.a libddutilup.a libddutil-dbg.a libddutilp-dbg.a libddutilu-dbg.a libddutilup-dbg.a

libddutil_a_SOURCES=	${SRCS} utdatabase.c utdatabase.h
libddutilp_a_SOURCES=	${SRCS} utdatabasep.c utdatabasep.h
libddutilu_a_SOURCES=	${SRCS} utdatabaseu.c utdatabaseu.h
libddutilup_a_SOURCES=	${SRCS} utdatabaseup.c utdatabaseup.h
libddutil_dbg_a_SOURCES=	${SRCS} utdatabase.c utdatabase.h
libddutilp_dbg_a_SOURCES=	${SRCS} utdatabasep.c utdatabasep.h
libddutilu_dbg_a_SOURCES=	${SRCS} utdatabaseu.c utdatabaseu.h
libddutilup_dbg_a_SOURCES=	${SRCS} utdatabaseup.c utdatabaseup.h

SRCS=	ddutil.h util.c utmanage.c utmem.c utmem.h utpersist.c utpersist.h utrand.c uttypes.h utoslayer.c

include_HEADERS= ddutil.h utdatabase.h utdatabasep.h utdatabaseu.h utdatabaseup.h utmem.h utpersist.h uttypes.h

# used by windows -- do I need to do something for cygwin/mingw here???
EXTRA_DIST=	utnt.c

libddutil_a_CPPFLAGS= -DUT_USE_UTDATABASE_H
libddutilp_a_CPPFLAGS= -DUT_USE_UTDATABASEP_H
libddutilu_a_CPPFLAGS= -DUT_USE_UTDATABASEU_H
libddutilup_a_CPPFLAGS= -DUT_USE_UTDATABASEUP_H
libddutil_dbg_a_CPPFLAGS= -DUT_USE_UTDATABASE_H -DDD_DEBUG
libddutilp_dbg_a_CPPFLAGS= -DUT_USE_UTDATABASEP_H -DDD_DEBUG
libddutilu_dbg_a_CPPFLAGS= -DUT_USE_UTDATABASEU_H -DDD_DEBUG
libddutilup_dbg_a_CPPFLAGS= -DUT_USE_UTDATABASEUP_H -DDD_DEBUG

# hopefully this won't cause a chicken and egg problem...
DDR=  ../src/datadraw
utdatabase.h utdatabase.c: $(srcdir)/DatadrawUtil.dd
	if test -f ${DDR} ; then \
		${DDR} -s utdatabase.c -h utdatabase.h $(srcdir)/DatadrawUtil.dd ; \
	else \
		echo "${DDR} has not been built yet.  Using pre-built $@" ; \
	fi

utdatabasep.h utdatabasep.c: $(srcdir)/DatadrawUtil.dd
	if test -f ${DDR} ; then \
		${DDR} -p -s utdatabasep.c -h utdatabasep.h $(srcdir)/DatadrawUtil.dd ; \
	else \
		echo "${DDR} has not been built yet.  Using pre-built $@" ; \
	fi

utdatabaseu.h utdatabaseu.c: $(srcdir)/DatadrawUtil.dd
	if test -f ${DDR} ; then \
		${DDR} -u -s utdatabaseu.c -h utdatabaseu.h $(srcdir)/DatadrawUtil.dd ; \
	else \
		echo "${DDR} has not been built yet.  Using pre-built $@" ; \
	fi

utdatabaseup.h utdatabaseup.c: $(srcdir)/DatadrawUtil.dd
	if test -f ${DDR} ; then \
		${DDR} -u -p -s utdatabaseup.c -h utdatabaseup.h $(srcdir)/DatadrawUtil.dd ; \
	else \
		echo "${DDR} has not been built yet.  Using pre-built $@" ; \
	fi


# should figure out how to appropriately pass down the different flags to a 
# suffix rule.  This may require exploiting some GNU make specific features
# which would lock the user into GNU make.  That is probably not too bad of
# a thing to do since many users have already been forced into GNU make by
# other software
# SUFFIXES
#.dd.c:
#   if test -x ${DDR} ; then \
#      ${DDR} -s $@ -h $ $@.h $(srcdir)/DatadrawUtil.dd ; \
#   else \
#      echo "WARNING:  DatadrawUtil.dd has changed making dvdatabase.c and dvdatabase.h out of date" ; \
#      echo "          However you have not finished building datadraw yet.  So you may want to" ; \
#      echo "          build again a 2nd time after your build (hopefully) finishes." ; \
#   fi

# be sure to include the .dd file in the distribution
EXTRA_DIST+= DatadrawUtil.dd

