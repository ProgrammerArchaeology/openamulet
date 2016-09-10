                    Readme file for OpenAmulet
---------------------------------------------------------------------
Contact: robert.m.muench@scrap.de
		 www.scrap.de/html/amulet.htm
---------------------------------------------------------------------

This file will give you some basic information how to build the
OpenAmulet library. There might exist an other readme file for
platform specific instrutions (like readme_win.txt, ...). Please have
a look if such a readme file exists for your platform and consider
the information provided.

To help you to easy alter the available makefiles, the file
'oa_lib_contents.txt' contains a list of all needed header and source
files. You can 'cut & paste' the list of source files to the
makefiles if this is needed (this could be the case where the
makefiles haven't been updated to reflect the newest changes of the
library source, that's because some makefiles are maintained by
volunteers).

The file 'changes.txt' gives you a short overview what has been
changed since the last release. It further gives you a good feeling
where you might have to change/adapt your current build system and if
you might have to change some of your application code.


Distributions:
--------------
OpenAmulet is distributed in several pieces. One distributions
contains the source code for the library. This distribution contains
the sources for ALL supported platforms.

Than you need a makefile for your platform / compiler combination.
For some platforms there exist more than one makefile. Choose the one
which you like most. Most of the makefiles are maintained by other
people and not directly by me. If you have problems, the best way is
to ask on the mailing-list or directly contact the author. We are
working hard to keep everything in sync and ensure that all build
systems work as expected.


Configuration:
--------------
The include file 'amulet.h' located in the .../header/ directory,
contains several #defines by which you can configure/alter the
behaviour of OpenAmulet. Mostly these defines are used to
enable/disable special C++ features.

Each available define has a comment describing what it does.


Makefile-Systems:
-----------------
As allready said OpenAmulet provides several makefile systems. This
is done to provide the user of OpenAmulet the most common makefile
system for his platform. We don't have a global & general makefile
hierarchy etc. which must be used on all platforms. Even this step
results in a higher maintainance we think it's the best way to keep
the platform feeling the user allready know.


SmartHeap support:
------------------
As I'm using SmartHeap for my memory-management, I have added
SmartHeap support to OpenAmulet. SmartHeap is a commercial
memory-management tool. If you don't have it, you can't use the
SmartHeap version of OpenAmulet. All SmartHeap version contain the
strings 'HA' or 'SH' to indicate them.

Please take care if you are using project makefile systems like the
one for VC++, where you can select which version to build. Select the
one not using SmartHeap.

If you want to use SmartHeap, provide the USE_SMARTHEAP define on the
command-line of your compiler. This define is although included into
'amulet.h' but is commented by default.


Compiler requirements:
----------------------
If you want to use the OpenAmulet version (OA_VERSION is defined),
you will need a compiler which provides a STL. Depending on the
features you want to have enabled in OpenAmulet, you might need a
quite modern C++ compiler which supports things like namespaces, RTTI
etc. Have a look at the available defines in 'amulet.h' to see what
you can enable/disable and what kind of compiler feature you might
need for this.


And now have fun with OpenAmulet and let us know what you do with it.
Robert M. Muench
