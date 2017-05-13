#ifndef _NET_HAS_BEEN_LOADED_
/****************************************************************************
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
 *     RULEWORKS compiler
 *
 * ABSTRACT:
 *	This module contains the exported functions
 *	for the discrimination network subsystem
 *	that represents the collected rule left-hand-sides.
 *
 * MODIFIED BY:
 *	DEC	Digital Equipment Corporation
**	CPQ	Compaq Computer Corporation
 *
 * MODIFICATION HISTORY:
 *
 *	28-Jan-1993	DEC	Initial version
 *	01-Dec-1999	CPQ	Compaq Computer Corporation
 */


typedef enum {
	NET__E_BAD_LINK = 0,

	NET__E_ONLY_ENTRY = 1537,
	NET__E_LEFT_ENTRY,
	NET__E_RIGHT_ENTRY
} Net_Link_Type;



/*
**  Complete network manipulation functions
*/

void 		rul__net_build (Ast_Node lhs, Mol_Symbol rule_name);

Boolean 	rul__net_is_valid (void);

void		rul__net_clear_network (void);


/*
**  Net_Node manipulation functions
*/
Net_Node	rul__net_create_decl_node (Mol_Symbol block_name);

Net_Node	rul__net_create_oin_node (
			Class ce_class,
			Mol_Symbol rule_name,
			long rule_index,
			long ce_index);

void		rul__net_add_node_to_cur_rule (
		        Net_Node node);

Net_Node	rul__net_create_tin_node (
			Boolean is_positive,
			Mol_Symbol rule_name,
			long rule_index,
			long ce_index);

Net_Node	rul__net_create_cs_node (
			Mol_Symbol rule_name,
			long rule_index,
			long ce_count);

void		rul__net_connect_target (
			Net_Node source_node, 
			Net_Node target_node, 
			Net_Link_Type type);

Net_Node 	rul__net_find_tail (Net_Node head_node);



/*
**  Net_Test manipulations
*/

Boolean 	rul__net_test_is_simple (Net_Test test);

Boolean 	rul__net_test_is_local (Net_Test test, long ce_index);

Net_Test 	rul__net_create_test (Pred_Type pred, Pred_Type opred,
				      Value val_1, Value val_2);

void 		rul__net_free_tests (Net_Test cur_test);

void		rul__net_attach_test (Net_Node node, Net_Test test);

void 		rul__net_print_tests (Net_Test first_test, char *indent);

void		rul__net_test_print (Net_Test test, char *indent);



/*
**  Functions for manipulating 2-input node variable propagation
*/

long		rul__net_add_lhs_var_binding (Net_Node node, long ce_index,
						Molecule var_name,
						Value bound_value);

void 		rul__net_add_tin_var_usage (Net_Node node, Value lhs_var_val);

void		rul__net_add_match_hsh_vars (Net_Node node, Value attr_val,
						Value other_val);


/*
**  Miscellaneous net functions
*/

long		rul__net_next_rule_index (void);
long 		rul__net_get_cur_rule_index (void);
Dynamic_Array	rul__net_get_cur_rules_nodes (void);

void		rul__net_propagate_node_info (void);

void		rul__net_for_each_node (
			void (*func_ptr)(Net_Node node),
			Boolean predive_invocation);
#define				NET__C_PRE_DIVE		TRUE
#define				NET__C_POST_DIVE	FALSE


long		rul__net_max_hsh_in_a_child (Net_Node node);
long		rul__net_max_aux_in_a_child (Net_Node node);

long 		rul__net_get_oin_ce_index (Net_Node node);
Class 		rul__net_get_oin_class (Net_Node node);

Boolean 	rul__net_node_is_1_input (Net_Node node);
Boolean 	rul__net_node_is_2_input (Net_Node node);

Net_Node        rul__net_find_matching_tin (Net_Node le_node,
					    Net_Node re_node,
					    Boolean pos_ce);


#define _NET_HAS_BEEN_LOADED_ 1
#endif
