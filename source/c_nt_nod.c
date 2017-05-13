/****************************************************************************
**                                                                         **
**                   C M P _ N E T _ N O D E S . C                         **
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
 *	RULEWORKS compiler
 *
 * ABSTRACT:
 *	This module provides the compiler functions for 
 *	building the RETE network for the current block.
 *
 * MODIFIED BY:
 *	DEC	Digital Equipment Corporation
 *	CPQ	Compaq Computer Corporation
 *
 * MODIFICATION HISTORY:
 *
 *	5-Jan-1992	DEC	Initial version
 *	01-Dec-1999	CPQ	Releasew ith GPL
 */


#include <common.h>
#include <cmp_comm.h>
#include <decl.h>
#include <dyar.h>
#include <ios.h>
#include <lvar.h>
#include <mol.h>
#include <net.h>
#include <val.h>

#include <net_p.h>

/*
**	Local data structures
*/
static Net_Link SA_net_decl_root = NULL;
static Net_Link SA_net_decl_root_last = NULL;
	/*  Net root;  list of links to declaration partition nodes  */

static long 	SA_net_next_rule_index = 1;
	/*  Index of rule within the current block  */

static long	SL_net_next_node_number = 1;
	/*  Index of node/side within the current block */

static Net_Node SA_net_node_list = NULL;
	/*  Simple linked list of all Net_Node's  */

static Net_Link SA_net_link_list = NULL;
	/*  Simple linked list of all Net_Link's  */

static Dynamic_Array SA_nodes_in_cur_rule = NULL;
	/*  List of all 1-in and 2-in nodes created for the current rule  */

static long	SL_class_specificity_this_rule = 0;
	/*  Class specificity of rule currently being compiled */

static long	SL_test_specificity_this_rule = 0;
	/*  Test specificity of rule currently being compiled */

/*
**	Forward declarations for all static local functions
*/
static Net_Node		net_create_node (Net_Node_Type type, 
				Mol_Symbol rule_name,
				long rule_index, long ce_index);
static void		net_free_node (Net_Node node);
static void 		net_free_binding_list (Net_Node node);
static void		net_free_entry_lists (Net_Node node);

static Net_Link 	net_create_link (Net_Node source, Net_Node target, 
				Net_Link_Type type);
static void 		net_append_link (Net_Link *first_link_ptr,
					 Net_Link *last_link_ptr, 
					 Net_Link new_link);
static void 		net_prepend_link (Net_Link *first_link_ptr,
					  Net_Link *last_link_ptr,
 					  Net_Link new_link);
static void 		net_unlink_link  (Net_Link *first_link_ptr,
				          Net_Link *last_link_ptr, 
					  Net_Link link_to_remove);
static void		net_free_link (Net_Link lnk);

static void		net_weave_new_oin_node (Net_1_In_Node oin_node, 
				Class ce_class);
static void 		weave_new_class_node (Net_Decl_Node db_node, 
				Net_1_In_Node node,  Class ce_class);
static void		weave_class_node_into_family (
				Net_Class_Node top_class_node, 
				Net_Class_Node new_class_node);

static Net_Decl_Node 	find_decl_node (Decl_Block db);
static Net_Class_Node 	find_class_node (Net_Decl_Node db_node, 
				Class this_class);
static void 		clear_all_visited_tags (void);
static char	       *link_type_to_chars (Net_Link_Type type);
static void 		for_each_child_node (
				void (*func_ptr)(Net_Node node),
				Net_Node cur_node,
				Boolean predive_invocation);
static void 		for_each_child_link (
				void (*func_ptr)(Net_Node from_node, 
					 Net_Node to_node,
					 Net_Link_Type direction),
				Net_Node cur_node);
static void 		net_propagate_node_info (
				Net_Node from_node, 
				Net_Node to_node, 
				Net_Link_Type lnk_type);
static void 		net_print_node_vars (Net_Node node, char *indent);
static void 		net_print_val_array (Dynamic_Array array, 
				char *title, char *indent);
static long 		net_equivalent_var_id (Dynamic_Array binding_list,
				Value val);


#ifndef NDEBUG


/******************************
**                           **
**  RUL___NET_NODE_IS_VALID  **
**                           **
******************************/

Boolean rul___net_node_is_valid (Net_Node node)
	/*
	**	Use ONLY inside assert statements
	*/
{
	return (node != NULL  &&  
		is_net_node (node)  &&
		node->verification == NET__C_NODE_VERIFY);
}




/******************************
**                           **
**  RUL___NET_LINK_IS_VALID  **
**                           **
******************************/

Boolean rul___net_link_is_valid (Net_Link link)
	/*
	**	Use ONLY inside assert statements
	*/
{
	return (link != NULL  &&  link->verification == NET__C_LINK_VERIFY);
}

#endif /* ifndef NDEBUG */





/*****************************
**                          **
**  RUL__NET_CLEAR_NETWORK  **
**                          **
*****************************/

void rul__net_clear_network (void)
{
	Net_Node node, tmp_node;
	Net_Link lnk, tmp_lnk;

	/*  Free all nodes  */
	node = SA_net_node_list;
	while (node) {
	    tmp_node = node;
	    node = node->global_next_node;
	    net_free_node (tmp_node);
	}
	SA_net_node_list = NULL;

	/*  Free all links  */
	lnk = SA_net_link_list;
	while (lnk) {
	    tmp_lnk = lnk;
	    lnk = lnk->global_next_link;
	    net_free_link (tmp_lnk);
	}
	SA_net_link_list = NULL;

	SA_net_decl_root = NULL;
	SA_net_decl_root_last = NULL;
	SA_net_next_rule_index = 1;
	SL_net_next_node_number = 1;
	SL_class_specificity_this_rule = 0;
	SL_test_specificity_this_rule = 0;
}




/************************
**                     **
**  RUL__NET_IS_VALID  **
**                     **
************************/

Boolean rul__net_is_valid ()
{
	if (SA_net_decl_root != NULL) {
	    return (TRUE);
	}
	return (FALSE);
}



/******************************
**                           **
**  RUL___NET_GET_ROOT_LINK  **
**                           **
******************************/

Net_Link rul___net_get_root_link (void)
{
	return (SA_net_decl_root);
}



/*******************************
**                            **
**  RUL__NET_NODE_IS_2_INPUT  **
**                            **
*******************************/

Boolean rul__net_node_is_2_input (Net_Node node)
{
	assert (rul___net_node_is_valid (node));
	return (is_2_in_node (node));
}



/*******************************
**                            **
**  RUL__NET_NODE_IS_1_INPUT  **
**                            **
*******************************/

Boolean rul__net_node_is_1_input (Net_Node node)
{
	assert (rul___net_node_is_valid (node));
	return (is_1_in_node (node));
}




/*******************************
**                            **
**  RUL__NET_NEXT_RULE_INDEX  **
**                            **
*******************************/

long rul__net_next_rule_index (void)
{
	long tmp;

	tmp = SA_net_next_rule_index;
	SA_net_next_rule_index ++;
	return (tmp);
}



/**********************************
**                               **
**  RUL__NET_GET_CUR_RULE_INDEX  **
**                               **
**********************************/

long rul__net_get_cur_rule_index (void)
{
	assert (SA_net_next_rule_index > 1);

	return (SA_net_next_rule_index - 1);
}



/**********************
**                   **
**  NET_CREATE_NODE  **
**                   **
**********************/

static Net_Node	net_create_node (
			Net_Node_Type type, Mol_Symbol rule_name,
			long rule_index, long ce_index)
{
	Net_Node node;
	int size = 0, step;

	step = 1;
	switch (type) {
		case NET__E_DECL :
			size = sizeof(struct net_decl_node);	break;
		case NET__E_CLASS :
			size = sizeof(struct net_class_node);	break;
		case NET__E_1_IN :
			size = sizeof(struct net_1_in_node);	break;
		case NET__E_2_IN_POS :
		case NET__E_2_IN_NEG :
			size = sizeof(struct net_2_in_node);
			step = 2;				break;
		case NET__E_CSE :
			size = sizeof(struct net_cse_node);	break;
		default :
			assert (BOGUS_NODE_TYPE);
			return (NULL);
	}
	assert (size != 0);
	node = rul__mem_malloc (size);
	assert (node != NULL);

	node->type = type;
	node->rule_index = rule_index;
	node->visited_tag = TRUE;

	if (is_nonterm_node (node)) {
	    ((Net_Nonterm_Node) node)->ce_index = ce_index;
    	    ((Net_Nonterm_Node) node)->first_link = NULL;
    	    ((Net_Nonterm_Node) node)->last_link = NULL;
	}
	node->source_rule = rule_name;
	rul__mol_incr_uses (rule_name);

        node->source_nodes = rul__dyar_create_array (10);

        node->global_next_node = SA_net_node_list;
	SA_net_node_list = node;
	if (node->global_next_node) {
	    node->global_next_node->global_prev_node = node;
	}

	node->node_number = SL_net_next_node_number;
	SL_net_next_node_number += step;

	rul__net_add_node_to_cur_rule (node);

#ifndef NDEBUG
	node->verification = NET__C_NODE_VERIFY;
#endif
	return (node);
}




/********************
**                 **
**  NET_FREE_NODE  **
**                 **
********************/

static void	net_free_node (Net_Node node)
{
	assert (rul___net_node_is_valid (node));

	if (node->source_rule) {
	    rul__mol_decr_uses (node->source_rule);
	}

	if (node->source_nodes) {
	    rul__dyar_free_array (node->source_nodes);
	}

	if (is_1_in_node (node)) {
	    rul__net_free_tests (((Net_1_In_Node)node)->first_test);
	    ((Net_1_In_Node)node)->first_test = NULL;
	    net_free_binding_list (node);

	} else if (is_2_in_node (node)) {
	    rul__net_free_tests (((Net_2_In_Node)node)->first_test);
	    ((Net_2_In_Node)node)->first_test = NULL;
	    net_free_binding_list (node);
	    net_free_entry_lists (node);
	}

#ifndef NDEBUG
        node->type = NET__E_INVALID;
        node->rule_index = 0;
        node->source_rule = NULL;
	node->verification = 0;
	node->global_next_node = NULL;
	node->global_prev_node = NULL;
	if (is_nonterm_node (node)) {
	    ((Net_Nonterm_Node) node)->ce_index = 0;
	    ((Net_Nonterm_Node) node)->first_link = NULL;
	}
#endif

	rul__mem_free (node);
}




/****************************
**                         **
**  NET_FREE_BINDING_LIST  **
**                         **
****************************/

static void net_free_binding_list (Net_Node node)
{
	Net_1_In_Node oin;
	Net_2_In_Node tin;
	long i, len;
	Value tmp;

	assert (rul___net_node_is_valid (node));

	if (is_1_in_node (node)) {

	    oin = (Net_1_In_Node) node;
	    if (oin->value_bindings) {
	    	len = rul__dyar_get_length (oin->value_bindings);
	    	for (i=0; i<len; i++) {
		    tmp = (Value) rul__dyar_get_nth (oin->value_bindings, i);
		    if (tmp) rul__val_free_value (tmp);
	    	}
	        rul__dyar_free_array (oin->value_bindings);
		oin->value_bindings = NULL;
	    }

	} else if (is_2_in_node (node)) {

	    tin = (Net_2_In_Node) node;

	    if (tin->value_bindings) {
	    	len = rul__dyar_get_length (tin->value_bindings);
	    	for (i=0; i<len; i++) {
		    tmp = (Value) rul__dyar_get_nth (tin->value_bindings, i);
		    if (tmp) rul__val_free_value (tmp);
	    	}
	        rul__dyar_free_array (tin->value_bindings);
		tin->value_bindings = NULL;
	    }
	    if (tin->vars_used_here) {
	    	len = rul__dyar_get_length (tin->vars_used_here);
	    	for (i=0; i<len; i++) {
		    tmp = (Value) rul__dyar_get_nth (tin->vars_used_here, i);
		    if (tmp) rul__val_free_value (tmp);
	    	}
	        rul__dyar_free_array (tin->vars_used_here);
		tin->vars_used_here = NULL;
	    }
	}
}




/***************************
**                        **
**  NET_FREE_ENTRY_LISTS  **
**                        **
***************************/

static void net_free_entry_lists (Net_Node node)
{
	Net_2_In_Node tin;
	long i, len;
	Value tmp;

	assert (rul___net_node_is_valid (node));
	assert (is_2_in_node (node));

	tin = (Net_2_In_Node) node;

	if (tin->left_entry_aux_values) {
	    len = rul__dyar_get_length (tin->left_entry_aux_values);
	    for (i=0; i<len; i++) {
		tmp = (Value) rul__dyar_get_nth(tin->left_entry_aux_values, i);
		if (tmp) rul__val_free_value (tmp);
	    }
	    rul__dyar_free_array (tin->left_entry_aux_values);
	}

	if (tin->left_entry_hsh_values) {
	    len = rul__dyar_get_length (tin->left_entry_hsh_values);
	    for (i=0; i<len; i++) {
		tmp = (Value) rul__dyar_get_nth(tin->left_entry_hsh_values, i);
		if (tmp) rul__val_free_value (tmp);
	    }
	    rul__dyar_free_array (tin->left_entry_hsh_values);
	}

	if (tin->right_entry_aux_values) {
	    len = rul__dyar_get_length (tin->right_entry_aux_values);
	    for (i=0; i<len; i++) {
		tmp = (Value) rul__dyar_get_nth(tin->right_entry_aux_values, i);
		if (tmp) rul__val_free_value (tmp);
	    }
	    rul__dyar_free_array (tin->right_entry_aux_values);
	}

	if (tin->right_entry_hsh_values) {
	    len = rul__dyar_get_length (tin->right_entry_hsh_values);
	    for (i=0; i<len; i++) {
		tmp = (Value) rul__dyar_get_nth(tin->right_entry_hsh_values, i);
		if (tmp) rul__val_free_value (tmp);
	    }
	    rul__dyar_free_array (tin->right_entry_hsh_values);
	}

#ifndef NDEBUG
	tin->left_entry_aux_values  = NULL;
	tin->left_entry_hsh_values  = NULL;
	tin->right_entry_aux_values = NULL;
	tin->right_entry_hsh_values = NULL;
#endif
}




/*******************************
**                            **
**  RUL__NET_CREATE_OIN_NODE  **
**                            **
*******************************/

Net_Node rul__net_create_oin_node (
		Class ce_class,	Mol_Symbol rule_name,
		long rule_index, long ce_index)
{
	Net_1_In_Node oin;

	oin = (Net_1_In_Node) 
		net_create_node (NET__E_1_IN, rule_name, rule_index, ce_index);

	oin->first_test = NULL;
	oin->ce_class = ce_class;
	oin->value_bindings = NULL;

	net_weave_new_oin_node (oin, ce_class);

	return ((Net_Node) oin);
}




/*****************************
**                          **
**  RUL__NET_GET_OIN_CLASS  **
**                          **
*****************************/

Class rul__net_get_oin_class (Net_Node node)
{
	assert (rul___net_node_is_valid (node));
	assert (is_1_in_node (node));

	return (((Net_1_In_Node)node)->ce_class);
}



/********************************
**                             **
**  RUL__NET_GET_OIN_CE_INDEX  **
**                             **
********************************/

long rul__net_get_oin_ce_index (Net_Node node)
{
	assert (rul___net_node_is_valid (node));
	assert (is_1_in_node (node));

	return (((Net_1_In_Node)node)->ce_index);
}




/*******************************
**                            **
**  RUL__NET_CREATE_TIN_NODE  **
**                            **
*******************************/

Net_Node rul__net_create_tin_node (Boolean is_positive, 
		Mol_Symbol rule_name, long rule_index, long ce_index)
{
	Net_2_In_Node tin;

	tin = (Net_2_In_Node) 
		net_create_node (
			(is_positive ? NET__E_2_IN_POS : NET__E_2_IN_NEG),
			rule_name, rule_index, ce_index);

	tin->first_test 	    = NULL;
	tin->value_bindings	    = NULL;
	tin->vars_used_here	    = NULL;

	tin->left_entry_obj_count   = 0;
	tin->left_entry_aux_values  = NULL;
	tin->left_entry_hsh_values  = NULL;

	tin->right_entry_obj_count  = 0;
	tin->right_entry_aux_values = NULL;
	tin->right_entry_hsh_values = NULL;

	return ((Net_Node) tin);
}




/******************************
**                           **
**  RUL__NET_CREATE_CS_NODE  **
**                           **
******************************/

Net_Node rul__net_create_cs_node (
		Mol_Symbol rule_name, long rule_index, long ce_count)
{
	Net_CSE_Node csn;

	csn = (Net_CSE_Node) 
		net_create_node (NET__E_CSE, rule_name, rule_index, 0);

	csn->class_specificity 	= SL_class_specificity_this_rule;
	csn->test_specificity 	= SL_test_specificity_this_rule;
	csn->object_count 	= 0;

	/* Reset specificity accumulators */
	SL_class_specificity_this_rule = 0;
	SL_test_specificity_this_rule = 0;

	return ((Net_Node) csn);
}



/**********************
**                   **
**  NET_CREATE_LINK  **
**                   **
**********************/

static Net_Link net_create_link (
		Net_Node source, Net_Node target, Net_Link_Type type)
{
	Net_Link lnk;

	assert (source == NULL || rul___net_node_is_valid (source));
	assert (rul___net_node_is_valid (target));
	
	lnk = rul__mem_malloc (sizeof(struct net_link));
	lnk->source_node = source;
	lnk->target_node = target;
	lnk->type = type;
	lnk->next_link = NULL;

        lnk->global_next_link = SA_net_link_list;
	SA_net_link_list = lnk;
	if (lnk->global_next_link) {
	    lnk->global_next_link->global_prev_link = lnk;
	}

	if (source != NULL) {
	    rul__dyar_append (target->source_nodes, source);
	}

#ifndef NDEBUG
	lnk->verification = NET__C_LINK_VERIFY;
#endif

	return (lnk);
}




/********************
**                 **
**  NET_FREE_LINK  **
**                 **
********************/

static void	net_free_link (Net_Link lnk)
{
	assert (rul___net_link_is_valid (lnk));

#ifndef NDEBUG
        lnk->type = NET__E_BAD_LINK;
        lnk->source_node = NULL;
        lnk->target_node = NULL;
	lnk->verification = 0;
	lnk->global_next_link = NULL;
	lnk->global_prev_link = NULL;
#endif
	rul__mem_free (lnk);
}




/******************************
**                           **
**  RUL__NET_CONNECT_TARGET  **
**                           **
******************************/

void rul__net_connect_target (
		Net_Node source_node,
		Net_Node target_node,
		Net_Link_Type type)
{
	Net_Link lnk;
	Net_Nonterm_Node nt_node;

	assert (rul___net_node_is_valid (source_node));
	assert (is_nonterm_node (source_node));
	assert (rul___net_node_is_valid (target_node));

	lnk = net_create_link (source_node, target_node, type);
	nt_node = (Net_Nonterm_Node) source_node;
	net_append_link (&nt_node->first_link, &nt_node->last_link, lnk);
}





/*****************************
**                          **
**  NET_WEAVE_NEW_OIN_NODE  **
**                          **
*****************************/

static void net_weave_new_oin_node (Net_1_In_Node oin_node, Class ce_class)
{
	Decl_Block db;
	Class top_class;
	Net_Decl_Node db_node;
	Net_Class_Node class_node;
	Net_Link new_link;

	assert (rul___net_node_is_valid ((Net_Node) oin_node));
	assert (is_1_in_node (oin_node));

	db = rul__decl_get_class_block (ce_class);

	db_node = find_decl_node (db);
	if (db_node) {

	    /*  If this db is already registered, use it  */
	    class_node = find_class_node (db_node, ce_class);

	    if (class_node) {
		/* if this class is already registered, use it's node */
		rul__net_connect_target ((Net_Node) class_node,
					 (Net_Node) oin_node, 
					 NET__E_ONLY_ENTRY);
	    } else {
		weave_new_class_node (db_node, oin_node, ce_class);
	    }

	} else {
	    /*
	    **  Since this db is not registered, make a 
	    **  new Decl_Node, and a new Class_Node
	    */
	    if (db != NULL) {
	        db_node = (Net_Decl_Node)
		      rul__net_create_decl_node (rul__decl_get_decl_name (db));
	    } else {
		/*  For $root, db is NULL, and there is no db name  */
		db_node = (Net_Decl_Node) rul__net_create_decl_node (NULL);
	    }
	    weave_new_class_node (db_node, oin_node, ce_class);
	}

	/* Update class and test specificity for the current rule */
	SL_test_specificity_this_rule += 1;
	SL_class_specificity_this_rule +=
		rul__decl_get_class_depth (ce_class);
}



/********************************
**                             **
**  RUL__NET_CREATE_DECL_NODE  **
**                             **
********************************/

Net_Node rul__net_create_decl_node (Mol_Symbol block_name)
{
	Net_Decl_Node db_node;
	Decl_Block db;
	Net_Link new_link;

	if (block_name != NULL) {
	    db = rul__decl_get_block (block_name);
	} else {
	    /*  For $root, block_name and db are NULL  */
	    db = NULL;
	}
	db_node = find_decl_node (db);
	if (db_node) return ((Net_Node) db_node);

	db_node = (Net_Decl_Node) net_create_node (NET__E_DECL, 0, 0, 0);
	db_node->declaring_block = db;

	new_link = net_create_link (	NULL, 
					(Net_Node) db_node, 
					NET__E_ONLY_ENTRY);
	if (block_name != NULL) {
	    net_append_link (&SA_net_decl_root,
			     &SA_net_decl_root_last,
			     new_link);
	} else {
	    /*  $root tests go the head of the list  */
	    net_prepend_link (&SA_net_decl_root,
			      &SA_net_decl_root_last,
			      new_link);
	}

	return ((Net_Node) db_node);	
}




/***************************
**                        **
**  WEAVE_NEW_CLASS_NODE  **
**                        **
***************************/

static void weave_new_class_node (
		Net_Decl_Node db_node, Net_1_In_Node oin_node, Class ce_class)
{
	Net_Class_Node	new_class_node, rel_class_node,
			top_class_node, tmp_class_node;
	Net_Link new_link, lnk;
	Class top_class;

	assert (rul___net_node_is_valid ((Net_Node) db_node));
	assert (is_decl_node (db_node));
        assert (rul___net_node_is_valid ((Net_Node) oin_node));
	assert (is_1_in_node (oin_node));

	/*
	**  First, create the new class node and link the 
	**  new 1-in-node to the new class node.
	*/
	top_class = rul__decl_get_class_ancestor (ce_class);
	new_class_node = (Net_Class_Node) net_create_node (
						NET__E_CLASS,
						oin_node->source_rule,
						oin_node->rule_index,
						oin_node->ce_index);
	new_class_node->this_class = ce_class;
	rul__net_connect_target ((Net_Node) new_class_node,
                                 (Net_Node) oin_node, 
				 NET__E_ONLY_ENTRY);

	/*
	**  Next, see if there is a related class already in the the DB's list
	*/
	lnk = db_node->first_link;
	rel_class_node = NULL;
	while (lnk != NULL  &&  rel_class_node == NULL) {
	    if (is_class_node (lnk->target_node)) {
		tmp_class_node = (Net_Class_Node) lnk->target_node;
		
	        if (top_class == tmp_class_node->this_class  ||
		    top_class == rul__decl_get_class_ancestor (
					tmp_class_node->this_class)) {
		    rel_class_node = tmp_class_node;
	        }
	    }
	    if (rel_class_node == NULL) lnk = lnk->next_link;
	}

	/*
	**  If there is no related class, just add this class to the DB's list
	*/
	if (rel_class_node == NULL) {	
	    rul__net_connect_target ((Net_Node) db_node,
	                     	     (Net_Node) new_class_node, 
				     NET__E_ONLY_ENTRY);
	    return;
	}

	/*
	**  If there is a related class, see if it is the
	**  ancestor class, and if is not:
	**	1)  Add a new node for the ancestor class
	**	2)  Change the displaced related class into a 
	**	    child of the new ancestor class
	*/
	if (top_class == rel_class_node->this_class) {
	    top_class_node = rel_class_node;
	} else {

	    /*  Create the new ancestor node  */
	    top_class_node = (Net_Class_Node) net_create_node (
						NET__E_CLASS,
						oin_node->source_rule,
						oin_node->rule_index,
						oin_node->ce_index);
	    top_class_node->this_class = top_class;
	    lnk->target_node = (Net_Node) top_class_node;

	    /*  Attach the displaced class node below the ancestor */
	    rul__net_connect_target ((Net_Node) top_class_node,
	                             (Net_Node) rel_class_node, 
				     NET__E_ONLY_ENTRY);
	}

	/*
	**  Last, weave the new class node into this family tree
	*/
	weave_class_node_into_family (top_class_node, new_class_node);
}



#ifdef DEBUG_CLASS_WEAVE
#define GET_NODE_NAME(x) \
	rul__mol_get_printform(rul__decl_get_class_name((x)->this_class))
#endif



/***********************************
**                                **
**  WEAVE_CLASS_NODE_INTO_FAMILY  **
**                                **
***********************************/

static void  weave_class_node_into_family (
			Net_Class_Node cur_class_node, 
			Net_Class_Node new_class_node)
{
	/*
	**  Weave the new class node into the network of related
	**  class nodes, somewhere below the cur_class_node
	**  (which better be a superclass of the new class).
	*/
	Class tmp_class, new_class, cur_class;
	Net_Class_Node tmp_class_node;
	Net_Link lnk, next_link;

#ifdef DEBUG_CLASS_WEAVE
	rul__ios_printf (RUL__C_STD_ERR, "\n  Weaving %s into tree below %s",
		GET_NODE_NAME(new_class_node), GET_NODE_NAME(cur_class_node));
#endif

        assert (rul___net_node_is_valid ((Net_Node) cur_class_node));
        assert (is_class_node (cur_class_node));
        assert (rul___net_node_is_valid ((Net_Node) new_class_node));
        assert (is_class_node (new_class_node));

	new_class = new_class_node->this_class;
	cur_class = cur_class_node->this_class;
	assert (rul__decl_is_subclass_of (new_class, cur_class));

	lnk = cur_class_node->first_link;
	while (lnk != NULL) {
	    if (is_class_node (lnk->target_node)) {

		tmp_class_node = (Net_Class_Node) lnk->target_node;
		tmp_class = tmp_class_node->this_class;

		if (rul__decl_is_subclass_of (new_class, tmp_class)) {
		    /*
		    **  If new_class is a subclass of tmp_class,
		    **  call this routine recursively.
		    */
		    weave_class_node_into_family (
			tmp_class_node, new_class_node);
		    return;

		} else if (rul__decl_is_subclass_of (tmp_class, new_class)) {
		    /*
		    **  If, on the other hand, tmp_class is a subclass of
		    **  the new_class, then we need to insert the new_class
		    **  here.  We also need to collect tmp_class plus any
		    **  children of curr_class that are subclasses of the
		    **  the new_class, and move them all below the
		    **  new_class_node.
		    */
#ifdef DEBUG_CLASS_WEAVE
 		    rul__ios_printf (RUL__C_STD_ERR,
				"\n    Inserting %s and shifting %s down",
				GET_NODE_NAME(new_class_node), 
				GET_NODE_NAME(tmp_class_node)); 
#endif

		    lnk->target_node = (Net_Node) new_class_node;
		    rul__net_connect_target ((Net_Node) new_class_node,
				(Net_Node) tmp_class_node, NET__E_ONLY_ENTRY);

		    /*  and now to collect any others...  */
		    while (lnk != NULL) {
			next_link = lnk->next_link;
			if (is_class_node (lnk->target_node)) {
			    tmp_class_node = (Net_Class_Node) lnk->target_node;
			    tmp_class = tmp_class_node->this_class;
			    if (tmp_class != new_class  &&
				rul__decl_is_subclass_of(tmp_class,new_class))
			    {
				/*
				**  Found another descendent of new_class.
				**  Remove it from this list, and add it as a
				**  child of new_class_node.
				*/
#ifdef DEBUG_CLASS_WEAVE
 		    rul__ios_printf (RUL__C_STD_ERR, "\n    Also shifting %s down",
			GET_NODE_NAME(tmp_class_node)); 
#endif
				net_unlink_link (&cur_class_node->first_link,
						 &cur_class_node->last_link,
						 lnk);
				net_append_link (&new_class_node->first_link,
						 &new_class_node->last_link,
						 lnk);
				lnk->source_node = (Net_Node) new_class_node;
			    }
			}
			lnk = next_link;
		    }
		    return;
		}
	    }
	    lnk = lnk->next_link;
	}

	/*
	**  If this new_class was not a subclass or a superclass of any of
	**  the children class nodes of cur_class_node, then just make it
	**  another child of cur_class_node.
	*/
	rul__net_connect_target ((Net_Node) cur_class_node,
	                         (Net_Node) new_class_node, 
				 NET__E_ONLY_ENTRY);
}





/*********************
**                  **
**  FIND_DECL_NODE  **
**                  **
*********************/

static Net_Decl_Node find_decl_node (Decl_Block db)
{
	Net_Decl_Node db_node;
	Net_Link lnk;

	lnk = SA_net_decl_root;
	while (lnk) {
	    assert (is_decl_node (lnk->target_node));
	    db_node = (Net_Decl_Node) lnk->target_node;
	    if (db_node->declaring_block == db) {
		return (db_node);
	    }
	    lnk = lnk->next_link;
	}
	return (NULL);
}




/**********************
**                   **
**  FIND_CLASS_NODE  **
**                   **
**********************/

static Net_Class_Node find_class_node (
		Net_Decl_Node db_node, Class ce_class)
{
	Net_Class_Node class_node;
	Net_Link lnk;

	lnk = db_node->first_link;
	while (lnk) {
	    if (is_class_node (lnk->target_node)) {
		class_node = (Net_Class_Node) lnk->target_node;
		
	        if (class_node->this_class == ce_class) {
		    return (class_node);
	        } else if (rul__decl_is_subclass_of (ce_class, 
						 class_node->this_class)) {
		    lnk = class_node->first_link;
	        } else {
		    lnk = lnk->next_link;
		}
	    } else {
	        lnk = lnk->next_link;
	    }
	}
	return (NULL);
}




/**********************
**                   **
**  NET_APPEND_LINK  **
**                   **
**********************/

static void net_append_link (Net_Link *first_link_ptr, 
			     Net_Link *last_link_ptr, Net_Link new_link)
{
	Net_Link tmp;

	assert (*first_link_ptr == NULL ||
		rul___net_link_is_valid (*first_link_ptr));
	assert (*last_link_ptr == NULL ||
		rul___net_link_is_valid (*last_link_ptr));
	assert (rul___net_link_is_valid (new_link));

	if (*last_link_ptr) {
	    tmp = *last_link_ptr;
	    while (tmp->next_link) tmp = tmp->next_link;
	    tmp->next_link = new_link;
	    *last_link_ptr = new_link;
	} else {
	    *first_link_ptr = new_link;
	    *last_link_ptr = new_link;
	}
}



/**********************
**                   **
**  NET_UNLINK_LINK  **
**                   **
**********************/

static void net_unlink_link (Net_Link *first_link_ptr,
                             Net_Link *last_link_ptr, 
			     Net_Link link_to_remove)
{
	Net_Link tmp, prev;

	assert (rul___net_link_is_valid (*first_link_ptr));
	assert (rul___net_link_is_valid (*last_link_ptr));
	assert (rul___net_link_is_valid (link_to_remove));

#ifdef DEBUG_CLASS_WEAVE
	rul__ios_printf (RUL__C_STD_ERR, "\n    List before unlinking %s:",
		GET_NODE_NAME((Net_Class_Node)link_to_remove->target_node));
	tmp = *first_link_ptr;
	while (tmp != NULL) {
	    if (is_class_node (tmp->target_node))
		rul__ios_printf (RUL__C_STD_ERR, "\n        %s",
			     GET_NODE_NAME((Net_Class_Node)tmp->target_node));
	    tmp = tmp->next_link;
	}
#endif

	if (*first_link_ptr == link_to_remove) {
	    /*
	    **  Target link was the first in the list
	    */
	    *first_link_ptr = link_to_remove->next_link;
	    if (*last_link_ptr == link_to_remove) {
	        /*  if necessary, clean up the last-link  */
	        *last_link_ptr = NULL;
	    }

	} else {
	    /*
	    **  We need to search the list of links for the target.
	    */
	    prev = NULL;
	    tmp = *first_link_ptr;
	    while (tmp != link_to_remove) {
		prev = tmp;
		tmp = tmp->next_link;
		assert (tmp != NULL);
	    }
	    assert (prev != NULL);
	    prev->next_link = link_to_remove->next_link;
	    if (*last_link_ptr == link_to_remove) {
	        *last_link_ptr = prev;
	    }
	}

	link_to_remove->next_link = NULL;

#ifdef DEBUG_CLASS_WEAVE
	rul__ios_printf (RUL__C_STD_ERR, "\n    List after unlink:");
	tmp = *first_link_ptr;
	while (tmp != NULL) {
	    if (is_class_node (tmp->target_node))
		rul__ios_printf (RUL__C_STD_ERR, "\n        %s",
			     GET_NODE_NAME((Net_Class_Node)tmp->target_node));
	    tmp = tmp->next_link;
	}
#endif
}




/***********************
**                    **
**  NET_PREPEND_LINK  **
**                    **
***********************/

static void net_prepend_link (Net_Link *first_link_ptr,
			      Net_Link *last_link_ptr, Net_Link new_link)
{
	Net_Link tmp;

	assert (*first_link_ptr == NULL  ||
		rul___net_link_is_valid (*first_link_ptr));
	assert (*last_link_ptr == NULL  ||
		rul___net_link_is_valid (*last_link_ptr));
	assert (rul___net_link_is_valid (new_link));

	new_link->next_link = *first_link_ptr;
	*first_link_ptr = new_link;
	if (*last_link_ptr == NULL)
	  *last_link_ptr = new_link;
}



#define USED_ONLY_WITH_LINEAR_SUBNETS (nt_node->first_link->next_link == NULL)



/*************************
**                      **
**  RUL__NET_FIND_TAIL  **
**                      **
*************************/

Net_Node rul__net_find_tail (Net_Node node)
{
	/*
	**  Note:   Implementation assumes linear (1 level) subnet.
	**	    If non-linear subnets are ever allowed, then this 
	**	    function will need to be re-written to return a 
	**	    collection of all the subnet's tails.
	*/
	Net_Nonterm_Node nt_node;

	assert (rul___net_node_is_valid (node));
	assert (is_nonterm_node (node));

	nt_node = (Net_Nonterm_Node) node;
	while (nt_node->first_link != NULL) {

	    assert (USED_ONLY_WITH_LINEAR_SUBNETS);
	    assert (is_nonterm_node (nt_node->first_link->target_node));

	    nt_node = (Net_Nonterm_Node) nt_node->first_link->target_node;
	}
	return ((Net_Node) nt_node);
}




/**********************************
**                               **
**  RUL__NET_MAX_HSH_IN_A_CHILD  **
**                               **
**********************************/

long rul__net_max_hsh_in_a_child (Net_Node node)
{
	Net_Link lnk;
	Net_Nonterm_Node nt_node;
	Net_2_In_Node tin;
	long maxval = 0;

	assert (rul___net_node_is_valid (node));
	assert (is_nonterm_node (node));

	nt_node = (Net_Nonterm_Node) node;

	lnk = nt_node->first_link;
	while (lnk) {
	    if (is_2_in_node (lnk->target_node)) {
		tin = (Net_2_In_Node) lnk->target_node;
		maxval = MAX (maxval, 
			      rul__dyar_get_length(tin->left_entry_hsh_values));
	    }
	    lnk = lnk->next_link;
	}
	return (maxval);
}




/**********************************
**                               **
**  RUL__NET_MAX_AUX_IN_A_CHILD  **
**                               **
**********************************/

long rul__net_max_aux_in_a_child (Net_Node node)
{
	Net_Link lnk;
	Net_Nonterm_Node nt_node;
	Net_2_In_Node tin;
	long maxval = 0;

	assert (rul___net_node_is_valid (node));
	assert (is_nonterm_node (node));

	nt_node = (Net_Nonterm_Node) node;

	lnk = nt_node->first_link;
	while (lnk) {
	    if (is_2_in_node (lnk->target_node)) {
		tin = (Net_2_In_Node) lnk->target_node;
		if (lnk->type == NET__E_LEFT_ENTRY) {
		    maxval = MAX (maxval, 
				   rul__dyar_get_length (
						tin->left_entry_aux_values));
		} else if (lnk->type == NET__E_RIGHT_ENTRY) {
		    maxval = MAX (maxval, 
				   rul__dyar_get_length (
						tin->right_entry_aux_values));
		}
	    }
	    lnk = lnk->next_link;
	}
	return (maxval);
}




/*****************************
**                          **
**  CLEAR_ALL_VISITED_TAGS  **
**                          **
*****************************/

static void clear_all_visited_tags (void)
{
	Net_Node node;

	node = SA_net_node_list;
	while (node) {
	    node->visited_tag = FALSE;
	    node = node->global_next_node;
	}
}




/*****************************
**                          **
**  RUL__NET_FOR_EACH_NODE  **
**                          **
*****************************/

void rul__net_for_each_node (void (*func_ptr)(Net_Node node), 
			     Boolean predive_invocation)
{
	Net_Link lnk;

	if (SA_net_decl_root) {
	    clear_all_visited_tags ();

	    lnk = SA_net_decl_root;
	    while (lnk) {
		for_each_child_node (func_ptr, lnk->target_node, 
				     predive_invocation);
		lnk = lnk->next_link;
	    }
	}
}



/**************************
**                       **
**  FOR_EACH_CHILD_NODE  **
**                       **
**************************/

static void for_each_child_node (
			void (*func_ptr)(Net_Node node),
			Net_Node cur_node,
			Boolean predive_invocation)
{
	Net_Link lnk;
	Net_Nonterm_Node nt_node;

	if (cur_node == NULL) return;
	assert (rul___net_node_is_valid (cur_node));

	if (predive_invocation) {
	    /*  Invoke the supplied function, if predive mode  */
	    (func_ptr) (cur_node);
	}
	cur_node->visited_tag = TRUE;

	/*  Next, return if this node has no children  */
	if (! is_nonterm_node (cur_node)) return;

	/*  
	**  If this node does have children, and those children
	**  have not yet been visited, then recursively visit them.
	*/ 
	nt_node = (Net_Nonterm_Node) cur_node;
	lnk = nt_node->first_link;
	while (lnk) {
	    if (lnk->target_node->visited_tag == FALSE) {
	        for_each_child_node (func_ptr, lnk->target_node, 
				     predive_invocation);
	    }
	    lnk = lnk->next_link;
	}

	if (predive_invocation == FALSE) {
	    /*  Invoke the supplied function, if postdive mode  */
	    (func_ptr) (cur_node);
	}
}




/******************************
**                           **
**  NET_PROPAGATE_NODE_INFO  **
**                           **
******************************/

static void net_propagate_node_info (
			Net_Node from_node, 
			Net_Node to_node, 
			Net_Link_Type lnk_type)
{
	long out_obj_cnt = 0;
	Net_2_In_Node tin, tin2;
	Net_CSE_Node csn;

	assert (rul___net_node_is_valid (from_node));
	assert (rul___net_node_is_valid (to_node));

	/*
	**  Compute the out-bound object count from the source node
	*/
	if (is_1_in_node (from_node)) {
	    out_obj_cnt = 1;

	} else if (is_2_in_node (from_node)) {
	    tin = (Net_2_In_Node) from_node;

	    /*  2-input out-bound object count depends on sign of the node */
	    if (is_2_in_pos_node(from_node)) {
		out_obj_cnt = tin->left_entry_obj_count + 
				tin->right_entry_obj_count;
	    } else {
		/*  Negated 2-input node  */
		out_obj_cnt = tin->left_entry_obj_count;
	    }
	}

	/*
	**  Now, propagate that object count
	*/
	if (is_2_in_node (to_node)) {
	    tin2 = (Net_2_In_Node) to_node;
	    if (lnk_type == NET__E_LEFT_ENTRY) {
		tin2->left_entry_obj_count = 
			    MAX (tin2->left_entry_obj_count, out_obj_cnt);
	    } else {
		tin2->right_entry_obj_count = 
			    MAX (tin2->right_entry_obj_count, out_obj_cnt);
	    }
	} else if (is_cse_node(to_node)) {
	    csn = (Net_CSE_Node) to_node;
	    csn->object_count = MAX (csn->object_count, out_obj_cnt);
	}
}




/***********************************
**                                **
**  RUL__NET_PROPAGATE_NODE_INFO  **
**                                **
***********************************/

void	rul__net_propagate_node_info (void)
{
	Net_Link lnk;

	if (SA_net_decl_root) {

	    lnk = SA_net_decl_root;
	    while (lnk) {
		for_each_child_link (
			net_propagate_node_info,
			lnk->target_node);
		lnk = lnk->next_link;
	    }
	}
}




/**************************
**                       **
**  FOR_EACH_CHILD_LINK  **
**                       **
**************************/

static void for_each_child_link (
			void (*func_ptr)(Net_Node from_node, 
					 Net_Node to_node,
					 Net_Link_Type direction),
			Net_Node cur_node)
{
	Net_Link lnk;
	Net_Nonterm_Node nt_node;

	if (cur_node == NULL) return;
	assert (rul___net_node_is_valid (cur_node));

	/*  Next, return if this node has no children  */
	if (! is_nonterm_node (cur_node)) return;

	/*  
	**  If this node does have children, and those children
	**  have not yet been visited, then recursively visit them.
	*/ 
	nt_node = (Net_Nonterm_Node) cur_node;
	lnk = nt_node->first_link;
	while (lnk) {

	    /*  Invoke the supplied function  */
	    (func_ptr) (cur_node, lnk->target_node, lnk->type);

	    /*  Recursively invoke it for children  */
	    for_each_child_link (func_ptr, lnk->target_node);

	    lnk = lnk->next_link;
	}
}




/***********************************
**                                **
**  RUL__NET_ADD_LHS_VAR_BINDING  **
**                                **
***********************************/

long rul__net_add_lhs_var_binding (Net_Node node, long ce_index,
		Molecule var_name, Value val)
{
	Dynamic_Array binding_list = NULL;
	Net_1_In_Node oin;
	Net_2_In_Node tin;
	Value assign_val;
	long lhs_var_id = LVAR__C_INVALID_VAR_NUMBER;


	/*  First, Make sure that the binding list has been initialized  */

	if (is_1_in_node (node)) {
	    oin = (Net_1_In_Node) node;
	    if (oin->value_bindings == NULL) {
		oin->value_bindings = rul__dyar_create_array (15);
	    }
	    binding_list = oin->value_bindings;
	} else if (is_2_in_node (node)) {
	    tin = (Net_2_In_Node) node;
	    if (tin->value_bindings == NULL) {
		tin->value_bindings = rul__dyar_create_array (15);
	    }
	    binding_list = tin->value_bindings;
	}


	/*  Next, check to see if there is already an equivalent binding  */

	if (var_name != NULL) {
	    /*  If it's named, we better know about it...  */
	    lhs_var_id = rul__lvar_get_id_number (var_name, ce_index);
	    assert (lhs_var_id != LVAR__C_INVALID_VAR_NUMBER);

	} else if (rul__val_is_position (val)) {
	    /*  Does it have a name we don't know about here ?  */
	    lhs_var_id = rul__lvar_get_pos_var_number (ce_index, val);
	}

	if (lhs_var_id == LVAR__C_INVALID_VAR_NUMBER) {
	    /*  Last, is there an equivalent unnamed binding  */
	    lhs_var_id = net_equivalent_var_id (binding_list, val);
	}

	if (lhs_var_id == LVAR__C_INVALID_VAR_NUMBER) {

	    /*  This position has no named or unnamed binding yet  */
	    lhs_var_id = rul__lvar_create_unnamed ();
	}

#ifdef DEBUG_LVAR
	if (var_name == NULL)
	  var_name = rul__lvar_get_name (lhs_var_id);
	rul__ios_printf (RUL__C_STD_ERR,
		 "\n**  RUL__NET_ADD_LHS_VAR_BINDING  ** CE: %d  var: ",
			 ce_index);
	if (var_name != NULL)
	  rul__mol_print_printform (var_name, RUL__C_STD_ERR);
	else
	  rul__ios_printf (RUL__C_STD_ERR, "  <undef>");
        rul__ios_printf (RUL__C_STD_ERR, "  ID: %d", lhs_var_id);
#endif

	/*  Last, if there is no local binding, create one  */

	if (rul___net_get_var_position (binding_list, lhs_var_id) ==
		INVALID_POSITION) {
	    /*  create the assignment value, and add it to the list  */
	    assign_val = rul__val_create_assignment (
			    rul__val_create_lhs_variable (var_name, lhs_var_id),
			    val);
	    rul__dyar_append (binding_list, assign_val);
	}
	return (lhs_var_id);
}



/*********************************
**                              **
**  RUL__NET_ADD_TIN_VAR_USAGE  **
**                              **
*********************************/

void rul__net_add_tin_var_usage (Net_Node node, Value lhs_var_val)
{
	Net_2_In_Node tin;
	long index;

	assert (is_2_in_node (node));
	tin = (Net_2_In_Node) node;

	/*  Make sure that the usage list has been initialized  */
	if (tin->vars_used_here == NULL) {
	    tin->vars_used_here = rul__dyar_create_array (15);
	}

	index = rul___net_get_var_position (
			tin->vars_used_here, 
			rul__val_get_lhs_variable_id (lhs_var_val));

	if (index == INVALID_POSITION) {
	    /*  
	    **  If it's not here already, 
	    **  create the value, and add it to the list
	    */
	    rul__dyar_append (tin->vars_used_here, 
			      rul__val_copy_value (lhs_var_val));
	}
}




/**********************************
**                               **
**  RUL__NET_ADD_MATCH_HSH_VARS  **
**                               **
**********************************/

void rul__net_add_match_hsh_vars (Net_Node node, 
				   Value attr_val, Value other_val)
{
	Net_2_In_Node tin;

	assert (is_2_in_node (node));
	tin = (Net_2_In_Node) node;

	/*  Make sure that the hsh lists has been initialized  */
	if (tin->left_entry_hsh_values == NULL) {
	    tin->left_entry_hsh_values = rul__dyar_create_array (15);
	}
	if (tin->right_entry_hsh_values == NULL) {
	    tin->right_entry_hsh_values = rul__dyar_create_array (15);
	}

	rul__dyar_append (tin->left_entry_hsh_values,  other_val);
	rul__dyar_append (tin->right_entry_hsh_values, attr_val);
}




/************************************
**                                 **
**  RUL___NET_ADD_LHS_VAR_TO_LIST  **
**                                 **
************************************/

void rul___net_add_lhs_var_to_list (Dynamic_Array *sites_ptr,
				    Value lhs_var_val)
{
	Dynamic_Array site_list;
	long lhs_var_id;
	long index;

	/*  Make sure that this binding list has been initialized  */
	if (*sites_ptr == NULL) {
	    *sites_ptr = rul__dyar_create_array (15);
	}
	site_list = *sites_ptr;

	/*  Check to see if this lhs_var_id is already in the list  */
	lhs_var_id = rul__val_get_lhs_variable_id (lhs_var_val);
	index = rul___net_get_var_position (site_list, lhs_var_id);

	if (index == INVALID_POSITION) {
	    /*  If it was not already in the list, add it  */
	    rul__dyar_append (site_list, rul__val_copy_value (lhs_var_val));
	}
}




/*********************************
**                              **
**  RUL___NET_GET_VAR_POSITION  **
**                              **
*********************************/

long rul___net_get_var_position (Dynamic_Array val_array, long target_id)
{
	long count, index;
	Value val_tmp;

	if (val_array == NULL) return (INVALID_POSITION);

	count = rul__dyar_get_length (val_array);

	for (index=0; index<count; index++) {

	    val_tmp = (Value) rul__dyar_get_nth (val_array, index);
	    if (rul__val_is_lhs_variable (val_tmp)) {
		/*  for references  */
		if (rul__val_get_lhs_variable_id (val_tmp) == target_id) {
		    return (index);
		}
	    } else if (rul__val_is_assignment (val_tmp)) {
		/*  for bindings  */
		val_tmp = rul__val_get_assignment_to (val_tmp);
		if (rul__val_get_lhs_variable_id (val_tmp) == target_id) {
		    return (index);
		}
	    } else {
		assert (FALSE); /*  bogus value type in bind or ref lists */
	    }
	}
	return (INVALID_POSITION);
}



/**********************************
**                               **
**  RUL___NET_IS_VAR_BOUND_HERE  **
**                               **
**********************************/

Boolean rul___net_is_var_bound_here (Net_Node node, long id)
	/*
	**  Given the id of an LHS variable, 
	**  see if this variable is bound in this node.
	*/
{
	Net_1_In_Node oin;
	Net_2_In_Node tin;
	long i;

	assert (rul___net_node_is_valid (node));

	if (is_1_in_node (node)) {

	    oin = (Net_1_In_Node) node;
	    i = rul___net_get_var_position (oin->value_bindings, id);
	    if (i != INVALID_POSITION) return (TRUE);

	} else if (is_2_in_node (node)) {

	    tin = (Net_2_In_Node) node;

	    i = rul___net_get_var_position (tin->value_bindings, id);
	    if (i != INVALID_POSITION) return (TRUE);
	}
	return (FALSE);
}



/*************************************
**                                  **
**  RUL___NET_IS_VAR_IN_HSH_VALUES  **
**                                  **
*************************************/

Boolean rul___net_is_var_in_hsh_values (Net_Node node, long id)
	/*
	**  Given the id of an LHS variable, 
	**  see if this variable is eq tested in this node.
	*/
{
	Net_2_In_Node tin;
	long i;

	assert (rul___net_node_is_valid (node));

	if (is_2_in_node (node)) {

	    tin = (Net_2_In_Node) node;

	    i = rul___net_get_var_position (tin->left_entry_hsh_values, id);
	    if (i != INVALID_POSITION) return (TRUE);

	    i = rul___net_get_var_position (tin->right_entry_hsh_values, id);
	    if (i != INVALID_POSITION) return (TRUE);
	}
	return (FALSE);
}




/*******************************
**                            **
**  RUL___NET_IS_VAR_VISIBLE  **
**                            **
*******************************/

Boolean rul___net_is_var_visible (Net_Node node, long id)
	/*
	**  Given the id of an LHS variable, 
	**  see if this node already knows about it.
	*/
{
	Net_1_In_Node oin;
	Net_2_In_Node tin;
	long i;

	assert (rul___net_node_is_valid (node));

	if (is_1_in_node (node)) {

	    /*  If this is a 1-input node, it can only be bound locally  */
	    oin = (Net_1_In_Node) node;
	    i = rul___net_get_var_position (oin->value_bindings, id);
	    if (i != INVALID_POSITION) return (TRUE);

	} else if (is_2_in_node (node)) {

	    tin = (Net_2_In_Node) node;

	    i = rul___net_get_var_position (tin->value_bindings, id);
	    if (i != INVALID_POSITION) return (TRUE);

	    i = rul___net_get_var_position (tin->left_entry_hsh_values, id);
	    if (i != INVALID_POSITION) return (TRUE);

	    i = rul___net_get_var_position (tin->left_entry_aux_values, id);
	    if (i != INVALID_POSITION) return (TRUE);

	    i = rul___net_get_var_position (tin->right_entry_hsh_values, id);
	    if (i != INVALID_POSITION) return (TRUE);

	    i = rul___net_get_var_position (tin->right_entry_aux_values, id);
	    if (i != INVALID_POSITION) return (TRUE);
	}

	return (FALSE);
}




/**********************************
**                               **
**  RUL___NET_FIND_RIGHT_SOURCE  **
**                               **
**********************************/

Net_Node rul___net_find_right_source (Net_Node cur_node)
{
	long i;
	Net_Link lnk;
	Net_Nonterm_Node nt_node;

	assert (rul___net_node_is_valid (cur_node));
	assert (is_nonterm_node (cur_node));

	for (i=0; i<rul__dyar_get_length(cur_node->source_nodes); i++) {

	    nt_node = (Net_Nonterm_Node) 
			rul__dyar_get_nth (cur_node->source_nodes, i);
	    assert (is_nonterm_node (nt_node));

	    lnk = nt_node->first_link;
	    while (lnk) {
	        if (lnk->type == NET__E_RIGHT_ENTRY  &&  
			cur_node == lnk->target_node) {
		    return ((Net_Node) nt_node);
	        }
		lnk = lnk->next_link;
	    }
	}
	assert (FALSE);	/*  should always be exactly one right parent  */
	return (NULL);
}




/****************************
**                         **
**  NET_EQUIVALENT_VAR_ID  **
**                         **
****************************/

static long net_equivalent_var_id (Dynamic_Array binding_list, Value val)
{
	long i, len;
	Value bind_val, var_val, assigned_val;

	len = rul__dyar_get_length (binding_list);
	for (i=0; i<len; i++) {

	    bind_val = (Value) rul__dyar_get_nth (binding_list, i);
	    assert (rul__val_is_assignment (bind_val));

	    assigned_val = rul__val_get_assignment_from (bind_val);
	    if (rul__val_equivalent (val, assigned_val)) {
		var_val = rul__val_get_assignment_to (bind_val);
		return (rul__val_get_lhs_variable_id (var_val));
	    }
	}
	return (LVAR__C_INVALID_VAR_NUMBER);
}




/************************************
**                                 **
**  RUL__NET_ADD_NODE_TO_CUR_RULE  **
**                                 **
************************************/

void rul__net_add_node_to_cur_rule (Net_Node node)
{
    if (is_1_in_node (node)  ||  is_2_in_node (node)) {

        if (SA_nodes_in_cur_rule == NULL)
	    SA_nodes_in_cur_rule = rul__dyar_create_array (10);

	rul__dyar_append (SA_nodes_in_cur_rule, node);
    }
}



/***********************************
**                                **
**  RUL__NET_GET_CUR_RULES_NODES  **
**                                **
***********************************/

Dynamic_Array rul__net_get_cur_rules_nodes (void)
{
	Dynamic_Array tmp;

	assert (SA_nodes_in_cur_rule != NULL);
	tmp = SA_nodes_in_cur_rule;
	SA_nodes_in_cur_rule = NULL;
	return (tmp);
}



/*************************************
**                                  **
**  RUL___NET_INCREMENT_TEST_COUNT  **
**                                  **
*************************************/

void rul___net_increment_test_count (void)
{
	SL_test_specificity_this_rule += 1;
}


Net_Node rul__net_find_matching_tin (Net_Node le, Net_Node re, Boolean pos)
{
   Net_Node node;
   Net_Nonterm_Node nt_node;
   Net_Link lnk;
   long     i;

   if (!is_nonterm_node (le)) {
      nt_node = (Net_Nonterm_Node) le;
      lnk = nt_node->first_link;

      while (lnk) {
	 if (lnk->type == NET__E_LEFT_ENTRY) {
	    node = lnk->target_node;
	    if ((pos && is_2_in_pos_node (node)) ||
		(!pos && is_2_in_neg_node (node))) {
	       if (re == rul___net_find_right_source (node))
		 return node;
	    }
	 }
	 lnk = lnk->next_link;
      }
   }

   return NULL;
}


#define INDENT_UNIT "   "

#ifndef NDEBUG



/*********************
**                  **
**  PRINT_NET_NODE  **
**                  **
*********************/

static void print_net_node (Net_Node node, Net_Link_Type from_dir, char *indent)
{
	char name[2*RUL_C_MAX_SYMBOL_SIZE+1];
	char name2[RUL_C_MAX_SYMBOL_SIZE+1];
	char whitespace[120], next_indent[120], *type_str = "?";
	Mol_Symbol mol;
	long col, i;
	Net_2_In_Node tin;

	switch (node->type) {

		case NET__E_DECL :
			type_str = "Declaring-Block:";
			mol = rul__decl_get_decl_name (
				((Net_Decl_Node) node)->declaring_block);
			rul__mol_use_printform (mol, name, 
				RUL_C_MAX_SYMBOL_SIZE);
			break;

		case NET__E_CLASS :
			type_str = "Class:";
			mol = rul__decl_get_class_name (
				((Net_Class_Node) node)->this_class);
			rul__mol_use_printform (mol, name, 
				RUL_C_MAX_SYMBOL_SIZE);
			break;

		case NET__E_1_IN :
			type_str = "One-Input:";
			rul__mol_use_printform (node->source_rule, name2,
				RUL_C_MAX_SYMBOL_SIZE);
			sprintf (name, "Rule:  %s, CE: %ld", name2,
				    ((Net_1_In_Node) node)->ce_index);
			break;

		case NET__E_2_IN_POS :
			type_str = "Two-Input (+):";
			if (from_dir == NET__E_RIGHT_ENTRY) {
			    rul__mol_use_printform (node->source_rule, name2,
				    RUL_C_MAX_SYMBOL_SIZE);
			    sprintf (name, "Left input: CE %ld of %s", 
				    node->rule_index, name2);
			} else {
			    strcpy (name, "");
			}
			break;

		case NET__E_2_IN_NEG :
			type_str = "Two-Input (-):";
			if (from_dir == NET__E_RIGHT_ENTRY) {
			    rul__mol_use_printform (node->source_rule, name2,
				    RUL_C_MAX_SYMBOL_SIZE);
			    sprintf (name, "Left input: CE %ld of %s", 
				    node->rule_index, name2);
			} else {
			    strcpy (name, "");
			}
			break;

		case NET__E_CSE :
			type_str = "Conflict-Set:";
			rul__mol_use_printform (node->source_rule, name2,
				RUL_C_MAX_SYMBOL_SIZE);
			sprintf (name, "Rule: %s", name2);
			break;

		default :
			assert (BOGUS_NODE_TYPE);
	}
	col = strlen (indent) + strlen (type_str) + 6;
	whitespace[0] = '\0';
	while (col < 40) {
	    if (col % 3 == 0) {
		strcat (whitespace, ".");
	    } else {
		strcat (whitespace, " ");
	    }
	    col++;
	}
	if (node->visited_tag == FALSE) {
	    /*
	    **  When visiting a node for the first time,
	    **  print the node's form plus all attached tests
	    */

	    node->visited_tag = TRUE;

	    rul__ios_printf (RUL__C_STD_ERR, "\n#%03d%s%s %s  %s",
                        node->node_number, indent, type_str, whitespace, name);

	    if (is_1_in_node (node)) {

		/*  Compute the indent for printing node info  */
		col = strlen (indent)+ strlen (type_str) + 6;
		for (i=0; i<col; i++) next_indent[i] = ' ';
		next_indent[i-1] = '\0';
		strcat (next_indent, whitespace);
		strcat (next_indent, "  ");

		/*  Print out the class for this OIN  */
		mol = rul__decl_get_class_name (
				((Net_1_In_Node) node)->ce_class);
		rul__mol_use_printform (mol, name, RUL_C_MAX_SYMBOL_SIZE);
		rul__ios_printf (RUL__C_STD_ERR, "\n%sClass: %s", 
				next_indent, name);

		/*  Then print out all the tests for this OIN  */
		net_print_node_vars (node, next_indent);
		rul__net_print_tests (((Net_1_In_Node) node)->first_test,
					next_indent);

	    } else if (is_2_in_node (node)) {

		/*  Compute the indent for printing node info  */
		col = strlen (indent)+ strlen (type_str) + 6;
		for (i=0; i<col; i++) next_indent[i] = ' ';
		next_indent[i-1] = '\0';
		strcat (next_indent, whitespace);
		strcat (next_indent, "  ");

		/*  Then print out all the tests for this TIN  */
		tin = (Net_2_In_Node) node;
		net_print_node_vars (node, next_indent);
		rul__net_print_tests (tin->first_test, next_indent);
	    }
	} else {
	    /*  
	    **  When re-visiting a node, print only the simple form
	    */
	    rul__ios_printf (RUL__C_STD_ERR, "\n#%03d%s%s %s  %s",
                        node->node_number, indent, type_str, whitespace, name);
	}
}




/********************
**                 **
**  PRINT_NETWORK  **
**                 **
********************/

static void print_network (Net_Node node, char *indent)
{
	char whitespace[250];
	Net_Link lnk;
	Net_Nonterm_Node nt_node;

	if (node == NULL) return;

	if (! is_nonterm_node (node)) return;

	nt_node = (Net_Nonterm_Node) node;
	sprintf (whitespace, "%s%s", indent, INDENT_UNIT);

	lnk = nt_node->first_link;
	while (lnk) {
	    rul__ios_printf (RUL__C_STD_ERR, "\n %s%s", 
				whitespace, link_type_to_chars (lnk->type));
	    if (lnk->target_node->visited_tag == FALSE) {
		/*
		**  When visiting a node for the first time,
		**  print the node with it's attached tests, and
		**  print all it's children.
		*/
		print_net_node (lnk->target_node, lnk->type, whitespace);
	        print_network (lnk->target_node, whitespace);

	    } else {
		/*
		**  When re-visiting a node, print only this node.
		*/
		print_net_node (lnk->target_node, lnk->type, whitespace);
	    }
	    lnk = lnk->next_link;
	}
}




/*************************
**                      **
**  LINK_TYPE_TO_CHARS  **
**                      **
*************************/

static char *link_type_to_chars (Net_Link_Type type)
{
	switch (type) {
	    case NET__E_ONLY_ENTRY :
			return ("On to --");
			break;
	    case NET__E_LEFT_ENTRY :
			return ("Down to LEFT entry of --");
			break;
	    case NET__E_RIGHT_ENTRY :
			return ("Down to RIGHT entry of --");
			break;
	}
	return ("- ? -");
}



/*********************
**                  **
**  RUL__NET_PRINT  **
**                  **
*********************/

void rul__net_print (void)
{
	Net_Link lnk;

	if (SA_net_decl_root) {
	    rul__ios_printf (RUL__C_STD_ERR, "\n");
	    clear_all_visited_tags ();

	    lnk = SA_net_decl_root;
	    while (lnk) {
		print_net_node (lnk->target_node, lnk->type, "  ");
	        print_network (lnk->target_node, "  ");
		lnk = lnk->next_link;
	    }
	}
	rul__ios_printf (RUL__C_STD_ERR, "\n");
}



/**************************
**                       **
**  NET_PRINT_NODE_VARS  **
**                       **
**************************/

static void net_print_node_vars (Net_Node node, char *indent)
{
	Net_2_In_Node tin;
	Net_1_In_Node oin;
	char *env_val;

	env_val = getenv ("TIN_DEBUG_NET");
	if (!env_val || !(strchr(env_val,'v') || strchr(env_val,'V'))) return;

	if (is_1_in_node (node)) {

	    oin = (Net_1_In_Node) node;
	    net_print_val_array (oin->value_bindings, "Bindings:", indent);

	} else if (is_2_in_node (node)) {

	    tin = (Net_2_In_Node) node;
	    net_print_val_array (tin->value_bindings, "Bindings:", indent);
	    net_print_val_array (tin->vars_used_here, "Tested Vars:", indent);
	    net_print_val_array (tin->left_entry_hsh_values, 
				 "Left Entry Hash Values:", indent);
	    net_print_val_array (tin->left_entry_aux_values, 
				 "Left Entry Aux Values:", indent);
	    net_print_val_array (tin->right_entry_hsh_values, 
				 "Right Entry Hash Values:", indent);
	    net_print_val_array (tin->right_entry_aux_values, 
				 "Right Entry Aux Values:", indent);
	}
}



/**************************
**                       **
**  NET_PRINT_VAL_ARRAY  **
**                       **
**************************/

static void net_print_val_array (Dynamic_Array array, char *title, char *indent)
{
	long 	i, len;
	Value 	val;
	Boolean print_domain = FALSE;
	char	*env_val, *shape_dom_str;
	extern 	char *rul___val_shape_domain_str (Value val);

	env_val = getenv ("TIN_DEBUG_NET");
	if (env_val && (strchr(env_val,'d') || strchr(env_val,'D')))
	    print_domain = TRUE;

	len = rul__dyar_get_length (array);
	if (len > 0) {
	    rul__ios_printf (RUL__C_STD_ERR, "\n%s%s", indent, title);
	    for (i=0; i<len; i++) {
		val = rul__dyar_get_nth (array, i);
		rul__ios_printf (RUL__C_STD_ERR, "\n%s   ", indent);
		rul___val_print (val);

		if (print_domain) {
		    rul__ios_printf (RUL__C_STD_ERR, "\n%s      ", indent);
		    shape_dom_str = rul___val_shape_domain_str (val);
		    rul__ios_printf (RUL__C_STD_ERR, "%s", shape_dom_str);
		    rul__mem_free (shape_dom_str);
		}
	    }
	}
}


#endif /* ifndef NDEBUG */
