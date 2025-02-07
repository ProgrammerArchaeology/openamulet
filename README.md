# Amulet User Interface Development Environment

[![CMake](https://github.com/ProgrammerArchaeology/openamulet/actions/workflows/build.yml/badge.svg)](https://github.com/ProgrammerArchaeology/openamulet/actions/)

Amulet includes many features specifically designed
to make the creation of highly-interactive, graphical,
direct manipulation user interfaces significantly easier,
including a prototype-instance object model, constraints,
high-level input handling including automatic undo,
built-in support for animation and gesture-recognition,
and a full set of widgets.

Amulet was originally developed in the 1990s at CMU:

http://www.cs.cmu.edu/~amulet/

## Building

Amulet can be built via `cmake`. Once you've cloned
this repository::

    git clone https://github.com/amulet-ui/openamulet.git
    cd openamulet
    mkdir build
    cd build
    cmake .. -G Ninja
    ninja

You can use other build tools instead of `ninja`.

Other samples or tests can be run, but some require assets
which can be found in the `data` directory. To help
the Amulet applications find these assets, set the
`AMULET_DIR` environment variable to point to that
`data` directory::

    export AMULET_DIR=/path/to/openamulet/data

And then run some samples::

    ./samples/checkers

## Platform Support

This build of Amulet has been used on macOS with
an X server installed. It doesn't have support for
native Mac GUIs.

It is also known to build on Linux and will probably
be easy to get working on other Unix platforms.

It has, in the past, run on Microsoft Windows. It
hopefully won't be hard to get that working again.

## Contributions

Contributions are welcome and encouraged.
