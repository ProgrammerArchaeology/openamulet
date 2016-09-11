# We define here the default location for OpenAmulet data
# files (general images, default gestures classification, etc.)
INCLUDES = -I$(top_srcdir)/header -DDEFAULT_AMULET_DIR=\"$(pkgdatadir)\"

if BUILD_develop
DEVELOP = libinspector.la
else
DEVELOP =
endif

noinst_LTLIBRARIES = $(DEVELOP)

AM_CPPFLAGS=-DDEBUG
## Inspector system, only built with DEBUG (debug/)

libinspector_la_SOURCES =	\
	inspector.cpp		\
	tracer.cpp		\
	input_parser.cpp
