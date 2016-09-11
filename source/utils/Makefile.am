# We define here the default location for OpenAmulet data
# files (general images, default gestures classification, etc.)
INCLUDES = -I$(top_srcdir)/header -DDEFAULT_AMULET_DIR=\"$(pkgdatadir)\"

if BUILD_release
RELEASE = libutils.la
else
RELEASE =
endif

if BUILD_develop
DEVELOP = libutils-develop.la
else
DEVELOP =
endif

noinst_LTLIBRARIES = $(RELEASE) $(DEVELOP)

## Utilities (utils/)
libutils_la_SOURCES =		\
	am_enum_support.cpp	\
	am_load_save.cpp	\
	am_method_wrapper.cpp	\
	am_string.cpp		\
	am_value.cpp		\
	am_wrapper.cpp		\
	initializer.cpp		\
	misc.cpp		\
	priority_list.cpp	\
	registered_type.cpp	\
	registry.cpp		\
	standard_slots.cpp	\
	stdvalue.cpp		\
	symbol_table.cpp	\
	types.cpp		\
	univ_lst.cpp		\
	univ_map.cpp		\
	value_list.cpp		\
	filefind.cpp

EXTRA_DIST = directory.cpp switchboard.cpp


LINK=$(CXXLINK) # Needed to force linking with c++

libutils_develop_la_SOURCES = 
libutils_develop_la_LIBADD = $(patsubst %.cpp,%-develop.lo,$(filter %.cpp,$(libutils_la_SOURCES)))

# Don't touch these...
%-develop.lo: %.cpp
	$(LTCXXCOMPILE) @AMULET_DEVELOP_FLAGS@ -c $< -o $@
