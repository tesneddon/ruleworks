/****************************************************************************
**                                                                         **
**                    R T S _ C B _ C O M M O N . C                        **
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
 *	This module provides private routines used by the user-visible
 *	callback routines defined in RTS_CB_ATOM_TABLE, RTS_CB_WM_QUERY,
 *	and RTS_CB_WMS.  Each of those modules is compiled seperately
 *	for each binding, so any code common to all bindings is here.
 *
 * MODIFIED BY:
 *	DEC	Digital Equipment Corporation
 *	CPQ	Compaq Computer Corporation
 *
 * MODIFICATION HISTORY:
 *
 *	 6-Jun-1991	DEC	Initial version
 *
 *	 5-Sep-1991	DEC	Added routines from rts_cb_atom_table.c
 *					and rts_cb_wm_query.c
 *
 *	13-Sep-1991	DEC	Added rts_cb_wms helper routines.
 *
 *	16-Sep-1991	DEC	Ensure last WME set_attr-ed is most recent.
 *
 *	19-Sep-1991	DEC	Added rul__SET_COMP_ATTR_STRING.
 *
 *	20-Sep-1991	DEC	Changed include from ops_compound.h to
 *					rts_compound.h.  Downcased function calls.
 *
 *	20-Sep-1991	DEC	Addded id translation and make wme routines
 *
 *	24-Sep-1991	DEC	Make rul__prepare_to_change_wme call
 *					ops$$load_current so compound values are copied
 *
 *	25-Sep-1991	DEC	Added helper routines for ALL rtl routines.
 *					Removed all uppercase routines references.
 *
 *	 3-Oct-1991	DEC	Call rul__delete_hash_table from
 *					rul__end_wme_id_translation.
 *
 *	23-Oct-1991	DEC	Call rul__free_comp_attr instead of
 *					rul__free_comp_attr_in_wm.
 *
 *	28-Jan-1992	DEC	Fixed rul__atom_to_string to return 
 *					readable form (quoted symbols)
 *
 *	26-Mar-1992	DEC	Made rul__string_has_specials much less
 *					casual about what it claims is special,
 *					although it still isn't exact about
 *					strings that look vaguely like numbers.
 *
 *	31-Mar-1992	DEC	Don't issue USERBUFTOOSMALL messages from
 *					ops_get_instance or ops_get_comp_attr_string.
 *
 *	20-Jan-1993	DEC	Changes for V5
 *
 *	26-May-1994	DEC	Fix rul___make_wmos for classname w/o parens
 *					or attributes; Catch & report more errors;
 *					Remove INVATMPAR message for rul_is_instance().
 *
 *	16-feb-1998	DEC	class type changed to rclass
 *
 *	30-Mar-1998	DEC	added rul__atom_is_oatom function
 *					added rul__set_comp_elem_opaque
 *
 *	31-Mar-1998	DEC	added rul__oatom_to_ptr and
 *					rul__ptr_to_oatom
 *
 *	01-Dec-1999	CPQ	Release with GPL
 */

/*
 *	Table of contents:
 *
 *    Private Helper Routines --
 *
 *	String	*rul__to_desc		      (char)
 *	void	 rul__ascic_to_asciz	      (char, char, short)
 *	Boolean	 rul__validate_wme_id	      (Molecule, Address)
 *	Boolean	 rul__validate_class_attr     (Object, char, Class*, Mol*)
 *
 *    ATOM_TABLE Helper Routines --
 *
 *	float  	  rul__fatom_to_float         (atom_value)
 *	double 	  rul__fatom_to_double        (atom_value)
 *	Molecule  rul__float_to_fatom         (float_value)
 *	Molecule  rul__double_to_fatom        (float_value)
 *	Molecule  rul__gensym		      (atom_value)
 *	Molecule  rul__genint		      (void)
 *	long	  rul__iatom_to_integer	      (atom_value)
 *	Molecule  rul__integer_to_iatom	      (long_value)
 *	void *	  rul__oatom_to_ptr	      (atom_value)
 *	Molecule  rul__ptr_to_oatom	      (ptr_value)
 *	Molecule  rul__string_to_symbol       (char_string)
 *	long	  rul__symbol_to_string	      (char_buf, long, atom_value)
 *	Molecule  rul__string_to_atom	      (char_string)
 *	long 	  rul__atom_to_string	      (char_buf, long, atom_value)
 *      long      rul__get_atom_string_length (atom_value)
 *	Molecule  rul__string_to_compound     (char_string)
 *
 *    WM_QUERY Helper Routines --
 *
 *      Boolean  rul__attr_is_compound        (char, char, char)
 *      Boolean  rul__is_attribute            (char, char, char)
 *      Boolean  rul__is_class                (char, char)
 *      Boolean  rul__is_subclass             (char, char)
 *      Boolean  rul__is_wme                  (Molecule)
 *      Molecule rul__get_attr_atom           (Molecule, char)
 *      long	 rul__get_class_string        (char, long, Molecule)
 *      long	 rul__get_class_string_length (Molecule)
 *      long	 rul__get_comp_attr_length    (Molecule, char)
 *      long	 rul__get_comp_attr_string    (char, long, Molecule, char)
 *      long	 rul__get_comp_attr_string_len(Molecule, char)
 *      Molecule rul__get_comp_elem_atom      (Molecule, char, long)
 *      long	 rul__get_wme                 (char, long, Molecule)
 *      long	 rul__get_wme_length          (Molecule)
 *      Molecule rul__get_next_wme            (Molecule)
 *
 *    WMS Helper Routines --
 *
 *	Molecule rul__copy_wme		     (Molecule)
 *	Molecule rul__make_wme		     (char, char)
 *	Molecule rul__modify_wme_start	     (Molecule)
 *	Molecule rul__modify_wme_end	     (Molecule)
 *	Boolean	 rul__remove_wme	     (Molecule)
 *	Molecule rul__specialize_wme	     (Molecule, char)
 *	Boolean	 rul__set_attr_atom	     (Molecule, char, Molecule)
 *	Boolean	 rul__set_attr_float	     (Molecule, char, Molecule)
 *	Boolean	 rul__set_attr_double	     (Molecule, char, Molecule)
 *	Boolean	 rul__set_attr_integer	     (Molecule, char, Molecule)
 *	Boolean	 rul__set_attr_string	     (Molecule, char, Molecule)
 *	Boolean	 rul__set_comp_attr_string   (Molecule, char, char)
 *	Boolean	 rul__set_comp_elem_atom     (Molecule, char, long, Molecule)
 *	Boolean	 rul__set_comp_elem_float    (Molecule, char, long, Molecule)
 *	Boolean	 rul__set_comp_elem_double   (Molecule, char, long, Molecule)
 *	Boolean	 rul__set_comp_elem_integer  (Molecule, char, long, Molecule)
 *	Boolean	 rul__set_comp_elem_opaque  (Molecule, char, long, Molecule)
 *	Boolean	 rul__set_comp_elem_string   (Molecule, char, long, char)
 *	void	 rul__clear		     (void)
 *      void	 rul__completion	     (void address())
 *	void	 rul__debug                  (void)
 *	Molecule rul__get_firing_rule        (void)
 *	void	 rul__initialize	     (void)
 *	void	 rul__run		     (void)
 *	void	 rul__startup		     (void)
 *	void	 rul__start_addstate_trans   (void)
 *	void	 rul__end_addstate_trans     (void)
 *	void	 rul__start_wme_id_translation(void)
 *	void	 rul__end_wme_id_translation  (void)
 *	Molecule rul__translate_wme_id 	      (Molecule)
 *	Boolean	 rul__is_wid_translation_on   (void)
 */


#include <cbupcase.h>

#include <string.h>
#include <stdio.h>
#include <common.h>
#include <callback.h>
#include <dsc.h>
#include <decl.h>
#include <decl_att.h>
#include <hash.h>
#include <mol.h>
#include <wm.h>
#include <ios.h>
#include <atom.h>
#include <msg.h>
#include <api_msg.h>

EXTERNAL long       rul__rac_rule_firing_cycle;
EXTERNAL Mol_Symbol rul__rac_active_rule_name;

#define RUL_WME_ID_TABLE_SIZE 2048

static Molecule       SM_mod_wme_id; /* current id of wme being modified    */
static Class          SC_mod_wme_cls;/* current class of wme being modified */
static Object         SO_mod_wme;    /* current wme object being modified   */
static long           SL_mod_attrs;  /* current wme attribute count         */

static Molecule	     *SM_result;     /* result and array ???	            */
static unsigned long  SL_result_len; /* lenght of result array              */

/* declare some static molecules */
static Mol_Symbol SM_id;	/* rul__mol_make_symbol ("$ID"); */
static Mol_Symbol SM_inst_of;	/* rul__mol_make_symbol ("$INSTANCE_OF"); */
static Mol_Symbol SM_dbkey;	/* rul__mol_make_symbol ("DBKEY"); */

/*
 *  Flags to determine when/how WME ID translation is performed.
 *  Initialized by the linker to FALSE.
 */

static Boolean		SB_save_translate_state /* = FALSE */;
static Boolean		SB_wid_translate_state  /* = FALSE */;
static Boolean		SB_add_translate_state  /* = FALSE */;
static Hash_Table	SA_wid_trans_table;
static Hash_Table	SA_save_trans_table;



/********************
**                 **
**  RUL__TO_DESC   **
**                 **
********************/

String
*rul__to_desc (char *in_string)
{
  String  *descr;
  char    *str_ptr;

  descr = (String *) rul__mem_malloc (sizeof(String) + strlen(in_string) + 1);
  str_ptr  = (char *) (((long) descr) + sizeof(String));
  strcpy (str_ptr, in_string);

  descr->dsc_w_length  = strlen (in_string);
  descr->dsc_b_dtype   = DSC_K_DTYPE_T;
  descr->dsc_b_class   = DSC_K_CLASS_S;
  descr->dsc_a_pointer = str_ptr;
  
  return (descr);
}

/***************************
**                        **
**  RUL__ASCIC_TO_ASCIZ   **
**                        **
***************************/

void
rul__ascic_to_asciz (char *return_str, char *char_str, short char_len)
{
  long  len;

  len = MIN (char_len, RUL_C_MAX_SYMBOL_SIZE);
  strncpy(return_str, char_str, len);
  return_str[len] = '\0';
}



/****************************
**                         **
**  RUL__VALIDATE_WME_ID   **
**                         **
****************************/

Boolean
rul__validate_wme_id (Molecule wme_id, Object *wme)
{
  if (!rul__mol_is_valid (wme_id)) {
    rul__msg_print (API_NONWMEIDARG, "- not a valid atom");
    return (FALSE);
  }
 
  if (!rul__mol_is_instance_id (wme_id)) {
    rul__msg_print_w_atoms (API_NONWMEIDARG, 1, wme_id);
    return (FALSE);
  }

  if ((*wme = rul__mol_instance_id_value (wme_id)) == NULL) {
    rul__msg_print_w_atoms (API_NOSUCHWME, 1, wme_id);
    return (FALSE);
  }

  return (TRUE);
}

/********************************
**                             **
**  RUL__VALIDATE_BLOCK_CLASS  **
**                             **
********************************/

Boolean
rul__validate_block_class (char     *block_name,
			   char     *class_name,
			   Class    *class_id)
{
  Molecule decl_name = NULL, rclass = NULL;

  if (block_name && strlen(block_name)) {
    decl_name = rul__mol_make_symbol (block_name);
    if (rul__decl_get_block (decl_name) == NULL) {
      rul__msg_print_w_atoms (API_INVDECLBK, 1, decl_name);
      rul__mol_decr_uses (decl_name);
      return FALSE;
    }
  }

  if (class_name && strlen(class_name)) {
    rclass = rul__mol_make_symbol (class_name);
    if (decl_name) {
      *class_id = rul__decl_get_visible_class_rt (rclass, &decl_name, 1);
      if (*class_id == NULL) {
	if (rul__decl_get_class_if_unique (rclass))
	  rul__msg_print_w_atoms (API_CLSNOTBLK, 2, rclass, decl_name);
	else
	  rul__msg_print_w_atoms (API_NOSUCHCLS, 1, rclass);
	return FALSE;
      }
    }
    else {
      *class_id = rul__decl_get_class_if_unique (rclass);
      if (*class_id == NULL) {
	rul__msg_print_w_atoms (API_NOSUCHCLS, 1, rclass);
	rul__mol_decr_uses (rclass);
	return FALSE;
      }
    }
  }
  else {
    rul__msg_print (API_NOCLSNAM);
    return FALSE;
  }
  
  return TRUE;
}

/*******************************
**                            **
**  RUL__VALIDATE_CLASS_ATTR  **
**                            **
*******************************/

Boolean
rul__validate_class_attr (Object    wmo,
			  char     *attr_name,
			  Class    *class_id,
			  Molecule *attr)
{
  /* wmo assumed valid */

  *class_id = rul__wm_get_class (wmo);
  *attr	= rul__mol_make_symbol (attr_name);
  if (!rul__decl_is_attr_in_class (*class_id, *attr)) {
    rul__msg_print_w_atoms (API_ATTNOTCLA, 2, *attr,
			    rul__decl_get_class_name (*class_id));
    rul__mol_decr_uses (*attr);
    *attr = NULL;
  }
  else
    return (TRUE);

  return (FALSE);
}

/*******************************
**                            **
**  RUL__VALIDATE_ATTR_VALUE  **
**                            **
*******************************/

Boolean
rul__validate_attr_value (Class class_id, Molecule attr, Molecule value)
{
  Decl_Shape    shape;
  Molecule_Type value_type;

  /* class, attr assumed valid */

  shape = rul__decl_get_attr_shape (class_id, attr);
  value_type = rul__mol_get_value_type (value);
  if (((shape == shape_compound) && (value_type == compound)) ||
      ((shape == shape_table)    && (value_type == table))    ||
      (((shape != shape_compound) && (value_type != compound)) &&
       ((shape != shape_table)    && (value_type != table)))) {
    return TRUE;
  }
  rul__msg_print_w_atoms (API_INVATTVAL, 2, attr, value);
  return FALSE;
}



/**************************
**                       **
**  RUL__ATOM_IS_FATOM   **
**                       **
**************************/

Boolean
rul__atom_is_fatom (Molecule atom_value)
{
  if (rul__mol_is_valid (atom_value))
    return (rul__mol_is_dbl_atom (atom_value));
  return FALSE;
}


/**************************
**                       **
**  RUL__ATOM_IS_IATOM   **
**                       **
**************************/

Boolean
rul__atom_is_iatom (Molecule atom_value)
{
  if (rul__mol_is_valid (atom_value))
    return (rul__mol_is_int_atom (atom_value));
  return FALSE;
}


/**************************
**                       **
**  RUL__ATOM_IS_SYMBOL  **
**                       **
**************************/

Boolean
rul__atom_is_symbol (Molecule atom_value)
{
  if (rul__mol_is_valid (atom_value))
    return (rul__mol_is_symbol (atom_value));
  return FALSE;
}


/****************************
**                         **
**  RUL__ATOM_IS_COMPOUND  **
**                         **
****************************/

Boolean
rul__atom_is_compound (Molecule atom_value)
{
  if (rul__mol_is_valid (atom_value))
    return (rul__mol_is_compound (atom_value));
  return FALSE;
}


/*******************************
**                            **
**  RUL__ATOM_IS_INSTANCE_ID  **
**                            **
*******************************/

Boolean
rul__atom_is_instance_id (Molecule atom_value)
{
  if (rul__mol_is_valid (atom_value))
    return (rul__mol_is_instance_id (atom_value));
  return FALSE;
}



/**************************
**                       **
**  RUL__ATOM_IS_OATOM  **
**                       **
**************************/

Boolean
rul__atom_is_oatom (Molecule atom_value)
{
  if (rul__mol_is_valid (atom_value))
    return (rul__mol_is_opaque (atom_value));
  return FALSE;
}



/**************************
**                       **
**  RUL__FATOM_TO_FLOAT  **
**                       **
**************************/

float
rul__fatom_to_float (Molecule atom_value)
{
  if (rul__mol_is_valid (atom_value))
    return (rul__mol_dbl_atom_value (atom_value));
  rul__msg_print (API_INVATMPAR, " in FATOM_TO_FLOAT");
  return 0.0;
}

/**************************
**                       **
**  RUL__FATOM_TO_DOUBLE **
**                       **
**************************/

double
rul__fatom_to_double (Molecule atom_value)
{
  if (rul__mol_is_valid (atom_value))
    return (rul__mol_dbl_atom_value (atom_value));
  rul__msg_print (API_INVATMPAR, " in FATOM_TO_DOUBLE");
  return 0.0;
}

/**************************
**                       **
**  RUL__FLOAT_TO_FATOM  **
**                       **
**************************/

Molecule
rul__float_to_fatom (float float_value)
{
  char buf[RUL_C_MAX_SYMBOL_SIZE+1];
  sprintf (buf, "%f", float_value);
  return (rul__string_to_atom (buf));
}

/**************************
**                       **
**  RUL__DOUBLE_TO_FATOM **
**                       **
**************************/

Molecule
rul__double_to_fatom (double double_value)
{
  return (rul__mol_make_dbl_atom (double_value));
}

/******************
**               **
**  RUL__GENSYM  **
**               **
******************/

Molecule
rul__gensym (Molecule prefix)
{
  return (rul__mol_gensym (prefix));
}

/******************
**               **
**  RUL__GENINT  **
**               **
******************/

Molecule
rul__genint (void)
{
  return (rul__mol_genint ());
}



/****************************
**                         **
**  RUL__IATOM_TO_INTEGER  **
**                         **
****************************/

long
rul__iatom_to_integer (Molecule atom_value)
{
  if (rul__mol_is_valid (atom_value))
    return (rul__mol_int_atom_value (atom_value));
  rul__msg_print (API_INVATMPAR, " in IATOM_TO_INTEGER");
  return 0;
}

/****************************
**                         **
**  RUL__INTEGER_TO_IATOM  **
**                         **
****************************/

Molecule
rul__integer_to_iatom (long long_value)
{
  return (rul__mol_make_int_atom (long_value));
}



/****************************
**                         **
**  RUL__OATOM_TO_PTR      **
**                         **
****************************/

void *
rul__oatom_to_ptr (Molecule atom_value)
{
  if (rul__mol_is_valid (atom_value))
    return (rul__mol_opaque_value (atom_value));
  rul__msg_print (API_INVATMPAR, " in OATOM_TO_PTR");
  return 0;
}

/****************************
**                         **
**  RUL__PTR_TO_OATOM   **
**                         **
****************************/

Molecule
rul__ptr_to_oatom (void * ptr_value)
{
  return (rul__mol_make_opaque (ptr_value));
}

/****************************
**                         **
**  RUL__STRING_TO_SYMBOL  **
**                         **
****************************/

Molecule
rul__string_to_symbol (char *char_string)
{
  return (rul__mol_make_symbol (char_string));
}



/****************************
**                         **
**  RUL__SYMBOL_TO_STRING  **
**                         **
****************************/

long
rul__symbol_to_string (char *char_buf, long buf_size, Molecule atom_value)
{

  if (char_buf && buf_size)
    char_buf[0] = '\0';

  if (rul__mol_is_valid (atom_value)) {
    if (rul__mol_is_symbol (atom_value)) {
      if (rul__mol_use_printform (atom_value, char_buf, buf_size))
	return (rul__mol_get_printform_length (atom_value));
      else
	return (RUL_C_INVALID_LENGTH);
    }
    else {
      rul__msg_print_w_atoms (API_NONSYMARG, 1, atom_value);
      return (RUL_C_INVALID_LENGTH);
    }	
  }
  rul__msg_print (API_INVATMPAR, " in SYMBOL_TO_STRING");
  return (RUL_C_INVALID_LENGTH);
}



/**************************
**                       **
**  RUL__STRING_TO_ATOM  **
**                       **
**************************/

Molecule
rul__string_to_atom (char *char_string)
{
  Molecule    atom, id_atom;
  Token_Type  tok_type;

  rul__atom_string_setup (char_string);
  tok_type = rul__atom_get_atom (&atom);
  if (tok_type == TOK_EOF || tok_type == TOK_EOL) {
    if (tok_type == TOK_EOL)
      rul__mol_decr_uses (atom);
    atom = rul__mol_make_symbol ("");
  }
  else if (tok_type == TOK_INSTANCE_ID && rul__is_wid_translation_on ()) {
    id_atom = atom;
    atom = rul__translate_wme_id (id_atom);
    rul__mol_decr_uses (id_atom);
  }

  return atom;
}


/**************************
**                       **
**  RUL__ATOM_TO_STRING  **
**                       **
**************************/

long
rul__atom_to_string (char *char_buf, long buf_size, Molecule atom_value)
{

  if (char_buf && buf_size)
    char_buf[0] = '\0';

  if (rul__mol_is_valid (atom_value)) {
    if (rul__mol_is_atom (atom_value)) {
      if (rul__mol_use_readform (atom_value, char_buf, buf_size))
	return (rul__mol_get_readform_length (atom_value));
      else
	return (RUL_C_INVALID_LENGTH);
    }
    else if (rul__mol_is_compound (atom_value)) {
      if (rul__mol_use_comp_raw_readform (atom_value, char_buf, buf_size))
	return (strlen (char_buf));
      else
	return (RUL_C_INVALID_LENGTH);
    }
    else {
      rul__msg_print_w_atoms (API_NONSYMARG, 1, atom_value);
      return (RUL_C_INVALID_LENGTH);
    }	
  }
  rul__msg_print (API_INVATMPAR, " in ATOM_TO_STRING");
  return (RUL_C_INVALID_LENGTH);
}


/*********************************
**                              **
**  RUL__GET_ATOM_STRING_LENGTH **
**                              **
*********************************/

long
rul__get_atom_string_length (Molecule atom_value)
{
  if (rul__mol_is_valid (atom_value)) {
    if (rul__mol_is_atom(atom_value))
      return (rul__mol_get_readform_length (atom_value));
    else if (rul__mol_is_compound (atom_value))
      return (rul__mol_get_comp_raw_length (atom_value));
    else {
      rul__msg_print_w_atoms (API_NONSYMARG, 1, atom_value);
      return (RUL_C_INVALID_LENGTH);
    }
  }
  rul__msg_print (API_INVATMPAR, " in GET_ATOM_STRING_LENGTH");
  return (RUL_C_INVALID_LENGTH);
}



/******************************
**                           **
**  RUL__STRING_TO_COMPOUND  **
**                           **
******************************/

Molecule
rul__string_to_compound (char *char_string)
{
  Molecule    atom, id_atom;
  Token_Type  tok_type;
  long        i = 0;

  rul__atom_string_setup (char_string);
  rul__mol_start_tmp_comp (10);
  tok_type = rul__atom_get_atom (&atom);
  while (tok_type != TOK_EOF && tok_type != TOK_EOL) {
    if (tok_type != TOK_FLOAT_CONST &&
	tok_type != TOK_INTEGER_CONST &&
	tok_type != TOK_OPAQUE_CONST &&
	tok_type != TOK_INSTANCE_ID &&
	tok_type != TOK_COMPOUND &&
	tok_type != TOK_QUOTED_SYMBOL &&
	tok_type != TOK_MIS_QUOTED_SYMBOL &&
	tok_type != TOK_SYMBOL_CONST) {
      rul__msg_print_w_atoms (API_INVCMPATM, 1, atom);
      rul__mol_decr_uses (atom);
    }
    else {
      if (tok_type == TOK_INSTANCE_ID && rul__is_wid_translation_on ()) {
	id_atom = atom;
	atom = rul__translate_wme_id (id_atom);
	rul__mol_decr_uses (id_atom);
      }
      rul__mol_set_tmp_comp_nth (i++, atom);
    }
    tok_type = rul__atom_get_atom (&atom);
  }
  if (tok_type == TOK_EOL)
    rul__mol_decr_uses (atom);

  return (rul__mol_end_tmp_comp_w_decr ());
}



/**************************
**                       **
** RUL__ATTR_IS_COMPOUND **
**                       **
**************************/

Boolean
rul__attr_is_compound (char *class_name, char *attr_name, char *block_name)
{
  Class    class_id;
  Molecule rclass;
  Molecule attr = NULL;
  Boolean  sts = FALSE;

  if (rul__validate_block_class (block_name, class_name, &class_id)) {
    attr = rul__mol_make_symbol (attr_name);
    if (!rul__decl_is_attr_in_class (class_id, attr)) {
      rclass = rul__mol_make_symbol(class_name);
      rul__msg_print_w_atoms (API_ATTNOTCLA, 2, attr, rclass);
    }
    else {
      sts = (shape_compound == rul__decl_get_attr_shape (class_id, attr));
    }
  }
  
  if (attr)
    rul__mol_decr_uses (attr);

  return (sts);
}

/*************************
**                      **
** RUL__IS_ATTRIBUTE    **
**                      **
*************************/

Boolean
rul__is_attribute (char *class_name, char *attr_name, char *block_name)
{
  Class    class_id;
  Molecule attr = NULL;
  Boolean  sts = FALSE;

  if (rul__validate_block_class (block_name, class_name, &class_id)) {
    attr = rul__mol_make_symbol (attr_name);
    sts = rul__decl_is_attr_in_class (class_id, attr);
  }

  if (attr)
    rul__mol_decr_uses (attr);

  return (sts);
}



/*************************
**                      **
** RUL__IS_CLASS        **
**                      **
*************************/

Boolean
rul__is_class (char *class_name,
	       char *block_name)
{
  Class    class_id;
  Molecule decl_name = NULL, rclass = NULL;

  if (block_name && strlen(block_name)) {
    decl_name = rul__mol_make_symbol (block_name);
    if (rul__decl_get_block (decl_name) == NULL) {
      rul__msg_print_w_atoms (API_INVDECLBK, 1, decl_name);
      rul__mol_decr_uses (decl_name);
      return FALSE;
    }
  }

  if (class_name && strlen(class_name)) {
    rclass = rul__mol_make_symbol (class_name);
    if (decl_name) {
      class_id = rul__decl_get_visible_class_rt (rclass, &decl_name, 1);
      if (class_id == NULL) {
	rul__mol_decr_uses (rclass);
	return FALSE;
      }
    }
    else {
      class_id = rul__decl_get_class_if_unique (rclass);
      if (class_id == NULL) {
	rul__msg_print_w_atoms (API_NOSUCHCLS, 1, rclass);
	rul__mol_decr_uses (rclass);
	return FALSE;
      }
    }
  }
  else {
    rul__msg_print (API_NOCLSNAM);
    return FALSE;
  }

  return TRUE;

}

/*************************
**                      **
** RUL__IS_SUBCLASS     **
**                      **
*************************/

Boolean
rul__is_subclass (char *chi_class, char *par_class, char *block_name)
{
  Class chi_id;
  Class par_id;

  if (rul__validate_block_class (block_name, par_class, &par_id))
    if (rul__validate_block_class (block_name, chi_class, &chi_id))
      return (rul__decl_is_subclass_of (chi_id, par_id) && (chi_id != par_id));
  
  return FALSE;
}

/************************
**                     **
** RUL__IS_WME         **
**                     **
************************/

Boolean
rul__is_wme (Molecule wme_id)
{
  if (rul__mol_is_valid (wme_id)) {
    return (rul__mol_is_instance_id (wme_id) &&
	    (rul__mol_instance_id_value (wme_id) != NULL));
  }
  return (FALSE);
}


/*************************
**                      **
** RUL__GET_ATTR_ATOM   **
**                      **
*************************/

Molecule
rul__get_attr_atom (Molecule wme_id, char *attr_name)
{
  Object     wme;
  Class      class_id;
  Molecule   attr = NULL;
  Molecule   atom = (RUL_C_INVALID_ATOM);
  
  if (rul__validate_wme_id (wme_id, &wme))   {
    if (rul__validate_class_attr (wme, attr_name, &class_id, &attr)) {
/* the get_attr_string* supports compounds 
    if (rul__decl_get_attr_shape (class_id, attr) != shape_atom) {
	rul__msg_print_w_atoms (API_ATTRNOTSCLR, 1, attr);
      }
      else
*/
        atom = rul__wm_get_attr_val (wme, attr);
    }
  }

  rul__mol_incr_uses (atom);
  return (atom);
}



/**************************
**                       **
** RUL__GET_CLASS_STRING **
**                       **
**************************/

long
rul__get_class_string (char	 *char_buf, 
		       long	  buf_len,
		       Molecule   wme_id)
{
  Object      wme;
  Molecule    class_name;

  if (rul__validate_wme_id (wme_id, &wme)) {
    class_name = rul__decl_get_class_name (rul__wm_get_class(wme));
    if (rul__mol_use_readform (class_name, char_buf, buf_len))
      return (rul__mol_get_readform_length (class_name));
  }
  return (RUL_C_INVALID_LENGTH);
}



/*********************************
**                              **
** RUL__GET_CLASS_STRING_LENGTH **
**                              **
*********************************/

long
rul__get_class_string_length (Molecule  wme_id)
{
  Object      wme;
  Molecule    class_name;

  if (rul__validate_wme_id (wme_id, &wme)) {
    class_name = rul__decl_get_class_name (rul__wm_get_class(wme));
    if (class_name)
      return (rul__mol_get_readform_length (class_name));
  }
  return (0);
}



/******************************
**                           **
** RUL__GET_COMP_ATTR_LENGTH **
**                           **
******************************/

long
rul__get_comp_attr_length (Molecule wme_id, char *attr_name)
{
  Object     wme;
  Class      class_id;
  Molecule   attr;
  long       len = (RUL_C_INVALID_LENGTH);

  if (rul__validate_wme_id (wme_id, &wme)) {
    if (rul__validate_class_attr (wme, attr_name, &class_id, &attr)) {
      if (rul__decl_get_attr_shape (class_id, attr) == shape_compound)
	len = rul__mol_get_poly_count (rul__wm_get_attr_val(wme, attr));
      else {
	rul__msg_print_w_atoms (API_ATTRNOTCOMP, 1, attr);
      }
    }
  }
  return len;
}


/*******************************
**                            **
** RUL__GET_COMP_ATTR_STRING  **
**                            **
*******************************/

long
rul__get_comp_attr_string (char     *char_buf,
			   long      buf_len,
			   Molecule  wme_id,
			   char     *attr_name)
{
  Object      wme;
  Class       class_id;
  Molecule    attr = NULL;
  Molecule    value;
  long        len = (RUL_C_INVALID_LENGTH);

  if (rul__validate_wme_id (wme_id, &wme))   {
    if (rul__validate_class_attr (wme, attr_name, &class_id, &attr)) {
      if (rul__decl_get_attr_shape (class_id, attr) == shape_compound) {
	value = rul__wm_get_attr_val (wme, attr);
	if (rul__mol_use_comp_raw_readform (value, char_buf, buf_len))
	  len = strlen (char_buf);
      }
      else {
	rul__msg_print_w_atoms (API_ATTRNOTCOMP, 1, attr);
      }
    }
  }

  return len;
}    



/***********************************
**                                **
** RUL__GET_COMP_ATTR_STRING_LEN  **
**                                **
***********************************/

long
rul__get_comp_attr_string_len (Molecule  wme_id, char *attr_name)
{
  Object      wme;
  Class       class_id;
  Molecule    attr = NULL;
  Molecule    value;
  long        len = (RUL_C_INVALID_LENGTH);

  if (rul__validate_wme_id (wme_id, &wme))   {
    if (rul__validate_class_attr (wme, attr_name, &class_id, &attr)) {
      if (rul__decl_get_attr_shape (class_id, attr) == shape_compound) {
	value = rul__wm_get_attr_val (wme, attr);
	len = rul__mol_get_comp_raw_length (value);
      }
      else {
	rul__msg_print_w_atoms (API_ATTRNOTCOMP, 1, attr);
      }
    }
  }

  return len;
}    




/*********************************
**				**
** RUL__GET_COMP_ELEM_ATOM	**
**				**
*********************************/

Molecule
rul__get_comp_elem_atom (Molecule  wme_id, char *attr_name, long index)
{
  Object    wme;
  Class     class_id;
  Molecule  attr = NULL;
  Molecule  atom = (RUL_C_INVALID_ATOM);
  Molecule  comp_val;
	
  if (rul__validate_wme_id (wme_id, &wme))   {
    if (rul__validate_class_attr (wme, attr_name, &class_id, &attr)) {
      if (rul__decl_get_attr_shape (class_id, attr) == shape_compound) {
	comp_val = rul__wm_get_attr_val (wme, attr);
	atom = rul__mol_get_comp_nth (comp_val, index);
	rul__mol_incr_uses (atom);
      }
      else {
	rul__msg_print_w_atoms (API_ATTRNOTCOMP, 1, attr);
      }
    }
  }

  return atom;
}



/*****************
**		**
** RUL__GET_WME	**
**		**
*****************/

long
rul__get_wme (char *char_buf, long buf_len, Molecule wme_id)
{
  Object	wme;
  
  if (rul__validate_wme_id (wme_id, &wme)) {
    if (rul__wm_use_wmoform (wme, char_buf, buf_len))
      return (rul__wm_get_wmoform_length (wme));
  }
  return (0);
}



/*************************
**	          	**
** RUL__GET_WME_LENGTH	**
**			**
*************************/

long
rul__get_wme_length (Molecule  wme_id)
{
  Object	wme;
  
  if (rul__validate_wme_id (wme_id, &wme)) {
    return (rul__wm_get_wmoform_length (wme));
  }
  return (0);
}



/*************************
**			**
** RUL__GET_NEXT_WME	**
**			**
*************************/

Molecule
rul__get_next_wme (Molecule wme_id)
{
  Molecule next_id;

  if (wme_id == RUL_C_RESET_WM_ATOM)
    next_id = rul__mol_next_instance_id (NULL);
  
  else if (rul__mol_is_instance_id (wme_id))
    next_id = rul__mol_next_instance_id (wme_id);

  else {
    if (rul__mol_is_valid (wme_id))
      rul__msg_print_w_atoms (API_NONWMEIDARG, 1, wme_id);
    else
      rul__msg_print (API_NONWMEIDARG, "- not a valid atom");
    return ((Molecule) RUL_C_INVALID_ATOM);
  }

  while (next_id != NULL) {
    if (rul__mol_instance_id_value (next_id))
      break;
    next_id = rul__mol_next_instance_id (next_id);
  }

  if (next_id)
    rul__mol_incr_uses (next_id);
  
  return (next_id);
  
}



/*************************
**			**
** RUL__COPY_WME	**
**			**
**************************/

Molecule
rul__copy_wme (Molecule wme_id)
{
  Object   wmo = NULL;
  Molecule atom = (RUL_C_INVALID_ATOM);

  if (rul__validate_wme_id (wme_id, &wmo))   {
    wmo = rul__wm_copy (wmo, NULL);
    if (wmo) {
      rul__wm_update_and_notify (wmo, NULL);
      atom = rul__wm_get_instance_id (wmo);
    }
  }
  if (atom != RUL_C_INVALID_ATOM)
    rul__mol_incr_uses (atom);
  return atom;
}



/*************************
**			**
** RUL__MAKE_WME	**
**			**
**************************/

Molecule
rul__make_wme (char *char_string, char *block_name)
{
  long     db_name_count = 0;
  Molecule wid, decl_name = NULL;
  
  if (block_name && strlen(block_name)) {
    decl_name = rul__mol_make_symbol (block_name);
    if (rul__decl_get_block (decl_name) == NULL) {
      rul__msg_print_w_atoms (API_INVDECLBK, 1, decl_name);
      rul__mol_decr_uses (decl_name);
      return NULL;
    }
    else
      db_name_count = 1;
  }

  rul__atom_string_setup (char_string);
  wid = rul___make_wmos (db_name_count, &decl_name);

  if (decl_name)
    rul__mol_decr_uses (decl_name);
  
  return (wid);
}



/*************************
**			**
** RUL___MAKE_WMOS	**
**			**
**************************/

Molecule rul___make_wmos (long db_count, Molecule *db_names)
{
   Boolean     status;
   Molecule    wme_id = NULL, attr, atom, id_atom, decl_name;
   Molecule    class_name = NULL;
   Token_Type  tok_type;
   long        num_attrs = 0;
   long	       j, comp_elmts;
   Class       class_id = NULL;
   Object      wmo;
   Boolean     wrap = FALSE;
   Boolean     tmp_comp = FALSE;
   Boolean     decl_valid;

   if (SM_id == NULL) {
      SM_id = rul__mol_symbol_id ();
      SM_inst_of = rul__mol_make_symbol ("$INSTANCE_OF");
      rul__mol_mark_perm (SM_inst_of);
      SM_dbkey = rul__mol_make_symbol ("DBKEY");
      rul__mol_mark_perm (SM_dbkey);
   }
   
   status = TRUE;
   class_name = NULL;
   decl_name = NULL;
   
   /* looking for ['('] ['declblk'~]'class' 
    *  			['^' 'attr' [(compound...)] ...] [')']
    */
   
   /* get the first token, must be a valid class */
   tok_type = rul__atom_get_atom(&atom);
   while (tok_type != TOK_EOF) {
      if (tok_type == TOK_SYMBOL_CONST ||
	  tok_type == TOK_QUOTED_SYMBOL ||
	  tok_type == TOK_COMPOUND ||
	  tok_type == TOK_MIS_QUOTED_SYMBOL)
	break;
      if (tok_type == TOK_LPAREN)
	wrap = TRUE;
      rul__mol_decr_uses (atom);
      tok_type = rul__atom_get_atom (&atom);
   }
   
   /* class/block name must be symbolic */
   if (tok_type == TOK_SYMBOL_CONST ||
       tok_type == TOK_QUOTED_SYMBOL ||
       tok_type == TOK_COMPOUND ||
       tok_type == TOK_MIS_QUOTED_SYMBOL) {
      
      class_name = atom;
      atom = NULL;
      
      tok_type = rul__atom_get_atom (&atom);
      if (tok_type == TOK_TILDE) {
	 decl_name = class_name;
	 class_name = NULL;
	 tok_type = rul__atom_get_atom (&atom);
	 
	 if (tok_type == TOK_SYMBOL_CONST ||
	     tok_type == TOK_QUOTED_SYMBOL ||
	     tok_type == TOK_COMPOUND ||
	     tok_type == TOK_MIS_QUOTED_SYMBOL) {
	    class_name = atom;
	    atom = NULL;
	 }
      }
      
      if (class_name) {
	 
	 class_id = NULL;
	 decl_valid = FALSE;
	 
	 if (decl_name) {	/* verify decl block */
	    if (db_count) {
	       for (j = 0; j < db_count; j++) {
		  if (decl_name == db_names[j]) {
		     decl_valid = TRUE;
		     break;
		  }
	       }
	       if (decl_valid == FALSE) {
		  status = FALSE;
		  rul__msg_print_w_atoms (API_INVBLOCK, 1, decl_name);
	       }
	       else if (rul__decl_get_block (decl_name) == NULL) {
		  status = FALSE;
		  rul__msg_print_w_atoms (API_INVBLOCK, 1, decl_name);
	       }
	    }
	    if (status != FALSE) 
	      class_id = rul__decl_get_visible_class_rt (class_name,
							 &decl_name, 1);
	 }
	 else {
	    if (db_count)
	      class_id = rul__decl_get_visible_class_rt (class_name,
							 db_names, db_count);
	    else
	      class_id = rul__decl_get_class_if_unique (class_name);
	 }
	 
	 if (class_id != NULL) {
	    status = TRUE;
	    num_attrs = rul__decl_get_class_num_attrs (class_id) + 1;
	    if (SL_result_len < num_attrs) {
	       SM_result = rul__mem_realloc (SM_result, (num_attrs*4));
	       SL_result_len = num_attrs;
	    }
	    for (j = 0; j < SL_result_len; j++) { SM_result[j] = NULL; }
	 }
	 else {
	    /* invalid class */
	    status = FALSE;
	    rul__msg_print_w_atoms (API_INVCLASS, 1, class_name);
	    rul__mol_decr_uses (class_name);
	 }
      }
   }
   else {
      /* invalid class name */
      if (tok_type != TOK_EOF) {
	 status = FALSE;
	 rul__msg_print_w_atoms (API_INVCLSNAM, 1, class_name);
	 rul__mol_decr_uses (class_name);
	 atom = NULL;
      }
   }
   assert (tok_type == TOK_EOF || status == FALSE || class_name != NULL);
   
   while (tok_type != TOK_EOF && status) {
      
      /* loop for finding the attribute */
      
      attr = NULL;
      while (attr == NULL) {
	 
	 /* TOK_HAT or TOK_RPAREN should be next*/
	 if (atom == NULL)
	   tok_type = rul__atom_get_atom (&atom);
	 
	 /* possibly ignore anything until ^ ??? */
	 if (tok_type == TOK_RPAREN) {
	    rul__mol_decr_uses (atom);
	    atom = NULL;
	    if (wrap)
	      break;
	    rul__msg_print (API_UNMRPAREN);
	    continue;
	 }
	 
	 if (tok_type == TOK_EOF) {
	    tok_type = TOK_RPAREN;
	    break;
	 }
	 
	 if (tok_type != TOK_HAT) {
	    rul__msg_print_w_atoms (API_INVWMEHAT, 1, atom);
	    status = FALSE;
	    rul__mol_decr_uses (atom);
	    atom = NULL;
	    break;
	 }
	 
	 /* decr uses on ^ */
	 rul__mol_decr_uses (atom);
	 atom = NULL;
	 
	 /* the attribute */
	 tok_type = rul__atom_get_atom (&atom);
	 
	 if (tok_type == TOK_EOF) {
	    /* Message will be output later. */
	    status = FALSE;
	    break;
	 }
	 
	 if (tok_type == TOK_EOL) {
	    rul__mol_decr_uses (atom);
	    atom = NULL;
	    if (wrap)
	      continue;	/* just skip... */
	    status = FALSE;
	    break;
	 }
	 
	 if (tok_type != TOK_SYMBOL_CONST &&
	     tok_type != TOK_QUOTED_SYMBOL &&
	     tok_type != TOK_COMPOUND &&
	     tok_type != TOK_MIS_QUOTED_SYMBOL) {
	    rul__msg_print_w_atoms (API_INVATTSTR, 1, atom);
	    rul__mol_decr_uses (atom);
	    atom = NULL;
	    status = FALSE;
	    break;
	 }
	 
	 if (!rul__decl_is_attr_in_class (class_id, atom)) {
	    rul__msg_print_w_atoms (API_INVCLSATT, 2, class_name, atom);
	    rul__mol_decr_uses (atom);
	    atom = NULL;
	    status = FALSE;
	    break;
	 }
	 
	 attr = atom;
	 atom = NULL;
      }
      
      if (tok_type == TOK_EOF || tok_type == TOK_RPAREN || status == FALSE)
	break;
      
      /* get the attribute value */
      tok_type = rul__atom_get_atom (&atom);
      
      if (tok_type == TOK_EOF) {
	  /* Message will be output later. */
	  status = FALSE;
	  break;
      }
      
      /* check for '(compound [values...] )' */
      if (tok_type == TOK_LPAREN) {
	 
	 /* next token must be a compound */
	 
	 rul__mol_decr_uses (atom);
	 tok_type = rul__atom_get_atom (&atom);
	 
	 if (tok_type != TOK_COMPOUND) {
	    /* error... encounter left-paren, not followed by compound */
	    rul__msg_print_w_atoms (API_BADCMPVAL, 1, atom);
	    status = FALSE;
	    break;
	 }
	 else if (rul__decl_get_attr_shape (class_id,
					    attr) != shape_compound) {
	    rul__msg_print_w_atoms (API_ATTRNOTCOMP, 1, attr);
	    status = FALSE;
	    break;
	 }
	 else {	/* a valid compound attr/value */
	    
	    rul__mol_decr_uses (atom);
	    atom = NULL;
	    comp_elmts = 0;
	    tmp_comp = TRUE;
	    rul__mol_start_tmp_comp (10);
	    tok_type = rul__atom_get_atom (&atom);
	    
	    while (TRUE) {
	    
	       if (tok_type == TOK_EOF) {
		  status = FALSE;
		  break;
	       }
	       else if (tok_type == TOK_RPAREN) {
		  rul__mol_decr_uses (atom);
		  atom = rul__mol_end_tmp_comp_w_decr ();
		  tmp_comp = FALSE;
		  break;
	       }
	       else if (tok_type == TOK_EOL && wrap) {
		  rul__mol_decr_uses (atom);
		  atom = NULL;
		  continue; /* skip it */
	       }
	       else if (tok_type != TOK_FLOAT_CONST &&
			tok_type != TOK_INTEGER_CONST &&
			tok_type != TOK_OPAQUE_CONST &&
			tok_type != TOK_INSTANCE_ID &&
			tok_type != TOK_COMPOUND &&
			tok_type != TOK_QUOTED_SYMBOL &&
			tok_type != TOK_MIS_QUOTED_SYMBOL &&
			tok_type != TOK_SYMBOL_CONST) {
		  rul__msg_print_w_atoms (API_INVCMPATM, 1, atom);
		  status = FALSE;
		  rul__mol_decr_uses (atom);
		  atom = NULL;
	       }
	       else {
		  if (tok_type == TOK_INSTANCE_ID &&
		      rul__is_wid_translation_on ()) {
		     id_atom = atom;
		     atom = rul__translate_wme_id (id_atom);
		     rul__mol_decr_uses (id_atom);
		  }
		  rul__mol_set_tmp_comp_nth (comp_elmts++, atom);
	       }
	       tok_type = rul__atom_get_atom (&atom);
	    }
	 }
      }
      else {
	 if (tok_type != TOK_FLOAT_CONST &&
	     tok_type != TOK_INTEGER_CONST &&
	     tok_type != TOK_OPAQUE_CONST &&
	     tok_type != TOK_INSTANCE_ID &&
	     tok_type != TOK_COMPOUND &&
	     tok_type != TOK_QUOTED_SYMBOL &&
	     tok_type != TOK_MIS_QUOTED_SYMBOL &&
	     tok_type != TOK_SYMBOL_CONST) {
	    rul__msg_print_w_atoms (API_INVVALTYP, 2, atom, attr);
	    status = FALSE;
	    rul__mol_decr_uses (atom);
	    atom = NULL;
	 }
	 else {
	    if (tok_type == TOK_INSTANCE_ID && rul__is_wid_translation_on ()) {
	       id_atom = atom;
	       atom = rul__translate_wme_id (id_atom);
	       rul__mol_decr_uses (id_atom);
	    }
	 }
      }
      
      if (status &&
	  (attr != SM_id || rul__is_wid_translation_on ()) &&
	  attr != SM_inst_of)
	SM_result[rul__decl_get_attr_offset (class_id, attr)] = atom;
      
      atom = NULL;
   }
   
   if (status && class_id) {
      
      if (rul__is_wid_translation_on () && SM_result[DECL_ID_ATTR_OFFSET])
	wme_id = SM_result[DECL_ID_ATTR_OFFSET];
      wmo = rul__wm_create (class_id, wme_id, NULL);
      if (wmo) {
	 for (j = DECL_FIRST_USER_ATTR_OFFSET; j < num_attrs; j++) {
	    if (SM_result[j] != NULL) {
	       rul__wm_set_offset_val (wmo, j, SM_result[j], NULL);
	       rul__mol_decr_uses (SM_result[j]);
	    }
	 }
	 rul__wm_update_and_notify (wmo, NULL);
	 wme_id = rul__wm_get_instance_id (wmo);
      }
   }
   
   if (tmp_comp == TRUE) {
      atom = rul__mol_end_tmp_comp_w_decr ();
      rul__mol_decr_uses (atom);
      atom = NULL;
   }
   
   if (wme_id)
     rul__mol_incr_uses (wme_id);
   
   if (tok_type == TOK_EOF && status == FALSE)
     rul__msg_print (API_INVWMEEOF);
   
   if (status == FALSE && wme_id == NULL && (SB_add_translate_state))
     return (Molecule) -1;
   
   return (wme_id);
}



/**************************
**			 **
** RUL__MODIFY_WME_START **
**			 **
***************************/

Boolean
rul__modify_wme_start (Molecule wme_id)
{
  Boolean status = FALSE;
  long    j;

  if (SM_mod_wme_id)
    rul__modify_wme_end (SM_mod_wme_id);

  if (rul__validate_wme_id (wme_id, &SO_mod_wme)) {
    SM_mod_wme_id = wme_id;
    SC_mod_wme_cls = rul__wm_get_class (SO_mod_wme);
    SL_mod_attrs = rul__decl_get_class_num_attrs (SC_mod_wme_cls) + 1;
    if (SL_result_len < SL_mod_attrs) {
      SM_result = rul__mem_realloc (SM_result,
				    (sizeof (Molecule) * SL_mod_attrs));
      SL_result_len = SL_mod_attrs;
    }
    for (j = 0; j < SL_result_len; j++) { SM_result[j] = NULL; }
    status = TRUE;
  }
  return (status);
}


/**************************
**			 **
** RUL__MODIFY_WME_END   **
**			 **
***************************/

Boolean
rul__modify_wme_end (Molecule wme_id)
{
  Boolean status = FALSE;
  long    j;

  if ((wme_id == SM_mod_wme_id) &&
      (rul__validate_wme_id (wme_id, &SO_mod_wme))) {
    rul__wm_modify (SO_mod_wme, NULL);
    for (j = DECL_FIRST_USER_ATTR_OFFSET; j < SL_mod_attrs; j++) {
      if (SM_result[j] != NULL) {
	rul__wm_set_offset_val (SO_mod_wme, j, SM_result[j], NULL);
	rul__mol_decr_uses (SM_result[j]);
      }
    }
    rul__wm_update_and_notify (SO_mod_wme, NULL);
    status = TRUE;
  }
  SO_mod_wme = NULL;
  SC_mod_wme_cls = NULL;
  SM_mod_wme_id = NULL;

  return (status);
}



/*************************
**			**
** RUL__SPECIALIZE_WME	**
**			**
**************************/

Molecule
rul__specialize_wme (Molecule wme_id, char *class_string)
{
  Molecule    class_name, par_class;
  Class	      class_id, par_class_id;
  Object      wmo = NULL;
  Molecule    atom = (RUL_C_INVALID_ATOM);

  if (rul__validate_wme_id (wme_id, &wmo))   {
    class_name = rul__mol_make_symbol (class_string);
    par_class_id = rul__wm_get_class (wmo);
    class_id = rul__decl_get_class_if_unique (class_name);
    if (class_id != NULL) {
      if (rul__decl_is_subclass_of (class_id, par_class_id) &&
	  class_id != par_class_id) {
	wmo = rul__wm_specialize (wmo, class_id, NULL);
	if (wmo != NULL) {
	  rul__wm_update_and_notify (wmo, NULL);
	  atom = rul__wm_get_instance_id (wmo);
	}
      }
      else {
	par_class = rul__decl_get_class_name (par_class_id);
	rul__msg_print_w_atoms (API_INVSPECLS, 2, class_name, par_class);
      }
    }
    else {
      rul__msg_print_w_atoms (API_INVCLASS, 1, class_name);
      rul__mol_decr_uses (class_name);
    }
  }

  if (atom != RUL_C_INVALID_ATOM)
    rul__mol_incr_uses (atom);

  return atom;
}



/*************************
**			**
** RUL__REMOVE_CLASS    **
**			**
**************************/

Boolean
rul__remove_class (char *class_name)
{
  Class    class_id;
  Molecule rclass = rul__mol_make_symbol (class_name);
  Boolean  sts = FALSE;

  class_id = rul__decl_get_visible_class (rclass);

  if (class_id) {
    rul__wm_destroy_and_notify_all (class_id, NULL);
    sts = TRUE;
  }
  else
    rul__msg_print (API_INVCLASS, class_name);

  rul__mol_decr_uses (rclass);
  return sts;
}



/*************************
**			**
** RUL__REMOVE_WME	**
**			**
**************************/

Boolean
rul__remove_wme (Molecule wme_id)
{
  Object wme;

  if (rul__validate_wme_id (wme_id, &wme)) {
    rul__wm_destroy_and_notify (wme, NULL);
    return (TRUE);
  }
  
  return FALSE;
}



/*************************
**			**
**  RUL__SET_ATTR_ATOM	**
**			**
*************************/

Boolean
rul__set_attr_atom (Molecule wme_id, char *attr_name, Molecule value)
{
  Boolean       status;
  Object        wme;
  Class         class_id;
  Molecule      attr = NULL;
  Molecule      rclass;
  Molecule      new_val = value;

  if (SM_id == NULL) {
    SM_id = rul__mol_symbol_id ();
    SM_inst_of = rul__mol_make_symbol ("$INSTANCE_OF");
    rul__mol_mark_perm (SM_inst_of);
    SM_dbkey = rul__mol_make_symbol ("DBKEY");
    rul__mol_mark_perm (SM_dbkey);
  }

  if ((status = rul__validate_wme_id (wme_id, &wme))) {
    if (wme_id == SM_mod_wme_id) {
      attr = rul__mol_make_symbol (attr_name);
      class_id = SC_mod_wme_cls;
      rclass = rul__decl_get_class_name (class_id);
      wme = SO_mod_wme;
      if (!rul__decl_is_attr_in_class (class_id, attr)) {
	/* attr not in class */
	rul__msg_print_w_atoms (API_ATTNOTCLA, 2, attr, rclass);
	rul__mol_decr_uses (attr);
	return FALSE;
      }
    }
    else { /* not the current modify object */
      if (!rul__validate_class_attr (wme, attr_name, &class_id, &attr)) {
	return FALSE;
      }
    }

    if (!rul__mol_is_valid (value)) {
      rul__msg_print (API_INVATMPAR, " for VALUE");
      if (attr)
	rul__mol_decr_uses (attr);
      return FALSE;
    }

    if ((status = rul__validate_attr_value (class_id, attr, value))) {
      if (attr != SM_id && attr != SM_inst_of) {
	if (rul__is_wid_translation_on () &&
	    rul__mol_get_value_type (value) == instance_id) {
	  new_val = rul__translate_wme_id (value);
	}
	if (wme_id == SM_mod_wme_id) {
	  SM_result[rul__decl_get_attr_offset (class_id, attr)] = new_val;
	}
	else {
	  rul__wm_modify (wme, NULL);
	  rul__wm_set_attr_val (wme, attr, new_val, NULL);
	  rul__wm_update_and_notify (wme, NULL);
	}
      }
    }
  }
  if (attr)
    rul__mol_decr_uses (attr);

  return (status);
}


/**************************
**                       **
**  RUL__SET_ATTR_FLOAT	 **
**                       **
**************************/

Boolean
rul__set_attr_float (Molecule  wme_id, char *attr_name, float value)
{
  Boolean  sts;
  Molecule atom = rul__float_to_fatom (value);

  sts = rul__set_attr_atom (wme_id, attr_name, atom);
  rul__mol_decr_uses (atom);
  return sts;
}

/**************************
**                       **
**  RUL__SET_ATTR_DOUBLE **
**                       **
**************************/

Boolean
rul__set_attr_double (Molecule  wme_id, char *attr_name, double value)
{
  Boolean  sts;
  Molecule atom = rul__mol_make_dbl_atom (value);

  sts = rul__set_attr_atom (wme_id, attr_name, atom);
  rul__mol_decr_uses (atom);
  return sts;
}

/****************************
**                         **
**  RUL__SET_ATTR_INTEGER  **
**                         **
****************************/

Boolean
rul__set_attr_integer (Molecule  wme_id, char *attr_name, long value)
{
  Boolean sts;
  Molecule atom = rul__mol_make_int_atom (value);

  sts = rul__set_attr_atom (wme_id, attr_name, atom);
  rul__mol_decr_uses (atom);
  return sts;
}

/***************************
**                        **
**  RUL__SET_ATTR_STRING  **
**                        **
***************************/

Boolean
rul__set_attr_string (Molecule wme_id, char *attr_name, char *value)
{
  Boolean sts;
  Molecule atom = rul__mol_make_symbol (value);

  sts = rul__set_attr_atom (wme_id, attr_name, atom);
  rul__mol_decr_uses (atom);
  return sts;
}



/*********************************
**				**
**  RUL__SET_COMP_ATTR_STRING	**
**				**
*********************************/

Boolean
rul__set_comp_attr_string (Molecule  wme_id, char *attr_name, char *values)
{
  Object      wme;
  Class       class_id;
  Molecule    attr = NULL;
  Molecule    rclass;
  Molecule    atom;		/* Contains interned atom from string */
  Molecule    comp_value;	/* We build a new c.v. in here */
  Token_Type  tok_type;

  /* Ensure WME exists and is new-style. */
  if (!rul__validate_wme_id (wme_id, &wme))
    return FALSE;

  if (wme_id == SM_mod_wme_id) {
    attr = rul__mol_make_symbol (attr_name);
    class_id = SC_mod_wme_cls;
    rclass = rul__decl_get_class_name (class_id);
    if (!rul__decl_is_attr_in_class (class_id, attr)) {
      rul__msg_print_w_atoms (API_ATTNOTCLA, 2, attr, rclass);
      rul__mol_decr_uses (attr);
      return FALSE;
    }
  }
  else {
    if (!rul__validate_class_attr (wme, attr_name, &class_id, &attr)) {
      return FALSE;
    }
  }
  if (rul__decl_get_attr_shape (class_id, attr) != shape_compound) {
    rul__msg_print_w_atoms (API_ATTRNOTCOMP, 1, attr);
    return FALSE;
  }
  
  comp_value = rul__string_to_compound (values);

  if (wme_id == SM_mod_wme_id) {
    SM_result[rul__decl_get_attr_offset (class_id, attr)] = comp_value;
  }
  else {
    rul__wm_modify (wme, NULL);
    rul__wm_set_attr_val (wme, attr, comp_value, NULL);
    rul__wm_update_and_notify (wme, NULL);
  }

  return TRUE;
}


/*********************************
**				**
**  RUL__SET_COMP_ELEM_ATOM	**
**				**
*********************************/

Boolean
rul__set_comp_elem_atom (Molecule  wme_id,
			 char     *attr_name,
			 long      index,
			 Molecule  value)
{
  Boolean       status;
  Object        wme;
  Class         class_id;
  Molecule      attr = NULL;
  Molecule      rclass;
  Molecule      comp_value, sub_comp1, sub_comp2;
  long          offset, comp_len;
  Molecule      new_val = value;

  if (index <= 0) {
    rul__msg_print (API_INVCOMPIDX, index);
    return FALSE;
  }

  if (!rul__mol_is_valid (value)) {
    rul__msg_print (API_INVATMPAR, " for VALUE");
    return FALSE;
  }

  if ((status = rul__validate_wme_id (wme_id, &wme))) {
    if (wme_id == SM_mod_wme_id) {
      attr = rul__mol_make_symbol (attr_name);
      class_id = SC_mod_wme_cls;
      rclass = rul__decl_get_class_name (class_id);
      wme = SO_mod_wme;
      if (!(status = rul__decl_is_attr_in_class (class_id, attr))) {
	rul__msg_print_w_atoms (API_ATTNOTCLA, 2, attr, rclass);
	rul__mol_decr_uses (attr);
	return FALSE;
      }
    }
    else {
      if (!rul__validate_class_attr (wme, attr_name, &class_id, &attr)) {
	return FALSE;
      }
    }

    if (rul__decl_get_attr_shape (class_id, attr) != shape_compound) {
      rul__msg_print_w_atoms (API_ATTRNOTCOMP, 1, attr);
      return FALSE;
    }

    offset = rul__decl_get_attr_offset (class_id, attr);
    if (wme_id == SM_mod_wme_id) {
      if (SM_result[offset] == NULL)
	SM_result[offset] = rul__wm_get_offset_val (wme, offset);
      comp_value = SM_result[offset];
    }
    else {
      comp_value = rul__wm_get_offset_val (wme, offset);
    }
    
    if (rul__mol_get_value_type (value) == compound) {
      comp_len = rul__mol_get_poly_count (comp_value);
      sub_comp1 = rul__mol_subcomp (comp_value, 1, index);
      sub_comp2 = rul__mol_subcomp (comp_value, index+1, comp_len);
      comp_value = rul__mol_make_comp (3, sub_comp1, value, sub_comp2);
      rul__mol_decr_uses (sub_comp1);
      rul__mol_decr_uses (sub_comp2);
    }
    else {
      if (rul__is_wid_translation_on () &&
	  rul__mol_get_value_type (value) == instance_id) {
	new_val = rul__translate_wme_id (value);
      }
      comp_value = rul__mol_set_comp_nth (comp_value, index, new_val, NULL);
    }

    if (wme_id == SM_mod_wme_id) {
      SM_result[offset] = comp_value;
    }
    else {
      rul__wm_modify (wme, NULL);
      rul__wm_set_offset_val (wme, offset, comp_value, NULL);
      rul__wm_update_and_notify (wme, NULL);
    }
  }
  if (attr)
    rul__mol_decr_uses (attr);

  return (status);
}



/**********************************
**                               **
**  RUL__SET_COMP_ELEM_FLOAT     **
**                               **
**********************************/

Boolean
rul__set_comp_elem_float (Molecule  wme_id,
			  char     *attr_name,
			  long      index,
			  float     value)
{
  Boolean sts;
  Molecule atom = rul__float_to_fatom (value);

  sts = rul__set_comp_elem_atom (wme_id, attr_name, index, atom);
  rul__mol_decr_uses (atom);
  return sts;
}

/**********************************
**                               **
**  RUL__SET_COMP_ELEM_DOUBLE    **
**                               **
**********************************/

Boolean
rul__set_comp_elem_double (Molecule  wme_id,
			   char     *attr_name,
			   long      index,
			   double    value)
{
  Boolean sts;
  Molecule atom = rul__mol_make_dbl_atom (value);

  sts = rul__set_comp_elem_atom (wme_id, attr_name, index, atom);
  rul__mol_decr_uses (atom);
  return sts;
}



/**********************************
**                               **
**  RUL__SET_COMP_ELEM_INTEGER   **
**                               **
**********************************/

Boolean
rul__set_comp_elem_integer (Molecule  wme_id,
			    char     *attr_name,
			    long      index,
			    long      value)
{
  Boolean sts;
  Molecule atom = rul__mol_make_int_atom (value);

  sts = rul__set_comp_elem_atom (wme_id, attr_name, index, atom);
  rul__mol_decr_uses (atom);
  return sts;
}



/**********************************
**                               **
**  RUL__SET_COMP_ELEM_OPAQUE   **
**                               **
**********************************/

Boolean
rul__set_comp_elem_opaque (Molecule  wme_id,
			    char     *attr_name,
			    long      index,
			    void      *value)
{
  Boolean sts;
  Molecule atom = rul__mol_make_opaque (value);

  sts = rul__set_comp_elem_atom (wme_id, attr_name, index, atom);
  rul__mol_decr_uses (atom);
  return sts;
}

/**********************************
**                               **
**  RUL__SET_COMP_ELEM_STRING    **
**                               **
**********************************/

Boolean
rul__set_comp_elem_string (Molecule  wme_id,
			   char     *attr_name,
			   long      index,
			   char     *value)
{
  Boolean sts;
  Molecule atom = rul__mol_make_symbol (value);

  sts = rul__set_comp_elem_atom (wme_id, attr_name, index, atom);
  rul__mol_decr_uses (atom);
  return sts;
}



/*************************
**                      **
**  RUL__CLEAR		**
**                      **
*************************/

void
rul__clear (void)
{
  rul__msg_print (API_OBSOLFEAT,
		  "OPS_CLEAR, replace with RUL_REMOVE_INSTANCE");
}


/*************************
**                      **
**  RUL__COMPLETION	**
**                      **
*************************/

void
rul__completion (void (*address)(void))
{
  rul__msg_print (API_OBSOLFEAT,
		  "OPS_COMPLETION, replace with RETURN in ENTRY-BLOCK");
}




/***************************
**                        **
**  RUL__GET_FIRING_RULE  **
**                        **
***************************/

Molecule
rul__get_firing_rule (void)
{
  return (rul__rac_active_rule_name);
}


/*************************
**                      **
**  RUL__INITIALIZE	**
**                      **
*************************/

void
rul__initialize (void)
{
  rul__mol_init ();
  rul__ios_init ();
}


/*************************
**                      **
**  RUL__RUN		**
**                      **
*************************/

void
rul__run (void)
{
  rul__msg_print (API_OBSOLFEAT,
		  "OPS_RUN, replace with call to ENTRY-BLOCK");
}


/*************************
**                      **
**  RUL__STARTUP	**
**                      **
*************************/

void
rul__startup (void)
{
  rul__msg_print (API_OBSOLFEAT,
		  "OPS_STARTUP, replace with call to ENTRY-BLOCK");
}



/************************************
**                                 **
**  RUL__START_ADDSTATE_TRANS      **
**                                 **
************************************/

void
rul__start_addstate_trans ()
{
  SB_save_translate_state = SB_wid_translate_state;
  if (SB_wid_translate_state) {
    SA_save_trans_table = SA_wid_trans_table;
  }
  SB_wid_translate_state = TRUE;
  SB_add_translate_state = TRUE;
  SA_wid_trans_table = rul__hash_create_table (RUL_WME_ID_TABLE_SIZE);
}

/**********************************
**                               **
**  RUL__END_ADDSTATE_TRANS      **
**                               **
**********************************/

void
rul__end_addstate_trans ()
{
  if (SB_wid_translate_state)
    rul__hash_delete_table (SA_wid_trans_table);
  if (SB_save_translate_state)
    SA_wid_trans_table = SA_save_trans_table;
  SB_wid_translate_state = SB_save_translate_state;
  SB_save_translate_state = FALSE;
  SB_add_translate_state = FALSE;
}



/************************************
**                                 **
**  RUL__START_WME_ID_TRANSLATION  **
**                                 **
************************************/

void
rul__start_wme_id_translation (void)
{
  if (SB_wid_translate_state) {
    rul__msg_print (API_WIDTRANSON);
  }
  else {
    SB_wid_translate_state = TRUE;
    SA_wid_trans_table = rul__hash_create_table (RUL_WME_ID_TABLE_SIZE);
  }
}

/**********************************
**                               **
**  RUL__END_WME_ID_TRANSLATION  **
**                               **
**********************************/

void
rul__end_wme_id_translation (void)
{
  if (SB_wid_translate_state) {
    SB_wid_translate_state = FALSE;
    rul__hash_delete_table (SA_wid_trans_table);
  }
  else {
    rul__msg_print (API_WIDTRANSOFF);
  }
}



/***************************
**                        **
** RUL__TRANSLATE_WME_ID  **
**                        **
***************************/

Molecule
rul__translate_wme_id (Molecule wme_id)
{
  Molecule  real_id, key;
  char      id_str[RUL_C_MAX_SYMBOL_SIZE+1];

  if (rul__is_wid_translation_on ()) {
    rul__mol_use_printform (wme_id, id_str, RUL_C_MAX_SYMBOL_SIZE);
    key = rul__mol_make_symbol (id_str);
    real_id = (Molecule) rul__hash_get_entry (SA_wid_trans_table, key);
    if (real_id == NULL) {
      real_id = rul__mol_make_instance_id (NULL);
      rul__hash_add_entry (SA_wid_trans_table, key, (void *) real_id);
    }
    rul__mol_decr_uses (key);
    return (real_id);
  }
  
  return (wme_id);
}

/********************************
**                             **
** RUL__IS_WID_TRANSLATION_ON  **
**                             **
********************************/

Boolean
rul__is_wid_translation_on (void)
{
  return (SB_wid_translate_state);
}

