/****************************************************************************
**                                                                         **
**                   R T S _ C B _ W M _ Q U E R Y . C                     **
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
 *	testing declarations and querying working memory.
 *
 *	Note that each entry point has three different variants to 
 *	comply with the NAS requirements.
 *
 * MODIFIED BY:
 *	DEC	Digital Equipment Corporation
 *	CPQ	Compaq Computer Corporation
 *
 *
 * MODIFICATION HISTORY:
 *
 *	27-Jun-1991	DEC	Initial version
 *
 *	25-Sep-1991	DEC	Moved all functionality to helper routines
 *					in RTS_C_COMMON.C
 *
 *	25-Sep-1997	DEC	#pragma module directives added to allow
 *					different module names
 *
 *	01-Dec-1999	CPQ	Release with GPL
 */

/*	Table of contents:
 *
 *    Predicate Routines --
 *
 *        rul_boolean  rul_attr_is_compound         (String, String, String) 
 *        rul_boolean  rul_is_attribute             (String, String, String)
 *        rul_boolean  rul_is_class                 (String, String)
 *        rul_boolean  rul_is_subclass              (String, String, String)
 *        rul_boolean  rul_is_wme                   (Molecule)
 *
 *    WME Retrieval Routines --
 *	
 *        rul_atom	rul_get_attr_atom           (Molecule, String)
 *        long		rul_get_class_string        (String, int, Molecule)
 *        long		rul_get_class_string_length (String, int, Molecule)
 *        long		rul_get_comp_attr_length    (Molecule, String)
 *        long		rul_get_comp_attr_string    (String, int,
 *						     Molecule, String)
 *        long		rul_get_comp_attr_string_length (Molecule, String)
 *        rul_atom	rul_get_comp_elem_atom      (Molecule, String, int)
 *        long		rul_get_instance            (String, int, Molecule)
 *        rul_atom	rul_get_next_instance       (Molecule)
 *
 */

#ifdef LOWERCASE_C_BIND
#define USE_LOWERCASE_BINDINGS
#define C_BIND
#elif defined(UPPERCASE_C_BIND)
#define C_BIND
#elif defined(VMS_BIND)
#define USE_LOWERCASE_BINDINGS
				/* Actually, don't munge rul_* to rul_*_c */
#endif

#ifdef C_BIND
#include <rul_rtl.h>
#endif


#include <string.h>
#include <stdio.h>

#include <cbupcase.h>

#include <common.h>
#include <callback.h>
#include <dsc.h>
#include <msg.h>
#include <api_msg.h>

static char tmpstr1[RUL_C_MAX_SYMBOL_SIZE+1];
static char tmpstr2[RUL_C_MAX_SYMBOL_SIZE+1];
static char tmpstr3[RUL_C_MAX_SYMBOL_SIZE+1];



/*************************
**                      **
** RUL_ATTR_IS_COMPOUND **
**                      **
*************************/

#ifdef C_BIND

rul_boolean  rul_attr_is_compound (char		*class_name, 
				   char		*attr_name,
				   char		*block_name)
{
  return (rul__attr_is_compound (class_name, attr_name, block_name)) ;
}

#elif defined(VMS_BIND)

Boolean  RUL_ATTR_IS_COMPOUND (String	*class_name, 
			       String	*attr_name,
			       String	*block_name)
{
  rul__ascic_to_asciz (tmpstr1,
		       class_name->dsc_a_pointer,
		       class_name->dsc_w_length);
  rul__ascic_to_asciz (tmpstr2,
		       attr_name->dsc_a_pointer,
		       attr_name->dsc_w_length);
  rul__ascic_to_asciz (tmpstr3,
		       block_name->dsc_a_pointer,
		       block_name->dsc_w_length);
  return (rul__attr_is_compound (tmpstr1, tmpstr2, tmpstr3)) ;
}

#elif defined(F77_BIND)

Boolean  rul_attr_is_compound_ (char	*class_name, 
				char	*attr_name,
				char	*block_name,
				long	class_name_len, 
				long	attr_name_len,
				long	block_name_len)
{
  rul__ascic_to_asciz (tmpstr1, class_name, class_name_len);
  rul__ascic_to_asciz (tmpstr2, attr_name,  attr_name_len);
  rul__ascic_to_asciz (tmpstr3, block_name,  block_name_len);
  return (rul__attr_is_compound (tmpstr1, tmpstr2, tmpstr3)) ;
}

#endif



/************************
**                     **
** RUL_IS_ATTRIBUTE    **
**                     **
************************/

#ifdef C_BIND

rul_boolean  rul_is_attribute	(char	*class_name, 
				 char	*attr_name,
				 char	*block_name)
{
  return (rul__is_attribute (class_name, attr_name, block_name)) ;
}

#elif defined(VMS_BIND)

Boolean  RUL_IS_ATTRIBUTE	(String *class_name, 
				 String *attr_name,
				 String *block_name)
{
  rul__ascic_to_asciz (tmpstr1,
		       class_name->dsc_a_pointer,
		       class_name->dsc_w_length);
  rul__ascic_to_asciz (tmpstr2,
		       attr_name->dsc_a_pointer,
		       attr_name->dsc_w_length);
  rul__ascic_to_asciz (tmpstr3,
		       block_name->dsc_a_pointer,
		       block_name->dsc_w_length);
  return (rul__is_attribute (tmpstr1, tmpstr2, tmpstr3)) ;
}

#elif defined(F77_BIND)

Boolean  rul_is_attribute_	(char	*class_name, 
				 char	*attr_name,
				 char	*block_name,
				 long	class_name_len,
				 long	attr_name_len,
				 long	block_name_len)
{
  rul__ascic_to_asciz (tmpstr1, class_name, class_name_len);
  rul__ascic_to_asciz (tmpstr2, attr_name,  attr_name_len);
  rul__ascic_to_asciz (tmpstr3, block_name, block_name_len);
  return (rul__is_attribute (tmpstr1, tmpstr2, tmpstr3)) ;
}

#endif



/************************
**                     **
** RUL_IS_CLASS        **
**                     **
************************/

#ifdef C_BIND

rul_boolean  rul_is_class       (char	*class_name, 
				 char	*block_name)
{
  return (rul__is_class (class_name, block_name)) ;
}

#elif defined(VMS_BIND)

Boolean  RUL_IS_CLASS       (String *class_name,
			     String *block_name)
{
  rul__ascic_to_asciz (tmpstr1,
		       class_name->dsc_a_pointer,
		       class_name->dsc_w_length);
  rul__ascic_to_asciz (tmpstr2,
		       block_name->dsc_a_pointer,
		       block_name->dsc_w_length);
  return (rul__is_class (tmpstr1, tmpstr2)) ;
}

#elif defined(F77_BIND)

Boolean  rul_is_class_      (char	*class_name, 
			     char	*block_name,
			     long	class_name_len, 
			     long	block_name_len)
{
  rul__ascic_to_asciz (tmpstr1, class_name, class_name_len);
  rul__ascic_to_asciz (tmpstr2, block_name, block_name_len);
  return (rul__is_class (tmpstr1, tmpstr2)) ;
}

#endif



/************************
**                     **
** RUL_IS_SUBCLASS     **
**                     **
************************/

#ifdef C_BIND

rul_boolean  rul_is_subclass      (char *child_class,
				   char *parent_class,
				   char *block_name)
{
  return (rul__is_subclass (child_class, parent_class, block_name)) ;
}

#elif defined(VMS_BIND)

Boolean  RUL_IS_SUBCLASS      (String *child_class,
			       String *parent_class,
			       String *block_name)
{
  rul__ascic_to_asciz (tmpstr1,
		       child_class->dsc_a_pointer,
		       child_class->dsc_w_length);
  rul__ascic_to_asciz (tmpstr2,
		       parent_class->dsc_a_pointer,
		       parent_class->dsc_w_length);
  rul__ascic_to_asciz (tmpstr3,
		       block_name->dsc_a_pointer,
		       block_name->dsc_w_length);
  return (rul__is_subclass (tmpstr1, tmpstr2, tmpstr3)) ;
}

#elif defined(F77_BIND)

Boolean  rul_is_subclass_     (char *child_class,
			       char *parent_class,
			       char *block_name,
			       long child_class_len,
			       long parent_class_len,
			       long block_name_len)
{
  rul__ascic_to_asciz (tmpstr1, child_class,  child_class_len);
  rul__ascic_to_asciz (tmpstr2, parent_class, parent_class_len);
  rul__ascic_to_asciz (tmpstr3, block_name,   block_name_len);
  return (rul__is_subclass (tmpstr1, tmpstr2, tmpstr3)) ;
}

#endif



/*************************
**			**
** RUL_IS_INSTANCE	**
**			**
*************************/

#ifdef C_BIND

rul_boolean	 rul_is_instance	(rul_atom wme_id)
{
  return (rul__is_wme (wme_id)) ;
}

#elif defined(VMS_BIND)

Boolean	 RUL_IS_INSTANCE	(Molecule *wme_id)
{
  return (rul__is_wme (*wme_id)) ;
}

#elif defined(F77_BIND)

Boolean	 rul_is_instance_	(Molecule *wme_id)
{
  return (rul__is_wme (*wme_id)) ;
}

#endif



/************************
**                     **
** RUL_GET_ATTR_ATOM   **
**                     **
************************/

#ifdef C_BIND

rul_atom	 rul_get_attr_atom  (rul_atom wme_id, char *attr_name)
{
  return (rul__get_attr_atom (wme_id, attr_name)) ;
}
	

#elif defined(VMS_BIND)

Molecule	 RUL_GET_ATTR_ATOM        (Molecule *wme_id,
					   String   *attr_name)
{
  rul__ascic_to_asciz (tmpstr1,
		       attr_name->dsc_a_pointer,
		       attr_name->dsc_w_length);
  return (rul__get_attr_atom (*wme_id, tmpstr1)) ;
}

#elif defined(F77_BIND)

Molecule	 rul_get_attr_atom_        (Molecule *wme_id,
					    char     *attr_name,
					    long      attr_name_len)
{
  rul__ascic_to_asciz (tmpstr1, attr_name, attr_name_len);
  return (rul__get_attr_atom (*wme_id, tmpstr1)) ;
}

#endif



/*************************
**                      **
** RUL_GET_CLASS_STRING **
**                      **
*************************/

#ifdef C_BIND

long rul_get_class_string     (char     *char_buffer, 
			       long	 buf_len,
			       rul_atom  wme_id)
{
  return (rul__get_class_string (char_buffer, buf_len, wme_id)) ;
}

#elif defined(VMS_BIND)

long RUL_GET_CLASS_STRING     (String   *char_str, 
			       long     *buf_len,
			       Molecule *wme_id)
{
  if (char_str->dsc_b_class == DSC_K_CLASS_S &&
      char_str->dsc_b_dtype == DSC_K_DTYPE_T)
    return (rul__get_class_string (char_str->dsc_a_pointer,
				   char_str->dsc_w_length,
				   *wme_id)) ;
  else {
    rul__msg_print(API_INVSTRDSC) ;
    return (RUL_C_INVALID_LENGTH) ;
  }
}

#elif defined(F77_BIND)

    long rul_get_class_string_     (char     *char_buffer, 
				    long     *buf_len,
				    Molecule *wme_id,
				    long      char_buffer_len)
    {
    return (rul__get_class_string (char_buffer, *buf_len, *wme_id)) ;
    }

#endif



/********************************
**                             **
** RUL_GET_CLASS_STRING_LENGTH **
**                             **
********************************/

#ifdef C_BIND

long rul_get_class_string_length     (rul_atom  wme_id)
{
  return (rul__get_class_string_length (wme_id)) ;
}

#elif defined(VMS_BIND)

long RUL_GET_CLASS_STRING_LENGTH     (Molecule *wme_id)
{
  return (rul__get_class_string_length (*wme_id)) ;
}

#elif defined(F77_BIND)

long rul_get_class_string_length_     (Molecule *wme_id)
{
  return (rul__get_class_string_length (*wme_id)) ;
}

#endif



/*****************************
**                          **
** RUL_GET_COMP_ATTR_LENGTH **
**                          **
*****************************/

#ifdef C_BIND

long rul_get_comp_attr_length (rul_atom wme_id, char *attr_name)
{
  return (rul__get_comp_attr_length (wme_id, attr_name)) ;
}

#elif defined(VMS_BIND)

long RUL_GET_COMP_ATTR_LENGTH (Molecule *wme_id, String *attr_name)
{
  rul__ascic_to_asciz (tmpstr1,
		       attr_name->dsc_a_pointer,
		       attr_name->dsc_w_length);
  return (rul__get_comp_attr_length (*wme_id, tmpstr1)) ;
}

#elif defined(F77_BIND)

long rul_get_comp_attr_length_ (Molecule *wme_id, 
				char     *attr_name,
				long      attr_name_len)
{
  rul__ascic_to_asciz (tmpstr1, attr_name, attr_name_len);
  return (rul__get_comp_attr_length (*wme_id, tmpstr1));
}

#endif



/*****************************
**                          **
** RUL_GET_COMP_ATTR_STRING **
**                          **
*****************************/

#ifdef C_BIND

long rul_get_comp_attr_string (char     *char_buffer,
			       long      buffer_len,
			       rul_atom  wme_id,
			       char     *attr_name)
{
  return (rul__get_comp_attr_string (char_buffer, buffer_len,
				     wme_id, attr_name)) ;
}

#elif defined(VMS_BIND)

long RUL_GET_COMP_ATTR_STRING (String   *buffer_dsc,
			       long     *buffer_len,
			       Molecule *wme_id,
			       String   *attr_name)
{
  rul__ascic_to_asciz (tmpstr1,
		       attr_name->dsc_a_pointer,
		       attr_name->dsc_w_length);
  return (rul__get_comp_attr_string (buffer_dsc->dsc_a_pointer,
				     buffer_dsc->dsc_w_length,
				     *wme_id, tmpstr1)) ;
}

#elif defined(F77_BIND)

long rul_get_comp_attr_string_ (char     *char_buffer,
				long     *buffer_len,
				Molecule *wme_id,
				char     *attr_name,
				long      char_buffer_len,
				long      attr_name_len)
{
  rul__ascic_to_asciz (tmpstr1, attr_name, attr_name_len);
  return (rul__get_comp_attr_string (char_buffer,
				     char_buffer_len,
				     *wme_id,
				     tmpstr1)) ;
}

#endif



/*********************************
**                              **
** RUL_GET_COMP_ATTR_STRING_LEN **
**                              **
*********************************/

#ifdef C_BIND

long rul_get_comp_attr_string_len (rul_atom  wme_id,
				   char     *attr_name)
{
  return (rul__get_comp_attr_string_len (wme_id, attr_name)) ;
}

#elif defined(VMS_BIND)

long RUL_GET_COMP_ATTR_STRING_LEN (Molecule *wme_id,
				   String   *attr_name)
{
  rul__ascic_to_asciz (tmpstr1,
		       attr_name->dsc_a_pointer,
		       attr_name->dsc_w_length);
  return (rul__get_comp_attr_string_len (*wme_id, tmpstr1)) ;
}

#elif defined(F77_BIND)

long rul_get_comp_attr_string_len_ (Molecule *wme_id,
				    char     *attr_name,
				    long      attr_name_len)
{
  rul__ascic_to_asciz (tmpstr1, attr_name, (short) attr_name_len);
  return (rul__get_comp_attr_string_len (*wme_id, tmpstr1)) ;
}

#endif



/***************************
**                        **
** RUL_GET_COMP_ELEM_ATOM **
**                        **
***************************/

#ifdef C_BIND

rul_atom rul_get_comp_elem_atom (rul_atom  wme_id,
				 char     *attr_name,
				 long      element_index)
{
  return (rul__get_comp_elem_atom (wme_id, attr_name, element_index)) ;
}

#elif defined(VMS_BIND)

Molecule	 RUL_GET_COMP_ELEM_ATOM   (Molecule *wme_id,
					   String   *attr_name,
					   long     *element_index)
{
  rul__ascic_to_asciz (tmpstr1,
		       attr_name->dsc_a_pointer,
		       attr_name->dsc_w_length);
  return (rul__get_comp_elem_atom (*wme_id, tmpstr1, *element_index)) ;
}

#elif defined(F77_BIND)

Molecule	 rul_get_comp_elem_atom_   (Molecule *wme_id,
					    char     *attr_name,
					    long     *element_index,
					    long      attr_name_len)
{
  rul__ascic_to_asciz (tmpstr1, attr_name, attr_name_len);
  return (rul__get_comp_elem_atom (*wme_id, tmpstr1, *element_index)) ;
}

#endif



/*************************
**                      **
**   RUL_GET_INSTANCE   **
**                      **
*************************/

#ifdef C_BIND

long rul_get_instance (char     *char_buffer,
		       long      buffer_len,
		       rul_atom  wme_id)

{
  return (rul__get_wme (char_buffer, buffer_len, wme_id)) ;
}
				
#elif defined(VMS_BIND)

long RUL_GET_INSTANCE         (String   *buffer_dsc, 
			       long     *buffer_len,
			       Molecule *wme_id)
{
  return (rul__get_wme (buffer_dsc->dsc_a_pointer,
			buffer_dsc->dsc_w_length,
			*wme_id)) ;
}

#elif defined(F77_BIND)

long rul_get_instance_         (char     *char_buffer,
				long     *buffer_len,
				Molecule *wme_id,
				long char_buffer_len)
     
{
  return (rul__get_wme (char_buffer, *buffer_len, *wme_id)) ;
}

#endif



/********************************
**         	               **
**   RUL_GET_INSTANCE_LENGTH   **
**                             **
********************************/

#ifdef C_BIND

long rul_get_instance_length	   (rul_atom wme_id)
{
  return (rul__get_wme_length (wme_id)) ;
}
				
#elif defined(VMS_BIND)

long RUL_GET_INSTANCE_LENGTH       (Molecule *wme_id)
{
  return (rul__get_wme_length (*wme_id)) ;
}

#elif defined(F77_BIND)

long rul_get_instance_length_      (Molecule *wme_id)
{
  return (rul__get_wme_length (*wme_id)) ;
}

#endif



/****************************
**                         **
**  RUL_GET_NEXT_INSTANCE  **
**                         **
****************************/

#ifdef C_BIND

rul_atom rul_get_next_instance (rul_atom wme_id)
{
  return (rul__get_next_wme (wme_id)) ;
}
				
#elif defined(VMS_BIND)

Molecule RUL_GET_NEXT_INSTANCE (Molecule *wme_id)
{
  return (rul__get_next_wme (*wme_id)) ;
}

#elif defined(F77_BIND)

Molecule rul_get_next_instance_ (Molecule *wme_id)
{
  return (rul__get_next_wme (*wme_id)) ;
}

#endif
