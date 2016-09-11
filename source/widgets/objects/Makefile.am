# We define here the default location for OpenAmulet data
# files (general images, default gestures classification, etc.)
INCLUDES = -I$(top_srcdir)/header -DDEFAULT_AMULET_DIR=\"$(pkgdatadir)\"

if BUILD_release
RELEASE = libwidgets_objects.la
else
RELEASE =
endif

if BUILD_develop
DEVELOP = libwidgets_objects-develop.la
else
DEVELOP =
endif

noinst_LTLIBRARIES = $(RELEASE) $(DEVELOP)

## Widgets drawing lib (widgets/)

libwidgets_objects_la_SOURCES = \
  border_rect.cpp \
  button.cpp \
  button_panel.cpp \
  checkbox.cpp \
  checkbox_panel.cpp \
  menu.cpp \
  menu_bar.cpp \
  menu_item.cpp \
  option_button.cpp \
  popup_inter.cpp \
  radiobutton.cpp \
  radiobutton_panel.cpp \
  scrolling_group.cpp

LINK=$(CXXLINK) # Needed to force linking with c++

libwidgets_objects_develop_la_SOURCES = 
libwidgets_objects_develop_la_LIBADD = $(patsubst %.cpp,%-develop.lo,$(filter %.cpp,$(libwidgets_objects_la_SOURCES)))

# Don't touch these...
%-develop.lo: %.cpp
	$(LTCXXCOMPILE) @AMULET_DEVELOP_FLAGS@ -c $< -o $@
