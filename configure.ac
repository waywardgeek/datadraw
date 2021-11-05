dnl Process this file with autoconf to produce a configure script.
dnl
dnl Contributed by Dan McMahill
dnl 

AC_INIT(src/dvmain.c)
AM_INIT_AUTOMAKE(datadraw, 3.1.X-unstable)
AM_CONFIG_HEADER(config.h)

AM_MAINTAINER_MODE

dnl checks for programs.
AC_PROG_CC
AC_PROG_RANLIB
AC_PROG_INSTALL

AM_PROG_LEX
AC_PROG_YACC

# Datadraw uses itself to produce one of its files.
# if we have a datadraw installed go ahead and enable
# some extra rules.  FIXME -- do we ultimately need a 2
# stage build like what gcc does?  Build datadraw then
# rebuild again??
#
# If we are cross compiling then we need to search for a 
# datadraw binary to use for the build of dataview.  This can
# either be an installed datadraw in our search path or 
# it can be specified like:
#
# env DATADRAW=/opt/ddr/bin/datadraw ./configure \
#    --host=alpha--netbsd --build=i686--linux
#

if test "$cross_compiling" = "yes" ; then
	AC_PATH_PROG(DATADRAW, datadraw, [notfound])
else
	DATADRAW=../src/datadraw
	AC_SUBST([DATADRAW])
fi
AM_CONDITIONAL(MISSING_DATADRAW, test x$DATADRAW = xnotfound)

# if we have gcc and we've asked for debugging then add lots of -W
if test "x$GCC" = "xyes" ; then
	for flag in -Wall -W -Wno-unused-parameter -Wno-unused-function; do
		case " ${CFLAGS} " in
			*\ ${flag}\ *)
				# flag is already present
				;;

			*)
				AC_MSG_CHECKING([If the compiler accepts ${flag}])
				old_CFLAGS="$CFLAGS"
				CFLAGS="$CFLAGS ${flag}"
				AC_COMPILE_IFELSE([AC_LANG_PROGRAM()],
					[AC_MSG_RESULT([yes])],
					[AC_MSG_RESULT([no])
					 CFLAGS="${old_CFLAGS}"
					]
				)
				;;
		esac
	done
fi

dnl checks for headers
#AC_CHECK_HEADERS([ctype.h stdlib.h string.h])


AC_MSG_RESULT([
** Configuration summary for $PACKAGE $VERSION:

   CPPFLAGS:     $CPPFLAGS
   CFLAGS:       $CFLAGS
   LIBS:         $LIBS

])

AC_OUTPUT([Makefile util/Makefile src/Makefile dataview/Makefile])

