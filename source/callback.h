/****************************************************************************
**                                                                         **
**                         C A L L B A C K . H                             **
**                 
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
**                                                                         **
****************************************************************************/

/*
 * FACILITY:
 *	RuleWorks run time system.
 *
 * ABSTRACT:
 *	This file declares the external specification for rts_cb_common.c,
 *	which provides private routines used by the user-visible
 *	callback routines defined in RTS_CB_ATOM_TABLE, RTS_CB_WM_QUERY,
 *	and RTS_CB_WMS.
 *
 * MODIFIED BY:
 *	DEC	Digital Equipment Corporation
 *	CPQ	Compaq Computer Corporation
 *
 * REVISION HISTORY:
 *
 *	13-Sep-1991	DEC	Added rts_cb_wms helper routines.
 *	
 *	6-Mar-1992	DEC	Added FOI error routines
 *
 *	30-Mar-1998	DEC	Added defn of rul__atom_is_oatom
 *
 *	01-Dec-1999	CPQ	Releasew ith GPL
 */

#if 0

#ifndef OPS_C_INVALID_ATOM
#define OPS_C_INVALID_ATOM	RUL_C_INVALID_ATOM
#endif

#ifndef OPS_C_RESET_WM_ATOM
#define OPS_C_RESET_WM_ATOM	RUL_C_RESET_WM_ATOM
#endif

#ifndef OPS_C_INVALID_LENGTH
#define OPS_C_INVALID_LENGTH	RUL_C_INVALID_LENGTH
#endif

#endif

/* RTS constant values */

#ifndef RUL_C_INVALID_ATOM
#define RUL_C_INVALID_ATOM   (Molecule)NULL
#endif

#ifndef RUL_C_RESET_WM_ATOM
#define RUL_C_RESET_WM_ATOM  (Molecule)NULL
#endif

#ifndef RUL_C_INVALID_LENGTH
#define RUL_C_INVALID_LENGTH -1
#endif

/* RTS helper function prototypes */

void
rul__debug                      (void);

extern Molecule
rul__mol_gensym(Molecule);

extern Molecule
rul__mol_genint(void);

extern String
*rul__to_desc			(char     *in_string);

extern void
rul__ascic_to_asciz		(char     *out_string,
				 char     *in_string,
				 short     in_string_len);

extern Boolean
rul__validate_wme_id		(Molecule  wme_id,
				 Object    *wme);

extern Boolean
rul__validate_block_class (char     *block_name,
			   char     *class_name,
			   Class    *class_id);

extern Boolean
rul__validate_class_attr        (Object    wmo,
				 char     *attr_name,
				 Class    *class_id,
				 Molecule *attr);

extern Boolean
rul__validate_attr_value        (Class     class_id,
				 Molecule  attr,
				 Molecule  value);

extern Boolean
rul__atom_is_fatom		(Molecule  atom_value);

extern Boolean
rul__atom_is_iatom		(Molecule  atom_value);

extern Boolean
rul__atom_is_symbol		(Molecule  atom_value);

extern Boolean
rul__atom_is_compound		(Molecule  atom_value);

extern Boolean
rul__atom_is_instance_id	(Molecule  atom_value);

extern Boolean
rul__atom_is_oatom		(Molecule  atom_value);

extern float
rul__fatom_to_float		(Molecule  atom_value);

extern double
rul__fatom_to_double		(Molecule  atom_value);

extern Molecule
rul__float_to_fatom		(float     float_value);

extern Molecule
rul__double_to_fatom		(double    double_value);

extern Molecule
rul__gensym			(Molecule  atom_value);

extern Molecule
rul__genint			(void);

extern long
rul__iatom_to_integer		(Molecule  atom_value);

extern Molecule
rul__integer_to_iatom		(long      long_value);

extern void *
rul__oatom_to_ptr		(Molecule  atom_value);

extern Molecule
rul__ptr_to_oatom		(void     *ptr_value);

extern Molecule
rul__string_to_symbol		(char     *char_string);

extern long
rul__symbol_to_string		(char     *char_buffer,
				 long      buffer_size,
				 Molecule  atom_value);

extern long
rul__get_atom_string_length	(Molecule  atom_value);

extern Molecule
rul__string_to_atom		(char     *char_string);

extern long
rul__atom_to_string		(char 	  *char_buffer,
				 long      buffer_size,
				 Molecule  atom_value);

extern Molecule
rul__string_to_compound		(char     *char_string);

extern Boolean
rul__attr_is_compound		(char	  *class_name,
				 char	  *attr_name,
				 char     *block_name);

extern Boolean
rul__is_attribute		(char	  *class_name, 
				 char	  *attr_name,
				 char     *block_name);

extern Boolean
rul__is_class			(char	  *class_name,
				 char     *block_name);

extern Boolean
rul__is_subclass      		(char	  *child_class,
				 char	  *parent_class,
				 char     *block_name);

extern Boolean
rul__is_wme			(Molecule  wme_id);

extern Molecule
rul__get_attr_atom		(Molecule  wme_id,
				 char     *attr_name);

extern long
rul__get_class_string		(char	  *char_buffer,
				 long	   buf_len,
				 Molecule  wme_id);

extern long
rul__get_class_string_length	(Molecule  wme_id);

extern long
rul__get_comp_attr_length	(Molecule  wme_id,
				 char     *attr_name);

extern long
rul__get_comp_attr_string	(char     *char_buffer,
				 long      buffer_len,
				 Molecule  wme_id,
				 char     *attr_name);

extern long
rul__get_comp_attr_string_len	(Molecule  wme_id,
				 char     *attr_name);

extern Molecule
rul__get_comp_elem_atom		(Molecule  wme_id,
				 char     *attr_name,
				 long      element_index);

extern long
rul__get_wme			(char     *char_buffer, 
				 long      buffer_len,
				 Molecule  wme_id);

extern long
rul__get_wme_length		(Molecule  wme_id);

extern Molecule
rul__get_next_wme		(Molecule  wme_id);

extern Molecule
rul__copy_wme			(Molecule  wme_id);

extern Molecule
rul__make_wme			(char     *char_string,
				 char     *block_name);

extern Molecule
rul___make_wmos			(long      db_count,
				 Molecule *db_names);

extern Boolean
rul__modify_wme_start		(Molecule  wme_id);

extern Boolean
rul__modify_wme_end		(Molecule  wme_id);

extern Boolean
rul__remove_wme			(Molecule  wme_id);

extern Molecule
rul__specialize_wme		(Molecule  wme_id,
				 char     *class_name);

extern Boolean
rul__set_attr_atom		(Molecule  wme_id,
				 char     *attr_name,
				 Molecule  value);

extern Boolean
rul__set_attr_float		(Molecule  wme_id,
				 char     *attr_name,
				 float     value);

extern Boolean
rul__set_attr_double		(Molecule  wme_id,
				 char     *attr_name,
				 double    value);

extern Boolean
rul__set_attr_integer		(Molecule  wme_id,
				 char     *attr_name,
				 long      value);

extern Boolean
rul__set_attr_string		(Molecule  wme_id,
				 char     *attr_name,
				 char     *value);

extern Boolean
rul__set_comp_attr_string	(Molecule  wme_id,
				 char     *attr_name,
				 char     *values);

extern Boolean
rul__set_comp_elem_atom		(Molecule  wme_id,
				 char     *attr_name,
				 long      element_index,
				 Molecule  value);

extern Boolean
rul__set_comp_elem_float	(Molecule  wme_id,
				 char     *attr_name,
				 long      index,
				 float     value);

extern Boolean
rul__set_comp_elem_double	(Molecule  wme_id,
				 char     *attr_name,
				 long      index,
				 double    value);

extern Boolean
rul__set_comp_elem_integer	(Molecule  wme_id,
				 char     *attr_name,
				 long      index,
				 long      value);

extern Boolean
rul__set_comp_elem_opaque	(Molecule  wme_id,
				 char     *attr_name,
				 long      index,
				 void     *value);

extern Boolean
rul__set_comp_elem_string	(Molecule  wme_id,
				 char     *attr_name,
				 long      index,
				 char     *value);

extern void
rul__clear			(void);

extern void
rul__completion			(void (*address)(void));

extern Molecule
rul__get_firing_rule		(void);

extern void
rul__initialize			(void);

extern void
rul__run			(void);

extern void
rul__startup			(void);

extern void
rul__start_addstate_trans 	(void);

extern void
rul__end_addstate_trans 	(void);

extern void
rul__start_wme_id_translation	(void);

extern void
rul__end_wme_id_translation	(void);

extern Molecule
rul__translate_wme_id		(Molecule  wme_id);

extern Boolean
rul__is_wid_translation_on	(void);

