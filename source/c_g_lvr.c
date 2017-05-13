/****************************************************************************
**                                                                         **
**               C M P _ G E N _ N E T _ L H S _ V A R . C                 **
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
 *  FACILITY:
 *	RULEWORKS compiler
 *
 *  ABSTRACT:
 *	Code generation associated with access to LHS variables
 *
 *  MODIFIED BY:
 *	DEC	Digital Equipment Corporation
 *	CPQ	Compaq Compauter Corporation
 *
 *  MODIFICATION HISTORY:
 *	16-Feb-1993	DEC	Initial version
 *	01-Dec-1999	CPQ	Release with GPL
 */

#include <common.h>
#include <cmp_comm.h>
#include <dyar.h>
#include <emit.h>
#include <gen.h>
#include <val.h>
#include <net.h>
#include <net_p.h>
#include <gennet_p.h>



/*
**      The following functions make use of these static values
**	to be able to make the complex decisions about how to 
**      express references to lhs variable values.
**
*/
static Net_Node SA_cur_gen_node = NULL;
static Boolean	SA_cur_gen_is_left_side;


static void gen_beta_array_ref (Rul_Internal_Field field, long index);
static void gen_local_array_ref (char *arg_name, long index);




/*************************************
**                                  **
**  RUL___GEN_LHS_VAR_SET_CUR_NODE  **
**                                  **
*************************************/

void rul___gen_lhs_var_set_cur_node (Net_Node node)
{
	SA_cur_gen_node = node;
}



/*************************************
**                                  **
**  RUL___GEN_LHS_VAR_SET_CUR_SIDE  **
**                                  **
*************************************/

void rul___gen_lhs_var_set_cur_side (Boolean is_left_side)
{
	SA_cur_gen_is_left_side = is_left_side;
}




/********************************
**                             **
**  RUL___GEN_LHS_VAR_ASSIGNS  **
**                             **
********************************/

void rul___gen_lhs_var_assigns (Value lhs_var, 
				Net_Node downstream_node)
{
	assert (SA_cur_gen_node != NULL);
	/* NYI */
}




/*********************************
**                              **
**  RUL__GEN_LHS_VAR_REFERENCE  **
**                              **
*********************************/

void rul__gen_lhs_var_reference (Value lhs_var)
{
	Net_1_In_Node oin;
	Net_2_In_Node tin;
	long index, lhs_var_id;
	Value val;

	assert (SA_cur_gen_node != NULL);
	assert (rul__val_is_lhs_variable (lhs_var));
	lhs_var_id = rul__val_get_lhs_variable_id (lhs_var);

	if (is_1_in_node (SA_cur_gen_node)) {

	    /*  If this is a 1-input node, it better be bound locally  */
	    oin = (Net_1_In_Node) SA_cur_gen_node;
	    index = rul___net_get_var_position (oin->value_bindings, 
						lhs_var_id);
	    assert (index != INVALID_POSITION);

	    val = rul__dyar_get_nth (oin->value_bindings, index);
	    assert (rul__val_is_assignment (val));
	    val = rul__val_get_assignment_from (val);
	    rul__emit_value_reference (val, TRUE);

	} else if (is_2_in_node (SA_cur_gen_node)) {

	    /*
	    **  For 2-input nodes, the lhs variable could be:
	    **    - bound here (e.g. ^list[<prev-ce-index>])
	    **    - propagated here as one of the arguments to this function
	    **    - stored in the opposite beta memory
	    */

	    tin = (Net_2_In_Node) SA_cur_gen_node;
	    index = rul___net_get_var_position (tin->value_bindings, 
						lhs_var_id);
	    if (index != INVALID_POSITION) {

		/*  found it in the bindings list  */
	        val = rul__dyar_get_nth (tin->value_bindings, index);
		assert (rul__val_is_assignment (val));
		val = rul__val_get_assignment_from (val);
		rul__emit_value_reference (val, TRUE);

	    } else {
		/*  Check the other four possible value sources  */

		index = rul___net_get_var_position (tin->left_entry_hsh_values,
						    lhs_var_id);
		if (index != INVALID_POSITION) {
		    if (SA_cur_gen_is_left_side) {
		        gen_local_array_ref (IN_HSH_ARRAY_ARG, index);
		    } else {
			gen_beta_array_ref (CMP__E_FIELD_BETA_HSH_VEC, index);
		    }
		    return;
		}

		index = rul___net_get_var_position (tin->left_entry_aux_values, 
						    lhs_var_id);
		if (index != INVALID_POSITION) {
		    if (SA_cur_gen_is_left_side) {
		        gen_local_array_ref (IN_AUX_ARRAY_ARG, index);
		    } else {
			gen_beta_array_ref (CMP__E_FIELD_BETA_AUX_VEC, index);
		    }
		    return;
		}

		index = rul___net_get_var_position (tin->right_entry_hsh_values,
						    lhs_var_id);
		if (index != INVALID_POSITION) {
		    if (SA_cur_gen_is_left_side) {
			gen_beta_array_ref (CMP__E_FIELD_BETA_HSH_VEC, index);
		    } else {
		        gen_local_array_ref (IN_HSH_ARRAY_ARG, index);
		    }
		    return;
		}

		index = rul___net_get_var_position (tin->right_entry_aux_values,
						    lhs_var_id);
		if (index != INVALID_POSITION) {
		    if (SA_cur_gen_is_left_side) {
			gen_beta_array_ref (CMP__E_FIELD_BETA_AUX_VEC, index);
		    } else {
		        gen_local_array_ref (IN_AUX_ARRAY_ARG, index);
		    }
		    return;
		}

		assert (FALSE);  /*  should never get here  */
	    }
	} else {
	    assert (FALSE);  /* SA_cur_gen_node had bogus node */
	}
}






/*************************
**                      **
**  GEN_BETA_ARRAY_REF  **
**                      **
*************************/

static void gen_beta_array_ref (Rul_Internal_Field field, long index)
{
	Value lhs_var_val;

	assert (field == CMP__E_FIELD_BETA_HSH_VEC  ||
		field == CMP__E_FIELD_BETA_AUX_VEC);

	lhs_var_val = rul__val_create_blk_loc_fld_vec (
				CMP__E_INT_BETA_TOK, 
				BETA_TOK_ARG,
				field,
				index);

	rul__emit_value_reference (lhs_var_val, TRUE);
	rul__val_free_value (lhs_var_val);
}




/**************************
**                       **
**  GEN_LOCAL_ARRAY_REF  **
**                       **
**************************/

static void gen_local_array_ref (char *arg_name, long index)
{
	Value lhs_var_val;

	lhs_var_val = rul__val_create_blk_or_loc_vec (
				CMP__E_INT_MOLECULE,
				arg_name,
				index);

	rul__emit_value_reference (lhs_var_val, TRUE);
	rul__val_free_value (lhs_var_val);
}

