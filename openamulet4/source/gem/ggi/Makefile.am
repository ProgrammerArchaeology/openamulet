# We define here the default location for OpenAmulet data
# files (general images, default gestures classification, etc.)
INCLUDES = -I$(top_srcdir)/header -I$(top_srcdir)/source/gem \
 -DDEFAULT_AMULET_DIR=\"$(pkgdatadir)\"

if BUILD_release
RELEASE = libggigem.la
else
RELEASE =
endif

if BUILD_develop
DEVELOP = libggigem-develop.la
else
DEVELOP =
endif

if USE_ggi
GEMSUBLIBS = $(RELEASE) $(DEVELOP)
else
GEMSUBLIBS = 
endif

noinst_LTLIBRARIES = $(GEMSUBLIBS)

## GEM sources - GGI-specific only (gem/ggigem/)
libggigem_la_SOURCES =		\
	gemG_cursor.cpp		\
	gemG_draw.cpp		\
	gemG_image.cpp		\
	gemG_input.cpp		\
	gemG_points.cpp		\
	gemG_regions.cpp	\
	gemG_styles.cpp		\
	gemG_text.cpp		\
	gemG_time.cpp		\
	gemG_windows.cpp

EXTRA_DIST = gemG_file.cpp

LINK=$(CXXLINK) # Needed to force linking with c++

libggigem_develop_la_SOURCES = 
libggigem_develop_la_LIBADD = $(patsubst %.cpp,%-develop.lo,$(filter %.cpp,$(libggigem_la_SOURCES)))

# Don't touch these...
%-develop.lo: %.cpp
	$(LTCXXCOMPILE) @AMULET_DEVELOP_FLAGS@ -c $< -o $@
