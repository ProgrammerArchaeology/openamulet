# We define here the default location for OpenAmulet data
# files (general images, default gestures classification, etc.)
INCLUDES = -I$(top_srcdir)/header -DDEFAULT_AMULET_DIR=\"$(pkgdatadir)\"

if BUILD_release
RELEASE = libnet.la
else
RELEASE =
endif

if BUILD_develop
DEVELOP = libnet-develop.la
else
DEVELOP =
endif

noinst_LTLIBRARIES = $(RELEASE) $(DEVELOP)

## Networking

libnet_la_SOURCES =	\
	connection.cpp	\
	network.cpp

EXTRA_DIST = 			\
	def_net_methods.cpp	\
	fborder_Little.cpp	\
	netmap.cpp

LINK=$(CXXLINK) # Needed to force linking with c++

libnet_develop_la_SOURCES = 
libnet_develop_la_LIBADD = $(patsubst %.cpp,%-develop.lo,$(filter %.cpp,$(libnet_la_SOURCES)))

# Don't touch these...
%-develop.lo: %.cpp
	$(LTCXXCOMPILE) @AMULET_DEVELOP_FLAGS@ -c $< -o $@
