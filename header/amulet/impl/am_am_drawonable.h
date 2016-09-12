//#include <amulet/am_value.hpp>
//#include <amulet/am_io.h>
#include "am_drawonable.h"
#include "types_pointer.h"

// Defines Am_Am_Drawonable which can be used to wrap
// Am_Drawonable in slots to make them more debuggable.
// Implementation is in gem_keytrans since is machine independent

Am_Define_Pointer_Wrapper(Am_Drawonable)
