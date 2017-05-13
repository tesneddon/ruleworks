/****************************************************************************
**                                                                         **
**                R T S _ C B _ A T O M _ T A B L E . C                    **
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
 *	interacting with the Molecule Table.
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
 *	 4-Jun-1991	DEC	Initial version
 *
 *	11-Jul-1991	DEC	Added:	ops_atom_to_string
 *					rul__atom_to_string
 *					ops_string_to_atom
 *
 *	17-Sep-1991	DEC	Changed interface to OPS$$ATOM_FROM_STRING.
 *
 *	18-Sep-1991	DEC	Changed ops_string_to_atom to return
 *					RUL_C_INVALID_ATOM if no atom found.
 *
 *	25-Sep-1991	DEC	Moved all functionality to helper routines
 *					in RTS_C_COMMON.C
 *
 *	23-Dec-1991	DEC	Fixed VMS OPS_ATOM_TO_STRING to use buffer_size
 *					argument instead of inferring from descriptor.
 *
 *	25-Sep-1997	DEC	#pragma module directives added to give 
 *			        different module names.
 *
 *	30-Mar-1998	DEC	Adding rul_atom_is_oatom function
 *
 *	31-mAR-1998	DEC	Adding rul_oatom_to_ptr and 
 *					rul_ptr_to_oatom
 *
 *	01-Dec-1999	CPQ	Release with GPL
 */


/*	Table of contents:

	Molecule Predicate Routines --

                rul_boolean  rul_atom_is_fatom       (atom_value)
                rul_boolean  rul_atom_is_iatom       (atom_value)
                rul_boolean  rul_atom_is_symbol      (atom_value)
                rul_boolean  rul_atom_is_compound    (atom_value)
                rul_boolean  rul_atom_is_instance_id (atom_value)
                rul_boolean  rul_atom_is_oatom       (atom_value)

	Molecule Conversion Routines --

                float    rul_fatom_to_float     (atom_value)
                double   rul_fatom_to_double    (atom_value)
                rul_atom rul_float_to_fatom     (float_value)
                rul_atom rul_double_to_fatom    (float_value)
		rul_atom rul_gensym		(void)
		rul_atom rul_gensymp		(char_string)
		rul_atom rul_genint		(void)
		long	 rul_iatom_to_integer	(atom_value)
		rul_atom rul_integer_to_iatom	(long_value)
		void *	 rul_oatom_to_ptr	(atom_value)
		rul_atom rul_ptr_to_oatom	(void *)
		rul_atom rul_string_to_symbol	(char_string)
		long 	 rul_symbol_to_string	(char_buffer, long, atom_value)
		rul_atom rul_string_to_atom	(char_string)
		long 	 rul_atom_to_string	(char_buffer, long, atom_value)
		long 	 rul_atom_to_string_length(atom_value)
		rul_atom rul_string_to_compound (char_buffer)
*/

#ifdef LOWERCASE_C_BIND
#pragma module RTS_CB_ATOM_TABLE
#define USE_LOWERCASE_BINDINGS
#define C_BIND
#elif defined(UPPERCASE_C_BIND)
#pragma module RTS_CB_ATOM_TABLE_C
#define C_BIND
#elif defined(VMS_BIND)
#pragma module RTS_CB_ATOM_TABLE_VMS
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

static char tmpstr[RUL_C_MAX_SYMBOL_SIZE+1];



/************************
**                     **
**  RUL_ATOM_IS_FATOM  **
**                     **
************************/

#ifdef C_BIND

    rul_boolean	 rul_atom_is_fatom	(rul_atom atom_value)
    {
      return (rul__atom_is_fatom(atom_value));
    }

#elif defined(VMS_BIND)

    Boolean	 RUL_ATOM_IS_FATOM	(Molecule *atom_value)
    {
      return (rul__atom_is_fatom(*atom_value));
    }

#elif defined(F77_BIND)

    Boolean	 rul_atom_is_fatom_	(Molecule *atom_value)
    {
      return (rul__atom_is_fatom(*atom_value));
    }

#endif



/************************
**                     **
**  RUL_ATOM_IS_IATOM  **
**                     **
************************/

#ifdef C_BIND

    rul_boolean	 rul_atom_is_iatom	(rul_atom atom_value)
    {
      return (rul__atom_is_iatom(atom_value));
    }

#elif defined(VMS_BIND)

    Boolean	 RUL_ATOM_IS_IATOM	(Molecule *atom_value)
    {
      return (rul__atom_is_iatom(*atom_value));
    }

#elif defined(F77_BIND)

    Boolean	 rul_atom_is_iatom_	(Molecule *atom_value)
    {
      return (rul__atom_is_iatom(*atom_value));
    }

#endif



/*************************
**                      **
**  RUL_ATOM_IS_SYMBOL  **
**                      **
*************************/

#ifdef C_BIND

    rul_boolean	 rul_atom_is_symbol	(rul_atom atom_value)
    {
      return (rul__atom_is_symbol(atom_value));
    }

#elif defined(VMS_BIND)

    Boolean	 RUL_ATOM_IS_SYMBOL	(Molecule *atom_value)
    {
      return (rul__atom_is_symbol(*atom_value));
    }

#elif defined(F77_BIND)

    Boolean	 rul_atom_is_symbol_	(Molecule *atom_value)
    {
      return (rul__atom_is_symbol(*atom_value));
    }

#endif



/***************************
**                        **
**  RUL_ATOM_IS_COMPOUND  **
**                        **
***************************/

#ifdef C_BIND

    rul_boolean	 rul_atom_is_compound	(rul_atom atom_value)
    {
      return (rul__atom_is_compound(atom_value));
    }

#elif defined(VMS_BIND)

    Boolean	 RUL_ATOM_IS_COMPOUND	(Molecule *atom_value)
    {
      return (rul__atom_is_compound(*atom_value));
    }

#elif defined(F77_BIND)

    Boolean	 rul_atom_is_compound_	(Molecule *atom_value)
    {
      return (rul__atom_is_compound(*atom_value));
    }

#endif



/*********************************
**				**
**  RUL_ATOM_IS_INSTANCE_ID	**
**				**
*********************************/

#ifdef C_BIND

    rul_boolean	 rul_atom_is_instance_id	(rul_atom atom_value)
    {
      return (rul__atom_is_instance_id(atom_value));
    }

#elif defined(VMS_BIND)

    Boolean	 RUL_ATOM_IS_INSTANCE_ID	(Molecule *atom_value)
    {
      return (rul__atom_is_instance_id(*atom_value));
    }

#elif defined(F77_BIND)

    Boolean	 rul_atom_is_instance_id_	(Molecule *atom_value)
    {
      return (rul__atom_is_instance_id(*atom_value));
    }

#endif



/************************
**                     **
**  RUL_ATOM_IS_OATOM **
**                     **
************************/

#ifdef C_BIND

    rul_boolean	 rul_atom_is_oatom	(rul_atom atom_value)
    {
      return (rul__atom_is_oatom(atom_value));
    }

#elif defined(VMS_BIND)

    Boolean	 RUL_ATOM_IS_OATOM	(Molecule *atom_value)
    {
      return (rul__atom_is_oatom(*atom_value));
    }

#elif defined(F77_BIND)

    Boolean	 rul_atom_is_oatom_	(Molecule *atom_value)
    {
      return (rul__atom_is_oatom(*atom_value));
    }

#endif



/*************************
**                      **
**  RUL_FATOM_TO_FLOAT  **
**                      **
*************************/

#ifdef C_BIND

    float	 rul_fatom_to_float	(rul_atom atom_value)
    {
      return (rul__fatom_to_float(atom_value));
    }

#elif defined(VMS_BIND)

    float	 RUL_FATOM_TO_FLOAT	(Molecule *atom_value)
    {
      return (rul__fatom_to_float(*atom_value));
    }

#elif defined(F77_BIND)

    float	 rul_fatom_to_float_	(Molecule *atom_value)
    {
      return (rul__fatom_to_float(*atom_value));
    }

#endif

/*************************
**                      **
**  RUL_FATOM_TO_DOUBLE **
**                      **
*************************/

#ifdef C_BIND

    double	 rul_fatom_to_double	(rul_atom atom_value)
    {
      return (rul__fatom_to_double(atom_value));
    }

#elif defined(VMS_BIND)

    double	 RUL_FATOM_TO_DOUBLE	(Molecule *atom_value)
    {
      return (rul__fatom_to_double(*atom_value));
    }

#elif defined(F77_BIND)

    double	 rul_fatom_to_double_	(Molecule *atom_value)
    {
      return (rul__fatom_to_double(*atom_value));
    }

#endif



/*************************
**                      **
**  RUL_FLOAT_TO_FATOM  **
**                      **
*************************/

#ifdef C_BIND

    rul_atom	 rul_float_to_fatom	(float float_value)
    {
      return (rul__float_to_fatom(float_value));
    }

#elif defined(VMS_BIND)

    Molecule	 RUL_FLOAT_TO_FATOM	(float *float_value)
    {
      return (rul__float_to_fatom(*float_value));
    }

#elif defined(F77_BIND)

    Molecule	 rul_float_to_fatom_	(float *float_value)
    {
      return (rul__float_to_fatom(*float_value));
    }

#endif


/*************************
**                      **
**  RUL_DOUBLE_TO_FATOM  **
**                      **
*************************/

#ifdef C_BIND

    rul_atom	 rul_double_to_fatom	(double float_value)
    {
      return (rul__double_to_fatom(float_value));
    }

#elif defined(VMS_BIND)

    Molecule	 RUL_DOUBLE_TO_FATOM	(double *float_value)
    {
      return (rul__double_to_fatom(*float_value));
    }

#elif defined(F77_BIND)

    Molecule	 rul_double_to_fatom_	(double *float_value)
    {
      return (rul__double_to_fatom(*float_value));
    }

#endif



/*****************
**              **
**  RUL_GENSYM  **
**              **
*****************/

#ifdef C_BIND

    rul_atom	 rul_gensym	(void)
    {
	return (rul__gensym (NULL));
    }

#elif defined(VMS_BIND)

    Molecule	 RUL_GENSYM	(void)
    {
	return (rul__gensym (NULL));
    }

#elif defined(F77_BIND)

    Molecule	 rul_gensym_	(void)
    {
	return (rul__gensym (NULL));
    }

#endif



/******************
**               **
**  RUL_GENSYMP  **
**               **
******************/

#ifdef C_BIND

    rul_atom	 rul_gensymp	(char *char_string)
    {
	return (rul__gensym (rul__string_to_symbol (char_string)));
    }

#elif defined(VMS_BIND)

    Molecule	 RUL_GENSYMP	(String *char_string)
    {
        rul__ascic_to_asciz (tmpstr,
			     char_string->dsc_a_pointer,
			     char_string->dsc_w_length);
	return (rul__gensym (rul__string_to_symbol (tmpstr)));
    }

#elif defined(F77_BIND)

    Molecule	 rul_gensymp_	(char *char_string, long char_count)
    {
        rul__ascic_to_asciz (tmpstr, char_string, (short) char_count);
	return (rul__gensym (rul__string_to_symbol (tmpstr)));
    }

#endif


/*****************
**              **
**  RUL_GENINT  **
**              **
*****************/

#ifdef C_BIND

    rul_atom	 rul_genint	(void)
    {
	return (rul__genint ());
    }

#elif defined(VMS_BIND)

    Molecule	 RUL_GENINT	(void)
    {
	return (rul__genint ());
    }

#elif defined(F77_BIND)

    Molecule	 rul_genint_	(void)
    {
	return (rul__genint ());
    }

#endif



/***************************
**                        **
**  RUL_IATOM_TO_INTEGER  **
**                        **
***************************/

#ifdef C_BIND

    long	 rul_iatom_to_integer	(rul_atom atom_value)
    {
      return (rul__iatom_to_integer (atom_value));
    }

#elif defined(VMS_BIND)

    long	 RUL_IATOM_TO_INTEGER	(Molecule *atom_value)
    {
      return (rul__iatom_to_integer (*atom_value));
    }

#elif defined(F77_BIND)

    long	 rul_iatom_to_integer_	(Molecule *atom_value)
    {
      return (rul__iatom_to_integer (*atom_value));
    }

#endif



/***************************
**                        **
**  RUL_INTEGER_TO_IATOM  **
**                        **
***************************/

#ifdef C_BIND

    rul_atom	 rul_integer_to_iatom	(long long_value)
    {
      return (rul__integer_to_iatom (long_value));
    }

#elif defined(VMS_BIND)

    Molecule	 RUL_INTEGER_TO_IATOM	(long *long_value)
    {
      return (rul__integer_to_iatom (*long_value));
    }

#elif defined(F77_BIND)

    Molecule	 rul_integer_to_iatom_	(long *long_value)
    {
      return (rul__integer_to_iatom (*long_value));
    }

#endif



/***************************
**                        **
**  RUL_OATOM_TO_PTR      **
**                        **
***************************/

#ifdef C_BIND

    void *	 rul_oatom_to_ptr	(rul_atom atom_value)
    {
      return (rul__oatom_to_ptr (atom_value));
    }

#elif defined(VMS_BIND)

    void *	 RUL_OATOM_TO_PTR	(Molecule *atom_value)
    {
      return (rul__oatom_to_ptr (*atom_value));
    }

#elif defined(F77_BIND)

    void *	 rul_oatom_to_ptr_	(Molecule *atom_value)
    {
      return (rul__oatom_to_ptr (*atom_value));
    }

#endif



/***************************
**                        **
**  RUL_PTR_TO_OATOM      **
**                        **
***************************/

#ifdef C_BIND

    rul_atom	 rul_ptr_to_oatom	(void * ptr_value)
    {
      return (rul__ptr_to_oatom (ptr_value));
    }

#elif defined(VMS_BIND)

    Molecule	 RUL_PTR_TO_OATOM	(void  *ptr_value)
    {
      return (rul__ptr_to_oatom (ptr_value));
    }

#elif defined(F77_BIND)

    Molecule	 rul_ptr_to_oatom_	(void *ptr_value)
    {
      return (rul__ptr_to_oatom (ptr_value));
    }

#endif



/***************************
**                        **
**  RUL_STRING_TO_SYMBOL  **
**                        **
***************************/

#ifdef C_BIND

    rul_atom	 rul_string_to_symbol	(char *char_string)
    {
      return (rul__string_to_symbol (char_string));
    }

#elif defined(VMS_BIND)

    Molecule	 RUL_STRING_TO_SYMBOL	(String *char_string)
    {
      rul__ascic_to_asciz (tmpstr,
			   char_string->dsc_a_pointer,
			   char_string->dsc_w_length);
      return (rul__string_to_symbol (tmpstr));
    }

#elif defined(F77_BIND)

    Molecule	 rul_string_to_symbol_	(char *char_string, long char_count)
    {
      rul__ascic_to_asciz (tmpstr, char_string, (short) char_count);
      return (rul__string_to_symbol (tmpstr));
    }

#endif


/***************************
**                        **
**  RUL_SYMBOL_TO_STRING  **
**                        **
***************************/

#ifdef C_BIND

    long	rul_symbol_to_string	(char 	  *char_buffer,
					 long 	   buffer_size,
					 rul_atom  atom_value)
    {
      return (rul__symbol_to_string(char_buffer, buffer_size, atom_value));
    }

#elif defined(VMS_BIND)

    long	RUL_SYMBOL_TO_STRING	(String   *char_str,
					 long	  *buffer_size,
					 Molecule *atom_value)
    {
      if (char_str->dsc_b_class == DSC_K_CLASS_S &&
	  char_str->dsc_b_dtype == DSC_K_DTYPE_T) {
	return (rul__symbol_to_string (char_str->dsc_a_pointer,
				       char_str->dsc_w_length,
				       *atom_value));
	}
      else {
	return (RUL_C_INVALID_LENGTH);
      }
    }

#elif defined(F77_BIND)

    long	rul_symbol_to_string_	(char 	  *char_buffer,
					 long 	  *buffer_size,
					 Molecule *atom_value,
					 long	   ignored_string_length)
    {
      return (rul__symbol_to_string (char_buffer, *buffer_size, *atom_value));
    }

#endif


/*************************
**                      **
**  RUL_STRING_TO_ATOM  **
**                      **
*************************/

#ifdef C_BIND

    rul_atom	 rul_string_to_atom	(char *char_string)
    {
	return (rul__string_to_atom (char_string));
    }

#elif defined(VMS_BIND)

    Molecule	 RUL_STRING_TO_ATOM	(String *char_string)
    {
      rul__ascic_to_asciz (tmpstr,
			   char_string->dsc_a_pointer,
			   char_string->dsc_w_length);
      return (rul__string_to_atom (tmpstr));
    }

#elif defined(F77_BIND)

    Molecule	 rul_string_to_atom_	(char *char_string, long char_count)
    {
      rul__ascic_to_asciz (tmpstr, char_string, (short) char_count);
      return (rul__string_to_atom (tmpstr));
    }

#endif



/*************************
**                      **
**  RUL_ATOM_TO_STRING  **
**                      **
*************************/

#ifdef C_BIND

    long	rul_atom_to_string	(char     *char_buffer,
					 long 	   buffer_size,
					 rul_atom  atom_value)
    {
      return (rul__atom_to_string (char_buffer, buffer_size, atom_value));
    }

#elif defined(VMS_BIND)

    long	RUL_ATOM_TO_STRING	(String   *char_str,
					 long	  *buffer_size,
					 Molecule *atom_value)
    {
      if (char_str->dsc_b_class == DSC_K_CLASS_S &&
	  char_str->dsc_b_dtype == DSC_K_DTYPE_T) {
	return (rul__atom_to_string (char_str->dsc_a_pointer,
				     char_str->dsc_w_length,
				     *atom_value));
	}
      else {
	return (RUL_C_INVALID_LENGTH);
      }
    }

#elif defined(F77_BIND)

    long	rul_atom_to_string_	(char 	  *char_buffer,
					 long 	  *buffer_size,
					 Molecule *atom_value,
					 long	   ignored_string_length)
    {
      return (rul__atom_to_string (char_buffer, *buffer_size, *atom_value));
    }

#endif



/*******************************
**                            **
**  RUL_ATOM_TO_STRING_LENGTH **
**                            **
*******************************/

#ifdef C_BIND

long	rul_atom_to_string_length	(rul_atom  atom_value)
{
  return (rul__get_atom_string_length(atom_value));
}

#elif defined(VMS_BIND)

long	RUL_ATOM_TO_STRING_LENGTH	(Molecule *atom_value)
{
  return (rul__get_atom_string_length(*atom_value));
}

#elif defined(F77_BIND)

long	rul_atom_to_string_length_	(Molecule *atom_value)
{
  return (rul__get_atom_string_length(*atom_value));
}

#endif



/*************************
**                      **
**  RUL_STRING_TO_COMPOUND  **
**                      **
*************************/

#ifdef C_BIND

    rul_atom	 rul_string_to_compound	(char *char_string)
    {
	return (rul__string_to_compound (char_string));
    }

#elif defined(VMS_BIND)

    Molecule	 RUL_STRING_TO_COMPOUND	(String *char_string)
    {
      rul__ascic_to_asciz (tmpstr,
			   char_string->dsc_a_pointer,
			   char_string->dsc_w_length);
      return (rul__string_to_compound (tmpstr));
    }

#elif defined(F77_BIND)

    Molecule	 rul_string_to_compound_ (char *char_string, long char_count)
    {
      rul__ascic_to_asciz (tmpstr, char_string, (short) char_count);
      return (rul__string_to_compound (tmpstr));
    }

#endif

