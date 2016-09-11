SUBDIRS = objects .

# We define here the default location for OpenAmulet data
# files (general images, default gestures classification, etc.)
INCLUDES = -I$(top_srcdir)/header -DDEFAULT_AMULET_DIR=\"$(pkgdatadir)\"

if BUILD_release
RELEASE = libopal.la
else
RELEASE =
endif

if BUILD_develop
DEVELOP = libopal-develop.la
else
DEVELOP =
endif

noinst_LTLIBRARIES = $(RELEASE) $(DEVELOP)

## Opal drawing lib (opal/)

libopal_la_SOURCES =	\
	color_conversion.cpp	\
	constraints.cpp		\
	constraints_alignment.cpp \
	constraints_with_arguments.cpp \
	event_timing.cpp	\
	load_save.cpp		\
	opal.cpp		\
	opal_code.cpp		\
	opal_operations.cpp	\
	slot_names.cpp		\
	state_store.cpp		\
	styles.cpp		\
	text_functions.cpp	\
	window_todo.cpp

libopal_la_LIBADD = objects/libopal_objects.la

LINK=$(CXXLINK) # Needed to force linking with c++

libopal_develop_la_SOURCES = 
libopal_develop_la_LIBADD = $(patsubst %.cpp,%-develop.lo,$(filter %.cpp,$(libopal_la_SOURCES))) objects/libopal_objects-develop.la

# Don't touch these...
%-develop.lo: %.cpp
	$(LTCXXCOMPILE) @AMULET_DEVELOP_FLAGS@ -c $< -o $@
