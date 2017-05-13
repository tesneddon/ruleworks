/*
 *	rts_wm_update.c - module to update Working Memory Objects
 */
/****************************************************************************
 RuleWorks - Rules based application development tool.
 
 Copyright (C) 1999  Compaq Computer Corporation
 
 This program is free software; you can redistribute it and/or modify it 
 under the terms of the GNU General Public License as published by the 
 Free Software Foundation; either version 2 of the License, or any later 
 version. 
 
 This program is distributed in the hope that it will be useful, but 
 WITHOUT ANY WARRANTY; without even the implied warranty of 
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General 
 Public License for more details. 
 
 You should have received a copy of the GNU General Public License along 
 with this program; if not, write to the Free Software Foundation, 
 Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 
 Email: info@ruleworks.co.uk

****************************************************************************/

/*
**  FACILITY:
**	RULEWORKS compiler
**
**  ABSTRACT:
**	
**
**  MODIFIED BY:
**	DEC	Digital Equipment Corporation
**	CPQ	Compaq Computer Corporation
**
**  MODIFICATION HISTORY:
**
**	11-Aug-1992	DEC	Initial version
**	 1-Dec-1992	DEC	Changes for WMH (debugger)
**	 2-Dec-1992	DEC	Changes for in specialize
**	23-Dec-1992	DEC	Changes for wbreak
**	26-Aug-1994	DEC	Avoid ACCVIO in rul__wm_set_var_attr_val()
**					when passed NULL WME.
**	16-Feb-1998	DEC	class type changed to rclass
**	01-Dec-1999	CPQ	Release with GPL
*/

#include <common.h>		/* declares types and includes headers	*/
#include <rts_msg.h>		/* declares Run-Time-System messages    */
#include <cli_msg.h>		/* defn Command-Language-Interface msgs */
#include <cmp_msg.h>		/* declares *COMPILER?* messages        */
#include <msg.h>		/* declares message functions           */
#include <mol.h>		/* declares molecule functions  	*/
#include <decl.h>		/* declares most class functions	*/
#include <decl_att.h>		/* declares attr offset info     	*/
#include <wm.h>			/* Working Memory function declarations	*/
#include <wm_p.h>		/* Working Memory type definitions     	*/
#include <list.h>		/* declares list functions		*/
#include <delta.h>		/* declares delta_queue			*/
#include <rac.h>		/* declares Entry_data struct           */
#include <rbs.h>		/* declares "Ruling-Block" interface	*/
#include <ios.h>		/* declares IO_Stream stuff		*/
#include <dbg.h>		/* declares debugger stuff		*/


/* Macro Definitions: */

#define RUL_MEM_COPY(to_addr, from_addr, num_bytes) \
	memcpy(to_addr, from_addr, num_bytes)

#define WM_GET_OBJECT_SIZE(num_attrs) \
	(sizeof(struct object) \
	 + ((num_attrs - DECL_NUM_BUILTIN_ATTRS) * sizeof(Molecule)))

#define WM_ALLOCATE_OBJECT(num_attrs) \
	rul__mem_malloc(WM_GET_OBJECT_SIZE(num_attrs))



/* future EXTERNAL Variable Declarations: */

/* I'm not sure which subsystem will own these variables. */

EXTERNAL Mol_Symbol	rul__rac_active_rule_name;
EXTERNAL long		rul__rac_rule_firing_cycle;
static   Boolean        rul__dbg_mod_match;

 /* room to store #id */
#define MAX_WBREAK_MSG_LEN 100
static   char           wbreak_msg[MAX_WBREAK_MSG_LEN] = "";
static   long           wbreak_msg_idx;



/* GLOBAL Variable Declarations: */



/* Static Variable Declarations: */

static long		wm_cur_time_tag;



static Boolean check_wmo_attr_value (Object wmo,
				     long attr_offset,
				     Molecule value);

static Boolean check_comp_attr_value (Object wmo,
				      Mol_Symbol attr,
				      Molecule value,
				      Molecule comp_index);

static Mol_Symbol *
wm_alloc_hist (Object wmo)
{
  Mol_Symbol *attr_rules;
  long        num_attrs;
  long        i;

  num_attrs = rul__decl_get_class_num_attrs (WM_GET_OBJ_CLASS(wmo));
  attr_rules = RUL_MEM_ALLOCATE (Mol_Symbol, num_attrs);
  for (i = 0; i < num_attrs; i++) {
    attr_rules[i] = MOL_NIL;
  }
  return attr_rules;
}

static Object
wm_create_object (Class rclass,
		  long num_attrs,
		  long cycle,
		  Mol_Instance_Id instance_id,
		  long time_tag,
		  Molecule *attr_vals)/* everything except $ID */
{
  register Object wmo;
  register long attr_num;

  if (rul__rac_active_rule_name == NULL)
    rul__rac_active_rule_name = rul__mol_symbol_nil ();

  wmo = WM_ALLOCATE_OBJECT (num_attrs);

  WM_SET_OBJ_LIST	(wmo, NULL);
  WM_SET_OBJ_CLASS    	(wmo, rclass);
  WM_SET_OBJ_NUM_ATTRS	(wmo, num_attrs);
  WM_SET_OBJ_CYCLE    	(wmo, cycle);
  WM_SET_OBJ_CHANGES    (wmo, 1);
  WM_SET_OBJ_TIME_TAG	(wmo, time_tag);
  WM_SET_OBJ_IDENTIFIER	(wmo, instance_id);
  WM_SET_OBJ_ATTR_RULES (wmo, NULL);
  WM_SET_OBJ_ATTR_RBS   (wmo, NULL);
  WM_SET_OBJ_MOD_RULE   (wmo, rul__rac_active_rule_name);
  WM_SET_OBJ_MOD_RB     (wmo, NULL);

  RUL_MEM_COPY (&WM_GET_OBJ_ATTR_VAL(wmo, DECL_INSTANCE_OF_ATTR_OFFSET),
		 attr_vals,
		 ((num_attrs - 1) * sizeof(Molecule)));	/* don't include $ID */

  for (attr_num = DECL_INSTANCE_OF_ATTR_OFFSET;
       attr_num < num_attrs;
       attr_num++)
    rul__mol_incr_uses (WM_GET_OBJ_ATTR_VAL(wmo, attr_num));

  return wmo;
}



static void
wm_destroy_object (register Object wmo)
{
  long num_attrs;
  register long attr_num;

  num_attrs = WM_GET_OBJ_NUM_ATTRS (wmo);

  if (! WM_IS_OBJ_SNAPSHOT(wmo)) {
    rul__mol_set_instance_id_value (WM_GET_OBJ_IDENTIFIER(wmo), NULL);
  }

  for (attr_num = DECL_ID_ATTR_OFFSET; attr_num < num_attrs; attr_num++) {
    rul__mol_decr_uses (WM_GET_OBJ_ATTR_VAL (wmo, attr_num));
  }

  /* note that we don't have to decrement the reference count on
     rule names since they are guaranteed to be permanent atoms */

  if (WM_GET_OBJ_ATTR_RULES (wmo) != NULL) {
    rul__mem_free (WM_GET_OBJ_ATTR_RULES (wmo));
    rul__mem_free (WM_GET_OBJ_ATTR_RBS (wmo));
  }

  rul__mem_free (wmo);
}



Object
rul__wm_create_var (Mol_Symbol class_mol, Entry_Data eb_data)
{
  Class class_id;

  class_id = rul__decl_get_visible_class_rt (class_mol, eb_data->db_names,
					     eb_data->db_name_count);
  if (class_id)
    return rul__wm_create (class_id, NULL, eb_data);

  rul__msg_print_w_atoms (RTS_NOSUCHCLS, 1, class_mol);
  return NULL;
}



Object
rul__wm_create (Class class_id, Mol_Instance_Id opt_id,
		Entry_Data eb_data)
{
  long            num_attrs;
  Object          wmo;		/* a ptr to the Working Memory Object created*/
  List            list;
  Mol_Instance_Id new_id = opt_id;
  Molecule        args[2];

  if (opt_id != NULL) {
    if (rul__mol_is_instance_id (opt_id)) {
      if (rul__mol_instance_id_value (opt_id)) {
	rul__msg_print_w_atoms (RTS_MAKWMEEXI, 1, opt_id);
	return NULL;
      }
    }
    else {
      rul__msg_print_w_atoms (RTS_INVWMOID, 1, opt_id);
      return NULL;
    }
  }

  if (rul__decl_get_class_name (class_id) == rul__mol_symbol_root ()) {
    rul__msg_print (RTS_INVMAKCLS);
    return NULL;
  }

  num_attrs = rul__decl_get_class_num_attrs (class_id);

  if (new_id == NULL)
    new_id = rul__mol_make_instance_id (NULL);

  wmo = wm_create_object(class_id,
			 num_attrs,
			 rul__rac_rule_firing_cycle,
			 new_id,
			 ++wm_cur_time_tag,
			 rul__decl_get_class_default (class_id));

  WM_SET_OBJ_OPER (wmo, WM__OPR_MAKE);
  rul__mol_set_instance_id_value (WM_GET_OBJ_IDENTIFIER (wmo), wmo);

  if (rul__dbg_gl_enable & DBG_M_ENABLE_DBG) {
    if (rul__dbg_gl_enable & DBG_M_ENABLE_WMH) {
      WM_SET_OBJ_ATTR_RULES (wmo, wm_alloc_hist (wmo));
      WM_SET_OBJ_ATTR_RBS (wmo, wm_alloc_hist (wmo));
      WM_SET_OBJ_CRE_RULE (wmo, rul__rac_active_rule_name);
      if (eb_data != NULL)
	WM_SET_OBJ_CRE_RB (wmo, eb_data->active_rb_name);
    }
  }

  list = rul__decl_get_block_wmo_list (rul__decl_get_class_block (class_id));

  WM_SET_OBJ_LIST (wmo, rul__list_add_last (list, wmo));

  /* check for system method call... */
  if ((eb_data != NULL) &&
      (rul__decl_get_class_sys_methods (class_id) & (1 << BEFORE_MAKE))) {
    args [0] = new_id;
    args [1] = rul__rac_active_rule_name;
    rul__call_method (rul__decl_get_sys_meth_name (BEFORE_MAKE),
		      2, args, NULL, eb_data);
  }

  return wmo;
}



Object
rul__wm_copy_id (Mol_Instance_Id wid, Entry_Data eb_data)
{
  Mol_Symbol class_mol;
  Object wmo;

  if (rul__mol_is_instance_id (wid)) {
    wmo = rul__mol_instance_id_value (wid);
    if (wmo) {
      if (rul__decl_is_class_visible_rt (WM_GET_OBJ_CLASS (wmo),
					  eb_data->db_names,
					  eb_data->db_name_count)) {
	return (rul__wm_copy (wmo, eb_data));
      } else {
	class_mol = rul__decl_get_class_name (WM_GET_OBJ_CLASS (wmo));
	rul__msg_print_w_atoms (RTS_NOTINSCOPE, 1, class_mol);
      }
    }
    else
      rul__msg_print_w_atoms (RTS_NOSUCHWM, 1, wid);
  }
  else
    rul__msg_print_w_atoms (RTS_NOSUCHID, 1, wid);

  return NULL;
}


Object
rul__wm_copy (Object old_wmo, Entry_Data eb_data)
{
  Class      class_id;
  Object     new_wmo;
  List       list;
  Molecule   args[2];

  if (WM_GET_OBJ_CHANGES (old_wmo) != 0) {
    WM_SET_OBJ_CHANGES (old_wmo, WM_GET_OBJ_CHANGES (old_wmo) + 1);
    rul__msg_print_w_atoms (RTS_INVWMONES, 1, WM_GET_OBJ_IDENTIFIER (old_wmo));
    return old_wmo;
  }

  class_id	= WM_GET_OBJ_CLASS (old_wmo);
  new_wmo	= wm_create_object (class_id,
				    WM_GET_OBJ_NUM_ATTRS (old_wmo),
				    rul__rac_rule_firing_cycle,
				    rul__mol_make_instance_id (NULL),
				    ++wm_cur_time_tag,
				    &WM_GET_OBJ_ATTR_VAL (old_wmo,
					  DECL_INSTANCE_OF_ATTR_OFFSET));

  WM_SET_OBJ_OPER (new_wmo, WM__OPR_COPY);
  rul__mol_set_instance_id_value (WM_GET_OBJ_IDENTIFIER (new_wmo), new_wmo);

  if (rul__dbg_gl_enable & DBG_M_ENABLE_DBG) {
    if (rul__dbg_gl_enable & DBG_M_ENABLE_WMH) {
      WM_SET_OBJ_ATTR_RULES (new_wmo, wm_alloc_hist (new_wmo));
      WM_SET_OBJ_ATTR_RBS (new_wmo, wm_alloc_hist (new_wmo));
      WM_SET_OBJ_CRE_RULE (new_wmo, rul__rac_active_rule_name);
      if (eb_data != NULL)
	WM_SET_OBJ_CRE_RULE (new_wmo, eb_data->active_rb_name);
    }
  }

  list = rul__decl_get_block_wmo_list (rul__decl_get_class_block (class_id));

  WM_SET_OBJ_LIST (new_wmo, rul__list_add_last (list, new_wmo));

  /* check for system method call... */
  if ((eb_data != NULL) &&
      (rul__decl_get_class_sys_methods (class_id) & (1 << BEFORE_MAKE))) {
    args [0] = WM_GET_OBJ_IDENTIFIER (new_wmo);
    args [1] = rul__rac_active_rule_name;
    rul__call_method (rul__decl_get_sys_meth_name (BEFORE_MAKE),
		      2, args, NULL, eb_data);
  }

  return new_wmo;
}



static void
wm_make_object (Object wmo)
{
  /* propogate object addition through RETE net*/
  rul__rbs_notify_wm_delta (wmo, DELTA__C_SIGN_POSITIVE);
}

static void
wm_remove_object (Object wmo)
{
  rul__rbs_notify_wm_delta (wmo, DELTA__C_SIGN_NEGATIVE);
}



void
rul__wm_destroy_and_notify_id (Mol_Instance_Id wid, Entry_Data eb_data)
{
  Mol_Symbol class_mol;
  Object wmo;

  if (rul__mol_is_instance_id (wid)) {
    wmo = rul__mol_instance_id_value (wid);
    if (wmo) {
      if (rul__decl_is_class_visible_rt (WM_GET_OBJ_CLASS (wmo),
					 eb_data->db_names,
					 eb_data->db_name_count)) {
	rul__wm_destroy_and_notify (wmo, eb_data);
      } else {
        class_mol = rul__decl_get_class_name (WM_GET_OBJ_CLASS (wmo));
	rul__msg_print_w_atoms (RTS_NOTINSCOPE, 1, class_mol);
      }
    }
    else
      rul__msg_print_w_atoms (RTS_NOSUCHWM, 1, wid);
  }
  else
    rul__msg_print_w_atoms (RTS_NOSUCHID, 1, wid);

  return;
}


void
rul__wm_destroy_and_notify (Object wmo, Entry_Data eb_data)
{
  Molecule   args[2];
  Class      class_id;

  if (WM_GET_OBJ_CHANGES (wmo) != 0) {
    rul__msg_print_w_atoms (RTS_INVWMONES, 1, WM_GET_OBJ_IDENTIFIER (wmo));
    return;
  }

  WM_SET_OBJ_OPER (wmo, WM__OPR_REMOVE);
  WM_SET_OBJ_CHANGES (wmo, WM_GET_OBJ_CHANGES (wmo) + 1);
  class_id = WM_GET_OBJ_CLASS (wmo);

  wm_remove_object (wmo);
  rul__list_remove_first (WM_GET_OBJ_LIST (wmo));

  /* debug watch on removes */
  if (rul__dbg_gl_enable & DBG_M_ENABLE_TRA) {
    if (rul__dbg_gl_trace & DBG_M_TRACE_WM) {
      if (rul__ios_curr_column (RUL__C_DEF_TRACE))
	rul__ios_print_newline (RUL__C_DEF_TRACE);
      rul__ios_printf (RUL__C_DEF_TRACE, "   <=WM:  ");
      rul__wm_print_readform (wmo, RUL__C_DEF_TRACE);
      rul__ios_print_newline (RUL__C_DEF_TRACE);
    }
  }

  /* if debugging enabled */
  if (rul__dbg_gl_enable & DBG_M_ENABLE_DBG) {

    /* if wbreaks set and this is one */
    if ((rul__dbg_ga_wbreak) && (*rul__dbg_ga_wbreak)(wmo)) {
      rul__dbg_gl_break |= DBG_M_BREAK_WMO;
      rul__mol_use_readform (WM_GET_OBJ_IDENTIFIER (wmo),
			     wbreak_msg, MAX_WBREAK_MSG_LEN);
      wbreak_msg_idx = rul__mol_get_readform_length (
					     WM_GET_OBJ_IDENTIFIER (wmo));
      wbreak_msg[wbreak_msg_idx++] = '\0';
      rul__msg_print (CLI_WBREAK, wbreak_msg);
    }
  }

  /* check for system method call... */
  if ((eb_data != NULL) &&
      (rul__decl_get_class_sys_methods (class_id) & (1 << BEFORE_REMOVE))) {
    args [0] = WM_GET_OBJ_IDENTIFIER (wmo);
    args [1] = rul__rac_active_rule_name;
    rul__call_method (rul__decl_get_sys_meth_name (BEFORE_REMOVE),
		      2, args, NULL, eb_data);
  }

  wm_destroy_object (wmo);
}

void
rul__wm_destroy_all_class_var (Mol_Symbol class_mol, Entry_Data eb_data)
{
  long i;
  Class rclass;

  if (class_mol == rul__mol_symbol_root ()) {
    for (i = 0; i < eb_data->db_name_count; i++) {
      if (rul__decl_block_has_classes (eb_data->db_names[i])) {
	rclass = rul__decl_get_block_classes (
			     rul__decl_get_block (eb_data->db_names[i]));
	while (rclass != NULL) {
	  rul__wm_destroy_and_notify_all (rclass, eb_data);
	  rclass = rul__decl_get_class_next (rclass);
	}
      }
    }
  }
  else {
    rclass = rul__decl_get_visible_class_rt (class_mol, eb_data->db_names,
					    eb_data->db_name_count);
    if (rclass)
      rul__wm_destroy_and_notify_all (rclass, eb_data);
  }
}

/*
 * special iterator for removes...!!!
 */
void
rul__wm_destroy_and_notify_all (Class class_id, Entry_Data eb_data)
{
  Object     wmo;
  Decl_Block block;
  List       list;

  for (block = rul__decl_get_first_decl_block ();
       block != NULL;
       block = rul__decl_get_next_decl_block (block)) {
      
    for (list = rul__decl_get_block_wmo_list (block);
	 list;
	 /* nothing here... */) {
	
      /* get wmo and check for empty list */
      if (!(wmo = (Object) rul__list_first (list)))
	break;
	
      /* get next list entry pointer before the destroy operation...  */
      list = rul__list_next (WM_GET_OBJ_LIST (wmo));
      
      if (!class_id || class_id == WM_GET_OBJ_CLASS (wmo))
	if (WM_GET_OBJ_CHANGES (wmo) == 0)
	  rul__wm_destroy_and_notify (wmo, eb_data);
    }
  }
}



Object
rul__wm_modify_id (Mol_Instance_Id wid, Entry_Data eb_data)
{
  Mol_Symbol class_mol;
  Object wmo;

  if (rul__mol_is_instance_id (wid)) {
    wmo = rul__mol_instance_id_value (wid);
    if (wmo) {
      if (rul__decl_is_class_visible_rt (WM_GET_OBJ_CLASS (wmo),
					 eb_data->db_names,
                                         eb_data->db_name_count)) {
	rul__wm_modify (wmo, eb_data);
	return wmo;
      } else {
        class_mol = rul__decl_get_class_name (WM_GET_OBJ_CLASS (wmo));
	rul__msg_print_w_atoms (RTS_NOTINSCOPE, 1, class_mol);
      }
    }
    else
      rul__msg_print_w_atoms (RTS_NOSUCHWM, 1, wid);
  }
  else
    rul__msg_print_w_atoms (RTS_NOSUCHID, 1, wid);
  
  return NULL;
}


void
rul__wm_modify (Object wmo, Entry_Data eb_data)
{
  Class      class_id;
  Molecule   args[2];

  WM_SET_OBJ_CHANGES (wmo, WM_GET_OBJ_CHANGES (wmo) + 1);
  if (WM_GET_OBJ_CHANGES (wmo) != 1) {
    rul__msg_print_w_atoms (RTS_INVWMONES, 1, WM_GET_OBJ_IDENTIFIER (wmo));
    return;
  }

  WM_SET_OBJ_OPER (wmo, WM__OPR_MODIFY);
  class_id = WM_GET_OBJ_CLASS (wmo);

  if (rul__dbg_gl_enable & DBG_M_ENABLE_TRA) {
    if (rul__dbg_gl_trace & DBG_M_TRACE_WM) {
      if (rul__ios_curr_column (RUL__C_DEF_TRACE))
	rul__ios_print_newline (RUL__C_DEF_TRACE);
      rul__ios_printf (RUL__C_DEF_TRACE, "   <-WM:  ");
      rul__wm_print_readform (wmo, RUL__C_DEF_TRACE);
      rul__ios_print_newline (RUL__C_DEF_TRACE);
    }
  }

  /* if debugging enabled */
  if (rul__dbg_gl_enable & DBG_M_ENABLE_DBG) {
    if (rul__dbg_ga_wbreak)
      rul__dbg_mod_match = (*rul__dbg_ga_wbreak)(wmo);
  }

  wm_remove_object (wmo);

  WM_SET_OBJ_TIME_TAG (wmo, ++wm_cur_time_tag);

  /* check for system method call... */
  if ((eb_data != NULL) &&
      (rul__decl_get_class_sys_methods (class_id) & (1 << BEFORE_MODIFY))) {
    args [0] = WM_GET_OBJ_IDENTIFIER (wmo);
    args [1] = rul__rac_active_rule_name;
    rul__call_method (rul__decl_get_sys_meth_name (BEFORE_MODIFY),
		      2, args, NULL, eb_data);
  }
}



Object
rul__wm_specialize_var (Mol_Instance_Id wid, Mol_Symbol class_mol,
			Entry_Data eb_data)
{
  Class  class_id;

  class_id = rul__decl_get_visible_class_rt (class_mol, eb_data->db_names,
					     eb_data->db_name_count);
  if (class_id)
    return (rul__wm_specialize_id (wid, class_id, eb_data));

  rul__msg_print_w_atoms (RTS_NOTINSCOPE, 1, class_mol);
  return NULL;
}




Object
rul__wm_specialize_id (Mol_Instance_Id wid, Class new_class_id,
		       Entry_Data eb_data)
{
  Mol_Symbol class_mol;
  Object wmo;

  if (rul__mol_is_instance_id (wid)) {
    wmo = rul__mol_instance_id_value (wid);
    if (wmo) {
      if (rul__decl_is_class_visible_rt (WM_GET_OBJ_CLASS (wmo), 
					 eb_data->db_names,
					 eb_data->db_name_count)) {
	return (rul__wm_specialize (wmo, new_class_id, eb_data));
      } else {
        class_mol = rul__decl_get_class_name (WM_GET_OBJ_CLASS (wmo));
	rul__msg_print_w_atoms (RTS_NOTINSCOPE, 1, class_mol);
      }
    }
    else 
      rul__msg_print_w_atoms (RTS_NOSUCHWM, 1, wid);
  }
  else
    rul__msg_print_w_atoms (RTS_NOSUCHID, 1, wid);

  return NULL;
}



Object
rul__wm_specialize (Object old_wmo, Class new_class,
		    Entry_Data eb_data)
{
  register Object new_wmo;
  long            old_num_attrs;
  long            new_num_attrs;
  long            attr_num;
  Molecule       *default_attr_vals; /* initial values for each class */
  Mol_Symbol     *old_attr_rules;
  Mol_Symbol     *old_attr_rbs;
  Molecule        args[2];

  if (WM_GET_OBJ_CHANGES (old_wmo) != 0) {
    WM_SET_OBJ_CHANGES (old_wmo, WM_GET_OBJ_CHANGES (old_wmo) + 1);
    rul__msg_print_w_atoms (RTS_INVWMONES, 1, WM_GET_OBJ_IDENTIFIER (old_wmo));
    return old_wmo;
  }

  assert (rul__decl_is_subclass_of (new_class, WM_GET_OBJ_CLASS (old_wmo)));

  wm_remove_object (old_wmo);

  old_num_attrs = WM_GET_OBJ_NUM_ATTRS (old_wmo);
  new_num_attrs	= rul__decl_get_class_num_attrs (new_class);
  default_attr_vals = rul__decl_get_class_default (new_class);
    
  if (new_num_attrs > old_num_attrs) {
    new_wmo = rul__mem_realloc (old_wmo, WM_GET_OBJECT_SIZE (new_num_attrs));

    RUL_MEM_COPY (&WM_GET_OBJ_ATTR_VAL (new_wmo, old_num_attrs),
		   &default_attr_vals[old_num_attrs-1],/* don't include $ID */
		   ((new_num_attrs - old_num_attrs) * sizeof(Molecule)));

   for (attr_num = old_num_attrs; attr_num < new_num_attrs; attr_num++)
     rul__mol_incr_uses (WM_GET_OBJ_ATTR_VAL (new_wmo, attr_num));

    rul__mol_set_instance_id_value (WM_GET_OBJ_IDENTIFIER (new_wmo), new_wmo);
    WM_SET_OBJ_NUM_ATTRS (new_wmo, new_num_attrs);
  }
  else
    new_wmo = old_wmo;

  WM_SET_OBJ_CLASS (new_wmo, new_class);
  WM_SET_OBJ_CHANGES (new_wmo, WM_GET_OBJ_CHANGES (new_wmo) + 1);
  WM_SET_OBJ_OPER (new_wmo, WM__OPR_SPECIALIZE);
  WM_SET_OBJ_CLASS_NAME	(new_wmo,
			 default_attr_vals[DECL_INSTANCE_OF_ATTR_OFFSET-1]);

  WM_SET_OBJ_TIME_TAG (new_wmo, ++wm_cur_time_tag);

  if (new_num_attrs > old_num_attrs) {
    /*
     * if WMH was enabled when this object was create/modified
     * than allocate a new array for rule pointers
     * and set old values 
     */

    if ((old_attr_rules = WM_GET_OBJ_ATTR_RULES (old_wmo)) != NULL) {
      old_attr_rbs = WM_GET_OBJ_ATTR_RBS (old_wmo);
      WM_SET_OBJ_ATTR_RULES (new_wmo, wm_alloc_hist (new_wmo));
      WM_SET_OBJ_ATTR_RBS (new_wmo, wm_alloc_hist (new_wmo));
      for (attr_num = 0; attr_num < old_num_attrs; attr_num++) {
	WM_SET_OBJ_ATTR_RULE (new_wmo, attr_num, old_attr_rules[attr_num]);
	WM_SET_OBJ_ATTR_RB (new_wmo, attr_num, old_attr_rbs[attr_num]);
      }
      rul__mem_free (old_attr_rules);
      rul__mem_free (old_attr_rbs);
    }
  }

  rul__list_remove_first (WM_GET_OBJ_LIST (new_wmo));

  WM_SET_OBJ_LIST (new_wmo,
    rul__list_add_last (rul__decl_get_block_wmo_list (
			      rul__decl_get_class_block (new_class)),
			new_wmo));

  /* check for system method call... */
  if ((eb_data != NULL) &&
      (rul__decl_get_class_sys_methods (new_class) & (1 << BEFORE_MODIFY))) {
    args [0] = WM_GET_OBJ_IDENTIFIER (new_wmo);
    args [1] = rul__rac_active_rule_name;
    rul__call_method (rul__decl_get_sys_meth_name (BEFORE_MODIFY),
		      2, args, NULL, eb_data);
  }

  return new_wmo;
}



/********************************************************************/
/*              attribute value routines                            */
/********************************************************************/

void
rul__wm_set_offset_val_simple (Object wmo, long attr_offset,
			       Molecule attr_val, Entry_Data eb_data)
{
  Molecule           args[4];
  Class              class_id;
  System_Method_Type meth_type;
  Molecule           old_value;

  if (WM_GET_OBJ_CHANGES (wmo) != 1)
    return;

  class_id = WM_GET_OBJ_CLASS (wmo);
  old_value = WM_GET_OBJ_ATTR_VAL (wmo, attr_offset);
  WM_SET_OBJ_ATTR_VAL (wmo, attr_offset, attr_val);
  rul__mol_incr_uses (attr_val);

  /* if debugging enabled */
  if (rul__dbg_gl_enable & DBG_M_ENABLE_DBG) {
      
    if (rul__dbg_gl_enable & DBG_M_ENABLE_WMH) {
      if (!WM_GET_OBJ_ATTR_RULES (wmo)) {
	WM_SET_OBJ_ATTR_RULES (wmo, wm_alloc_hist (wmo));
	WM_SET_OBJ_ATTR_RBS (wmo, wm_alloc_hist (wmo));
      }
      WM_SET_OBJ_ATTR_RULE (wmo, attr_offset, rul__rac_active_rule_name);
      if (eb_data != NULL)
	WM_SET_OBJ_ATTR_RB (wmo, attr_offset, eb_data->active_rb_name);
    }
  }

  /* system method call... */
  if (rul__mol_is_atom (old_value))
    meth_type = ON_ATTR_MODIFY;
  else
    meth_type = ON_COMP_ATTR_MODIFY;

  if ((eb_data != NULL) &&
      (rul__decl_get_class_sys_methods (class_id) & (1 << meth_type))) {
    args [0] = WM_GET_OBJ_IDENTIFIER (wmo);
    args [1] = rul__rac_active_rule_name;
    args [2] = rul__decl_get_attr_name (class_id, attr_offset);
    args [3] = old_value;
    rul__call_method (rul__decl_get_sys_meth_name (meth_type),
		      4, args, NULL, eb_data);
  }
  rul__mol_decr_uses (old_value);
}

/********************************************************************/

void
rul__wm_set_offset_val (Object wmo, long attr_offset,
			Molecule attr_val, Entry_Data eb_data)
{
  /*
  **  First, check the value specified against any shape
  **  or domain restrictions this attribute may have.
  */
  if (check_wmo_attr_value (wmo, attr_offset, attr_val)) {
    rul__wm_set_offset_val_simple (wmo, attr_offset, attr_val, eb_data);
  }
}


/********************************************************************/

void
rul__wm_set_attr_val (Object wmo, Mol_Symbol attr_name,
		      Molecule attr_val, Entry_Data eb_data)
{
  rul__wm_set_offset_val (wmo,
		  rul__decl_get_attr_offset (WM_GET_OBJ_CLASS(wmo), attr_name),
			  attr_val, eb_data);
}



/********************************************************************/

void
rul__wm_set_var_attr_val (Object wmo, Mol_Symbol attr,
			  Molecule value, Entry_Data eb_data)
{
  /*
  **  Runtime check is needed to verify that the named attribute
  **  is defined in the instantiating class of the WMO, and that
  **  this is not a system read-only attribute
  */

  if (wmo == NULL)
    return;			/* Avoid ACCVIO */

  if (attr == rul__mol_symbol_id ()) {
    rul__msg_print (CMP_NOMODIFYID);
  }
  else if (attr == rul__mol_symbol_instance_of ()) {
    rul__msg_print (CMP_NOMODIFYCLASS);
  }
  else if (!rul__decl_is_attr_in_class (WM_GET_OBJ_CLASS (wmo), attr)) {
    rul__msg_print_w_atoms (RTS_INVWMOATT, 2, attr,
			    WM_GET_OBJ_IDENTIFIER (wmo));
  }
  else
    rul__wm_set_attr_val (wmo, attr, value, eb_data);
}

/********************************************************************/


void
rul__wm_set_comp_attr_val (Object wmo, Mol_Symbol attr,
			   Molecule value, Molecule comp_index,
			   Entry_Data eb_data)
{
  Molecule new_val;
  long     offset, idx;

  if (check_comp_attr_value (wmo, attr, value, comp_index)) {

    offset = rul__decl_get_attr_offset (WM_GET_OBJ_CLASS (wmo), attr);
    idx = rul__mol_int_atom_value (comp_index);
    new_val = rul__mol_set_comp_nth (rul__wm_get_offset_val (wmo, offset),
				     idx, value,
			     rul__decl_get_attr_fill (WM_GET_OBJ_CLASS (wmo),
						      attr));
    if (new_val) {
      rul__wm_set_offset_val (wmo, offset, new_val, eb_data);
      rul__mol_decr_uses (new_val);
    }
  }
}

/********************************************************************/

void
rul__wm_set_comp_attr_last (Object wmo, Mol_Symbol attr,
			    Molecule value, Entry_Data eb_data)
{
  Molecule new_val, old_comp;
  long     offset, last_pos;

  if (check_comp_attr_value (wmo, attr, value, NULL)) {

    offset = rul__decl_get_attr_offset (WM_GET_OBJ_CLASS (wmo), attr);
    old_comp = rul__wm_get_offset_val (wmo, offset);
    last_pos = rul__mol_get_poly_count_last (old_comp);
    new_val = rul__mol_set_comp_nth (old_comp, last_pos, value,
		    rul__decl_get_attr_fill (WM_GET_OBJ_CLASS (wmo), attr));
    if (new_val) {
      rul__wm_set_offset_val (wmo, offset, new_val, eb_data);
      rul__mol_decr_uses (new_val);
    }
  }
}



/********************************************************************/

void
rul__wm_update_and_notify (Object wmo, Entry_Data eb_data)
{
  Molecule           args[2];
  long               i;
  System_Method_Type meth_type;
  Class              class_id;

  if (wmo == NULL)
    return;

  i = WM_GET_OBJ_CHANGES (wmo);

  assert (i > 0);

  WM_SET_OBJ_CHANGES (wmo, --i);

  if (WM_GET_OBJ_CHANGES (wmo))
    return;

  wm_make_object (wmo);
  class_id = WM_GET_OBJ_CLASS (wmo);

  WM_SET_OBJ_MOD_RULE (wmo, rul__rac_active_rule_name);
  if (eb_data != NULL)
    WM_SET_OBJ_MOD_RB (wmo, eb_data->active_rb_name);

  /* debug trace on makes, copies, ... */
  if (rul__dbg_gl_enable & DBG_M_ENABLE_TRA) {
    if (rul__dbg_gl_trace & DBG_M_TRACE_WM) {
      if (rul__ios_curr_column (RUL__C_DEF_TRACE))
	rul__ios_print_newline (RUL__C_DEF_TRACE);
      switch (WM_GET_OBJ_OPER (wmo)) {
      case WM__OPR_MAKE:
	rul__ios_printf (RUL__C_DEF_TRACE, "   =>WM:  ");
	break;
      case WM__OPR_COPY:
	rul__ios_printf (RUL__C_DEF_TRACE, "   =>WM:  ");
	break;
      case WM__OPR_MODIFY:
	rul__ios_printf (RUL__C_DEF_TRACE, "   ->WM:  ");
	break;
      case WM__OPR_SPECIALIZE:
	rul__ios_printf (RUL__C_DEF_TRACE, "   ->WM:  ");
	break;
      }
      rul__wm_print_readform (wmo, RUL__C_DEF_TRACE);
      rul__ios_print_newline (RUL__C_DEF_TRACE);
    }
  }
  
  /* if debugging enabled */
  if (rul__dbg_gl_enable & DBG_M_ENABLE_DBG) {
    /*
     * if wbreaks have been set see if a match
     */
    if (rul__dbg_ga_wbreak) {
      if ((*rul__dbg_ga_wbreak)(wmo)) {
	rul__dbg_gl_break |= DBG_M_BREAK_WMO;
	rul__mol_use_readform (WM_GET_OBJ_IDENTIFIER (wmo),
			       wbreak_msg, MAX_WBREAK_MSG_LEN);
	wbreak_msg_idx = rul__mol_get_readform_length (
					       WM_GET_OBJ_IDENTIFIER (wmo));
	wbreak_msg[wbreak_msg_idx++] = '\0';
	rul__msg_print (CLI_WBREAK, wbreak_msg);
      }
      /*
       * if a modify, check for old == match, new != match
       */
      else if (WM_GET_OBJ_OPER (wmo) == WM__OPR_MODIFY && rul__dbg_mod_match) {
	rul__dbg_gl_break |= DBG_M_BREAK_WMO;
	rul__mol_use_readform (WM_GET_OBJ_IDENTIFIER(wmo),
			       wbreak_msg, MAX_WBREAK_MSG_LEN);
	wbreak_msg_idx = rul__mol_get_readform_length (
					       WM_GET_OBJ_IDENTIFIER (wmo));
	wbreak_msg[wbreak_msg_idx++] = '\0';
	rul__msg_print (CLI_WBREAK, wbreak_msg);
      }
    }
  }

  switch (WM_GET_OBJ_OPER (wmo)) {
  case WM__OPR_MAKE:
  case WM__OPR_COPY:
    meth_type = AFTER_MAKE;
    break;
  case WM__OPR_MODIFY:
  case WM__OPR_SPECIALIZE:
    meth_type = AFTER_MODIFY;
    break;
  }
  if ((eb_data != NULL) &&
      (rul__decl_get_class_sys_methods (class_id) & (1 << meth_type))) {
    args [0] = WM_GET_OBJ_IDENTIFIER (wmo);
    args [1] = rul__rac_active_rule_name;
    rul__call_method (rul__decl_get_sys_meth_name (meth_type),
		      2, args, NULL, eb_data);
  }
}



/********************************************************************/

long
rul__wm_get_cur_time_tag (void)
{
  return wm_cur_time_tag;
}



/********************************************************************/

Object
rul__wm_create_snapshot (Object instance_ptr)
{
  Mol_Instance_Id	instance_id;
  Object		shadow_ptr;

  instance_id = WM_GET_OBJ_IDENTIFIER (instance_ptr);

  shadow_ptr = wm_create_object (WM_GET_OBJ_CLASS (instance_ptr),
				 WM_GET_OBJ_NUM_ATTRS (instance_ptr),
				 WM_GET_OBJ_CYCLE (instance_ptr),
				 instance_id,
				 WM_GET_OBJ_TIME_TAG (instance_ptr),
				 &WM_GET_OBJ_ATTR_VAL (instance_ptr,
					       DECL_INSTANCE_OF_ATTR_OFFSET));

  rul__mol_incr_uses (instance_id);
  return shadow_ptr;
}

/*******************************************************************/

void
rul__wm_free_wmh_data (Object wmo)
{
  if (WM_GET_OBJ_ATTR_RULES (wmo)) {
    rul__mem_free (WM_GET_OBJ_ATTR_RULES (wmo));
    rul__mem_free (WM_GET_OBJ_ATTR_RBS (wmo));
    WM_SET_OBJ_ATTR_RULES (wmo, NULL);
    WM_SET_OBJ_ATTR_RBS (wmo, NULL);
  }
}






/********************************************************************/

void
rul__wm_destroy_snapshot (Object shadow_ptr)
{
  wm_destroy_object (shadow_ptr);
}



/********************************************************************/

static Boolean
check_wmo_attr_value (Object wmo,
		      long attr_offset,
		      Molecule value)
{
  Decl_Domain  attr_dom;
  Decl_Shape   attr_shape;
  Mol_Symbol   attr_name;
  Class        attr_class, val_class;
  Object       val_wmo;
  Molecule     comp_atom;
  long         i, comp_elems;

  if (wmo == NULL)
    return FALSE; /* somebody will have already complained */

  attr_name = rul__decl_get_attr_name (WM_GET_OBJ_CLASS (wmo), attr_offset);
  attr_shape = rul__decl_get_attr_shape (WM_GET_OBJ_CLASS (wmo), attr_name);
  attr_dom = rul__decl_get_attr_domain (WM_GET_OBJ_CLASS (wmo), attr_name);

  switch (attr_shape) {

  case shape_atom:
    if (!rul__mol_is_atom (value)) {
      rul__msg_print_w_atoms (RTS_VALNOTSCLR, 2, value, attr_name);
      return FALSE;
    }
    break;

  case shape_compound:
    if (!rul__mol_is_compound (value)) {
      rul__msg_print_w_atoms (RTS_VALNOTCOMP, 2, value, attr_name);
      return FALSE;
    }

    /* check all mols in compound for shape and domain */
    attr_class = rul__decl_get_attr_class (WM_GET_OBJ_CLASS (wmo), attr_name);
    comp_elems = rul__mol_get_poly_count (value);
    for (i = 1; i <= comp_elems; i++) {

      comp_atom = rul__mol_get_comp_nth (value, i);

      switch (attr_dom) {

      case dom_symbol:
	if (!rul__mol_is_symbol (comp_atom)) {
	  rul__msg_print_w_atoms (RTS_VALNOTSYM, 2, comp_atom, attr_name);
	  return FALSE;
	}
	break;

      case dom_instance_id:
	if (!rul__mol_is_instance_id (comp_atom)) {
	  rul__msg_print_w_atoms (RTS_VALNOTID, 2, comp_atom, attr_name);
	  return FALSE;
	}

	if (attr_class != NULL && (comp_atom != rul__mol_instance_id_zero())) {
	  val_wmo = rul__mol_instance_id_value (comp_atom);
	  if (val_wmo == NULL) {
	    rul__msg_print_w_atoms (RTS_NOVALWMO, 2, comp_atom, attr_name);
	    return FALSE;
	  }
	  if (attr_class != rul__wm_get_class (val_wmo)) {
	    rul__msg_print_w_atoms (RTS_INVCLSID, 2, comp_atom, attr_name);
	    return FALSE;
	  }
	}
	break;

      case dom_opaque:
	if (!rul__mol_is_opaque (comp_atom)) {
	  rul__msg_print_w_atoms (RTS_VALNOTOPA, 2, comp_atom, attr_name);
	  return FALSE;
	}
	break;

      case dom_number:
	if (!rul__mol_is_int_atom (comp_atom) &&
	    !rul__mol_is_dbl_atom (comp_atom)) {
	  rul__msg_print_w_atoms (RTS_VALNOTNUM, 2, comp_atom, attr_name);
	  return FALSE;
	}
	break;

      case dom_int_atom:
	if (!rul__mol_is_int_atom (comp_atom)) {
	  rul__msg_print_w_atoms (RTS_VALNOTINT, 2, comp_atom, attr_name);
	  return FALSE;
	}
	break;

      case dom_dbl_atom:
	if (!rul__mol_is_dbl_atom (comp_atom)) {
	  rul__msg_print_w_atoms (RTS_VALNOTDBL, 2, comp_atom, attr_name);
	  return FALSE;
	}
	break;
      }
    }
    break;

  case shape_table:
    if (!rul__mol_is_table (value)) {
      rul__msg_print_w_atoms (RTS_VALNOTTAB, 2, value, attr_name);
      return FALSE;
    }
    break;
  }

  if (attr_shape == shape_atom) {

    switch (attr_dom) {

    case dom_symbol:
      if (!rul__mol_is_symbol (value)) {
	rul__msg_print_w_atoms (RTS_VALNOTSYM, 2, value, attr_name);
	return FALSE;
      }
      break;

    case dom_instance_id:
      if (!rul__mol_is_instance_id (value)) {
	rul__msg_print_w_atoms (RTS_VALNOTID, 2, value, attr_name);
	return FALSE;
      }
      attr_class = rul__decl_get_attr_class (WM_GET_OBJ_CLASS (wmo),
					     attr_name);
      if (attr_class != NULL && (value != rul__mol_instance_id_zero ())) {
	val_wmo = rul__mol_instance_id_value (value);
	if (val_wmo == NULL) {
	  rul__msg_print_w_atoms (RTS_NOVALWMO, 2, value, attr_name);
	  return FALSE;
	}
	if (attr_class != rul__wm_get_class (val_wmo)) {
	  rul__msg_print_w_atoms (RTS_INVCLSID, 2, value, attr_name);
	  return FALSE;
	}
      }
      break;

    case dom_opaque:
      if (!rul__mol_is_opaque (value)) {
	rul__msg_print_w_atoms (RTS_VALNOTOPA, 2, value, attr_name);
	return FALSE;
      }
      break;
      
    case dom_number:
      if (!rul__mol_is_int_atom (value) &&
	  !rul__mol_is_dbl_atom (value)) {
	rul__msg_print_w_atoms (RTS_VALNOTNUM, 2, value, attr_name);
	return FALSE;
      }
      break;
      
    case dom_int_atom:
      if (!rul__mol_is_int_atom (value)) {
	rul__msg_print_w_atoms (RTS_VALNOTINT, 2, value, attr_name);
	return FALSE;
      }
      break;
      
    case dom_dbl_atom:
      if (!rul__mol_is_dbl_atom (value)) {
	rul__msg_print_w_atoms (RTS_VALNOTDBL, 2, value, attr_name);
	return FALSE;
      }
      break;
    }
  }

  return TRUE;
}


static Boolean
check_comp_attr_value (Object wmo, Mol_Symbol attr,
		       Molecule value, Molecule comp_index)
{
  static Molecule last_mol = NULL;
  Molecule        idx = comp_index;
  Decl_Shape      attr_shape;

  if (wmo == NULL)
    return FALSE; /* somebody will have already complained */

  if (!rul__decl_is_attr_in_class (WM_GET_OBJ_CLASS (wmo), attr)) {
    rul__msg_print_w_atoms (RTS_INVWMOATT, 2, attr,
			    WM_GET_OBJ_IDENTIFIER (wmo));
    return FALSE;
  }

  if (idx == NULL) {
    if (last_mol == NULL) {
      last_mol = rul__mol_make_symbol ("$LAST");
      rul__mol_mark_perm (last_mol);
    }
    idx = last_mol;
  }
  else if (!rul__mol_is_int_atom (idx)) {
    rul__msg_print_w_atoms (RTS_INVCMPIDX, 1, idx);
    return FALSE;
  }
      
  attr_shape = rul__decl_get_attr_shape (WM_GET_OBJ_CLASS (wmo), attr);

  if (attr_shape != shape_compound) {
    rul__msg_print_w_atoms (RTS_ATTNOTCOMP, 2, attr, idx);
    return FALSE;
  }

  if (rul__mol_get_shape (value) != shape_atom) {
    rul__msg_print_w_atoms (RTS_VALNOTSCX, 3, value, attr, idx);
    return FALSE;
  }


  return TRUE;
}
