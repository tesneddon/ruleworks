/*
 *	wm.h - implements the Working Memory subsystem
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
**	RuleWorks compiler
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
**	01-Dec-1992	DEC	Add *_histforms
**	01-Dec-1999	CPQ	Release with GPL
*/

/* INCLUDE-IN-GEND.H  *********************************************** */
#ifdef RUL__C_IN_GENERATED_CODE
#define WCR     rul__wm_create
#define WCRI    rul__wm_create_id
#define WCRV    rul__wm_create_var
#define WMO     rul__wm_modify
#define WMOI    rul__wm_modify_id
#define WCO     rul__wm_copy
#define WCOI    rul__wm_copy_id
#define WSP     rul__wm_specialize
#define WSPI    rul__wm_specialize_id
#define WSPV    rul__wm_specialize_var
#define WSOV    rul__wm_set_offset_val
#define WSOVS   rul__wm_set_offset_val_simple
#define WSVAV   rul__wm_set_var_attr_val
#define WSCAV   rul__wm_set_comp_attr_val
#define WSCAL   rul__wm_set_comp_attr_last
#define WUN     rul__wm_update_and_notify
#define WGOV    rul__wm_get_offset_val
#define WGOVI   rul__wm_get_offset_val_id
#define WGAVI   rul__wm_get_attr_val_id
#define WGAVV   rul__wm_get_attr_val_var
#define WGII    rul__wm_get_instance_id
#define WGE     rul__wm_get_every
#define WDNA    rul__wm_destroy_and_notify_all
#define WDNI    rul__wm_destroy_and_notify_id
#define WDACV   rul__wm_destroy_all_class_var
#endif
/* END-INCLUDE-IN-GEND.H  *********************************************** */

typedef enum {
  WM__OPR_MAKE = 1,	/* create a new wmo */
  WM__OPR_COPY,		/* copy an existing wmo */
  WM__OPR_MODIFY,	/* modify an existing wmo */
  WM__OPR_REMOVE,	/* delete an existing wmo */
  WM__OPR_SPECIALIZE	/* specialize an existing wmo */
  } Wm_Opr_Code;


char           *rul__wm_get_printform (Object wmo);

Boolean         rul__wm_use_printform (Object wmo,
				       char *buffer,
				       long max_chars);

void            rul__wm_print_printform (Object wmo,
					 IO_Stream ios);

char            *rul__wm_get_readform (Object wmo);

Boolean         rul__wm_use_readform (Object wmo,
				      char *buffer,
				      long max_chars);

long            rul__wm_get_readform_length (Object wmo);

void            rul__wm_print_readform (Object wmo,
					IO_Stream ios);

char           *rul__wm_get_histform (Object wmo, Molecule att);

Boolean         rul__wm_use_histform (Object wmo,
				      Molecule att,
				      char *buffer,
				      long max_chars);

void            rul__wm_print_histform (Object wmo,
					Molecule att,
					IO_Stream ios);

Boolean         rul__wm_use_wmoform (Object wmo,
				     char *buffer,
				     long max_chars);

long            rul__wm_get_wmoform_length (Object wmo);

void            rul__wm_print_saveform (Object wmo,
					IO_Stream ios);

/* INCLUDE-IN-GEND.H  *********************************************** */
Object          rul__wm_create_var (Mol_Symbol class_mol,
				    Entry_Data eb_data);

Object          rul__wm_create (Class class_id,
				Mol_Instance_Id opt_id,
				Entry_Data eb_data);

Object          rul__wm_copy_id (Mol_Instance_Id wid,
				 Entry_Data eb_data);

Object          rul__wm_copy (Object wmo_old,
			      Entry_Data eb_data);

void            rul__wm_destroy_and_notify_id (Mol_Instance_Id wid,
					       Entry_Data eb_data);

void            rul__wm_destroy_and_notify (Object wmo,
					    Entry_Data eb_data);

void            rul__wm_destroy_all_class_var (Mol_Symbol class_mol,
					       Entry_Data eb_data);

void            rul__wm_destroy_and_notify_all (Class class_id,
						Entry_Data eb_data);

Object          rul__wm_modify_id (Mol_Instance_Id wid,
				   Entry_Data eb_data);

void            rul__wm_modify (Object wmo,
				Entry_Data eb_data);

Object          rul__wm_specialize_var (Mol_Instance_Id wid,
					Mol_Symbol class_mol,
					Entry_Data eb_data);

Object          rul__wm_specialize_id (Mol_Instance_Id wid,
				       Class new_class_id,
				       Entry_Data eb_data);

Object          rul__wm_specialize (Object old_wmo,
				    Class new_class_id,
				    Entry_Data eb_data);

void            rul__wm_set_offset_val (Object wmo,
					long attr_offset,
					Molecule attr_val,
					Entry_Data eb_data);

void            rul__wm_set_offset_val_simple (Object wmo,
					       long attr_offset,
					       Molecule attr_val,
					       Entry_Data eb_data);

void            rul__wm_set_attr_val (Object wmo,
				      Mol_Symbol attr_name,
				      Molecule attr_val,
				      Entry_Data eb_data);

void            rul__wm_set_var_attr_val (Object wmo,
					  Mol_Symbol attr_name,
					  Molecule attr_val,
					  Entry_Data eb_data);

void            rul__wm_set_comp_attr_val (Object wmo,
					   Mol_Symbol attr_name,
					   Molecule value,
					   Molecule index,
					   Entry_Data eb_data);

void            rul__wm_set_comp_attr_last (Object wmo,
					    Mol_Symbol attr_name,
					    Molecule value,
					    Entry_Data eb_data);

void            rul__wm_update_and_notify (Object wmo,
					   Entry_Data eb_data);

Mol_Instance_Id rul__wm_get_instance_id (Object wmo);

/* END-INCLUDE-IN-GEND.H  *********************************************** */

long            rul__wm_get_cur_time_tag (void);

Class           rul__wm_get_class (Object wmo);

long            rul__wm_get_time_tag (Object wmo);

/* INCLUDE-IN-GEND.H  *********************************************** */

Molecule        rul__wm_get_offset_val (Object wmo,
					long attr_offset);

Molecule        rul__wm_get_offset_val_id (Mol_Instance_Id wid,
					   long attr_offset);

Molecule        rul__wm_get_attr_val (Object wmo,
				      Mol_Symbol attr_name);

Molecule        rul__wm_get_attr_val_id (Mol_Instance_Id wid,
					 Mol_Symbol attr_name,
					 Entry_Data eb_data);

Molecule        rul__wm_get_attr_val_var (Mol_Instance_Id wid,
					  Mol_Symbol attr_name,
					  Entry_Data eb_data);

Mol_Compound    rul__wm_get_every (Mol_Symbol class_name,
				   Entry_Data eb_data);

/* END-INCLUDE-IN-GEND.H  *********************************************** */

void            rul__wm_for_each_object_in_wm (void (*funcptr)(Object inst));

void            rul__wm_for_each_known_object (long db_count,
					       Mol_Symbol db_names[],
					       void (*funcptr)(Object inst));

void            rul__wm_for_each_object_from_db (Mol_Symbol db_name,
						 void (*funcptr)(Object inst));

void            rul__wm_for_each_member_of (Class superclass_id,
					    void (*funcptr)(Object inst));

void            rul__wm_for_each_instance_of (Class class_id,
					      void (*funcptr)(Object inst));

void            rul__wm_free_wmh_data (Object inst);

Boolean         rul__wm_is_snapshot (Object inst_ptr);

void            rul__wm_destroy_snapshot (Object inst_ptr);

Object          rul__wm_create_snapshot (Object inst_ptr);

