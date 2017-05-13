/****************************************************************************
**                                                                         **
**                C M P _ G E N _ R U L E _ R H S . C                      **
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
****************************************************************************/


/*
 * FACILITY:
 *	RULEWORKS compiler
 *
 * ABSTRACT:
 *	This module provides the code generation for rule RHS's
 *
 * MODIFIED BY:
 *	DEC	Digiatl Equipment Corporation
 *	CPQ	Compaq Computer Corporation
 *
 * MODIFICATION HISTORY:
 *
 *	16-Dec-1992	DEC	Initial version
 *	16-Feb-1998	DEC	class type changed to rclass
 *	01-Dec-1999	CPQ	Release with GPL
 */



#include <common.h>
#include <cmp_comm.h>
#include <cli.h>
#include <decl.h>
#include <ast.h>
#include <conrg.h>
#include <sem.h>
#include <val.h>
#include <gen.h>
#include <emit.h>
#include <mol.h>

static  Ast_Node_Type	SE_curr_construct_type;

static 	void		gen_emit_quit (Value ret_value, long nesting_level);
static 	void		gen_emit_return (Value ret_value, long nesting_level);
static  void	    	gen_emit_return_after_actions ();




/***************************
**                        **
**  GEN_RULE_RHS_ACTIONS  **
**                        **
***************************/

static Boolean gen_rule_rhs_actions (Ast_Node ast, long nesting_level)
{
    Ast_Node		curr_ast_node, node;
    Ast_Node_Type	node_type;
    Value	 	action_value;
    long         	tmp_mol_cnt;

    assert (ast != NULL);

    /*
    **  Loop through each of the actions in this linear sequence,
    **  and return TRUE if and only if the last action was RETURN
    */
    for (curr_ast_node = ast;
	 curr_ast_node != NULL;
	 curr_ast_node = rul__ast_get_sibling (curr_ast_node)) {

	action_value = (Value) rul__ast_get_value (curr_ast_node);
	node_type = rul__ast_get_type (curr_ast_node);

	switch (node_type) {


	    case AST__E_RETURN_ACTION:
		/*
		**  Don't bother to emit anything after a 
		**  RETURN action within the same code sequence.
		*/
		gen_emit_return (action_value, nesting_level);
		return TRUE;
		break;

	    case AST__E_QUIT_ACTION:
		/*
		**  Don't bother to emit anything after a 
		**  QUIT action within the same code sequence.
		*/
		gen_emit_quit (action_value, nesting_level);
		return FALSE;  /* for top level sequence, still gen a return */
		break;


	    case AST__E_DEBUG_ACTION:
	    case AST__E_TRACE_ACTION:
		/*
		**  Only generate these when /DEBUG was YES or MAYBE
		*/
		if (rul__cli_debug_option () != CLI_DEBUG_NO) {
		    rul__emit_blank_line ();
		    rul__emit_value (action_value);
		    rul__gen_decr_rhs_tmps ();
		} else {
		    rul__gen_decr_rhs_tmps ();
		}
		break;


		    
	    case AST__E_FOR_EACH_ACTION:
		/*
		** Generate FOR-EACH action
		*/
	    
		/* do pre-loop stuff */
		rul__emit_blank_line ();
		rul__emit_value (action_value);

		/* decr uses on tmp_mol's used to compute rhs_var */
		rul__gen_decr_rhs_tmps ();

		/* emit the while statement */
		node = rul__ast_get_child (curr_ast_node);
		action_value = (Value) rul__ast_get_value (node);
		rul__emit_begin_loop ((Relation_Operator) 
			rul__val_get_animal_long (
				  rul__val_get_nth_complex (action_value, 1)),
			rul__val_get_nth_complex (action_value, 2),
			rul__val_get_nth_complex (action_value, 3));

		/* emit assignments inside the while loop */
	        node = rul__ast_get_sibling (node);
	        action_value = (Value) rul__ast_get_value (node);
	        rul__emit_value (action_value);

	        /* now do the nested actions */
		(void)  gen_rule_rhs_actions (
				rul__ast_get_sibling (node),
				nesting_level + 1);

		/* do the end-loop stuff */
		rul__emit_end_loop();
		break;



	    case AST__E_IF_ACTION:
		/*
		** Generate IF action
		*/

		/* do pre-if stuff */
		rul__emit_blank_line ();
		if (action_value != NULL) rul__emit_value (action_value);
      
		/* emit the actual if statement */
		node = rul__ast_get_child (curr_ast_node);
		assert (rul__ast_get_type (node) == AST__E_COND_EXPR);
		rul__emit_cplx_conditional ();
		rul__emit_value_reference (
			(Value) rul__ast_get_value (node), TRUE);
 		rul__emit_close_cplx_conditions ();

		/* decr uses on tmp_mol's used in if statement */
		tmp_mol_cnt = rul__gen_decr_rhs_tmps ();

	        /* emit 'then' actions */
		node = rul__ast_get_sibling (node);
		(void)  gen_rule_rhs_actions (
				rul__ast_get_child (node), nesting_level + 1);

	        /* emit 'else' actions (if any) */
	        node = rul__ast_get_sibling (node);
	        if (node) {
		    rul__emit_else_condition ();
		    rul__gen_decr_rhs_tmps_count (tmp_mol_cnt);
		    (void)  gen_rule_rhs_actions (
				rul__ast_get_child (node), nesting_level + 1);
		} else if (tmp_mol_cnt) {
		    /*  Even when there is no user supplied else,
		    **  we need to decr uses on tmp_mol's used in
		    **  the if statement's condition
		    */
		    rul__emit_else_condition ();
		    rul__gen_decr_rhs_tmps_count (tmp_mol_cnt);
		}
		/* do the end-loop stuff */
		rul__emit_end_conditional ();
		break;



	    case AST__E_WHILE_ACTION:
		/*
		** Generate WHILE action
		*/

		/* do pre-if stuff */
		rul__emit_blank_line ();
		if (action_value != NULL) rul__emit_value (action_value);

		/* emit the while statement */
		node = rul__ast_get_child (curr_ast_node);
		rul__emit_begin_cplx_while ();
		rul__emit_value_reference (
			(Value) rul__ast_get_value (node), TRUE);
		rul__emit_close_cplx_conditions ();
      
		/* decr uses on tmp_mol's used in if statement */
		rul__gen_decr_rhs_tmps ();

		/* emit the 'do' actions */
		node = rul__ast_get_sibling (node);
		(void)  gen_rule_rhs_actions (
				rul__ast_get_child (node), nesting_level + 1);

		/* re-compute while params */
		if (action_value) {
		    rul__emit_blank_line ();
		    rul__emit_value (action_value);
		}

		/* do the end-loop stuff */
		rul__emit_end_conditional ();
		break;


	    case AST__E_SQL_FETCH_EACH_ACTION:
		/*
		**  Generate SQL-FETCH-EACH action
		*/
		/* do pre-loop stuff */
		rul__emit_blank_line ();
		rul__emit_value (action_value);
      
		/* first child node contains a complex value */
		/* must be one variable for fetch */
		node = rul__ast_get_child (curr_ast_node);
      
		/* emit the while statement */
		action_value = (Value) rul__ast_get_value (node);
		rul__emit_begin_loop (
			(Relation_Operator) rul__val_get_animal_long (
				  rul__val_get_nth_complex (action_value, 1)),
			rul__val_get_nth_complex (action_value, 2),
			rul__val_get_nth_complex (action_value, 3));

		/* decr uses on tmp_mol's used to compute rhs_var */
		rul__gen_decr_rhs_tmps ();

		/* now find the post-loop node (prior to action node) */
		while (rul__ast_get_type (node) != AST__E_SQL_RSE) {
		    node = rul__ast_get_sibling (node);
		}

		/* emit assignments inside the while loop */
		action_value = (Value) rul__ast_get_value (node);
		if (action_value != NULL) rul__emit_value (action_value);

		/* move to the SQL_ACTIONS node */
		node = rul__ast_get_sibling (node);
		(void)  gen_rule_rhs_actions (
				rul__ast_get_child (node), nesting_level + 1);

		/* do the end-loop stuff */
		rul__emit_end_loop();

		/* do post loop processing - fetch_cleanup */
		action_value = (Value) rul__ast_get_value (node);
		rul__emit_value (action_value);
		break;



	default:
		/*
		** Generate all other actions
		*/
		rul__emit_blank_line ();
		rul__emit_value (action_value);
		rul__gen_decr_rhs_tmps ();
		break;
	}
    }
    return FALSE;  /* leaving because we ran out of actions */
}



/************************
**                     **
**  RUL__GEN_ON_RHS    **
**                     **
************************/

void rul__gen_on_rhs (Ast_Node ast)
{
  Ast_Node	action_node;
  Boolean	did_return_already;

  SE_curr_construct_type = rul__ast_get_type (ast);

  action_node = rul__ast_get_child (ast);
  if (SE_curr_construct_type == AST__E_CATCH) {
    action_node = rul__ast_get_sibling (action_node);
  }

  did_return_already = gen_rule_rhs_actions (action_node, 0);

  /* emit function cleanup */
  if (! did_return_already) gen_emit_return_after_actions ();
  rul__sem_init_temp_rhs_vars ();
}


/************************
**                     **
**  RUL__GEN_RULE_RHS  **
**                     **
************************/

void rul__gen_rule_rhs (Ast_Node ast)
{
  Mol_Symbol	rule_name;
  Ast_Node	rhs_node, action_node;
  char	        buffer[RUL_C_MAX_SYMBOL_SIZE+34];
  Boolean	did_return_already;

  assert (rul__ast_get_type (ast) == AST__E_RULE);

  /* Emit RHS entry point */

  SE_curr_construct_type = AST__E_RULE;

  rule_name = (Mol_Symbol) rul__ast_get_value (rul__ast_get_child (ast));

  sprintf (buffer, "Rule RHS: %s",
	   rul__mol_get_printform (rule_name));
  rul__emit_real_comment (buffer);

  rul__emit_entry_point_str (rul__gen_get_cur_rhs_func (),
			     EMIT__E_SCOPE_STATIC,
			     EMIT__E_HAS_RETURN,
			     ext_type_long,
			     EXT__C_NOT_ARRAY,
			     ext_mech_value);
  rul__emit_entry_arg_internal (CMP__E_INT_CS_ENTRY,
				GEN__C_CS_ENTRY_PTR_STR, 
				EXT__C_NOT_ARRAY,
				ext_mech_value);
  rul__emit_entry_arg_internal (CMP__E_INT_ENTRY_DATA,
				"eb_data",
				EXT__C_NOT_ARRAY,
				ext_mech_value);
  rul__emit_entry_args_done (EMIT__E_ENTRY_DEFINITION);

  /* Emit RHS function local declarations and initializations */

  rul__gen_rhs_tmp_decl ();
  rul__gen_rhs_var_decl ();
  rul__gen_rhs_tmp_mol_inits ();
  rul__gen_rhs_var_inits ();

  /* emit rhs action code */
  rhs_node = rul__ast_get_sibling (
			   rul__ast_get_sibling (
					 rul__ast_get_child (ast)));

  assert (rul__ast_get_type (rhs_node) == AST__E_RULE_RHS);

  rul__emit_comment ("--- Start RHS Actions ---");
  did_return_already = gen_rule_rhs_actions (rul__ast_get_child (rhs_node), 0);

  if (! did_return_already) gen_emit_return_after_actions ();
  rul__sem_init_temp_rhs_vars ();
  rul__emit_entry_done ();
}



/******************************
**                           **
**  RUL__GEN_GENERIC_METHOD  **
**                           **
******************************/

void rul__gen_generic_method (Ast_Node ast)
{
  /* ??? any thing here ??? */
}


/**********************
**                   **
**  RUL__GEN_METHOD  **
**                   **
**********************/

void rul__gen_method (Ast_Node ast)
{
  Decl_Block  block = rul__decl_get_curr_decl_block ();
  Mol_Symbol  meth_name, class_name;
  Ast_Node    node, acc_node, class_node, ret_node, action_node;
  Class       rclass;
  Method      meth, gen_meth;
  char        buf[(RUL_C_MAX_SYMBOL_SIZE*2)+1];
  char        buf1[RUL_C_MAX_SYMBOL_SIZE+1];
  char        buf2[RUL_C_MAX_SYMBOL_SIZE+1];
  long        blk_idx;
  Decl_Shape  ret_shape;
  Decl_Domain ret_domain;
  Value       def_val, ret_val, asn_val;
  Molecule    ret_mol;
  Boolean     did_return_already;

  SE_curr_construct_type = AST__E_METHOD;

  node = rul__ast_get_child (ast); /* meth_name */
  meth_name = rul__ast_get_value (node);
  acc_node = rul__ast_get_sibling (node); /* accepts_decl */
  assert (rul__ast_get_type (acc_node) == AST__E_ACCEPTS_DECL);

  ret_node = rul__ast_get_sibling (acc_node); /* returns_decl */
  if (ret_node && rul__ast_get_type (ret_node) == AST__E_RETURNS_DECL) {
    action_node = rul__ast_get_sibling (ret_node);
    ret_node = rul__ast_get_child (ret_node);
  }
  else
    action_node = ret_node;

  acc_node = rul__ast_get_child (acc_node);

  assert (rul__ast_get_type (acc_node) == AST__E_METHOD_WHEN);

  /* checks for class in when_param */
  class_node = rul__ast_get_sibling (rul__ast_get_child (acc_node));
  class_name = rul__ast_get_value (class_node);
  rclass = rul__decl_get_class (block, class_name);

  meth = rul__decl_get_class_method (rclass, meth_name);

  /* emit a comment stating what this is */
  rul__mol_use_printform (meth_name, buf1, RUL_C_MAX_SYMBOL_SIZE);
  rul__mol_use_printform (class_name, buf2, RUL_C_MAX_SYMBOL_SIZE);
  sprintf (buf, "Method %s, Instance-of %s", buf1, buf2);
  rul__emit_real_comment (buf);

  sprintf (buf, "%s%ld_meth_%ld", GEN_NAME_PREFIX,
	   rul__conrg_get_cur_block_index (),
	   rul__decl_get_method_num (meth));

  rul__emit_entry_point_str (buf, EMIT__E_SCOPE_STATIC, TRUE,
			     ext_type_atom, EXT__C_NOT_ARRAY, ext_mech_value);

  rul__emit_entry_arg_internal (CMP__E_INT_LONG, GEN__C_METHOD_PARAM_CNT,
				EXT__C_NOT_ARRAY, ext_mech_value);
  rul__emit_entry_arg_internal (CMP__E_INT_MOLECULE, GEN__C_METHOD_PARAM_STR,
				EXT__C_NOT_ARRAY, ext_mech_ref_ro);
  rul__emit_entry_arg_internal (CMP__E_INT_ENTRY_DATA, GEN__C_EB_DATA_STR,
				EXT__C_NOT_ARRAY, ext_mech_value);
  rul__emit_entry_args_done (EMIT__E_ENTRY_DEFINITION);

  /*
   * return value setup...
   */
  gen_meth = rul__decl_get_generic_method (rclass, meth_name);
  ret_shape = rul__decl_get_method_par_shape (gen_meth,
			      rul__decl_get_method_num_params (gen_meth));
  rul__emit_comment ("Method return molecule");
  rul__emit_stack_internal (CMP__E_INT_MOLECULE, GEN__C_METHOD_RETURN,
			    EXT__C_NOT_ARRAY);
  rul__emit_blank_line ();

  rul__gen_rhs_tmp_decl ();
  rul__gen_rhs_var_decl ();
  rul__gen_rhs_tmp_mol_inits ();
  rul__gen_rhs_var_inits ();


  /* initialize return molecule */
  rul__emit_comment("Initialization of Method return");
  if (ret_shape == shape_compound)
    ret_mol = rul__mol_compound_empty ();

  else {
    ret_domain = rul__decl_get_method_par_domain (gen_meth,
			  rul__decl_get_method_num_params (gen_meth));
    switch (ret_domain) {
    case dom_any:
    case dom_symbol:
      ret_mol = rul__mol_symbol_nil ();
      break;
    case dom_number:
    case dom_int_atom:
      ret_mol = rul__mol_integer_zero ();
      break;
    case dom_dbl_atom:
      ret_mol = rul__mol_double_zero ();
      break;
    case dom_instance_id:
      ret_mol = rul__mol_instance_id_zero ();
      break;
    case dom_opaque:
      ret_mol = rul__mol_opaque_null ();
      break;
    }
  }

  def_val = rul__val_create_rul_constant (ret_mol);
  ret_val = rul__val_create_blk_or_loc (CMP__E_INT_MOLECULE,
					GEN__C_METHOD_RETURN);
  asn_val = rul__val_create_assignment (rul__val_copy_value (ret_val),
					def_val);
  rul__emit_value (asn_val);
  rul__val_free_value (asn_val);
  rul__emit_blank_line ();

  /* make sure we've got the correct node */

  assert (rul__ast_get_type (action_node) == AST__E_METHOD_RHS);


  rul__emit_comment ("--- Start Method Actions ---");
  did_return_already = 
	gen_rule_rhs_actions (rul__ast_get_child (action_node), 0);

  if (! did_return_already) gen_emit_return_after_actions ();
  rul__sem_init_temp_rhs_vars ();
  rul__val_free_value (ret_val);
  rul__emit_entry_done ();
}



/************************************
**                                 **
**  GEN_EMIT_RETURN_AFTER_ACTIONS  **
**                                 **
************************************/

static void
gen_emit_return_after_actions ()
    /*
    **  Called only for an action sequence that ended without a RETURN
    */
{
    Value ret_val;

    /*  First, do any necessary local cleanups
    */
    if (  SE_curr_construct_type == AST__E_METHOD || 
	  SE_curr_construct_type == AST__E_RULE   ||
	  SE_curr_construct_type == AST__E_CATCH) {
	rul__gen_decr_rhs_vars ();
    }

    /*  Now generate the actual C return statement
    */
    if (SE_curr_construct_type == AST__E_METHOD) {
	ret_val = rul__val_create_blk_or_loc (CMP__E_INT_MOLECULE,
					      GEN__C_METHOD_RETURN);
    } else {
        ret_val = rul__val_create_long_animal (RUL__C_SUCCESS);
    }
    rul__emit_return_value (ret_val);
    rul__val_free_value (ret_val);
}





/**********************
**                   **
**  GEN_EMIT_RETURN  **
**                   **
**********************/

static void
gen_emit_return (Value ret_value, long nesting_level)
    /*
    **  Emit the code for a RETURN action.
    **  It's a bit messy because of various cleanups that need to be
    **  done before the actual C return statement.
    */
{
    Value c_ret_val;

    assert (ret_value != NULL);
    rul__emit_blank_line ();

    if (ret_value && rul__val_get_next_value(ret_value) 
		  && rul__val_get_prev_value(ret_value)) {
	/*
	**  If there was an explicit RETURN argument,
	**  then we need to do the emit piece-meal
	*/
	rul__emit_value_setup (ret_value);
	rul__emit_value_statement (ret_value);
	/*  The next should always be the 'return' function  */
	assert (rul__val_is_function (rul__val_get_next_value(ret_value)));
    }

    /*
    **  Once the return value (if any) has been saved and its
    **  reference count incremented, emit the function cleanup.
    */
    rul__gen_decr_rhs_tmps ();

    /*
    **  Cleanup the named RHS variables, if needed
    */
    if (SE_curr_construct_type == AST__E_METHOD || 
	SE_curr_construct_type == AST__E_RULE   ||
	SE_curr_construct_type == AST__E_CATCH)
    {
	if (nesting_level == 0) {
	    /*
	    **  If at top-level, generate decr_uses,
	    **  and then clear the RHS variable table.
	    */
	    rul__gen_decr_rhs_vars ();
	} else {
	    /*
	    **  If not at top-level, just decr_uses and
	    **  do not clear the RHS variable table
	    */
	    rul__gen_decr_rhs_vars_simple ();
	}
    }

    /*
    **  Last, emit the actual C return statement
    */
    if (SE_curr_construct_type == AST__E_METHOD) {
	/*
	**  Methods are weird!.
	*/
	c_ret_val = rul__val_create_blk_or_loc (CMP__E_INT_MOLECULE,
					        GEN__C_METHOD_RETURN);
    } else {
	/*
	**  For on-clauses, catchers, and rule-rhs-functions,
	**  the actual return value is always the flag indicating
	**  to the recognize act cycle whether it should cause 
	**  the entry-block to return or continue.
	*/
	c_ret_val = rul__val_create_long_animal (RUL__C_RETURN);
    }
    rul__emit_return_value (c_ret_val);
    rul__val_free_value (c_ret_val);
}




/********************
**                 **
**  GEN_EMIT_QUIT  **
**                 **
********************/

static void gen_emit_quit (Value quit_value, long nesting_level)
{
    /*
    **  Emit the code for a QUIT action.
    **  It's a bit messy because of various cleanups that need to be
    **  done before the actual C return statement.
    */
    assert (quit_value != NULL);
    rul__emit_blank_line ();

    rul__emit_value_setup (quit_value);
    rul__gen_decr_rhs_tmps ();

    /*
    **  Cleanup the named RHS variables, if needed
    */
    if (SE_curr_construct_type == AST__E_METHOD || 
	SE_curr_construct_type == AST__E_RULE   ||
	SE_curr_construct_type == AST__E_CATCH)
    {
	if (nesting_level == 0) {
	    /*
	    **  If at top-level, generate decr_uses,
	    **  and then clear the RHS variable table.
	    */
	    rul__gen_decr_rhs_vars ();
	} else {
	    /*
	    **  If not at top-level, just decr_uses and
	    **  do not clear the RHS variable table
	    */
	    rul__gen_decr_rhs_vars_simple ();
	}
    }

    /*
    **  Last, emit the actual quit action
    */
    rul__emit_value_statement (quit_value);
}
