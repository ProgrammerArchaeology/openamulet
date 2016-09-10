AUTOMAKE_OPTIONS = dejagnu

DEJATOOL = gem

# Test subdirs
SUBDIRS = gem.cmu config

check_PROGRAMS = testclip testcopy  testdpy testgem testinput \
 testloop testpixmap testpoints testsubwins testtrans testwinsizes

testclip_SOURCES = testclip.cpp
testcopy_SOURCES = testcopy.cpp
testdpy_SOURCES = testdpy.cpp testcolor.cpp testfonts.cpp testbits.cpp \
 testwinprops.cpp test_utils.cpp test_utils.h
testgem_SOURCES = testgem.cpp test_utils.cpp testlineprops.cpp
testinput_SOURCES = testinput.cpp
testloop_SOURCES = testloop.cpp
testpixmap_SOURCES = testpixmap.cpp
testpoints_SOURCES = testpoints.cpp
testsubwins_SOURCES = testsubwins.cpp
testtrans_SOURCES = testtrans.cpp
testwinsizes_SOURCES = testwinsizes.cpp

INCLUDES = -I$(top_srcdir)/header
LDADD = ${top_builddir}/source/libamulet-develop.la

EXTRA_DIST = testMS1.cpp
