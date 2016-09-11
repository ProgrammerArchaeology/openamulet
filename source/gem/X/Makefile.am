# We define here the default location for OpenAmulet data
# files (general images, default gestures classification, etc.)
INCLUDES = -I$(top_srcdir)/header -DDEFAULT_AMULET_DIR=\"$(pkgdatadir)\"

if BUILD_release
RELEASE = libXgem.la
else
RELEASE =
endif

if BUILD_develop
DEVELOP = libXgem-develop.la
else
DEVELOP =
endif

GEMSUBLIBS = $(RELEASE) $(DEVELOP)

noinst_LTLIBRARIES = $(GEMSUBLIBS)

## GEM sources - X11-specific only (gem/Xgen/)
libXgem_la_SOURCES = gemX_cursor.cpp	\
	gemX_draw.cpp		\
	gemX_image.cpp		\
	gemX_input.cpp		\
	gemX_points.cpp		\
	gemX_regions.cpp	\
	gemX_styles.cpp		\
	gemX_text.cpp		\
	gemX_time.cpp		\
	gemX_windows.cpp

EXTRA_DIST = gemX_file.cpp

LINK=$(CXXLINK) # Needed to force linking with c++

libXgem_develop_la_SOURCES = 
libXgem_develop_la_LIBADD = $(patsubst %.cpp,%-develop.lo,$(filter %.cpp,$(libXgem_la_SOURCES)))

# Don't touch these...
%-develop.lo: %.cpp
	$(LTCXXCOMPILE) @AMULET_DEVELOP_FLAGS@ -c $< -o $@
