/*
 *	rts_wm_query.c - module to query Working Memory Objects
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
**	RULEWORKS
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
**	01-Dec-1999	CPQ	Release with GPL
*/

#include <common.h>		/* declares types and includes headers	*/
#include <decl.h>		/* declares most class functions	*/
#include <decl_att.h>           /* declares attr offset info            */
#include <wm.h>			/* Working Memory function declarations	*/
#include <wm_p.h>		/* Working Memory type definitions     	*/
#include <mol.h>
#include <rac.h>
#include <msg.h>
#include <rts_msg.h>

static long SL_every_count;



Class
rul__wm_get_class (Object wmo)
{
  return WM_GET_OBJ_CLASS(wmo);
}



Mol_Instance_Id
rul__wm_get_instance_id (Object wmo)
{
  if (wmo)
    return WM_GET_OBJ_IDENTIFIER(wmo);
  else
    return (rul__mol_instance_id_zero ());
}



long
rul__wm_get_time_tag (Object wmo)
{
  return WM_GET_OBJ_TIME_TAG(wmo);
}



Molecule
rul__wm_get_offset_val (Object wmo, long attr_offset)
{
  return WM_GET_OBJ_ATTR_VAL(wmo,attr_offset);
}



Molecule
rul__wm_get_offset_val_id (Mol_Instance_Id wid, long attr_offset)
{
  Object    wmo;
  Molecule  mol;

  /* (get <$id> ^attr-offset) */

  wmo = rul__mol_instance_id_value (wid);

  if (wmo) {
    mol = WM_GET_OBJ_ATTR_VAL (wmo, attr_offset);
    rul__mol_incr_uses (mol);
    return mol;
  }

  rul__msg_print_w_atoms (RTS_NOSUCHWM, 1, wid);
  return rul__mol_symbol_nil();
}



Molecule
rul__wm_get_attr_val (Object wmo, Mol_Symbol attr_name)
{
  return rul__wm_get_offset_val(wmo,
				rul__decl_get_attr_offset(
					rul__wm_get_class(wmo),attr_name));
}

Molecule
rul__wm_get_attr_val_id (Mol_Instance_Id wid, Mol_Symbol attr_name,
			 Entry_Data eb_data)
{
  Mol_Symbol  class_mol;
  Molecule    mol;
  Object      wmo;

  /* (get <$id> ^<attr>) */

  wmo = rul__mol_instance_id_value (wid);
  if (wmo) {
    if (rul__decl_is_class_visible_rt (WM_GET_OBJ_CLASS (wmo), 
					   eb_data->db_names,
					   eb_data->db_name_count)) {
      if (rul__decl_is_attr_in_class (WM_GET_OBJ_CLASS (wmo), attr_name)) {
	mol = rul__wm_get_offset_val (wmo,
		       rul__decl_get_attr_offset (
					  WM_GET_OBJ_CLASS (wmo), attr_name));
	rul__mol_incr_uses (mol);
	return mol;
      }
      else
	rul__msg_print_w_atoms (RTS_INVWMOATT, 2, attr_name, wid);
    }
    else {
      class_mol = rul__decl_get_class_name (WM_GET_OBJ_CLASS (wmo));
      rul__msg_print_w_atoms (RTS_NOTINSCOPE, 1, class_mol);
    }
  }
  else
    rul__msg_print_w_atoms (RTS_NOSUCHWM, 1, wid);

  return rul__mol_symbol_nil();
}

Molecule
rul__wm_get_attr_val_var (Mol_Instance_Id wid, Mol_Symbol attr_name,
			  Entry_Data eb_data)
{

  /* (get <$id> ^<attr>) */
  /* similar to get_attr_val_id, must also validate the id */

  if (rul__mol_is_instance_id (wid))
    return (rul__wm_get_attr_val_id (wid, attr_name, eb_data));

  rul__msg_print_w_atoms (RTS_INVWMOID, 1, wid);
     
  return rul__mol_symbol_nil();
}



Boolean
rul__wm_is_snapshot(Object instance_ptr)
{
  return WM_IS_OBJ_SNAPSHOT(instance_ptr);
}

static void rul___wm_every_id (Object wmo)
{
  rul__mol_set_tmp_comp_nth (SL_every_count++, WM_GET_OBJ_IDENTIFIER (wmo));
}



Mol_Compound rul__wm_get_every (Mol_Symbol class_name, Entry_Data eb_data)
{
  Class class_id;

  class_id = rul__decl_get_visible_class_rt (class_name, eb_data->db_names,
					     eb_data->db_name_count);
  if (class_id) {
    rul__mol_start_tmp_comp (10);
    SL_every_count = 0;
    rul__wm_for_each_member_of (class_id, rul___wm_every_id);
    return (rul__mol_end_tmp_comp ());
  }

  rul__msg_print_w_atoms (RTS_NOSUCHCLS, 1, class_name);
  return rul__mol_compound_empty ();
}
