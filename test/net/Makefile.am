check_PROGRAMS = test_perobj testdemon testobj \
 testorder testtype

test_perobj_SOURCES = test_perobj.cpp
testdemon_SOURCES = testdemon.cpp
##testmap_SOURCES = testmap.cpp
##testnet_SOURCES = testnet.cpp
testobj_SOURCES = testobj.cpp
testorder_SOURCES = testorder.cpp
##testproto_SOURCES = testproto.cpp
testtype_SOURCES = testtype.cpp

INCLUDES = -I$(top_srcdir)/header
LDADD = ${top_builddir}/source/libamulet-develop.la

EXTRA_DIST = testmap.cpp testproto.cpp
