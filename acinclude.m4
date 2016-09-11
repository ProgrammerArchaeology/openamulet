dnl Macro for testing location of GGI Project libraries
dnl  by: Rodolphe Ortalo (w/ lots of cut and past from the
dnl standard definition of AC_PATH_X).
AC_DEFUN(AC_PATH_GGI,
[AC_REQUIRE_CPP()dnl Set CPP; we run AC_PATH_GGI_DIRECT conditionally.
# If we find GGI libs, set shell vars ggi_includes and ggi_libraries to the
# paths, otherwise set no_ggi=yes.
# Uses ac_ vars as temps to allow command line to override cache and checks.
# --without-ggi overrides everything else, but does not touch the cache.
AC_MSG_CHECKING(for GGI libraries)

AC_ARG_WITH(ggi, [  --with-ggi              use the GGI Project Libraries])
# $have_ggi is `yes', `no', `disabled', or empty when we do not yet know.
if test "x$with_ggi" = xno; then
  # The user explicitly disabled GGI.
  have_ggi=disabled
else
  if test "x$with_ggi" = xyes || test "x$with_ggi" = x; then
    ggi_includes="NONE"
    ggi_libraries="NONE"
  else
    ggi_includes="$with_ggi/include"
    ggi_libraries="$with_ggi/lib"    
  fi
  if test "x$ggi_includes" != xNONE && test "x$ggi_libraries" != xNONE; then
    # Both variables are already set.
    have_ggi=yes
  else
AC_CACHE_VAL(ac_cv_have_ggi,
[# One or both of the vars are not set, and there is no cached value.
ac_ggi_includes=NO ac_ggi_libraries=NO
AC_PATH_GGI_DIRECT
if test "$ac_ggi_includes" = NO || test "$ac_ggi_libraries" = NO; then
  # Didn't find GGI anywhere.  Cache the known absence of GGI.
  ac_cv_have_ggi="have_ggi=no"
else
  # Record where we found GGI for the cache.
  ac_cv_have_ggi="have_ggi=yes \
	          ac_ggi_includes=$ac_ggi_includes ac_ggi_libraries=$ac_ggi_libraries"
fi])dnl
  fi
  eval "$ac_cv_have_ggi"
fi # $with_ggi != no

if test "$have_ggi" != yes; then
  AC_MSG_RESULT($have_ggi)
  no_ggi=yes
else
  # If each of the values was on the command line, it overrides each guess.
  test "x$ggi_includes" = xNONE && ggi_includes=$ac_ggi_includes
  test "x$ggi_libraries" = xNONE && ggi_libraries=$ac_ggi_libraries
  # Update the cache value to reflect the command line values.
  ac_cv_have_ggi="have_ggi=yes \
		  ac_ggi_includes=$ggi_includes ac_ggi_libraries=$ggi_libraries"
  AC_MSG_RESULT([libraries $ggi_libraries, headers $ggi_includes])
fi
])

dnl Internal subroutine of AC_PATH_GGI.
dnl Set ac_ggi_includes and/or ac_ggi_libraries.
AC_DEFUN(AC_PATH_GGI_DIRECT,
[if test "$ac_ggi_includes" = NO; then
  # Guess where to find include files, by looking for this one GGI .h file.
  test -z "$ggi_direct_test_include" && ggi_direct_test_include=ggi/gii.h

  # First, try using that file with no special directory specified.
AC_TRY_CPP([#include <$ggi_direct_test_include>],
[# We can compile using GGI headers with no special include directory.
ac_ggi_includes=],
[# Look for the header file in a wide set of common directories.
# Check global directories _before_ local or user directories...
# We try also the nicknames of distributions (releases) starting with degas.
  for ac_dir in               \
    /usr/GGI/include          \
    /usr/include/GGI          \
    /usr/ggi/include          \
    /usr/include/ggi          \
                              \
    /usr/local/GGI/include    \
    /usr/local/include/GGI    \
    /usr/local/ggi/include    \
    /usr/local/include/ggi    \
                              \
    /usr/degas/include        \
    /usr/include/degas        \
    /usr/local/degas/include  \
    /usr/local/include/degas  \
                              \
    /usr/include              \
    /usr/local/include        \
    /usr/unsupported/include  \
    ; \
  do
    if test -r "$ac_dir/$ggi_direct_test_include"; then
      ac_ggi_includes=$ac_dir
      break
    fi
  done])
fi # $ac_ggi_includes = NO

if test "$ac_ggi_libraries" = NO; then
  # Check for the libraries. We rely on gii (Generic Input Interface)
  # to be a core lib of all GGI libraries.

  test -z "$ggi_direct_test_library" && ggi_direct_test_library=gii
  test -z "$ggi_direct_test_function" && ggi_direct_test_function=giiInit

  # See if we find them without any special options.
  # Don't add to $LIBS permanently.
  ac_save_LIBS="$LIBS"
  LIBS="-l$ggi_direct_test_library $LIBS"
AC_TRY_LINK(, [${ggi_direct_test_function}()],
[LIBS="$ac_save_LIBS"
# We can link GGI programs with gii with no special library path.
ac_ggi_libraries=],
[LIBS="$ac_save_LIBS"
# First see if replacing the include by lib works.
# (Other ideas anyone ? -- rodolphe)
for ac_dir in `echo "$ac_ggi_includes" | sed s/include/lib/` \
    /usr/GGI/lib          \
    /usr/lib/GGI          \
    /usr/ggi/lib          \
    /usr/lib/ggi          \
                          \
    /usr/local/GGI/lib    \
    /usr/local/lib/GGI    \
    /usr/local/ggi/lib    \
    /usr/local/lib/ggi    \
                          \
    /usr/degas/lib        \
    /usr/lib/degas        \
    /usr/local/degas/lib  \
    /usr/local/lib/degas  \
                          \
    /usr/lib              \
    /usr/local/lib        \
    /usr/unsupported/lib  \
    ; \
do
dnl Don't even attempt the hair of trying to link a GGI program!
  for ac_extension in a so sl; do
    if test -r $ac_dir/lib${ggi_direct_test_library}.$ac_extension; then
      ac_ggi_libraries=$ac_dir
      break 2
    fi
  done
done])
fi # $ac_ggi_libraries = NO
])

dnl AC_CHECK_GGI_LIBS(LIBRARIES_LIST[,ACTION_IF_FOUND[,ACTION_IF_NOT_FOUND]])
AC_DEFUN(AC_CHECK_GGI_LIBS,
[AC_REQUIRE([AC_PATH_GGI])dnl
success=yes
if test "x$no_ggi" != xyes; then
  ac_here_save_LIBS="$LIBS"
  if test "x$ggi_libraries" != x; then
    LIBS="$LIBS -L$ggi_libraries"
  fi
  GGI_LIBS=""
  for ac_lib in $1; do
    case "$ac_lib" in
    gg)
      ac_lib_fct=ggLoadConfig;;
    gii)
      ac_lib_fct=giiInit ;;
    ggi)
      ac_lib_fct=ggiInit ;;
    ggimisc)
      ac_lib_fct=ggiMiscInit ;;
    ggi2d)
      ac_lib_fct=ggi2dInit ;;
    ggi3d)
      ac_lib_fct=ggi3DInit ;;
    gwt)
      ac_lib_fct=gwtInit ;;
    *)  AC_MSG_ERROR([Unknow GGI library component: $ac_lib])
      ac_lib_fct="" ;;
    esac
    AC_CHECK_LIB($ac_lib,$ac_lib_fct,GGI_LIBS="$GGI_LIBS -l$ac_lib",success=no,$GGI_LIBS)
  done
  LIBS="$ac_here_save_LIBS"
else
  success=no
fi
if test "x$success" = xyes; then
  ifelse([$2], , [if test "x$ggi_libraries" != x; then
LIBS="$LIBS -L$ggi_libraries"
fi
LIBS="$LIBS $GGI_LIBS"],[$2])
  ifelse([$3], , ,[else $3])
fi
])

dnl AC_CHECK_GGI_HEADERS(LIBRARIES_LIST[,ACTION_IF_FOUND[,ACTION_IF_NOT_FOUND]])
AC_DEFUN(AC_CHECK_GGI_HEADERS,
[AC_REQUIRE([AC_PATH_GGI])dnl
success=yes
if test "x$no_ggi" != xyes; then
  ac_save_CPPFLAGS="$CPPFLAGS"
  if test "x$ggi_includes" != x; then
    CPPFLAGS="$CPPFLAGS -I$ggi_includes"
  fi
  for ac_inc in $1; do
    case "$ac_inc" in
    gg)
      ac_inc_name=ggi/gg.h ;;
    gii)
      ac_inc_name=ggi/gii.h ;;
    ggi)
      ac_inc_name=ggi/ggi.h ;;
    ggimisc)
      ac_inc_name=ggi/ext/misc.h ;;
    ggi2d)
      ac_inc_name=ggi/ggi2d.h ;;
    ggi3d)
      ac_inc_name=ggi/ggi3d.h ;;
    gwt)
      ac_inc_name=ggi/gwt.h ;;
    *)  AC_MSG_ERROR([Unknown GGI include component: $ac_inc])
      ac_lib_fct="wiuyeriu.zyx" ;;
    esac
    AC_CHECK_HEADER(${ac_inc_name},,success=no)
  done
  CPPFLAGS="$ac_save_CPPFLAGS"
else
  success=no
fi
if test $success = yes; then
  ifelse([$2], , [if test "x$ggi_includes" != x; then
CPPFLAGS="$CPPFLAGS -I$ggi_includes"
fi], [$2])
  ifelse([$3], , , [else $3])
fi
])
