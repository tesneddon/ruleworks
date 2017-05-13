/*
 *	rts_wm_iterate.c - module to iterate over Working Memory Objects
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
#include <wm.h>			/* Working Memory function declarations	*/
#include <list.h>		/* declares list functions		*/
#include <rbs.h>		/* declares ruling block system */



static void
wm_for_each_object_from_decl_bl(Decl_Block block,
				void (*function_ptr) (Object instance_ptr))
{
  List   wmo_list;
  Object wmo;

  wmo_list = rul__decl_get_block_wmo_list (block);
  while (! rul__list_is_empty (wmo_list)) {
    wmo = (Object) rul__list_first (wmo_list);
    wmo_list = rul__list_rest (wmo_list);
    (*function_ptr) (wmo);
  }
}



void
rul__wm_for_each_object_in_wm (void (*function_ptr) (Object instance_ptr))
{
  Decl_Block block;

  for (block = rul__decl_get_first_decl_block ();
       block != NULL;
       block = rul__decl_get_next_decl_block (block))
    wm_for_each_object_from_decl_bl (block, function_ptr);
}



void
rul__wm_for_each_known_object (long db_count, Mol_Symbol db_names[],
			       void (*function_ptr)(Object instance_ptr))
{
  long i;

  for (i = 0; i < db_count; i++) {
    wm_for_each_object_from_decl_bl (rul__decl_get_block (db_names[i]),
				     function_ptr);
  }
}



void
rul__wm_for_each_object_from_db (Mol_Symbol declaring_block_name,
				 void (*function_ptr) (Object instance_ptr))
{
  wm_for_each_object_from_decl_bl (rul__decl_get_block (declaring_block_name),
				   function_ptr);
}



void
rul__wm_for_each_member_of (Class superclass_id,
			    void (*function_ptr) (Object instance_ptr))
{
  List   wmo_list;
  Object wmo;

  wmo_list = rul__decl_get_block_wmo_list (
				   rul__decl_get_class_block (superclass_id));
  while (! rul__list_is_empty (wmo_list)) {
    wmo = (Object) rul__list_first (wmo_list);
    wmo_list = rul__list_rest (wmo_list);
    if (rul__decl_is_subclass_of (rul__wm_get_class (wmo), superclass_id))
      (*function_ptr) (wmo);
  }
}



void
rul__wm_for_each_instance_of (Class class_id,
			      void (*function_ptr) (Object instance_ptr))
{
  register List wmo_list;
  Object        wmo;

  wmo_list = rul__decl_get_block_wmo_list (
				   rul__decl_get_class_block (class_id));
  while (! rul__list_is_empty (wmo_list)) {
    wmo = (Object) rul__list_first (wmo_list);
    wmo_list = rul__list_rest (wmo_list);
    if (rul__wm_get_class (wmo) == class_id)
      (*function_ptr) (wmo);
  }
}

