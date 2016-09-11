# We define here the default location for OpenAmulet data
# files (general images, default gestures classification, etc.)
INCLUDES = -I$(top_srcdir)/header -DDEFAULT_AMULET_DIR=\"$(pkgdatadir)\"

if BUILD_release
RELEASE = libanim.la
else
RELEASE =
endif

if BUILD_develop
DEVELOP = libanim-develop.la
else
DEVELOP =
endif

noinst_LTLIBRARIES = $(RELEASE) $(DEVELOP)

## Animators (anim/)

libanim_la_SOURCES =		\
	anim_constraint.cpp	\
	anim_interactor.cpp	\
	anim_animators.cpp

LINK=$(CXXLINK) # Needed to force linking with c++

libanim_develop_la_SOURCES = 
libanim_develop_la_LIBADD = $(patsubst %.cpp,%-develop.lo,$(filter %.cpp,$(libanim_la_SOURCES)))

# Don't touch these...
%-develop.lo: %.cpp
	$(LTCXXCOMPILE) @AMULET_DEVELOP_FLAGS@ -c $< -o $@
