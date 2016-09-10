SUBDIRS = objects .
# We define here the default location for OpenAmulet data
# files (general images, default gestures classification, etc.)
INCLUDES = -I$(top_srcdir)/header -DDEFAULT_AMULET_DIR=\"$(pkgdatadir)\"

if BUILD_release
RELEASE = libwidgets.la
else
RELEASE =
endif

if BUILD_develop
DEVELOP = libwidgets-develop.la
else
DEVELOP =
endif

noinst_LTLIBRARIES = $(RELEASE) $(DEVELOP)

## Widgets finally (widgets/)

libwidgets_la_SOURCES =		\
	border_misc.cpp		\
	button_commands.cpp	\
	button_draw.cpp		\
	button_widgets.cpp	\
	dialog_widgets.cpp	\
	editing_commands.cpp	\
	listbox_widget.cpp	\
	panel_code.cpp		\
	rich_text.cpp		\
	scroll_widgets.cpp	\
	selection_widgets.cpp	\
	text_widgets.cpp	\
	widget_globals.cpp	\
	widgets.cpp		\
	widgets_code.cpp

libwidgets_la_LIBADD = objects/libwidgets_objects.la

LINK=$(CXXLINK) # Needed to force linking with c++

libwidgets_develop_la_SOURCES = 
libwidgets_develop_la_LIBADD = $(patsubst %.cpp,%-develop.lo,$(filter %.cpp,$(libwidgets_la_SOURCES))) objects/libwidgets_objects-develop.la

# Don't touch these...
%-develop.lo: %.cpp
	$(LTCXXCOMPILE) @AMULET_DEVELOP_FLAGS@ -c $< -o $@
