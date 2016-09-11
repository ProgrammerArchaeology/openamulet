# We define here the default location for OpenAmulet data
# files (general images, default gestures classification, etc.)
INCLUDES = -I$(top_srcdir)/header -DDEFAULT_AMULET_DIR=\"$(pkgdatadir)\"

if BUILD_release
RELEASE = libgesture.la
else
RELEASE =
endif

if BUILD_develop
DEVELOP = libgesture-develop.la
else
DEVELOP =
endif

noinst_LTLIBRARIES = $(RELEASE) $(DEVELOP)

## Gesture recognition (gesture/)

libgesture_la_SOURCES =		\
	gest_classifier.cpp	\
	gest_features.cpp	\
	gest_matrix.cpp		\
	gest_trainer.cpp	\
	inter_gesture.cpp

LINK=$(CXXLINK) # Needed to force linking with c++

libgesture_develop_la_SOURCES = 
libgesture_develop_la_LIBADD = $(patsubst %.cpp,%-develop.lo,$(filter %.cpp,$(libgesture_la_SOURCES)))

# Don't touch these...
%-develop.lo: %.cpp
	$(LTCXXCOMPILE) @AMULET_DEVELOP_FLAGS@ -c $< -o $@
