# We define here the default location for OpenAmulet data
# files (general images, default gestures classification, etc.)
INCLUDES = -I$(top_srcdir)/header -DDEFAULT_AMULET_DIR=\"$(pkgdatadir)\"

if BUILD_release
RELEASE = libinter.la
else
RELEASE =
endif

if BUILD_develop
DEVELOP = libinter-develop.la
else
DEVELOP =
endif

noinst_LTLIBRARIES = $(RELEASE) $(DEVELOP)

## Interactors (inter/)

libinter_la_SOURCES =		\
	command.cpp		\
	command_basics.cpp	\
	commands_other.cpp	\
	inter_basics.cpp	\
	inter_choice.cpp	\
	inter_globals.cpp	\
	inter_location.cpp	\
	inter_move_grow.cpp	\
	inter_new_points.cpp	\
	inter_text.cpp		\
	multiple_undo.cpp	\
	single_undo.cpp		\
	undo_handler.cpp	\
	user_id.cpp

LINK=$(CXXLINK) # Needed to force linking with c++

libinter_develop_la_SOURCES = 
libinter_develop_la_LIBADD = $(patsubst %.cpp,%-develop.lo,$(filter %.cpp,$(libinter_la_SOURCES)))

# Don't touch these...
%-develop.lo: %.cpp
	$(LTCXXCOMPILE) @AMULET_DEVELOP_FLAGS@ -c $< -o $@
