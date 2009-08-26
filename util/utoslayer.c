#if _WIN32
#  ifdef _MSC_VER
#    include "utnt.c"
#  else /* utcygwin works in MinGW too */
#    include "utcygwin.c"
#  endif
#else
#  include "utunix.c"
#endif
