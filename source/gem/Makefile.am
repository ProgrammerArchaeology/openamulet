SUBDIRS = X ggi . mac win32

# We define here the default location for OpenAmulet data
# files (general images, default gestures classification, etc.)
INCLUDES = -I$(top_srcdir)/header -DDEFAULT_AMULET_DIR=\"$(pkgdatadir)\"

if BUILD_release
RELEASE = libgem.la
else
RELEASE =
endif

if BUILD_develop
DEVELOP = libgem-develop.la
else
DEVELOP =
endif

noinst_LTLIBRARIES = $(RELEASE) $(DEVELOP)

# Describes which sub-library to include in GEM
## TODO: A 'case' would be better here probably...
if USE_ggi
GEM_SUBLIB = ggi/libggigem.la
GEM_SUBLIB_DEV = ggi/libggigem-develop.la
else
# Default is X windows
GEM_SUBLIB = X/libXgem.la
GEM_SUBLIB_DEV = X/libXgem-develop.la
endif

libgem_la_SOURCES =		\
	gem_keytrans.cpp	\
	gem_points.cpp		\
	gem_GIF.cpp

libgem_la_LIBADD = $(GEM_SUBLIB)

EXTRA_DIST = console.cpp

LINK=$(CXXLINK) # Needed to force linking with c++

libgem_develop_la_SOURCES = 
libgem_develop_la_LIBADD = $(patsubst %.cpp,%-develop.lo,$(filter %.cpp,$(libgem_la_SOURCES))) $(GEM_SUBLIB_DEV)

# Don't touch these...
%-develop.lo: %.cpp
	$(LTCXXCOMPILE) @AMULET_DEVELOP_FLAGS@ -c $< -o $@
