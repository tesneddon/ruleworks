/****************************************************************************
**                                                                         **
**               C M P _ S E M _ B U I L D _ V A L . C                     **
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
 *	RULEWORKS compiler
 *
 * ABSTRACT:
 *	This module converts the abstract syntax trees for a
 *	value expression into the Value intermediate format.
 *
 * MODIFIED BY:
 *	DEC	Digital Equipment Corporation
 *	CPQ	Compaq Computer Corporation
 *
 * MODIFICATION HISTORY:
 *
 *	19-Jan-1993	DEC	Initial version
 *	01-Dec-1999	CPQ	Release with GPL
 */

#include <common.h>
#include <cmp_comm.h>
#include <sem.h>
#include <ast.h>
#include <val.h>
#include <mol.h>
#include <ios.h>
#include <msg.h>
#include <cmp_msg.h>



/***************************
**                        **
**  RUL__SEM_CHECK_VALUE  **
**                        **
***************************/

Value 
rul__sem_check_value (Ast_Node ast, 
		      Boolean on_lhs, long ce_index,
		      Boolean return_molecule,
		      Decl_Domain domain, Decl_Shape shape,
		      Value return_to_value)
{
    if (on_lhs)
	return rul__sem_check_lhs_value (ast, ce_index,
					 return_molecule, domain, shape);
    else
	return rul__sem_check_rhs_value (ast, return_molecule, 
				  	 domain, shape, return_to_value);
}





/**********************************
**                               **
**  RUL__SEM_CHECK_NESTED_VALUE  **
**                               **
**********************************/

Value rul__sem_check_nested_value (Ast_Node ast,
                            Boolean on_lhs, long ce_index,
                            Boolean return_molecule,
                            Decl_Domain domain, Decl_Shape shape,
                            Value return_to_value)
{
	/*
	**  This function is a variant of rul__sem_check_value that will
	**  create a temporary molecule variable for a return_to_value,
	**  if one is needed because of the nesting of expressions.
	**  It will also add a call to rul__mol_incr_uses, if one is
	**  needed for RHS named variables.
	**
	**  All the arithmetic operator functions and all built-in 
	**  functions always increment the reference count on the molecule 
	**  being returned before they return it.
	*/
	Value result_val = NULL;
	Value last_val, incr_func_val;
	Ast_Node_Type type;

	assert (ast != NULL);
	type = rul__ast_get_type (ast);

	if (return_molecule == TRUE		&&
		    type != AST__E_CONSTANT		&&
		    type != AST__E_ARITH_EXPR		&&
		    type != AST__E_VARIABLE		&&
		    type != AST__E_CALL_INHERITED	&&
		    type != AST__E_EXT_RT_CALL		&&
		    type != AST__E_BTI_ACCEPT_ATOM	&&
		    type != AST__E_BTI_ACCEPTLINE_COMPOUND &&
		    type != AST__E_BTI_COMPOUND		&&
		    type != AST__E_BTI_CONCAT		&&
		    type != AST__E_BTI_CRLF		&&
		    type != AST__E_BTI_EVERY		&&
		    type != AST__E_BTI_FLOAT		&&
		    type != AST__E_BTI_GENATOM		&&
		    type != AST__E_BTI_GENINT		&&
		    type != AST__E_BTI_GET		&&
		    type != AST__E_BTI_INTEGER		&&
		    type != AST__E_BTI_IS_OPEN		&&
		    type != AST__E_BTI_LENGTH		&&
		    type != AST__E_BTI_MAX		&&
		    type != AST__E_BTI_MIN		&&
		    type != AST__E_BTI_NTH		&&
		    type != AST__E_BTI_POSITION		&&
		    type != AST__E_BTI_RJUST		&&
		    type != AST__E_BTI_SUBSYMBOL	&&
		    type != AST__E_BTI_SUBCOMPOUND	&&
		    type != AST__E_BTI_SYMBOL		&&
		    type != AST__E_BTI_SYMBOL_LENGTH	&&
		    type != AST__E_BTI_TABTO	)
	{
	    /*
	    **  We now know we need to increment the reference count.
	    **  Now we need to either call incr_uses on the return_to_value
	    **  or we need to rul__sem_create_temp_var, which will do the
	    **  incr_uses for us automatically.
	    */

	    if (return_to_value == NULL) {

		/*  Create a temporary molecule variable for the return_to  */
		result_val = rul__sem_check_value (ast, on_lhs, ce_index,
					   return_molecule, domain, shape, 
					   rul__sem_create_temp_var (on_lhs));

	    } else if (rul__val_is_rhs_variable (return_to_value)) {

		/*  Special case if the return is to a named RHS variable
		**  (i.e. this is the value within a bind action)
		*/
		result_val = rul__sem_check_value (ast, on_lhs, ce_index,
					   return_molecule, domain, shape, 
					   return_to_value);
		if (result_val == NULL)
		    return NULL;
		last_val = result_val;
		while (rul__val_get_next_value (last_val) != NULL) {
		    last_val = rul__val_get_next_value (last_val);
		}
		incr_func_val = rul__val_create_function_str (
#ifndef NDEBUG
						"rul__mol_incr_uses", 1);
#else
						"MIU", 1);
#endif
		rul__val_set_nth_subvalue (incr_func_val, 1,
					rul__val_copy_value (return_to_value));
		rul__val_set_next_value (last_val, incr_func_val);
	    }
	}


	if (result_val == NULL) {
	    /*
	    **  If none of the special cases apply, then we have
	    **  no need to mess with reference counts.
	    */
	    result_val = rul__sem_check_value (ast, on_lhs, ce_index,
					   return_molecule, domain, shape, 
					   return_to_value);
	}
	return (result_val);
}




/*******************************
**                            **
**  RUL__SEM_CHECK_RHS_VALUE  **
**                            **
*******************************/

Value 
rul__sem_check_rhs_value (Ast_Node ast, Boolean return_molecule, 
			  Decl_Domain domain, Decl_Shape shape,
			  Value return_to_value)
{
    Ast_Node_Type       type;
    Value		value, incr_val;

    type = rul__ast_get_type (ast);

    switch (type)
	{

	/* constant */
	case AST__E_CONSTANT:
	  value =  rul__sem_check_rul_constant (ast, return_molecule);
	  if (value == NULL) return value;
	  if (rul__sem_check_value_type (ast, domain, shape,
					 rul__val_get_domain (value),
					 rul__val_get_shape (value),
					 "situation")) {
	    if (return_to_value != NULL)
	      value = rul__val_create_assignment (return_to_value, value);
	    return value;
	  }
	  else {
	    rul__val_free_value (value);
	    return NULL;
	  }

	/* variable */
	case AST__E_VARIABLE:
	    value = rul__sem_check_rhs_var (ast, SEM__C_VALUE_ACCESS);
	    if (value == NULL)
		return value;
	    else 
		{
		if (rul__sem_check_value_type (ast, domain, shape,
					       rul__val_get_domain (value),
					       rul__val_get_shape (value),
					       "situation")) {
		  if (return_to_value == NULL) return value;
		  value = rul__val_create_assignment (return_to_value, value);
		  return value;
		}
		else
		    {
		    rul__val_free_value (value);
		    return NULL;
		    }
		}

	/* built_ins */
	case AST__E_BTI_ACCEPT_ATOM:
	    if (rul__sem_check_value_type (ast, domain, shape, dom_any,
				   shape_atom, "ACCEPT-ATOM's return value"))
		return rul__sem_check_bti_accept (ast, shape_atom,
					          return_to_value);
	    else 
		{
		return NULL;
		}

	case AST__E_BTI_ACCEPTLINE_COMPOUND:
	    if (rul__sem_check_value_type (ast, domain, shape,
					   dom_any, shape_compound,
				   "ACCEPTLINE-COMPOUND's return value"))
		return rul__sem_check_bti_accept (ast, shape_compound,
					          return_to_value);
	    else 
		{
		return NULL;
		}

	case AST__E_BTI_COMPOUND:
	    value = rul__sem_check_bti_compound (ast, SEM__C_ON_RHS,
			SEM__C_UNDEFINED_CE_INDEX, return_to_value);
	    if (value == NULL) return NULL;
	    if (rul__sem_check_value_type (ast, domain, shape, 
				rul__val_get_domain (value),
				shape_compound, "COMPOUND's return value")) {
		return value;
	    }
	    return NULL;
	    break;

	case AST__E_BTI_CONCAT:
	    if (rul__sem_check_value_type (ast, domain, shape, 
					   dom_symbol, shape_atom,
					   "CONCAT's return value"))
		return rul__sem_check_bti_concat (ast, SEM__C_ON_RHS, 
						  SEM__C_UNDEFINED_CE_INDEX,
					          return_to_value);
	    else
		{
		return NULL;
		}

	case AST__E_BTI_CRLF:
	    return rul__sem_check_bti_crlf (ast);

	case AST__E_BTI_EVERY:
	    if (rul__sem_check_value_type (ast, domain, shape, 
					   dom_instance_id, shape_compound,
					   "EVERY's return value"))
		return rul__sem_check_bti_every (ast, SEM__C_ON_RHS,
						 SEM__C_UNDEFINED_CE_INDEX,
					         return_to_value);
	    else
		{
		return NULL;
		}

	case AST__E_BTI_FLOAT:
	    if (rul__sem_check_value_type (ast, domain, shape, 
					   dom_dbl_atom, shape_atom,
					   "FLOAT's return value"))
		return rul__sem_check_bti_float (ast, SEM__C_ON_RHS,
						 SEM__C_UNDEFINED_CE_INDEX,
					         return_to_value);
	    else
		{
		return NULL;
		}

	case AST__E_BTI_GENATOM:
	    if (rul__sem_check_value_type (ast, domain, shape, 
					   dom_symbol, shape_atom,
					   "GENATOM's return value"))
		return rul__sem_check_bti_genatom (ast, return_to_value);
	    else
		{
		return NULL;
		}

	case AST__E_BTI_GENINT:
	    if (rul__sem_check_value_type (ast, domain, shape, 
					   dom_int_atom, shape_atom,
					   "GENINT's return value"))
		return rul__sem_check_bti_genint (ast, return_to_value);
	    else
		{
		return NULL;
		}

	case AST__E_BTI_GET:
	      return rul__sem_check_bti_get (ast, domain, 
					     shape, return_to_value);

	case AST__E_BTI_INTEGER:
	    if (rul__sem_check_value_type (ast, domain, shape, 
					   dom_int_atom, shape_atom,
					   "INTEGER's return value"))
		return rul__sem_check_bti_integer (ast, SEM__C_ON_RHS,
						   SEM__C_UNDEFINED_CE_INDEX,
					           return_to_value);
	    else
		{
		return NULL;
		}

	case AST__E_BTI_IS_OPEN:
	    if (rul__sem_check_value_type (ast, domain, shape, 
					   dom_symbol, shape_atom,
					   "IS-OPEN's return value"))
		return rul__sem_check_bti_is_open (ast, SEM__C_ON_RHS,
						   SEM__C_UNDEFINED_CE_INDEX,
					           return_to_value);
	    else
		{
		return NULL;
		}

	case AST__E_BTI_LENGTH:
	    if (rul__sem_check_value_type (ast, domain, shape, 
					   dom_int_atom, shape_atom,
					   "LENGTH's return value"))
		return rul__sem_check_bti_length (ast, SEM__C_ON_RHS, 
						  SEM__C_UNDEFINED_CE_INDEX,
					          return_to_value);
	    else
		{
		return NULL;
		}

	case AST__E_BTI_MAX:
	    if (rul__sem_check_value_type (ast, domain, shape,
					   dom_any, shape_atom,
					   "MAX's return value"))
		return rul__sem_check_bti_max (ast, TRUE, SEM__C_ON_RHS,
					       SEM__C_UNDEFINED_CE_INDEX,
					       return_to_value);
	    else
		{
		return NULL;
		}

	case AST__E_BTI_MIN:
	    if (rul__sem_check_value_type (ast, domain, shape,
					   dom_any, shape_atom,
					   "MIN's return value"))
		return rul__sem_check_bti_max (ast, FALSE, SEM__C_ON_RHS,
					       SEM__C_UNDEFINED_CE_INDEX,
					       return_to_value);
	    else
		{
		return NULL;
		}

	case AST__E_BTI_NTH:
	    if (rul__sem_check_value_type (ast, domain, shape,
					   dom_any, shape_atom,
					   "NTH's return value"))
		return rul__sem_check_bti_nth (ast, SEM__C_ON_RHS,
					       SEM__C_UNDEFINED_CE_INDEX,
					       return_to_value);
	    else
		{
		return NULL;
		}

	case AST__E_BTI_POSITION:
	    if (rul__sem_check_value_type (ast, domain, shape, 
					   dom_int_atom, shape_atom,
					   "POSITION's return value"))
		return rul__sem_check_bti_position (ast, SEM__C_ON_RHS, 
						    SEM__C_UNDEFINED_CE_INDEX,
					            return_to_value);
	    else
		{
		return NULL;
		}

	case AST__E_BTI_RJUST:
	    return  rul__sem_check_bti_rjust (ast);

	case AST__E_BTI_SUBCOMPOUND:
	    value = rul__sem_check_bti_subcompound (ast, SEM__C_ON_RHS, 
				SEM__C_UNDEFINED_CE_INDEX, return_to_value);
	    if (value == NULL) return NULL;
	    if (rul__sem_check_value_type (ast, domain, shape, 
				rul__val_get_domain (value),
				shape_compound, "SUBCOMPOUND's return value")){
		return value;
	    }
	    return NULL;
	    break;

	case AST__E_BTI_SUBSYMBOL:
	    value = rul__sem_check_bti_subsymbol(ast, SEM__C_ON_RHS, 
				SEM__C_UNDEFINED_CE_INDEX, return_to_value);
	    if (value == NULL) return NULL;
	    if (rul__sem_check_value_type (ast, domain, shape, 
					   dom_symbol, shape_atom,
					   "SUBSYMBOL's return value")){
		return value;
	    }
	    return NULL;
	    break;

	case AST__E_BTI_SYMBOL:
	    if (rul__sem_check_value_type (ast, domain, shape, 
					   dom_symbol, shape_atom,
					   "SYMBOL's return value"))
		return rul__sem_check_bti_symbol (ast, SEM__C_ON_RHS,
						  SEM__C_UNDEFINED_CE_INDEX,
					          return_to_value);
	    else
		{
		return NULL;
		}

	case AST__E_BTI_SYMBOL_LENGTH:
	    value = rul__sem_check_bti_symbol_len(ast, SEM__C_ON_RHS, 
				SEM__C_UNDEFINED_CE_INDEX, return_to_value);
	    if (value == NULL) return NULL;
	    if (rul__sem_check_value_type (ast, domain, shape, 
					   dom_int_atom, shape_atom,
					   "SYMBOL-LENGTH's return value")){
		return value;
	    }
	    return NULL;
	    break;

	case AST__E_BTI_TABTO:
	    return rul__sem_check_bti_tabto (ast);

	/* function_call */
	case AST__E_EXT_RT_CALL:
    	    return rul__sem_check_ext_rt_call (ast,
					       SEM__C_RETURN_VALUE_REQUIRED,
					       SEM__C_ON_RHS,
					       SEM__C_UNDEFINED_CE_INDEX,
					       return_molecule, domain, shape,
					       return_to_value);

	/* arith_expr */
	case AST__E_ARITH_EXPR:
    	    return rul__sem_check_arith_expr (rul__ast_get_child (ast), 
					      SEM__C_ON_RHS,
					      SEM__C_UNDEFINED_CE_INDEX,
					      domain, shape, return_to_value);
	case AST__E_OPR_PLUS:
	case AST__E_OPR_MINUS:
	case AST__E_OPR_TIMES:
	case AST__E_OPR_DIVIDE:
	case AST__E_OPR_MODULUS:
	case AST__E_OPR_UMINUS:
	    return rul__sem_check_arith_expr (ast, SEM__C_ON_RHS,
					      SEM__C_UNDEFINED_CE_INDEX,
					      domain, shape, return_to_value);

	/* special actions used as values... */
	case AST__E_COPY_ACTION:
	    if (rul__sem_check_value_type (ast, domain, shape,
					   dom_instance_id, shape_atom,
					   "COPY's return value"))
    		return rul__sem_check_copy (ast, SEM__C_RETURN_VALUE_REQUIRED, 
					    return_to_value);
	    else
		{
		return NULL;
		}

	case AST__E_MAKE_ACTION:
	    if (rul__sem_check_value_type (ast, domain, shape, 
					   dom_instance_id, shape_atom,
					   "MAKE's return value"))
		return rul__sem_check_make (ast, SEM__C_RETURN_VALUE_REQUIRED, 
					    return_to_value);
	    else
		{
		return NULL;
		}

	case AST__E_MODIFY_ACTION:
	    if (rul__sem_check_value_type (ast, domain, shape, 
					   dom_instance_id, shape_atom,
					   "MODIFY's return value"))
		return rul__sem_check_modify (ast,SEM__C_RETURN_VALUE_REQUIRED,
					      return_to_value);
	    else
		{
		return NULL;
		}

	case AST__E_SPECIALIZE_ACTION:
	    if (rul__sem_check_value_type (ast, domain, shape, 
					   dom_instance_id, shape_atom,
					   "SPECIALIZE's return value"))
		return rul__sem_check_specialize (ast,
						  SEM__C_RETURN_VALUE_REQUIRED,
					          return_to_value);
	    else
		{
		return NULL;
		}
      
	case AST__E_CALL_INHERITED:
	    if (rul__sem_check_value_type (ast, domain, shape, 
					   dom_instance_id, shape_atom,
					   "CALL-INHERITED's return value"))
		return rul__sem_check_call_inherited (ast,
					      SEM__C_RETURN_VALUE_REQUIRED,
						      return_to_value);
	    else
		{
		return NULL;
		}
      
	case AST__E_SQL_FETCH_AS_OBJECT_ACTION:
	    if (rul__sem_check_value_type (ast, domain, shape, 
					   dom_instance_id, shape_compound,
				   "SQL-FETCH-AS-OBJECT's return value"))
		return rul__sem_check_sql_fetch_obj(ast,
					    SEM__C_RETURN_VALUE_REQUIRED,
						    return_to_value);
	    else
		{
		return NULL;
		}

	/* Invalid value */
	default:
#ifndef NDEBUG
	    rul__ios_printf (RUL__C_STD_ERR,
			     "\n rul__val_build got unknown ast type: %s\n",
			     rul___ast_type_to_string (type));
#endif
	    rul__msg_cmp_print(CMP_INVRHSVAL, ast);
	    return NULL;
	}
}


/*******************************
**                            **
**  RUL__SEM_CHECK_LHS_VALUE  **
**                            **
*******************************/

Value rul__sem_check_lhs_value (Ast_Node ast, long ce_index,
				Boolean return_molecule, 
				Decl_Domain domain, Decl_Shape shape)
{
    Ast_Node_Type	type;
    Ast_Node		tmp_node;
    Value		val, tmp_val;
    long		i;

    type = rul__ast_get_type (ast);

    switch (type) 
	{
	case AST__E_CONSTANT :
	    val = rul__sem_check_rul_constant (ast, return_molecule);
	    if ((val == NULL) ||
		(rul__sem_check_value_type (ast, domain, shape,
					    rul__val_get_domain (val),
					    rul__val_get_shape (val),
					    "situation")))
		return val;
	    else 
		{
		rul__val_free_value (val);
		return NULL;
		}

	case AST__E_VARIABLE :
	    return rul__sem_check_lhs_var (ast, ce_index, 
					   domain, shape, "situation");

	/* built_ins */
	case AST__E_BTI_COMPOUND:
	    if (rul__sem_check_value_type (ast, domain, shape, 
					   dom_symbol, shape_compound,
					   "COMPOUND's return value"))
		return rul__sem_check_bti_compound (ast, SEM__C_ON_LHS,
						    ce_index, NULL);
	    else
		{
		return NULL;
		}

	case AST__E_BTI_CONCAT:
	    if (rul__sem_check_value_type (ast, domain, shape, 
					   dom_symbol, shape_atom,
					   "CONCAT's return value"))
	      return rul__sem_check_bti_concat (ast, SEM__C_ON_LHS,
						ce_index, NULL);
	    else
		{
		return NULL;
		}

	case AST__E_BTI_FLOAT:
	    if (rul__sem_check_value_type (ast, domain, shape, 
					   dom_dbl_atom, shape_atom,
					   "FLOAT's return value"))
		return rul__sem_check_bti_float (ast, SEM__C_ON_LHS,
						 ce_index, NULL);
	    else
		{
		return NULL;
		}

	case AST__E_BTI_INTEGER:
	    if (rul__sem_check_value_type (ast, domain, shape, 
					   dom_int_atom, shape_atom,
					   "INTEGER's return value"))
		return rul__sem_check_bti_integer (ast, SEM__C_ON_LHS,
						   ce_index, NULL);
	    else
		{
		return NULL;
		}

	case AST__E_BTI_LENGTH:
	    if (rul__sem_check_value_type (ast, domain, shape, 
					   dom_int_atom, shape_atom,
					   "LENGTH's return value"))
	      return rul__sem_check_bti_length (ast, SEM__C_ON_LHS,
						ce_index, NULL);
	    else
		{
		return NULL;
		}

	case AST__E_BTI_MAX:
	    if (rul__sem_check_value_type (ast, domain, shape,
					   dom_any, shape_atom,
					   "MAX's return value"))
		return rul__sem_check_bti_max (ast, TRUE, SEM__C_ON_LHS,
					       ce_index, NULL);
	    else
		{
		return NULL;
		}

	case AST__E_BTI_MIN:
	    if (rul__sem_check_value_type (ast, domain, shape,
					   dom_any, shape_atom,
					   "MIN's return value"))
		return rul__sem_check_bti_max (ast, FALSE, SEM__C_ON_LHS,
					       ce_index, NULL);
	    else
		{
		return NULL;
		}

	case AST__E_BTI_NTH:
	    if (rul__sem_check_value_type (ast, domain, shape,
					   dom_any, shape_atom,
					   "NTH's return value"))
		return rul__sem_check_bti_nth (ast, SEM__C_ON_LHS,
					       ce_index, NULL);
	    else
		{
		return NULL;
		}

	case AST__E_BTI_POSITION:
	    if (rul__sem_check_value_type (ast, domain, shape, 
					   dom_int_atom, shape_atom,
					   "POSITION's return value"))
		return rul__sem_check_bti_position (ast, SEM__C_ON_LHS,
						    ce_index, NULL);
	    else
		{
		return NULL;
		}

	case AST__E_BTI_SUBCOMPOUND:
	    val = rul__sem_check_bti_subcompound (ast, SEM__C_ON_LHS, 
						  ce_index, NULL);
	    if (val == NULL) return NULL;
	    if (rul__sem_check_value_type (ast, domain, shape, 
					   rul__val_get_domain (val),
					   shape_compound,
					   "SUBCOMPOUND's return value")) {
		return val;
	    }
	    return NULL;
	    break;

	case AST__E_BTI_SUBSYMBOL:
	    val = rul__sem_check_bti_subsymbol (ast, SEM__C_ON_LHS, 
						ce_index, NULL);
	    if (val == NULL) return NULL;
	    if (rul__sem_check_value_type (ast, domain, shape, 
					   dom_symbol, shape_atom,
					   "SUBSYMBOL's return value")) {
		return val;
	    }
	    return NULL;
	    break;

	case AST__E_BTI_SYMBOL:
	    if (rul__sem_check_value_type (ast, domain, shape, 
					   dom_symbol, shape_atom,
					   "SYMBOL's return value"))
		return rul__sem_check_bti_symbol (ast, SEM__C_ON_LHS,
						  ce_index, NULL);
	    else
		{
		return NULL;
		}

	case AST__E_BTI_SYMBOL_LENGTH:
	    val = rul__sem_check_bti_symbol_len (ast, SEM__C_ON_LHS, 
						 ce_index, NULL);
	    if (val == NULL) return NULL;
	    if (rul__sem_check_value_type (ast, domain, shape, 
					   dom_int_atom, shape_atom,
					   "SYMBOL-LENGTH's return value")) {
		return val;
	    }
	    return NULL;
	    break;

	/* function_call */
	case AST__E_EXT_RT_CALL:
    	    return rul__sem_check_ext_rt_call (ast,
					       SEM__C_RETURN_VALUE_REQUIRED,
					       SEM__C_ON_LHS, ce_index,
					       return_molecule, domain, shape,
					       NULL);

	/* arith_expr */
	case AST__E_ARITH_EXPR:
    	    return rul__sem_check_arith_expr (
			    rul__ast_get_child (ast),
			    SEM__C_ON_LHS, ce_index, domain, shape, NULL);
	case AST__E_OPR_PLUS:
	case AST__E_OPR_MINUS:
	case AST__E_OPR_TIMES:
	case AST__E_OPR_DIVIDE:
	case AST__E_OPR_MODULUS:
	case AST__E_OPR_UMINUS:
	    return rul__sem_check_arith_expr (ast, SEM__C_ON_LHS, ce_index,
					      domain, shape, NULL);

	case AST__E_VALUE_DISJ :
	    val = rul__val_create_disjunction (
		      rul__ast_get_child_count (ast));
	    tmp_node = rul__ast_get_child (ast);
	    i = 1;
	    while (tmp_node) {
		tmp_val = rul__sem_check_lhs_value (tmp_node, ce_index,
						    return_molecule,
						    domain, shape);
		rul__val_set_nth_subvalue (val, i, tmp_val);
		i++;
		tmp_node = rul__ast_get_sibling (tmp_node);
		}
	    return val;

	default:
	    rul__msg_cmp_print (CMP_INVLHSFUN, ast,
				rul___ast_type_to_string (type));
	    return NULL;
	}
}



char *rul__sem_shape_to_string (Decl_Shape shape)
{
  if (shape == shape_molecule)
    return "a molecule";
  else if (shape == shape_atom)
    return "an atom";
  else if (shape == shape_compound)
    return "a compound";
  else if (shape == shape_table)
    return "a table";
  else
    return "unknown";
}

char *rul__sem_domain_to_string (Decl_Domain dom)
{
  if (dom == dom_any)
    return "any";
  else if (dom == dom_symbol)
    return "a symbol";
  else if (dom == dom_instance_id)
    return "an instance-id";
  else if (dom == dom_opaque)
    return "an opaque";
  else if (dom == dom_number)
    return "a numeric atom";
  else if (dom == dom_int_atom)
    return "an integer";
  else if (dom == dom_dbl_atom)
    return "a float";
  else
    return "unknown";
}

/*******************************
**                            **
**  RUL__SEM_CHECK_VALUE_TYPE **
**                            **
*******************************/

Boolean rul__sem_check_value_type (Ast_Node     ast,
				   Decl_Domain  required_domain,
                                   Decl_Shape   required_shape,
                                   Decl_Domain  return_domain,
                                   Decl_Shape   return_shape,
				   char        *msgstring)
{
  Boolean        wrong_shape = FALSE;
  char          *what;
  char          *exp;
  char          *was;
  Molecule       name;
  Ast_Node_Type	 type, parent_type;


  if ((required_domain == return_domain && required_shape == return_shape) ||
      (required_domain == dom_any && required_shape == shape_molecule) ||
      (return_domain == dom_any && return_shape == shape_molecule))
    /* perfect match or no information with which to detect error */
    return TRUE;

  /* simple case done, check shape */
  
  if ((required_shape == shape_compound) && 
      (return_shape != shape_compound) &&
      (return_shape != shape_molecule)) {
    /* require compound; returning non compound */
    wrong_shape = TRUE;
  }

  else if ((required_shape == shape_atom) &&
	   (return_shape != shape_atom) &&
	   (return_shape != shape_molecule)) {
    /* require scalar; returning non scalar */
    wrong_shape = TRUE;
  }

  else if ((required_shape == shape_table) &&
	   (return_shape != shape_table) &&
	   (return_shape != shape_molecule)) {
    /* require table; returning non table */
    wrong_shape = TRUE;
  }

  /* Shape OK check Domain */
  
  else if (required_domain == dom_any || 
	   return_domain == dom_any ||
	   required_domain == return_domain) {
    /* perfect match or no domain restriction */
    return TRUE;
  }
  
  else if (required_domain == dom_number) {
    if (return_domain == dom_int_atom || return_domain == dom_dbl_atom)
      return TRUE;
  }

  else if (required_domain == dom_int_atom) {
    if (return_domain == dom_number)
      return TRUE;
  }

  else if (required_domain == dom_dbl_atom) {
    if (return_domain == dom_number)
      return TRUE;
  }
  
  if (wrong_shape) {
    if (required_domain != dom_any) {
        exp = rul__sem_domain_to_string (required_domain);
    } else {
        exp = rul__sem_shape_to_string (required_shape);
    }
    was = rul__sem_shape_to_string (return_shape);
  }
  else {
    exp = rul__sem_domain_to_string (required_domain);
    was = rul__sem_domain_to_string (return_domain);
  }

  type = rul__ast_get_type (ast);
  parent_type = rul__ast_get_type (rul__ast_get_parent (ast));

  /*  Figure out how to describe this 'thing' that has the wrong type  */

  switch (parent_type) {
    case AST__E_OPR_PLUS:
    case AST__E_OPR_MINUS:
    case AST__E_OPR_TIMES:
    case AST__E_OPR_DIVIDE:
    case AST__E_OPR_MODULUS:
    case AST__E_OPR_UMINUS:
		msgstring = CMP__C_MSG_ARITH_OPERAND;
		break;
    case AST__E_EXT_RT_CALL:
		msgstring = CMP__C_MSG_ARGUMENT;
		break;
    case AST__E_ATTR_INDEX:
		msgstring = CMP__C_MSG_ATTR_INDEX;
		break;
    case AST__E_RHS_ACTION_TERM:
		if (rul__ast_get_value_type (ast) == AST__E_TYPE_MOL) {
		  msgstring = CMP__C_MSG_ATTRIBUTE;
		}
		break;
  }

  if (rul__ast_get_value_type (ast) == AST__E_TYPE_MOL) {

    what = rul__mol_get_printform ((Molecule) rul__ast_get_value (ast));
    if (parent_type == AST__E_PRED_TEST) {
      rul__msg_cmp_print (CMP_MISMATCHTYP, ast, exp, what, was);
      return TRUE; /* forgivable, just can't ever match */
    } else {
      rul__msg_cmp_print (CMP_INVVALTYP, ast, msgstring, exp, what, was);
    }
    rul__mem_free (what);

  } else {
    if (parent_type == AST__E_PRED_TEST) {
      rul__msg_cmp_print (CMP_MISMATCHTYP, ast, exp, msgstring, was);
      return TRUE; /* forgivable, just can't ever match */
    } else if (parent_type == AST__E_RHS_ACTION_TERM) {
      rul__msg_cmp_print (CMP_INVVALTYP, ast, CMP__C_MSG_ATTRIBUTE,
					exp, msgstring, was);
    } else {
      rul__msg_cmp_print (CMP_INVVALUE, ast, exp, msgstring, was);
    }
  }

  return FALSE;
}


/********************************
**                             **
**  CONVERT_AST_TO_ARITH_TYPE  **
**                             **
********************************/

static Arith_Oper_Type  convert_ast_to_arith_type (Ast_Node_Type type)
{
  register Arith_Oper_Type oper_type;

	switch (type) {

	    case AST__E_OPR_UMINUS :
			oper_type = CMP__E_ARITH_UNARY_MINUS;
			break;
	    case AST__E_OPR_PLUS :
			oper_type = CMP__E_ARITH_PLUS;
			break;
	    case AST__E_OPR_MINUS :
			oper_type = CMP__E_ARITH_MINUS;
			break;
	    case AST__E_OPR_TIMES :
			oper_type = CMP__E_ARITH_TIMES;
			break;
	    case AST__E_OPR_DIVIDE :
			oper_type = CMP__E_ARITH_DIVIDE;
			break;
	    case AST__E_OPR_MODULUS :
			oper_type = CMP__E_ARITH_MODULUS;
			break;
			default :
			  assert (FALSE);
			oper_type = (Arith_Oper_Type) 0;
			break;
	}

  return oper_type;
}


/********************************
**                             **
**  RUL__SEM_CHECK_ARITH_EXPR  **
**                             **
********************************/

Value
rul__sem_check_arith_expr (Ast_Node ast,
			   Boolean on_lhs, long ce_index,
			   Decl_Domain domain, Decl_Shape shape,
			   Value return_to_value)
{
    Ast_Node        operand1,
		    operand2;
    Ast_Node_Type   type;
    Value	    var_value, value = NULL, op1_val, op2_val;
    Decl_Shape      actual_shape;
    Decl_Domain     actual_domain;

    type = rul__ast_get_type (ast);
    switch (type)
      {
      case AST__E_OPR_UMINUS:
	operand1 = rul__ast_get_child (ast);
	op1_val = rul__sem_check_value (operand1, on_lhs, ce_index,
					SEM__C_RETURN_RUL_TYPE,
					dom_number, shape_atom, NULL);
	if (op1_val)
	  value = rul__val_create_arith_expr (CMP__E_ARITH_UNARY_MINUS,
					      op1_val, NULL);
	break;

      case AST__E_OPR_PLUS:
      case AST__E_OPR_MINUS:
      case AST__E_OPR_TIMES:
      case AST__E_OPR_DIVIDE:
      case AST__E_OPR_MODULUS:      
	operand1 = rul__ast_get_child (ast);
	op1_val = rul__sem_check_value (operand1, on_lhs, ce_index,
					SEM__C_RETURN_RUL_TYPE, 
					dom_number, shape_atom, NULL);
	operand2 = rul__ast_get_sibling (operand1);
	op2_val = rul__sem_check_value (operand2, on_lhs, ce_index,
					  SEM__C_RETURN_RUL_TYPE,
					  dom_number, shape_atom, NULL);
	if (op1_val && op2_val)
	  value = rul__val_create_arith_expr (convert_ast_to_arith_type (type),
					      op1_val, op2_val );

	break;

      case AST__E_ARITH_EXPR:
	return rul__sem_check_arith_expr (rul__ast_get_child (ast), on_lhs,
					  ce_index, domain, shape, 
					  return_to_value);

      default:
	return rul__sem_check_value (ast, on_lhs, ce_index,
				     SEM__C_RETURN_RUL_TYPE,
				     domain, shape, return_to_value);
      }

    if (value == NULL) return NULL;

    /*
    **  Check the resulting domain (set based on the operand 
    **  domains) against the required domain.
    */
    actual_domain = rul__val_get_domain (value);
    actual_shape  = rul__val_get_shape  (value);
    if (!rul__sem_check_value_type (ast, domain, shape,
				    actual_domain, actual_shape,
				    "arithmetic expression")) {
	rul__val_free_value (value);
	return NULL;
    }

    if (return_to_value == NULL)
	var_value = rul__val_create_unnamed_mol_var (
                       rul__sem_register_temp_var (on_lhs));
    else 
	var_value = return_to_value;

    return rul__val_create_assignment (var_value, value);
}

