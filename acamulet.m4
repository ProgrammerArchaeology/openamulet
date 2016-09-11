dnl Macros searching the location of Open-Amulet libraries
dnl  Author: 1999 Rodolphe Ortalo   (w/ lots of cut and paste
dnl             from the standard definition of AC_PATH_X).
dnl ==============================================================
dnl Usage: AC_PATH_AMULET()
dnl  Locates the OpenAmulet libraries and headers (search in
dnl several common locations) and sets output variables:
dnl    $no_amulet : yes,[]
dnl    $amulet_includes : path
dnl    $amulet_libraries : path
dnl  Also add one option '--with-amulet' to the configure script
dnl  that allows the builder to indicate a custom location prefix.
dnl ==============================================================
dnl Usage: AC_PATH_AMULET_EXTRA()
dnl  Defines a few variables used directly for compiler OpenAmulet
dnl programs:
dnl  Preprocessor symbol: AMULET_MISSING
dnl  Variables: AMULET_CFLAGS AMULET_LIBS
dnl             AMULET_PRE_LIBS AMULET_EXTRA_LIBS
dnl   (for CPPFLAGS and CXXFLAGS)
dnl Find additional X libraries, magic flags, etc.
dnl ==============================================================
AC_DEFUN(AC_PATH_AMULET,
[AC_REQUIRE_CPP()dnl Set CPP; we run AC_PATH_AMULET_DIRECT conditionally.
# If we find Amulet libs, set shell vars amulet_includes and amulet_libraries
# to the paths, otherwise set no_amulet=yes.
# Uses ac_ vars as temps to allow command line to override cache and checks.
# --without-amulet overrides everything else, but does not touch the cache.
AC_MSG_CHECKING(for OpenAmulet)

AC_ARG_WITH(amulet, [  --with-amulet           use the OpenAmulet libraries])
# $have_amulet is `yes', `no', `disabled', or empty when we do not yet know.
if test "x$with_amulet" = xno; then
  # The user explicitly disabled Amulet.
  have_amulet=disabled
else
  if test "x$with_amulet" = xyes || test "x$with_amulet" = x; then
    amulet_includes="NONE"
    amulet_libraries="NONE"
  else
    amulet_includes="$with_amulet/include"
    amulet_libraries="$with_amulet/lib"    
  fi
  if test "x$amulet_includes" != xNONE && test "x$amulet_libraries" != xNONE; then
    # Both variables are already set.
    have_amulet=yes
  else
AC_CACHE_VAL(ac_cv_have_amulet,
[# One or both of the vars are not set, and there is no cached value.
ac_amulet_includes=NO ac_amulet_libraries=NO
AC_PATH_AMULET_DIRECT
if test "$ac_amulet_includes" = NO || test "$ac_amulet_libraries" = NO; then
  # Didn't find Amulet anywhere.  Cache the known absence of Amulet.
  ac_cv_have_amulet="have_amulet=no"
else
  # Record where we found AMULET for the cache.
  ac_cv_have_amulet="have_amulet=yes \
	          ac_amulet_includes=$ac_amulet_includes ac_amulet_libraries=$ac_amulet_libraries"
fi])dnl
  fi
  eval "$ac_cv_have_amulet"
fi # $with_amulet != no

if test "$have_amulet" != yes; then
  AC_MSG_RESULT($have_amulet)
  no_amulet=yes
else
  # If each of the values was on the command line, it overrides each guess.
  test "x$amulet_includes" = xNONE && amulet_includes=$ac_amulet_includes
  test "x$amulet_libraries" = xNONE && amulet_libraries=$ac_amulet_libraries
  # Update the cache value to reflect the command line values.
  ac_cv_have_amulet="have_amulet=yes \
		  ac_amulet_includes=$amulet_includes ac_amulet_libraries=$amulet_libraries"
  AC_MSG_RESULT([libraries $amulet_libraries, headers $amulet_includes])
fi
])

dnl Internal subroutine of AC_PATH_AMULET.
dnl Set ac_amulet_includes and/or ac_amulet_libraries.
AC_DEFUN(AC_PATH_AMULET_DIRECT,
[if test "$ac_amulet_includes" = NO; then
  # Guess where to find include files, by looking for this one Amulet .h file.
  test -z "$amulet_direct_test_include" && amulet_direct_test_include=amulet/opal.h

  # First, try using that file with no special directory specified.
AC_TRY_CPP([#include <$amulet_direct_test_include>],
[# We can compile using Amulet headers with no special include directory.
ac_amulet_includes=],
[# Look for the header file in a wide set of common directories.
# Check global directories _before_ local or user directories...
  for ac_dir in                        \
    /usr/amulet/include                \
    /usr/include/amulet                \
    /usr/openamulet/include            \
    /usr/include/openamulet            \
    /usr/open-amulet/include           \
    /usr/include/open-amulet           \
                                       \
    /usr/local/amulet/include          \
    /usr/local/include/amulet          \
    /usr/local/openamulet/include      \
    /usr/local/include/openamulet      \
    /usr/local/open-amulet/include     \
    /usr/local/include/open-amulet     \
                                       \
    /usr/include                       \
    /usr/local/include                 \
    /usr/unsupported/include           \
    ; \
  do
    if test -r "$ac_dir/$amulet_direct_test_include"; then
      ac_amulet_includes=$ac_dir
      break
    fi
  done])
fi # $ac_amulet_includes = NO

if test "$ac_amulet_libraries" = NO; then
  # Check for the libraries. We rely on libamulet
  # to be a core lib of all AMULET libraries (develop etc.)

  test -z "$amulet_direct_test_library" && amulet_direct_test_library=amulet
  test -z "$amulet_direct_test_function" && amulet_direct_test_function=Am_Initialize

  # See if we find them without any special options.
  # Don't add to $LIBS permanently.
  ac_save_LIBS="$LIBS"
  LIBS="-l$amulet_direct_test_library $LIBS"
AC_TRY_LINK([#include <amulet.h>],[${amulet_direct_test_function}();],
[LIBS="$ac_save_LIBS"
# We can link Amulet programs with libamulet with no special library path.
ac_amulet_libraries=],
[LIBS="$ac_save_LIBS"
# First see if replacing the include by lib works.
# (Other ideas anyone ? -- rodolphe)
for ac_dir in `echo "$ac_amulet_includes" | sed s/include/lib/` \
    /usr/amulet/lib                \
    /usr/lib/amulet                \
    /usr/openamulet/lib            \
    /usr/lib/openamulet            \
    /usr/open-amulet/lib           \
    /usr/lib/open-amulet           \
                                   \
    /usr/local/amulet/lib          \
    /usr/local/lib/amulet          \
    /usr/local/openamulet/lib      \
    /usr/local/lib/openamulet      \
    /usr/local/open-amulet/lib     \
    /usr/local/lib/open-amulet     \
                                   \
    /usr/lib                       \
    /usr/local/lib                 \
    /usr/unsupported/lib           \
    ; \
do
dnl Don't even attempt the hair of trying to link an Amulet program!
  for ac_extension in a so sl; do
    if test -r $ac_dir/lib${amulet_direct_test_library}.$ac_extension; then
      ac_amulet_libraries=$ac_dir
      break 2
    fi
  done
done])
fi # $ac_amulet_libraries = NO
])
AC_DEFUN(AC_PATH_AMULET_EXTRA,
[AC_REQUIRE([AC_PATH_AMULET])dnl
if test "$no_amulet" = yes; then
  # Not all programs may use this symbol, but it does not hurt to define it.
  AC_DEFINE(AMULET_MISSING)
  AMULET_CFLAGS= AMULET_PRE_LIBS= AMULET_LIBS= AMULET_EXTRA_LIBS=
else
  if test -n "$amulet_includes"; then
    AMULET_CFLAGS="$AMULET_CFLAGS -I$amulet_includes"
  fi
  if test -n "$amulet_libraries"; then
    AMULET_LIBS="$AMULET_LIBS -L$amulet_libraries"
  fi
  # Currently, no further check are done. But this will surely
  # change in the future
  AMULET_PRE_LIBS= AMULET_EXTRA_LIBS=
fi
AC_SUBST(AMULET_CFLAGS)dnl
AC_SUBST(AMULET_PRE_LIBS)dnl
AC_SUBST(AMULET_LIBS)dnl
AC_SUBST(AMULET_EXTRA_LIBS)dnl
])
