/****************************************************************************
**                                                                         **
**                  C M P _ E M I T _ C _ V A L . C                        **
**                                                                         **
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
**
****************************************************************************/


/*
 * FACILITY:
 *	RULEWORKS compiler
 *
 * ABSTRACT:
 *	This module contains the C code generator implementation
 *	of the low-level generic code emitter.
 *
 *	Specifically, this file contains the subset of the gen
 *	routines that need private knowledge of the VAL subsystem.
 *
 * MODIFIED BY:
 *	DEC	Digital Computer Corporation
 *	CPQ	Compaq Computer Corporation
 *
 * MODIFICATION HISTORY:
 *
 *	28-Jan-1993	DEC	Initial version
 *	01-Dec-1999	CPQ	Release with GPL
 *
 *	22-Aug-1994	LE	Quote `\' and `?' in addition to `"' when
 *				putting animal strings.
 */

#include <common.h>
#include <cmp_comm.h>
#include <cons.h>
#include <conrg.h>
#include <emit.h>
#include <emit_c.h>
#include <gen.h>
#include <gennet_p.h>	/* for the name of the DEL_TOK_ARG */
#include <mol.h>
#include <val.h>
#include <val_p.h>

static void put_animal_const_in_string (char buffer[], Value value);
static void put_unm_mol_var_in_string (char buffer[], Value value);
static void put_unm_ext_var_in_string (char buffer[], Value value,
				       Boolean value_access);
static void put_nam_ext_var_in_string (char buffer[], Value value,
				       Boolean value_access);
static void put_rhs_variable_in_string (char buffer[], Value value);
static void put_local_ref_in_string (char buffer[], Rul_Internal_Field field);
static char *cond_oper_to_string (Cond_Oper_Type oper);
static char *arith_oper_to_string (Arith_Oper_Type oper);
static char *arith_oper_to_function (Arith_Oper_Type oper);
static Boolean all_arith_elems_are_numbers (Arith_Expr_Value arith_value);
static Boolean value_is_a_number (Value value);
static void emit_value_setups (Value value);
static void emit_value_partials (Value value);




/**********************
**                   **
**  RUL__EMIT_VALUE  **
**                   **
**********************/

void rul__emit_value (Value value)
{
	if (value == NULL) return;

	rul__emit_value_setup (value);
	if (value->type != VAL__E_ASSIGNMENT)
	    /* where the value statement was made during setup... */
	    rul__emit_value_statement (value);
	rul__emit_value_cleanup (value);
}





/****************************
**                         **
**  RUL__EMIT_VALUE_SETUP  **
**                         **
****************************/

void rul__emit_value_setup (Value value)
{
        long i;
	Assignment_Value asn_val;
	Function_Value f_val;
	Arith_Expr_Value ar_val;
	Cond_Expr_Value cd_val;
	Disjunction_Value d_val;

        switch (value->type) {

            case VAL__E_POS :
	    case VAL__E_LHS_VARIABLE :
	    case VAL__E_BLK_OR_LOCAL :
            case VAL__E_RUL_CONST :
            case VAL__E_ANIMAL_CONST :
            case VAL__E_CLASS_CONST :
            case VAL__E_CONSTRUCT :
            case VAL__E_CONSTRUCT_TYPE :
	    case VAL__E_UNM_EXT_VAR :
	    case VAL__E_COND_EXPR :
		/* do nothing in this case... */
                break;

	    case VAL__E_ASSIGNMENT :
		asn_val = (Assignment_Value) value;
		emit_value_partials (asn_val->from_value);
		emit_value_setups (value);
		rul__emit_value_statement (value);
		break;

            case VAL__E_FUNCTION :
		f_val = (Function_Value) value;
		for (i=0; i<f_val->subvalue_count; i++) {
		    emit_value_partials (f_val->subvalues[i]);
		}
		emit_value_setups (value);
		break;

	    case VAL__E_DISJUNCTION :
		d_val = (Disjunction_Value) value;
		for (i=0; i<d_val->subvalue_count; i++) {
		    emit_value_partials (d_val->subvalues[i]);
		}
		break;

	    case VAL__E_ARITH_EXPR :
		ar_val = (Arith_Expr_Value) value;
		if (!all_arith_elems_are_numbers (ar_val)) {
		    if (ar_val->arith_operator == CMP__E_ARITH_UNARY_MINUS) {
			emit_value_partials (ar_val->operand_1);
		    } else {
			emit_value_partials (ar_val->operand_1);
			emit_value_partials (ar_val->operand_2);
		    }
		}
		break;
  
            default :
		rul___emit_string (" -? value setup ?- ");
		break;
        }
}



/**************************
**                       **
**  EMIT_VALUE_PARTIALS  **
**                       **
**************************/

static void emit_value_partials (Value value)
{
        long i;
	Assignment_Value asn_val;
	Function_Value f_val;
	Arith_Expr_Value ar_val;
	Cond_Expr_Value cd_val;

        switch (value->type) {

            case VAL__E_POS :
	    case VAL__E_BLK_OR_LOCAL :
	    case VAL__E_LHS_VARIABLE :
            case VAL__E_RUL_CONST :
            case VAL__E_ANIMAL_CONST :
	    case VAL__E_UNM_MOL_VAR :
	    case VAL__E_UNM_EXT_VAR :
	    case VAL__E_NAM_EXT_VAR :
	    case VAL__E_RHS_VARIABLE :
            case VAL__E_CLASS_CONST :
            case VAL__E_CONSTRUCT :
            case VAL__E_CONSTRUCT_TYPE :
	    case VAL__E_COND_EXPR :
		/* do nothing in this case... */
                break;

	    case VAL__E_DISJUNCTION :
		assert (FALSE /* We should never be here */);
                break;

	    case VAL__E_ASSIGNMENT :
		emit_value_setups (value);
		asn_val = (Assignment_Value) value;
		emit_value_partials (asn_val->from_value);
		rul__emit_value_statement (value);
		rul__emit_value_cleanup (value);
		break;

            case VAL__E_FUNCTION :
		emit_value_setups (value);
		f_val = (Function_Value) value;
		for (i=0; i<f_val->subvalue_count; i++) {
		    emit_value_partials (f_val->subvalues[i]);
		}
		rul__emit_value_cleanup (value);
		break;

	    case VAL__E_ARITH_EXPR :
		ar_val = (Arith_Expr_Value) value;
		if (ar_val->arith_operator == CMP__E_ARITH_UNARY_MINUS) {
		    emit_value_partials (ar_val->operand_1);
		} else {
		    emit_value_partials (ar_val->operand_1);
		    emit_value_partials (ar_val->operand_2);
		}
		break;
  
            default :
		rul___emit_string (" -? value partials ?- ");
		break;
        }
}



/********************************
**                             **
**  RUL__EMIT_VALUE_REFERENCE  **
**                             **
********************************/

void rul__emit_value_reference (Value value, Boolean value_access)
{
        long index, bl_num, i;
        char buffer[3*RUL_C_MAX_SYMBOL_SIZE+40];
	Assignment_Value asn_val;
	Function_Value f_val;
	Blk_Or_Local_Value l_val;
	Arith_Expr_Value ar_val;
	Cond_Expr_Value cd_val;
	Value index_val;
	Mol_Symbol mol;
	Boolean elem_index_is_constant;


        switch (value->type) {


            case VAL__E_POS_ELEM :
		index_val = ((Position_Elem_Value)value)->index;
		if (rul__val_is_animal_const (index_val)) {
#ifndef NDEBUG
		    rul___emit_string ("rul__mol_get_comp_nth (");
#else
		    rul___emit_string ("MGCN (");
#endif
		} else {
#ifndef NDEBUG
		    rul___emit_string ("rul__mol_get_comp_nth_mol (");
#else
		    rul___emit_string ("MGCNM (");
#endif
		}
#ifndef NDEBUG
		rul___emit_string ("rul__wm_get_offset_val (");
#else
		rul___emit_string ("WGOV (");
#endif
		sprintf (buffer, "%s->instance,", DEL_TOK_ARG);
		rul___emit_string (buffer);
		sprintf (buffer, " %ld), ", 
			 ((Position_Value)value)->attr_offset);
		rul___emit_string (buffer);
		rul__emit_value_reference (index_val, TRUE);
		rul___emit_string (")");
		break;


            case VAL__E_POS :
#ifndef NDEBUG
		rul___emit_string ("rul__wm_get_offset_val (");
#else
		rul___emit_string ("WGOV (");
#endif
		sprintf (buffer, "%s->instance,", DEL_TOK_ARG);
		rul___emit_string (buffer);
		sprintf (buffer, " %ld)", 
			 ((Position_Value)value)->attr_offset);
		rul___emit_string (buffer);
                break;

            case VAL__E_RUL_CONST :
	        bl_num = rul__conrg_get_cur_block_index ();
                index = rul__cons_get_index (
				((Rul_Constant_Value)value)->constant_value);
                sprintf (buffer, "%s%ld_m", GEN_NAME_PREFIX, bl_num);
		rul___emit_string (buffer);
		sprintf (buffer, "[%ld]", index-1);
		rul___emit_string (buffer);
                break;

	    case VAL__E_UNM_MOL_VAR :
		put_unm_mol_var_in_string (buffer, value);
		rul___emit_string (buffer);
		break;

	    case VAL__E_UNM_EXT_VAR :
		put_unm_ext_var_in_string (buffer, value, value_access);
		rul___emit_string (buffer);
		break;

	    case VAL__E_NAM_EXT_VAR :
	        put_nam_ext_var_in_string (buffer, value, value_access);
		rul___emit_string(buffer);
		break;

	    case VAL__E_RHS_VARIABLE :
		put_rhs_variable_in_string (buffer, value);
		rul___emit_string (buffer);
		break;

           case VAL__E_LHS_VARIABLE :
                rul__gen_lhs_var_reference (value);
                break;

            case VAL__E_ANIMAL_CONST :
                put_animal_const_in_string (buffer, value);
		rul___emit_string (buffer);
                break;

            case VAL__E_CLASS_CONST :
	        bl_num = rul__conrg_get_cur_block_index ();
                index = rul__class_get_index (
				((Class_Constant_Value)value)->class_value);
                sprintf (buffer, "%s%ld_cl", GEN_NAME_PREFIX, bl_num);
		rul___emit_string (buffer);
                sprintf (buffer, "[%ld]", index-1);
		rul___emit_string (buffer);
                break;

            case VAL__E_CONSTRUCT :
	        bl_num = rul__conrg_get_cur_block_index ();
                index = rul__conrg_get_construct_index (
				((Construct_Value)value)->construct_value);
                sprintf (buffer, "%s%ld_cons", GEN_NAME_PREFIX, bl_num);
		rul___emit_string (buffer);
                sprintf (buffer, "[%ld]", index-1);
		rul___emit_string (buffer);
                break;

            case VAL__E_CONSTRUCT_TYPE :
	        bl_num = rul__conrg_get_cur_block_index ();
                index = rul__conrg_get_construct_index (
			((Construct_Type_Value)value)->construct_value);
                sprintf (buffer, "%s%ld_cons_type", GEN_NAME_PREFIX, bl_num);
		rul___emit_string (buffer);
                sprintf (buffer, "[%ld]", index-1);
		rul___emit_string (buffer);
                break;

	    case VAL__E_BLK_OR_LOCAL :
		l_val = (Blk_Or_Local_Value) value;
		mol = l_val->arg_name;
		if (l_val->arg_indirect) rul___emit_string ("&");
		rul__mol_use_printform (mol, buffer, RUL_C_MAX_SYMBOL_SIZE+1);
		rul___emit_string (buffer);
		if (l_val->arg_field != CMP__E_FIELD__NONE) {
		    put_local_ref_in_string (buffer, l_val->arg_field);
		    rul___emit_string (buffer);
		}
		if (l_val->arg_index != VAL__C_NOT_AN_INDEX) {
		    sprintf (buffer, "[%ld]", l_val->arg_index);
		    rul___emit_string (buffer);
		}
		break;

	    case VAL__E_ASSIGNMENT :
		asn_val = (Assignment_Value) value;
		rul__emit_value_reference (asn_val->to_variable, value_access);
		if (!value_access)
		    {
		    rul___emit_string (" = ");
		    rul__emit_value_reference (asn_val->from_value, TRUE);
		    }
		break;

            case VAL__E_FUNCTION :
		f_val = (Function_Value) value;
		if (f_val->passing_mech != ext_mech_value)
		    rul___emit_string ("*");
		mol = f_val->function_name;
		rul__mol_use_printform (mol, buffer, RUL_C_MAX_SYMBOL_SIZE+1);
		rul___emit_string (buffer);
		rul___emit_string (" (");
		for (i=0; i<f_val->subvalue_count; i++) {
		    if (i > 0) rul___emit_string (", ");
		    rul__emit_value_reference (f_val->subvalues[i], TRUE);
		}
		rul___emit_string (")");
		break;

	    case VAL__E_ARITH_EXPR :
		ar_val = (Arith_Expr_Value) value;
		if (ar_val->arith_operator == CMP__E_ARITH_UNARY_MINUS) {
		    rul___emit_string (
			arith_oper_to_function (CMP__E_ARITH_UNARY_MINUS));
		    rul___emit_string (" (");
		    rul__emit_value_reference (ar_val->operand_1, TRUE);
		    rul___emit_string (")");
		} else if (all_arith_elems_are_numbers (ar_val)) {
		    rul___emit_string ("(");
		    rul__emit_value_reference (ar_val->operand_1, TRUE);
		    rul___emit_string (
			arith_oper_to_string (ar_val->arith_operator));
		    rul__emit_value_reference (ar_val->operand_2, TRUE);
		    rul___emit_string (")");
		} else {
		    rul___emit_string (
			arith_oper_to_function (ar_val->arith_operator));
		    rul___emit_string (" (");
		    rul__emit_value_reference (ar_val->operand_1, TRUE);
		    rul___emit_string (", ");
		    rul__emit_value_reference (ar_val->operand_2, TRUE);
		    rul___emit_string (")");
		}
		break;

	    case VAL__E_COND_EXPR :
		cd_val = (Cond_Expr_Value) value;
		if (cd_val->cond_operator == CMP__E_COND_NOT) {
		    rul___emit_string (
			cond_oper_to_string (CMP__E_COND_NOT));
		    rul___emit_string ("(");
		    rul__emit_value_reference (cd_val->operand_1, TRUE);
		    rul___emit_string (")");
		} else {
		    rul___emit_string ("(");
		    rul__emit_value_reference (cd_val->operand_1, TRUE);
		    rul___emit_string (
			cond_oper_to_string (cd_val->cond_operator));
		    rul__emit_value_reference (cd_val->operand_2, TRUE);
		    rul___emit_string (")");
		}
		break;

	    case VAL__E_DISJUNCTION :
		/*
		**  Value disjunctions should never get to this
		**  level.  They should always be dealt with as part
		**  of the ..._emit_conditional.
		*/
		assert (FALSE); 
  
            default :
		rul___emit_string (" -?- ");
		break;
        }
}



/*************************
**                      **
**  EMIT_VALUE_SETUPS   **
**                      **
*************************/

static void emit_value_setups (Value value)
{
	Assignment_Value as_val;
	Function_Value	 fn_val;

	if (value == NULL) return; /* done with cleanup forms */

        switch (value->type) {

            case VAL__E_POS :
	    case VAL__E_BLK_OR_LOCAL :
            case VAL__E_RUL_CONST :
            case VAL__E_ANIMAL_CONST :
            case VAL__E_CLASS_CONST :
            case VAL__E_CONSTRUCT :
            case VAL__E_CONSTRUCT_TYPE :
	    case VAL__E_UNM_MOL_VAR :
	    case VAL__E_UNM_EXT_VAR :
	    case VAL__E_RHS_VARIABLE :
	    case VAL__E_ARITH_EXPR :
	    case VAL__E_COND_EXPR :
		/*  do nothing in these cases  */
		break;

	    case VAL__E_ASSIGNMENT :
		as_val = (Assignment_Value) value;
		rul__emit_value (as_val->prev_value);
		break;

            case VAL__E_FUNCTION :
		fn_val = (Function_Value) value;
		rul__emit_value (fn_val->prev_value);
		break;

            case VAL__E_DISJUNCTION :
		assert (FALSE /* Is this an error? */);
		break;
 
            default :
		rul___emit_string (" -? previous ?- ");
		break;
        }
}


/******************************
**                           **
**  RUL__EMIT_VALUE_CLEANUP  **
**                           **
******************************/

void rul__emit_value_cleanup (Value value)
{
	Complex_Value	 cmplx_val;
	Assignment_Value as_val;
	Function_Value	 fn_val;
	long		 i;

	if (value == NULL) return; /* done with cleanup forms */

        switch (value->type) {

            case VAL__E_POS :
	    case VAL__E_LHS_VARIABLE :
	    case VAL__E_BLK_OR_LOCAL :
            case VAL__E_RUL_CONST :
            case VAL__E_ANIMAL_CONST :
            case VAL__E_CLASS_CONST :
            case VAL__E_CONSTRUCT :
            case VAL__E_CONSTRUCT_TYPE :
		/*  do nothing in these cases  */
		break;

	    case VAL__E_UNM_MOL_VAR :
	    case VAL__E_UNM_EXT_VAR :
	    case VAL__E_RHS_VARIABLE :
	    case VAL__E_ARITH_EXPR :
	    case VAL__E_COND_EXPR :
		assert (FALSE /* We should never be here */);
                break;

	    case VAL__E_ASSIGNMENT :
		as_val = (Assignment_Value) value;
		rul__emit_value (as_val->next_value);
		break;

            case VAL__E_FUNCTION :
		fn_val = (Function_Value) value;
		rul__emit_value (fn_val->next_value);
		break;

            case VAL__E_DISJUNCTION :
		cmplx_val = (Complex_Value) value;
		for (i=0; i<cmplx_val->subvalue_count; i++) {
		    rul__emit_value_cleanup (cmplx_val->subvalues[i]);
		}
		break;
 
            default :
		rul___emit_string (" -? cleanup ?- ");
		break;
        }
}




/***********************************
**                                **
**  RUL___EMIT_GET_EXT_TYPE_ABBR  **
**                                **
***********************************/

char *rul___emit_get_ext_type_abbr (Ext_Type ext_type, Cardinality a_len)
{
  char *str;
  static char buf[RUL_C_MAX_SYMBOL_SIZE];

  switch (ext_type) {
  case ext_type_long:
    str = "tl";
    break;
  case ext_type_short:
    str = "ts";
    break;
  case ext_type_byte:
    str = "tb";
    break;
  case ext_type_uns_long:
    str = "tul";
    break;
  case ext_type_uns_short:
    str = "tus";
    break;
  case ext_type_uns_byte:
    str = "tub";
    break;
  case ext_type_float:
    str = "tf";
    break;
  case ext_type_double:
    str = "td";
    break;
  case ext_type_asciz:
    str = "taz";
    break;
  case ext_type_ascid:
    str = "tad";
    break;
  case ext_type_void:
    str = "tv";
    break;
  case ext_type_void_ptr:
    str = "tvp";
    break;
  case ext_type_atom:
    str = "tm";
    break;
  case ext_type_atom_ptr:
    str = "tap";
    break;
  case ext_type_object:
    str = "to";
    break;
  case ext_type_for_each:
    str = "tfe";
    break;
  default:
    assert (FALSE /* Bogus external type */);
    break;
  }
  if (a_len != EXT__C_NOT_ARRAY) {
    strcpy (buf, str);
    strcat (buf, "a");
    str = buf;
  }
  return str;
}




/********************************
**                             **
**  PUT_UNM_EXT_VAR_IN_STRING  **
**                             **
********************************/

static void put_unm_ext_var_in_string (char buffer[], Value value,
				       Boolean value_access)
{
    Unnamed_Ext_Variable_Value v_val;
    char       *mech_str;
    char        loc_buf_arr[20];
    char        loc_buf_num[20];

    v_val = (Unnamed_Ext_Variable_Value) value;
    buffer[0] = '\0';

    if ((value_access) &&
	(v_val->passing_mech != ext_mech_value) &&
	(v_val->array_length == EXT__C_NOT_ARRAY) &&
	(v_val->cache_type != ext_type_asciz) &&
	(v_val->cache_type != ext_type_ascid)) {
      mech_str = "&";
    }
    else {
      mech_str = "";
    }

    strcat (buffer, mech_str);
    strcat (buffer, rul___emit_get_ext_type_abbr (v_val->cache_type,
						  v_val->array_length));

    sprintf (loc_buf_num, "%ld", v_val->cache_index);
    strcat (buffer, loc_buf_num);

    if ((v_val->array_length != EXT__C_NOT_ARRAY) &&     /* it is an array */
	(v_val->array_index != VAL__C_NOT_AN_INDEX))	 /* elem index     */
      sprintf (loc_buf_arr, "[%ld]", v_val->array_index);
    else
      loc_buf_arr[0] = '\0';
    strcat (buffer, loc_buf_arr);
}




/********************************
**                             **
**  PUT_NAM_EXT_VAR_IN_STRING  **
**                             **
********************************/

static void put_nam_ext_var_in_string (char buffer[], Value value,
				       Boolean value_access)
{
    Named_Ext_Variable_Value v_val;
    char *buf = buffer;

    v_val = (Named_Ext_Variable_Value) value;

    if (value_access) {
      if ((v_val->passing_mech != ext_mech_value) &&
	  (v_val->array_length == EXT__C_NOT_ARRAY) &&
	  ((v_val->cache_type != ext_type_asciz) &&
	   (v_val->cache_type != ext_type_ascid))) {
	*buf++ = '*';
      }
    }
    else {
      if (v_val->passing_mech != ext_mech_value)
	*buf++ = '*';
    }
    rul__mol_use_printform(v_val->variable_name,
			   buf, 3*RUL_C_MAX_SYMBOL_SIZE+40);
}




/********************************
**                             **
**  PUT_UNM_MOL_VAR_IN_STRING  **
**                             **
********************************/

static void put_unm_mol_var_in_string (char buffer[], Value value)
{
    Unnamed_Mol_Variable_Value v_val;

    v_val = (Unnamed_Mol_Variable_Value) value;
    sprintf (buffer, "%s%ld", 
	     rul___emit_get_ext_type_abbr (ext_type_atom, EXT__C_NOT_ARRAY),
	     v_val->cache_index);
}

/*********************************
**                              **
**  PUT_RHS_VARIABLE_IN_STRING  **
**                              **
*********************************/

static void put_rhs_variable_in_string (char buffer[], Value value)
{
    RHS_Variable_Value v_val;

    v_val = (RHS_Variable_Value) value;
    sprintf (buffer, "%s[%ld]", GEN__C_RHS_VARIABLE_STR, v_val->cache_index);
}


/********************************
**                             **
**  RUL__EMIT_RHS_VAR_DECL     **
**                             **
********************************/

void rul__emit_rhs_var_decl (long number_vars)
{
    rul__emit_stack_internal (CMP__E_INT_MOLECULE,
			      GEN__C_RHS_VARIABLE_STR, 
			      number_vars);
}



/*********************************
**                              **
**  PUT_ANIMAL_CONST_IN_STRING  **
**                              **
*********************************/

static void put_animal_const_in_string (char buffer[], Value value)
{
	Animal_Constant_Value a_val;
	long i, j;
	unsigned char *in_str;
	unsigned char c;

	assert (is_animal_constant_val (value));

	a_val = (Animal_Constant_Value) value;
	switch (a_val->ext_type) {

	    case ext_type_long :
	    case ext_type_short :
	    case ext_type_byte :
			sprintf (buffer, "%ld", a_val->val.sign_int_val);
			break;

	    case ext_type_uns_long :
	    case ext_type_uns_short :
	    case ext_type_uns_byte :
			sprintf (buffer, "%luul", a_val->val.unsign_int_val);
			break;

	    case ext_type_float :
	    case ext_type_double :
			strcpy (buffer, 
				rul__mol_format_double (a_val->val.dbl_val));
			break;

	    case ext_type_asciz :
			in_str = (unsigned char *) a_val->val.asciz_val;
			j = 0;
			buffer[j++] = '"';
			i = 0;
			while (in_str[i] != '\0') {
			    c = in_str[i];
			    if (c == '"') {
				buffer[j++] = '\\';
				buffer[j++] = '"';
			    } else if (in_str[i] == '\\') { /* Quote `\' */
				buffer[j++] = '\\';
				buffer[j++] = '\\';
			    } else if (in_str[i] == '?') { /* Quote `?' */
				buffer[j++] = '\\';
				buffer[j++] = '?';
			    } else if (in_str[i] < ' ') {
				char oct_buff[6];
				sprintf (oct_buff, "%03o", c);
				buffer[j++] = '\\';
				buffer[j] = '\0';
				strcat (buffer, oct_buff);
				j += strlen (oct_buff);
			    } else {
				buffer[j++] = c;
			    }
			    i++;
			}
			buffer[j++] = '"';
			buffer[j] = '\0';
			break;

	    case ext_type_void_ptr :
			sprintf (buffer, "0x%p", a_val->val.opaque_val);
			break;

	    default :
			buffer[0] = '\0';
			break;
	}
}




/******************************
**                           **
**  PUT_LOCAL_REF_IN_STRING  **
**                           **
******************************/

static void put_local_ref_in_string (char buffer[], Rul_Internal_Field field)
{
	char *field_name = "";

	switch (field) {

	    case CMP__E_FIELD_BETA_INST_COUNT :
			field_name = "->inst_count";
			break;
	    case CMP__E_FIELD_BETA_INST_VEC :
			field_name = "->inst_array";
			break;
	    case CMP__E_FIELD_BETA_HSH_VEC :
			field_name = "->val_array";
			break;
	    case CMP__E_FIELD_BETA_AUX_VEC :
			field_name = "->aux_array";
			break;

	    case CMP__E_FIELD_DELTA_SIGN :
			field_name = "->sign";
			break;
	    case CMP__E_FIELD_DELTA_OBJ :
			field_name = "->instance";
			break;
	    case CMP__E_FIELD_DELTA_INST_ID :
			field_name = "->instance_id";
			break;
	    case CMP__E_FIELD_DELTA_CLASS :
			field_name = "->instance_class";
			break;
	}
	strcpy (buffer, field_name);
}




/**************************
**                       **
**  COND_OPER_TO_STRING  **
**                       **
**************************/

static char *cond_oper_to_string (Cond_Oper_Type oper)
{
	char *oper_string = "";

	switch (oper) {

	    case CMP__E_COND_AND :
			oper_string = " && ";
			break;
	    case CMP__E_COND_OR :
			oper_string = " || ";
			break;
	    case CMP__E_COND_NOT :
			oper_string = "!";
			break;
	}
	return (oper_string);
}






/***************************
**                        **
**  ARITH_OPER_TO_STRING  **
**                        **
***************************/

static char *arith_oper_to_string (Arith_Oper_Type oper)
{
	char *oper_string = "";

	switch (oper) {

	    case CMP__E_ARITH_PLUS :
			oper_string = " + ";
			break;
	    case CMP__E_ARITH_MINUS :
	    case CMP__E_ARITH_UNARY_MINUS :
			oper_string = " - ";
			break;
	    case CMP__E_ARITH_TIMES :
			oper_string = " * ";
			break;
	    case CMP__E_ARITH_DIVIDE :
			oper_string = " / ";
			break;
	    case CMP__E_ARITH_MODULUS :
			oper_string = " % ";
			break;
	}
	return (oper_string);
}


/****************************
**                         **
**  ARITH_OPER_TO_FUNCTION **
**                         **
****************************/

static char *arith_oper_to_function (Arith_Oper_Type oper)
{
	char *oper_string = "";

	switch (oper) {

	    case CMP__E_ARITH_PLUS :
#ifndef NDEBUG
			oper_string = "rul__mol_arith_add";
#else
			oper_string = "MAA";
#endif
			break;
	    case CMP__E_ARITH_MINUS :
#ifndef NDEBUG
			oper_string = "rul__mol_arith_subtract";
#else
			oper_string = "MAS";
#endif
			break;
	    case CMP__E_ARITH_UNARY_MINUS :
#ifndef NDEBUG
			oper_string = "rul__mol_arith_neg";
#else
			oper_string = "MAN";
#endif
			break;
	    case CMP__E_ARITH_TIMES :
#ifndef NDEBUG
			oper_string = "rul__mol_arith_multiply";
#else
			oper_string = "MAM";
#endif
			break;
	    case CMP__E_ARITH_DIVIDE :
#ifndef NDEBUG
			oper_string = "rul__mol_arith_divide";
#else
			oper_string = "MAD";
#endif
			break;
	    case CMP__E_ARITH_MODULUS :
#ifndef NDEBUG
			oper_string = "rul__mol_arith_modulo";
#else
			oper_string = "MAO";
#endif
			break;
	}
	return (oper_string);
}




/**********************************
**                               **
**  ALL_ARITH_ELEMS_ARE_NUMBERS  **
**                               **
**********************************/

static Boolean all_arith_elems_are_numbers (Arith_Expr_Value a_val)
{
	if (a_val->arith_operator == CMP__E_ARITH_UNARY_MINUS) {
	    return (value_is_a_number (a_val->operand_1));
	} else {
	    return (value_is_a_number (a_val->operand_1)  &&
		    value_is_a_number (a_val->operand_2));
	}
}



/************************
**                     **
**  VALUE_IS_A_NUMBER  **
**                     **
************************/

static Boolean value_is_a_number (Value value)
{
	Animal_Constant_Value      an_val;
	Unnamed_Ext_Variable_Value ext_val;

	switch (value->type) {

	    case VAL__E_ANIMAL_CONST :
		an_val = (Animal_Constant_Value) value;
		if (an_val->ext_type == ext_type_asciz) return (FALSE);
		if (an_val->ext_type == ext_type_ascid) return (FALSE);
		if (an_val->ext_type == ext_type_void_ptr)  return (FALSE);
		if (an_val->ext_type == ext_type_atom)  return (FALSE);
		if (an_val->ext_type == ext_type_atom_ptr)  return (FALSE);
		return (TRUE);
		break;

	    case VAL__E_BLK_OR_LOCAL :
		if (((Blk_Or_Local_Value)value)->arg_type == CMP__E_INT_LONG) {
		    return (TRUE);
		}
		return (FALSE);
		break;

	    case VAL__E_ARITH_EXPR :
		return (all_arith_elems_are_numbers ((Arith_Expr_Value) value));
		break;

	    /*?*
	    **	What about VAL__E_UNM_EXT_VAR, or VAL__E_FUNCTION's whose
	    **  return type is a number, or VAL__E_RUL_CONST where the
	    **  molecule happens to be a number, or ...
	    */
	      case VAL__E_UNM_EXT_VAR :
		ext_val = (Unnamed_Ext_Variable_Value) value;
		if ((ext_val->array_length == 0) &&
		    (ext_val->cache_type == ext_type_long ||
		     ext_val->cache_type == ext_type_short ||
		     ext_val->cache_type == ext_type_byte ||
		     ext_val->cache_type == ext_type_uns_long ||
		     ext_val->cache_type == ext_type_uns_short ||
		     ext_val->cache_type == ext_type_uns_byte ||
		     ext_val->cache_type == ext_type_float ||
		     ext_val->cache_type == ext_type_double ||
		     ext_val->cache_type == ext_type_for_each) &&
		    (ext_val->passing_mech == ext_mech_value))
		  return TRUE;

		return (FALSE);
		break;
	}	

	return (FALSE);
}
