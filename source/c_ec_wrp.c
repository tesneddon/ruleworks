/****************************************************************************
**                                                                         **
**                 C M P _  E M I T _ C _ W R A P . C                      **
**                                                                         **
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
 *	RuleWorks compiler
 *
 * ABSTRACT:
 *	This module contains the C code generator implementation
 *	of the generic low-level code emitter.
 *
 *	Specifically, this file contains the subset of the gen
 *	routines that do not need any private knowledge of oher 
 *	subsystems.  Thes are primarily statement creation functions.
 *
 * MODIFIED BY:
 *	DEC	Digital Equipment Corporation
 *	CPQ	Compaq Computer Corporation
 *
 * MODIFICATION HISTORY:
 *
 *	28-Jan-1993	DEC	Initial version
 *  18-Dec-1997 DEC	rul__emit_global_external_ptr function added
 *	01-Dec-1999	CPQ	Release with GPL
 */

#include <common.h>
#include <cmp_comm.h>
#include <emit.h>
#include <emit_c.h>
#include <val.h>
#include <ver_msg.h>
#include <mol.h>

/*
 * Forward declarations
 */
static void emit_string_to_buf (char *str);
static void emit_append_to_buf (char *str);
static void emit_buffer (void);
static char *relation_to_string (Relation_Operator oper);
static char *rul_type_to_string (Rul_Internal_Type type);
static char *animal_type_to_string (Ext_Type type);
static void emit_stack_internal (Rul_Internal_Type var_type,
				 Ext_Mech var_mech,
				 char *var_name,
				 Cardinality size);


static Boolean	SB_func_args_in_progress = FALSE;
static long	SL_func_arg_count = 0;
static Boolean	SB_func_defn_in_progress = FALSE;
static char     SC_local_buf [RUL_C_MAX_SYMBOL_SIZE+1];


static void emit_string_to_buf (char *str)
{
  strcpy (SC_local_buf, str);
}

static void emit_append_to_buf (char *str)
{
  strcat (SC_local_buf, str);
}

static void emit_buffer (void)
{
  rul___emit_string (SC_local_buf);
}



/*************************
**                      **
**  RELATION_TO_STRING  **
**                      **
*************************/

static char *relation_to_string (Relation_Operator oper)
{
	switch (oper) {
	    case EMIT__E_REL_EQ:	return (" == ");	break;
	    case EMIT__E_REL_NEQ:	return (" != ");	break;
	    case EMIT__E_REL_LT:	return (" < "); 	break;
	    case EMIT__E_REL_LTE:	return (" <= "); 	break;
	    case EMIT__E_REL_GT:	return (" > "); 	break;
	    case EMIT__E_REL_GTE:	return (" >= "); 	break;
	    case EMIT__E_REL_AND:	return (" && "); 	break;
	    case EMIT__E_REL_OR:	return (" || "); 	break;
	    case EMIT__E_REL_NOT:	return (" ! "); 	break;
	}
	assert (FALSE);
	return (" ");
}



/*************************
**                      **
**  RUL_TYPE_TO_STRING  **
**                      **
*************************/

static char *rul_type_to_string (Rul_Internal_Type type)
{
	switch (type) {
	    case CMP__E_INT_OBJECT:	return ("RulObject");
					break;
	    case CMP__E_INT_VOID_PTR:	return ("RulPointer");
				 	break;
	    case CMP__E_INT_LONG:	return ("long");
				 	break;
	    case CMP__E_INT_CS:		return ("RulConflict_Subset");
					break;
	    case CMP__E_INT_CS_ENTRY:	return ("RulConflict_Set_Entry");
				 	break;
	    case CMP__E_INT_REFRACTION_SET: return ("RulRefraction_Set");
					break;
	    case CMP__E_INT_DELTA_TOK:	return ("RulDelta_Token");
				 	break;
	    case CMP__E_INT_DELTA_STRUCT:  return ("struct delta_token");
				 	break;
	    case CMP__E_INT_DEL_TOK_SIGN:  return ("RulToken_Sign");
				 	break;
	    case CMP__E_INT_MOLECULE:	return ("RulMolecule");
					break;
	    case CMP__E_INT_CONSTRUCT_TYPE: return ("RulConstruct_Type");
					break;
	    case CMP__E_INT_BETA_SET:	return ("RulBeta_Collection");
					break;
	    case CMP__E_INT_BETA_TOK:	return ("RulBeta_Token");
					break;
	    case CMP__E_INT_CATCHER_FUNC: return ("RulCatcher_Function");
					break;
	    case CMP__E_INT_MATCHES_FUNC: return ("RulMatches_Function");
					break;
	    case CMP__E_INT_PROP_FUNC:	return ("RulPropagate_Function");
					break;
	    case CMP__E_INT_CLASS:	return ("RulClass");
					break;
	    case CMP__E_INT_ENTRY_DATA:	return ("RulEntry_Data");
					break;
	}
	assert (FALSE);
	return ("");
}




/****************************
**                         **
**  ANIMAL_TYPE_TO_STRING  **
**                         **
****************************/

static char *animal_type_to_string (Ext_Type type)
{
	switch (type) {
	    case  ext_type_long:	return ("long");
					break;
	    case  ext_type_short:	return ("short");
					break;
	    case  ext_type_byte:	return ("char");
					break;
	    case  ext_type_uns_long:	return ("unsigned long");
					break;
	    case  ext_type_uns_short:	return ("unsigned short");
					break;
	    case  ext_type_uns_byte:	return ("unsigned char");
					break;
	    case  ext_type_float:	return ("float");
					break;
	    case  ext_type_double:	return ("double");
					break;
	    case  ext_type_asciz:	return ("char");
					break;
	    case  ext_type_ascid:	return ("RulString");
					break;
	    case  ext_type_void:	return ("void");
					break;
	    case  ext_type_void_ptr:	return ("RulPointer");
					break;
	    case  ext_type_atom:	return ("RulMolecule");
					break;
	    case  ext_type_atom_ptr:	return ("RulMolecule_Ptr");
					break;
	    case  ext_type_object:	return ("RulObject");
	      				break;
	    case  ext_type_for_each:	return ("unsigned long");
					break;
	    case  ext_type_none:	return ("");
					break;
	}
	assert (FALSE);
	return (" ");
}




/************************
**                     **
**  RUL__EMIT_COMMENT  **
**                     **
************************/

void rul__emit_comment (char *arbitrary_str)
{
#ifndef NDEBUG
	rul___emit_verify_newline ();
	rul___emit_string ("/* ");
	rul___emit_string (arbitrary_str);
	rul___emit_string (" */");
	rul___emit_newline ();
#endif
}




/*****************************
**                          **
**  RUL__EMIT_REAL_COMMENT  **
**                          **
*****************************/

void rul__emit_real_comment (char *arbitrary_str)
{
	rul___emit_verify_newline ();
	rul___emit_string ("/* ");
	rul___emit_string (arbitrary_str);
	rul___emit_string (" */");
	rul___emit_newline ();
}



/*********************************
**                              **
**  RUL__EMIT_EMBEDDED_COMMENT  **
**                              **
*********************************/

void rul__emit_embedded_comment (char *arbitrary_str)
{
#ifndef NDEBUG
	rul___emit_string ("/* ");
	rul___emit_string (arbitrary_str);
	rul___emit_string (" */");
#endif
}



/***************************
**                        **
**  RUL__EMIT_BLANK_LINE  **
**                        **
***************************/

void rul__emit_blank_line (void)
{
	rul___emit_verify_newline ();
#ifndef NDEBUG
	rul___emit_newline ();
#endif
}



/***************************
**                        **
**  RUL__EMIT_PAGE_BREAK  **
**                        **
***************************/

void rul__emit_page_break (void)
{
	rul___emit_verify_newline ();
#ifndef NDEBUG
	rul___emit_string ("");
	rul___emit_newline ();
#endif
}




/****************************
**                         **
**  RUL__EMIT_ENTRY_POINT  **
**                         **
****************************/

void rul__emit_entry_point (Mol_Symbol entry_point_name, Name_Scope func_scope,
			    Boolean has_return, Ext_Type return_type,
			    Cardinality array_len, Ext_Mech return_mech)
{
	char name[RUL_C_MAX_SYMBOL_SIZE+1];

	rul__mol_use_printform (entry_point_name, name, RUL_C_MAX_SYMBOL_SIZE);
	rul__emit_entry_point_str (name, func_scope, has_return,
				   return_type, array_len, return_mech);
}



/********************************
**                             **
**  RUL__EMIT_ENTRY_POINT_STR  **
**                             **
********************************/

void rul__emit_entry_point_str (char *entry_point_name, Name_Scope func_scope,
				Boolean has_return, Ext_Type ret_type,
				Cardinality array_len, Ext_Mech ret_mech)
{
	rul___emit_verify_newline ();
	
/*	rul___emit_newline (); */

	if (func_scope == EMIT__E_SCOPE_STATIC)
	  rul___emit_string ("static ");

	if (has_return) {
	  rul___emit_string (animal_type_to_string (ret_type));
	  rul___emit_string (" ");
	}
	else {
	  rul___emit_string (animal_type_to_string (ext_type_void));
	  rul___emit_string (" ");
	}

	if (((ret_mech == ext_mech_ref_rw) || (ret_mech == ext_mech_ref_ro)) &&
	    ((ret_type == ext_type_asciz) || (ret_type == ext_type_ascid)))
	  rul___emit_string ("*");
	if (array_len != EXT__C_NOT_ARRAY)
	  rul___emit_string ("*");
	rul___emit_string (entry_point_name);

	rul___emit_string (" (");
	rul___emit_incr_indent ();
	rul___emit_incr_indent ();
	rul___emit_incr_indent ();
	rul___emit_incr_indent ();

	SB_func_args_in_progress = TRUE;
	SL_func_arg_count = 0;
}





/***********************************
**                                **
**  RUL__EMIT_ENTRY_ARG_EXTERNAL  **
**                                **
***********************************/

void rul__emit_entry_arg_external (Ext_Type arg_type, char *arg_name,
				   Cardinality array_size, Ext_Mech arg_mech)
{
	char buffer[30];

	assert (SB_func_args_in_progress);

	if (SL_func_arg_count != 0) rul___emit_string (", ");
	SL_func_arg_count++;

	if ((arg_type != ext_type_invalid) && (arg_type != ext_type_none)) {
	  rul___emit_string (animal_type_to_string (arg_type));
	  rul___emit_string (" ");
	}
	emit_string_to_buf ("");
	if (((arg_mech == ext_mech_ref_rw) || (arg_mech == ext_mech_ref_ro)) &&
	    ((array_size == EXT__C_NOT_ARRAY) ||
	     (arg_type == ext_type_asciz) || (arg_type == ext_type_ascid)))
	  emit_append_to_buf ("*");
	emit_append_to_buf (arg_name);
	if (array_size == EXT__C_IMPLICIT_ARRAY) {
	    emit_append_to_buf ("[]");
	} else if (array_size != EXT__C_NOT_ARRAY) {
	    sprintf (buffer, "[%ld]", array_size);
	    emit_append_to_buf (buffer);
	}
	emit_buffer ();
}




/***********************************
**                                **
**  RUL__EMIT_ENTRY_ARG_INTERNAL  **
**                                **
***********************************/

void rul__emit_entry_arg_internal (Rul_Internal_Type arg_type,
				   char *arg_name,
				   Cardinality array_size,
				   Ext_Mech arg_mech)
{
	char buffer[30];

	assert (SB_func_args_in_progress);

	if (SL_func_arg_count != 0) rul___emit_string (", ");
	SL_func_arg_count++;

	rul___emit_string (rul_type_to_string (arg_type));
	rul___emit_string (" ");
	emit_string_to_buf ("");
	if (((arg_mech == ext_mech_ref_rw) || (arg_mech == ext_mech_ref_ro)) &&
	    ((array_size == EXT__C_NOT_ARRAY) ||
	     (arg_type == ext_type_asciz) || (arg_type == ext_type_ascid)))
	  emit_append_to_buf ("*");
	emit_append_to_buf (arg_name);
	if (array_size == EXT__C_IMPLICIT_ARRAY) {
	    emit_append_to_buf ("[]");
	} else if (array_size > 0) {
	    sprintf (buffer, "[%ld]", array_size);
	    emit_append_to_buf (buffer);
	}
	emit_buffer ();
}




/********************************
**                             **
**  RUL__EMIT_ENTRY_ARGS_DONE  **
**                             **
********************************/

void rul__emit_entry_args_done (Entry_Type define_or_declare)
{
	assert (SB_func_args_in_progress);

	SB_func_args_in_progress = FALSE;

	if (SL_func_arg_count == 0)
	  rul___emit_string (animal_type_to_string(ext_type_void));

	rul___emit_string (")");
	rul___emit_decr_indent ();
	rul___emit_decr_indent ();
	rul___emit_decr_indent ();
	rul___emit_decr_indent ();
	if (define_or_declare == EMIT__E_ENTRY_DEFINITION) {
	    rul___emit_newline ();
	    rul___emit_string ("{");
	    rul___emit_incr_indent ();
	    rul___emit_newline ();
	    SB_func_defn_in_progress = TRUE;

	} else {
	    rul___emit_eos_and_newline ();
	    SB_func_defn_in_progress = FALSE;
	}
}



/***************************
**                        **
**  RUL__EMIT_ENTRY_DONE  **
**                        **
***************************/

void rul__emit_entry_done (void)
{
	assert (SB_func_defn_in_progress);

	SB_func_defn_in_progress = FALSE;

	rul___emit_verify_newline ();
	rul___emit_decr_indent ();
	rul___emit_string ("}");
	rul___emit_newline ();
	rul___emit_newline ();
	rul___emit_newline ();
}




/********************************
**                             **
**  RUL__EMIT_STACK_TMP_START  **
**                             **
********************************/

void rul__emit_stack_tmp_start (Ext_Type type)
{
    rul___emit_verify_newline ();

    if (SB_func_defn_in_progress != TRUE) 
	{
        rul___emit_string ("static  ");
        }
    /* else  add to local scope known var list */

    rul___emit_string (animal_type_to_string (type));
    rul___emit_string (" ");
    rul___emit_incr_indent ();
    rul___emit_incr_indent ();
}



/**************************
**                       **
**  RUL__EMIT_STACK_TMP  **
**                       **
**************************/

Boolean rul__emit_stack_tmp (Ext_Type type,
			     Cardinality a_len,
			     Ext_Mech mech,
			     long index,
			     Boolean first_done)
{
    char buffer[20];
    int i;

    if (first_done)
      rul___emit_string (", ");

    emit_string_to_buf ("");

    if (((mech == ext_mech_ref_rw) || (mech == ext_mech_ref_ro)) &&
	((type == ext_type_asciz) || (type == ext_type_ascid)))
      emit_append_to_buf ("*");

    if (a_len == EXT__C_IMPLICIT_ARRAY)
      emit_append_to_buf ("*");

    sprintf (buffer, "%s", rul___emit_get_ext_type_abbr (type, a_len));
    emit_append_to_buf (buffer);

    if (a_len > 0)
      sprintf (buffer, "%ld[%ld]", index, a_len);
    else
      sprintf (buffer, "%ld", index);

    emit_append_to_buf (buffer);
    emit_buffer ();

    return TRUE;
}



/******************************
**                           **
**  RUL__EMIT_STACK_TMP_END  **
**                           **
******************************/

void rul__emit_stack_tmp_end (void)
{
    rul___emit_decr_indent ();
    rul___emit_decr_indent ();
    rul___emit_eos_and_newline ();
}





/*******************************
**                            **
**  RUL__EMIT_STACK_MOL_INIT  **
**                            **
*******************************/

void rul__emit_stack_mol_init (long index)
{
    char buffer[65];

    sprintf (buffer, "%s%ld = NULL",
	     rul___emit_get_ext_type_abbr (ext_type_atom, EXT__C_NOT_ARRAY),
	     index);
    rul___emit_string (buffer);
    rul___emit_eos_and_newline ();
}



/*******************************
**                            **
**  RUL__EMIT_STACK_EXTERNAL  **
**                            **
*******************************/

void rul__emit_stack_external (
		Ext_Type var_type, char *var_name, Cardinality array_size)
{
	char buffer[30];

	rul___emit_verify_newline ();
	if (SB_func_defn_in_progress != TRUE) {
	    rul___emit_string ("static  ");
	} else {
	    /*?*  add to local scope known var list */
	}
	assert ((var_type != ext_type_asciz) && (var_type != ext_type_ascid));
	rul___emit_string (animal_type_to_string (var_type));
	rul___emit_string (" ");
	emit_string_to_buf (var_name);
	if (array_size == EXT__C_IMPLICIT_ARRAY) {
	    emit_append_to_buf ("[]");
	} else if (array_size > 0) {
	    sprintf (buffer, "[%ld]", array_size);
	    emit_append_to_buf (buffer);
	}
	emit_buffer ();
	rul___emit_eos_and_newline ();
}

/********************************
**                             **
**  RUL__EMIT_GLOBAL_EXTERNAL  **
**                             **
********************************/

void rul__emit_global_external (Ext_Type var_type, char *var_name,
				Cardinality array_size, Boolean referencing)
{
	char buffer[30];

	assert (SB_func_defn_in_progress != TRUE);

	rul___emit_verify_newline ();
	if (referencing)
	    rul___emit_string ("extern ");
	assert ((var_type != ext_type_asciz) && (var_type != ext_type_ascid));
	rul___emit_string (animal_type_to_string (var_type));
	rul___emit_string (" ");
	emit_string_to_buf (var_name);
	if (array_size == EXT__C_IMPLICIT_ARRAY) {
	    emit_append_to_buf ("[]");
	} else if (array_size > 0) {
	    sprintf (buffer, "[%ld]", array_size);
	    emit_append_to_buf (buffer);
	}
	emit_buffer ();
	rul___emit_eos_and_newline ();
}

/* 
** This function supports the RUL__EMIT_GLOBAL_EXTERNAL function.
** It appears that extern symbols are sometimes optimized away at C compile
** time.  Setting up a 'dummy' pointer to these symbols will prevent this
** from happening.
**
** RUL__EMIT_GLOBAL_EXTERNAL writes the line :
**
**	<storage class> <data type> <variable name>;
**
** RUL__EMIT_GLOBAL_EXTERNAL_PTR adds the line :
**
**	static <data type> *<variable name>_ptr = &<variable name>;
**
** The second line prevents the first from being optimized away.
*/
void rul__emit_global_external_ptr (Ext_Type var_type, char *var_name)
{
	char buffer[30];

	assert (SB_func_defn_in_progress != TRUE);

	rul___emit_verify_newline ();

	rul___emit_string ("static ");
	assert ((var_type != ext_type_asciz) && (var_type != ext_type_ascid));
	rul___emit_string (animal_type_to_string (var_type));
	rul___emit_string (" *");
	emit_string_to_buf (var_name);
	emit_append_to_buf ("_ptr = &");
	emit_append_to_buf (var_name);
	emit_buffer ();
	rul___emit_eos_and_newline ();
}


/*******************************
**                            **
**  RUL__EMIT_STACK_INTERNAL  **
**                            **
*******************************/

void rul__emit_stack_internal (Rul_Internal_Type var_type,
			       char *var_name,
			       Cardinality array_size)
{
    emit_stack_internal(var_type, ext_mech_value, var_name, array_size);
}

/***********************************
**                                **
**  RUL__EMIT_STACK_INTERNAL_PTR  **
**                                **
***********************************/

void rul__emit_stack_internal_ptr (Rul_Internal_Type var_type,
				   char *var_name,
				   Cardinality array_size)
{
    emit_stack_internal(var_type, ext_mech_ref_rw, var_name, array_size);
}

/**************************
**	                 **
**  EMIT_STACK_INTERNAL  **
**	                 **
**************************/

static void emit_stack_internal (Rul_Internal_Type var_type,
				 Ext_Mech var_mech,
				 char *var_name,
				 Cardinality array_size)
{
	char buffer[30];

	rul___emit_verify_newline ();
	if (SB_func_defn_in_progress != TRUE) {
	    rul___emit_string ("static  ");
	} else {
	    /*?*  add to local scope known var list */
	}
	rul___emit_string (rul_type_to_string (var_type));
	rul___emit_string (" ");
	emit_string_to_buf ("");
	if (var_mech == ext_mech_ref_rw)
	    emit_append_to_buf ("*");
	emit_append_to_buf (var_name);
	if (array_size == EXT__C_IMPLICIT_ARRAY) {
	    emit_append_to_buf ("[]");
	} else if (array_size > 0) {
	    sprintf (buffer, "[%ld]", array_size);
	    emit_append_to_buf (buffer);
	}
	emit_buffer ();
	rul___emit_eos_and_newline ();
}




/**********************************
**                               **
**  RUL__EMIT_BEGIN_CONDITIONAL  **
**                               **
**********************************/

void rul__emit_begin_conditional (
		Relation_Operator oper, Value operand_1, Value operand_2)
{
	long i, count;

	rul___emit_verify_newline ();
	rul___emit_string ("if (");
	rul___emit_incr_indent ();
	rul___emit_incr_indent ();

	if (oper == EMIT__E_REL_NONE) {
	    /*
	    **	Simple predicate function case
	    */
	    rul__emit_value_reference (operand_1, TRUE);

	} else {
	    assert (operand_2 != NULL);
	    if (rul__val_is_disjunction (operand_2)) {
		/*
		**  For value disjunction cases, we split it out here
		**
		**  Note:  We could also catch it here for other
		**         predicates, by checking for :
		**             op1 and !op2 and is_disj(get_nth(op1,2))
		*/
		count = rul__val_get_disjunct_count (operand_2);
		for (i=1; i<=count; i++) {
		    if (i > 1) rul___emit_string ("  ||  ");
		    rul___emit_string ("(");
	    	    rul__emit_value_reference (operand_1, TRUE);
	            rul___emit_string (relation_to_string (oper));
	            rul__emit_value_reference (
			    rul__val_get_nth_disjunct (operand_2, i), TRUE);
		    rul___emit_string (")");
		}

	    } else {
		/*
		**  A real (and normal) relational test
		*/
		rul__emit_value_reference (operand_1, TRUE);
	        rul___emit_string (relation_to_string (oper));
	        rul__emit_value_reference (operand_2, TRUE);
	    }
	}
	rul___emit_decr_indent ();
	rul___emit_decr_indent ();
	rul___emit_string (") {");
	rul___emit_incr_indent ();
	rul___emit_newline ();
}




/**********************************
**                               **
**  RUL__EMIT_ELSE_IF_CONDITION  **
**                               **
**********************************/

void rul__emit_else_if_condition (
		Relation_Operator oper, Value operand_1, Value operand_2)
{
	rul___emit_verify_newline ();
	rul___emit_decr_indent ();
	rul___emit_string ("} else if (");
	rul___emit_incr_indent ();
	rul___emit_incr_indent ();

	if (oper == EMIT__E_REL_NONE) {
	    /*
	    **	Simple predicate function case
	    */
	    rul__emit_value_reference (operand_1, TRUE);

	} else {
	    assert (operand_2 != NULL);
	    /*  A normal relational test  */
	    rul__emit_value_reference (operand_1, TRUE);
	    rul___emit_string (relation_to_string (oper));
	    rul__emit_value_reference (operand_2, TRUE);
	}
	rul___emit_decr_indent ();
	rul___emit_decr_indent ();
	rul___emit_string (") {");
	rul___emit_incr_indent ();
	rul___emit_newline ();
}




/*******************************
**                            **
**  RUL__EMIT_ELSE_CONDITION  **
**                            **
*******************************/

void rul__emit_else_condition (void)
{
	rul___emit_verify_newline ();
	rul___emit_decr_indent ();
	rul___emit_string ("} else {");
	rul___emit_incr_indent ();
	rul___emit_newline ();
}



/********************************
**                             **
**  RUL__EMIT_END_CONDITIONAL  **
**                             **
********************************/

void rul__emit_end_conditional (void)
{
	rul___emit_verify_newline ();
	rul___emit_decr_indent ();
	rul___emit_string ("}");
	rul___emit_newline ();
}

static Boolean SB_in_cplx_conditional = FALSE;
static long    SL_num_cplx_conditions = 0;

/*********************************
**                              **
**  RUL__EMIT_BEGIN_CPLX_WHILE  **
**                              **
*********************************/

void rul__emit_begin_cplx_while (void)
{
	rul___emit_verify_newline ();
	rul___emit_string ("while (");
	rul___emit_incr_indent ();
	rul___emit_incr_indent ();

	SB_in_cplx_conditional = TRUE;
	SL_num_cplx_conditions = 0;
}


/*********************************
**                              **
**  RUL__EMIT_CPLX_CONDITIONAL  **
**                              **
*********************************/

void rul__emit_cplx_conditional (void)
{
	rul___emit_verify_newline ();
	rul___emit_string ("if (");
	rul___emit_incr_indent ();
	rul___emit_incr_indent ();

	SB_in_cplx_conditional = TRUE;
	SL_num_cplx_conditions = 0;
}




/**************************************
**                                   **
**  RUL__EMIT_CLOSE_CPLX_CONDITIONS  **
**                                   **
**************************************/

void rul__emit_close_cplx_conditions (void)
{
	assert (SB_in_cplx_conditional);

	rul___emit_decr_indent ();
	rul___emit_decr_indent ();
	rul___emit_string (") {");
	rul___emit_incr_indent ();
	rul___emit_newline ();

	SB_in_cplx_conditional = FALSE;
	SL_num_cplx_conditions = 0;
}




/*******************************
**                            **
**  RUL__EMIT_CPLX_CONDITION  **
**                            **
*******************************/

void rul__emit_cplx_condition (
		Relation_Operator oper, Value operand_1, Value operand_2)
{
	long i, count;

	assert (SB_in_cplx_conditional);
	if (SL_num_cplx_conditions > 0) rul___emit_string ("  &&  ");
	SL_num_cplx_conditions++;

	if (oper == EMIT__E_REL_NONE) {
	    /*
	    **	Simple predicate function case
	    */
	    rul__emit_value_reference (operand_1, TRUE);

	} else {
	    rul___emit_string ("(");

	    assert (operand_2 != NULL);
	    if (rul__val_is_disjunction (operand_2)) {
		/*
		**  For value disjunction cases, we split it out here
		**
		**  Note:  We could also catch it here for other
		**         predicates, by checking for :
		**             op1 and !op2 and is_disj(get_nth(op1,2))
		*/
		count = rul__val_get_disjunct_count (operand_2);
		for (i=1; i<=count; i++) {
		    if (i > 1) rul___emit_string ("  ||  ");
		    rul___emit_string ("(");
	    	    rul__emit_value_reference (operand_1, TRUE);
	            rul___emit_string (relation_to_string (oper));
	            rul__emit_value_reference (
			    rul__val_get_nth_disjunct (operand_2, i), TRUE);
		    rul___emit_string (")");
		}

	    } else {
		/*
		**  A real (and normal) relational test
		*/
		rul__emit_value_reference (operand_1, TRUE);
	        rul___emit_string (relation_to_string (oper));
	        rul__emit_value_reference (operand_2, TRUE);
	    }
	    rul___emit_string (")");
	}
}




/********************************
**                             **
**  RUL__EMIT_BEGIN_ITER_LOOP  **
**                             **
********************************/

void rul__emit_begin_iter_loop (char *iterator_name, long number_loops)
{
	char buffer[20];

	rul___emit_verify_newline ();
	rul___emit_string ("for (");
	rul___emit_string (iterator_name);
	rul___emit_string (" = 0; ");
	rul___emit_string (iterator_name);
	sprintf (buffer, " < %ld; ", number_loops);
	rul___emit_string (buffer);
	rul___emit_string (iterator_name);
	rul___emit_string ("++)");
	rul___emit_incr_indent ();
	rul___emit_string (" {");
	rul___emit_newline ();
}




/***************************
**                        **
**  RUL__EMIT_BEGIN_LOOP  **
**                        **
***************************/

void rul__emit_begin_loop (
		Relation_Operator oper, Value operand_1, Value operand_2)
{
	rul___emit_verify_newline ();
	rul___emit_string ("while (");
	rul___emit_incr_indent ();
	rul___emit_incr_indent ();
	rul__emit_value_reference (operand_1, TRUE);
	if (oper != EMIT__E_REL_NONE  &&  operand_2 != NULL) {
	    rul___emit_string (relation_to_string (oper));
	    rul__emit_value_reference (operand_2, TRUE);
	}
	rul___emit_decr_indent ();
	rul___emit_decr_indent ();
	rul___emit_string (") {");
	rul___emit_incr_indent ();
	rul___emit_newline ();
}



/*************************
**                      **
**  RUL__EMIT_END_LOOP  **
**                      **
*************************/

void rul__emit_end_loop (void)
{
	rul___emit_verify_newline ();
	rul___emit_decr_indent ();
	rul___emit_string ("}");
	rul___emit_newline ();
}




/********************************
**                             **
**  RUL__EMIT_VALUE_STATEMENT  **
**                             **
********************************/

void rul__emit_value_statement (Value value)
{
	assert (rul__val_is_function(value) || rul__val_is_assignment(value));

	rul___emit_string ("");		/* to force the line start */
	rul___emit_incr_indent ();	/* force any wrapped lines to indent */
	rul___emit_incr_indent ();

	rul__emit_value_reference (value, FALSE);

	rul___emit_eos_and_newline ();
	rul___emit_decr_indent ();
	rul___emit_decr_indent ();
}




/***********************
**                    **
**  RUL__EMIT_RETURN  **
**                    **
***********************/

void rul__emit_return (void)
{
	rul___emit_verify_newline ();
	rul___emit_string ("return");
	rul___emit_eos_and_newline ();
}



/*****************************
**                          **
**  RUL__EMIT_RETURN_VALUE  **
**                          **
*****************************/

void rul__emit_return_value (Value val)
{
	/*  Note there better not be any value_cleanup required  */

	rul___emit_verify_newline ();
	rul__emit_value_setup (val);
	rul___emit_string ("return ");
	rul__emit_value_reference (val, TRUE);
	rul___emit_eos_and_newline ();
}



/***************************************
**                                    **
**  RUL__EMIT_INCLUDE_HEADER_FILE     **
**                                    **
***************************************/

void rul__emit_include_header_file (Boolean library_header, char *header_file)
{
	rul___emit_verify_newline ();
	rul___emit_string ("#include ");
	if (library_header) {
	  rul___emit_string ("<");
	  rul___emit_string (header_file);
	  rul___emit_string (">");
	}
	else {
	  rul___emit_string ("\"");
	  rul___emit_string (header_file);
	  rul___emit_string ("\"");
	}
}




/***************************************
**                                    **
**  RUL__EMIT_CONSTANT_DEFINITION     **
**                                    **
***************************************/

void rul__emit_constant_definition (char *constant_name, char *constant_value)
{
	rul___emit_verify_newline ();
	rul___emit_string ("#define ");
	rul___emit_string (constant_name);
	rul___emit_string (" ");
	rul___emit_string (constant_value);
}




/***********************
**                    **
**  RUL__EMIT_TAB     **
**                    **
***********************/

void rul__emit_tab (unsigned int number_of_tabs)
{
  unsigned int i;
  assert (number_of_tabs >= 1);

  for (i = 0; i < number_of_tabs; i++)
    rul___emit_string ("\t");
}


/*************************
**                      **
**  RUL__EMIT_PROLOGUE  **
**                      **
*************************/

void rul__emit_prologue (void)
{
  char buffer[512];

  sprintf (buffer, "Generated by:  %s", CMP_VERSION);
  rul__emit_real_comment (buffer);

  rul__emit_constant_definition ("RUL__C_IN_GENERATED_CODE","1");
  rul__emit_include_header_file (TRUE, "rul_gend.h");
}

