# We define here the default location for OpenAmulet data
# files (general images, default gestures classification, etc.)
INCLUDES = -I$(top_srcdir)/header -DDEFAULT_AMULET_DIR=\"$(pkgdatadir)\"

if BUILD_release
RELEASE = libore.la
else
RELEASE =
endif

if BUILD_develop
DEVELOP = libore-develop.la
else
DEVELOP =
endif

noinst_LTLIBRARIES = $(RELEASE) $(DEVELOP)

## ORE object system (object/)

libore_la_SOURCES =	\
	object.cpp	\
	object_data.cpp	\
	formula.cpp	\
	web.cpp		\
	dynarray.cpp	\
	dynmm.cpp

LINK=$(CXXLINK) # Needed to force linking with c++

libore_develop_la_SOURCES = 
libore_develop_la_LIBADD = $(patsubst %.cpp,%-develop.lo,$(filter %.cpp,$(libore_la_SOURCES)))

## These are included conditionally
EXTRA_libore_la_SOURCES =	\
	dynarray_default.cpp	\
	dynmm_default.cpp	\
	dynarray_smartheap.cpp	\
	dynmm_smartheap.cpp

# Don't touch these...
%-develop.lo: %.cpp
	$(LTCXXCOMPILE) @AMULET_DEVELOP_FLAGS@ -c $< -o $@
