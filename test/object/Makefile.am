TESTS_ENVIRONMENT = AMULET_DIR=$(top_srcdir)/data
TESTS = testobject

check_PROGRAMS = testobject

testobject_SOURCES = testobject.cpp

INCLUDES = -I$(top_srcdir)/header -I$(top_srcdir)/header/amulet
LDADD = ${top_builddir}/source/libamulet-develop.la
