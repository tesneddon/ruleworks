/****************************************************************************
**                                                                         **
**                  C M P _ S E M _ B U I L D _ N E T . C                  **
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
****************************************************************************/


/*
 * FACILITY:
 *	RULEWORKS compiler
 *
 * ABSTRACT:
 *	This module creates the RETE net given an abstract syntax tree.
 *
 * MODIFIED BY:
 *	DEC	Digital Equipment Corporation
 *	CPQ	Compaq Computer Corporation
 *
 * MODIFICATION HISTORY:
 *
 *	11-Jan-1993	DEC	Initial version
 *	16-Feb-1998	DEC	class type changed to rclass
 *	01-Dec-1999	CPQ	Releasew ith GPL
 */
#include <common.h>
#include <cmp_comm.h>
#include <ast.h>
#include <decl.h>
#include <gen.h>
#include <ios.h>
#include <lvar.h>
#include <net.h>
#include <mol.h>
#include <val.h>
#include <hash.h>

#define MALFORMED_LHS_AST_STRUCTURE 0
#define NYI_LHS_FEATURE 0

static void build_ce_nodes (
			Ast_Node ce_ast, Mol_Symbol rule_name,
			long rule_index, long ce_index,
			long num_prevs, Net_Node *prevs,
			Boolean is_top_level_chain);

static Net_Node build_left_top_oin (
			Ast_Node ce_ast, Mol_Symbol rule_name,
			long rule_index, long ce_index);

static Net_Node build_oin_and_tin_pair (
			Ast_Node ce_ast, Mol_Symbol rule_name,
			long rule_index, long ce_index,
			long num_prevs, Net_Node *prevs);

static void 	build_ce_tests (
			Ast_Node first_test_ast, Class ce_class,
			Net_Node oin, Net_Node tin,
			Mol_Symbol rule_name, long ce_index);

static void	build_a_test (
			Ast_Node test_ast, Class ce_class,
			Mol_Symbol rule_name, long ce_index,
			Net_Node oin, Net_Node tin);

static void 	sem__create_implied_length_test (
			Class ce_class, Mol_Symbol attr_name, 
			Value comp_index_val,
			long ce_index, Net_Node oin, Net_Node binding_node);

static Net_Node sem_find_matching_oin (Molecule ce_mol_str);
static Molecule sem_get_string_mol (char *ce_str);
static void     sem_set_oin_string (Net_Node oin, Molecule ce_mol_str);

#define OIN_HASH_TABLE_SIZE 1009

static Hash_Table oin_ht;







/*********************
**                  **
**  RUL__NET_BUILD  **
**                  **
*********************/

void rul__net_build (Ast_Node lhs, Mol_Symbol rule_name)
{
	Ast_Node ce_ast;
	long rule_index;

	assert (rul__ast_get_type (lhs) == AST__E_RULE_LHS);

	rule_index = rul__net_next_rule_index ();
	ce_ast = rul__ast_get_child (lhs);

	build_ce_nodes (ce_ast, rule_name, rule_index, 1, 0, NULL, TRUE);

	rul__gen_net_rules_matches (rule_name);
}




/*********************
**                  **
**  BUILD_CE_NODES  **
**                  **
*********************/

static void build_ce_nodes (
		Ast_Node ce_ast, Mol_Symbol rule_name,
		long rule_index, long ce_index,
		long num_prevs, Net_Node *prevs,
		Boolean is_top_level_chain)
{
	long i, count = 0;
	Ast_Node_Type type;
	Ast_Node sub_ce_ast;
	Net_Node tin, csn, end;
	Net_Node *prev_list = NULL;
	Class ce_class;

	if (ce_ast == NULL) {
	    if (is_top_level_chain != TRUE) return;
	    /*
	    **  This is the end of the top level chain, make a CS entry node
	    */
	    csn = rul__net_create_cs_node (rule_name, rule_index, ce_index - 1);
	    for (i=0; i<num_prevs; i++) {
		rul__net_connect_target (prevs[i], csn, NET__E_ONLY_ENTRY);
	    }

	} else {
	    type = rul__ast_get_type (ce_ast);

	    if (type == AST__E_POS_CE  ||  type == AST__E_NEG_CE) {
	        /*
	        **  This is a scalar CE at this level
	        */
		if (ce_index == 1) {
		    tin = build_left_top_oin (
				ce_ast, rule_name, 
				rule_index, ce_index++);
		} else {
		    tin = build_oin_and_tin_pair (
				ce_ast, rule_name, 
				rule_index, ce_index++, 
				num_prevs, prevs);
		}
		count = 1;
		prev_list = &tin;

	    } else if (type == AST__E_CE_DISJ) {
		/*
		**  Note:   Implementation assumes linear (1 level) subnet,
		**	    but does not assume only positive ce's.
		*/

		sub_ce_ast = rul__ast_get_child (ce_ast);
		count = rul__ast_get_child_count (ce_ast);
		prev_list = (Net_Node *) rul__mem_malloc (
						sizeof (Net_Node) * count);

		for (i=0; i<count; i++) {
		    /*  What's to be done for the CE disj members  */
		    prev_list[i] = build_oin_and_tin_pair (
					sub_ce_ast, rule_name, 
					rule_index, ce_index++, 
					num_prevs, prevs);
		    sub_ce_ast = rul__ast_get_sibling (sub_ce_ast);
		}

	    } else if (type == AST__E_CE_NEG_CONJ) {
		/*
		**  Note:   Implementation assumes linear (1 level) subnet,
		**	    but does not assume only positive ce's.
		*/

		/*  Make the negated 2-in node that acts as a collector */
		tin = rul__net_create_tin_node (
				FALSE, rule_name, rule_index, ce_index);
		for (i=0; i<num_prevs; i++) {
		    rul__net_connect_target (prevs[i], tin, NET__E_LEFT_ENTRY);
	    	}

		/*  Make the sub-chain within the negated conjunction  */
		sub_ce_ast = rul__ast_get_child (ce_ast);
	    	build_ce_nodes (sub_ce_ast, rule_name, rule_index, ce_index++,
				num_prevs, prevs, FALSE) ;

		/*  Tie the sub-chain to the collector  */
		assert (rul__ast_get_value_type (sub_ce_ast) ==
				AST__E_TYPE_NET_NODE);
		end = rul__net_find_tail (
				(Net_Node) rul__ast_get_value (sub_ce_ast));
		rul__net_connect_target (end, tin, NET__E_RIGHT_ENTRY);

		count = 1;
		prev_list = &tin;

	    } else {
		assert (MALFORMED_LHS_AST_STRUCTURE);
	    }
	    build_ce_nodes (
			rul__ast_get_sibling (ce_ast),
			rule_name, rule_index, ce_index,
			count, prev_list, is_top_level_chain) ;
	}
	if (prev_list != &tin) rul__mem_free (prev_list);
}





/*************************
**                      **
**  BUILD_LEFT_TOP_OIN  **
**                      **
*************************/

static Net_Node build_left_top_oin (
		Ast_Node ce_ast, Mol_Symbol rule_name,
		long rule_index, long ce_index)
{
	Ast_Node rclass, simple_ce;
	Ast_Node_Type type;
	Mol_Symbol name;
	Net_Node oin;
	Class ce_class;
#ifdef DEBUG_SHARING_OIN
	Molecule ce_mol_str;
#endif

	type = rul__ast_get_type (ce_ast);
	assert (type == AST__E_POS_CE);

	/*
	**  Make the 1-input node that corresponds to this CE
	*/

	simple_ce = rul__ast_get_child (ce_ast);
	assert (rul__ast_get_type (simple_ce) == AST__E_CE);

	rclass = rul__ast_get_child (simple_ce);
	assert (rul__ast_get_type (rclass) == AST__E_CONSTANT);

	name = (Mol_Symbol) rul__ast_get_value (rclass);
	ce_class = rul__decl_get_visible_class (name);
	oin = NULL;

#ifdef DEBUG_SHARING_OIN
	ce_mol_str = sem_get_string_mol (rul__ast_get_ce_string (ce_ast));
	oin = sem_find_matching_oin (ce_mol_str);
#endif
	if (oin == NULL) {
	   oin = rul__net_create_oin_node (ce_class, rule_name,
					   rule_index, ce_index);
#ifdef DEBUG_SHARING_OIN
	   sem_set_oin_string (oin, ce_mol_str);
#endif
	   build_ce_tests (rul__ast_get_sibling (rclass), ce_class, oin, NULL, 
			   rule_name, ce_index);
	}
	else
	  rul__net_add_node_to_cur_rule (oin);

	rul__ast_attach_value (simple_ce, AST__E_TYPE_NET_NODE, oin);
#ifdef DEBUG_SHARING_OIN
	rul__mol_decr_uses (ce_mol_str);
#endif
	return (oin);
}




/*****************************
**                          **
**  BUILD_OIN_AND_TIN_PAIR  **
**                          **
*****************************/

static Net_Node build_oin_and_tin_pair (
		Ast_Node ce_ast, Mol_Symbol rule_name,
		long rule_index, long ce_index,
		long num_prevs, Net_Node *prevs)
{
	long i;
	Ast_Node rclass, simple_ce;
	Ast_Node_Type type;
	Net_Node oin, tin;
	Class ce_class;
	Boolean this_ce_is_positive;
	Boolean found_tin = TRUE;
#ifdef DEBUG_SHARING_OIN
	Molecule ce_mol_str;
#endif

	type = rul__ast_get_type (ce_ast);
	assert (type == AST__E_POS_CE  ||  type == AST__E_NEG_CE);

	/*
	**  Make the 1-input node that corresponds to this CE
	*/

	simple_ce = rul__ast_get_child (ce_ast);
	assert (rul__ast_get_type (simple_ce) == AST__E_CE);

	rclass = rul__ast_get_child (simple_ce);
	assert (rul__ast_get_type (rclass) == AST__E_CONSTANT);

	ce_class = rul__decl_get_visible_class (
			(Mol_Symbol) rul__ast_get_value (rclass));

	/*
	**  Find out if we need to create a new 1-In node, or can we share
	*/
	oin = NULL;

#ifdef DEBUG_SHARING_OIN
 	ce_mol_str = sem_get_string_mol (rul__ast_get_ce_string (ce_ast));
	oin = sem_find_matching_oin (ce_mol_str);
#endif

	if (oin == NULL) {
	   /*  Couldn't find a 1-in node to share  */
	   oin = rul__net_create_oin_node (ce_class, rule_name,
					   rule_index, ce_index);
#ifdef DEBUG_SHARING_OIN
	   sem_set_oin_string (oin, ce_mol_str);
#endif
	}
	else
	  rul__net_add_node_to_cur_rule (oin);

	rul__ast_attach_value (simple_ce, AST__E_TYPE_NET_NODE, oin);
#ifdef DEBUG_SHARING_OIN
	rul__mol_decr_uses (ce_mol_str);
#endif

	/*
	**  Find out if we need to create a new 2-In node, or can we share
	*/
	this_ce_is_positive = (type == AST__E_POS_CE);
	assert (num_prevs == 1); /* true until disjunctions */
	tin = NULL;

#ifdef DEBUG_SHARING_TIN
	tin = rul__net_find_matching_tin (prevs[0], oin, this_ce_is_positive);
#endif
	if (tin == NULL) {
	   found_tin = FALSE;
	   tin = rul__net_create_tin_node (this_ce_is_positive,
					   rule_name, rule_index, ce_index);
	   /*?? may need to be done after connects */
	   /*? if (!found_tin) */
	   build_ce_tests (rul__ast_get_sibling (rclass), ce_class, oin, tin, 
			   rule_name, ce_index);
	}
	else
	  rul__net_add_node_to_cur_rule (tin);

	if (ce_index == 1 /* first ce, only in disjunctions ? */)  {
	    rul__net_connect_target (oin, tin, NET__E_LEFT_ENTRY);
	} else {
	    rul__net_connect_target (oin, tin, NET__E_RIGHT_ENTRY);
	    for (i=0; i<num_prevs; i++) {
		rul__net_connect_target (prevs[i], tin, NET__E_LEFT_ENTRY);
	    }
	}

	return (tin);
}




/*********************
**                  **
**  BUILD_CE_TESTS  **
**                  **
*********************/

static void 	build_ce_tests (
			Ast_Node first_test_ast, Class ce_class,
			Net_Node oin, Net_Node tin,
			Mol_Symbol rule_name, long ce_index)
{
	Ast_Node test_ast;

	test_ast = first_test_ast;
	while (test_ast) {
	    build_a_test (test_ast, ce_class, rule_name, ce_index, oin, tin);
	    test_ast = rul__ast_get_sibling (test_ast);
	}
}




/*******************
**                **
**  BUILD_A_TEST  **
**                **
*******************/

static void build_a_test (Ast_Node test_ast, Class ce_class,
			  Mol_Symbol rule_name, long ce_index,
			  Net_Node oin, Net_Node tin)
{
  Net_Test   test;
  Ast_Node   attr_expr_ast, attr_name_ast, attr_index_ast;
  Ast_Node   pred_ast, pred_val_ast, opt_pred_ast;
  Pred_Type  ptype, optype;
  Net_Node   binding_node;
  Mol_Symbol attr_name, var_name;
  Value      orig_attr_val = NULL, attr_val, other_val, get_last_val,
             comp_attr_val, index_val, bound_val;
  Boolean    attr_used, forced_tint = FALSE;
  long       unnamed_var_id; /*  implicit LHS variable for attribute expr  */
  long       unnamed_comp_var_id;



  assert (rul__ast_get_type (test_ast) == AST__E_ATTR_TEST);

  attr_expr_ast = rul__ast_get_child (test_ast);
  assert (rul__ast_get_type (attr_expr_ast) == AST__E_ATTR_EXPR);

  attr_name_ast = rul__ast_get_child (attr_expr_ast);
  attr_name = (Mol_Symbol) rul__ast_get_value (attr_name_ast);


  if (rul__ast_get_child_count (attr_expr_ast) == 1) {

    /*  Simple attribute, no element expression  */
    if (rul__ast_get_type (attr_name_ast) == AST__E_CONSTANT) {

      /*  Find/create a var for a simple direct attr reference  */
      unnamed_var_id = rul__net_add_lhs_var_binding (oin, ce_index, NULL,
			     rul__val_create_position (ce_class,
						       attr_name, ce_index));
      orig_attr_val = rul__val_create_lhs_variable (NULL, unnamed_var_id);
      forced_tint = FALSE;

    }
    else {
      assert (MALFORMED_LHS_AST_STRUCTURE);
    }

  }
  else if (rul__ast_get_child_count (attr_expr_ast) == 2) {

    /*  Attribute element expression  */
    attr_index_ast = rul__ast_get_sibling (attr_name_ast);

    /*  Move the index value tree from the ast to the net structures  */
    index_val = (Value) rul__ast_get_value (attr_index_ast);
    rul__ast_attach_value (attr_index_ast, AST__E_TYPE_NULL, NULL);

    if (rul__ast_get_type (attr_name_ast) == AST__E_CONSTANT) {

      /*  Named attribute indexed reference  */

      if (rul__ast_get_type (rul__ast_get_child (attr_index_ast))
	                                       == AST__E_DOLLAR_LAST) {

	/*  Special index, $last  */
	forced_tint = FALSE;
	get_last_val = rul__val_create_function_str (
					     "rul__mol_get_comp_last", 1);
	rul__val_set_nth_subvalue (get_last_val, 1,
				   rul__val_create_position (
					     ce_class, attr_name, ce_index));
	unnamed_var_id = rul__net_add_lhs_var_binding (
				       oin, ce_index, NULL, get_last_val);
	orig_attr_val = rul__val_create_lhs_variable (NULL, unnamed_var_id);

	/*  Add implied length (> 0) test on the 1-in node  */
	sem__create_implied_length_test (ce_class, attr_name,
					 rul__val_create_long_animal (1),
					 ce_index, oin, oin);

      }
      else if (rul__val_is_local (index_val, ce_index)) {

	/*  Element index can be resolved at 1-input node  */
	assert (index_val != NULL);
	forced_tint = FALSE;
	unnamed_var_id = rul__net_add_lhs_var_binding (oin, ce_index, NULL,
		       rul__val_create_position_elem (ce_class, attr_name, 
						      index_val, ce_index));
	orig_attr_val = rul__val_create_lhs_variable (NULL, unnamed_var_id);

	/*  Insert the implied length test on the 1-in node  */
	sem__create_implied_length_test (ce_class, attr_name,
					 index_val, ce_index, oin, oin);

      }

      else {
	/*
	 **  Element index cannnot be resolved at 1-input node, 
	 **  forcing all associated tests to be 2-input tests.
	 */
	assert (index_val != NULL);
	forced_tint = TRUE;
	
	/*  Bind the entire compound value at the 1-in node */
	unnamed_comp_var_id = rul__net_add_lhs_var_binding (
						    oin, ce_index, NULL,
			    rul__val_create_position (ce_class, attr_name, 
						      ce_index));

	/*  Bind the compound element at the 2-input node  */
	comp_attr_val = rul__val_create_function_str (
					      "rul__mol_get_comp_nth_mol", 2);
	rul__val_set_nth_subvalue (comp_attr_val, 1,
			   rul__val_create_lhs_variable (NULL, 
							 unnamed_comp_var_id));
	rul__val_set_nth_subvalue (comp_attr_val, 2, index_val);

	unnamed_var_id = rul__net_add_lhs_var_binding (tin, ce_index, NULL,
						       comp_attr_val);
	rul__val_mark_lhs_variables (comp_attr_val, tin);
	orig_attr_val = rul__val_create_lhs_variable (NULL, unnamed_var_id);

	/*  Insert the implied length test on the 2-in node  */
	sem__create_implied_length_test (ce_class, attr_name,
					 index_val, ce_index, oin, tin);
      }
      
    }
    else {
      assert (MALFORMED_LHS_AST_STRUCTURE);
    }
  }

  attr_used = FALSE;
  pred_ast = rul__ast_get_sibling (attr_expr_ast);

  while (pred_ast) {

    /*  In a test conjunction, multiple predicates/binds are possible */

    if (attr_used) {
      attr_val = rul__val_copy_value (orig_attr_val);
    }
    else {
      attr_val = orig_attr_val;
      attr_used = TRUE;
    }

    assert (rul__ast_get_value_type (pred_ast) == AST__E_TYPE_PRED);
    ptype = (Pred_Type) rul__ast_get_long_value (pred_ast);
    opt_pred_ast = rul__ast_get_child (pred_ast);
    if (rul__ast_get_value_type (opt_pred_ast) == AST__E_TYPE_PRED) {
      optype = (Pred_Type) rul__ast_get_long_value (opt_pred_ast);
      pred_val_ast = rul__ast_get_sibling (opt_pred_ast);
    }
    else {
      optype = PRED__E_NOT_A_PRED;
      pred_val_ast = opt_pred_ast;
    }

    if (rul__ast_get_type (pred_ast) == AST__E_LHS_VAR_BIND) {

      /*  Set the 1-input (or 2-input) node for the binding  */
      var_name = (Molecule) rul__ast_get_value (pred_val_ast);

      if (forced_tint)
	binding_node = tin;
      else
	binding_node = oin;

      rul__lvar_set_binding_node (var_name, ce_index, binding_node);

      if (ptype == PRED__E_LENGTH_EQUAL) {
	bound_val = rul__val_create_function_str (
					  "rul__mol_get_poly_count_atom", 1);
	rul__val_set_nth_subvalue (bound_val, 1, attr_val);
      }
      else {
	bound_val = attr_val;
      }

      /*
       **  This is where named variable info (in the LVAR subsystem)
       **  is recreated and attached to the network nodes.
       */
      (void) rul__net_add_lhs_var_binding (binding_node, ce_index,
					   var_name, bound_val);

    }
    else {

      /*
       **	Test (not a bind)
       */
      
      /*  Move the predicate value from the Ast_Node into a test  */
      other_val = (Value) rul__ast_get_value (pred_val_ast);
      rul__ast_attach_value (pred_val_ast, AST__E_TYPE_NULL, NULL);
      
      if (attr_used) {
	attr_val = rul__val_copy_value (orig_attr_val);
      }
      else {
	attr_val = orig_attr_val;
	attr_used = TRUE;
      }

      /*  Create and attach the predicate test  */

      if (rul__val_is_local (other_val, ce_index) && !forced_tint) {

	/*
	 **  Attach all 1-input tests to the one input node
	 */
	test = rul__net_create_test (ptype, optype,
				     attr_val, other_val);
	rul__net_attach_test (oin, test);

      }
      else {

	/*
	 **  For 2-input test we need to seperate them into
	 **  the EQ tests done implicitly by the beta memories,
	 **  and all the other 2-input tests that must be done
	 **  in the more normal fashion.
	 */
	if (rul__val_is_lhs_variable (other_val)  && 
	    ptype == PRED__E_EQ && optype == PRED__E_NOT_A_PRED &&
	    !forced_tint) {

	  /*  Do an EQ match hash instead of normal test  */
	  rul__net_add_match_hsh_vars (tin, 
				       attr_val, other_val);
	  rul__val_mark_lhs_variables (other_val, tin);
	  
	}
	else {

	  /* Do the normal 2-input test stuff  */
	  test = rul__net_create_test (ptype, optype,
				       attr_val, other_val);
	  rul__net_attach_test (tin, test);
	  rul__val_mark_lhs_variables (other_val, tin);
	  if (!forced_tint) {
	    rul__val_mark_lhs_variables (attr_val, tin);
	  }
	}
      }
    }

    /*  Go to the next predicate on this same attribute  */
    pred_ast = rul__ast_get_sibling (pred_ast);
  }
}




/**************************************
**                                   **
**  SEM__CREATE_IMPLIED_LENGTH_TEST  **
**                                   **
**************************************/

static void sem__create_implied_length_test (
			Class ce_class, Mol_Symbol attr_name, 
			Value comp_index_val,
			long ce_index, Net_Node oin, Net_Node binding_node)
{
	Net_Test test;
	long unnamed_var_id;
	Value attr_val, index_val;

	/*  create/find the unnamed var for the full compound value  */
	unnamed_var_id = rul__net_add_lhs_var_binding (
				oin, ce_index, NULL,
		    		rul__val_create_position (
					ce_class, attr_name, ce_index));

	if (rul__val_is_animal_const (comp_index_val)  &&
	    rul__val_get_animal_type (comp_index_val) == ext_type_long)
	{
	    /*  if the index was a constant, make an atomic version of it */
	    index_val = rul__val_create_rul_constant (
			    rul__mol_make_int_atom (
				rul__val_get_animal_long (comp_index_val)));
	} else {
	    index_val = rul__val_copy_value (comp_index_val);
	}

	/*  Create and attach the implied length test  */
	attr_val = rul__val_create_lhs_variable (NULL, unnamed_var_id);
	test = rul__net_create_test (PRED__E_COMP_INDEX_VALID,
				     PRED__E_NOT_A_PRED,
				     attr_val,
				     index_val);
	rul__net_attach_test (binding_node, test);

	if (oin != binding_node) {
	    rul__val_mark_lhs_variables (attr_val, binding_node);
	}
}



/************************************
**  Node sharing utility routines  **
************************************/

static Net_Node sem_find_matching_oin (Molecule ce_mol_str)
{
   Net_Node oin = NULL;

   if (oin_ht)
      oin = rul__hash_get_entry (oin_ht, ce_mol_str);

   return oin;
}


void rul__sem_initialize_oin_ht (void)
{
   if (oin_ht) {
     rul__hash_delete_table (oin_ht);
     oin_ht = NULL;
  }
}

static void sem_set_oin_string (Net_Node oin, Molecule ce_mol_str)
{

   if (oin_ht == NULL)
     oin_ht = rul__hash_create_table (OIN_HASH_TABLE_SIZE);

   rul__hash_add_entry (oin_ht, ce_mol_str, oin);
}

static Molecule sem_get_string_mol (char *ce_str)
{
   Molecule  mol;
   char     *ptr, c;
   long      len = strlen (ce_str);
   long      i, j, k;

   if (len <= RUL_C_MAX_SYMBOL_SIZE)
     mol = rul__mol_make_symbol (ce_str);
   else {
      j = (len / RUL_C_MAX_SYMBOL_SIZE) + 1;
      rul__mol_start_tmp_comp (j);
      ptr = ce_str;
      c = ptr[0];
      for (i = 0; i < j; i++) {
	 ptr[0] = c;
	 if (len > RUL_C_MAX_SYMBOL_SIZE) {
	    c = ptr[RUL_C_MAX_SYMBOL_SIZE];
	    ptr[RUL_C_MAX_SYMBOL_SIZE] = '\0';
	 }
	 rul__mol_set_tmp_comp_nth (i, rul__mol_make_symbol (ptr));
	 ptr += RUL_C_MAX_SYMBOL_SIZE;
	 len -= RUL_C_MAX_SYMBOL_SIZE;
      }
      mol = rul__mol_end_tmp_comp ();
   }

   return mol;
}

