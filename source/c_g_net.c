/****************************************************************************
**                                                                         **
**                      C M P _ G E N _ N E T . C                          **
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
**                                                                         **
****************************************************************************/


/*
 * FACILITY:
 *	RULEWORKS compiler
 *
 * ABSTRACT:
 *	This module contains the high-level code generator
 *	for the discrimination network formed from the
 *	collected rule left-hand-sides.
 *
 * MODIFIED BY:
 *	DEC	Digiatl Equipment Corporation
 *	CPQ	Compaq Computer Corporation
 *
 * MODIFICATION HISTORY:
 *
 *	28-Jan-1993	DEC	Initial version
 *	01-Dec-1999	CPQ	Releasew with GPL
 */


#include <common.h>
#include <cmp_comm.h>
#include <conrg.h>
#include <decl.h>
#include <dyar.h>
#include <emit.h>
#include <gen.h>
#include <mol.h>
#include <sem.h>
#include <val.h>
#include <net.h>
#include <net_p.h>
#include <gennet_p.h>



static void gen_propagate_wm_functions (void);

static void gen_matches_function (void);

static void gen_do_class_dispatches (Net_Node decl_node);
static void gen_do_a_class_dispatch (Net_Node class_node, Boolean is_first);
static void gen_calls_to_downstream_oins (Net_Node class_node);

static void gen_arrange_var_passage (Net_Node node, Value lhs_var_val);
static void gen_arrange_all_var_paths (Net_Node node);

static void gen_node_func_decl (Net_Node node);

static void gen_oin_forward_decl (Net_Node node);
static void gen_oin_definition (Net_Node node);
static void gen_oin_node_comment (Net_Node node);
static void gen_oin_subclass_test (Class ce_class);
static void gen_oin_inst_array_init (void);

static void gen_unnamed_var_decls (Net_Node node);

static void gen_calls_to_downstream (Net_Node node, Boolean is_left_entry);
static void gen_vector_elem_assignment (char *target_vec_name, 
			long vec_index, Value lhs_var_value);
static void gen_a_tin_invocation (Net_2_In_Node tin,
			Boolean is_source_2_input, 
			Boolean is_source_positive, 
			Boolean is_left_entry_of_source, 
			Net_Link_Type type);

static void gen_tin_forward_decls (Net_Node node);
static void gen_tin_definitions (Net_Node node);
static void gen_tin_side_definition (Net_Node node, Boolean is_left_entry);
static void gen_tin_pos_body (Net_Node node, Boolean is_left_entry);
static void gen_tin_neg_left_body (Net_Node node);
static void gen_tin_neg_right_body (Net_Node node);

static void gen_tin_node_comment (Net_Node node);
static void gen_tin_beta_get_first (long opp_node_num, long entry_hsh_count);
static void gen_tin_inst_array_init (Net_Node node, Boolean is_left_entry);
static void gen_tin_neg_le_incr_match (void);
static void gen_tin_neg_re_if_prefix (void);
static void gen_tin_neg_re_if_match_count (Net_Node node);

static void gen_predicate_test_set (Net_Node node, 
			Net_Test first_test, Boolean is_left_entry,
			Boolean in_test_conjunction);
static void gen_one_predicate_test (Net_Test cur_test);
static void gen_one_predicate_condition (Net_Test cur_test);
static void gen_when_all_tests_passed (Net_Node node, Boolean is_left_entry);

static void gen_tin_beta_modify (long node_num, long entry_obj_count,
			long entry_hsh_count, long entry_aux_count);
static void gen_tin_beta_remove (long node_num, long entry_obj_count,
			long entry_hsh_count);
static void gen_tin_beta_add (long node_num, long entry_obj_count,
			long entry_hsh_count, long entry_aux_count);

static void gen_modify_cs_invocation (Net_CSE_Node cs_node, 
			Net_Node source_node, Boolean is_left_entry);

static char *gen_get_cur_css_name (void);
static char *gen_get_cur_beta_name (void);
static char *gen_get_cse_rhs_func_name (Net_CSE_Node cs_node);
static char *make_an_rhs_func_name (long rule_index);
static char *func_name_for_predicate (Pred_Type pred);

#define BUFFER_SIZE 50





/*******************************
**                            **
**  RUL__GEN_MATCH_TOP_DECLS  **
**                            **
*******************************/

void	rul__gen_match_top_decls (void)
	/*
	**  Called when a rule-block or an entry-block construct
	**  has been received and been semantically checked.
	*/
{
	char buffer[BUFFER_SIZE];
	Mol_Symbol name;
	Construct_Type type;

	name = rul__conrg_get_cur_block_name ();
	type = rul__conrg_get_block_type(name);

	/*
	**  Do various block-based initializations
	*/
	rul__sem_initialize_lhs_vars ();

	if (type == RUL__C_ENTRY_BLOCK) {
	  rul__emit_comment ("The shared return_value location");
	  rul__emit_stack_internal (CMP__E_INT_MOLECULE, 
				    rul__gen_get_cur_ret_val_name (),
				    EXT__C_NOT_ARRAY);
	}

	/*
	**  And generate various block-scoped declarations
	*/
	rul__emit_page_break ();
	rul__emit_blank_line ();
	rul__emit_comment ("The collected node memories for this ruling block");
	rul__emit_stack_internal (CMP__E_INT_BETA_SET, 
				  gen_get_cur_beta_name (),
				  EXT__C_NOT_ARRAY);

	rul__emit_blank_line ();
	rul__emit_comment ("The conflict subset for this ruling block");
	rul__emit_stack_internal (CMP__E_INT_CS,
				  gen_get_cur_css_name (),
				  EXT__C_NOT_ARRAY);

	rul__emit_blank_line ();
	rul__emit_comment ("Forward declaration for the matches function");
        sprintf (buffer, "%s%ld_matches", GEN_NAME_PREFIX, 
		rul__conrg_get_cur_block_index ());

	rul__emit_entry_point_str (buffer, EMIT__E_SCOPE_STATIC, 
				   EMIT__E_NO_RETURN, ext_type_invalid,
				   EXT__C_NOT_ARRAY, ext_mech_invalid);
	rul__emit_entry_arg_internal (CMP__E_INT_MOLECULE, RULE_NAME_ARG,
			EXT__C_NOT_ARRAY, ext_mech_value);
	rul__emit_entry_args_done (EMIT__E_ENTRY_DECLARATION);

	rul__emit_blank_line ();
}




/*****************************
**                          **
**  RUL__GEN_MATCH_NETWORK  **
**                          **
*****************************/

void rul__gen_match_network (void)
{
	char *env_val;

	if (rul__net_is_valid ()) {

	    /*      Do final setup before net generation  */
	    rul__net_propagate_node_info ();
	    rul__net_for_each_node (gen_arrange_all_var_paths, 
				    NET__C_POST_DIVE);

#ifndef NDEBUG
	    /*	    If appropriate, print out the network  */
	    env_val = getenv ("TIN_DEBUG_NET");
	    if (env_val  &&  env_val[0] != '\0') {
	        rul__net_print ();
	    }
#endif

	    /*      Start emitting  */

	    rul__emit_blank_line ();
	    rul__emit_comment ("Match node function forward declarations");
	    rul__net_for_each_node (gen_node_func_decl, NET__C_PRE_DIVE);

	    gen_matches_function ();

	    rul__net_for_each_node (gen_tin_definitions, NET__C_POST_DIVE);
	    rul__net_for_each_node (gen_oin_definition, NET__C_POST_DIVE);
	    gen_propagate_wm_functions ();
	}
}




/********************************
**                             **
**  GEN_ARRANGE_ALL_VAR_PATHS  **
**                             **
********************************/

static void gen_arrange_all_var_paths (Net_Node node)
{
	Net_2_In_Node tin;
	long var_count, i;
	Value lhs_var_val;

	if (is_2_in_node (node)) {

	    tin = (Net_2_In_Node) node;
	    var_count = rul__dyar_get_length (tin->vars_used_here);

	    for (i=0; i<var_count; i++) {
		lhs_var_val = (Value) 
				rul__dyar_get_nth (tin->vars_used_here, i);

		gen_arrange_var_passage (node, lhs_var_val);
	    }
	}
}




/******************************
**                           **
**  GEN_ARRANGE_VAR_PASSAGE  **
**                           **
******************************/

static void gen_arrange_var_passage (Net_Node node, Value lhs_var_val)
{
	Net_2_In_Node tin;
	Net_Node r_source, l_source;
	long source_count, j, lhs_var_id;

	lhs_var_id = rul__val_get_lhs_variable_id (lhs_var_val);
	if (rul___net_is_var_bound_here (node, lhs_var_id)) return;

	assert (is_2_in_node (node));
	tin = (Net_2_In_Node) node;

	/*
	**  Since this variable isn't bound here, we need to 
	**  find another node that is the source, and propagate
	**  the value between them;
	**
	**  Try the right parent (typically a 1-input node) first.
	*/
	r_source = rul___net_find_right_source (node);
	if (rul___net_is_var_bound_here (r_source, lhs_var_id)) {

	    /*  Found it in the (1 and only) right parent  */
	    if (! rul___net_is_var_in_hsh_values (node, lhs_var_id)) {
		/*
		**  If we aren't already stashing it in the hsh_values
		**  then stash it amoung the right-entry aux_values.
		*/
		rul___net_add_lhs_var_to_list ( &(tin->right_entry_aux_values), 
						lhs_var_val);
	    }

        } else {

	    if (! rul___net_is_var_in_hsh_values (node, lhs_var_id)) {
		/*
		**  If we aren't already stashing it in the hsh_values
		**  then stash it amoung the left-entry aux_values.
		*/
		rul___net_add_lhs_var_to_list ( &(tin->left_entry_aux_values), 
						lhs_var_val);
	    }

	    /*
	    **  If the variable did not come from the right
	    **  parent, then a path must be found from/through
	    **  ALL left parents (and there may be several).
	    */
	    source_count = rul__dyar_get_length (tin->source_nodes);
	    for (j=0; j<source_count; j++) {
		l_source = rul__dyar_get_nth (tin->source_nodes, j);
		if (l_source != r_source) {
		    gen_arrange_var_passage (l_source, lhs_var_val);
		}
	    }
	}
}




/************************************
**                                 **
**  RUL__GEN_PROP_FUNC_FORW_DECLS  **
**                                 **
************************************/

void rul__gen_prop_func_forw_decls (long number_of_assoc_dblocks)
{
	long index;
	char buffer[BUFFER_SIZE];

	for (index=1; index<=number_of_assoc_dblocks; index++) {
	    /* for each associated declaring block */

	    rul__emit_blank_line ();

            sprintf (buffer, "%s%ld_pr_db%ld", GEN_NAME_PREFIX, 
		rul__conrg_get_cur_block_index (),
		index);

	    rul__emit_entry_point_str (buffer, EMIT__E_SCOPE_STATIC, 
				       EMIT__E_NO_RETURN, ext_type_invalid,
				       EXT__C_NOT_ARRAY, ext_mech_invalid);
	    rul__emit_entry_arg_internal (CMP__E_INT_DELTA_TOK, DEL_TOK_ARG,
				EXT__C_NOT_ARRAY, ext_mech_value);
	    rul__emit_entry_args_done (EMIT__E_ENTRY_DECLARATION);
	}
}




/*********************************
**                              **
**  GEN_PROPAGATE_WM_FUNCTIONS  **
**                              **
*********************************/

static void gen_propagate_wm_functions (void)
{
  volatile	Net_Link lnk = rul___net_get_root_link ();
	long index = 1;
	char buffer[BUFFER_SIZE], *name;
	Net_Decl_Node db_node;
	Net_Class_Node dollar_root_family_node = NULL;

	if (lnk == NULL) return;

	db_node = (Net_Decl_Node)(lnk->target_node);
	if (db_node->declaring_block == NULL) {
	   /*  there were $root class tests found  */
	   assert (db_node->first_link != NULL);
	   assert (db_node->first_link->next_link == NULL);
	   assert (db_node->first_link->target_node != NULL);
	   dollar_root_family_node = 
			(Net_Class_Node) db_node->first_link->target_node;
	   /* skip this one for special processing later  */
	   lnk = lnk->next_link;
	}

	while (lnk) {
	    /* for each associated named declaring block */

	    rul__emit_page_break ();
	    rul__emit_blank_line ();

            sprintf (buffer, "%s%ld_pr_db%ld", GEN_NAME_PREFIX, 
		rul__conrg_get_cur_block_index (),
		index);

	    rul__emit_entry_point_str (buffer, EMIT__E_SCOPE_STATIC, 
				       EMIT__E_NO_RETURN, ext_type_invalid,
				       EXT__C_NOT_ARRAY, ext_mech_invalid);
	    rul__emit_entry_arg_internal (CMP__E_INT_DELTA_TOK, DEL_TOK_ARG,
					  EXT__C_NOT_ARRAY, ext_mech_value);
	    rul__emit_entry_args_done (EMIT__E_ENTRY_DEFINITION);

#ifndef NDEBUG
	    rul__emit_comment ("Used for initial WM propagation into the");
	    rul__emit_comment ("match network, for instances of classes ");
	    db_node = (Net_Decl_Node)(lnk->target_node);
	    name = rul__mol_get_printform (
			    rul__decl_get_decl_name (db_node->declaring_block));
	    sprintf (buffer, "declared in block, %s  ", name);
	    rul__emit_comment (buffer);
	    rul__mem_free (name);
#endif
	    rul__emit_blank_line ();

	    if (dollar_root_family_node != NULL) {
		/*  from each prop_db function forward to all $root oin's */
		rul__emit_comment ("Forward to all $ROOT 1-input nodes");
		gen_calls_to_downstream_oins (
				(Net_Node) dollar_root_family_node);
		rul__emit_blank_line ();
	    }

	    gen_do_class_dispatches (lnk->target_node);

	    rul__emit_entry_done ();

	    index++;
	    lnk = lnk->next_link;
	}
}




/******************************
**                           **
**  GEN_DO_CLASS_DISPATCHES  **
**                           **
******************************/

static void gen_do_class_dispatches (Net_Node decl_node)
{
        volatile Net_Link lnk;
	Boolean is_first = TRUE;

	assert (is_decl_node (decl_node));

	lnk = ((Net_Decl_Node)decl_node)->first_link;

	while (lnk != NULL) {
	    gen_do_a_class_dispatch (lnk->target_node, is_first);
	    if (is_first == TRUE) is_first = FALSE;
	    lnk = lnk->next_link;
	}

	if (((Net_Decl_Node)decl_node)->first_link != NULL) {
	    rul__emit_end_conditional (); /* emit close of all family tests */
	}
}	    





/******************************
**                           **
**  GEN_DO_A_CLASS_DISPATCH  **
**                           **
******************************/

static void gen_do_a_class_dispatch (Net_Node net_node, Boolean is_first)
{
        volatile Net_Link lnk;
	Boolean first_child = TRUE;
	Net_Class_Node class_node;
	Value val1, val2, deltas_class_val, this_class_val;
	Relation_Operator rel;
	char name[RUL_C_MAX_SYMBOL_SIZE+1];
	char comment[2*RUL_C_MAX_SYMBOL_SIZE+1];

	if (net_node == NULL) return;
	assert (is_class_node (net_node));

	class_node = (Net_Class_Node) net_node;
	
	/*
	**  Emit the test for this class
	*/

	/*  Create the class_constant Value  */
	this_class_val = rul__val_create_class_constant (
				class_node->this_class);

	/*  Create the value for the class contained in the delta token  */
	deltas_class_val = rul__val_create_blk_or_loc_fld (
				CMP__E_INT_DELTA_TOK, DEL_TOK_ARG,
				CMP__E_FIELD_DELTA_CLASS);

	if (rul__decl_is_leaf_class (class_node->this_class)) {
	    val1 = deltas_class_val;
	    rel = EMIT__E_REL_EQ;
	    val2 = this_class_val;
	} else {
	    val1 = rul__val_create_function_str (
				"rul__decl_is_subclass_of", 2);
	    rul__val_set_nth_subvalue (val1, 1, deltas_class_val);
	    rul__val_set_nth_subvalue (val1, 2, this_class_val);
	    rel = EMIT__E_REL_NONE;
	    val2 = NULL;
	}

	if (is_first) {
	    rul__emit_begin_conditional (rel, val1, val2);
	} else {
	    rul__emit_blank_line ();
	    rul__emit_else_if_condition (rel, val1, val2);
	}
	rul__val_free_value (val1);
	if (val2) rul__val_free_value (val2);

	rul__emit_blank_line ();

#ifndef NDEBUG
	rul__mol_use_printform (
			rul__decl_get_class_name (class_node->this_class), 
			name, RUL_C_MAX_SYMBOL_SIZE + 1);
	sprintf (comment, "Class:      %s", name);
	rul__emit_comment (comment);
#endif

	/*
	**  Once done with this class test, generate calls to 
	**  downstream 1-in-nodes, and then any nested class tests.
	*/
	gen_calls_to_downstream_oins ((Net_Node) class_node);

	lnk = class_node->first_link;
	while (lnk != NULL) {
	    if (is_class_node (lnk->target_node)) {
		gen_do_a_class_dispatch (lnk->target_node, first_child);
		if (first_child == TRUE) first_child = FALSE;
	    }
	    lnk = lnk->next_link;
	}
	if (first_child == FALSE) {
	    rul__emit_end_conditional (); /* close nested class tests */
	}

}




/***********************************
**                                **
**  GEN_CALLS_TO_DOWNSTREAM_OINS  **
**                                **
***********************************/

static void gen_calls_to_downstream_oins (Net_Node class_node)
{
  volatile	Net_Link lnk;
	Net_1_In_Node oin;
	Value func_val;
	char buffer[BUFFER_SIZE];
	long bl_num;

	assert (is_class_node (class_node));
	bl_num = rul__conrg_get_cur_block_index ();

	lnk = ((Net_Class_Node)class_node)->first_link;

	while (lnk) {

	    if (is_1_in_node (lnk->target_node)) {
	        oin = (Net_1_In_Node) lnk->target_node;

                sprintf (buffer, "%s%ld_%ld", GEN_NAME_PREFIX, 
			 bl_num, oin->node_number);

	        func_val = rul__val_create_function_str (buffer, 1);
	        rul__val_set_nth_subvalue (func_val, 1,
			rul__val_create_blk_or_loc (CMP__E_INT_DELTA_TOK,
				DEL_TOK_ARG));
	        rul__emit_value_statement (func_val);
	        rul__val_free_value (func_val);
	    }
	    lnk = lnk->next_link;
	}
}




/*************************
**                      **
**  GEN_NODE_FUNC_DECL  **
**                      **
*************************/

static void gen_node_func_decl (Net_Node node)
{
	assert (rul___net_node_is_valid (node));

/*	Reordered node function generation, 
	so we don't need the node function protypes anymore ... 

	if (is_1_in_node (node)) {
	    gen_oin_forward_decl (node);
	} else if (is_2_in_node (node)) {
	    gen_tin_forward_decls (node);
	}
*/
}



/****************************** 1-Input Nodes ******************************/



/***************************
**                        **
**  GEN_OIN_FORWARD_DECL  **
**                        **
***************************/

static void gen_oin_forward_decl (Net_Node node)
{
	char buffer[BUFFER_SIZE];
	long bl_num;

	assert (is_1_in_node(node));

	bl_num = rul__conrg_get_cur_block_index ();

        sprintf (buffer, "%s%ld_%ld", GEN_NAME_PREFIX, bl_num,
		 node->node_number);

	rul__emit_entry_point_str (buffer, EMIT__E_SCOPE_STATIC, 
				   EMIT__E_NO_RETURN, ext_type_invalid,
				   EXT__C_NOT_ARRAY, ext_mech_invalid);
	rul__emit_entry_arg_internal (CMP__E_INT_DELTA_TOK, DEL_TOK_ARG,
				      EXT__C_NOT_ARRAY, ext_mech_value);
	rul__emit_entry_args_done (EMIT__E_ENTRY_DECLARATION);
}




/*************************
**                      **
**  GEN_OIN_DEFINITION  **
**                      **
*************************/

static void gen_oin_definition (Net_Node node)
{
	char buffer[BUFFER_SIZE];
	long bl_num, i;
	Net_1_In_Node oin;

	assert (rul___net_node_is_valid (node));
	if (! is_1_in_node(node)) return;

	oin = (Net_1_In_Node) node;

	rul__emit_page_break ();
	rul__emit_blank_line ();

	bl_num = rul__conrg_get_cur_block_index ();

        sprintf (buffer, "%s%ld_%ld", GEN_NAME_PREFIX, bl_num,
		 node->node_number);

	rul__emit_entry_point_str (buffer, EMIT__E_SCOPE_STATIC, 
				   EMIT__E_NO_RETURN, ext_type_invalid,
				   EXT__C_NOT_ARRAY, ext_mech_invalid);
	rul__emit_entry_arg_internal (CMP__E_INT_DELTA_TOK, DEL_TOK_ARG,
				      EXT__C_NOT_ARRAY, ext_mech_value);
	rul__emit_entry_args_done (EMIT__E_ENTRY_DEFINITION);

	gen_oin_node_comment (node);

	/*  Emit local declarations.  */
	rul___gen_lhs_var_set_cur_node (node);

	/*  Always emit the inst_array  */
	rul__emit_stack_internal (CMP__E_INT_MOLECULE, OUT_INST_ARRAY_ARG, 1);
		/*?*  What inst_array size for CE disjunctions ?  *?*/

	/*  If needed emit decls for aux_array, and hash_array  */
	i = rul__net_max_hsh_in_a_child (node);
	if (i > 0) {
	    rul__emit_stack_internal (CMP__E_INT_MOLECULE, 
					OUT_HSH_ARRAY_ARG, i);
	}
	i = rul__net_max_aux_in_a_child (node);
	if (i > 0) {
	    rul__emit_stack_internal (CMP__E_INT_MOLECULE, 
					OUT_AUX_ARRAY_ARG, i);
	}
	gen_unnamed_var_decls (node);
	rul__gen_lhs_tmp_mol_inits ();
	rul__emit_blank_line ();
	/*  Done with local decls  */

	/*
	**  Generate the 1-input tests, and the conditional calls 
	**  to all the downstream 2-input nodes.
	*/
	gen_predicate_test_set (node, oin->first_test, TRUE, FALSE);

	rul__gen_decr_lhs_tmps ();
	rul___gen_lhs_var_set_cur_node (NULL);
	rul__emit_entry_done ();
}



/****************************
**                         **
**  GEN_OIN_SUBCLASS_TEST  **
**                         **
****************************/

static void gen_oin_subclass_test (Class ce_class)
{
	Value ce_class_val, instance_class_val, val1, val2;
	Relation_Operator rel;

	/*
	**	Gen the following for leaf class tests:
	**
	**	    if (del_tok->instance_class == SA_bl1_classes[11]) {
	**
	**	Gen the following for non-leaf class tests:
	**
	**	    if (rul__decl_is_subclass_of (
	**			del_tok->instance_class,
	**			SA_bl1_classes[11]) {
	*/

	/*  	Create the fetch of the class from the Delta_Token  */
	instance_class_val = rul__val_create_blk_or_loc_fld (
					CMP__E_INT_DELTA_TOK, DEL_TOK_ARG,
					CMP__E_FIELD_DELTA_CLASS);

	/*	Create the class_constant Value  */
	ce_class_val = rul__val_create_class_constant (ce_class);

	if (rul__decl_is_leaf_class (ce_class)) {
	    /*	    For leaf classes, generate the == test  */
	    val1 = instance_class_val;
	    rel = EMIT__E_REL_EQ;
	    val2 = ce_class_val;

	} else {
	    /*      For non-leaf classes generate the ..._is_subclass  */
	    val1 = rul__val_create_function_str (
				"rul__decl_is_subclass_of", 2);
	    rul__val_set_nth_subvalue (val1, 1, instance_class_val);
	    rul__val_set_nth_subvalue (val1, 2, ce_class_val);
	    rel = EMIT__E_REL_NONE;
	    val2 = NULL;
	}
	rul__emit_begin_conditional (rel, val1, val2);
	rul__val_free_value (val1);
	if (val2 != NULL) rul__val_free_value (val2);
}




/***************************
**                        **
**  GEN_OIN_NODE_COMMENT  **
**                        **
***************************/

static void gen_oin_node_comment (Net_Node node)
{
	Net_1_In_Node oin;
	char name[RUL_C_MAX_SYMBOL_SIZE+1];
	char comment[2*RUL_C_MAX_SYMBOL_SIZE+1];

	oin = (Net_1_In_Node) node;

	rul__mol_use_printform (node->source_rule, name, 
				RUL_C_MAX_SYMBOL_SIZE + 1);
	sprintf (comment, "Rule:   %s  (ce %ld)", name, oin->ce_index);
	rul__emit_real_comment (comment);

	rul__mol_use_printform (rul__decl_get_class_name (oin->ce_class), 
				name, RUL_C_MAX_SYMBOL_SIZE + 1);
	sprintf (comment, "Class:  %s", name);
	rul__emit_comment (comment);
	rul__emit_blank_line ();
}




/******************************
**                           **
**  GEN_OIN_INST_ARRAY_INIT  **
**                           **
******************************/

static void gen_oin_inst_array_init (void)
{
	Value assign_inst;

	/*?*  Do we need to do anything different for CE disj's ? */

	/*  Emit the instance id assignment  */
	assign_inst = 
		rul__val_create_assignment (
			rul__val_create_blk_or_loc_vec (
				CMP__E_INT_MOLECULE, OUT_INST_ARRAY_ARG, 0),
			rul__val_create_blk_or_loc_fld (
				CMP__E_INT_DELTA_TOK, DEL_TOK_ARG,
				CMP__E_FIELD_DELTA_INST_ID));
	rul__emit_value_statement (assign_inst);
	rul__val_free_value (assign_inst);
}




/****************************
**                         **
**  GEN_UNNAMED_VAR_DECLS  **
**                         **
****************************/

static void gen_unnamed_var_decls (Net_Node node)
{
  volatile	Net_Test test = NULL;

	if (is_1_in_node(node)) {
	    test = ((Net_1_In_Node)node)->first_test;
	} else if (is_2_in_node(node)) {
	    test = ((Net_2_In_Node)node)->first_test;
	} else assert (FALSE);

	while (test) {
	    /*
	    **  For each test, check it's values for unnamed variables
	    */
	    rul__val_register_lhs_tmp_vars (test->value_1);
	    rul__val_register_lhs_tmp_vars (test->value_2);

	    test = test->next_test;
	}

	/*
	**  Now generate declarations for the accumulated set
	**  of unnamed variables.
	*/
	rul__gen_lhs_tmp_decl ();
}



/****************************** Net_Test's ******************************/



/*****************************
**                          **
**  GEN_PREDICATE_TEST_SET  **
**                          **
*****************************/

static void gen_predicate_test_set (Net_Node node, 
			Net_Test cur_test, Boolean is_left_entry,
			Boolean in_test_conjunction)
{
	if (cur_test == NULL) {

	    /*  No mode tests  */
	    if (in_test_conjunction) rul__emit_close_cplx_conditions ();
	    gen_when_all_tests_passed (node, is_left_entry);

	} else {

	    if (in_test_conjunction) {

		/*  If we're in a conjunction, see if we can continue it.  */
	        if (rul__net_test_is_simple (cur_test)) {

		    gen_one_predicate_condition (cur_test);
	    	    gen_predicate_test_set (node, cur_test->next_test,
					    is_left_entry, TRUE);
		    return;

		} else {
		    rul__emit_close_cplx_conditions ();
		}
	    }

	    /*  If we're not in a conjunction, see if we should start one  */

	    if (rul__net_test_is_simple (cur_test)  &&
		cur_test->next_test  &&
		rul__net_test_is_simple (cur_test->next_test)) {

		rul__emit_cplx_conditional ();
		gen_one_predicate_condition (cur_test);
	        gen_predicate_test_set (node, cur_test->next_test,
					is_left_entry, TRUE);

		rul__emit_end_conditional ();

	    } else {

		/*  No conjunction, so it with seperate conditionals  */

		/*  Emit the setup code for this test, if any  */
		rul__emit_value_setup (cur_test->value_1);
		rul__emit_value_setup (cur_test->value_2);

		/*  Emit the conditional and it's condition  */
		rul__emit_blank_line ();
		gen_one_predicate_test (cur_test);

		/*  Embed additional tests within this conditional  */
		gen_predicate_test_set (node, cur_test->next_test,
					is_left_entry, FALSE);

		rul__emit_end_conditional (); /* emit close of this test */

		/*  Emit the cleanup code for this test, if any  */
		rul__emit_value_cleanup (cur_test->value_1);
		rul__emit_value_cleanup (cur_test->value_2);
	    }
	}
}



/*****************************
**                          **
**  GEN_ONE_PREDICATE_TEST  **
**                          **
*****************************/

static void gen_one_predicate_test (Net_Test cur_test)
{
	Value     func_val;
	Pred_Type tmp_pred;

	if (cur_test->predicate == PRED__E_EQ &&
	    cur_test->opt_pred == PRED__E_NOT_A_PRED) {
	    rul__emit_begin_conditional (EMIT__E_REL_EQ,
			cur_test->value_1, cur_test->value_2);

	} else if (cur_test->predicate == PRED__E_NOT_EQ &&
	    cur_test->opt_pred == PRED__E_NOT_A_PRED) {
	    rul__emit_begin_conditional (EMIT__E_REL_NEQ,
			cur_test->value_1, cur_test->value_2);
	    
	} else {
	    /*	    Need to use a predicate function  */

	    if (cur_test->predicate == PRED__E_CONTAINS ||
		cur_test->predicate == PRED__E_DOES_NOT_CONTAIN ||
		cur_test->opt_pred == PRED__E_CONTAINS ||
		cur_test->opt_pred == PRED__E_DOES_NOT_CONTAIN) {

	        if (cur_test->opt_pred == PRED__E_CONTAINS ||
		    cur_test->opt_pred == PRED__E_DOES_NOT_CONTAIN) {

		    tmp_pred = cur_test->predicate;
		    cur_test->predicate = cur_test->opt_pred;
		    cur_test->opt_pred = tmp_pred;
		}

		func_val = rul__val_create_function_str (
			    func_name_for_predicate (cur_test->predicate), 3);

		if (cur_test->opt_pred == PRED__E_EQ ||
		    cur_test->opt_pred == PRED__E_NOT_A_PRED)
		    rul__val_set_nth_subvalue (func_val, 3,
			       rul__val_create_blk_or_loc (CMP__E_INT_MOLECULE,
							   "NULL"));
		else
		    rul__val_set_nth_subvalue (func_val, 3,
		         rul__val_create_blk_or_loc (CMP__E_INT_MOLECULE,
                               rul__gen_pred_func_name (cur_test->opt_pred)));

	    }
	    else {
	        func_val = rul__val_create_function_str (
			    func_name_for_predicate (cur_test->predicate), 2);
	    }

	    rul__val_set_nth_subvalue (func_val, 1, cur_test->value_1);
	    rul__val_set_nth_subvalue (func_val, 2, cur_test->value_2);

	    rul__emit_begin_conditional (EMIT__E_REL_NONE, func_val, NULL);

	    /*	    Now cleanup  */
	    rul__val_set_nth_subvalue (func_val, 1, NULL);
	    rul__val_set_nth_subvalue (func_val, 2, NULL);
	    rul__val_free_value (func_val);
	}
}




/**********************************
**                               **
**  GEN_ONE_PREDICATE_CONDITION  **
**                               **
**********************************/

static void gen_one_predicate_condition (Net_Test cur_test)
{
	Value     func_val;
	Pred_Type tmp_pred;

	if (cur_test->predicate == PRED__E_EQ &&
	    cur_test->opt_pred == PRED__E_NOT_A_PRED) {
	    rul__emit_cplx_condition (EMIT__E_REL_EQ,
			cur_test->value_1, cur_test->value_2);

	} else if (cur_test->predicate == PRED__E_NOT_EQ &&
	    cur_test->opt_pred == PRED__E_NOT_A_PRED) {
	    rul__emit_cplx_condition (EMIT__E_REL_NEQ,
			cur_test->value_1, cur_test->value_2);
	    
	} else {
	    /*	    Need to use a predicate function  */

            if (cur_test->predicate == PRED__E_CONTAINS ||
                cur_test->predicate == PRED__E_DOES_NOT_CONTAIN ||
                cur_test->opt_pred == PRED__E_CONTAINS ||
                cur_test->opt_pred == PRED__E_DOES_NOT_CONTAIN) {

                if (cur_test->opt_pred == PRED__E_CONTAINS ||
                    cur_test->opt_pred == PRED__E_DOES_NOT_CONTAIN) {

                    tmp_pred = cur_test->predicate;
                    cur_test->predicate = cur_test->opt_pred;
                    cur_test->opt_pred = tmp_pred;
                }

                func_val = rul__val_create_function_str (
                            func_name_for_predicate (cur_test->predicate),
							 3);

                if (cur_test->opt_pred == PRED__E_EQ ||
                    cur_test->opt_pred == PRED__E_NOT_A_PRED)
                    rul__val_set_nth_subvalue (func_val, 3,
                               rul__val_create_blk_or_loc
					       (CMP__E_INT_MOLECULE, "NULL"));
                else
                    rul__val_set_nth_subvalue (func_val, 3,
                          rul__val_create_blk_or_loc (CMP__E_INT_MOLECULE,
                                rul__gen_pred_func_name (cur_test->opt_pred)));
            }
            else {
                func_val = rul__val_create_function_str (
                            func_name_for_predicate (cur_test->predicate), 2);
            }

	    rul__val_set_nth_subvalue (func_val, 1, cur_test->value_1);
	    rul__val_set_nth_subvalue (func_val, 2, cur_test->value_2);

	    rul__emit_cplx_condition (EMIT__E_REL_NONE, func_val, NULL);

	    /*	    Now cleanup  */
	    rul__val_set_nth_subvalue (func_val, 1, NULL);
	    rul__val_set_nth_subvalue (func_val, 2, NULL);
	    rul__val_free_value (func_val);
	}
}




/********************************
**                             **
**  GEN_WHEN_ALL_TESTS_PASSED  **
**                             **
********************************/

static void gen_when_all_tests_passed (Net_Node node, Boolean is_left_entry)
{
	/*
	**  No more tests.  Generate innermost nested actions
	*/
	if (is_1_in_node (node)  ||  is_2_in_pos_node (node)) {

	    /*  For all positive nodes, call downstream nodes  */
	    rul__emit_blank_line ();
	    rul__emit_comment ("Initialize the outbound id array");
	    if (is_1_in_node (node)) {
		gen_oin_inst_array_init ();
	    } else {
		gen_tin_inst_array_init (node, is_left_entry);
	    }
	    rul__emit_blank_line ();
	    rul__emit_comment ("Call all downstream nodes");
	    gen_calls_to_downstream (node, is_left_entry);

	} else {

	    assert (is_2_in_neg_node (node));
	    if (is_left_entry) {
		/*  left-entry negated  */
		gen_tin_neg_le_incr_match ();
	    } else {
		/*  right-entry negated  */
		gen_tin_neg_re_if_match_count (node);
	    }
	}
}


/****************************** 2-Input Nodes ******************************/




/****************************
**                         **
**  GEN_TIN_FORWARD_DECLS  **
**                         **
****************************/

static void gen_tin_forward_decls (Net_Node node)
{
	char buffer[BUFFER_SIZE];
	long bl_num;
	Net_2_In_Node tin;

	assert (is_2_in_node (node));
	tin = (Net_2_In_Node) node;

	bl_num = rul__conrg_get_cur_block_index ();

	/*
	**  Left-Entry for this 2-input node
	*/
        sprintf (buffer, "%s%ld_%ldl", GEN_NAME_PREFIX, 
		bl_num, node->node_number);

	rul__emit_entry_point_str (buffer, EMIT__E_SCOPE_STATIC, 
				   EMIT__E_NO_RETURN, ext_type_invalid,
				   EXT__C_NOT_ARRAY, ext_mech_invalid);
	rul__emit_entry_arg_internal (CMP__E_INT_DEL_TOK_SIGN, DEL_SIGN_ARG,
				      EXT__C_NOT_ARRAY, ext_mech_value);
	rul__emit_entry_arg_internal (CMP__E_INT_MOLECULE, IN_INST_ARRAY_ARG,
				      EXT__C_IMPLICIT_ARRAY, ext_mech_value);
	if (rul__dyar_get_length (tin->left_entry_hsh_values)  > 0) {
	    rul__emit_entry_arg_internal(CMP__E_INT_MOLECULE,IN_HSH_ARRAY_ARG,
					 EXT__C_IMPLICIT_ARRAY,ext_mech_value);
	}
	if (rul__dyar_get_length (tin->left_entry_aux_values)  > 0) {
	    rul__emit_entry_arg_internal(CMP__E_INT_MOLECULE,IN_AUX_ARRAY_ARG,
					 EXT__C_IMPLICIT_ARRAY,ext_mech_value);
	}
	rul__emit_entry_args_done (EMIT__E_ENTRY_DECLARATION);

	/*
	**  Right-Entry for this 2-input node
	*/
        sprintf (buffer, "%s%ld_%ldr", GEN_NAME_PREFIX, 
		bl_num, node->node_number);

	rul__emit_entry_point_str (buffer, EMIT__E_SCOPE_STATIC, 
				   EMIT__E_NO_RETURN, ext_type_invalid,
				   EXT__C_NOT_ARRAY, ext_mech_invalid);
	rul__emit_entry_arg_internal (CMP__E_INT_DEL_TOK_SIGN, DEL_SIGN_ARG,
				      EXT__C_NOT_ARRAY, ext_mech_value);
	rul__emit_entry_arg_internal (CMP__E_INT_MOLECULE, IN_INST_ARRAY_ARG,
				      EXT__C_IMPLICIT_ARRAY, ext_mech_value);
	if (rul__dyar_get_length (tin->right_entry_hsh_values)  > 0) {
	    rul__emit_entry_arg_internal(CMP__E_INT_MOLECULE,IN_HSH_ARRAY_ARG,
					 EXT__C_IMPLICIT_ARRAY,ext_mech_value);
	}
	if (rul__dyar_get_length (tin->right_entry_aux_values)  > 0) {
	    rul__emit_entry_arg_internal(CMP__E_INT_MOLECULE,IN_AUX_ARRAY_ARG,
					 EXT__C_IMPLICIT_ARRAY,ext_mech_value);
	}
	rul__emit_entry_args_done (EMIT__E_ENTRY_DECLARATION);
}




/**************************
**                       **
**  GEN_TIN_DEFINITIONS  **
**                       **
**************************/

static void gen_tin_definitions (Net_Node node)
{
	assert (rul___net_node_is_valid (node));
	if (! is_2_in_node(node)) return;

	gen_tin_side_definition (node, TRUE  /* left entry */);
	gen_tin_side_definition (node, FALSE /* right entry */);
}




/******************************
**                           **
**  GEN_TIN_SIDE_DEFINITION  **
**                           **
******************************/

static void gen_tin_side_definition (Net_Node node, Boolean is_left_entry)
	/*
	**  Generates the function definition line, and the
	**  declarations that all TIN entries contain.
	*/
{
	char buffer[BUFFER_SIZE];
	long bl_num, entry_hsh, entry_aux;
	char *side;
	Net_2_In_Node tin;

	assert (rul___net_node_is_valid (node));
	tin = (Net_2_In_Node) node;
	entry_hsh = rul__dyar_get_length (tin->left_entry_hsh_values);
	assert (entry_hsh == rul__dyar_get_length(tin->right_entry_hsh_values));

	if (is_left_entry) {
	    side = "l";
	    entry_aux = rul__dyar_get_length (tin->left_entry_aux_values);
	} else {
	    side = "r";
	    entry_aux = rul__dyar_get_length (tin->right_entry_aux_values);
	}

	bl_num = rul__conrg_get_cur_block_index ();

	/*
	**  One of the 2 entries for this 2-input node
	*/
	rul__emit_page_break ();
	rul__emit_blank_line ();

        sprintf (buffer, "%s%ld_%ld%s", GEN_NAME_PREFIX, 
		bl_num, node->node_number, side);

	rul__emit_entry_point_str (buffer, EMIT__E_SCOPE_STATIC, 
				   EMIT__E_NO_RETURN, ext_type_invalid,
				   EXT__C_NOT_ARRAY, ext_mech_invalid);
	rul__emit_entry_arg_internal (CMP__E_INT_DEL_TOK_SIGN, DEL_SIGN_ARG,
				      EXT__C_NOT_ARRAY, ext_mech_value);
	rul__emit_entry_arg_internal (CMP__E_INT_MOLECULE, IN_INST_ARRAY_ARG,
				      EXT__C_IMPLICIT_ARRAY, ext_mech_value);
	if (entry_hsh > 0) {
	    rul__emit_entry_arg_internal(CMP__E_INT_MOLECULE, IN_HSH_ARRAY_ARG,
					 EXT__C_IMPLICIT_ARRAY,ext_mech_value);
	}
	if (entry_aux > 0) {
	    rul__emit_entry_arg_internal(CMP__E_INT_MOLECULE,IN_AUX_ARRAY_ARG,
					 EXT__C_IMPLICIT_ARRAY,ext_mech_value);
	}
	rul__emit_entry_args_done (EMIT__E_ENTRY_DEFINITION);
	rul___gen_lhs_var_set_cur_node (node);
	rul___gen_lhs_var_set_cur_side (is_left_entry);

	gen_tin_node_comment (node);

	/*
	**  Local declarations:
	**		Always emit the beta token.
	*/
	rul__emit_stack_internal (CMP__E_INT_BETA_TOK, 
					BETA_TOK_ARG, EXT__C_NOT_ARRAY);

	if (is_2_in_pos_node (node)) {
	    gen_tin_pos_body (node, is_left_entry);
	} else {
	    if (is_left_entry) {
	        gen_tin_neg_left_body (node);
	    } else {
	        gen_tin_neg_right_body (node);
	    }
	}

	rul___gen_lhs_var_set_cur_node (NULL);
	rul__emit_entry_done ();
}



/***********************
**                    **
**  GEN_TIN_POS_BODY  **
**                    **
***********************/

static void  gen_tin_pos_body (Net_Node node, Boolean is_left_entry)
{
	long node_num, opp_node_num, i;
	long entry_obj, entry_hsh, entry_aux, out_obj_count;
	Net_2_In_Node tin;
	Value bt_val, func_val, assn_val;

	assert (rul___net_node_is_valid (node));
	tin = (Net_2_In_Node) node;
	entry_hsh = rul__dyar_get_length (tin->left_entry_hsh_values);

	if (is_left_entry) {
	    node_num = tin->node_number;
	    opp_node_num = tin->node_number + 1;
	    entry_obj = tin->left_entry_obj_count;
	    entry_aux = rul__dyar_get_length (tin->left_entry_aux_values);
	} else {
	    node_num = tin->node_number + 1;
	    opp_node_num = tin->node_number;
	    entry_obj = tin->right_entry_obj_count;
	    entry_aux = rul__dyar_get_length (tin->right_entry_aux_values);
	}

	/*  Finish the local declarations  */
	out_obj_count = tin->left_entry_obj_count + 
			    tin->right_entry_obj_count;
	rul__emit_stack_internal (CMP__E_INT_MOLECULE, 
				OUT_INST_ARRAY_ARG, out_obj_count);
	/*?*  What inst_array size for CE disjunctions ?  *?*/

	/*  If needed emit decls for aux_array, and hash_array  */
	i = rul__net_max_hsh_in_a_child (node);
	if (i > 0) {
	    rul__emit_stack_internal (CMP__E_INT_MOLECULE, 
					OUT_HSH_ARRAY_ARG, i);
	}
	i = rul__net_max_aux_in_a_child (node);
	if (i > 0) {
	    rul__emit_stack_internal (CMP__E_INT_MOLECULE, 
					OUT_AUX_ARRAY_ARG, i);
	}
	gen_unnamed_var_decls (node);
	rul__emit_blank_line ();
	/*  Done with local decls  */


	rul__emit_comment ("Store or remove token from local beta memory");
	gen_tin_beta_modify (node_num, entry_obj, entry_hsh, entry_aux);
	rul__emit_blank_line ();

	rul__emit_comment ("Iterate over opposite side memory of this node");
	gen_tin_beta_get_first (opp_node_num, entry_hsh);
	rul__emit_blank_line ();

	bt_val = rul__val_create_blk_or_loc (CMP__E_INT_BETA_TOK, BETA_TOK_ARG);
	rul__emit_begin_loop (EMIT__E_REL_NONE, bt_val, NULL);
	rul__gen_lhs_tmp_mol_inits ();

	if (tin->first_test) {
	    rul__emit_blank_line ();
	    rul__emit_comment ("Now do the non-identity comparisons");
	}
	gen_predicate_test_set (node, tin->first_test, is_left_entry, FALSE);

	rul__emit_blank_line ();
	func_val = rul__val_create_function_str (
#ifndef NDEBUG
			"rul__beta_get_next_token", 1);
#else
			"BGNT", 1);
#endif
	rul__val_set_nth_subvalue (func_val, 1, bt_val);
	assn_val = rul__val_create_assignment (
			rul__val_copy_value (bt_val), func_val);
	rul__emit_value_statement (assn_val);
	rul__val_free_value (assn_val);

	rul__gen_decr_lhs_tmps ();
	rul__emit_end_loop ();
}




/****************************
**                         **
**  GEN_TIN_NEG_LEFT_BODY  **
**                         **
****************************/

static void gen_tin_neg_left_body (Net_Node node)
{
	Net_2_In_Node tin;
	long node_num, opp_node_num;
	long entry_obj_count, entry_hsh_count, entry_aux_count, i;
	Value in_sign, sign_neg, assn_val, val1, mat_cnt, zero, func_val;

	assert (rul___net_node_is_valid (node));
	tin = (Net_2_In_Node) node;

	node_num = tin->node_number;
	opp_node_num = tin->node_number + 1;
	entry_obj_count = tin->left_entry_obj_count;
	entry_hsh_count = rul__dyar_get_length (tin->left_entry_hsh_values);
	entry_aux_count = rul__dyar_get_length (tin->left_entry_aux_values);

	/*  If needed emit decls for aux_array, and hash_array  */
	i = rul__net_max_hsh_in_a_child (node);
	if (i > 0) {
	    rul__emit_stack_internal (CMP__E_INT_MOLECULE, 
					OUT_HSH_ARRAY_ARG, i);
	}
	i = rul__net_max_aux_in_a_child (node);
	if (i > 0) {
	    rul__emit_stack_internal (CMP__E_INT_MOLECULE, 
					OUT_AUX_ARRAY_ARG, i);
	}

	/*  Finish the local declarations  */
	rul__emit_stack_internal (CMP__E_INT_LONG, 
				X_CNT_ARG, EXT__C_NOT_ARRAY);
	gen_unnamed_var_decls (node);
	rul__emit_blank_line ();
	/*  Done with local decls  */


	/*	if (sign == DELTA__C_SIGN_NEGATIVE) {	*/
	in_sign = rul__val_create_blk_or_loc (
			CMP__E_INT_DEL_TOK_SIGN, DEL_SIGN_ARG);
	sign_neg = rul__val_create_long_animal (DELTA__C_SIGN_NEGATIVE);
	rul__emit_begin_conditional (EMIT__E_REL_EQ, in_sign, sign_neg);
	rul__val_free_value (sign_neg);
	rul__val_free_value (in_sign);

	rul__emit_blank_line ();
	rul__emit_comment ("Remove the token, but stash its cross match count");
	gen_tin_beta_remove (node_num, entry_obj_count, entry_hsh_count);

	rul__emit_else_condition ();

	rul__emit_blank_line ();
	rul__emit_comment ("Iterate over opposite side memory of this node");
	gen_tin_beta_get_first (opp_node_num, entry_hsh_count);

	assn_val = rul__val_create_assignment (
			rul__val_create_blk_or_loc (CMP__E_INT_LONG, X_CNT_ARG),
			rul__val_create_long_animal (0));
	rul__emit_value_statement (assn_val);
	rul__val_free_value (assn_val);

	rul__emit_blank_line ();
	val1 = rul__val_create_blk_or_loc (CMP__E_INT_BETA_TOK, BETA_TOK_ARG);
	rul__emit_begin_loop (EMIT__E_REL_NONE, val1, NULL);
	rul__val_free_value (val1);
	rul__gen_lhs_tmp_mol_inits ();

	gen_predicate_test_set (node, tin->first_test, TRUE, FALSE);

	rul__emit_blank_line ();
	func_val = rul__val_create_function_str (
#ifndef NDEBUG
			"rul__beta_get_next_token", 1);
#else
			"BGNT", 1);
#endif
	rul__val_set_nth_subvalue (func_val, 1,
			rul__val_create_blk_or_loc (
					CMP__E_INT_BETA_TOK, BETA_TOK_ARG));
	assn_val = rul__val_create_assignment (
			rul__val_create_blk_or_loc (
					CMP__E_INT_BETA_TOK, BETA_TOK_ARG),
			func_val);
	rul__emit_value_statement (assn_val);
	rul__val_free_value (assn_val);

	rul__gen_decr_lhs_tmps ();
	rul__emit_end_loop ();

	rul__emit_blank_line ();
	rul__emit_comment ("Save the new token, and its cross match count");
	gen_tin_beta_add (node_num, entry_obj_count, 
				entry_hsh_count, entry_aux_count);

	rul__emit_end_conditional ();

	rul__emit_blank_line ();
	rul__emit_comment (
		"Forward token downstream only if it had no cross matches");
	/*	if (cross_matches == 0)    */
	mat_cnt = rul__val_create_blk_or_loc (CMP__E_INT_LONG, X_CNT_ARG);
	zero = rul__val_create_long_animal (0);
	rul__emit_begin_conditional (EMIT__E_REL_EQ, mat_cnt, zero);
	rul__val_free_value (mat_cnt);
	rul__val_free_value (zero);

	rul__emit_blank_line ();
	gen_calls_to_downstream (node, TRUE);

	rul__emit_end_conditional ();
}



/*****************************
**                          **
**  GEN_TIN_NEG_RIGHT_BODY  **
**                          **
*****************************/

static void gen_tin_neg_right_body (Net_Node node)
{
	Net_2_In_Node tin;
	long node_num, opp_node_num, entry_obj, entry_hsh, entry_aux, i;
	Value bt_val, func_val, assn_val;

	assert (rul___net_node_is_valid (node));
	tin = (Net_2_In_Node) node;

	node_num = tin->node_number + 1;
	opp_node_num = tin->node_number;
	entry_obj = tin->right_entry_obj_count;
	entry_hsh = rul__dyar_get_length (tin->left_entry_hsh_values);
	entry_aux = rul__dyar_get_length (tin->right_entry_aux_values);

	/*  If needed emit decls for aux_array, and hash_array  */
	i = rul__net_max_hsh_in_a_child (node);
	if (i > 0) {
	    rul__emit_stack_internal (CMP__E_INT_MOLECULE, 
					OUT_HSH_ARRAY_ARG, i);
	}
	i = rul__net_max_aux_in_a_child (node);
	if (i > 0) {
	    rul__emit_stack_internal (CMP__E_INT_MOLECULE, 
					OUT_AUX_ARRAY_ARG, i);
	}

	/*  Finish the local declarations  */
	rul__emit_stack_internal (CMP__E_INT_LONG, 
				ACT_ON_ARG, EXT__C_NOT_ARRAY);
	rul__emit_stack_internal (CMP__E_INT_LONG, 
				OUT_SIGN_ARG, EXT__C_NOT_ARRAY);
	gen_unnamed_var_decls (node);
	rul__emit_blank_line ();
	/*  Done with local decls  */

	gen_tin_neg_re_if_prefix ();

	rul__emit_comment ("Store or remove token from local beta memory");
	gen_tin_beta_modify (node_num, entry_obj, entry_hsh, entry_aux);
	rul__emit_blank_line ();

	rul__emit_comment ("Iterate over opposite side memory of this node");
	gen_tin_beta_get_first (opp_node_num, entry_hsh);
	rul__emit_blank_line ();

	bt_val = rul__val_create_blk_or_loc (CMP__E_INT_BETA_TOK, BETA_TOK_ARG);
	rul__emit_begin_loop (EMIT__E_REL_NONE, bt_val, NULL);
	rul__gen_lhs_tmp_mol_inits ();

	if (tin->first_test) {
	    rul__emit_blank_line ();
	    rul__emit_comment ("Check non-identity comparisons");
	}

	gen_predicate_test_set (node, tin->first_test,
				FALSE /* Right-Entry*/, FALSE);

	rul__emit_blank_line ();
	func_val = rul__val_create_function_str (
#ifndef NDEBUG
			"rul__beta_get_next_token", 1);
#else
			"BGNT", 1);
#endif
	rul__val_set_nth_subvalue (func_val, 1, bt_val);
	assn_val = rul__val_create_assignment (
			rul__val_copy_value (bt_val), func_val);
	rul__emit_value_statement (assn_val);
	rul__val_free_value (assn_val);

	rul__gen_decr_lhs_tmps ();
	rul__emit_end_loop ();

}



/***************************
**                        **
**  GEN_TIN_NODE_COMMENT  **
**                        **
***************************/

static void gen_tin_node_comment (Net_Node node)
{
	Net_2_In_Node tin;
	char name[RUL_C_MAX_SYMBOL_SIZE+1];
	char comment[2*RUL_C_MAX_SYMBOL_SIZE+1];
	char *sign;

	tin = (Net_2_In_Node) node;

	rul__mol_use_printform (node->source_rule, name, 
				RUL_C_MAX_SYMBOL_SIZE + 1);
	sprintf (comment, "Rule:   %s  (ce %ld)", name, tin->ce_index);
	rul__emit_real_comment (comment);

#ifndef NDEBUG
	sign = (is_2_in_pos_node(node) ? "Positive" : "Negative");
	sprintf (comment, "Node Type:           %s", sign);
	rul__emit_comment (comment);

	sprintf (comment, "Left-In Obj Count:   %ld       ", 
		tin->left_entry_obj_count);
	rul__emit_comment (comment);

	sprintf (comment, "Right-In Obj Count:  %ld       ", 
		tin->right_entry_obj_count);
	rul__emit_comment (comment);
#endif

	rul__emit_blank_line ();
}




/**************************
**                       **
**  GEN_TIN_BETA_MODIFY  **
**                       **
**************************/

static void gen_tin_beta_modify (long node_num, long entry_obj_count,
			long entry_hsh_count, long entry_aux_count)
{
	Value func_val;

#ifndef NDEBUG
	func_val = rul__val_create_function_str ("rul__beta_modify_memory",10);
#else
	func_val = rul__val_create_function_str ("BMM",10);
#endif

	rul__val_set_nth_subvalue (func_val, 1,
		rul__val_create_blk_or_loc (CMP__E_INT_BETA_SET,
					 gen_get_cur_beta_name ()));

	rul__val_set_nth_subvalue (func_val, 2,
		rul__val_create_long_animal (node_num));

	rul__val_set_nth_subvalue (func_val, 3,
		rul__val_create_uns_long_animal (
				rul__mol_integer_to_hash_num (node_num)));

	rul__val_set_nth_subvalue (func_val, 4,
		rul__val_create_blk_or_loc (CMP__E_INT_DEL_TOK_SIGN, 
					DEL_SIGN_ARG));

	rul__val_set_nth_subvalue (func_val, 5,
		rul__val_create_long_animal (entry_obj_count));

	rul__val_set_nth_subvalue (func_val, 6,
		rul__val_create_blk_or_loc (CMP__E_INT_MOLECULE, 
					IN_INST_ARRAY_ARG));

	rul__val_set_nth_subvalue (func_val, 7, 
		rul__val_create_long_animal (entry_aux_count));

	if (entry_aux_count > 0) {
	    rul__val_set_nth_subvalue (func_val, 8, 
			rul__val_create_blk_or_loc (CMP__E_INT_MOLECULE,
						    IN_AUX_ARRAY_ARG));
	} else {
	    rul__val_set_nth_subvalue (func_val, 8, 
			rul__val_create_null (CMP__E_INT_MOLECULE));
	}

	rul__val_set_nth_subvalue (func_val, 9, 
		rul__val_create_long_animal (entry_hsh_count));

	if (entry_hsh_count > 0) {
	    rul__val_set_nth_subvalue (func_val, 10, 
			rul__val_create_blk_or_loc (CMP__E_INT_MOLECULE,
						    IN_HSH_ARRAY_ARG));
	} else {
	    rul__val_set_nth_subvalue (func_val, 10, 
			rul__val_create_null (CMP__E_INT_MOLECULE));
	}

	rul__emit_value_statement (func_val);
	rul__val_free_value (func_val);
}




/**************************
**                       **
**  GEN_TIN_BETA_REMOVE  **
**                       **
**************************/

static void gen_tin_beta_remove (long node_num,
				 long entry_obj_count, 
				 long entry_hsh_count)
{
	Value func_val, assn_val;

	func_val = rul__val_create_function_str (
#ifndef NDEBUG
				"rul__beta_remove_from_memory", 7);
#else
				"BRFM", 7);
#endif

	rul__val_set_nth_subvalue (func_val, 1,
		rul__val_create_blk_or_loc (CMP__E_INT_BETA_SET,
					 gen_get_cur_beta_name ()));

	rul__val_set_nth_subvalue (func_val, 2,
		rul__val_create_long_animal (node_num));

	rul__val_set_nth_subvalue (func_val, 3,
		rul__val_create_uns_long_animal (
				rul__mol_integer_to_hash_num (node_num)));

	rul__val_set_nth_subvalue (func_val, 4,
		rul__val_create_long_animal (entry_obj_count));

	rul__val_set_nth_subvalue (func_val, 5,
		rul__val_create_blk_or_loc (CMP__E_INT_MOLECULE, 
					IN_INST_ARRAY_ARG));

	rul__val_set_nth_subvalue (func_val, 6, 
		rul__val_create_long_animal (entry_hsh_count));

	if (entry_hsh_count > 0) {
	    rul__val_set_nth_subvalue (func_val, 7, 
			rul__val_create_blk_or_loc (CMP__E_INT_MOLECULE,
						    IN_HSH_ARRAY_ARG));
	} else {
	    rul__val_set_nth_subvalue (func_val, 7, 
			rul__val_create_null (CMP__E_INT_MOLECULE));
	}

	assn_val = rul__val_create_assignment (
		rul__val_create_blk_or_loc (CMP__E_INT_LONG, X_CNT_ARG),
		func_val);

	rul__emit_value_statement (assn_val);
	rul__val_free_value (assn_val);
}




/***********************
**                    **
**  GEN_TIN_BETA_ADD  **
**                    **
***********************/

static void gen_tin_beta_add (long node_num, long entry_obj_count,
			long entry_hsh_count, long entry_aux_count)
{
	Value func_val;

	func_val = rul__val_create_function_str (
#ifndef NDEBUG
				"rul__beta_add_to_memory", 10);
#else
				"BATM", 10);
#endif

	rul__val_set_nth_subvalue (func_val, 1,
		rul__val_create_blk_or_loc (CMP__E_INT_BETA_SET,
					 gen_get_cur_beta_name ()));

	rul__val_set_nth_subvalue (func_val, 2,
		rul__val_create_long_animal (node_num));

	rul__val_set_nth_subvalue (func_val, 3,
		rul__val_create_uns_long_animal (
				rul__mol_integer_to_hash_num (node_num)));

	rul__val_set_nth_subvalue (func_val, 4,
		rul__val_create_blk_or_loc (CMP__E_INT_LONG, X_CNT_ARG));

	rul__val_set_nth_subvalue (func_val, 5,
		rul__val_create_long_animal (entry_obj_count));

	rul__val_set_nth_subvalue (func_val, 6,
		rul__val_create_blk_or_loc (CMP__E_INT_MOLECULE, 
					IN_INST_ARRAY_ARG));

	rul__val_set_nth_subvalue (func_val, 7, 
		rul__val_create_long_animal (entry_aux_count));

	if (entry_aux_count > 0) {
	    rul__val_set_nth_subvalue (func_val, 8, 
			rul__val_create_blk_or_loc (CMP__E_INT_MOLECULE,
						    IN_AUX_ARRAY_ARG));
	} else {
	    rul__val_set_nth_subvalue (func_val, 8, 
			rul__val_create_null (CMP__E_INT_MOLECULE));
	}

	rul__val_set_nth_subvalue (func_val, 9, 
		rul__val_create_long_animal (entry_hsh_count));

	if (entry_hsh_count > 0) {
	    rul__val_set_nth_subvalue (func_val, 10, 
			rul__val_create_blk_or_loc (CMP__E_INT_MOLECULE,
						    IN_HSH_ARRAY_ARG));
	} else {
	    rul__val_set_nth_subvalue (func_val, 10, 
			rul__val_create_null (CMP__E_INT_MOLECULE));
	}

	rul__emit_value_statement (func_val);
	rul__val_free_value (func_val);
}




/*****************************
**                          **
**  GEN_TIN_BETA_GET_FIRST  **
**                          **
*****************************/

static void gen_tin_beta_get_first (long opp_node_num, long entry_hsh_count)
{
	Value func_val, assn_val;


	func_val = rul__val_create_function_str (
#ifndef NDEBUG
					"rul__beta_get_first_token", 5);
#else
					"BGFT", 5);
#endif

	rul__val_set_nth_subvalue (func_val, 1,
		rul__val_create_blk_or_loc (CMP__E_INT_BETA_SET,
					 gen_get_cur_beta_name ()));

	rul__val_set_nth_subvalue (func_val, 2,
		rul__val_create_long_animal (opp_node_num));

	rul__val_set_nth_subvalue (func_val, 3,
		rul__val_create_uns_long_animal (
				rul__mol_integer_to_hash_num (opp_node_num)));

	rul__val_set_nth_subvalue (func_val, 4, 
		rul__val_create_long_animal (entry_hsh_count));

	if (entry_hsh_count > 0) {
	    rul__val_set_nth_subvalue (func_val, 5, 
			rul__val_create_blk_or_loc (CMP__E_INT_MOLECULE,
						    IN_HSH_ARRAY_ARG));
	} else {
	    rul__val_set_nth_subvalue (func_val, 5, 
			rul__val_create_null (CMP__E_INT_MOLECULE));
	}

	assn_val = rul__val_create_assignment (
		rul__val_create_blk_or_loc (CMP__E_INT_BETA_TOK, BETA_TOK_ARG),
		func_val);

	rul__emit_value_statement (assn_val);
	rul__val_free_value (assn_val);
}




/******************************
**                           **
**  GEN_TIN_INST_ARRAY_INIT  **
**                           **
******************************/

static void gen_tin_inst_array_init (Net_Node node, Boolean is_left_entry)
{
	Net_2_In_Node tin;
	Value assign_inst, source;
	long i, out_obj_count;

	assert (is_2_in_node (node));

	tin = (Net_2_In_Node) node;

	if (is_2_in_pos_node (node)) {
	    out_obj_count = tin->left_entry_obj_count + 
			    tin->right_entry_obj_count;
	} else {
	    out_obj_count = tin->left_entry_obj_count;
	}

	/*?*  Do we need to do anything different for CE disj's ? */

	for (i=0; i<out_obj_count; i++) {

	    /*  Determine the instance id source  */
	    if (i < tin->left_entry_obj_count) {

		/*  source is left side  */
		if (is_left_entry) {
		    source = rul__val_create_blk_or_loc_vec (
				CMP__E_INT_MOLECULE, IN_INST_ARRAY_ARG, i);
		} else {
		    source = rul__val_create_blk_loc_fld_vec (
				CMP__E_INT_BETA_TOK, BETA_TOK_ARG, 
				CMP__E_FIELD_BETA_INST_VEC, i);
		}

	    } else {

		/*  source is right side  */
		if (is_left_entry) {
		    source = rul__val_create_blk_loc_fld_vec (
				CMP__E_INT_BETA_TOK, BETA_TOK_ARG, 
				CMP__E_FIELD_BETA_INST_VEC,
				i - tin->left_entry_obj_count);
		} else {
		    source = rul__val_create_blk_or_loc_vec (
				CMP__E_INT_MOLECULE, IN_INST_ARRAY_ARG,
				i - tin->left_entry_obj_count);
		}
	    }

	    /*  Emit the actual instance id assignment  */
	    assign_inst = 
		rul__val_create_assignment (
			rul__val_create_blk_or_loc_vec (
				CMP__E_INT_MOLECULE, OUT_INST_ARRAY_ARG, i),
			source);
	    rul__emit_value_statement (assign_inst);
	    rul__val_free_value (assign_inst);
	}
}




/********************************
**                             **
**  GEN_TIN_NEG_LE_INCR_MATCH  **
**                             **
********************************/

static void gen_tin_neg_le_incr_match (void)
{
	Value assn_val, one, arith, xmatch;

	rul__emit_blank_line ();
	rul__emit_comment ("For match of negated, increment cross match count");

	xmatch = rul__val_create_blk_or_loc (CMP__E_INT_LONG, X_CNT_ARG);
	one = rul__val_create_long_animal (1);
	arith =  rul__val_create_arith_expr (CMP__E_ARITH_PLUS, xmatch, one);
	assn_val = rul__val_create_assignment (
			rul__val_copy_value (xmatch), arith);
	rul__emit_value_statement (assn_val);
	rul__val_free_value (assn_val);
}




/*******************************
**                            **
**  GEN_TIN_NEG_RE_IF_PREFIX  **
**                            **
*******************************/

static void gen_tin_neg_re_if_prefix (void)
{
	Value in_sign, sign_neg, assn_val, val1, val2;

	/*	if (sign == DELTA__C_SIGN_NEGATIVE) {	*/
	in_sign = rul__val_create_blk_or_loc (
			CMP__E_INT_DEL_TOK_SIGN, DEL_SIGN_ARG);
	sign_neg = rul__val_create_long_animal (DELTA__C_SIGN_NEGATIVE);
	rul__emit_begin_conditional (EMIT__E_REL_EQ, in_sign, sign_neg);
	rul__val_free_value (sign_neg);
	rul__val_free_value (in_sign);

	/*	    out_sign = DELTA__C_SIGN_POSITIVE;  */
	val1 = rul__val_create_blk_or_loc (CMP__E_INT_DEL_TOK_SIGN,
					   OUT_SIGN_ARG);
	val2 = rul__val_create_long_animal (DELTA__C_SIGN_POSITIVE);
	assn_val = rul__val_create_assignment (val1, val2);
	rul__emit_value_statement (assn_val);
	rul__val_free_value (assn_val);

	/*	    act_on =  0;  */
	val1 = rul__val_create_blk_or_loc (CMP__E_INT_LONG, ACT_ON_ARG);
	val2 = rul__val_create_long_animal (0);
	assn_val = rul__val_create_assignment (val1, val2);
	rul__emit_value_statement (assn_val);
	rul__val_free_value (assn_val);

	rul__emit_else_condition ();

	/*	    out_sign = DELTA__C_SIGN_NEGATIVE;  */
	val1 = rul__val_create_blk_or_loc (CMP__E_INT_DEL_TOK_SIGN,
					   OUT_SIGN_ARG);
	val2 = rul__val_create_long_animal (DELTA__C_SIGN_NEGATIVE);
	assn_val = rul__val_create_assignment (val1, val2);
	rul__emit_value_statement (assn_val);
	rul__val_free_value (assn_val);

	/*	    act_on =  1;  */
	val1 = rul__val_create_blk_or_loc (CMP__E_INT_LONG, ACT_ON_ARG);
	val2 = rul__val_create_long_animal (1);
	assn_val = rul__val_create_assignment (val1, val2);
	rul__emit_value_statement (assn_val);
	rul__val_free_value (assn_val);

	rul__emit_end_conditional ();
	rul__emit_blank_line ();
}




/************************************
**                                 **
**  GEN_TIN_NEG_RE_IF_MATCH_COUNT  **
**                                 **
************************************/

static void gen_tin_neg_re_if_match_count (Net_Node node)
{
	Value func_val, act_on_val;

	rul__emit_blank_line ();
	act_on_val = rul__val_create_blk_or_loc (CMP__E_INT_LONG, ACT_ON_ARG);
	func_val = rul__val_create_function_str (
#ifndef NDEBUG
				"rul__beta_modify_match_count", 2);
#else
				"BMMC", 2);
#endif
	rul__val_set_nth_subvalue (func_val, 1,
		  rul__val_create_blk_or_loc (
				CMP__E_INT_BETA_TOK, BETA_TOK_ARG));
	rul__val_set_nth_subvalue (func_val, 2,
		  rul__val_create_blk_or_loc (CMP__E_INT_LONG, DEL_SIGN_ARG));

	rul__emit_begin_conditional (EMIT__E_REL_EQ, func_val, act_on_val);
	rul__val_free_value (act_on_val);
	rul__val_free_value (func_val);

	rul__emit_blank_line ();
	rul__emit_comment ("Send left token to downstream nodes");
	gen_calls_to_downstream (node, FALSE);

	rul__emit_end_conditional ();
}





/******************************
**                           **
**  GEN_CALLS_TO_DOWNSTREAM  **
**                           **
******************************/

static void gen_calls_to_downstream (Net_Node node, Boolean is_left_entry)
{
	Net_Nonterm_Node nt_node;
  volatile	Net_Link lnk;
	Boolean is_source_positive;
	Boolean is_source_2_input;

	assert (rul___net_node_is_valid (node));
	assert (is_nonterm_node (node));

	is_source_positive = !is_2_in_neg_node (node);
	is_source_2_input = is_2_in_node (node);

	nt_node = (Net_Nonterm_Node) node;
	lnk = nt_node->first_link;
	while (lnk) {

	    if (is_2_in_node (lnk->target_node)) {
	        gen_a_tin_invocation (
			(Net_2_In_Node) lnk->target_node,
			is_source_2_input, is_source_positive, 
			is_left_entry, lnk->type);
	    } else {
		assert (is_cse_node (lnk->target_node));
                gen_modify_cs_invocation (
			(Net_CSE_Node) lnk->target_node, node, is_left_entry);
	    }
	    lnk = lnk->next_link;
	}
}




/***************************
**                        **
**  GEN_A_TIN_INVOCATION  **
**                        **
***************************/

static void gen_a_tin_invocation (Net_2_In_Node tin, 
			Boolean is_source_2_input, 
			Boolean is_source_positive, 
			Boolean is_left_entry_of_source, 
			Net_Link_Type type)
{
	char buffer[BUFFER_SIZE], *dir;
	long bl_num, arg_count, i, count;
	Value args[4], func_val, var_val;
	Dynamic_Array hsh_array, aux_array;

	assert (rul___net_node_is_valid ((Net_Node) tin));
	assert (is_2_in_node (tin));

	/*	Figure out the appropriate function name  */
	bl_num = rul__conrg_get_cur_block_index ();
	if (type == NET__E_LEFT_ENTRY) dir = "l";  else  dir = "r";

        sprintf (buffer, "%s%ld_%ld%s", GEN_NAME_PREFIX, 
		bl_num, tin->node_number, dir);

	if (is_source_2_input) {
	    if (!is_source_positive  &&  !is_left_entry_of_source) {
		args[0] = rul__val_create_blk_or_loc (
				CMP__E_INT_DEL_TOK_SIGN, OUT_SIGN_ARG);
	    } else {
		args[0] = rul__val_create_blk_or_loc (
				CMP__E_INT_DEL_TOK_SIGN, DEL_SIGN_ARG);
	    }
	} else {
	    args[0] = rul__val_create_blk_or_loc_fld (
				CMP__E_INT_DELTA_TOK, DEL_TOK_ARG, 
				CMP__E_FIELD_DELTA_SIGN);
	}

	if (is_source_positive) {
	    args[1] = rul__val_create_blk_or_loc (
				CMP__E_INT_MOLECULE, OUT_INST_ARRAY_ARG);
	} else {
	    if (is_left_entry_of_source) {
		args[1] = rul__val_create_blk_or_loc (
				CMP__E_INT_MOLECULE, IN_INST_ARRAY_ARG);
	    } else {
		args[1] = rul__val_create_blk_or_loc_fld (
				CMP__E_INT_BETA_TOK, BETA_TOK_ARG,
				CMP__E_FIELD_BETA_INST_VEC);
	    }
	}
	arg_count = 2;

	if (type == NET__E_LEFT_ENTRY) {
	    hsh_array = tin->left_entry_hsh_values;
	    aux_array = tin->left_entry_aux_values;
	} else {
	    hsh_array = tin->right_entry_hsh_values;
	    aux_array = tin->right_entry_aux_values;
	}
	    
	count = rul__dyar_get_length (hsh_array);
	if (count  > 0) {
	    for (i=0; i<count; i++) {

	        /*  Create assignments into hsh_array  */
		var_val = rul__dyar_get_nth (hsh_array, i);
		gen_vector_elem_assignment (OUT_HSH_ARRAY_ARG, i, var_val);
	    }

	    /*  set up the hsh_array argument  */
	    args[arg_count] = rul__val_create_blk_or_loc (
				    CMP__E_INT_MOLECULE, OUT_HSH_ARRAY_ARG);
	    arg_count ++;
	}

	count = rul__dyar_get_length (aux_array);
	if (count > 0) {
	    for (i=0; i<count; i++) {

	        /*  Create assignments into aux_array  */
		var_val = rul__dyar_get_nth (aux_array, i);
		gen_vector_elem_assignment (OUT_AUX_ARRAY_ARG, i, var_val);
	    }
	    args[arg_count] = rul__val_create_blk_or_loc (
				    CMP__E_INT_MOLECULE, OUT_AUX_ARRAY_ARG);
	    arg_count ++;
	}

	/*	Now generate and emit the actual invocation  */
	func_val = rul__val_create_function_str (buffer, arg_count);
	for (i=0; i<arg_count; i++) {
	    rul__val_set_nth_subvalue (func_val, i + 1, args[i]);
	}
	rul__emit_value_statement (func_val);
	rul__val_free_value (func_val);
}



/*********************************
**                              **
**  GEN_VECTOR_ELEM_ASSIGNMENT  **
**                              **
*********************************/

static void gen_vector_elem_assignment (char *target_vec_name,
			long vec_index, Value var_val)
{
	Value asgn_val;

	asgn_val = rul__val_create_assignment (
			rul__val_create_blk_or_loc_vec (
					CMP__E_INT_MOLECULE, 
					target_vec_name, 
					vec_index),
			rul__val_copy_value (var_val));

	rul__emit_value_statement (asgn_val);
	rul__val_free_value (asgn_val);
}



/****************************** CSE Nodes ******************************/




/*******************************
**                            **
**  GEN_MODIFY_CS_INVOCATION  **
**                            **
*******************************/

static void gen_modify_cs_invocation (Net_CSE_Node cs_node, 
			Net_Node source_node, Boolean is_left_entry_of_source)
{
	Value func_val;

#ifndef NDEBUG
	func_val = rul__val_create_function_str ("rul__cs_modify", 9);
#else
	func_val = rul__val_create_function_str ("CMO", 9);
#endif

	rul__val_set_nth_subvalue (func_val, 1,
		rul__val_create_blk_or_loc (CMP__E_INT_CS, 
				gen_get_cur_css_name ()));

	if (is_1_in_node (source_node)) {
	    rul__val_set_nth_subvalue (func_val, 2,
		    rul__val_create_blk_or_loc_fld (
				CMP__E_INT_DELTA_TOK, DEL_TOK_ARG, 
				CMP__E_FIELD_DELTA_SIGN));
	} else {
	    if (is_2_in_pos_node (source_node)  ||  is_left_entry_of_source) {
	        rul__val_set_nth_subvalue (func_val, 2,
			rul__val_create_blk_or_loc (CMP__E_INT_DEL_TOK_SIGN, 
						DEL_SIGN_ARG));
	    } else {
		rul__val_set_nth_subvalue (func_val, 2,
			rul__val_create_blk_or_loc (CMP__E_INT_DEL_TOK_SIGN, 
						OUT_SIGN_ARG));
	    }
	}

	rul__val_set_nth_subvalue (func_val, 3,
		rul__val_create_rul_constant (cs_node->source_rule));

	rul__val_set_nth_subvalue (func_val, 4,
		rul__val_create_rul_constant (
				rul__conrg_get_cur_block_name ()));

	rul__val_set_nth_subvalue (func_val, 5,
		rul__val_create_blk_or_loc (CMP__E_INT_FUNC_PTR, 
				gen_get_cse_rhs_func_name (cs_node)));

	rul__val_set_nth_subvalue (func_val, 6,
		rul__val_create_long_animal (cs_node->class_specificity));

	rul__val_set_nth_subvalue (func_val, 7,
		rul__val_create_long_animal (cs_node->test_specificity));

	rul__val_set_nth_subvalue (func_val, 8,
		rul__val_create_long_animal (cs_node->object_count));

	if (is_2_in_neg_node (source_node)) {
	    if (is_left_entry_of_source) {
		rul__val_set_nth_subvalue (func_val, 9,
			rul__val_create_blk_or_loc (
				CMP__E_INT_MOLECULE, IN_INST_ARRAY_ARG));
	    } else {
		rul__val_set_nth_subvalue (func_val, 9,
			rul__val_create_blk_or_loc_fld (CMP__E_INT_BETA_TOK,
				BETA_TOK_ARG, CMP__E_FIELD_BETA_INST_VEC));
	    }
	} else {
	    rul__val_set_nth_subvalue (func_val, 9,
			rul__val_create_blk_or_loc (
				CMP__E_INT_MOLECULE, OUT_INST_ARRAY_ARG));
	}

	rul__emit_value_statement (func_val);
	rul__val_free_value (func_val);
}




/***************************
**                        **
**  GEN_GET_CUR_CSS_NAME  **
**                        **
***************************/

static char *gen_get_cur_css_name (void)
{
	static char name_buffer[BUFFER_SIZE];
	long bl_num;

	bl_num = rul__conrg_get_cur_block_index ();

        sprintf (name_buffer, "%s%ld_cs_subset", GEN_NAME_PREFIX, bl_num); 
	return (name_buffer);
}



/****************************
**                         **
**  GEN_GET_CUR_BETA_NAME  **
**                         **
****************************/

static char *gen_get_cur_beta_name (void)
{
	static char name_buffer[BUFFER_SIZE];
	long bl_num;

	bl_num = rul__conrg_get_cur_block_index ();

        sprintf (name_buffer, "%s%ld_bc", GEN_NAME_PREFIX, bl_num); 
	return (name_buffer);
}



/************************************
**                                 **
**  RUL__GEN_GET_CUR_RET_VAL_NAME  **
**                                 **
************************************/

char   *rul__gen_get_cur_ret_val_name (void)
{
	static char name_buffer[BUFFER_SIZE];
	long bl_num;

	bl_num = rul__conrg_get_cur_block_index ();

	sprintf (name_buffer, "%s%ld_ret_val", GEN_NAME_PREFIX, bl_num);
	return (name_buffer);
}





/********************************
**                             **
**  GEN_GET_CSE_RHS_FUNC_NAME  **
**                             **
********************************/

static char *gen_get_cse_rhs_func_name (Net_CSE_Node cs_node)
{
	return (make_an_rhs_func_name (cs_node->rule_index));
}



/********************************
**                             **
**  RUL__GEN_GET_CUR_RHS_FUNC  **
**                             **
********************************/

char *rul__gen_get_cur_rhs_func (void)
{
	return (make_an_rhs_func_name (rul__net_get_cur_rule_index ()));
}



/****************************
**                         **
**  MAKE_AN_RHS_FUNC_NAME  **
**                         **
****************************/

static char *make_an_rhs_func_name (long rule_index)
{
	static char buffer[BUFFER_SIZE];
	long bl_num;

	bl_num = rul__conrg_get_cur_block_index ();

        sprintf (buffer, "%s%ld_rule_%ld", GEN_NAME_PREFIX, bl_num,
		 rule_index);

	return (buffer);
}



char *rul__gen_pred_func_name (Pred_Type pred)
{
  return func_name_for_predicate (pred);
}

/******************************
**                           **
**  FUNC_NAME_FOR_PREDICATE  **
**                           **
******************************/

static char *func_name_for_predicate (Pred_Type pred)
{
	char *ret_val = "?";
	/*
	**  Note:  LHS: PRED__E_EQ and PRED__E_NOT_EQ are not done with
	**		a function, but with a relational comparison.
	*/

	switch (pred) {
		case PRED__E_EQ :
#ifndef NDEBUG
				ret_val = "rul__mol_eq_pred";
#else
				ret_val = "MPEQ";
#endif
				break;
		case PRED__E_EQUAL :
#ifndef NDEBUG
				ret_val = "rul__mol_equal_pred";
#else
				ret_val = "MPEQL";
#endif
				break;
		case PRED__E_NOT_EQ :
#ifndef NDEBUG
				ret_val = "rul__mol_not_eq_pred";
#else
				ret_val = "MPNEQ";
#endif
				break;
		case PRED__E_NOT_EQUAL :
#ifndef NDEBUG
				ret_val = "rul__mol_not_equal_pred";
#else
				ret_val = "MPNEQL";
#endif
				break;
		case PRED__E_APPROX_EQUAL :
#ifndef NDEBUG
				ret_val = "rul__mol_approx_eq_pred";
#else
				ret_val = "MPAE";
#endif
				break;
		case PRED__E_NOT_APPROX_EQUAL :
#ifndef NDEBUG
				ret_val = "rul__mol_not_approx_eq_pred";
#else
				ret_val = "MPNAE";
#endif
				break;
		case PRED__E_SAME_TYPE :
#ifndef NDEBUG
				ret_val = "rul__mol_same_type_pred";
#else
				ret_val = "MPST";
#endif
				break;
		case PRED__E_DIFF_TYPE :
#ifndef NDEBUG
				ret_val = "rul__mol_diff_type_pred";
#else
				ret_val = "MPDT";
#endif
				break;
		case PRED__E_LESS :
#ifndef NDEBUG
				ret_val = "rul__mol_lt_pred";
#else
				ret_val = "MPLT";
#endif
				break;
		case PRED__E_LESS_EQUAL :
#ifndef NDEBUG
				ret_val = "rul__mol_lte_pred";
#else
				ret_val = "MPLTE";
#endif
				break;
		case PRED__E_GREATER :
#ifndef NDEBUG
				ret_val = "rul__mol_gt_pred";
#else
				ret_val = "MPGT";
#endif
				break;
		case PRED__E_GREATER_EQUAL :
#ifndef NDEBUG
				ret_val = "rul__mol_gte_pred";
#else
				ret_val = "MPGTE";
#endif
				break;
		case PRED__E_CONTAINS :
#ifndef NDEBUG
				ret_val = "rul__mol_contains_pred";
#else
				ret_val = "MPC";
#endif
				break;
		case PRED__E_DOES_NOT_CONTAIN :
#ifndef NDEBUG
				ret_val = "rul__mol_not_contains_pred";
#else
				ret_val = "MPNC";
#endif
				break;
		case PRED__E_LENGTH_LESS_EQUAL :
#ifndef NDEBUG
				ret_val = "rul__mol_len_lte_pred";
#else
				ret_val = "MPLLE";
#endif
				break;
		case PRED__E_LENGTH_NOT_EQUAL :
#ifndef NDEBUG
				ret_val = "rul__mol_len_neq_pred";
#else
				ret_val = "MPLNE";
#endif
				break;
		case PRED__E_LENGTH_LESS :
#ifndef NDEBUG
				ret_val = "rul__mol_len_lt_pred";
#else
				ret_val = "MPLLT";
#endif
				break;
		case PRED__E_LENGTH_EQUAL :
#ifndef NDEBUG
				ret_val = "rul__mol_len_eq_pred";
#else
				ret_val = "MPLEQ";
#endif
				break;
		case PRED__E_LENGTH_GREATER_EQUAL :
#ifndef NDEBUG
				ret_val = "rul__mol_len_gte_pred";
#else
				ret_val = "MPLGE";
#endif
				break;
		case PRED__E_LENGTH_GREATER :
#ifndef NDEBUG
				ret_val = "rul__mol_len_gt_pred";
#else
				ret_val = "MPLGT";
#endif
				break;
		case PRED__E_COMP_INDEX_VALID :
#ifndef NDEBUG
				ret_val = "rul__mol_index_valid_pred";
#else
				ret_val = "MPIV";
#endif
				break;
	}
	assert (strcmp (ret_val, "?") != 0);
	return (ret_val);
}


/*
**	Storage for the networks matches information.
**	Used only in the following functions.
*/
static Dynamic_Array 	SA_list_of_rules = NULL;
static Dynamic_Array 	SA_list_of_param_lists = NULL;



/*********************************
**                              **
**  RUL__GEN_NET_RULES_MATCHES  **
**                              **
*********************************/

void rul__gen_net_rules_matches (Mol_Symbol rule_name)
{
	Dynamic_Array cur_rule_net_nodes, param_list;
	long num_nodes, i, param_num, ce_index;
	Net_Node node;
	Mol_Symbol class_name;
	Value tmp_val;
	char str[1000], str_tmp[50];

	cur_rule_net_nodes = rul__net_get_cur_rules_nodes ();
	assert (cur_rule_net_nodes != NULL);
	num_nodes = rul__dyar_get_length (cur_rule_net_nodes);
	assert (num_nodes > 0);

	/*  Set up the param list  */
	param_list = rul__dyar_create_array (num_nodes * 2);
	param_num = 0;
	ce_index = 1;
	for (i=0; i<num_nodes; i++) {

	    node = (Net_Node) rul__dyar_get_nth (cur_rule_net_nodes, i);

	    if (is_1_in_node (node)) {
/*?* 		*  we can get fancy later ... ignore for now ...
		class_name = rul__decl_get_class_name (
					rul__net_get_oin_class (node));
*?*/
	    } else {
		assert (is_2_in_node (node));

		if (ce_index == 1) {		    
		    rul__dyar_set_nth (param_list, param_num,
				   rul__val_create_long_animal (
						node->node_number));
		    rul__dyar_set_nth (param_list, param_num + 1,
				   rul__val_create_asciz_animal ("1"));

		    rul__dyar_set_nth (param_list, param_num + 2,
				   rul__val_create_long_animal (
						node->node_number + 1));
		    rul__dyar_set_nth (param_list, param_num + 3,
				   rul__val_create_asciz_animal ("2"));
		    strcpy (str, "1 2");
		    ce_index = 3;
		    param_num += 4;

		} else {
		    rul__dyar_set_nth (param_list, param_num,
				   rul__val_create_long_animal (
						node->node_number));
		    rul__dyar_set_nth (param_list, param_num + 1,
				   rul__val_create_asciz_animal (str));

		    rul__dyar_set_nth (param_list, param_num + 2,
				   rul__val_create_long_animal (
						node->node_number + 1));
		    sprintf (str_tmp, "%ld", ce_index);
		    rul__dyar_set_nth (param_list, param_num + 3,
				   rul__val_create_asciz_animal (str_tmp));
		    strcat (str, " ");
		    strcat (str, str_tmp);
		    ce_index ++;
		    param_num += 4;
		}
	    }
	}

	/*  Make sure the global lists have been initialized  */
	if (SA_list_of_rules == NULL) {
	    SA_list_of_rules = rul__dyar_create_array (40);
	    SA_list_of_param_lists = rul__dyar_create_array (40);
	}

	/*  Now stash this rule name and its param list in the global lists */
	rul__dyar_append (SA_list_of_rules, 
			rul__val_create_rul_constant (rule_name));
	rul__dyar_append (SA_list_of_param_lists, param_list);
}




/***************************
**                        **
**  GEN_MATCHES_FUNCTION  **
**                        **
***************************/

static void gen_matches_function (void)
{
	char buffer[BUFFER_SIZE];
	long num_rules, rule_index, num_params, param_index;
	Value rule_name_val, rule_name_arg, func_val;
	Dynamic_Array param_list;


	assert ((SA_list_of_rules == NULL &&  SA_list_of_param_lists == NULL)
		||  (rul__dyar_get_length (SA_list_of_rules) ==
		     rul__dyar_get_length (SA_list_of_param_lists)));

	rul__emit_page_break ();
	rul__emit_blank_line ();

        sprintf (buffer, "%s%ld_matches", GEN_NAME_PREFIX, 
		rul__conrg_get_cur_block_index ());

	rul__emit_entry_point_str (buffer, EMIT__E_SCOPE_STATIC, 
				   EMIT__E_NO_RETURN, ext_type_invalid,
				   EXT__C_NOT_ARRAY, ext_mech_invalid);
	rul__emit_entry_arg_internal (CMP__E_INT_MOLECULE, RULE_NAME_ARG,
				      EXT__C_NOT_ARRAY, ext_mech_value);
	rul__emit_entry_args_done (EMIT__E_ENTRY_DEFINITION);

	if (SA_list_of_rules != NULL  &&
	    rul__dyar_get_length (SA_list_of_rules) > 0) {

#ifndef NDEBUG
	    rul__emit_comment ("Get the matches for the specified rule");
	    rul__emit_blank_line ();
#endif

	    rule_name_arg = rul__val_create_blk_or_loc (
				CMP__E_INT_MOLECULE, RULE_NAME_ARG);
	    num_rules = rul__dyar_get_length (SA_list_of_rules);

	    for (rule_index=0; rule_index<num_rules; rule_index++) {

		/*  Get the nth rule and its param_list  */
		rule_name_val = (Value) rul__dyar_get_nth (
					SA_list_of_rules, rule_index);
		param_list = (Dynamic_Array) rul__dyar_get_nth (
					SA_list_of_param_lists, rule_index);
		num_params = rul__dyar_get_length (param_list);

		if (num_params > 0) {	/*  Ignore single CE rules   */

		    rul__emit_begin_conditional (EMIT__E_REL_EQ,
					rule_name_arg, rule_name_val);

		    /*  create the actual invocation of the beta mem printer */
		    func_val = rul__val_create_function_str (
#ifndef NDEBUG
					"rul__beta_print_partial_matches",
#else
							     "BPPM",
#endif
							     num_params + 2);

		    rul__val_set_nth_subvalue (func_val, 1,
					rul__val_create_blk_or_loc (
						CMP__E_INT_BETA_SET,
						gen_get_cur_beta_name ()));
		    rul__val_set_nth_subvalue (func_val, 2,
					rul__val_create_long_animal (
						num_params / 2));

		    for (param_index=0; param_index<num_params;param_index++) {
		        rul__val_set_nth_subvalue (func_val, param_index + 3,
				rul__dyar_get_nth (param_list, param_index));
		    }
		    rul__emit_value_statement (func_val);
		    rul__val_free_value (func_val);

		    rul__emit_end_conditional ();
		}
		/*  clean-up  */
		rul__val_free_value (rule_name_val);
		rul__dyar_free_array (param_list);
	    }
	    rul__val_free_value (rule_name_arg);
	}
	rul__emit_entry_done ();

	/*  clean-up the global lists  */
	if (SA_list_of_rules != NULL) {
	    rul__dyar_free_array (SA_list_of_rules);
	    SA_list_of_rules = NULL;
	}
	if (SA_list_of_param_lists != NULL) {
	    rul__dyar_free_array (SA_list_of_param_lists);
	    SA_list_of_param_lists = NULL;
	}
}

