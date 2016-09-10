check_PROGRAMS =  testgobs testlines testmap testopal testpolyweb \
 testJIO testoffset testcoord testcursor testfade testflip

testlines_SOURCES = testlines.cpp
testgobs_SOURCES = testgobs.cpp
testmap_SOURCES = testmap.cpp
testopal_SOURCES = testopal.cpp
testpolyweb_SOURCES = testpolyweb.cpp
testJIO_SOURCES = testJIO.cpp testJIO_EUC.h testJIO_JIS.h testJIO_SJIS.h
testoffset_SOURCES = testoffset.cpp
testcoord_SOURCES = testcoord.cpp
testcursor_SOURCES = testcursor.cpp
testfade_SOURCES = testfade.cpp
testflip_SOURCES = testflip.cpp

INCLUDES = -I$(top_srcdir)/header -I$(top_srcdir)/header/amulet
LDADD = ${top_builddir}/source/libamulet-develop.la

## Deprecated thing ?
##example_SOURCES = example.cpp

EXTRA_DIST = example.cpp
