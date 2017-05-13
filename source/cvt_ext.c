/***************************************************************************
**                                                                        **
**                     R T S _ C V T _ E X T . C                          **
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
***************************************************************************/

/*
**
**  FACILITY:
**      RULEWORKS run time system
**
**  ABSTRACT:
**      This module contains the functions used in the generated
**	code that convert between external types and RULEWORKS types
**
**      The exported interface descriptions are in the header file, cvt.h.
**
**  MODIFIED BY:
**      DEC	Digital Equipment Corporation
**		CPQ	Compaq Computer Corporation
**
**  MODIFICATION HISTORY:
**
**  	23-Jun-1992     DEC	Initial version
**
**	28-Oct-1993	DEC	additions and general fixes
**
**	31-May-1994	DEC	Fix calls to rul__msg_print() to be to
**					rul__msg_print_w_atoms().
**
**	11-Jul-1998	DEC	Only increment once in cvt_s_at_s_ta not twice
**	01-Dec-1999	CPQ	Release with GPL
**
*/



/***************************************************************************

  This module contains routines to convert from RULEWORKS types to external
  types and from external types to RULEWORKS types.

  The conversion routines in this module are named with the convention:
RUL__CVT_M1_T1_M2_T2
        where:  CVT is the run-time subsystem identifier.
                M1 is the type modifier for the type being converted
       from.
                T1 is an abbreviation for the type being converted from.
                M2 is the type modifier for the type being converted to.
                T2 is an abbreviation for the type being converted to.
 Type Modifiers                    Abbreviation
        Scalar value                    S
        Array value                     A
        Compound value                  C
        Table value                     T
 RULEWORKS Types                   Abbreviation
        Symbol atom                     SY
        Integer atom                    IN
        Double (float) atom             DB
        Opaque atom                     OP
        Any type (variable)             AT
 External Types                    Abbreviation
        long (signed)                   LO
        short (signed)                  SH
        byte (signed)                   BY
        unsigned long                   UL
        unsigned short                  US
        unsigned byte                   UB
        single float                    SF
        double float                    DF
        asciz                           AZ
        ascid                           AD
        opaque (atom)                   VD
        tin atom (RuleWorks)            TA

  All the routines that with rul__cvt_*
  use one of the following argument lists:


  Routines which convert a scalar RULEWORKS type to a scalar external type
    take as arguments: ( an-atom )

  Routines which convert a scalar external type to a scalar RULEWORKS type
    take as arguments: ( an-ext-type )

  Routines which convert a scalar RULEWORKS symbol to an external string
    take as arguments: ( an-atom )

  Routines which convert an external string to a scalar RULEWORKS symbol
    take as arguments: ( an-ext-ptr )

  Routines which convert a compound tin type to an external array
    take as arguments: ( an-rul-ptr, array-len )

  Routines which convert an external array to a tin compound variable
    take as arguments: ( an-ext-ptr, array-len)

*****************************************************************************/

#include <limits.h>
#include <stdlib.h>
#include <common.h>
#include <cvt.h>
#include <dsc.h>
#include <mol.h>
#include <msg.h>
#include <rts_msg.h>
#include <ios.h>

extern Molecule rul__string_to_atom (char *char_string);
extern Molecule rul__float_to_fatom (float float_value);

static char  *get_molecule_type_string (Molecule tin_atom);
static char   tmp_buf[RUL_C_MAX_SYMBOL_SIZE+1];
static long   tmp_int;
static double tmp_dbl;

/*****************************
*  NUMERIC ATOM CONVERSIONS  *
*****************************/

/*****************************************************************************
	RUL__CVT_S_IN_S_LO

    Convert from an Integer Atom to Long

*****************************************************************************/

long
rul__cvt_s_in_s_lo (Mol_Int_Atom tin_atom)
{

#ifdef DEBUG
  rul__ios_printf (RUL__C_STD_ERR,"\nRUL__CVT_S_NA_S_LO    %p",tin_atom);
#endif

  return (rul__mol_int_atom_value (tin_atom));
}


/*****************************************************************************
	RUL__CVT_S_LO_S_IN

    Convert from Long to an Integer Atom

*****************************************************************************/

Mol_Int_Atom
rul__cvt_s_lo_s_in (long ext_obj)
{

#ifdef DEBUG
  rul__ios_printf (RUL__C_STD_ERR,"\nRUL__CVT_S_LO_S_IN    %ld",ext_obj);
#endif

  return (rul__mol_make_int_atom (ext_obj));
}

/*****************************************************************************
	RUL__CVT_S_IN_S_SH

    Convert from Integer Atom to Short

*****************************************************************************/

short
rul__cvt_s_in_s_sh (Mol_Int_Atom tin_atom)
{
  long value;

#ifdef DEBUG
  rul__ios_printf (RUL__C_STD_ERR,"\nRUL__CVT_S_IN_S_SH    %p",tin_atom);
#endif

  value = rul__mol_int_atom_value (tin_atom) ;
  if ((value < SHRT_MIN) || (value > SHRT_MAX)) {
    rul__msg_print (RTS_INVCVTSH);
    return ((short) 0);
  }
  else
    return ((short) value);
}


/*****************************************************************************
	RUL__CVT_S_SH_S_IN

    Convert from Short to Integer Atom

*****************************************************************************/

Mol_Int_Atom
rul__cvt_s_sh_s_in (short ext_obj)
{

#ifdef DEBUG
  rul__ios_printf (RUL__C_STD_ERR,"\nRUL__CVT_S_SH_S_IN    %hd",ext_obj);
#endif

  return (rul__mol_make_int_atom ((long) ext_obj)) ;
}

/*****************************************************************************
	RUL__CVT_S_IN_S_BY

    Convert from Integer Atom to Byte

*****************************************************************************/

char
rul__cvt_s_in_s_by (Mol_Int_Atom tin_atom)
{
  long value;

#ifdef DEBUG
  rul__ios_printf (RUL__C_STD_ERR,"\nRUL__CVT_S_IN_S_BY    %p",tin_atom);
#endif

  value = rul__mol_int_atom_value (tin_atom) ;
  if ((value < SCHAR_MIN) || (value > SCHAR_MAX)) {
    rul__msg_print (RTS_INVCVTBY);
    return ((char) 0);
  }
  else
    return ((char) value);
}


/*****************************************************************************
	RUL__CVT_S_BY_S_IN

    Convert from Byte to Integer Atom

*****************************************************************************/

Mol_Int_Atom
rul__cvt_s_by_s_in (char ext_obj)
{

#ifdef DEBUG
  rul__ios_printf (RUL__C_STD_ERR,"\nRUL__CVT_S_BY_S_IN    %d",ext_obj);
#endif

  return (rul__mol_make_int_atom ((long) ext_obj)) ;
}

/*****************************************************************************
	RUL__CVT_S_IN_S_UL

    Convert from Integer Atom to Unsigned Long

*****************************************************************************/

unsigned long
rul__cvt_s_in_s_ul (Mol_Int_Atom tin_atom)
{

#ifdef DEBUG
  rul__ios_printf (RUL__C_STD_ERR,"\nRUL__CVT_S_IN_S_UL    %p",tin_atom);
#endif

  return ((unsigned long) rul__mol_int_atom_value (tin_atom));
}


/*****************************************************************************
	RUL__CVT_S_UL_S_IN

    Convert from Unsigned Long to Integer Atom

*****************************************************************************/

Mol_Int_Atom
rul__cvt_s_ul_s_in (unsigned long ext_obj)
{

#ifdef DEBUG
  rul__ios_printf (RUL__C_STD_ERR,"\nRUL__CVT_S_UL_S_IN    %lu",ext_obj);
#endif

  if (ext_obj > LONG_MAX) {
    rul__msg_print (RTS_INVCVTIN);
  }
  return (rul__mol_make_int_atom ((long) ext_obj));
}

/*****************************************************************************
	RUL__CVT_S_IN_S_US

    Convert from Integer Atom to Unsigned Short

*****************************************************************************/

unsigned short
rul__cvt_s_in_s_us (Mol_Int_Atom tin_atom)
{
  long value;

#ifdef DEBUG
  rul__ios_printf (RUL__C_STD_ERR,"\nRUL__CVT_S_IN_S_US    %p",tin_atom);
#endif

  value = rul__mol_int_atom_value (tin_atom) ;
  if ((value < 0) || (value > USHRT_MAX)) {
    rul__msg_print (RTS_INVCVTUS);
    return ((unsigned short) 0);
  }
  else
    return ((unsigned short) value);
}


/*****************************************************************************
	RUL__CVT_S_US_S_IN

    Convert from Unsigned Short to Integer Atom

*****************************************************************************/

Mol_Int_Atom
rul__cvt_s_us_s_in (unsigned short ext_obj)
{

#ifdef DEBUG
  rul__ios_printf (RUL__C_STD_ERR,"\nRUL__CVT_S_US_S_IN    %hu",ext_obj);
#endif

  return (rul__mol_make_int_atom ((unsigned long) ext_obj));
}

/*****************************************************************************
	RUL__CVT_S_IN_S_UB

    Convert from Integer Atom to Unsigned Byte

*****************************************************************************/

unsigned char
rul__cvt_s_in_s_ub (Mol_Int_Atom tin_atom)
{
  long value;

#ifdef DEBUG
  rul__ios_printf (RUL__C_STD_ERR,"\nRUL__CVT_S_IN_S_UB    %p",tin_atom);
#endif

  value = rul__mol_int_atom_value (tin_atom) ;
  if ((value < 0) || (value > UCHAR_MAX)) {
    rul__msg_print (RTS_INVCVTUB);
    return ((unsigned char) 0);
  }
  else
    return ((unsigned char) value);
}


/*****************************************************************************
	RUL__CVT_S_UB_S_IN

    Convert from Unsigned Byte to Integer Atom

*****************************************************************************/

Mol_Int_Atom
rul__cvt_s_ub_s_in (unsigned char ext_obj)
{

#ifdef DEBUG
  rul__ios_printf (RUL__C_STD_ERR,"\nRUL__CVT_S_UB_S_IN    %u",ext_obj);
#endif

  return (rul__mol_make_int_atom ((unsigned long) ext_obj));
}

/*****************************************************************************
	RUL__CVT_S_IN_S_SF

    Convert from Integer Atom to Single Float

*****************************************************************************/

float
rul__cvt_s_in_s_sf (Mol_Int_Atom tin_atom)
{

#ifdef DEBUG
  rul__ios_printf (RUL__C_STD_ERR,"\nRUL__CVT_S_IN_S_SF    %p",tin_atom);
#endif

  return ((float) rul__mol_int_atom_value (tin_atom));
}


/*****************************************************************************
	RUL__CVT_S_SF_S_IN

    Convert from Single Float to Integer Atom

*****************************************************************************/

Mol_Int_Atom
rul__cvt_s_sf_s_in (float ext_obj)
{

#ifdef DEBUG
  rul__ios_printf (RUL__C_STD_ERR,"\nRUL__CVT_S_SF_S_IN    %f",ext_obj);
#endif

  return (rul__mol_make_int_atom ((long) ext_obj));
}

/*****************************************************************************
	RUL__CVT_S_IN_S_DF

    Convert from Integer Atom to Double Float

*****************************************************************************/

double
rul__cvt_s_in_s_df (Mol_Int_Atom tin_atom)
{

#ifdef DEBUG
  rul__ios_printf (RUL__C_STD_ERR,"\nRUL__CVT_S_IN_S_DF    %p",tin_atom);
#endif

  return ((double) rul__mol_int_atom_value (tin_atom));
}


/*****************************************************************************
	RUL__CVT_S_DF_S_IN

    Convert from Double Float to Integer Atom

*****************************************************************************/

Mol_Int_Atom
rul__cvt_s_df_s_in (double ext_obj)
{

#ifdef DEBUG
  rul__ios_printf (RUL__C_STD_ERR,"\nRUL__CVT_S_DF_S_IN    %g",ext_obj);
#endif

  return (rul__mol_make_int_atom ((long) ext_obj));
}


/***************************
*  FLOAT ATOM CONVERSIONS  *
***************************/

/*****************************************************************************
	RUL__CVT_S_DB_S_LO

    Convert from Float Atom to Long

*****************************************************************************/

long
rul__cvt_s_db_s_lo (Mol_Dbl_Atom tin_atom)
{
  double real ;

#ifdef DEBUG
  rul__ios_printf (RUL__C_STD_ERR,"\nRUL__CVT_S_DB_S_LO    %p",tin_atom);
#endif

  real = rul__mol_dbl_atom_value (tin_atom) + .5 ;
  return ((long) real);
}


/*****************************************************************************
	RUL__CVT_S_LO_S_DB

    Convert from Long to Float Atom

*****************************************************************************/

Mol_Dbl_Atom
rul__cvt_s_lo_s_db (long ext_obj)
{
  double real ;

#ifdef DEBUG
  rul__ios_printf (RUL__C_STD_ERR,"\nRUL__CVT_S_LO_S_DB    %ld",ext_obj);
#endif

  real = ((double) ext_obj) ;
  return (rul__mol_make_dbl_atom(real));
}

/*****************************************************************************
	RUL__CVT_S_DB_S_SH

    Convert from Float Atom to Short

*****************************************************************************/

short
rul__cvt_s_db_s_sh (Mol_Dbl_Atom tin_atom)
{
  double real ;

#ifdef DEBUG
  rul__ios_printf (RUL__C_STD_ERR,"\nRUL__CVT_S_DB_S_SH    %p",tin_atom);
#endif

  real = rul__mol_dbl_atom_value (tin_atom) + .5 ;
  return ((short) real) ;
}


/*****************************************************************************
	RUL__CVT_S_SH_S_DB

    Convert from Short to Float Atom

*****************************************************************************/

Mol_Dbl_Atom
rul__cvt_s_sh_s_db (short ext_obj)
{
  double real ;

#ifdef DEBUG
  rul__ios_printf (RUL__C_STD_ERR,"\nRUL__CVT_S_SH_S_DB    %hd",ext_obj);
#endif

  real = ((double) ext_obj) ;
  return (rul__mol_make_dbl_atom(real));
}

/*****************************************************************************
	RUL__CVT_S_DB_S_BY

    Convert from Float Atom to Byte

*****************************************************************************/

char
rul__cvt_s_db_s_by (Mol_Dbl_Atom tin_atom)
{
  double real ;

#ifdef DEBUG
  rul__ios_printf (RUL__C_STD_ERR,"\nRUL__CVT_S_DB_S_BY    %p",tin_atom);
#endif

  real = rul__mol_dbl_atom_value (tin_atom) + .5 ;
  return ((char) real);
}


/*****************************************************************************
	RUL__CVT_S_BY_S_DB

    Convert from Byte to Float Atom

*****************************************************************************/

Mol_Dbl_Atom
rul__cvt_s_by_s_db (char ext_obj)
{
  double real ;

#ifdef DEBUG
  rul__ios_printf (RUL__C_STD_ERR,"\nRUL__CVT_S_BY_S_DB    %d",ext_obj);
#endif

  real = ((double) ext_obj) ;
  return (rul__mol_make_dbl_atom(real));
}

/*****************************************************************************
	RUL__CVT_S_DB_S_UL

    Convert from Float Atom to Unsigned Long

*****************************************************************************/

unsigned long
rul__cvt_s_db_s_ul (Mol_Dbl_Atom tin_atom)
{
  double real;

#ifdef DEBUG
  rul__ios_printf (RUL__C_STD_ERR,"\nRUL__CVT_S_DB_S_UL    %p",tin_atom);
#endif

  real = rul__mol_dbl_atom_value(tin_atom) + .5 ;
  return ((unsigned long) real);
}


/*****************************************************************************
	RUL__CVT_S_UL_S_DB

    Convert from Unsigned Long to Float Atom

*****************************************************************************/

Mol_Dbl_Atom
rul__cvt_s_ul_s_db (unsigned long ext_obj)
{
  double real ;

#ifdef DEBUG
  rul__ios_printf (RUL__C_STD_ERR,"\nRUL__CVT_S_UL_S_DB    %lu",ext_obj);
#endif

  real = ((double) ext_obj) ;
  return (rul__mol_make_dbl_atom (real));
}

/*****************************************************************************
	RUL__CVT_S_DB_S_US

    Convert from Float Atom to Unsigned Short

*****************************************************************************/

unsigned short
rul__cvt_s_db_s_us (Mol_Dbl_Atom tin_atom)
{
  double real;

#ifdef DEBUG
  rul__ios_printf (RUL__C_STD_ERR,"\nRUL__CVT_S_DB_S_US    %p",tin_atom);
#endif

  real = rul__mol_dbl_atom_value(tin_atom) + .5 ;
  return ((unsigned short) real);
}


/*****************************************************************************
	RUL__CVT_S_US_S_DB

    Convert from Unsigned Short to Float Atom

*****************************************************************************/

Mol_Dbl_Atom
rul__cvt_s_us_s_db (unsigned short ext_obj)
{
  double real ;

#ifdef DEBUG
  rul__ios_printf (RUL__C_STD_ERR,"\nRUL__CVT_S_US_S_DB    %hu",ext_obj);
#endif

  real = ((double) ext_obj) ;
  return (rul__mol_make_dbl_atom (real));
}

/*****************************************************************************
	RUL__CVT_S_DB_S_UB

    Convert from Float Atom to Unsigned Byte

*****************************************************************************/

unsigned char
rul__cvt_s_db_s_ub (Mol_Dbl_Atom tin_atom)
{
  double real ;

#ifdef DEBUG
  rul__ios_printf (RUL__C_STD_ERR,"\nRUL__CVT_S_DB_S_UB    %p",tin_atom);
#endif

  real = rul__mol_dbl_atom_value(tin_atom) + .5 ;
  return ((unsigned char) real);
}


/*****************************************************************************
	RUL__CVT_S_UB_S_DB

    Convert from Unsigned Byte to Float Atom

*****************************************************************************/

Mol_Dbl_Atom
rul__cvt_s_ub_s_db (unsigned char ext_obj)
{
  double real ;

#ifdef DEBUG
  rul__ios_printf (RUL__C_STD_ERR,"\nRUL__CVT_S_UB_S_DB    %u",ext_obj);
#endif

  real = ((double) ext_obj) ;
  return (rul__mol_make_dbl_atom (real));
}

/*****************************************************************************
	RUL__CVT_S_DB_S_SF

    Convert from Float Atom to Single Float

*****************************************************************************/

float
rul__cvt_s_db_s_sf (Mol_Dbl_Atom tin_atom)
{

#ifdef DEBUG
  rul__ios_printf (RUL__C_STD_ERR,"\nRUL__CVT_S_DB_S_SF    %p",tin_atom);
#endif

  return (rul__mol_dbl_atom_value (tin_atom));
}


/*****************************************************************************
	RUL__CVT_S_SF_S_DB

    Convert from Single Float to Float Atom

*****************************************************************************/

Mol_Dbl_Atom
rul__cvt_s_sf_s_db (float ext_obj)
{

#ifdef DEBUG
  rul__ios_printf (RUL__C_STD_ERR,"\nRUL__CVT_S_SF_S_DB    %f",ext_obj);
#endif

  return (rul__float_to_fatom (ext_obj));
}

/*****************************************************************************
	RUL__CVT_S_DB_S_DF

    Convert from Float Atom to Double Float

*****************************************************************************/

double
rul__cvt_s_db_s_df (Mol_Dbl_Atom tin_atom)
{

#ifdef DEBUG
  rul__ios_printf (RUL__C_STD_ERR,"\nRUL__CVT_S_DB_S_DF    %p",tin_atom);
#endif

  if (rul__mol_is_dbl_atom (tin_atom))
    return (rul__mol_dbl_atom_value (tin_atom));

  else {
    rul__msg_print (RTS_INVCVTXXXX, get_molecule_type_string (tin_atom),
		    "DOUBLE");
    return ((double) 0.0);
  }
}


/*****************************************************************************
	RUL__CVT_S_DF_S_DB

    Convert from Double Float to Float Atom

*****************************************************************************/

Mol_Dbl_Atom
rul__cvt_s_df_s_db (double ext_obj)
{

#ifdef DEBUG
  rul__ios_printf (RUL__C_STD_ERR,"\nRUL__CVT_S_DF_S_DB    %g",ext_obj);
#endif

  return (rul__mol_make_dbl_atom (ext_obj));
}



/****************************
*  OPAQUE ATOM CONVERSIONS  *
****************************/

/*****************************************************************************
	RUL__CVT_S_OP_S_VD

    Convert from an Opaque Atom to external opaque

*****************************************************************************/

void *
rul__cvt_s_op_s_vd (Mol_Opaque tin_atom)
{

#ifdef DEBUG
  rul__ios_printf (RUL__C_STD_ERR,"\nRUL__CVT_S_OP_S_VD    %p",tin_atom);
#endif

  if (rul__mol_is_opaque (tin_atom))
    return (rul__mol_opaque_value (tin_atom));

  else {
    rul__msg_print (RTS_INVCVTXXXX, get_molecule_type_string (tin_atom),
		    "OPAQUE");
    return NULL;
  }
}


/*****************************************************************************
	RUL__CVT_S_VD_S_OP

    Convert from External to an Opaque Atom

*****************************************************************************/

Mol_Opaque
rul__cvt_s_vd_s_op (void *ext_obj )
{

#ifdef DEBUG
  rul__ios_printf (RUL__C_STD_ERR,"\nRUL__CVT_S_VD_S_OP    %p",ext_obj);
#endif
  
  return (rul__mol_make_opaque (ext_obj));
}


/******************************
*  SYMBOLIC ATOM CONVERSIONS  *
******************************/

/*****************************************************************************
	RUL__CVT_S_AT_S_AZ

    Convert from Untyped to Asciz string

*****************************************************************************/

char *
rul__cvt_s_at_s_az (Mol_Atom tin_atom, char *optr)
{
  /*
   **  Allocate max_symbol_size wotrth of space just in case this
   **  string being returned is being used as a read/write argument
   */
  char *buff;
  long  len;

#ifdef DEBUG
  rul__ios_printf (RUL__C_STD_ERR,"\nRUL__CVT_S_AT_S_AZ    %p",tin_atom);
#endif

  if (optr) {
    len = strlen (optr);
    buff = optr;
  }
  else {
    len = RUL_C_MAX_SYMBOL_SIZE;
    buff = (char *) rul__mem_malloc (len + 1);
  }
  rul__mol_use_printform (tin_atom, buff, len + 1);
  if (len < rul__mol_get_printform_length (tin_atom))
    rul__msg_print_w_atoms (CVT_STRTRUNC, 1, tin_atom);

  return buff;
}


/*****************************************************************************
	RUL__CVT_S_AZ_S_SY

    Convert from Asciz string to Symbolic Atom

*****************************************************************************/

Mol_Symbol
rul__cvt_s_az_s_sy (char *ext_pointer)
{

#ifdef DEBUG
  rul__ios_printf (RUL__C_STD_ERR,"\nRUL__CVT_S_AZ_S_SY    %s", ext_pointer);
#endif

  if (ext_pointer != NULL) {
    return (rul__mol_make_symbol (ext_pointer));
  }
  return (rul__mol_make_symbol (""));
}
/*****************************************************************************
	RUL__CVT_S_AZ_S_AT

    Convert from Asciz string to Untyped Atom

*****************************************************************************/

Mol_Atom
rul__cvt_s_az_s_at (char *ext_pointer)
{

#ifdef DEBUG
  rul__ios_printf (RUL__C_STD_ERR,"\nRUL__CVT_S_AZ_S_AT    %s", ext_pointer);
#endif

  if (ext_pointer != NULL) {
    return (rul__string_to_atom (ext_pointer));
  }
  return (rul__mol_make_symbol (""));
}

/*****************************************************************************
	RUL__CVT_S_AT_S_AD

    Convert from Untyped to Ascid string

*****************************************************************************/

String *
rul__cvt_s_at_s_ad (Mol_Atom tin_atom, String *optr)
{
  short     str_len;
  String   *descr;
  long      len;
  long	    dsc_size = sizeof (String);

#ifdef DEBUG
  rul__ios_printf (RUL__C_STD_ERR,"\nRUL__CVT_S_AT_S_AD    %p",tin_atom);
#endif

  str_len = rul__mol_get_printform_length (tin_atom);

  if (optr) {
    if (optr->dsc_w_length < str_len)
      rul__msg_print_w_atoms (CVT_STRTRUNC, 1, tin_atom);
    descr = optr;
    descr->dsc_w_length = str_len;
  }
  else {
    /*
     * Allocate space for descriptor
     */
    descr = (String *) rul__mem_malloc (dsc_size + str_len + 1);
    descr->dsc_w_length  = str_len;
    descr->dsc_b_dtype   = DSC_K_DTYPE_T;
    descr->dsc_b_class   = DSC_K_CLASS_S;
    descr->dsc_a_pointer = (char *) descr + dsc_size ;
  }
  rul__mol_use_printform (tin_atom, descr->dsc_a_pointer, str_len + 1);
  return (descr);
}


/*****************************************************************************
	RUL__CVT_S_AD_S_SY

    Convert from Ascid string to Symbolic Atom

*****************************************************************************/

Mol_Symbol
rul__cvt_s_ad_s_sy (String *descr)
{
  long	     len;

#ifdef DEBUG
  rul__ios_printf (RUL__C_STD_ERR,"\nRUL__CVT_S_AD_S_SY    %p", descr);
#endif

  if ((descr != NULL) && ((len = descr->dsc_w_length) > 0)) {
    len = MIN (len, RUL_C_MAX_SYMBOL_SIZE);
    strncpy (tmp_buf, descr->dsc_a_pointer, len);
    tmp_buf[len] = '\0';
    return (rul__mol_make_symbol (tmp_buf));
  }
  else {
    return (rul__mol_make_symbol (""));
  }
}
/*****************************************************************************
	RUL__CVT_S_AD_S_AT

    Convert from Ascid string to Untyped Atom

*****************************************************************************/

Mol_Atom
rul__cvt_s_ad_s_at (String *descr)
{
  long	     len;

#ifdef DEBUG
  rul__ios_printf (RUL__C_STD_ERR,"\nRUL__CVT_S_AD_S_AT    %p", descr);
#endif

  if ((descr != NULL) && ((len = descr->dsc_w_length) > 0)) {
    len = MIN (len, RUL_C_MAX_SYMBOL_SIZE);
    strncpy (tmp_buf, descr->dsc_a_pointer, len);
    tmp_buf[len] = '\0';
    return (rul__string_to_atom (tmp_buf));
  }
  else {
    return (rul__mol_make_symbol (""));
  }
}


/***********************************
*  CONVERSIONS FROM UNTYPED ATOMS  *
***********************************/

static char *
get_molecule_type_string (Molecule tin_atom)
{
  if (rul__mol_is_polyatom (tin_atom))	return "Poly Atom";
  if (rul__mol_is_opaque (tin_atom))		return "Opaque";
  if (rul__mol_is_instance_id (tin_atom))	return "Instance Id";
  if (rul__mol_is_number (tin_atom)) 		return "Numeric Atom";
  return "Symbol";
}

/*****************************************************************************
	RUL__CVT_S_AT_S_LO

    Convert from Untyped Atom to Long

*****************************************************************************/

long
rul__cvt_s_at_s_lo (Mol_Atom tin_atom)
{
  Molecule mol;

#ifdef DEBUG
  rul__ios_printf (RUL__C_STD_ERR,"\nRUL__CVT_S_AT_S_LO    %p",tin_atom);
#endif

  if (rul__mol_is_int_atom (tin_atom)) 
    return (rul__cvt_s_in_s_lo (tin_atom));

  else if (rul__mol_is_dbl_atom (tin_atom))
    return (rul__cvt_s_db_s_lo (tin_atom));

  else {
    rul__mol_use_printform (tin_atom, tmp_buf, RUL_C_MAX_SYMBOL_SIZE+1);
    mol = rul__string_to_atom (tmp_buf);

    if (rul__mol_is_int_atom (mol)) 
      return (rul__cvt_s_in_s_lo (mol));

    else if (rul__mol_is_dbl_atom (mol))
      return (rul__cvt_s_db_s_lo (mol));

    else {
      rul__msg_print (RTS_INVCVTXXXX, get_molecule_type_string (tin_atom),
		      "LONG");
      return ((long) 0);
    }
  }
}

/*****************************************************************************
	RUL__CVT_S_AT_S_SH

    Convert from Untyped Atom to Short

*****************************************************************************/

short
rul__cvt_s_at_s_sh (Mol_Atom tin_atom)
{
  Molecule mol;

#ifdef DEBUG
  rul__ios_printf (RUL__C_STD_ERR,"\nRUL__CVT_S_AT_S_SH    %p",tin_atom);
#endif

  if (rul__mol_is_int_atom (tin_atom)) 
    return (rul__cvt_s_in_s_sh (tin_atom));

  else if (rul__mol_is_dbl_atom (tin_atom))
    return (rul__cvt_s_db_s_sh (tin_atom));

  else {
    rul__mol_use_printform (tin_atom, tmp_buf, RUL_C_MAX_SYMBOL_SIZE+1);
    mol = rul__string_to_atom (tmp_buf);

    if (rul__mol_is_int_atom (mol)) 
      return (rul__cvt_s_in_s_sh (mol));

    else if (rul__mol_is_dbl_atom (mol))
      return (rul__cvt_s_db_s_sh (mol));

    else {
      rul__msg_print (RTS_INVCVTXXXX, get_molecule_type_string (tin_atom),
		      "SHORT");
      return ((short) 0);
    }
  }
}

/*****************************************************************************
	RUL__CVT_S_AT_S_BY

    Convert from Untyped Atom to Byte

*****************************************************************************/

char
rul__cvt_s_at_s_by (Mol_Atom tin_atom)
{
  Molecule mol;

#ifdef DEBUG
  rul__ios_printf (RUL__C_STD_ERR,"\nRUL__CVT_S_AT_S_BY    %p",tin_atom);
#endif

  if (rul__mol_is_int_atom (tin_atom)) 
    return (rul__cvt_s_in_s_by (tin_atom));

  else if (rul__mol_is_dbl_atom (tin_atom))
    return (rul__cvt_s_db_s_by (tin_atom));

  else {
    rul__mol_use_printform (tin_atom, tmp_buf, RUL_C_MAX_SYMBOL_SIZE+1);
    mol = rul__string_to_atom (tmp_buf);

    if (rul__mol_is_int_atom (mol)) 
      return (rul__cvt_s_in_s_by (mol));

    else if (rul__mol_is_dbl_atom (mol))
      return (rul__cvt_s_db_s_by (mol));

    else {
      rul__msg_print (RTS_INVCVTXXXX, get_molecule_type_string (tin_atom),
		      "BYTE");
      return ((char) 0);
    }
  }
}

/*****************************************************************************
	RUL__CVT_S_AT_S_UL

    Convert from Untyped Atom to Unsigned Long

*****************************************************************************/

unsigned long
rul__cvt_s_at_s_ul (Mol_Atom tin_atom)
{
  Molecule mol;

#ifdef DEBUG
  rul__ios_printf (RUL__C_STD_ERR,"\nRUL__CVT_S_AT_S_UL    %p",tin_atom);
#endif

  if (rul__mol_is_int_atom (tin_atom)) 
    return (rul__cvt_s_in_s_ul (tin_atom));

  else if (rul__mol_is_dbl_atom (tin_atom))
    return (rul__cvt_s_db_s_ul (tin_atom));

  else {
    rul__mol_use_printform (tin_atom, tmp_buf, RUL_C_MAX_SYMBOL_SIZE+1);
    mol = rul__string_to_atom (tmp_buf);

    if (rul__mol_is_int_atom (mol)) 
      return (rul__cvt_s_in_s_ul (mol));

    else if (rul__mol_is_dbl_atom (mol))
      return (rul__cvt_s_db_s_ul (mol));

    else {
      rul__msg_print (RTS_INVCVTXXXX, get_molecule_type_string (tin_atom),
		      "UNSIGNED-LONG");
      return ((unsigned long) 0);
    }
  }
}

/*****************************************************************************
	RUL__CVT_S_AT_S_US

    Convert from Untyped Atom to Unsigned Short

*****************************************************************************/

unsigned short
rul__cvt_s_at_s_us (Mol_Atom tin_atom)
{
  Molecule mol;

#ifdef DEBUG
  rul__ios_printf (RUL__C_STD_ERR,"\nRUL__CVT_S_AT_S_US    %p",tin_atom);
#endif

  if (rul__mol_is_int_atom (tin_atom)) 
    return (rul__cvt_s_in_s_us (tin_atom));

  else if (rul__mol_is_dbl_atom (tin_atom))
    return (rul__cvt_s_db_s_us (tin_atom));

  else {
    rul__mol_use_printform (tin_atom, tmp_buf, RUL_C_MAX_SYMBOL_SIZE+1);
    mol = rul__string_to_atom (tmp_buf);

    if (rul__mol_is_int_atom (mol)) 
      return (rul__cvt_s_in_s_us (mol));
    
    else if (rul__mol_is_dbl_atom (mol))
      return (rul__cvt_s_db_s_us (mol));
    
    else {
      rul__msg_print (RTS_INVCVTXXXX, get_molecule_type_string (tin_atom),
		      "UNSIGNED-SHORT");
      return ((unsigned short) 0);
    }
  }
}

/*****************************************************************************
	RUL__CVT_S_AT_S_UB

    Convert from Untyped Atom to Unsigned Byte

*****************************************************************************/

unsigned char
rul__cvt_s_at_s_ub (Mol_Atom tin_atom)
{
  Molecule mol;

#ifdef DEBUG
  rul__ios_printf (RUL__C_STD_ERR,"\nRUL__CVT_S_AT_S_UB    %p",tin_atom);
#endif

  if (rul__mol_is_int_atom (tin_atom)) 
    return (rul__cvt_s_in_s_by (tin_atom));

  else if (rul__mol_is_dbl_atom (tin_atom))
    return (rul__cvt_s_db_s_by (tin_atom));

  else {
    rul__mol_use_printform (tin_atom, tmp_buf, RUL_C_MAX_SYMBOL_SIZE+1);
    mol = rul__string_to_atom (tmp_buf);

    if (rul__mol_is_int_atom (mol)) 
      return (rul__cvt_s_in_s_by (mol));

    else if (rul__mol_is_dbl_atom (mol))
      return (rul__cvt_s_db_s_by (mol));

    else {
      rul__msg_print (RTS_INVCVTXXXX, get_molecule_type_string (tin_atom),
		      "UNSIGNED-BYTE");
      return ((unsigned char) 0);
    }
  }
}

/*****************************************************************************
	RUL__CVT_S_AT_S_SF

    Convert from Untyped Atom to Single Float

*****************************************************************************/

float
rul__cvt_s_at_s_sf (Mol_Atom tin_atom)
{
  Molecule mol;

#ifdef DEBUG
  rul__ios_printf (RUL__C_STD_ERR,"\nRUL__CVT_S_AT_S_SF    %p",tin_atom);
#endif

  if (rul__mol_is_int_atom (tin_atom)) 
    return (rul__cvt_s_in_s_sf (tin_atom));

  else if (rul__mol_is_dbl_atom (tin_atom))
    return (rul__cvt_s_db_s_sf (tin_atom));

  else {
    rul__mol_use_printform (tin_atom, tmp_buf, RUL_C_MAX_SYMBOL_SIZE+1);
    mol = rul__string_to_atom (tmp_buf);

    if (rul__mol_is_int_atom (mol)) 
      return (rul__cvt_s_in_s_sf (mol));
    
    else if (rul__mol_is_dbl_atom (mol))
      return (rul__cvt_s_db_s_sf (mol));
    
    else {
      rul__msg_print (RTS_INVCVTXXXX, get_molecule_type_string (tin_atom),
		      "FLOAT");
      return ((float) 0.0);
    }
  }
}

/*****************************************************************************
	RUL__CVT_S_AT_S_DF

    Convert from Untyped Atom to Double Float

*****************************************************************************/

double
rul__cvt_s_at_s_df (Mol_Atom tin_atom)
{
  Molecule mol;

#ifdef DEBUG
  rul__ios_printf (RUL__C_STD_ERR,"\nRUL__CVT_S_AT_S_DF    %p",tin_atom);
#endif

  if (rul__mol_is_int_atom (tin_atom)) 
    return (rul__cvt_s_in_s_df (tin_atom));

  else if (rul__mol_is_dbl_atom (tin_atom))
    return (rul__cvt_s_db_s_df (tin_atom));

  else {
    rul__mol_use_printform (tin_atom, tmp_buf, RUL_C_MAX_SYMBOL_SIZE+1);
    mol = rul__string_to_atom (tmp_buf);

    if (rul__mol_is_int_atom (mol)) 
      return (rul__cvt_s_in_s_df (mol));
    
    else if (rul__mol_is_dbl_atom (mol))
      return (rul__cvt_s_db_s_df (mol));
    
    else {
      rul__msg_print (RTS_INVCVTXXXX, get_molecule_type_string (tin_atom),
		      "DOUBLE");
      return ((double) 0.0);
    }
  }
}

/*****************************************************************************
	RUL__CVT_S_AT_S_VD

    Convert from Untyped Atom to External Opaque

*****************************************************************************/

void *
rul__cvt_s_at_s_vd (Mol_Atom tin_atom)
{
  Molecule mol;

#ifdef DEBUG
  rul__ios_printf (RUL__C_STD_ERR,"\nRUL__CVT_S_AT_S_VD    %p",tin_atom);
#endif

  if (rul__mol_is_opaque (tin_atom))
    return (rul__cvt_s_op_s_vd (tin_atom));

  else {
    rul__mol_use_printform (tin_atom, tmp_buf, RUL_C_MAX_SYMBOL_SIZE+1);
    mol = rul__string_to_atom (tmp_buf);

    if (rul__mol_is_opaque (mol))
      return (rul__cvt_s_op_s_vd (mol));

    else {
      rul__msg_print (RTS_INVCVTXXXX, get_molecule_type_string (tin_atom),
		      "OPAQUE");
      return NULL;
    }
  }
}


/*****************************************************************************
	RUL__CVT_S_AT_S_TA

    Convert from Untyped Atom to External Atom

*****************************************************************************/

Molecule
rul__cvt_s_at_s_ta (Molecule tin_atom)
{

#ifdef DEBUG
  rul__ios_printf (RUL__C_STD_ERR,"\nRUL__CVT_S_AT_S_TA    %p",tin_atom);
#endif

  /* Increment the use count here to handle erxternal use... */
	rul__mol_incr_uses (tin_atom);
  /* Increment the use count here to handle a temporary molecule copy... */
    rul__mol_incr_uses (tin_atom); 
  return (tin_atom);
}

/*****************************************************************************
	RUL__CVT_S_TA_S_AT

    Convert from External Tin Atom to an Untyped Atom

*****************************************************************************/

Molecule
rul__cvt_s_ta_s_at (Molecule tin_atom)
{

#ifdef DEBUG
  rul__ios_printf (RUL__C_STD_ERR,"\nRUL__CVT_S_TA_S_AT    %p",tin_atom);
#endif

  /* Increment the use count here to handle molecule copy... */
    rul__mol_incr_uses (tin_atom); 
  return (tin_atom);
}

/*****************************************************************************
	RUL__CVT_S_TA_S_NA

    Convert from External Tin Atom to an Numeric Atom

*****************************************************************************/

Mol_Atom
rul__cvt_s_ta_s_na (Mol_Atom tin_atom)
{

#ifdef DEBUG
  rul__ios_printf (RUL__C_STD_ERR,"\nRUL__CVT_S_TA_S_NA    %p",tin_atom);
#endif

  if (rul__mol_is_int_atom (tin_atom) || rul__mol_is_dbl_atom (tin_atom)) {
    /* Increment the use count here to handle molecule copy... */
    rul__mol_incr_uses (tin_atom); 
    return (tin_atom);
  }

  else {
    rul__msg_print (RTS_INVCVTXXXX, get_molecule_type_string (tin_atom),
		    "Numeric Atom");
    return rul__mol_make_int_atom (0);
  }
}

/*****************************************************************************
	RUL__CVT_S_TA_S_IN

    Convert from External Tin Atom to an Integer Atom

*****************************************************************************/

Mol_Atom
rul__cvt_s_ta_s_in (Mol_Atom tin_atom)
{

#ifdef DEBUG
  rul__ios_printf (RUL__C_STD_ERR,"\nRUL__CVT_S_TA_S_IN    %p",tin_atom);
#endif

  if (rul__mol_is_int_atom (tin_atom)) {
    /* Increment the use count here to handle molecule copy... */
    rul__mol_incr_uses (tin_atom); 
    return (tin_atom);
  }

  else if (rul__mol_is_dbl_atom (tin_atom))
    return rul__mol_make_int_atom (rul__cvt_s_db_s_lo (tin_atom));

  else {
    rul__msg_print (RTS_INVCVTXXXX, get_molecule_type_string (tin_atom),
		    "Numeric Atom");
    return rul__mol_make_int_atom (0);
  }
}

/*****************************************************************************
	RUL__CVT_S_TA_S_DB

    Convert from External Tin Atom to an Double Atom

*****************************************************************************/

Mol_Atom
rul__cvt_s_ta_s_db (Mol_Atom tin_atom)
{

#ifdef DEBUG
  rul__ios_printf (RUL__C_STD_ERR,"\nRUL__CVT_S_TA_S_DB    %p",tin_atom);
#endif

  if (rul__mol_is_dbl_atom (tin_atom)) {
    /* Increment the use count here to handle molecule copy... */
    rul__mol_incr_uses (tin_atom); 
    return (tin_atom);
  }

  else if (rul__mol_is_int_atom (tin_atom))
    return rul__mol_make_dbl_atom (rul__cvt_s_in_s_df (tin_atom));

  else {
    rul__msg_print (RTS_INVCVTXXXX, get_molecule_type_string (tin_atom),
		    "Numeric Atom");
    return rul__mol_make_dbl_atom (0.0);
  }
}

/*****************************************************************************
	RUL__CVT_S_TA_S_SY

    Convert from External Tin Atom to an Symbolic Atom

*****************************************************************************/

Mol_Atom
rul__cvt_s_ta_s_sy (Mol_Atom tin_atom)
{
  long     len;
  Mol_Atom ret_atom = tin_atom;

#ifdef DEBUG
  rul__ios_printf (RUL__C_STD_ERR,"\nRUL__CVT_S_TA_S_SY    %p",tin_atom);
#endif

  if (!rul__mol_is_symbol (tin_atom)) {
    rul__mol_use_printform (tin_atom, tmp_buf, RUL_C_MAX_SYMBOL_SIZE + 1);
    ret_atom = rul__mol_make_symbol (tmp_buf);
  }

  /* Increment the use count here to handle molecule copy... */
  rul__mol_incr_uses (ret_atom); 
  return (ret_atom);
}

/*****************************************************************************
	RUL__CVT_S_AT_S_SY

    Convert from Mol Atom to an Symbolic Atom

*****************************************************************************/

Mol_Atom
rul__cvt_s_at_s_sy (Mol_Atom tin_atom)
{
  long     len;
  Mol_Atom ret_atom = tin_atom;

#ifdef DEBUG
  rul__ios_printf (RUL__C_STD_ERR,"\nRUL__CVT_S_AT_S_SY    %p",tin_atom);
#endif

  if (!rul__mol_is_symbol (tin_atom)) {
    rul__mol_use_printform (tin_atom, tmp_buf, RUL_C_MAX_SYMBOL_SIZE + 1);
    ret_atom = rul__mol_make_symbol (tmp_buf);
  }
  else   /* Increment the use count here to handle molecule copy... */
    rul__mol_incr_uses (ret_atom); 

  return (ret_atom);
}

/*****************************************************************************
	RUL__CVT_S_TA_S_OP

    Convert from External Tin Atom to an Opaque Atom

*****************************************************************************/

Mol_Atom
rul__cvt_s_ta_s_op (Mol_Atom tin_atom)
{

#ifdef DEBUG
  rul__ios_printf (RUL__C_STD_ERR,"\nRUL__CVT_S_TA_S_OP    %p",tin_atom);
#endif

  if (rul__mol_is_opaque (tin_atom)) {
    /* Increment the use count here to handle molecule copy... */
    rul__mol_incr_uses (tin_atom); 
    return (tin_atom);
  }
  else {
    rul__msg_print (RTS_INVCVTXXXX, get_molecule_type_string (tin_atom),
		    "Opaque");
    return rul__mol_make_opaque (NULL);
  }
}

/*****************************************************************************
	RUL__CVT_S_TA_S_ID

    Convert from External Tin Atom to an Instance ID Atom

*****************************************************************************/

Mol_Atom
rul__cvt_s_ta_s_id (Mol_Atom tin_atom)
{

#ifdef DEBUG
  rul__ios_printf (RUL__C_STD_ERR,"\nRUL__CVT_S_TA_S_ID    %p",tin_atom);
#endif

  if (rul__mol_is_instance_id (tin_atom)) {
    /* Increment the use count here to handle molecule copy... */
    rul__mol_incr_uses (tin_atom); 
    return (tin_atom);
  }
  else {
    rul__msg_print (RTS_INVCVTXXXX, get_molecule_type_string (tin_atom),
		    "Instance Id");
    return rul__mol_symbol_nil ();
  }
}



/*****************************
*  CONVERSIONS TO COMPOUNDS  *
*****************************/

/*****************************************************************************
	RUL__CVT_A_LO_C_IN

    Convert from array of Longs to Compound of Integer Atoms

*****************************************************************************/

Mol_Compound
rul__cvt_a_lo_c_in (long array[], long a_len)
{
  long i;

#ifdef DEBUG
  rul__ios_printf (RUL__C_STD_ERR, "\nRUL__CVT_A_LO_C_IN %p %ld",
		   array, a_len);
#endif

  /* this conversion requires the creation of a new value */

  rul__mol_start_tmp_comp (a_len);
  for (i=0; i < a_len; i++) {
    rul__mol_set_tmp_comp_nth (i, rul__cvt_s_lo_s_in(array[i]));
  }

  return rul__mol_end_tmp_comp ();
}


/*****************************************************************************
	RUL__CVT_A_SH_C_IN

    Convert from array of Shorts to Compound of Integer Atoms

*****************************************************************************/

Mol_Compound
rul__cvt_a_sh_c_in (short array[], long a_len)
{
  long i;

#ifdef DEBUG
  rul__ios_printf (RUL__C_STD_ERR,"\nRUL__CVT_A_SH_C_IN    %p %ld",
		   array, a_len);
#endif

  /* this conversion requires the creation of a new value */
  
  rul__mol_start_tmp_comp (a_len);
  for (i=0; i < a_len; i++) {
    rul__mol_set_tmp_comp_nth (i, rul__cvt_s_sh_s_in(array[i]));
  }

  return rul__mol_end_tmp_comp ();
}


/*****************************************************************************
	RUL__CVT_A_BY_C_IN

    Convert from array of Bytes to Compound of Integer Atoms

*****************************************************************************/

Mol_Compound
rul__cvt_a_by_c_in (char array[], long a_len)
{
  long i;

#ifdef DEBUG
  rul__ios_printf (RUL__C_STD_ERR,"\nRUL__CVT_A_BY_C_IN    %p %ld",
		   array, a_len);
#endif

  /* this conversion requires the creation of a new value */

  rul__mol_start_tmp_comp (a_len);
  for (i=0; i < a_len; i++) {
    rul__mol_set_tmp_comp_nth (i, rul__cvt_s_by_s_in(array[i]));
  }

  return rul__mol_end_tmp_comp ();
}


/*****************************************************************************
	RUL__CVT_A_UL_C_IN

    Convert from array of Unsigned Longs to Compound of Integer Atoms

*****************************************************************************/

Mol_Compound
rul__cvt_a_ul_c_in (unsigned long array[], long a_len)
{
  long i;

#ifdef DEBUG
  rul__ios_printf (RUL__C_STD_ERR,"\nRUL__CVT_A_UL_C_IN    %p %ld",
		   array, a_len);
#endif

  /* this conversion requires the creation of a new value */

  rul__mol_start_tmp_comp (a_len);
  for (i=0; i < a_len; i++) {
    rul__mol_set_tmp_comp_nth (i, rul__cvt_s_ul_s_in(array[i]));
  }
  return rul__mol_end_tmp_comp ();
}


/*****************************************************************************
	RUL__CVT_A_US_C_IN

    Convert from array of Unsigned Shorts to Compound of Integer Atoms

*****************************************************************************/

Mol_Compound
rul__cvt_a_us_c_in (unsigned short array[], long a_len)
{
  long i;

#ifdef DEBUG
  rul__ios_printf (RUL__C_STD_ERR,"\nRUL__CVT_A_US_C_IN    %p %ld",
		   array, a_len);
#endif

  /* this conversion requires the creation of a new value */

  rul__mol_start_tmp_comp (a_len);
  for (i=0; i < a_len; i++) {
    rul__mol_set_tmp_comp_nth (i, rul__cvt_s_us_s_in(array[i]));
  }

  return rul__mol_end_tmp_comp ();
}


/*****************************************************************************
	RUL__CVT_A_UB_C_IN

    Convert from array of Unsigned Bytes to Compound of Integer Atoms

*****************************************************************************/

Mol_Compound
rul__cvt_a_ub_c_in (unsigned char array[], long a_len)
{
  long i;

#ifdef DEBUG
  rul__ios_printf (RUL__C_STD_ERR,"\nRUL__CVT_A_UB_C_IN    %p %ld",
		   array, a_len);
#endif

  /* this conversion requires the creation of a new value */
  
  rul__mol_start_tmp_comp (a_len);
  for (i=0; i < a_len; i++) {
    rul__mol_set_tmp_comp_nth (i, rul__cvt_s_ub_s_in(array[i]));
  }
  return rul__mol_end_tmp_comp ();
}


/*****************************************************************************
	RUL__CVT_A_SF_C_DB

    Convert from array of Single Floats to Compound of Float Atoms

*****************************************************************************/

Mol_Compound
rul__cvt_a_sf_c_db (float array[], long a_len)
{
  long i;

#ifdef DEBUG
  rul__ios_printf (RUL__C_STD_ERR,"\nRUL__CVT_A_SF_C_DB    %p %ld",
		   array, a_len);
#endif

  /* this conversion requires the creation of a new value */

  rul__mol_start_tmp_comp (a_len);
  for (i=0; i < a_len; i++) {
    rul__mol_set_tmp_comp_nth (i, rul__cvt_s_sf_s_db(array[i]));
  }

  return rul__mol_end_tmp_comp ();
}


/*****************************************************************************
	RUL__CVT_A_DF_C_DB

    Convert from array of Double Floats to Compound

*****************************************************************************/

Mol_Compound
rul__cvt_a_df_c_db (double array[], long a_len)
{
  long i;

#ifdef DEBUG
  rul__ios_printf (RUL__C_STD_ERR,"\nRUL__CVT_A_DF_C_DB    %p %ld",
		   array, a_len);
#endif

  /* this conversion requires the creation of a new value */

  rul__mol_start_tmp_comp (a_len);
  for (i=0; i < a_len; i++) {
    rul__mol_set_tmp_comp_nth (i, rul__cvt_s_df_s_db(array[i]));
  }

  return rul__mol_end_tmp_comp ();
}


/*****************************************************************************
	RUL__CVT_A_AZ_C_SY

    Convert from array of Asciz strings to Compound of Symbolic Atoms

*****************************************************************************/


Mol_Compound
rul__cvt_a_az_c_sy (char *array[], long a_len)
{
  long i;

#ifdef DEBUG
  rul__ios_printf (RUL__C_STD_ERR,"\nRUL__CVT_A_AZ_C_SY    %p %ld",
		   array, a_len);
#endif

  /* this conversion requires the creation of a new value */
  
  rul__mol_start_tmp_comp (a_len);
  for (i=0; i < a_len; i++) {
    if (array[i] == NULL)
      rul__mol_set_tmp_comp_nth (i, rul__mol_symbol_nil ());

    else
      rul__mol_set_tmp_comp_nth (i, rul__cvt_s_az_s_sy (array[i]));
  }

  return rul__mol_end_tmp_comp ();
}


/*****************************************************************************
	RUL__CVT_A_AD_C_SY

    Convert from array of Ascid strings to Compound of Symbolic Atoms

*****************************************************************************/

Mol_Compound
rul__cvt_a_ad_c_sy (String *array[], long a_len)
{
  long i;

#ifdef DEBUG
  rul__ios_printf (RUL__C_STD_ERR,"\nRUL__CVT_A_AD_C_SY    %p %ld",
		   array, a_len);
#endif

  /* this conversion requires the creation of a new value */
  
  rul__mol_start_tmp_comp (a_len);
  for (i=0; i < a_len; i++) {
    if (array[i] == NULL)
      rul__mol_set_tmp_comp_nth (i, rul__mol_symbol_nil ());

    else
      rul__mol_set_tmp_comp_nth (i, rul__cvt_s_ad_s_sy(array[i]));
  }

  return rul__mol_end_tmp_comp ();
}


/*****************************************************************************
	RUL__CVT_A_AZ_C_AT

    Convert from array of Asciz strings to Compound of Untyped Atoms

*****************************************************************************/


Mol_Compound
rul__cvt_a_az_c_at (char *array[], long a_len)
{
  long i;

#ifdef DEBUG
  rul__ios_printf (RUL__C_STD_ERR,"\nRUL__CVT_A_AZ_C_AT    %p %ld",
		   array, a_len);
#endif
  
  /* this conversion requires the creation of a new value */
  
  rul__mol_start_tmp_comp (a_len);
  for (i=0; i < a_len; i++) {
    if (array[i] == NULL)
      rul__mol_set_tmp_comp_nth (i, rul__mol_symbol_nil ());

    else
      rul__mol_set_tmp_comp_nth (i, rul__cvt_s_az_s_at (array[i]));
  }

  return rul__mol_end_tmp_comp ();
}


/*****************************************************************************
	RUL__CVT_A_AD_C_AT

    Convert from array of Ascid strings to Compound of Untyped Atoms

*****************************************************************************/

Mol_Compound
rul__cvt_a_ad_c_at (String *array[], long a_len)
{
  long i;

#ifdef DEBUG
  rul__ios_printf (RUL__C_STD_ERR,"\nRUL__CVT_A_AD_C_AT    %p %ld",
		   array, a_len);
#endif

  /* this conversion requires the creation of a new value */
  
  rul__mol_start_tmp_comp (a_len);
  for (i=0; i < a_len; i++) {
    if (array[i] == NULL)
      rul__mol_set_tmp_comp_nth (i, rul__mol_symbol_nil ());

    else
      rul__mol_set_tmp_comp_nth (i, rul__cvt_s_ad_s_at (array[i]));
  }

  return rul__mol_end_tmp_comp ();
}


/*****************************************************************************
	RUL__CVT_A_TA_C_AT

    Convert from array of atoms to Compound of atoms

*****************************************************************************/

Mol_Compound
rul__cvt_a_ta_c_at (Mol_Atom array[], long a_len)
{
  long i;

#ifdef DEBUG
  rul__ios_printf (RUL__C_STD_ERR,"\nRUL__CVT_A_TA_C_AT    %p %ld",
		   array, a_len);
#endif

  /* this conversion requires the creation of a new value */
  
  rul__mol_start_tmp_comp (a_len);
  for (i=0; i < a_len; i++) {
    if (array[i] == NULL)
      rul__mol_set_tmp_comp_nth (i, rul__mol_symbol_nil ());

    else
      rul__mol_set_tmp_comp_nth (i, array[i]);
  }

  return rul__mol_end_tmp_comp ();
}


/*****************************************************************************
	RUL__CVT_A_VD_C_OP

    Convert from array of External Opaques to Compound of Opaques

*****************************************************************************/

Mol_Compound
rul__cvt_a_vd_c_op (void *array[], long a_len)
{
  long i;

#ifdef DEBUG
  rul__ios_printf (RUL__C_STD_ERR,"\nRUL__CVT_A_VD_C_OP    %p %ld",
		   array, a_len);
#endif
  
  /* this conversion requires the creation of a new value */
  
  rul__mol_start_tmp_comp (a_len);
  for (i=0; i < a_len; i++) {
    if (array[i] == NULL)
      rul__mol_set_tmp_comp_nth (i, rul__mol_opaque_null ());

    else
      rul__mol_set_tmp_comp_nth (i, rul__cvt_s_vd_s_op(array[i]));
  }

  return rul__mol_end_tmp_comp ();
}



/*******************************
*  CONVERSIONS FROM COMPOUNDS  *
*******************************/

/*****************************************************************************
	RUL__CVT_C_AT_A_LO

    Convert from Compound to an array of Longs

*****************************************************************************/

long *
rul__cvt_c_at_a_lo (Mol_Compound tin_pointer, long a_len,
		    long *data_ptr)
{
    long *pointer;
    long i, c_len, len;
    long ele_size = sizeof(long);

#ifdef DEBUG
	rul__ios_printf (RUL__C_STD_ERR,"\nRUL__CVT_C_AT_A_LO    %p %ld",
			 tin_pointer, a_len);
#endif

    c_len = rul__mol_get_poly_count (tin_pointer);
    if (a_len < 0) a_len = c_len ;
    if (c_len > a_len) {
      rul__msg_print (RTS_CMPELTLOST);
    }

    if (data_ptr)
      pointer = data_ptr;
    else {
      len = a_len * ele_size;
      pointer = (long *) rul__mem_malloc (len);
    }

    for (i=0; i < a_len; i++) {
      if (i < c_len)
	pointer[i] = rul__cvt_s_at_s_lo (
				 rul__mol_get_comp_nth (tin_pointer, i + 1));
      else 
	pointer[i] = (long) 0;
    }
    return pointer;
}


/*****************************************************************************
	RUL__CVT_C_AT_A_SH

    Convert from Compound to an array of Shorts

*****************************************************************************/

short *
rul__cvt_c_at_a_sh (Mol_Compound tin_pointer, long a_len,
		    short *data_ptr)
{
    short *pointer;
    long i, c_len, len;
    long ele_size = sizeof(short);

#ifdef DEBUG
	rul__ios_printf (RUL__C_STD_ERR,"\nRUL__CVT_C_AT_A_SH    %p %ld",
			 tin_pointer, a_len);
#endif

    c_len = rul__mol_get_poly_count (tin_pointer);
    if (a_len < 0) a_len = c_len;
    if (c_len > a_len) {
      rul__msg_print (RTS_CMPELTLOST);
    }

    if (data_ptr)
      pointer = data_ptr;
    else {
      len = a_len * ele_size;
      pointer = (short *) rul__mem_malloc (len);
    }

    for (i=0; i < a_len	; i++) {
      if (i < c_len)
	pointer[i] = rul__cvt_s_at_s_sh (
				rul__mol_get_comp_nth (tin_pointer, i + 1));
      else
	pointer[i] = (short) 0;
    }
    return pointer;
}


/*****************************************************************************
	RUL__CVT_C_AT_A_BY

    Convert from Compound to an array of Bytes

*****************************************************************************/

char *
rul__cvt_c_at_a_by (Mol_Compound tin_pointer, long a_len,
		    char *data_ptr)
{
    char *pointer;
    long i, c_len, len;
    long ele_size = sizeof(char);

#ifdef DEBUG
	rul__ios_printf (RUL__C_STD_ERR,"\nRUL__CVT_C_AT_A_BY    %p %ld",
			 tin_pointer, a_len);
#endif

    c_len = rul__mol_get_poly_count (tin_pointer);
    if (a_len < 0) a_len = c_len;
    if (c_len > a_len) {
      rul__msg_print (RTS_CMPELTLOST);
    }

    if (data_ptr)
      pointer = data_ptr;
    else {
      len = a_len * ele_size;
      pointer = (char *) rul__mem_malloc (len);
    }

    for (i=0; i < a_len; i++) {
      if (i < c_len)
	pointer[i] = rul__cvt_s_at_s_by (
				 rul__mol_get_comp_nth (tin_pointer, i + 1));
      else
	pointer[i] = (char) 0;
    }
    return pointer;
}


/*****************************************************************************
	RUL__CVT_C_AT_A_UL

    Convert from Compound to an array of Unsigned Longs

*****************************************************************************/

unsigned long *
rul__cvt_c_at_a_ul (Mol_Compound tin_pointer, long a_len,
		    unsigned long *data_ptr)
{
    unsigned long *pointer;
    long i, c_len, len;
    long ele_size = sizeof(unsigned long);

#ifdef DEBUG
	rul__ios_printf (RUL__C_STD_ERR,"\nRUL__CVT_C_AT_A_UL    %p %ld",
			 tin_pointer, a_len);
#endif

    c_len = rul__mol_get_poly_count (tin_pointer);
    if (a_len < 0) a_len = c_len;
    if (c_len > a_len) {
      rul__msg_print (RTS_CMPELTLOST);
    }

    if (data_ptr)
      pointer = data_ptr;
    else {
      len = a_len *  ele_size;
      pointer = (unsigned long *) rul__mem_malloc (len);
    }

    for (i=0; i < a_len; i++) {
      if (i < c_len)
	pointer[i] = rul__cvt_s_at_s_ul (
			 rul__mol_get_comp_nth (tin_pointer, i + 1));
      else
	pointer[i] = (unsigned long) 0;
    }
    return pointer;
}


/*****************************************************************************
	RUL__CVT_C_AT_A_US

    Convert from Compound to an array of Unsigned Shorts

*****************************************************************************/

unsigned short *
rul__cvt_c_at_a_us (Mol_Compound tin_pointer, long a_len,
		    unsigned short *data_ptr)
{
    unsigned short *pointer;
    long i, c_len, len;
    long ele_size = sizeof(unsigned short);

#ifdef DEBUG
	rul__ios_printf (RUL__C_STD_ERR,"\nRUL__CVT_C_AT_A_US    %p %ld",
			 tin_pointer, a_len);
#endif

    c_len = rul__mol_get_poly_count (tin_pointer);
    if (a_len < 0) a_len = c_len;
    if (c_len > a_len) {
      rul__msg_print (RTS_CMPELTLOST);
    }

    if (data_ptr)
      pointer = data_ptr;
    else {
      len = a_len *  ele_size;
      pointer = (unsigned short *) rul__mem_malloc (len);
    }

    for (i=0; i < a_len; i++) {
      if (i < c_len)
	pointer[i] = rul__cvt_s_at_s_us (
			 rul__mol_get_comp_nth (tin_pointer, i + 1));
      else
	pointer[i] = (unsigned short) 0;
    }
    return pointer;
}


/*****************************************************************************
	RUL__CVT_C_AT_A_UB

    Convert from Compound to an array of Unsigned Bytes

*****************************************************************************/

unsigned char *
rul__cvt_c_at_a_ub (Mol_Compound tin_pointer, long a_len,
		    unsigned char *data_ptr)
{
    unsigned char *pointer;
    long i, c_len, len;
    long ele_size = sizeof(unsigned char);

#ifdef DEBUG
	rul__ios_printf (RUL__C_STD_ERR,"\nRUL__CVT_C_AT_A_UB    %p %ld",
			 tin_pointer, a_len);
#endif

    c_len = rul__mol_get_poly_count (tin_pointer);
    if (a_len < 0) a_len = c_len;
    if (c_len > a_len) {
      rul__msg_print (RTS_CMPELTLOST);
    }

    if (data_ptr)
      pointer = data_ptr;
    else {
      len = a_len *  ele_size;
      pointer = (unsigned char *) rul__mem_malloc (len);
    }

    for (i=0; i < a_len; i++) {
      if (i < c_len)
	pointer[i] = rul__cvt_s_at_s_ub (
			 rul__mol_get_comp_nth (tin_pointer, i + 1));
      else
	pointer[i] = (unsigned char) 0;
    }
    return pointer;
}


/*****************************************************************************
	RUL__CVT_C_AT_A_SF

    Convert from Compound to an array of Single Floats

*****************************************************************************/

float *
rul__cvt_c_at_a_sf (Mol_Compound tin_pointer, long a_len,
		    float *data_ptr)
{
    float *pointer;
    long i, c_len, len;
    long ele_size = sizeof(float);

#ifdef DEBUG
	rul__ios_printf (RUL__C_STD_ERR,"\nRUL__CVT_C_AT_A_SF    %p %ld",
			 tin_pointer, a_len);
#endif

    c_len = rul__mol_get_poly_count (tin_pointer);
    if (a_len < 0) a_len = c_len;
    if (c_len > a_len) {
      rul__msg_print (RTS_CMPELTLOST);
    }

    if (data_ptr)
      pointer = data_ptr;
    else {
      len = a_len *  ele_size;
      pointer = (float *) rul__mem_malloc (len);
    }

    for (i=0; i < a_len; i++) {
      if (i < c_len)
	pointer[i] = rul__cvt_s_at_s_sf (
			 rul__mol_get_comp_nth (tin_pointer, i + 1));
      else
	pointer[i] = (float) 0.0;
    }
    return pointer;
}


/*****************************************************************************
	RUL__CVT_C_AT_A_DF

    Convert from Compound to an array of Double Floats

*****************************************************************************/

double *
rul__cvt_c_at_a_df (Mol_Compound tin_pointer, long a_len,
		    double *data_ptr)
{
    double *pointer;
    long i, c_len, len;
    long ele_size = sizeof(double);

#ifdef DEBUG
	rul__ios_printf (RUL__C_STD_ERR,"\nRUL__CVT_C_AT_A_DF    %p %ld",
			 tin_pointer, a_len);
#endif

    c_len = rul__mol_get_poly_count (tin_pointer);
    if (a_len < 0) a_len = c_len;
    if (c_len > a_len) {
      rul__msg_print (RTS_CMPELTLOST);
    } 

    if (data_ptr)
      pointer = data_ptr;
    else {
      len = a_len *  ele_size;
      pointer = (double *) rul__mem_malloc (len);
    }

    for (i=0; i < a_len; i++) {
      if (i < c_len)
	pointer[i] = rul__cvt_s_at_s_df (
			 rul__mol_get_comp_nth (tin_pointer, i + 1));
      else
	pointer[i] = (double) 0.0;
    }
    return pointer;
}


/*****************************************************************************
	RUL__CVT_C_AT_A_AZ

    Convert from Compound to an array of Asciz strings

*****************************************************************************/

char **
rul__cvt_c_at_a_az (Mol_Compound tin_pointer, long a_len,
		    char **data_ptr)
{
  char **pointer;
  long    i, c_len, len;
  char (*characters)[RUL_C_MAX_SYMBOL_SIZE+1];

#ifdef DEBUG
  rul__ios_printf (RUL__C_STD_ERR, "\nRUL__CVT_C_AT_A_AZ    %p %ld",
		   tin_pointer, a_len);
#endif

  c_len = rul__mol_get_poly_count (tin_pointer);
  if (a_len < 0) a_len = c_len;
  if (c_len > a_len)  {
    rul__msg_print (RTS_CMPELTLOST);
  }

  if (data_ptr) {
    pointer = data_ptr;
    len = (MIN (a_len, c_len) *
	      (RUL_C_MAX_SYMBOL_SIZE + 1) * sizeof(char));
    characters = (char (*)[(RUL_C_MAX_SYMBOL_SIZE+1)]) rul__mem_malloc (len);
  }

  else {
    len = (a_len * sizeof(char *)) +	            /* space for pointers  */
      (MIN (a_len, c_len) *
       (RUL_C_MAX_SYMBOL_SIZE + 1) * sizeof(char)); /* strings */
    pointer = (char **) rul__mem_malloc (len);
    /* the array of RUL_C_MAX_SYMBOL_SIZE character strings begins after
       the array of a_len pointers to character strings */
    characters = (char (*)[(RUL_C_MAX_SYMBOL_SIZE+1)]) (pointer + a_len);
    
  }

  for (i = 0; i < a_len; i++) {
    if (i < c_len) {
      pointer[i] = characters[i];
      rul__mol_use_printform (rul__mol_get_comp_nth (tin_pointer, i + 1),
			      characters[i], (RUL_C_MAX_SYMBOL_SIZE+1));
    }
    else
      pointer[i] = NULL;
  }
  return pointer;
}


/*****************************************************************************
	RUL__CVT_C_AT_A_AD

    Convert from Compound to an array of Ascid strings

*****************************************************************************/

String **
rul__cvt_c_at_a_ad (Mol_Compound tin_pointer, long a_len,
		    String **data_ptr)
{
  long	      c_len;
  long	      len;
  String    **pointer;
  String     *descriptor;
  char      (*characters)[(RUL_C_MAX_SYMBOL_SIZE + 1)];
  long         i;

#ifdef DEBUG
  rul__ios_printf (RUL__C_STD_ERR, "\nRUL__CVT_C_AT_A_AD    %p %ld",
		   tin_pointer, a_len);
#endif

  c_len = rul__mol_get_poly_count (tin_pointer);
  if (a_len < 0)
    a_len = c_len;
  if (c_len > a_len) {
    rul__msg_print (RTS_CMPELTLOST);
  }

  if (data_ptr) {
    pointer = data_ptr;
    len = (a_len * sizeof (String)) +	        /* descriptors space */
	(MIN (a_len, c_len) *
	 (RUL_C_MAX_SYMBOL_SIZE + 1) * sizeof(char));   /*strings */
    descriptor = (String *) rul__mem_malloc (len);
    characters = (char (*)[(RUL_C_MAX_SYMBOL_SIZE + 1)]) (descriptor + a_len);
  }

  else {
    len = (a_len * sizeof (String *)) +	 	/* space for pointers*/
      (a_len * sizeof (String)) +		        /* descriptors space */
	(MIN (a_len, c_len) *
	 (RUL_C_MAX_SYMBOL_SIZE + 1) * sizeof(char));   /*strings */
    pointer    = (String **) rul__mem_malloc (len);

    /* the array of a_len descriptors begins after
       the array of a_len pointers to descriptors */
    descriptor = (String *)(pointer + a_len);
  
    /* the array of RUL_C_MAX_SYMBOL_SIZE character strings begins after
       the array of a_len descriptors */
    characters = (char (*)[(RUL_C_MAX_SYMBOL_SIZE + 1)]) (descriptor + a_len);
  }
    
  for (i = 0; i < a_len; i++) {
    if (i < c_len) {
      pointer[i] = &descriptor[i];
      rul__mol_use_printform (rul__mol_get_comp_nth (tin_pointer, i + 1),
			      characters[i], (RUL_C_MAX_SYMBOL_SIZE + 1));
      descriptor[i].dsc_w_length  = strlen (characters[i]);
      descriptor[i].dsc_b_dtype   = DSC_K_DTYPE_T ;
      descriptor[i].dsc_b_class   = DSC_K_CLASS_S ;
      descriptor[i].dsc_a_pointer = characters[i] ;
    }
    else {
      descriptor[i].dsc_w_length  = 0 ;
      descriptor[i].dsc_b_dtype   = DSC_K_DTYPE_T ;
      descriptor[i].dsc_b_class   = DSC_K_CLASS_S ;
      descriptor[i].dsc_a_pointer = NULL ;
    }
  }
  return pointer;
}


/*****************************************************************************
	RUL__CVT_C_AT_A_TA

    Convert from Compound to an array of Tin atoms

*****************************************************************************/

Mol_Atom *
rul__cvt_c_at_a_ta (Mol_Compound tin_pointer, long a_len,
		    Mol_Atom *data_ptr)
{
  Mol_Atom *pointer;
  long i, c_len, len;
  long ele_size = sizeof(Mol_Atom);

#ifdef DEBUG
  rul__ios_printf (RUL__C_STD_ERR,"\nRUL__CVT_C_AT_A_TA    %p %ld",
		   tin_pointer, a_len);
#endif

  c_len = rul__mol_get_poly_count (tin_pointer);
  if (a_len < 0) a_len = c_len ;
  if (c_len > a_len) {
    rul__msg_print (RTS_CMPELTLOST);
  }

  if (data_ptr)
    pointer = data_ptr;
  else {
    len = a_len * ele_size;
    pointer = (Mol_Atom *) rul__mem_malloc (len);
  }

  for (i=0; i < a_len; i++) {
    if (i < c_len) {
      pointer[i] = rul__mol_get_comp_nth (tin_pointer, i + 1);
      rul__mol_incr_uses (pointer[i]);
    }
    else 
      pointer[i] = rul__mol_symbol_nil ();
  }
  return pointer;
}


/*****************************************************************************
	RUL__CVT_C_AT_A_VD

    Convert from Compound to an array of External Opaques

*****************************************************************************/

void **
rul__cvt_c_at_a_vd (Mol_Compound tin_pointer, long a_len,
		    void **data_ptr)
{
  void **pointer;
  long i, c_len, len;
  long ele_size = sizeof(void *);

#ifdef DEBUG
  rul__ios_printf (RUL__C_STD_ERR,"\nRUL__CVT_C_AT_A_DF    %p %ld",
		   tin_pointer, a_len);
#endif

  c_len = rul__mol_get_poly_count (tin_pointer);
  if (a_len < 0) a_len = c_len;
  if (c_len > a_len) {
    rul__msg_print (RTS_CMPELTLOST);
  }

  if (data_ptr)
    pointer = data_ptr;
  else {
    len = a_len * ele_size;
    pointer = (void **) rul__mem_malloc (len);
  }

  for (i = 0; i < a_len; i++) {
    if (i < c_len)
      pointer[i] = rul__cvt_s_at_s_vd (
			       rul__mol_get_comp_nth (tin_pointer, i + 1));
    else
      pointer[i] = (void *) 0;
  }
  return pointer;
}



/*********************
*  SPECIAL ROUTINES  *
*********************/

/*****************************************************************************
	RUL__CVT_FREE

    Free memory used to convert tin types to external types

*****************************************************************************/

void
rul__cvt_free (void *ext_pointer)
{

#ifdef DEBUG
	rul__ios_printf (RUL__C_STD_ERR,"\nRUL__CVT_S_FREE	%p",
			 ext_pointer);
#endif
        rul__mem_free (ext_pointer);
	return;
}

