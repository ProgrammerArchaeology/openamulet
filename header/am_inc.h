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

#if !defined(_MACINTOSH)
// These are common to Unix and the PC
#define AM_IO__H		<amulet/am_io.h>
#define ANIM__H			<amulet/anim.h>
#define AM_VALUE__H		<amulet/am_value.hpp>
#define DEBUGGER__H 	<amulet/debugger.h>
#define DYNARRAY__H 	<amulet/dynarray.h>
#define FORMULA__H 		<amulet/formula.h>
#define GDEFS__H 		<amulet/gdefs.h>
#define GEM__H 			<amulet/gem.h>
#define GESTURE__H 		<amulet/gesture.h>
#define IDEFS__H 		<amulet/idefs.h>
#define INTER__H 		<amulet/inter.h>
#define MISC__H 		<amulet/misc.h>
#define OBJECT__H 		<amulet/object.h>
#define OPAL__H 		<amulet/opal.h>
#define REGISTRY__H 	<amulet/registry.h>
#define RICH_TEXT__H 	<amulet/rich_text.h>
#define SCRIPTING__H 	<amulet/scripting.h>
#define STDVALUE__H 	<amulet/stdvalue.h>
#define STR_STREAM__H 	<amulet/am_strstream.h>
#define TEXT_FNS__H 	<amulet/text_fns.h>
#define TYPES__H 		<amulet/types.h>
#define UNIV_LST__H 	<amulet/univ_lst.h>
#define UNIV_MAP__H 	<amulet/univ_map.h>
#define UNIV_MAP__TPL 	<amulet/univ_map.hpp>
#define WEB__H 			<amulet/web.h>
#define WIDGETS__H 		<amulet/widgets.h>
#define INITIALIZER__H 	<amulet/initializer.h>
#define NETWORK__H 		<amulet/network.h>
#define CONNECTION__H 	<amulet/connection.h>
#define SOCKET_STUBS__H <amulet/socket_stubs.h>
#define AMULET_NETWORKS 1 // this includes NETWORK__H in amulet.h

#define REGISTERED_TYPE__HPP	<amulet/registered_type.hpp>
#define TYPEDEFS__HPP			<amulet/typedefs.hpp>
#define AM_WRAPPER__HPP			<amulet/am_wrapper.hpp>
#endif

#if !defined(_MACINTOSH) & !defined(SHORT_NAMES)
// These are Unix or Windows with long-filenames ONLY
#define FORMULA_ADVANCED__H 	<amulet/formula_advanced.h>
#define WIDGETS_ADVANCED__H 	<amulet/widgets_advanced.h>
#define SCRIPTING_ADVANCED__H 	<amulet/scripting_advanced.h>
#define VALUE_LIST__H 			<amulet/value_list.h>
#define UNDO_DIALOG__H 			<amulet/undo_dialog.h>
#define SYMBOL_TABLE__H 		<amulet/symbol_table.h>
#define STANDARD_SLOTS__H 		<amulet/standard_slots.h>
#define PRIORITY_LIST__H 		<amulet/priority_list.h>
#define OPAL_ADVANCED__H 		<amulet/opal_advanced.h>
#define OBJECT_ADVANCED__H 		<amulet/object_advanced.h>
#define INTER_ADVANCED__H 		<amulet/inter_advanced.h>
#define GEM_IMAGE__H 			<amulet/gem_image.h>

// Really Unix only
#define GEMX__H 				<amulet/gemX.h>
#define GEMG__H         <amulet/gemG.h>
#endif

#if defined (SHORT_NAMES)
// These are Windows ONLY
#define FORMULA_ADVANCED__H 	<amulet/form_a.h>
#define WIDGETS_ADVANCED__H 	<amulet/widgts_a.h>
#define SCRIPTING_ADVANCED__H 	<amulet/scripadv.h>
#define VALUE_LIST__H 			<amulet/val_lst.h>
#define UNDO_DIALOG__H 			<amulet/undo_dia.h>
#define SYMBOL_TABLE__H 		<amulet/symb_tbl.h>
#define STANDARD_SLOTS__H 		<amulet/std_slot.h>
#define PRIORITY_LIST__H 		<amulet/prty_lst.h>
#define OPAL_ADVANCED__H 		<amulet/opal_a.h>
#define OBJECT_ADVANCED__H 		<amulet/object_a.h>
#define INTER_ADVANCED__H 		<amulet/inter_a.h>
#define GEM_IMAGE__H 			<amulet/gem_imag.h>
#endif

#if defined(_MACINTOSH)
// These are Macintosh ONLY
#define AM_IO__H 				<am_io.h>
#define ANIM__H 				<anim.h>
#define DEBUGGER__H 			<debugger.h>
#define DYNARRAY__H 			<dynarray.h>
#define FORMULA__H 				<formula.h>
#define FORMULA_ADVANCED__H 	<formula_advanced.h>
#define GDEFS__H 				<gdefs.h>
#define GEM__H 					<gem.h>
#define GEM_IMAGE__H 			<gem_image.h>
#define GESTURE__H 				<gesture.h>
#define IDEFS__H 				<idefs.h>
#define INTER__H 				<inter.h>
#define INTER_ADVANCED__H 		<inter_advanced.h>
#define MISC__H 				<misc.h>
#define OBJECT__H 				<object.h>
#define OBJECT_ADVANCED__H 		<object_advanced.h>
#define OPAL__H 				<opal.h>
#define OPAL_ADVANCED__H 		<opal_advanced.h>
#define PRIORITY_LIST__H 		<priority_list.h>
#define REGISTRY__H 			<registry.h>
#define RICH_TEXT__H 			<rich_text.h>
#define SCRIPTING__H 			<scripting.h>
#define SCRIPTING_ADVANCED__H 	<scripting_advanced.h>
#define STANDARD_SLOTS__H 		<standard_slots.h>
#define STDVALUE__H 			<stdvalue.h>
#define STR_STREAM__H 			<am_strstream.h>
#define SYMBOL_TABLE__H 		<symbol_table.h>
#define TEXT_FNS__H 			<text_fns.h>
#define TYPES__H 				<amulet/types.h>
#define UNDO_DIALOG__H 			<undo_dialog.h>
#define UNIV_LST__H 			<univ_lst.h>
#define UNIV_MAP__H 			<univ_map.h>
#define UNIV_MAP__TPL 			<univ_map.hpp>
#define VALUE_LIST__H 			<value_list.h>
#define WEB__H 					<web.h>
#define WIDGETS__H 				<widgets.h>
#define INITIALIZER__H 			<initializer.h>
#define WIDGETS_ADVANCED__H 	<widgets_advanced.h>

	#ifdef AMULET_NETWORKS
		#define NETWORK__H 		<network.h>
		#define CONNECTION__H 	<connection.h>
		#define NETMAP__H 		<netmap.h>
		#define B_ORDER__H 		<b_order.h>
		#define SOCKET_STUBS__H <socket_stubs.h> // not used, these are defined in GUSI
	#endif // AMULET_NETWORKS
#endif // _MACINTOSH

#endif // AM_INCLUDES_H
