SUBDIRS = gem object opal inter utils debug widgets anim gesture scripting net .

if BUILD_release
RELEASE = libamulet.la
else
RELEASE =
endif

if BUILD_develop
DEVELOP = libamulet-develop.la
else
DEVELOP =
endif

#
#  Real automake part (short, no?)
#

# All variants of the library
lib_LTLIBRARIES = $(RELEASE) $(DEVELOP)

# Main (and essential) variant
libamulet_la_SOURCES = dummy.cc
libamulet_la_LDFLAGS = -version-info 4:0:0
libamulet_la_LIBADD = gem/libgem.la object/libore.la opal/libopal.la \
inter/libinter.la utils/libutils.la widgets/libwidgets.la anim/libanim.la \
gesture/libgesture.la scripting/libscripting.la net/libnet.la

# Development variant (with inspector)
libamulet_develop_la_SOURCES = dummy.cc
libamulet_develop_la_LDFLAGS = -version-info 4:0:0
libamulet_develop_la_LIBADD = debug/libinspector.la \
gem/libgem-develop.la object/libore-develop.la opal/libopal-develop.la \
inter/libinter-develop.la utils/libutils-develop.la \
widgets/libwidgets-develop.la anim/libanim-develop.la \
gesture/libgesture-develop.la scripting/libscripting-develop.la \
net/libnet-develop.la
