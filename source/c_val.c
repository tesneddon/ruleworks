/****************************************************************************
**                                                                         **
**                   C M P _ V A L _ V A L U E S . C                       **
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
 *	This module provides the compiler functions for 
 *	dealing with program values in a unified way.
 *
 * MODIFIED BY:
 *	DEC	Digital Equipment Corporation
 *	CPQ	Compaq Computer Corporation
 *
 * MODIFICATION HISTORY:
 *
 *	15-Jan-1993	DEC	Initial version
 *
 *	16-Aug-1993	DEC	Made unnamed_mol_var's honorary
 *					unnamed_ext_var's of type atom	
 *
 *	16-Feb-1998	DEC	class type changed to rclass
 *
 *	01-Dec-1999	CPQ	Release with GPL
 */


#ifndef NDEBUG
#define DONT_FREE_VALUES
#endif


#include <common.h>
#include <cmp_comm.h>
#include <cons.h>
#include <decl.h>
#include <ios.h>
#include <lvar.h>
#include <mol.h>
#include <net.h>
#include <val.h>
#include <val_p.h>
#include <sem.h>


/*
**	Forward declarations for all static local functions
*/
static Boolean		is_valid_value (Value value);
static Value		val_create_value (Value_Type typ, long subvalue_count);




/*********************
**                  **
**  IS_VALID_VALUE  **
**                  **
*********************/

static Boolean
is_valid_value (Value value)
{
#ifndef NDEBUG
	if (value == NULL) {
	    rul__ios_printf (RUL__C_STD_ERR,
		"\n Internal Error:  null value received.\n");
	    return (FALSE);
	} else if (! is_value (value)) {
	    rul__ios_printf (RUL__C_STD_ERR,
		"\n Internal Error:  non-value received (%p).\n", value);
	    return (FALSE);
	} else if (value->verification == (-1 * VAL__C_VALUE_VERIFY)) {
	    rul__ios_printf (RUL__C_STD_ERR,
		"\n Internal Error:  already freed value, ");
	    rul___val_print (value);
	    rul__ios_printf (RUL__C_STD_ERR, ", received.\n");
	    return (TRUE);  /* we'll be nice */
	}
#endif
	return (value != NULL  &&  
		is_value (value)
#ifndef NDEBUG
		&& value->verification == VAL__C_VALUE_VERIFY
#endif
		);
}




/******************************
**                           **
**  IS_VALID_OR_FREED_VALUE  **
**                           **
******************************/

static Boolean
is_valid_or_freed_value (Value value)
{
	return (value != NULL  &&  
		is_value (value)
#ifndef NDEBUG
		&& (value->verification == VAL__C_VALUE_VERIFY  ||
		   value->verification == (-1 * VAL__C_VALUE_VERIFY))
#endif
		);
}



/***********************
**                    **
**  VAL_CREATE_VALUE  **
**                    **
***********************/

static Value
val_create_value (Value_Type type, long subvalue_count)
{
	Value value;
	int size = 0;

	switch (type) {
		case VAL__E_POS :
			size = 	sizeof(struct position_value);
			break;
		case VAL__E_POS_ELEM :
			size = 	sizeof(struct position_elem_value);
			break;
		case VAL__E_RUL_CONST :
			size = 	sizeof(struct rul_constant_value);
			break;
		case VAL__E_ANIMAL_CONST :
			size = 	sizeof(struct animal_constant_value);
			break;
		case VAL__E_CLASS_CONST :
			size = 	sizeof(struct class_constant_value);
			break;
		case VAL__E_CONSTRUCT :
			size = 	sizeof(struct construct_value);
			break;
		case VAL__E_CONSTRUCT_TYPE :
			size = 	sizeof(struct construct_type_value);
			break;
		case VAL__E_LHS_VARIABLE :
			size = 	sizeof(struct lhs_variable_value);
			break;
		case VAL__E_BLK_OR_LOCAL :
			size = 	sizeof(struct blk_or_local_value);
			break;
		case VAL__E_RHS_VARIABLE :
			size = 	sizeof(struct rhs_variable_value);
			break;
		case VAL__E_UNM_MOL_VAR :
			size = 	sizeof(struct unnamed_mol_variable_value);
			break;
		case VAL__E_UNM_EXT_VAR :
			size = 	sizeof(struct unnamed_ext_variable_value);
			break;
		case VAL__E_NAM_EXT_VAR :
			size =  sizeof(struct named_ext_variable_value);
			break;
		case VAL__E_ARITH_EXPR :
			size = 	sizeof(struct arith_expr_value);
			break;
		case VAL__E_COND_EXPR :
			size = 	sizeof(struct cond_expr_value);
			break;
		case VAL__E_ASSIGNMENT :
			size=  sizeof(struct assignment_value);
			break;
		case VAL__E_FUNCTION :
	        case VAL__E_COMPLEX :
		case VAL__E_DISJUNCTION :
			assert (subvalue_count >= 0);
			size =	sizeof(struct complex_value) +
				(MAX(subvalue_count - 1, 0) * sizeof(Value));
			break;
		default :
			assert (BOGUS_VALUE_TYPE);
			return (NULL);
	}
	value = rul__mem_malloc (size);
	assert (value != NULL);

	value->type   = type;
	value->domain = dom_any;		/* no atom type known */
	value->shape  = shape_molecule;		/* no structure known */
	value->rclass  = NULL;			/* no class id  known */

#ifndef NDEBUG
	value->verification = VAL__C_VALUE_VERIFY;
#endif

	return (value);
}




/************************************
**                                 **
**  RUL__VAL_SET_SHAPE_AND_DOMAIN  **
**                                 **
************************************/

void
rul__val_set_shape_and_domain (Value value, 
		Decl_Shape shape, Decl_Domain domain)
{
	assert (is_valid_value (value));
	assert (shape == shape_molecule ||
		shape == shape_atom	||
		shape == shape_compound ||
		shape == shape_table);
	assert (domain == dom_any	||
		domain == dom_symbol	||
		domain == dom_instance_id ||
                domain == dom_opaque	||
                domain == dom_number	||
                domain == dom_int_atom	||
                domain == dom_dbl_atom);

	value->domain = domain;
	value->shape = shape;
	value->rclass  = NULL;  /* just incase of re-use */
}


/*************************
**                      **
**  RUL__VAL_SET_CLASS  **
**                      **
*************************/

void
rul__val_set_class (Value value, Class rclass)
{
	assert (is_valid_value (value));
	value->rclass = rclass;
}



/*************************
**                      **
**  RUL__VAL_GET_CLASS  **
**                      **
*************************/

Class
rul__val_get_class (Value value)
{
	assert (is_valid_value (value));
	return (value->rclass);
}



/*************************
**                      **
**  RUL__VAL_GET_SHAPE  **
**                      **
*************************/

Decl_Shape
rul__val_get_shape (Value value)
{
	assert (is_valid_value (value));
	return (value->shape);
}



/**************************
**                       **
**  RUL__VAL_GET_DOMAIN  **
**                       **
**************************/

Decl_Domain
rul__val_get_domain (Value value)
{
	assert (is_valid_value (value));
	return (value->domain);
}


/******************************
**                           **
**  RUL__VAL_IS_SIMPLE_EXPR  **
**                           **
******************************/

Boolean
rul__val_is_simple_expr (Value value)
{
	Position_Elem_Value pe_val;
	Complex_Value cx_val;
	long i;

	assert (is_valid_value (value));

	switch (value->type)
	    {
	    case VAL__E_RUL_CONST :
	    case VAL__E_POS :
	    case VAL__E_BLK_OR_LOCAL :
	    case VAL__E_LHS_VARIABLE :
	    case VAL__E_RHS_VARIABLE :
	    case VAL__E_ANIMAL_CONST :
	    case VAL__E_CLASS_CONST :
	    case VAL__E_CONSTRUCT :
	    case VAL__E_CONSTRUCT_TYPE :
		return TRUE;

	    case VAL__E_DISJUNCTION : 
		cx_val = (Complex_Value) value;
		if ((cx_val->next_value) || (cx_val->prev_value))
		    return FALSE;
		for (i=0; i<cx_val->subvalue_count; i++) 
		    {
		    if (! rul__val_is_simple_expr (cx_val->subvalues[i]))
			return FALSE;
		    }
		return TRUE;

	    case VAL__E_POS_ELEM :
		pe_val = (Position_Elem_Value) value;
		return rul__val_is_simple_expr (pe_val->index);

	    default:
		return FALSE;
	    }
}




/**************************
**                       **
**  RUL__VAL_EQUIVALENT  **
**                       **
**************************/

Boolean
rul__val_equivalent (Value val1, Value val2)
{
	Molecule  mol1,  mol2;
	Ext_Type  ext1,  ext2;
	char     *str1, *str2;
	long   	  i;

	if (val1 == NULL  &&  val2 == NULL) 	return (TRUE);
	if (val1 == NULL  ||  val2 == NULL) 	return (FALSE);

	assert (is_valid_value (val1));
	assert (is_valid_value (val2));

	if (val1->type   != val2->type) 	return (FALSE);
	if (val1->shape  != val2->shape) 	return (FALSE);

	switch (val1->type) {

	    case VAL__E_RUL_CONST :
		return (((Rul_Constant_Value)val1)->constant_value ==
			((Rul_Constant_Value)val2)->constant_value);
		break;

	    case VAL__E_CLASS_CONST :
		return (((Class_Constant_Value)val1)->class_value ==
		        ((Class_Constant_Value)val2)->class_value);
		break;

	    case VAL__E_POS_ELEM :
		if (! rul__val_equivalent (
				((Position_Elem_Value)val1)->index,
				((Position_Elem_Value)val2)->index)) {
		    return (FALSE);
		}
		/*  Note:  deliberately falls through to VAL__E_POS */

	    case VAL__E_POS :
		if  (((Position_Value)val1)->attr_name  !=
		     ((Position_Value)val2)->attr_name)     	return (FALSE);
		if  (((Position_Value)val1)->ce_index   != 
		     ((Position_Value)val2)->ce_index)      	return (FALSE);
		return (((Position_Value)val1)->attr_offset ==
			((Position_Value)val2)->attr_offset);
		break;

	    case VAL__E_LHS_VARIABLE :
		return (((LHS_Variable_Value)val1)->lhs_var_id_number  ==
			((LHS_Variable_Value)val2)->lhs_var_id_number);
		break;

	    case VAL__E_BLK_OR_LOCAL :
		if  (((Blk_Or_Local_Value)val1)->arg_name   !=
		     ((Blk_Or_Local_Value)val2)->arg_name)  	return (FALSE);
		if  (((Blk_Or_Local_Value)val1)->arg_type   != 
		     ((Blk_Or_Local_Value)val2)->arg_type)	return (FALSE);
		if  (((Blk_Or_Local_Value)val1)->arg_field  != 
		     ((Blk_Or_Local_Value)val2)->arg_field)     return (FALSE);
		if  (((Blk_Or_Local_Value)val1)->arg_index  != 
		     ((Blk_Or_Local_Value)val2)->arg_index)	return (FALSE);
		return (((Blk_Or_Local_Value)val1)->arg_indirect == 
			((Blk_Or_Local_Value)val2)->arg_indirect);
		break;

	    case VAL__E_ARITH_EXPR :
		if (((Arith_Expr_Value)val1)->arith_operator !=
		    ((Arith_Expr_Value)val2)->arith_operator)	return (FALSE);
		if (! rul__val_equivalent (
			((Arith_Expr_Value)val1)->operand_1,
			((Arith_Expr_Value)val2)->operand_1))	return (FALSE);
		return (rul__val_equivalent (
				((Arith_Expr_Value)val1)->operand_2,
				((Arith_Expr_Value)val2)->operand_2));
		break;

	    case VAL__E_COND_EXPR :
		if (((Cond_Expr_Value)val1)->cond_operator !=
		    ((Cond_Expr_Value)val2)->cond_operator)	return (FALSE);
		if (! rul__val_equivalent (
			((Cond_Expr_Value)val1)->operand_1,
			((Cond_Expr_Value)val2)->operand_1))	return (FALSE);
		return (rul__val_equivalent (
				((Cond_Expr_Value)val1)->operand_2,
				((Cond_Expr_Value)val2)->operand_2));
		break;

	    case VAL__E_FUNCTION :
		if (((Function_Value)val1)->function_name !=
		    ((Function_Value)val2)->function_name)	return (FALSE);
		if (((Function_Value)val1)->subvalue_count !=
		    ((Function_Value)val2)->subvalue_count)	return (FALSE);
		if (((Function_Value)val1)->prev_value != NULL)	return (FALSE);
		if (((Function_Value)val2)->prev_value != NULL)	return (FALSE);
		if (((Function_Value)val1)->next_value != NULL)	return (FALSE);
		if (((Function_Value)val2)->next_value != NULL)	return (FALSE);
		for (i=0; i<((Function_Value)val1)->subvalue_count; i++) {
		    if (! rul__val_equivalent (
				((Function_Value)val1)->subvalues[i],
				((Function_Value)val2)->subvalues[i])) {
			return (FALSE);
		    }
		}
		return (TRUE);
		break;

	    case VAL__E_ASSIGNMENT :
		return (rul__val_equivalent (
				((Assignment_Value)val1)->from_value,
				((Assignment_Value)val2)->from_value));
		break;

	    case VAL__E_ANIMAL_CONST :
		ext1 = ((Animal_Constant_Value)val1)->ext_type;
		ext2 = ((Animal_Constant_Value)val2)->ext_type;
		if (ext1 != ext2) return (FALSE);
		if (ext1 == ext_type_asciz) {
		    str1 = ((Animal_Constant_Value)val1)->val.asciz_val;
		    str2 = ((Animal_Constant_Value)val2)->val.asciz_val;
		    if (strcmp (str1, str2) == 0) return (TRUE);
		} else if (ext1 == ext_type_double    || 
			   ext1 == ext_type_float) {
		    return (((Animal_Constant_Value)val1)->val.dbl_val ==
		            ((Animal_Constant_Value)val2)->val.dbl_val);
		} else if (ext1 == ext_type_long      ||
			   ext1 == ext_type_short     ||
			   ext1 == ext_type_byte) {
		    return (((Animal_Constant_Value)val1)->val.sign_int_val ==
		    	    ((Animal_Constant_Value)val2)->val.sign_int_val);
		} else if (ext1 == ext_type_uns_long  || 
			   ext1 == ext_type_uns_short ||
			   ext1 == ext_type_uns_byte) {
		    return (((Animal_Constant_Value)val1)->val.unsign_int_val ==
		    	    ((Animal_Constant_Value)val2)->val.unsign_int_val);
		} else if (ext1 == ext_type_void_ptr) {
		    return (((Animal_Constant_Value)val1)->val.opaque_val ==
			    ((Animal_Constant_Value)val2)->val.opaque_val);
		}
		break;

	    case VAL__E_UNM_MOL_VAR :
	    case VAL__E_UNM_EXT_VAR :
	    case VAL__E_NAM_EXT_VAR :
		return (FALSE);
		break;

	    case VAL__E_CONSTRUCT :		/*  should never ask these */
	    case VAL__E_CONSTRUCT_TYPE :
	    case VAL__E_DISJUNCTION :
	    case VAL__E_RHS_VARIABLE :
		assert (FALSE);
		break;
	}
	return (FALSE);
}


/**************************
**                       **
**  RUL__VAL_COPY_VALUE  **
**                       **
**************************/

Value
rul__val_copy_value (Value old_val)
{
	Value new_val;
	Animal_Constant_Value old_anim, new_anim;
	Unnamed_Ext_Variable_Value old_ext, new_ext;
	Named_Ext_Variable_Value old_nam_ext, new_nam_ext;
	Arith_Expr_Value old_arith, new_arith;
	Cond_Expr_Value old_cond, new_cond;
	Complex_Value old_cmplx, new_cmplx;
	Assignment_Value old_asmt, new_asmt;
	long i;
	char *str;
	Molecule mol;
	Class    cls;
	Construct_Type crt_typ;

	if (old_val == NULL) return (NULL);

	assert (is_valid_value (old_val));

	/*	Create the new value body  */
	if (is_complex_val (old_val)) {
	    new_val = val_create_value (old_val->type,
				((Complex_Value)old_val)->subvalue_count);
	} else {
	    new_val = val_create_value (old_val->type, 0);
	}

	/*	Set the shared attributes */
	new_val->domain = old_val->domain;
	new_val->shape = old_val->shape;
	new_val->rclass = old_val->rclass;

	/*	Set the disjoint attributes  */
	switch (old_val->type) {

	    case VAL__E_RUL_CONST :
		mol = ((Rul_Constant_Value)old_val)->constant_value;
		((Rul_Constant_Value)new_val)->constant_value = mol;
		rul__mol_incr_uses (mol);
		break;

	    case VAL__E_ANIMAL_CONST :
		old_anim = (Animal_Constant_Value) old_val;
		new_anim = (Animal_Constant_Value) new_val;
		new_anim->ext_type = old_anim->ext_type;
		if (new_anim->ext_type == ext_type_asciz) {
		    str = old_anim->val.asciz_val;
		    new_anim->val.asciz_val = (char *)
			    rul__mem_malloc (strlen (str) + 1);
		    strcpy (new_anim->val.asciz_val, str);
		} else {
		    new_anim->val = old_anim->val;
		}
		break;

	    case VAL__E_CLASS_CONST :
		cls = ((Class_Constant_Value)old_val)->class_value;
		((Class_Constant_Value)new_val)->class_value = cls;
		break;

	    case VAL__E_CONSTRUCT :
		mol = ((Construct_Value)old_val)->construct_value;
		((Construct_Value)new_val)->construct_value = mol;
		rul__mol_incr_uses (mol);
		break;

	    case VAL__E_CONSTRUCT_TYPE :
		mol = ((Construct_Type_Value)old_val)->construct_value;
		((Construct_Type_Value)new_val)->construct_value = mol;
		crt_typ = ((Construct_Type_Value)old_val)->construct_type;
		((Construct_Type_Value)new_val)->construct_type = crt_typ;
		break;

	    case VAL__E_POS_ELEM :
		((Position_Elem_Value)new_val)->index =
			rul__val_copy_value (
				((Position_Elem_Value)old_val)->index);
		/*  Note:  deliberately falls through to VAL__E_POS */

	    case VAL__E_POS :
		mol = ((Position_Value)old_val)->attr_name;
		((Position_Value)new_val)->attr_name = mol;
		rul__mol_incr_uses (mol);
		((Position_Value)new_val)->ce_index = 
			((Position_Value)old_val)->ce_index;
		((Position_Value)new_val)->attr_offset = 
			((Position_Value)old_val)->attr_offset;
		break;

	    case VAL__E_LHS_VARIABLE :
		mol = ((LHS_Variable_Value)old_val)->variable_name;
		((LHS_Variable_Value)new_val)->variable_name = mol;
		rul__mol_incr_uses (mol);
		((LHS_Variable_Value)new_val)->lhs_var_id_number = 
			((LHS_Variable_Value)old_val)->lhs_var_id_number;
		break;

	    case VAL__E_BLK_OR_LOCAL :
		mol = ((Blk_Or_Local_Value)old_val)->arg_name;
		((Blk_Or_Local_Value)new_val)->arg_name = mol;
		rul__mol_incr_uses (mol);
		((Blk_Or_Local_Value)new_val)->arg_type = 
			((Blk_Or_Local_Value)old_val)->arg_type;
		((Blk_Or_Local_Value)new_val)->arg_field = 
			((Blk_Or_Local_Value)old_val)->arg_field;
		((Blk_Or_Local_Value)new_val)->arg_index = 
			((Blk_Or_Local_Value)old_val)->arg_index;
		((Blk_Or_Local_Value)new_val)->arg_indirect = 
			((Blk_Or_Local_Value)old_val)->arg_indirect;
		break;

	    case VAL__E_RHS_VARIABLE :
		mol = ((RHS_Variable_Value)old_val)->variable_name;
		((RHS_Variable_Value)new_val)->variable_name = mol;
		rul__mol_incr_uses (mol);
		((RHS_Variable_Value)new_val)->cache_index = 
			((RHS_Variable_Value)old_val)->cache_index;
		break;

	    case VAL__E_UNM_MOL_VAR :
		((Unnamed_Mol_Variable_Value)new_val)->variable_name = NULL;
		((Unnamed_Mol_Variable_Value)new_val)->cache_index =
			((Unnamed_Mol_Variable_Value)old_val)->cache_index;
		break;

	    case VAL__E_UNM_EXT_VAR :
		old_ext = (Unnamed_Ext_Variable_Value) old_val;
		new_ext = (Unnamed_Ext_Variable_Value) new_val;
		new_ext->variable_name = NULL;
		new_ext->cache_index = old_ext->cache_index;
		new_ext->cache_type = old_ext->cache_type;
		new_ext->array_length = old_ext->array_length;
		new_ext->array_index = old_ext->array_index;
		new_ext->passing_mech = old_ext->passing_mech;
		new_ext->memory_allocated = old_ext->memory_allocated;
		break;

	    case VAL__E_NAM_EXT_VAR :
		old_nam_ext = (Named_Ext_Variable_Value) old_val;
		new_nam_ext = (Named_Ext_Variable_Value) new_val;
		new_nam_ext->variable_name = old_nam_ext->variable_name;
		new_nam_ext->cache_type = old_nam_ext->cache_type;
		new_nam_ext->array_length = old_nam_ext->array_length;
		new_nam_ext->array_index = old_nam_ext->array_index;
		new_nam_ext->passing_mech = old_nam_ext->passing_mech;
		new_nam_ext->memory_allocated = old_nam_ext->memory_allocated;
		break;

	    case VAL__E_ARITH_EXPR :
		old_arith = (Arith_Expr_Value) old_val;
		new_arith = (Arith_Expr_Value) new_val;
		new_arith->arith_operator = old_arith->arith_operator;
		new_arith->operand_1 = 
			rul__val_copy_value (old_arith->operand_1);
		new_arith->operand_2 = 
			rul__val_copy_value (old_arith->operand_2);
		break;

	    case VAL__E_COND_EXPR :
		old_cond = (Cond_Expr_Value) old_val;
		new_cond = (Cond_Expr_Value) new_val;
		new_cond->cond_operator = old_cond->cond_operator;
		new_cond->operand_1 = 
			rul__val_copy_value (old_cond->operand_1);
		new_cond->operand_2 = 
			rul__val_copy_value (old_cond->operand_2);
		break;

	    case VAL__E_FUNCTION :
		mol = ((Function_Value)old_val)->function_name;
		((Function_Value)new_val)->function_name = mol;
		rul__mol_incr_uses (mol);
		/*  Note:  deliberately falls through to VAL__E_DISJUNCTION */

	    case VAL__E_COMPLEX :
	    case VAL__E_DISJUNCTION :
		old_cmplx = (Complex_Value) old_val;
		new_cmplx = (Complex_Value) new_val;
		new_cmplx->subvalue_count = old_cmplx->subvalue_count;
		new_cmplx->next_value = 
			rul__val_copy_value (old_cmplx->next_value);
		new_cmplx->prev_value = 
			rul__val_copy_value (old_cmplx->prev_value);
		new_cmplx->passing_mech = old_cmplx->passing_mech;
		for (i=0; i<new_cmplx->subvalue_count; i++) {
		    new_cmplx->subvalues[i] = 
			    rul__val_copy_value (old_cmplx->subvalues[i]);
		}
		break;

	    case VAL__E_ASSIGNMENT :
		old_asmt = (Assignment_Value) old_val;
		new_asmt = (Assignment_Value) new_val;
		new_asmt->to_variable =
			rul__val_copy_value (old_asmt->to_variable);
		new_asmt->from_value =
			rul__val_copy_value (old_asmt->from_value);
		new_asmt->next_value = 
			rul__val_copy_value (old_asmt->next_value);
		new_asmt->prev_value = 
			rul__val_copy_value (old_asmt->prev_value);
		break;
	}
	return (new_val);
}



/**************************
**                       **
**  RUL__VAL_FREE_VALUE  **
**                       **
**************************/

void
rul__val_free_value (Value value)
{
	long i;
	Position_Elem_Value pe_val;
	Complex_Value cx_val;
	Arith_Expr_Value ax_val;
	Cond_Expr_Value cd_val;
	Assignment_Value as_val;
	Animal_Constant_Value an_val;
	
	if (value == NULL) return;

	assert (is_value (value));

#ifndef NDEBUG
	if (value->verification == (-1 * VAL__C_VALUE_VERIFY)) {
	    rul__ios_printf (RUL__C_STD_ERR,
		"\n Internal Error:  Attempt to free already freed value, ");
	    rul___val_print (value);
	    rul__ios_printf (RUL__C_STD_ERR, "\n");
	    return;
	}
#endif
	assert (is_valid_value (value));

	switch (value->type) {
	    case VAL__E_RUL_CONST :
		    rul__mol_decr_uses (
			((Rul_Constant_Value)value)->constant_value);
		    break;

	    case VAL__E_POS :
		    rul__mol_decr_uses (((Position_Value)value)->attr_name);
		    break;

	    case VAL__E_BLK_OR_LOCAL :
		    rul__mol_decr_uses (((Blk_Or_Local_Value)value)->arg_name);
		    break;

	    case VAL__E_POS_ELEM :
		    pe_val = (Position_Elem_Value) value;
		    rul__val_free_value (pe_val->index);
		    rul__mol_decr_uses (pe_val->attr_name);
		    break;

	    case VAL__E_ARITH_EXPR :
		    ax_val = (Arith_Expr_Value) value;
		    rul__val_free_value (ax_val->operand_1);
		    if (ax_val->operand_2) {
			rul__val_free_value (ax_val->operand_2);
		    }
		    break;

	    case VAL__E_COND_EXPR :
		    cd_val = (Cond_Expr_Value) value;
		    rul__val_free_value (cd_val->operand_1);
		    if (cd_val->operand_2) {
			rul__val_free_value (cd_val->operand_2);
		    }
		    break;

	    case VAL__E_LHS_VARIABLE :
	    case VAL__E_RHS_VARIABLE :
	    case VAL__E_NAM_EXT_VAR  :
		    if (((Variable_Value)value)->variable_name) {
			rul__mol_decr_uses (
				((Variable_Value)value)->variable_name);
		    }
		    break;

	    case VAL__E_ASSIGNMENT :
		    as_val = (Assignment_Value) value;
		    rul__val_free_value (as_val->to_variable);
		    rul__val_free_value (as_val->from_value);
		    rul__val_free_value (as_val->next_value);
		    rul__val_free_value (as_val->prev_value);
		    break;

	    case VAL__E_FUNCTION :
	    case VAL__E_COMPLEX :
	    case VAL__E_DISJUNCTION : 
		    cx_val = (Complex_Value) value;
		    if (cx_val->function_name) {
		    	rul__mol_decr_uses (cx_val->function_name);
		    }
		    for (i=0; i<cx_val->subvalue_count; i++) {
			rul__val_free_value (cx_val->subvalues[i]);
		    }
		    rul__val_free_value (cx_val->next_value);
		    rul__val_free_value (cx_val->prev_value);
		    break;

	    case VAL__E_ANIMAL_CONST :
		    an_val = (Animal_Constant_Value) value;
		    if (an_val->ext_type == ext_type_asciz)
#ifdef DONT_FREE_VALUES
			rul__mem_free (an_val->val.asciz_val);
#endif
		    break;

	    case VAL__E_CLASS_CONST :
	    case VAL__E_CONSTRUCT :
	    case VAL__E_CONSTRUCT_TYPE :
	            break;
	}

#ifndef NDEBUG
	value->verification = -1 * VAL__C_VALUE_VERIFY;
#endif
#ifndef DONT_FREE_VALUES
        value->type = VAL__E_INVALID;
	rul__mem_free (value);
#endif
}




/*************************************
**                                  **
**  RUL__VAL_REGISTER_LHS_TMP_VARS  **
**                                  **
*************************************/

void
rul__val_register_lhs_tmp_vars (Value value)
{
	long i;

	Unnamed_Mol_Variable_Value	ml_val;
	Unnamed_Ext_Variable_Value	ex_val;
	Complex_Value			cx_val;
	Arith_Expr_Value		ax_val;
	Cond_Expr_Value			cd_val;
	Assignment_Value		as_val;
	Animal_Constant_Value		an_val;
	
	if (value == NULL) return;

	assert (is_valid_value (value));

	switch (value->type) {
	    case VAL__E_POS :
	    case VAL__E_POS_ELEM :
	    case VAL__E_LHS_VARIABLE :
	    case VAL__E_RHS_VARIABLE :
	    case VAL__E_BLK_OR_LOCAL :
	    case VAL__E_RUL_CONST :
	    case VAL__E_ANIMAL_CONST :
	    case VAL__E_CLASS_CONST :
	    case VAL__E_CONSTRUCT :
	    case VAL__E_CONSTRUCT_TYPE :
		    break;

	    case VAL__E_UNM_MOL_VAR :
		    ml_val = (Unnamed_Mol_Variable_Value) value;
		    rul__sem_add_lhs_tmp_var (ext_type_atom,
					      EXT__C_NOT_ARRAY,
					      ext_mech_value,
					      ml_val->cache_index);
		    break;

	    case VAL__E_UNM_EXT_VAR :
		    ex_val = (Unnamed_Ext_Variable_Value) value;
		    rul__sem_add_lhs_tmp_var (ex_val->cache_type,
					      ex_val->array_length,
					      ex_val->passing_mech,
					      ex_val->cache_index);
		    break;

	    case VAL__E_ARITH_EXPR :
		    ax_val = (Arith_Expr_Value) value;
		    rul__val_register_lhs_tmp_vars (ax_val->operand_1);
		    if (ax_val->operand_2) {
			rul__val_register_lhs_tmp_vars (ax_val->operand_2);
		    }
		    break;

	    case VAL__E_COND_EXPR :
		    cd_val = (Cond_Expr_Value) value;
		    rul__val_register_lhs_tmp_vars (cd_val->operand_1);
		    if (cd_val->operand_2) {
			rul__val_register_lhs_tmp_vars (cd_val->operand_2);
		    }
		    break;

	    case VAL__E_ASSIGNMENT :
		    as_val = (Assignment_Value) value;
		    rul__val_register_lhs_tmp_vars (as_val->to_variable);
		    rul__val_register_lhs_tmp_vars (as_val->from_value);
		    rul__val_register_lhs_tmp_vars (as_val->next_value);
		    rul__val_register_lhs_tmp_vars (as_val->prev_value);
		    break;

	    case VAL__E_FUNCTION :
	    case VAL__E_COMPLEX :
	    case VAL__E_DISJUNCTION : 
		    cx_val = (Complex_Value) value;
		    for (i=0; i<cx_val->subvalue_count; i++) {
			rul__val_register_lhs_tmp_vars (cx_val->subvalues[i]);
		    }
		    rul__val_register_lhs_tmp_vars (cx_val->next_value);
		    rul__val_register_lhs_tmp_vars (cx_val->prev_value);
		    break;
	}
}


 /* RUL constants */

/*******************************
**                            **
**  RUL__VAL_IS_RUL_CONSTANT  **
**                            **
*******************************/

Boolean
rul__val_is_rul_constant (Value value)
{
	assert (is_valid_value (value));
	return (is_rul_constant_val  (value));
}


/***********************************
**                                **
**  RUL__VAL_CREATE_RUL_CONSTANT  **
**                                **
***********************************/

Value
rul__val_create_rul_constant (Molecule con)
{
	Rul_Constant_Value c_val;

	assert (rul__mol_is_valid (con));

	c_val = (Rul_Constant_Value) 
			val_create_value (VAL__E_RUL_CONST, 0);
	c_val->constant_value = con;
	rul__mol_incr_uses (con);
	(void) rul__cons_create_cons (con);

	/*  Set the domain and shape to match the constant */
	rul__val_set_shape_and_domain ((Value) c_val,
				       rul__mol_get_shape (con),
				       rul__mol_get_domain (con));

	return ((Value) c_val);
}



 /* Animal Constants */


/*******************************
**                            **
**  RUL__VAL_IS_ANIMAL_CONST  **
**                            **
*******************************/

Boolean
rul__val_is_animal_const (Value value)
{
	assert (is_valid_value (value));
        return (is_animal_constant_val (value));
}


/***********************************
**                                **
**  RUL__VAL_CREATE_LONG_ANIMAL   **
**                                **
***********************************/

Value
rul__val_create_long_animal (long long_val)
{
	Animal_Constant_Value a_val;

	a_val = (Animal_Constant_Value)
			val_create_value (VAL__E_ANIMAL_CONST, 0);

	a_val->ext_type = ext_type_long;
	a_val->val.sign_int_val = long_val;

	return ((Value) a_val);
}


/**************************************
**                                   **
**  RUL__VAL_CREATE_UNS_LONG_ANIMAL  **
**                                   **
**************************************/

Value
rul__val_create_uns_long_animal (unsigned long uns_long_val)
{
	Animal_Constant_Value a_val;

	a_val = (Animal_Constant_Value)
			val_create_value (VAL__E_ANIMAL_CONST, 0);

	a_val->ext_type = ext_type_uns_long;
	a_val->val.unsign_int_val = uns_long_val;

	return ((Value) a_val);
}



/***********************************
**                                **
**  RUL__VAL_CREATE_ASCIZ_ANIMAL  **
**                                **
***********************************/

Value
rul__val_create_asciz_animal (char *val)
{
	Animal_Constant_Value a_val;

	a_val = (Animal_Constant_Value)
			val_create_value (VAL__E_ANIMAL_CONST, 0);

	a_val->ext_type = ext_type_asciz;
	a_val->val.asciz_val = (char *) rul__mem_malloc (strlen (val) + 1);
	strcpy (a_val->val.asciz_val, val);

	return ((Value) a_val);
}


/***********************************
**                                **
**  RUL__VAL_CREATE_DOUBLE_ANIMAL **
**                                **
***********************************/

Value
rul__val_create_double_animal (double val)
{
	Animal_Constant_Value a_val;

	a_val = (Animal_Constant_Value)
			val_create_value (VAL__E_ANIMAL_CONST, 0);

	a_val->ext_type = ext_type_double;
	a_val->val.dbl_val = val;

	return ((Value) a_val);
}



/***********************************
**                                **
**  RUL__VAL_GET_ANIMAL_TYPE      **
**                                **
***********************************/

Ext_Type
rul__val_get_animal_type (Value val)
{
	Animal_Constant_Value an_val;

	assert (is_animal_constant_val (val));

	an_val = (Animal_Constant_Value) val;

	return an_val->ext_type;
}

/***********************************
**                                **
**  RUL__VAL_GET_ANIMAL_LONG      **
**                                **
***********************************/

long
rul__val_get_animal_long (Value val)
{
	Animal_Constant_Value an_val;

        assert (is_animal_constant_val (val));

	an_val = (Animal_Constant_Value) val;

        return an_val->val.sign_int_val;
}

/***********************************
**                                **
**  RUL__VAL_GET_ANIMAL_ASCIZ     **
**                                **
***********************************/

char *
rul__val_get_animal_asciz (Value val)
{
	Animal_Constant_Value an_val;

        assert (is_animal_constant_val (val));

	an_val = (Animal_Constant_Value) val;

        return an_val->val.asciz_val;
}

/***********************************
**                                **
**  RUL__VAL_GET_ANIMAL_DOUBLE    **
**                                **
***********************************/

double
rul__val_get_animal_double (Value val)
{
	Animal_Constant_Value an_val;

        assert (is_animal_constant_val (val));

	an_val = (Animal_Constant_Value) val;

        return an_val->val.dbl_val;
}



 /* RUL classes */

/*********************************
**                              **
**  RUL__VAL_IS_CLASS_CONSTANT  **
**                              **
*********************************/

Boolean
rul__val_is_class_constant (Value value)
{
	assert (is_valid_value (value));
	return (is_class_constant_val  (value));
}


/*************************************
**                                  **
**  RUL__VAL_CREATE_CLASS_CONSTANT  **
**                                  **
*************************************/

Value
rul__val_create_class_constant (Class class_id)
{
	Class_Constant_Value c_val;

	/* ??? assert (rul__??? (class_id));  */

	c_val = (Class_Constant_Value) 
			val_create_value (VAL__E_CLASS_CONST, 0);
	c_val->class_value = class_id;
	rul__class_create_class (class_id);

	return ((Value) c_val);
}



 /* RUL constructs and types */

/****************************
**                         **
**  RUL__VAL_IS_CONSTRUCT  **
**                         **
****************************/

Boolean
rul__val_is_construct (Value value)
{
	assert (is_valid_value (value));
	return (is_construct_val  (value));
}


/********************************
**                             **
**  RUL__VAL_CREATE_CONSTRUCT  **
**                             **
********************************/

Value
rul__val_create_construct (Molecule con)
{
	Construct_Value c_val;

	assert (rul__mol_is_valid (con));

	c_val = (Construct_Value) val_create_value (VAL__E_CONSTRUCT, 0);
	c_val->construct_value = con;

	return ((Value) c_val);
}


/*********************************
**                              **
**  RUL__VAL_IS_CONSTRUCT_TYPE  **
**                              **
*********************************/

Boolean
rul__val_is_construct_type (Value value)
{
	assert (is_valid_value (value));
	return (is_construct_type_val  (value));
}


/*************************************
**                                  **
**  RUL__VAL_CREATE_CONSTRUCT_TYPE  **
**                                  **
*************************************/

Value
rul__val_create_construct_type (Mol_Symbol name, Construct_Type ctyp)
{
	Construct_Type_Value c_val;

	assert (rul__mol_is_valid (name));
	/*? assume correct ??? assert (rul__mol_is_valid (ctyp)); */

	c_val = (Construct_Type_Value) 
		        val_create_value (VAL__E_CONSTRUCT_TYPE, 0);
	c_val->construct_value = name;
	c_val->construct_type = ctyp;

	return ((Value) c_val);
}



 /* Position Values */





/***************************
**                        **
**  RUL__VAL_IS_POSITION  **
**                        **
***************************/

Boolean
rul__val_is_position (Value val)
{
	assert (is_valid_value (val));
	return (is_position_val (val));
}



/********************************
**                             **
**  RUL__VAL_IS_POSITION_ELEM  **
**                             **
********************************/

Boolean
rul__val_is_position_elem (Value val)
{
	assert (is_valid_value (val));
	return (is_position_elem_val (val));
}



/********************************
**                             **
**  RUL__VAL_GET_POS_CE_INDEX  **
**                             **
********************************/

long
rul__val_get_pos_ce_index (Value val)
{
	assert (is_valid_value (val));
	assert (is_position_val (val));

	return (((Position_Value)val)->ce_index);
}



/***********************************
**                                **
**  RUL__VAL_GET_POS_ATTR_OFFSET  **
**                                **
***********************************/

long
rul__val_get_pos_attr_offset (Value val)
{
	assert (is_valid_value (val));
	assert (is_position_val (val));

	return (((Position_Value)val)->attr_offset);
}



/**********************************
**                               **
**  RUL__VAL_GET_POS_ELEM_INDEX  **
**                               **
**********************************/

Value
rul__val_get_pos_elem_index (Value val)
{
	assert (is_valid_value (val));
	assert (is_position_elem_val (val));

	return (((Position_Elem_Value)val)->index);
}




/*******************************
**                            **
**  RUL__VAL_CREATE_POSITION  **
**                            **
*******************************/

Value
rul__val_create_position (Class ce_class, Mol_Symbol attr_name, long ce_index)
{
	Position_Value p_val;

	p_val = (Position_Value) val_create_value (VAL__E_POS, 0);

	p_val->ce_index = ce_index;
	p_val->attr_name = attr_name;
	rul__mol_incr_uses (attr_name);
	p_val->attr_offset = rul__decl_get_attr_offset (ce_class, attr_name);

	return ((Value) p_val);
}





/************************************
**                                 **
**  RUL__VAL_CREATE_POSITION_ELEM  **
**                                 **
************************************/

Value
rul__val_create_position_elem (
		Class ce_class, Mol_Symbol attr_name, Value index_val,
		long ce_index)
{
	Position_Elem_Value p_val;

	assert (is_valid_value (index_val));

	p_val = (Position_Elem_Value) 
			val_create_value (VAL__E_POS_ELEM, 0);

	p_val->ce_index = ce_index;
	p_val->attr_name = attr_name;
	rul__mol_incr_uses (attr_name);
	p_val->attr_offset = rul__decl_get_attr_offset (ce_class, attr_name);
	p_val->index = index_val;

	return ((Value) p_val);
}


 /* LHS Variables */


/*******************************
**                            **
**  RUL__VAL_IS_LHS_VARIABLE  **
**                            **
*******************************/

Boolean
rul__val_is_lhs_variable (Value val)
{
        assert (is_valid_value (val));
        return (is_lhs_var_val (val));
}




/***********************************
**                                **
**  RUL__VAL_GET_LHS_VARIABLE_ID  **
**                                **
***********************************/

long
rul__val_get_lhs_variable_id (Value val)
{
        assert (is_valid_value (val));
        assert (is_lhs_var_val (val));
	return (((LHS_Variable_Value)val)->lhs_var_id_number);
}



/*************************************
**                                  **
**  RUL__VAL_SET_LHS_VARIABLE_NAME  **
**                                  **
*************************************/

void rul__val_set_lhs_variable_name (Value val, Mol_Symbol variable_name)
{
   LHS_Variable_Value v_val = (LHS_Variable_Value) val;

   assert (is_valid_value (val));
   assert (is_lhs_var_val (val));
   assert (variable_name == NULL);

   v_val->variable_name = variable_name;
   if (variable_name)
     rul__mol_incr_uses (variable_name);
}



/***********************************
**                                **
**  RUL__VAL_CREATE_LHS_VARIABLE  **
**                                **
***********************************/

Value
rul__val_create_lhs_variable (Mol_Symbol variable_name, long var_id)
{
	LHS_Variable_Value v_val;

	v_val = (LHS_Variable_Value) 
		val_create_value (VAL__E_LHS_VARIABLE, 0);

	v_val->lhs_var_id_number = var_id;

	v_val->variable_name = variable_name;
	if (variable_name) rul__mol_incr_uses (variable_name);

	return ((Value) v_val);
}


/*  Generated block variables, or function stack or argument values  */



/*********************************
**                              **
**  RUL__VAL_CREATE_BLK_OR_LOC  **
**                              **
*********************************/

Value
rul__val_create_blk_or_loc (Rul_Internal_Type arg_type, char *arg_name)
{
	Blk_Or_Local_Value l_val;

	l_val = (Blk_Or_Local_Value) 
		val_create_value (VAL__E_BLK_OR_LOCAL, 0);

	l_val->variable_name = NULL;
	l_val->arg_type = arg_type;
	l_val->arg_name = rul__mol_make_symbol (arg_name);
	l_val->arg_field = CMP__E_FIELD__NONE;
	l_val->arg_index = VAL__C_NOT_AN_INDEX;
	l_val->arg_indirect = FALSE;

	return ((Value) l_val);
}




/**************************************
**                                   **
**  RUL__VAL_CREATE_BLK_OR_LOC_ADDR  **
**                                   **
**************************************/

Value
rul__val_create_blk_or_loc_addr (Rul_Internal_Type arg_type, char *arg_name)
{
	Blk_Or_Local_Value l_val;

	l_val = (Blk_Or_Local_Value) 
		val_create_value (VAL__E_BLK_OR_LOCAL, 0);

	l_val->variable_name = NULL;
	l_val->arg_type = arg_type;
	l_val->arg_name = rul__mol_make_symbol (arg_name);
	l_val->arg_field = CMP__E_FIELD__NONE;
	l_val->arg_index = VAL__C_NOT_AN_INDEX;
	l_val->arg_indirect = TRUE;

	return ((Value) l_val);
}




/*************************************
**                                  **
**  RUL__VAL_CREATE_BLK_OR_LOC_VEC  **
**                                  **
*************************************/

Value
rul__val_create_blk_or_loc_vec (
		Rul_Internal_Type arg_type, char *arg_name,
		long array_index)
{
	Blk_Or_Local_Value l_val;

	l_val = (Blk_Or_Local_Value) 
		val_create_value (VAL__E_BLK_OR_LOCAL, 0);

	l_val->variable_name = NULL;
	l_val->arg_type = arg_type;
	l_val->arg_name = rul__mol_make_symbol (arg_name);
	l_val->arg_field = CMP__E_FIELD__NONE;
	l_val->arg_index = array_index;
	l_val->arg_indirect = FALSE;

	return ((Value) l_val);
}




/*************************************
**                                  **
**  RUL__VAL_CREATE_BLK_OR_LOC_FLD  **
**                                  **
*************************************/

Value
rul__val_create_blk_or_loc_fld (
		Rul_Internal_Type arg_type, char *arg_name,
		Rul_Internal_Field field)
{
	Blk_Or_Local_Value l_val;

	/*?*  should verify that this field is valid for this type */

	l_val = (Blk_Or_Local_Value) 
		val_create_value (VAL__E_BLK_OR_LOCAL, 0);

	l_val->variable_name = NULL;
	l_val->arg_type = arg_type;
	l_val->arg_name = rul__mol_make_symbol (arg_name);
	l_val->arg_field = field;
	l_val->arg_index = VAL__C_NOT_AN_INDEX;
	l_val->arg_indirect = FALSE;

	return ((Value) l_val);
}



/**************************************
**                                   **
**  RUL__VAL_CREATE_BLK_LOC_FLD_VEC  **
**                                   **
**************************************/

Value
rul__val_create_blk_loc_fld_vec (
		Rul_Internal_Type arg_type, char *arg_name,
		Rul_Internal_Field field, long array_index)
{
	Blk_Or_Local_Value l_val;

	/*?*  should verify that this field is valid for this type */

	l_val = (Blk_Or_Local_Value) 
		val_create_value (VAL__E_BLK_OR_LOCAL, 0);

	l_val->variable_name = NULL;
	l_val->arg_type = arg_type;
	l_val->arg_name = rul__mol_make_symbol (arg_name);
	l_val->arg_field = field;
	l_val->arg_index = array_index;
	l_val->arg_indirect = FALSE;

	return ((Value) l_val);
}




/***************************
**                        **
**  RUL__VAL_CREATE_NULL  **
**                        **
***************************/

Value
rul__val_create_null (Rul_Internal_Type arg_type)
{
	/*
	**  When emitting C code, create a NULL by faking
	**  a reference to a stack variable named NULL.
	*/
	return (rul__val_create_blk_or_loc (arg_type, "NULL"));
}



 
/* Named RHS variables (containing molecules) */

/*******************************
**                            **
**  RUL__VAL_IS_RHS_VARIABLE  **
**                            **
*******************************/

Boolean
rul__val_is_rhs_variable (Value val)
{
        assert (is_valid_value (val));
        return (is_rhs_var_val (val));
}


/***********************************
**                                **
**  RUL__VAL_CREATE_RHS_VARIABLE  **
**                                **
***********************************/

Value
rul__val_create_rhs_variable (Mol_Symbol variable_name)
{
	RHS_Variable_Value v_val;

	v_val = (RHS_Variable_Value) 
		val_create_value (VAL__E_RHS_VARIABLE, 0);

	v_val->variable_name = variable_name;
	rul__mol_incr_uses (variable_name);

	v_val->cache_index = VAL__C_NOT_AN_INDEX;

	return ((Value) v_val);
}


/***********************************
**                                **
**  RUL__VAL_SET_RHS_VAR_INDEX    **
**                                **
***********************************/

void
rul__val_set_rhs_var_index (Value value, long index)
{
    RHS_Variable_Value v_val;

        v_val = (RHS_Variable_Value) value;
	v_val->cache_index = index;
}


 /* Unnamed molecule variables */


/*******************************
**                            **
**  RUL__VAL_IS_MOL_VARIABLE  **
**                            **
*******************************/

Boolean
rul__val_is_mol_variable (Value val)
{
        assert (is_valid_value (val));
        return (is_unnamed_mol_var_val (val));
}


/**************************************
**                                   **
**  RUL__VAL_CREATE_UNNAMED_MOL_VAR  **
**                                   **
**************************************/

Value
rul__val_create_unnamed_mol_var (long cache_index)
{
	Unnamed_Mol_Variable_Value v_val;

	v_val = (Unnamed_Mol_Variable_Value) 
		val_create_value (VAL__E_UNM_MOL_VAR, 0);

	v_val->cache_index = cache_index;
	v_val->variable_name = NULL;

	return ((Value) v_val);
}





/************************************
**                                 **
**  RUL__VAL_FOR_EACH_UNNAMED_MOL  **
**                                 **
************************************/

void 
rul__val_for_each_unnamed_mol (Value value, 
			       void (*function_ptr)(Value unm_var_val))
	/*
	**  Recursively traverses a value and call the specified
	**  function for each unnamed mol variable encountered.
	*/
{
	long          i;
	Assignment_Value as_val;
	Complex_Value cx_val;

	if (value == NULL) return;
	assert (is_valid_value (value));

	switch (value->type) {

	    case VAL__E_UNM_MOL_VAR :
		    (*function_ptr) (value);
		    break;

	    case VAL__E_ARITH_EXPR :
		    rul__val_for_each_unnamed_mol (
				((Arith_Expr_Value)value)->operand_1,
				function_ptr);
		    if (((Arith_Expr_Value)value)->operand_2 != NULL) {
			rul__val_for_each_unnamed_mol (
				((Arith_Expr_Value)value)->operand_2,
				function_ptr);
		    }
		    break;

	    case VAL__E_COND_EXPR :
		    rul__val_for_each_unnamed_mol (
				((Cond_Expr_Value)value)->operand_1,
				function_ptr);
		    if (((Cond_Expr_Value)value)->operand_2 != NULL) {
			rul__val_for_each_unnamed_mol (
				((Cond_Expr_Value)value)->operand_2,
				function_ptr);
		    }
		    break;

	    case VAL__E_ASSIGNMENT :
		    as_val = (Assignment_Value) value;
		    rul__val_for_each_unnamed_mol (as_val->prev_value,
				function_ptr);
		    rul__val_for_each_unnamed_mol (as_val->to_variable,
				function_ptr);
		    rul__val_for_each_unnamed_mol (as_val->from_value,
				function_ptr);
		    rul__val_for_each_unnamed_mol (as_val->next_value,
				function_ptr);
		    break;

	    case VAL__E_FUNCTION :
	    case VAL__E_COMPLEX :
	    case VAL__E_DISJUNCTION :
		    cx_val = (Complex_Value) value;
		    rul__val_for_each_unnamed_mol (cx_val->prev_value,
				function_ptr);
		    for (i=0; i<cx_val->subvalue_count; i++) {
			rul__val_for_each_unnamed_mol (cx_val->subvalues[i],
				function_ptr);
		    }
		    rul__val_for_each_unnamed_mol (cx_val->next_value,
				function_ptr);
		    break;

	    default : /* constants */
		    break;
	}
}
 

 /* External temp variables */

/*******************************
**                            **
**  RUL__VAL_IS_EXT_VARIABLE  **
**                            **
*******************************/

Boolean
rul__val_is_ext_variable (Value value)
{
        assert (is_valid_value (value));

        return (is_unnamed_ext_var_val (value)	|| 
		is_named_ext_var_val(value)	||
		is_unnamed_mol_var_val(value));
}


/**************************************
**                                   **
**  RUL__VAL_CREATE_UNNAMED_EXT_VAR  **
**                                   **
**************************************/

Value
rul__val_create_unnamed_ext_var (long cache_index,
					 Ext_Type type,
					 Cardinality array_len,
					 Ext_Mech mechanism,
					 Boolean memory_allocated)
{
	Unnamed_Ext_Variable_Value v_val;

	v_val = (Unnamed_Ext_Variable_Value) 
		val_create_value (VAL__E_UNM_EXT_VAR, 0);

	v_val->variable_name = NULL;
	v_val->cache_index = cache_index;
	v_val->cache_type = type;
	v_val->array_length = array_len;
	v_val->array_index = VAL__C_NOT_AN_INDEX;
	v_val->passing_mech = mechanism;
	v_val->memory_allocated = memory_allocated;

	return ((Value) v_val);
}

/**************************************
**                                   **
**  RUL__VAL_CREATE_NAMED_EXT_VAR    **
**                                   **
**************************************/

Value
rul__val_create_named_ext_var (Mol_Symbol var_name,
				       Ext_Type type,
				       Cardinality array_len,
				       Ext_Mech mechanism,
				       Boolean memory_allocated)
{
	Named_Ext_Variable_Value v_val;

	v_val = (Named_Ext_Variable_Value) 
		val_create_value (VAL__E_NAM_EXT_VAR, 0);

	v_val->variable_name = var_name;
	v_val->cache_type = type;
	v_val->array_length = array_len;
	v_val->array_index = VAL__C_NOT_AN_INDEX;
	v_val->passing_mech = mechanism;
	v_val->memory_allocated = memory_allocated;

	return ((Value) v_val);
}



/********************************
**                             **
**  RUL__VAL_GET_EXT_VAR_TYPE  **
**                             **
********************************/

Ext_Type
rul__val_get_ext_var_type (Value value)
{
	register Ext_Type type;

	assert (rul__val_is_ext_variable (value));

	if (is_unnamed_mol_var_val(value)) return ext_type_atom;

	if (value->type == VAL__E_UNM_EXT_VAR)
	  type = ((Unnamed_Ext_Variable_Value)value)->cache_type;
	else /* if (value->type == VAL__E_NAM_EXT_VAR) */
	  type = ((Named_Ext_Variable_Value)value)->cache_type;

	return type;
}

/***********************************
**                                **
**  RUL__VAL_GET_EXT_VAR_ARR_LEN  **
**                                **
***********************************/

Cardinality
rul__val_get_ext_var_arr_len (Value value)
{
	register Cardinality arr_len;

	assert (rul__val_is_ext_variable (value));

	if (is_unnamed_mol_var_val(value)) return 0;

	if (value->type == VAL__E_UNM_EXT_VAR)
	  arr_len = ((Unnamed_Ext_Variable_Value)value)->array_length;
	else /* if (value->type == VAL__E_NAM_EXT_VAR) */
	  arr_len = ((Named_Ext_Variable_Value)value)->array_length;

	return arr_len;
}

/***********************************
**                                **
**  RUL__VAL_GET_EXT_VAR_INDEX    **
**                                **
***********************************/

long
rul__val_get_ext_var_index (Value value)
{
	Unnamed_Ext_Variable_Value ext_var_val;

	assert (rul__val_is_ext_variable (value)  &&
		!is_unnamed_mol_var_val(value));

	ext_var_val =  (Unnamed_Ext_Variable_Value) value;

	return ext_var_val->array_index;
}

/********************************
**                             **
**  RUL__VAL_SET_EXT_VAR_INDEX **
**                             **
********************************/

void
rul__val_set_ext_var_index (Value value, long index)
{
	Unnamed_Ext_Variable_Value ext_var_val;

	assert (rul__val_is_ext_variable (value)  &&
		!is_unnamed_mol_var_val(value));

	ext_var_val =  (Unnamed_Ext_Variable_Value) value;

	ext_var_val->array_index = index;
}

/********************************
**                             **
**  RUL__VAL_GET_EXT_VAR_MECH  **
**                             **
********************************/

Ext_Mech
rul__val_get_ext_var_mech (Value value)
{
	Unnamed_Ext_Variable_Value ext_var_val;

	assert (rul__val_is_ext_variable (value));

	if (is_unnamed_mol_var_val(value)) return ext_mech_value;

	ext_var_val =  (Unnamed_Ext_Variable_Value) value;

	return ext_var_val->passing_mech;
}

/********************************
**                             **
**  RUL__VAL_SET_EXT_VAR_MECH  **
**                             **
********************************/

void
rul__val_set_ext_var_mech (Value value, Ext_Mech mech)
{
	Unnamed_Ext_Variable_Value ext_var_val;

	assert (rul__val_is_ext_variable (value));

	if (is_unnamed_mol_var_val(value)) {
		assert (mech == ext_mech_value);
		return; /* do nothing */
	}
	
	ext_var_val =  (Unnamed_Ext_Variable_Value) value;

	ext_var_val->passing_mech = mech;
}

/**************************************
**                                   **
**  RUL__VAL_GET_EXT_VAR_MEM_DEALOC  **
**                                   **
**************************************/

Boolean
rul__val_get_ext_var_mem_dealoc (Value value)
{
	Unnamed_Ext_Variable_Value ext_var_val;

        assert (rul__val_is_ext_variable (value));

	if (is_unnamed_mol_var_val(value)) return FALSE; /* never allocates */

	ext_var_val =  (Unnamed_Ext_Variable_Value) value;

        return ext_var_val->memory_allocated;
}


 /* Arithmetic Expression Values */


/*******************************
**                            **
**  RUL__VAL_IS_ARITH_EXPR    **
**                            **
*******************************/

Boolean
rul__val_is_arith_expr (Value val)
{
        assert (is_valid_value (val));
        return (is_arith_expr_val (val));
}


/*********************************
**                              **
**  RUL__VAL_CREATE_ARITH_EXPR  **
**                              **
*********************************/

Value
rul__val_create_arith_expr (Arith_Oper_Type arith_operator,
			    Value operand_1, Value operand_2)
{
	Arith_Expr_Value a_val;

	assert (is_valid_value (operand_1));
	assert (operand_2 == NULL || is_valid_value (operand_2));

	assert (arith_operator == CMP__E_ARITH_PLUS   	||
		arith_operator == CMP__E_ARITH_MINUS  	||
                arith_operator == CMP__E_ARITH_TIMES	||
                arith_operator == CMP__E_ARITH_DIVIDE	||
                arith_operator == CMP__E_ARITH_MODULUS	||
                arith_operator == CMP__E_ARITH_UNARY_MINUS);


	if (operand_1 == NULL)
	    return NULL;

	if (arith_operator == CMP__E_ARITH_PLUS   	||
	    arith_operator == CMP__E_ARITH_MINUS  	||
	    arith_operator == CMP__E_ARITH_TIMES	||
	    arith_operator == CMP__E_ARITH_DIVIDE	||
	    arith_operator == CMP__E_ARITH_MODULUS)
 	    if (operand_2 == NULL)
	        return NULL;
	
	a_val = (Arith_Expr_Value) val_create_value (VAL__E_ARITH_EXPR, 0);

	a_val->arith_operator = arith_operator;
	a_val->operand_1 = operand_1;
	a_val->operand_2 = operand_2;

	/*
	**  Refine the type of the resulting value, if we can.
	*/
	if (operand_2 == NULL) {
	    assert (arith_operator == CMP__E_ARITH_UNARY_MINUS);
	    if (operand_1->domain == dom_int_atom) {
		a_val->domain = dom_int_atom;
	    } else if (operand_1->domain == dom_dbl_atom) {
		a_val->domain = dom_dbl_atom;
	    } else {
		a_val->domain = dom_number;
	    }
	} else {
	    /*  A binary operator  */
	    if (operand_1->domain == dom_int_atom  &&  
		    operand_2->domain == dom_int_atom) {
	        a_val->domain = dom_int_atom;
	    } else if (operand_1->domain == dom_dbl_atom ||
		    operand_2->domain == dom_dbl_atom) {
	        a_val->domain = dom_dbl_atom;
	    } else {
	        a_val->domain = dom_number;
	    }
	}
	/*  Arithmetic operations always return a scalar value  */
	a_val->shape = shape_atom; 

	return ((Value) a_val);
}


 /* Conditional Expression Values */


/******************************
**                           **
**  RUL__VAL_IS_COND_EXPR    **
**                           **
******************************/

Boolean
rul__val_is_cond_expr (Value val)
{
        assert (is_valid_value (val));
        return (is_cond_expr_val (val));
}


/*********************************
**                              **
**  RUL__VAL_CREATE_COND_EXPR  **
**                              **
*********************************/

Value
rul__val_create_cond_expr (Cond_Oper_Type cond_operator,
				   Value operand_1, Value operand_2)
{
	Cond_Expr_Value a_val;

	assert (is_valid_value (operand_1));
	assert (operand_2 == NULL || is_valid_value (operand_2));

	assert (cond_operator == CMP__E_COND_AND   	||
		cond_operator == CMP__E_COND_OR  	||
                cond_operator == CMP__E_COND_NOT);

	if (operand_1 == NULL)
	    return NULL;

	if (cond_operator == CMP__E_COND_AND   	||
	    cond_operator == CMP__E_COND_OR)
 	    if (operand_2 == NULL)
	        return NULL;
	
	a_val = (Cond_Expr_Value) val_create_value (VAL__E_COND_EXPR, 0);

	a_val->cond_operator = cond_operator;
	a_val->operand_1 = operand_1;
	a_val->operand_2 = operand_2;

	return ((Value) a_val);
}


 /* Function Values */


/***************************
**                        **
**  RUL__VAL_IS_FUNCTION  **
**                        **
***************************/

Boolean
rul__val_is_function (Value func_value)
{
	assert (is_valid_value (func_value));
	return (is_function_val (func_value));
}



/*******************************
**                            **
**  RUL__VAL_CREATE_FUNCTION  **
**                            **
*******************************/

Value
rul__val_create_function (Mol_Symbol func_name, long arg_count)
{
	Function_Value f_val;
	long i;

	assert (rul__mol_is_valid (func_name));

	f_val = (Function_Value) 
		val_create_value (VAL__E_FUNCTION, arg_count);

	f_val->function_name = func_name;
	rul__mol_incr_uses (func_name);
	f_val->next_value = NULL;
	f_val->prev_value = NULL;
	f_val->passing_mech = ext_mech_value;

	f_val->subvalue_count = arg_count;
	for (i=0; i<arg_count; i++) f_val->subvalues[i] = NULL;

	return ((Value) f_val);
}


/***********************************
**                                **
**  RUL__VAL_CREATE_FUNCTION_STR  **
**                                **
***********************************/

Value
rul__val_create_function_str (char *func_name, long arg_count)
{
	Mol_Symbol mol;
	Value val;

	mol = rul__mol_make_symbol (func_name);
	val = rul__val_create_function (mol, arg_count);
	rul__mol_decr_uses (mol);
	return val;
}



/*****************************
**                          **
**  RUL__VAL_SET_FUNC_MECH  **
**                          **
*****************************/

void
rul__val_set_func_mech (Value value, Ext_Mech mech)
{
	Function_Value fn_val;

	assert (is_valid_value (value));
	assert (rul__val_is_function (value));

	fn_val = (Function_Value) value;
	fn_val->passing_mech = mech;
}

/********************************
**                             **
**  RUL__VAL_SET_NTH_SUBVALUE  **
**                             **
********************************/

void
rul__val_set_nth_subvalue (Value complex_val, long index, Value subval)
{
	Complex_Value cx_val;

	assert (is_valid_value (complex_val)  &&  is_complex_val(complex_val));
	assert (subval == NULL  ||  is_valid_value (subval));

	cx_val = (Complex_Value) complex_val;
	assert (index > 0  &&  index <= cx_val->subvalue_count);
	assert (subval == NULL  ||  cx_val->subvalues[index-1] == NULL);

	cx_val->subvalues[index-1] = subval;
}



/******************************
**                           **
**  RUL__VAL_GET_LAST_VALUE  **
**                           **
******************************/

Value
rul__val_get_last_value (Value value)
{
	Assignment_Value as_val;
	Function_Value fn_val;

	if (value == NULL)
	  return NULL;

	assert (is_valid_value (value));

	if (!rul__val_is_assignment (value) &&
	    !rul__val_is_function (value))
	  return NULL;

	if (rul__val_is_assignment (value))
	  {
	    as_val = (Assignment_Value) value;
	    if (as_val->next_value)
	      return rul__val_get_last_value (as_val->next_value);
	  }
	else
	  {
	    fn_val = (Function_Value) value;
	    if (fn_val->next_value)
	      return rul__val_get_last_value (fn_val->next_value);
	  }
	
	return value;
}

/******************************
**                           **
**  RUL__VAL_SET_NEXT_VALUE  **
**                           **
******************************/

Value
rul__val_set_next_value (Value value, Value next_value)
{
	Assignment_Value as_val;
	Function_Value fn_val;

	if (next_value == NULL) return value;

	assert (is_valid_value (value));
	assert (is_valid_value (next_value));
	assert (rul__val_is_assignment (value) || 
		rul__val_is_function (value));
	assert (rul__val_is_assignment (next_value) || 
		rul__val_is_function (next_value));

	if (rul__val_is_assignment (value))
	    {
	    as_val = (Assignment_Value) value;
	    assert (as_val->next_value == NULL);
	    as_val->next_value = next_value;
	    }
	else
	    {
	    fn_val = (Function_Value) value;
	    assert (fn_val->next_value == NULL);
	    fn_val->next_value = next_value;
	    }
	return next_value;
}

/******************************
**                           **
**  RUL__VAL_SET_PREV_VALUE  **
**                           **
******************************/

Value
rul__val_set_prev_value (Value value, Value prev_value)
{
	Assignment_Value as_val;
	Function_Value fn_val;

	assert (is_valid_value (value));
	assert (rul__val_is_assignment (value) || 
		rul__val_is_function (value));

	if (prev_value == NULL) 
	    {
	    /* assure there are no memory leaks while making these trees */
	    if (rul__val_is_assignment (value))
		{
		as_val = (Assignment_Value) value;
		assert (as_val->prev_value == NULL);
		}
	    else
		{
		fn_val = (Function_Value) value;
		assert (fn_val->prev_value == NULL);
		}
	    return value;
	    }
	    

	assert (is_valid_value (prev_value));
	assert (rul__val_is_assignment (prev_value) || 
		rul__val_is_function (prev_value));

	if (rul__val_is_assignment (value))
	    {
	    as_val = (Assignment_Value) value;
	    assert (as_val->prev_value == NULL);
	    as_val->prev_value = prev_value;
	    }
	else
	    {
	    fn_val = (Function_Value) value;
	    assert (fn_val->prev_value == NULL);
	    fn_val->prev_value = prev_value;
	    }
	return value;
}

/******************************
**                           **
**  RUL__VAL_GET_NEXT_VALUE  **
**                           **
******************************/

Value
rul__val_get_next_value (Value value)
{
	Assignment_Value as_val;
	Function_Value fn_val;

	assert (rul__val_is_assignment (value) || 
		rul__val_is_function (value));

	if (rul__val_is_assignment (value))
	    {
	    as_val = (Assignment_Value) value;
	    return as_val->next_value;
	    }
	else
	    {
	    fn_val = (Function_Value) value;
	    return fn_val->next_value;
	    }
}

/******************************
**                           **
**  RUL__VAL_GET_PREV_VALUE  **
**                           **
******************************/

Value
rul__val_get_prev_value (Value value)
{
	Assignment_Value as_val;
	Function_Value fn_val;

	assert (rul__val_is_assignment (value) || 
		rul__val_is_function (value));

	if (rul__val_is_assignment (value))
	    {
	    as_val = (Assignment_Value) value;
	    return as_val->prev_value;
	    }
	else
	    {
	    fn_val = (Function_Value) value;
	    return fn_val->prev_value;
	    }
}


 /* Assignment Value routines */


/*****************************
**                          **
**  RUL__VAL_IS_ASSIGNMENT  **
**                          **
*****************************/

Boolean
rul__val_is_assignment (Value assign_val)
{
	assert (is_valid_value (assign_val));
	return (is_assignment_val (assign_val));
}



/*********************************
**                              **
**  RUL__VAL_CREATE_ASSIGNMENT  **
**                              **
*********************************/

Value
rul__val_create_assignment (Value to_variable, Value from_value)
{
	Assignment_Value as_val;

	assert (is_valid_value (to_variable));
	assert (is_valid_value (from_value));

	as_val = (Assignment_Value) 
		val_create_value (VAL__E_ASSIGNMENT, 0);

	as_val->to_variable = to_variable;
	as_val->from_value = from_value;
	as_val->next_value = NULL;
	as_val->prev_value = NULL;

	/*
	**  Propogate the shape and domain up to the variable
	**  and to the assignment value itself.
	*/
	as_val->domain = from_value->domain;
	as_val->shape =  from_value->shape;
	as_val->rclass =  from_value->rclass;
	to_variable->domain = from_value->domain;
	to_variable->shape =  from_value->shape;
	to_variable->rclass =  from_value->rclass;

	return ((Value) as_val);
}



/*********************************
**                              **
**  RUL__VAL_GET_ASSIGNMENT_TO  **
**                              **
*********************************/

Value
rul__val_get_assignment_to (Value value)
{
	Assignment_Value as_val;

	assert (rul__val_is_assignment (value));

	as_val = (Assignment_Value) value;

	return as_val->to_variable;
}

/***********************************
**                                **
**  RUL__VAL_GET_ASSIGNMENT_FROM  **
**                                **
***********************************/

Value
rul__val_get_assignment_from (Value value)
{
	Assignment_Value as_val;

	assert (rul__val_is_assignment (value));

	as_val = (Assignment_Value) value;

	return as_val->from_value;
}


 /* Complex Values */

/******************************
**                           **
**  RUL__VAL_CREATE_COMPLEX  **
**                           **
******************************/

Value
rul__val_create_complex (long cmplx_count)
{
	Complex_Value c_val;
	long i;

	c_val = (Complex_Value) 
		val_create_value (VAL__E_COMPLEX, cmplx_count);

	c_val->function_name = NULL;
	c_val->subvalue_count = cmplx_count;
	c_val->next_value = NULL;
	c_val->prev_value = NULL;
	c_val->passing_mech = ext_mech_invalid;
	for (i=0; i<cmplx_count; i++) c_val->subvalues[i] = NULL;

	return ((Value) c_val);
}



/**************************
**                       **
**  RUL__VAL_IS_COMPLEX  **
**                       **
**************************/

Boolean
rul__val_is_complex (Value value)
{
	assert (is_valid_value (value));
	return (is_complex_val (value));
}





/*********************************
**                              **
**  RUL__VAL_GET_COMPLEX_COUNT  **
**                              **
*********************************/

long
rul__val_get_complex_count (Value value)
{
	assert (is_valid_value (value));
	assert (is_complex_val (value));

	return (((Complex_Value)value)->subvalue_count);
}



/*******************************
**                            **
**  RUL__VAL_GET_NTH_COMPLEX  **
**                            **
*******************************/

Value
rul__val_get_nth_complex (Value value, long n)
{
	assert (is_valid_value (value));
	assert (is_complex_val (value));
	assert (n > 0  &&  n <= ((Complex_Value)value)->subvalue_count);

	return (((Complex_Value)value)->subvalues[n-1]);
}


 /* Disjunction Values */


/**********************************
**                               **
**  RUL__VAL_CREATE_DISJUNCTION  **
**                               **
**********************************/

Value
rul__val_create_disjunction (long disj_count)
{
	Disjunction_Value d_val;
	long i;

	d_val = (Disjunction_Value) 
		val_create_value (VAL__E_DISJUNCTION, disj_count);

	d_val->function_name = NULL;
	d_val->next_value = NULL;
	d_val->prev_value = NULL;
	d_val->passing_mech = ext_mech_invalid;
	d_val->subvalue_count = disj_count;
	for (i=0; i<disj_count; i++) d_val->subvalues[i] = NULL;

	return ((Value) d_val);
}



/******************************
**                           **
**  RUL__VAL_IS_DISJUNCTION  **
**                           **
******************************/

Boolean
rul__val_is_disjunction (Value value)
{
	assert (is_valid_value (value));
	return (is_disjunction_val (value));
}





/**********************************
**                               **
**  RUL__VAL_GET_DISJUNCT_COUNT  **
**                               **
**********************************/

long
rul__val_get_disjunct_count (Value value)
{
	assert (is_valid_value (value));
	assert (is_disjunction_val (value));

	return (((Disjunction_Value)value)->subvalue_count);
}



/********************************
**                             **
**  RUL__VAL_GET_NTH_DISJUNCT  **
**                             **
********************************/

Value
rul__val_get_nth_disjunct (Value value, long n)
{
	assert (is_valid_value (value));
	assert (is_disjunction_val (value));
	assert (n > 0  &&  n <= ((Disjunction_Value)value)->subvalue_count);

	return (((Disjunction_Value)value)->subvalues[n-1]);
}



/************************
**                     **
**  RUL__VAL_IS_LOCAL  **
**                     **
************************/

Boolean
rul__val_is_local (Value value, long ce_index)
{
	long attr_index, i;
	Value index_expr;
	Complex_Value cx_val;
	Assignment_Value as_val;
	Boolean local;

	assert (is_valid_value (value));

	switch (value->type) {

	    case VAL__E_POS :
		    attr_index = ((Position_Value)value)->ce_index;
		    return (attr_index == ce_index);
		    break;

	    case VAL__E_POS_ELEM :
		    attr_index = ((Position_Elem_Value)value)->ce_index;
		    index_expr = ((Position_Elem_Value)value)->index;
		    return (attr_index == ce_index  &&  
			    rul__val_is_local (index_expr, ce_index));
		    break;

	    case VAL__E_ARITH_EXPR :
		    local = TRUE;
		    local &= rul__val_is_local (
					((Arith_Expr_Value)value)->operand_1,
					ce_index);
		    if (((Arith_Expr_Value)value)->operand_2 != NULL) {
			local &= rul__val_is_local (
					((Arith_Expr_Value)value)->operand_2,
					ce_index);
		    }
		    return (local);
		    break;

	    case VAL__E_COND_EXPR :
		    local = TRUE;
		    local &= rul__val_is_local (
					((Cond_Expr_Value)value)->operand_1,
					ce_index);
		    if (((Cond_Expr_Value)value)->operand_2 != NULL) {
			local &= rul__val_is_local (
					((Cond_Expr_Value)value)->operand_2,
					ce_index);
		    }
		    return (local);
		    break;

	    case VAL__E_ASSIGNMENT :
		    as_val = (Assignment_Value) value;
		    local = rul__val_is_local (as_val->from_value, ce_index);
		    if (as_val->prev_value)
		      local &= rul__val_is_local (as_val->prev_value,ce_index);
		    return (local);
		    break;

	    case VAL__E_FUNCTION :
	    case VAL__E_COMPLEX :
	    case VAL__E_DISJUNCTION :
		    local = TRUE;
		    cx_val = (Complex_Value) value;
		    for (i=0; i<cx_val->subvalue_count; i++) {
			local &= rul__val_is_local (
					cx_val->subvalues[i],
					ce_index);
		    }
		    if (cx_val->prev_value)
		      local &= rul__val_is_local (cx_val->prev_value,ce_index);
		    return (local);
		    break;

	    case VAL__E_LHS_VARIABLE :
		    local = rul__lvar_variable_is_local (
				    ((Variable_Value)value)->variable_name,
				    ce_index);
		    return (local);
		    break;

	    case VAL__E_RHS_VARIABLE :
		    assert (FALSE); /*  question should never be asked  */
		    break;

	    default : /* constants */
		    return (TRUE);
	}

	assert (FALSE); /*  question should never be asked  */
	return (FALSE);
}




/**********************************
**                               **
**  RUL__VAL_MARK_LHS_VARIABLES  **
**                               **
**********************************/

void
rul__val_mark_lhs_variables (Value value, Net_Node node)
{
	long          i;
	Assignment_Value as_val;
	Complex_Value cx_val;

	assert (is_valid_value (value));

	switch (value->type) {

	    case VAL__E_LHS_VARIABLE :
		    rul__net_add_tin_var_usage (node, value);
		    break;

	    case VAL__E_ARITH_EXPR :
		    rul__val_mark_lhs_variables (
				((Arith_Expr_Value)value)->operand_1,
				node);
		    if (((Arith_Expr_Value)value)->operand_2 != NULL) {
			rul__val_mark_lhs_variables (
				((Arith_Expr_Value)value)->operand_2,
				node);
		    }
		    break;

	    case VAL__E_COND_EXPR :
		    rul__val_mark_lhs_variables (
				((Cond_Expr_Value)value)->operand_1,
				node);
		    if (((Cond_Expr_Value)value)->operand_2 != NULL) {
			rul__val_mark_lhs_variables (
				((Cond_Expr_Value)value)->operand_2,
				node);
		    }
		    break;

	    case VAL__E_ASSIGNMENT :
		    as_val = (Assignment_Value) value;
		    rul__val_mark_lhs_variables (as_val->from_value, node);
		    break;

	    case VAL__E_FUNCTION :
	    case VAL__E_COMPLEX :
	    case VAL__E_DISJUNCTION :
		    cx_val = (Complex_Value) value;
		    for (i=0; i<cx_val->subvalue_count; i++) {
			rul__val_mark_lhs_variables (
					cx_val->subvalues[i],
					node);
		    }
		    break;

	    case VAL__E_POS :
	    case VAL__E_POS_ELEM :
	    case VAL__E_RHS_VARIABLE :
		    assert (FALSE); /*  question should never be asked  */
		    break;

	    default : /* constants */
		    break;
	}

}




/************************************
**                                 **
**  RUL__VAL_PRINT_CONSTRUCT_TYPE  **
**                                 **
************************************/

static void
rul__val_print_construct_type (Construct_Type crt, IO_Stream ios)
{
  char *cons_type_str;

  switch (crt) {
  case RUL__C_NOISE:
    cons_type_str = "RUL__C_NOISE";
    break;
  case RUL__C_RULE:
    cons_type_str = "RUL__C_RULE";
    break;
  case RUL__C_CATCH:
    cons_type_str = "RUL__C_CATCH";
    break;
  case RUL__C_COMMENT:
    cons_type_str = "RUL__C_COMMENT";
    break;
  case RUL__C_OBJ_CLASS:
    cons_type_str = "RUL__C_OBJ_CLASS";
    break;
  case RUL__C_EXT_ROUTINE:
    cons_type_str = "RUL__C_EXT_ROUTINE";
    break;
  case RUL__C_DECL_BLOCK:
    cons_type_str = "RUL__C_DECL_BLOCK";
    break;
  case RUL__C_ENTRY_BLOCK:
    cons_type_str = "RUL__C_ENTRY_BLOCK";
    break;
  case RUL__C_RULE_BLOCK:
    cons_type_str = "RUL__C_RULE_BLOCK";
    break;
  case RUL__C_END_BLOCK:
    cons_type_str = "RUL__C_END_BLOCK";
    break;
  case RUL__C_RULE_GROUP:
    cons_type_str = "RUL__C_RULE_GROUP";
    break;
  case RUL__C_END_GROUP:
    cons_type_str = "RUL__C_END_GROUP";
    break;
  case RUL__C_ON_ENTRY:
    cons_type_str = "RUL__C_ON_ENTRY";
    break;
  case RUL__C_ON_EVERY:
    cons_type_str = "RUL__C_ON_EVERY";
    break;
  case RUL__C_ON_EMPTY:
    cons_type_str = "RUL__C_ON_EMPTY";
    break;
  case RUL__C_ON_EXIT:
    cons_type_str = "RUL__C_ON_EXIT";
    break;
  case RUL__C_METHOD:
    cons_type_str = "RUL__C_METHOD";
    break;
  default:
    cons_type_str = "--UNKNOWN CONSTRUCT TYPE--";
    }
  
  rul__ios_printf (ios, "%s", cons_type_str);
}




/**********************************
**                               **
**  RUL__VAL_PRINT_ANIMAL_CONST  **
**                               **
**********************************/

void
rul__val_print_animal_const (Value value, IO_Stream ios)
{
	Animal_Constant_Value a_val;

	assert (is_valid_or_freed_value (value));
	assert (is_animal_constant_val (value));

	a_val = (Animal_Constant_Value) value;
	switch (a_val->ext_type) {

		case ext_type_long:
		case ext_type_short:
		case ext_type_byte:
			rul__ios_printf (ios, "%ld",
					 a_val->val.sign_int_val);
			break;

		case ext_type_uns_long:
		case ext_type_uns_short:
		case ext_type_uns_byte:
			rul__ios_printf (ios, "%lu",
					 a_val->val.unsign_int_val);
			break;

		case ext_type_float:
		case ext_type_double:
			rul__ios_printf (ios, "%s", 
				rul__mol_format_double (a_val->val.dbl_val));
			break;

		case ext_type_asciz:
			rul__ios_printf (ios, "%s", a_val->val.asciz_val);
			break;

		case ext_type_void_ptr:
			rul__ios_printf (ios, "0x%p", a_val->val.opaque_val);
			break;

		case ext_type_ascid:
			rul__ios_printf (ios, "ascid ... printer is NYI");
			break;

		default:
			rul__ios_printf (ios, "?-unknown-animal-type-?");
	}
}



#ifndef NDEBUG



/**********************
**                   **
**  RUL___VAL_PRINT  **
**                   **
**********************/

void
rul___val_print (Value value)
{
	char name[RUL_C_MAX_SYMBOL_SIZE+1];
	Arith_Oper_Type type;
	Cond_Oper_Type ctype;
	Function_Value f_val;
	Complex_Value c_val;
	Disjunction_Value d_val;
	Assignment_Value as_val;
	Blk_Or_Local_Value l_val;
	Mol_Symbol mol;
	long i;
	Class cls;
	Construct_Type crt;
	extern void rul___mol_quiet_print_printform (Molecule, IO_Stream);

	assert (is_valid_or_freed_value (value));

	switch (value->type) {

	    case VAL__E_POS :
		mol = ((Position_Value)value)->attr_name;
		rul__ios_printf (RUL__C_STD_ERR, "^");
		rul___mol_quiet_print_printform (mol, RUL__C_STD_ERR);
		break;

	    case VAL__E_POS_ELEM :
		mol = ((Position_Elem_Value)value)->attr_name;
		rul__ios_printf (RUL__C_STD_ERR, "^");
		rul___mol_quiet_print_printform (mol, RUL__C_STD_ERR);
		rul__ios_printf (RUL__C_STD_ERR, "[");
		rul___val_print (
			((Position_Elem_Value)value)->index);
		rul__ios_printf (RUL__C_STD_ERR, "]");
		break;

	    case VAL__E_ARITH_EXPR :
		type = ((Arith_Expr_Value)value)->arith_operator ;
		switch (type) {
		    case CMP__E_ARITH_PLUS:
			rul__ios_printf (RUL__C_STD_ERR, "(+ ");
			break;
		    case CMP__E_ARITH_MINUS:
			rul__ios_printf (RUL__C_STD_ERR, "(- ");
			break;
		    case CMP__E_ARITH_TIMES:
			rul__ios_printf (RUL__C_STD_ERR, "(* ");
			break;
		    case CMP__E_ARITH_DIVIDE:
			rul__ios_printf (RUL__C_STD_ERR, "(/ ");
			break;
		    case CMP__E_ARITH_MODULUS:
			rul__ios_printf (RUL__C_STD_ERR, "(modulo ");
			break;
		    case CMP__E_ARITH_UNARY_MINUS:
			rul__ios_printf (RUL__C_STD_ERR, "(-- ");
			break;
		}
		rul___val_print (
			((Arith_Expr_Value)value)->operand_1);
		if (type != CMP__E_ARITH_UNARY_MINUS) {
		    rul__ios_printf (RUL__C_STD_ERR, " ");
		    rul___val_print (
			((Arith_Expr_Value)value)->operand_2);
		}
		rul__ios_printf (RUL__C_STD_ERR, ")");
		break;

	    case VAL__E_COND_EXPR :
		ctype = ((Cond_Expr_Value)value)->cond_operator ;
		switch (ctype) {
		    case CMP__E_COND_AND:
			rul__ios_printf (RUL__C_STD_ERR, "(&& ");
			break;
		    case CMP__E_COND_OR:
			rul__ios_printf (RUL__C_STD_ERR, "(|| ");
			break;
		    case CMP__E_COND_NOT:
			rul__ios_printf (RUL__C_STD_ERR, "(! ");
			break;
		}
		rul___val_print (
			((Cond_Expr_Value)value)->operand_1);
		if (ctype != CMP__E_COND_NOT) {
		    rul__ios_printf (RUL__C_STD_ERR, " ");
		    rul___val_print (
			((Cond_Expr_Value)value)->operand_2);
		}
		rul__ios_printf (RUL__C_STD_ERR, ")");
		break;

	    case VAL__E_RUL_CONST :
		mol = ((Rul_Constant_Value)value)->constant_value;
		rul___mol_quiet_print_printform (mol, RUL__C_STD_ERR);
		break;

	    case VAL__E_ANIMAL_CONST :
		rul__val_print_animal_const (value, RUL__C_STD_ERR);
		break;

	    case VAL__E_CLASS_CONST :
		cls = ((Class_Constant_Value)value)->class_value;
		rul__ios_printf (RUL__C_STD_ERR, "Class:");
		rul___mol_quiet_print_printform (
			rul__decl_get_class_name (cls), RUL__C_STD_ERR);
		break;

	    case VAL__E_CONSTRUCT :
		mol = ((Construct_Value)value)->construct_value;
		rul__ios_printf (RUL__C_STD_ERR, "Construct:");
		rul___mol_quiet_print_printform (mol, RUL__C_STD_ERR);
		break;

	    case VAL__E_CONSTRUCT_TYPE :
		crt = ((Construct_Type_Value)value)->construct_type;
		rul__val_print_construct_type (crt, RUL__C_STD_ERR);
		break;

	    case VAL__E_RHS_VARIABLE :
		mol = ((Variable_Value)value)->variable_name;
		assert (mol != NULL);
		rul___mol_quiet_print_printform (mol, RUL__C_STD_ERR);
		break;

	    case VAL__E_LHS_VARIABLE :
		mol = ((Variable_Value)value)->variable_name;
		if (mol != NULL) {
		    rul___mol_quiet_print_printform (mol, RUL__C_STD_ERR);
		} else {
		    rul__ios_printf (RUL__C_STD_ERR, "<unnamed>");
		}
		rul__ios_printf (RUL__C_STD_ERR, ":%ld",
			    ((LHS_Variable_Value)value)->lhs_var_id_number);
		break;

	    case VAL__E_UNM_EXT_VAR :
		rul__ios_printf (RUL__C_STD_ERR, "<unnamed-ext:%ld:%d>",
		    ((Unnamed_Ext_Variable_Value)value)->cache_index,
		    (long)((Unnamed_Ext_Variable_Value)value)->cache_type);
		break;

	    case VAL__E_NAM_EXT_VAR :
		mol = ((Named_Ext_Variable_Value)value)->variable_name;
		rul___mol_quiet_print_printform (mol, RUL__C_STD_ERR);
		break;

	    case VAL__E_UNM_MOL_VAR :
		rul__ios_printf (RUL__C_STD_ERR, "<unnamed-mol:%ld>",
		    ((Unnamed_Mol_Variable_Value)value)->cache_index);
		break;

	    case VAL__E_FUNCTION :
		f_val = (Function_Value) value;
		mol = f_val->function_name;
		rul__ios_printf (RUL__C_STD_ERR, "(");
		rul___mol_quiet_print_printform (mol, RUL__C_STD_ERR);
		for (i=0; i<f_val->subvalue_count; i++) {
		    rul__ios_printf (RUL__C_STD_ERR, " ");
		    if (is_valid_or_freed_value (f_val->subvalues[i]))
		        rul___val_print (f_val->subvalues[i]);
		    else
			rul__ios_printf (RUL__C_STD_ERR, " -undefined- ");
		}
		rul__ios_printf (RUL__C_STD_ERR, ")");
		if (f_val->prev_value != NULL)
		    {
		    rul__ios_printf (RUL__C_STD_ERR, "\n <-prev- ");
		    rul___val_print (f_val->prev_value);
	 	    }
		if (f_val->next_value != NULL)
		    {
		    rul__ios_printf (RUL__C_STD_ERR, "\n -next-> ");
		    rul___val_print (f_val->next_value);
	 	    }
		break;

	    case VAL__E_COMPLEX :
		c_val = (Complex_Value) value;
		rul__ios_printf (RUL__C_STD_ERR, "<C");
		for (i=0; i<c_val->subvalue_count; i++) {
		    rul__ios_printf (RUL__C_STD_ERR, " ");
		    rul___val_print (c_val->subvalues[i]);
		}
		rul__ios_printf (RUL__C_STD_ERR, " C>");
		break;

	    case VAL__E_DISJUNCTION :
		d_val = (Disjunction_Value) value;
		rul__ios_printf (RUL__C_STD_ERR, "<<");
		for (i=0; i<d_val->subvalue_count; i++) {
		    rul__ios_printf (RUL__C_STD_ERR, " ");
		    rul___val_print (d_val->subvalues[i]);
		}
		rul__ios_printf (RUL__C_STD_ERR, " >>");
		break;

	    case VAL__E_ASSIGNMENT :
		rul__ios_printf (RUL__C_STD_ERR, "(");
		as_val = (Assignment_Value) value;
		rul___val_print (as_val->to_variable);
		rul__ios_printf (RUL__C_STD_ERR, " <= ");
		rul___val_print (as_val->from_value);
		rul__ios_printf (RUL__C_STD_ERR, ")");
		if (as_val->prev_value != NULL)
		    {
		    rul__ios_printf (RUL__C_STD_ERR, "\n <-prev- ");
		    rul___val_print (as_val->prev_value);
	 	    }
		if (as_val->next_value != NULL)
		    {
		    rul__ios_printf (RUL__C_STD_ERR, "\n -next-> ");
		    rul___val_print (as_val->next_value);
	 	    }
	        break;

	    case VAL__E_BLK_OR_LOCAL :
		l_val = (Blk_Or_Local_Value) value;
		rul__ios_printf (RUL__C_STD_ERR, "'");
		if (l_val->arg_indirect) rul__ios_printf (RUL__C_STD_ERR, "&");
		rul___mol_quiet_print_printform (
				l_val->arg_name, RUL__C_STD_ERR);
		if (l_val->arg_field != CMP__E_FIELD__NONE) {
		    switch (l_val->arg_field) {
			case CMP__E_FIELD_BETA_INST_COUNT:
			    rul__ios_printf (RUL__C_STD_ERR, "->inst_count");
			    break;
			case CMP__E_FIELD_BETA_INST_VEC :
			    rul__ios_printf (RUL__C_STD_ERR, "->inst_array");
			    break;
			case CMP__E_FIELD_BETA_HSH_VEC :
			    rul__ios_printf (RUL__C_STD_ERR, "->val_array");
			    break;
                        case CMP__E_FIELD_BETA_AUX_VEC :
			    rul__ios_printf (RUL__C_STD_ERR, "->->aux_array");
			    break;
                        case CMP__E_FIELD_DELTA_SIGN :
			    rul__ios_printf (RUL__C_STD_ERR, "->sign");
			    break;
                        case CMP__E_FIELD_DELTA_OBJ :
			    rul__ios_printf (RUL__C_STD_ERR, "->instance");
			    break;
                        case CMP__E_FIELD_DELTA_INST_ID :
			    rul__ios_printf (RUL__C_STD_ERR, "->instance_id");
			    break;
                        case CMP__E_FIELD_DELTA_CLASS :
			    rul__ios_printf (RUL__C_STD_ERR, "->instance_class");
			    break;
			default:
			    rul__ios_printf (RUL__C_STD_ERR, "-> ???");
			    break;
		    }
		}
		if (l_val->arg_index != VAL__C_NOT_AN_INDEX) {
		    rul__ios_printf (RUL__C_STD_ERR, "[%ld]", 
				l_val->arg_index);
		}
		rul__ios_printf (RUL__C_STD_ERR, "'");
		break;

	    default :
		/* assert (BOGUS_VALUE_TYPE); */
		  rul__ios_printf (RUL__C_STD_ERR,
				   "\n UNKNOWN type %d",value->type);
		    break;
	 }
}



/*********************************
**                              **
**  RUL___VAL_SHAPE_DOMAIN_STR  **
**                              **
*********************************/

char *
rul___val_shape_domain_str (Value val)
{
	char 	*domain_str, *shape_str, 
		class_str[RUL_C_MAX_SYMBOL_SIZE+1],
		buffer[2 * RUL_C_MAX_SYMBOL_SIZE+1],
	 	*result;

	domain_str = rul__sem_domain_to_string (val->domain);
	shape_str = rul__sem_shape_to_string (val->shape);

	if (val->domain == dom_instance_id) {
	    if (val->rclass == NULL) {
		strcpy (class_str, "is unknown");
	    } else {
		rul__mol_use_printform (rul__decl_get_class_name (val->rclass),
					class_str, RUL_C_MAX_SYMBOL_SIZE);
	    }
	    sprintf (buffer, "[Shape: %s  Domain: %s  Class: %s]",
			shape_str, domain_str, class_str);
	} else {
	    sprintf (buffer, "[Shape: %s  Domain: %s]", shape_str, domain_str);
	}
	result = rul__mem_malloc (strlen(buffer) + 1);
	strcpy (result, buffer);
	return (result);
}




/*********************
**                  **
**  RUL__VAL_PRINT  **
**                  **
*********************/

void
rul__val_print (
#ifdef __VMS
		Value *in_val)
#else
		Value in_val)
#endif
{
	Value val;
	char *shape_n_domain;

	fprintf (stderr, "\n");

#ifdef __VMS
	val = *in_val;
#else
	val = in_val;
#endif
	rul___val_print (val);
	shape_n_domain = rul___val_shape_domain_str (val);
	rul__ios_printf (RUL__C_STD_ERR, "   %s", shape_n_domain);
	rul__mem_free (shape_n_domain);
}


#endif /* ifndef NDEBUG */
