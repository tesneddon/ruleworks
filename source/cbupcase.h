/****************************************************************************
**                                                                         **
**                    R T S _ C B _ U P C A S E . H                        **
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
 *	RULEWORKS run time system.
 *
 * ABSTRACT:
 *	This file contains definitions to upcase the helper routines
 *	used by the rts callback bindings in:
 *	RTS_CB_ATOM_TABLE, RTS_CB_WM_QUERY, and RTS_CB_WMS 
 *	allowing them to compiled /name=as_is and also 
 *	be upcase on VMS and downcase on unix.
 *
 * MODIFIED BY:
 *	DEC	Digital Equipment Corporation
 *	CPQ	Compaq Computer Corporation
 *
 * REVISION HISTORY:
 *
 *	25-Sep-1991	DEC	Initial Version
 *	01-Dec-1999	CPQ	Releasew ith GPL
 */

#ifdef __VMS

#define rul__to_desc			RUL__TO_DESC
#define rul__ascic_to_asciz		RUL__ASCIC_TO_ASCIZ
#define rul__validate_wme_id		RUL__VALIDATE_WME_ID
#define rul__validate_block_class	RUL__VALIDATE_BLOCK_CLASS
#define rul__validate_class_attr	RUL__VALIDATE_CLASS_ATTR
#define rul__validate_attr_value	RUL__VALIDATE_ATTR_VALUE
#define rul__mol_is_atom		RUL__MOL_IS_ATOM
#define rul__mol_is_polyatom		RUL__MOL_IS_POLYATOM
#define rul__mol_is_number		RUL__MOL_IS_NUMBRE
#define rul__mol_is_symbol		RUL__MOL_IS_SYMBOL
#define rul__mol_is_instance_id		RUL__MOL_IS_INSTANCE_ID
#define rul__mol_is_opaque		RUL__MOL_IS_OPAQUE
#define rul__mol_is_int_atom		RUL__MOL_IS_INT_ATOM
#define rul__mol_is_dbl_atom		RUL__MOL_IS_DBL_ATOM
#define rul__mol_is_compound		RUL__MOL_IS_COMPOUND
#define rul__mol_is_table		RUL__MOL_IS_TABLE
#define rul__fatom_to_float		RUL__FATOM_TO_FLOAT
#define rul__fatom_to_double		RUL__FATOM_TO_DOUBLE
#define rul__float_to_fatom		RUL__FLOAT_TO_FATOM
#define rul__double_to_fatom		RUL__DOUBLE_TO_FATOM
#define rul__gensym			RUL__GENSYM
#define rul__genint			RUL__GENINT
#define rul__iatom_to_integer		RUL__IATOM_TO_INTEGER
#define rul__integer_to_iatom		RUL__INTEGER_TO_IATOM
#define rul__string_to_symbol		RUL__STRING_TO_SYMBOL
#define rul__symbol_to_string		RUL__SYMBOL_TO_STRING
#define rul__string_to_atom		RUL__STRING_TO_ATOM
#define rul__atom_to_string		RUL__ATOM_TO_STRING
#define rul__string_to_compound		RUL__STRING_TO_COMPOUND
#define rul__attr_is_compound		RUL__ATTR_IS_COMPOUND
#define rul__is_attribute		RUL__IS_ATTRIBUTE
#define rul__is_class			RUL__IS_CLASS
#define rul__is_subclass		RUL__IS_SUBCLASS
#define rul__is_wme			RUL__IS_WME
#define rul__put_string_in_buffer	RUL__PUT_STRING_IN_BUFFER
#define rul__get_attr_atom		RUL__GET_ATTR_ATOM
#define rul__get_class_string		RUL__GET_CLASS_STRING
#define rul__get_comp_attr_length	RUL__GET_COMP_ATTR_LENGTH
#define rul__get_comp_attr_string	RUL__GET_COMP_ATTR_STRING
#define rul__get_comp_elem_atom		RUL__GET_COMP_ELEM_ATOM
#define rul__get_wme			RUL__GET_WME
#define rul__get_next_wme		RUL__GET_NEXT_WME
#define rul__copy_wme			RUL__COPY_WME
#define rul__make_wme			RUL__MAKE_WME
#define rul___make_setup		RUL___MAKE_SETUP
#define rul___make_input		RUL___MAKE_INPUT
#define rul__modify_wme_start		RUL__MODIFY_WME_START
#define rul__modify_wme_end		RUL__MODIFY_WME_END
#define rul__remove_wme			RUL__REMOVE_WME
#define rul__specialize_wme		RUL__SPECIALIZE_WME
#define rul__set_attr_atom		RUL__SET_ATTR_ATOM
#define rul__set_attr_float		RUL__SET_ATTR_FLOAT
#define rul__set_attr_double		RUL__SET_ATTR_DOUBLE
#define rul__set_attr_integer		RUL__SET_ATTR_INTEGER
#define rul__set_attr_string		RUL__SET_ATTR_STRING
#define rul__set_comp_attr_string	RUL__SET_COMP_ATTR_STRING
#define rul__set_comp_attr_string_len	RUL__SET_COMP_ATTR_STRING_LEN
#define rul__set_comp_elem_atom		RUL__SET_COMP_ELEM_ATOM
#define rul__set_comp_elem_float	RUL__SET_COMP_ELEM_FLOAT
#define rul__set_comp_elem_double	RUL__SET_COMP_ELEM_DOUBLE
#define rul__set_comp_elem_integer	RUL__SET_COMP_ELEM_INTEGER
#define rul__set_comp_elem_string	RUL__SET_COMP_ELEM_STRING
#define rul__clear			RUL__CLEAR
#define rul__completion			RUL__COMPLETION
#define rul__debug			RUL__DEBUG
#define rul__get_firing_rule		RUL__GET_FIRING_RULE
#define rul__initialize			RUL__INITIALIZE
#define rul__run			RUL__RUN
#define rul__startup			RUL__STARTUP
#define rul__start_wme_id_translation	RUL__START_WME_ID_TRANSLATION
#define rul__end_wme_id_translation	RUL__END_WME_ID_TRANSLATION
#define rul__translate_wme_id		RUL__TRANSLATE_WME_ID
#define rul__is_wid_translation_on	RUL__IS_WID_TRANSLATION_ON

#endif
