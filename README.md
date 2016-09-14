# Amulet User Interface Development Environment

[![Build Status](https://travis-ci.org/amulet-ui/openamulet.svg?branch=master)](https://travis-ci.org/amulet-ui/openamulet)

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
this repository:

* `git clone https://github.com/amulet-ui/openamulet.git`
* `cd openamulet`
* `mkdir build`
* `cd build`
* `cmake .. -G Ninja`
* `ninja`
* `./samples/checkers`

You can use other build tools instead of `ninja`. Other
samples or tests can be run, but some require assets
which can be found in the `data` directory. To help
the Amulet applications find these assets, set the
`AMULET_DIR` environment variable to point to that
`data` directory:

* `export AMULET_DIR=/path/to/openamulet/data`
