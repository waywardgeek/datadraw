## -*- Makefile -*-
##
##

bin_PROGRAMS=	dataview

AUTOMAKE_OPTIONS = subdir-objects
INCLUDES= -I$(srcdir)/../src -I$(srcdir)/../util
LDADD= -lm ../util/libddutil.a

AM_YFLAGS=      -d -p dv
AM_LFLAGS=      -f

dataview_SOURCES= \
	dwdatabase.c dwdatabase.h \
	dw.h dwanneal.c dwmain.c dwplace.c dwpostscript.c \
	dwroute.c \
	$(srcdir)/../src/dvbuild.c \
	$(srcdir)/../src/dvdatabase.c \
	$(srcdir)/../src/dvdatabase.h \
	$(srcdir)/../src/dvlexwrap.c \
	$(srcdir)/../src/dvparse.y \
	$(srcdir)/../src/dvread.c \
	$(srcdir)/../src/dvscan.l \
	$(srcdir)/../src/dvutil.c

dwdatabase.h dwdatabase.c: Dataview.dd
	$(DATADRAW) -I $(srcdir)/../src Dataview.dd

# to help with parallel builds.  This prevents the 
# datadraw command from being executed twice in parallel.
dwdatabase.h: dwdatabase.c

CLEANFILES= dwdatabase.h dwdatabase.c

EXTRA_DIST=	Dataview.dd dwwrite.c

