/****************************************************************************
**                                                                         **
**                        R T S _ C B _ W M S . C                          **
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
 *	RuleWorks run time system
 *
 * ABSTRACT:
 *	This module provides the externally visible RTL routines for
 *	creating and modifying WMEs.
 *
 *	Note that each entry point has three different variants to 
 *	comply with the NAS requirements.
 *
 * MODIFIED BY:
 *	DEC	Digital Equipment Corporation
 *	CPQ	Compaq Computer Corporation
 *
 * MODIFICATION HISTORY:
 *
 *	12-Sep-1991	DEC	Initial version
 *
 *	16-Sep-1991	DEC	Added rest of the wrappers
 *
 *	25-Sep-1991	DEC	Moved all functionality to helper routines
 *					in RTS_C_COMMON.C
 *
 *	14-Nov-1991	DEC	Fixed call to rul__set_attr_string for f77.
 *
 *	06-Dec-1991	DEC 	Added wrappers for OPS$CLEAR, OPS$COMPLETION,
 *					OPS$RUN, and OPS$STARTUP
 *
 *	25-Sep-1997	DEC	#pragma module directives added to allow
 *					different module names 
 *
 *	30-Mar-1998	DEC	Added rul_set_comp_elem_opaque
 *
 *	01-Dec-1999	CPQ	Releasew ith GPL
 */

/*	Table of contents:
 *
 *    Working Memory Modification Routines --
 *
 *        rul_boolean  rul_set_attr_atom          (Molecule, String, Molecule) 
 *        rul_boolean  rul_set_attr_float         (Molecule, String, float) 
 *        rul_boolean  rul_set_attr_double        (Molecule, String, double) 
 *        rul_boolean  rul_set_attr_integer       (Molecule, String, long) 
 *        rul_boolean  rul_set_attr_string        (Molecule, String, String) 
 *        rul_boolean  rul_set_comp_attr_string   (Molecule, String, String) 
 *        rul_boolean  rul_set_comp_elem_atom     (Molecule, String,
 *						   long, Molecule) 
 *        rul_boolean  rul_set_comp_elem_float    (Molecule, String,
 * 						   long, float) 
 *        rul_boolean  rul_set_comp_elem_double   (Molecule, String,
 * 						   long, double) 
 *        rul_boolean  rul_set_comp_elem_integer  (Molecule, String,
 *						   long, long) 
 *        rul_boolean  rul_set_comp_elem_string   (Molecule, String,
 *						   long, String) 
 *        rul_boolean  rul_set_comp_elem_opaque   (Molecule, String,
 *						   long, void*) 
 *        rul_atom     rul_copy_instance	  (Molecule)
 *        rul_atom     rul_make_instance	  (String, String)
 *        rul_boolean  rul_modify_instance_start  (Molecule)
 *        rul_boolean  rul_modify_instance_end	  (Molecule)
 *        rul_boolean  rul_remove_instance	  (Molecule)
 *        rul_atom     rul_specialize_instance	  (Molecule, String)
 *
 *    Miscellaneous Routines --
 *	
 *	  Void     rul_clear		    ()
 *        Void     rul_completion	    (Address)
 *	  Void	   rul_debug                ()
 *	  Molecule rul_get_firing_rule	    ()
 *	  Void     rul_initialize	    ()
 *	  Void     rul_run		    ()
 *	  Void     rul_startup		    ()
 *        Void	   rul_start_id_translation ()
 *        Void	   rul_start_id_translation ()
 *
 */

#ifdef LOWERCASE_C_BIND
#pragma module RTS_CB_WMS
#define C_BIND
#define USE_LOWERCASE_BINDINGS
#elif defined(UPPERCASE_C_BIND)
#pragma module RTS_CB_WMS_C
#define C_BIND
#elif defined(VMS_BIND)
#pragma module RTS_CB_WMS_VMS
#define USE_LOWERCASE_BINDINGS
				/* Actually, don't munge rul_* to rul_*_c */
#endif

#ifdef C_BIND
#include <rul_rtl.h>
#endif


#include <stddef.h>
#include <string.h>
#include <stdio.h>

#include <cbupcase.h>

#include <common.h>
#include <callback.h>
#include <dsc.h>

static  char     tmpstr1[RUL_C_MAX_SYMBOL_SIZE+1];
static  char     tmpstr2[RUL_C_MAX_SYMBOL_SIZE+1];



/*************************
**                      **
**  RUL_SET_ATTR_ATOM	**
**                      **
*************************/

#if defined(C_BIND)

rul_boolean
rul_set_attr_atom(rul_atom wme_id, char *attr_name, rul_atom value)
{
  return rul__set_attr_atom(wme_id, attr_name, value);
}

#elif defined(VMS_BIND)

Boolean
RUL_SET_ATTR_ATOM(Molecule *wme_id, String *attr_name, Molecule *value)
{
  rul__ascic_to_asciz (tmpstr1,
		       attr_name->dsc_a_pointer,
		       attr_name->dsc_w_length);
  return rul__set_attr_atom (*wme_id, tmpstr1, *value);
}

#elif defined(F77_BIND)

Boolean
rul_set_attr_atom_(Molecule *wme_id, char *attr_name, Molecule *value,
		   long attr_name_len)
{
  rul__ascic_to_asciz (tmpstr1, attr_name, attr_name_len);
  return rul__set_attr_atom (*wme_id, tmpstr1, *value);
}

#endif



/*************************
**                      **
**  RUL_SET_ATTR_FLOAT	**
**                      **
*************************/

#if defined(C_BIND)

rul_boolean
rul_set_attr_float(rul_atom wme_id, char *attr_name, float value)
{
  return rul__set_attr_float(wme_id, attr_name, value);
}

#elif defined(VMS_BIND)

Boolean
RUL_SET_ATTR_FLOAT(Molecule *wme_id, String *attr_name, float *value)
{

  rul__ascic_to_asciz (tmpstr1,
		       attr_name->dsc_a_pointer,
		       attr_name->dsc_w_length);
  return rul__set_attr_float (*wme_id, tmpstr1, *value);
}

#elif defined(F77_BIND)

Boolean
rul_set_attr_float_ (Molecule *wme_id, char *attr_name, float *value,
		     long attr_name_len)
{
  rul__ascic_to_asciz (tmpstr1, attr_name, attr_name_len);
  return rul__set_attr_float (*wme_id, tmpstr1, *value);
}

#endif

/*************************
**                      **
**  RUL_SET_ATTR_DOUBLE	**
**                      **
*************************/

#if defined(C_BIND)

rul_boolean
rul_set_attr_double(rul_atom wme_id, char *attr_name, double value)
{
  return rul__set_attr_double(wme_id, attr_name, value);
}

#elif defined(VMS_BIND)

Boolean
RUL_SET_ATTR_DOUBLE(Molecule *wme_id, String *attr_name, double *value)
{
  rul__ascic_to_asciz (tmpstr1,
		       attr_name->dsc_a_pointer,
		       attr_name->dsc_w_length);
  return rul__set_attr_double (*wme_id, tmpstr1, *value);
}

#elif defined(F77_BIND)

Boolean
rul_set_attr_double_(Molecule *wme_id, char *attr_name, double *value,
		     long attr_name_len)
{
  rul__ascic_to_asciz (tmpstr1, attr_name, attr_name_len);
  return rul__set_attr_double (*wme_id, tmpstr1, *value);
}

#endif



/***************************
**                        **
**  RUL_SET_ATTR_INTEGER  **
**                        **
***************************/

#if defined(C_BIND)

rul_boolean
rul_set_attr_integer(rul_atom wme_id, char *attr_name, long value)
{
  return rul__set_attr_integer(wme_id, attr_name, value);
}

#elif defined(VMS_BIND)

Boolean
RUL_SET_ATTR_INTEGER(Molecule *wme_id, String *attr_name, long *value)
{
  rul__ascic_to_asciz (tmpstr1,
		       attr_name->dsc_a_pointer,
		       attr_name->dsc_w_length);
  return rul__set_attr_integer (*wme_id, tmpstr1, *value);
}

#elif defined(F77_BIND)

Boolean
rul_set_attr_integer_(Molecule *wme_id, char *attr_name, long *value,
		      long attr_name_len)
{
  rul__ascic_to_asciz (tmpstr1, attr_name, attr_name_len);
  return rul__set_attr_integer (*wme_id, tmpstr1, *value);
}

#endif



/*************************
**                      **
**  RUL_SET_ATTR_STRING	**
**                      **
*************************/

#if defined(C_BIND)

rul_boolean
rul_set_attr_string(rul_atom wme_id, char *attr_name, char *value)
{
  return rul__set_attr_string(wme_id, attr_name, value);
}

#elif defined(VMS_BIND)

Boolean
RUL_SET_ATTR_STRING(Molecule *wme_id, String *attr_name, String *value)
{
  rul__ascic_to_asciz (tmpstr1,
		       attr_name->dsc_a_pointer,
		       attr_name->dsc_w_length);
  rul__ascic_to_asciz (tmpstr2,
		       value->dsc_a_pointer,
		       value->dsc_w_length);
  return rul__set_attr_string (*wme_id, tmpstr1, tmpstr2);
}

#elif defined(F77_BIND)

Boolean
rul_set_attr_string_(Molecule *wme_id, char *attr_name, char *value,
		   long attr_name_len, long value_len)
{
  rul__ascic_to_asciz (tmpstr1, attr_name, attr_name_len);
  rul__ascic_to_asciz (tmpstr2, value,     value_len);
  return rul__set_attr_string (*wme_id, tmpstr1, tmpstr2);
}

#endif



/*******************************
**                            **
**  RUL_SET_COMP_ATTR_STRING  **
**                            **
*******************************/

#if defined(C_BIND)

rul_boolean
rul_set_comp_attr_string(rul_atom wme_id, char *attr_name, char *value)
{
  return rul__set_comp_attr_string(wme_id, attr_name, value);
}

#elif defined(VMS_BIND)

Boolean
RUL_SET_COMP_ATTR_STRING(Molecule *wme_id, String *attr_name, String *value)
{
  rul__ascic_to_asciz (tmpstr1,
		       attr_name->dsc_a_pointer,
		       attr_name->dsc_w_length);
  rul__ascic_to_asciz (tmpstr2,
		       value->dsc_a_pointer,
		       value->dsc_w_length);
  return rul__set_comp_attr_string (*wme_id, tmpstr1, tmpstr2);
}

#elif defined(F77_BIND)

Boolean
rul_set_comp_attr_string_(Molecule *wme_id, char *attr_name, char *value,
			  long attr_name_len, long value_len)
{
  rul__ascic_to_asciz (tmpstr1, attr_name, attr_name_len);
  rul__ascic_to_asciz (tmpstr2, value, value_len);
  return rul__set_comp_attr_string (*wme_id, tmpstr1, tmpstr2);
}

#endif



/*********************************
**                              **
**  RUL_SET_COMP_ELEM_ATOM	**
**                              **
*********************************/

#if defined(C_BIND)

rul_boolean
rul_set_comp_elem_atom(rul_atom wme_id, char *attr_name, long index,
		       rul_atom value)
{
  return rul__set_comp_elem_atom(wme_id, attr_name, index, value);
}

#elif defined(VMS_BIND)

Boolean
RUL_SET_COMP_ELEM_ATOM(Molecule *wme_id, String *attr_name, long *index,
		       Molecule *value)
{
  rul__ascic_to_asciz (tmpstr1,
		       attr_name->dsc_a_pointer,
		       attr_name->dsc_w_length);
  return rul__set_comp_elem_atom (*wme_id, tmpstr1, *index, *value);
}

#elif defined(F77_BIND)

Boolean
rul_set_comp_elem_atom_(Molecule *wme_id, char *attr_name, long *index, 
			Molecule *value, long attr_name_len)
{
  rul__ascic_to_asciz (tmpstr1, attr_name, attr_name_len);
  return rul__set_comp_elem_atom (*wme_id, tmpstr1, *index, *value);
}

#endif



/*********************************
**                              **
**  RUL_SET_COMP_ELEM_FLOAT	**
**                              **
*********************************/

#if defined(C_BIND)

rul_boolean
rul_set_comp_elem_float(rul_atom wme_id, char *attr_name, long index,
			float value)
{
  return rul__set_comp_elem_float(wme_id, attr_name, index, value);
}

#elif defined(VMS_BIND)

Boolean
RUL_SET_COMP_ELEM_FLOAT(Molecule *wme_id, String *attr_name, long *index,
		        float *value)
{
  rul__ascic_to_asciz (tmpstr1,
		       attr_name->dsc_a_pointer,
		       attr_name->dsc_w_length);
  return rul__set_comp_elem_float (*wme_id, tmpstr1, *index, *value);
}

#elif defined(F77_BIND)

Boolean
rul_set_comp_elem_float_(Molecule *wme_id, char *attr_name, long *index, 
			 float *value, long attr_name_len)
{
  rul__ascic_to_asciz (tmpstr1, attr_name, attr_name_len);
  return rul__set_comp_elem_float (*wme_id, tmpstr1, *index, *value);
}

#endif

/*********************************
**                              **
**  RUL_SET_COMP_ELEM_DOUBLE	**
**                              **
*********************************/

#if defined(C_BIND)

rul_boolean
rul_set_comp_elem_double(rul_atom wme_id, char *attr_name, long index,
			 double value)
{
  return rul__set_comp_elem_double(wme_id, attr_name, index, value);
}

#elif defined(VMS_BIND)

Boolean
RUL_SET_COMP_ELEM_DOUBLE(Molecule *wme_id, String *attr_name, long *index,
			 double *value)
{
  rul__ascic_to_asciz (tmpstr1,
		       attr_name->dsc_a_pointer,
		       attr_name->dsc_w_length);
  return rul__set_comp_elem_double (*wme_id, tmpstr1, *index, *value);
}

#elif defined(F77_BIND)

Boolean
rul_set_comp_elem_double_(Molecule *wme_id, char *attr_name, long *index, 
			  double *value, long attr_name_len)
{
  rul__ascic_to_asciz (tmpstr1, attr_name, attr_name_len);
  return rul__set_comp_elem_double (*wme_id, tmpstr1, *index, *value);
}

#endif



/*********************************
**                              **
**  RUL_SET_COMP_ELEM_INTEGER	**
**                              **
*********************************/

#if defined(C_BIND)

rul_boolean
rul_set_comp_elem_integer(rul_atom wme_id, char *attr_name, long index,
			  long value)
{
  return rul__set_comp_elem_integer(wme_id, attr_name, index, value);
}

#elif defined(VMS_BIND)

Boolean
RUL_SET_COMP_ELEM_INTEGER(Molecule *wme_id, String *attr_name, long *index,
		          long *value)
{
  rul__ascic_to_asciz (tmpstr1,
		       attr_name->dsc_a_pointer,
		       attr_name->dsc_w_length);
  return rul__set_comp_elem_integer (*wme_id, tmpstr1, *index, *value);
}

#elif defined(F77_BIND)

Boolean
rul_set_comp_elem_integer_(Molecule *wme_id, char *attr_name, long *index, 
			   long *value, long attr_name_len)
{
  rul__ascic_to_asciz (tmpstr1, attr_name, attr_name_len);
  return rul__set_comp_elem_integer (*wme_id, tmpstr1, *index, *value);
}

#endif



/*********************************
**                              **
**  RUL_SET_COMP_ELEM_STRING	**
**                              **
*********************************/

#if defined(C_BIND)

rul_boolean
rul_set_comp_elem_string(rul_atom wme_id, char *attr_name, long index,
			 char *value)
{
  return rul__set_comp_elem_string(wme_id, attr_name, index, value);
}

#elif defined(VMS_BIND)

Boolean
RUL_SET_COMP_ELEM_STRING(Molecule *wme_id, String *attr_name, long *index,
		         String *value)
{
  rul__ascic_to_asciz (tmpstr1,
		       attr_name->dsc_a_pointer,
		       attr_name->dsc_w_length);
  rul__ascic_to_asciz (tmpstr2,
		       value->dsc_a_pointer,
		       value->dsc_w_length);
  return rul__set_comp_elem_string (*wme_id, tmpstr1, *index, tmpstr2);
}

#elif defined(F77_BIND)

Boolean
rul_set_comp_elem_string_(Molecule *wme_id, char *attr_name, long *index, 
			  char *value, long attr_name_len, long value_len)
{
  rul__ascic_to_asciz (tmpstr1, attr_name, attr_name_len);
  rul__ascic_to_asciz (tmpstr2, value,     value_len);
  return rul__set_comp_elem_string (*wme_id, tmpstr1, *index, tmpstr2);
}

#endif

/*********************************
**                              **
**  RUL_SET_COMP_ELEM_OPAQUE	**
**                              **
*********************************/

#if defined(C_BIND)

rul_boolean
rul_set_comp_elem_opaque(rul_atom wme_id, char *attr_name, long index,
			  void *value)
{
  return rul__set_comp_elem_opaque(wme_id, attr_name, index, value);
}

#elif defined(VMS_BIND)

Boolean
RUL_SET_COMP_ELEM_OPAQUE(Molecule *wme_id, String *attr_name, long *index,
		          void *value)
{
  rul__ascic_to_asciz (tmpstr1,
		       attr_name->dsc_a_pointer,
		       attr_name->dsc_w_length);
  return rul__set_comp_elem_opaque (*wme_id, tmpstr1, *index, value);
}

#elif defined(F77_BIND)

Boolean
rul_set_comp_elem_opaque_(Molecule *wme_id, char *attr_name, long *index, 
			   void *value, long attr_name_len)
{
  rul__ascic_to_asciz (tmpstr1, attr_name, attr_name_len);
  return rul__set_comp_elem_opaque (*wme_id, tmpstr1, *index, *value);
}

#endif



/*************************
**                      **
**  RUL_COPY_INSTANCE	**
**                      **
*************************/

#if defined(C_BIND)

rul_atom
rul_copy_instance(rul_atom wme_id)
{
  return rul__copy_wme(wme_id);
}

#elif defined(VMS_BIND)

Molecule
RUL_COPY_INSTANCE(Molecule *wme_id)
{
  return rul__copy_wme(*wme_id);
}

#elif defined(F77_BIND)

Molecule
rul_copy_instance_(Molecule *wme_id)
{
  return rul__copy_wme(*wme_id);
}
#endif



/*************************
**                      **
**  RUL_MAKE_INSTANCE	**
**                      **
*************************/

#if defined(C_BIND)

rul_atom
rul_make_instance(char *char_string, char *block_name)
{
  return (rul__make_wme (char_string, block_name));
}

#elif defined(VMS_BIND)

Molecule
RUL_MAKE_INSTANCE(String *char_string, String *block_name)
{
  rul__ascic_to_asciz (tmpstr1,
		       char_string->dsc_a_pointer,
		       char_string->dsc_w_length);
  rul__ascic_to_asciz (tmpstr2,
		       block_name->dsc_a_pointer,
		       block_name->dsc_w_length);
  return (rul__make_wme (tmpstr1, tmpstr2));
}

#elif defined(F77_BIND)

Molecule
rul_make_instance_(char *char_string, char *block_name,
	      long char_count, long block_name_count)
{
  rul__ascic_to_asciz (tmpstr1, char_string, char_count);
  rul__ascic_to_asciz (tmpstr2, block_name,  block_name_count);
  return (rul__make_wme (tmpstr1, tmpstr2));
}

#endif



/*******************************
**                            **
**  RUL_MODIFY_INSTANCE_START **
**                            **
*******************************/

#if defined(C_BIND)

rul_boolean
rul_modify_instance_start(rul_atom wme_id)
{
  return rul__modify_wme_start(wme_id);
}

#elif defined(VMS_BIND)

Boolean
RUL_MODIFY_INSTANCE_START(Molecule *wme_id)
{
  return rul__modify_wme_start(*wme_id);
}

#elif defined(F77_BIND)

Boolean
rul_modify_instance_start_(Molecule *wme_id)
{
  return rul__modify_wme_start(*wme_id);
}
#endif



/*****************************
**                          **
**  RUL_MODIFY_INSTANCE_END **
**                          **
*****************************/

#if defined(C_BIND)

rul_boolean
rul_modify_instance_end(rul_atom wme_id)
{
  return rul__modify_wme_end(wme_id);
}

#elif defined(VMS_BIND)

Boolean
RUL_MODIFY_INSTANCE_END(Molecule *wme_id)
{
  return rul__modify_wme_end(*wme_id);
}

#elif defined(F77_BIND)

Boolean
rul_modify_instance_end_(Molecule *wme_id)
{
  return rul__modify_wme_end(*wme_id);
}
#endif



/*************************
**                      **
**  RUL_REMOVE_INSTANCE	**
**                      **
*************************/

#if defined(C_BIND)

rul_boolean
rul_remove_instance(rul_atom wme_id)
{
  return rul__remove_wme(wme_id);
}

#elif defined(VMS_BIND)

Boolean
RUL_REMOVE_INSTANCE(Molecule *wme_id)
{
  return rul__remove_wme(*wme_id);
}

#elif defined(F77_BIND)

Boolean
rul_remove_instance_(Molecule *wme_id)
{
  return rul__remove_wme(*wme_id);
}
#endif



/*****************************
**                          **
**  RUL_SPECIALIZE_INSTANCE **
**                          **
*****************************/

#if defined(C_BIND)

rul_atom
rul_specialize_instance(rul_atom wme_id, char *class_name)
{
  return rul__specialize_wme(wme_id, class_name);
}

#elif defined(VMS_BIND)

Molecule
RUL_SPECIALIZE_INSTANCE(Molecule *wme_id, String *class_name)
{
  rul__ascic_to_asciz (tmpstr1,
		       class_name->dsc_a_pointer,
		       class_name->dsc_w_length);
  return rul__specialize_wme (*wme_id, tmpstr1);
}

#elif defined(F77_BIND)

Molecule
rul_specialize_instance_(Molecule *wme_id,
			 char *class_name, long class_name_len)
{
  rul__ascic_to_asciz (tmpstr1, class_name, class_name_len);
  return rul__specialize_wme (*wme_id, tmpstr1);
}
#endif



/*************************
**                      **
**  RUL_CLEAR		**
**                      **
*************************/

#if defined(C_BIND)

void
rul_clear(void)
{
  rul__clear();
}

#elif defined(VMS_BIND)

void
RUL_CLEAR(void)
{
  rul__clear();
}

#elif defined(F77_BIND)

void
rul_clear_(void)
{
  rul__clear();
}
#endif



/*************************
**                      **
**  RUL_COMPLETION	**
**                      **
*************************/

#if defined(C_BIND)

void
rul_completion(void address())
{
  rul__completion(address);
}

#elif defined(VMS_BIND)

void
RUL_COMPLETION(void address())
{
  rul__completion(address);
}

#elif defined(F77_BIND)

void
rul_completion_(void address())
{
  rul__completion(address);
}
#endif



/*************************
**                      **
**  RUL_DEBUG		**
**                      **
*************************/

#if defined(C_BIND)

void
rul_debug(void)
{
  rul__debug();
}

#elif defined(VMS_BIND)

void
RUL_DEBUG(void)
{
  rul__debug();
}

#elif defined(F77_BIND)

void
rul_debug_(void)
{
  rul__debug();
}
#endif



/**************************
**                       **
**  RUL_GET_FIRING_RULE  **
**                       **
**************************/

#if defined(C_BIND)

rul_atom
rul_get_firing_rule(void)
{
  return rul__get_firing_rule();
}

#elif defined(VMS_BIND)

Molecule
RUL_GET_FIRING_RULE(void)
{
  return rul__get_firing_rule();
}

#elif defined(F77_BIND)

Molecule
rul_get_firing_rule_(void)
{
  return rul__get_firing_rule();
}
#endif



/*************************
**                      **
**  RUL_INITIALIZE	**
**                      **
*************************/

#if defined(C_BIND)

void
rul_initialize(void)
{
  rul__initialize();
}

#elif defined(VMS_BIND)

void
RUL_INITIALIZE(void)
{
  rul__initialize();
}

#elif defined(F77_BIND)

void
rul_initialize_(void)
{
  rul__initialize();
}
#endif



/*************************
**                      **
**  RUL_RUN		**
**                      **
*************************/

#if defined(C_BIND)

void
rul_run(void)
{
  rul__run();
}

#elif defined(VMS_BIND)

void
RUL_RUN(void)
{
  rul__run();
}

#elif defined(F77_BIND)

void
rul_run_(void)
{
  rul__run();
}
#endif



/*************************
**                      **
**  RUL_STARTUP		**
**                      **
*************************/

#if defined(C_BIND)

void
rul_startup(void)
{
  rul__startup();
}

#elif defined(VMS_BIND)

void
RUL_STARTUP(void)
{
  rul__startup();
}

#elif defined(F77_BIND)

void
rul_startup_(void)
{
  rul__startup();
}
#endif



/*********************************
**				**
**  RUL_START_ID_TRANSLATION	**
**				**
*********************************/

#if defined(C_BIND)

void
rul_start_id_translation(void)
{
  rul__start_wme_id_translation();
}

#elif defined(VMS_BIND)

void
RUL_START_ID_TRANSLATION(void)
{
  rul__start_wme_id_translation();
}

#elif defined(F77_BIND)

void
rul_start_id_translation_(void)
{
  rul__start_wme_id_translation();
}
#endif



/*********************************
**        		        **
**  RUL_END_ID_TRANSLATION	**
**                              **
*********************************/

#if defined(C_BIND)

void
rul_end_id_translation(void)
{
  rul__end_wme_id_translation();
}

#elif defined(VMS_BIND)

void
RUL_END_ID_TRANSLATION(void)
{
  rul__end_wme_id_translation();
}

#elif defined(F77_BIND)

void
rul_end_id_translation_(void)
{
  rul__end_wme_id_translation();
}
#endif
