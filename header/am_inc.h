//
// ------------------------------------------------------------------------
// 		The OpenAmulet User Interface Development Environment
// ------------------------------------------------------------------------
// This code is based on the Amulet project of Carnegie Mellon University,
// and has been placed in the public domain.  If you are using this code
// or any part of OpenAmulet, please contact amulet@cs.cmu.edu to be put
// on the mailing list or have a look at:
// http://www.openip.org
//

// An include file of include files.  This defines machine specific names
// for each of the include files in the amulet include directory.

#ifndef AM_INCLUDES_H
#define AM_INCLUDES_H

#include "oa_dl_import.h"
#include "am_config.h"

#define AM_IO__H <amulet/am_io.h>
#define ANIM__H <amulet/anim.h>
#define AM_VALUE__H <amulet/am_value.hpp>
#define DEBUGGER__H <amulet/debugger.h>
#define DYNARRAY__H <amulet/dynarray.h>
#define FORMULA__H <amulet/formula.h>
#define GDEFS__H <amulet/gdefs.h>
#define GEM__H <amulet/gem.h>
#define GESTURE__H <amulet/gesture.h>
#define IDEFS__H <amulet/idefs.h>
#define INTER__H <amulet/inter.h>
#define MISC__H <amulet/misc.h>
#define OBJECT__H <amulet/object.h>
#define OPAL__H <amulet/opal.h>
#define REGISTRY__H <amulet/registry.h>
#define RICH_TEXT__H <amulet/rich_text.h>
#define SCRIPTING__H <amulet/scripting.h>
#define STDVALUE__H <amulet/stdvalue.h>
#define STR_STREAM__H <amulet/am_strstream.h>
#define TEXT_FNS__H <amulet/text_fns.h>
#define TYPES__H <amulet/types.h>
#define UNIV_LST__H <amulet/univ_lst.h>
#define UNIV_MAP__H <amulet/univ_map.h>
#define UNIV_MAP__TPL <amulet/univ_map.hpp>
#define WEB__H <amulet/web.h>
#define WIDGETS__H <amulet/widgets.h>
#define INITIALIZER__H <amulet/initializer.h>
#define NETWORK__H <amulet/network.h>
#define CONNECTION__H <amulet/connection.h>
#define SOCKET_STUBS__H <amulet/socket_stubs.h>
#define AMULET_NETWORKS 1 // this includes NETWORK__H in amulet.h

#define REGISTERED_TYPE__HPP <amulet/registered_type.hpp>
#define TYPEDEFS__HPP <amulet/typedefs.hpp>
#define AM_WRAPPER__HPP <amulet/am_wrapper.hpp>

#define FORMULA_ADVANCED__H <amulet/formula_advanced.h>
#define WIDGETS_ADVANCED__H <amulet/widgets_advanced.h>
#define SCRIPTING_ADVANCED__H <amulet/scripting_advanced.h>
#define VALUE_LIST__H <amulet/value_list.h>
#define UNDO_DIALOG__H <amulet/undo_dialog.h>
#define SYMBOL_TABLE__H <amulet/symbol_table.h>
#define STANDARD_SLOTS__H <amulet/standard_slots.h>
#define PRIORITY_LIST__H <amulet/priority_list.h>
#define OPAL_ADVANCED__H <amulet/opal_advanced.h>
#define OBJECT_ADVANCED__H <amulet/object_advanced.h>
#define INTER_ADVANCED__H <amulet/inter_advanced.h>
#define GEM_IMAGE__H <amulet/gem_image.h>

// Really Unix only
#define GEMX__H <amulet/gemX.h>

#endif // AM_INCLUDES_H
