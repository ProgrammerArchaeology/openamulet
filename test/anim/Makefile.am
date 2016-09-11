check_PROGRAMS = testanimators testaniminter

testanimators_SOURCES = testanimators.cpp
testaniminter_SOURCES = testaniminter.cpp

## Refers to an unknown 'event_count' variable
##testtiming_SOURCES = testtiming.cpp

INCLUDES = -I$(top_srcdir)/header -I$(top_srcdir)/header/amulet
LDADD = ${top_builddir}/source/libamulet-develop.la

EXTRA_DIST = testtiming.cpp
