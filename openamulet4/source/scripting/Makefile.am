# We define here the default location for OpenAmulet data
# files (general images, default gestures classification, etc.)
INCLUDES = -I$(top_srcdir)/header -DDEFAULT_AMULET_DIR=\"$(pkgdatadir)\"

if BUILD_release
RELEASE = libscripting.la
else
RELEASE =
endif

if BUILD_develop
DEVELOP = libscripting-develop.la
else
DEVELOP =
endif

noinst_LTLIBRARIES = $(RELEASE) $(DEVELOP)

## The following is temptative and incorporate as much
## as possible from Amulet V4 (from CMU)
##
## Scripting

libscripting_la_SOURCES =	\
	askclickpos.cpp		\
	askforvalue.cpp		\
	askpositionandsize.cpp	\
	askpositionorsize.cpp	\
	askselectobj.cpp	\
	locgen.cpp		\
	generalizers.cpp	\
	genval.cpp		\
	invoke.cpp		\
	objgen.cpp		\
	scripting_window.cpp	\
	search_window.cpp	\
	undo_dialog_box.cpp	\
	valgen.cpp		\
	valinvokegen.cpp

LINK=$(CXXLINK) # Needed to force linking with c++

libscripting_develop_la_SOURCES = 
libscripting_develop_la_LIBADD = $(patsubst %.cpp,%-develop.lo,$(filter %.cpp,$(libscripting_la_SOURCES)))

# Don't touch these...
%-develop.lo: %.cpp
	$(LTCXXCOMPILE) @AMULET_DEVELOP_FLAGS@ -c $< -o $@
