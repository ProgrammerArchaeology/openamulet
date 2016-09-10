# We define here the default location for OpenAmulet data
# files (general images, default gestures classification, etc.)
INCLUDES = -I$(top_srcdir)/header -DDEFAULT_AMULET_DIR=\"$(pkgdatadir)\"

if BUILD_release
RELEASE = libopal_objects.la
else
RELEASE =
endif

if BUILD_develop
DEVELOP = libopal_objects-develop.la
else
DEVELOP =
endif

noinst_LTLIBRARIES = $(RELEASE) $(DEVELOP)

## Opal drawing lib (opal/)

libopal_objects_la_SOURCES = \
	aggregates.cpp  \
	arc.cpp		\
	arrow_line.cpp	\
	bitmap.cpp	\
	fade_group.cpp	\
	flip_book_group.cpp \
	graphical_object.cpp \
	hidden_text.cpp	\
	line.cpp	\
	map.cpp		\
	polygon.cpp	\
	rectangle.cpp	\
	resize_group.cpp \
	roundtangle.cpp	\
	screen.cpp	\
	text.cpp	\
	window.cpp

LINK=$(CXXLINK) # Needed to force linking with c++

libopal_objects_develop_la_SOURCES = 
libopal_objects_develop_la_LIBADD = $(patsubst %.cpp,%-develop.lo,$(filter %.cpp,$(libopal_objects_la_SOURCES)))

# Don't touch these...
%-develop.lo: %.cpp
	$(LTCXXCOMPILE) @AMULET_DEVELOP_FLAGS@ -c $< -o $@
