## -*- Makefile -*-
##
##

bin_PROGRAMS=	datadraw

datadraw_SOURCES= \
	dv.h dvadmin.c dvbuild.c dvdatabase.c dvdatabase.h dvgenc.c dvgenh.c \
	dvgenerate.c dvlexwrap.c dvmain.c dvparse.h dvparse.y dvread.c \
	dvscan.l dvutil.c 

INCLUDES=	-I $(srcdir)/../util
LDADD=	../util/libddutil-dbg.a

AM_YFLAGS=	-d -p dv
AM_LFLAGS=	-f

# hopefully this won't cause a chicken and egg problem...
DDR=	datadraw
dvdatabase.h dvdatabase.c: $(srcdir)/Database.dd
	@if test -x ./${DDR} ; then \
		echo "${DDR} $(srcdir)/Database.dd" ; \
		./${DDR} $(srcdir)/Database.dd || ${DDR} $(srcdir)/Database.dd; \
	else \
		echo "WARNING:  Database.dd has changed making dvdatabase.c and dvdatabase.h out of date" ; \
		echo "          However you have not finished building datadraw yet.  So you may want to" ; \
		echo "          build again a 2nd time after your build (hopefully) finishes." ; \
	fi

dvlexwrap.c: dvparse.h

# be sure to include the .dd file in the distribution
EXTRA_DIST=	Database.dd

