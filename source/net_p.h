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
 *	RULEWORKS compiler system
 *
 * ABSTRACT:
 *	This module defines the internals for the match network data
 *	structures.
 *
 * MODIFIED BY:
 *	DEC	Digital Equipment Corporation
**	CPQ	Compaq Computer Corporation
 *
 * MODIFICATION HISTORY:
 *
 *	 6-Jan-1993	DEC	Initial version
 *	01-Dec-1999	CPQ	Release with GPL
 */

#ifndef _NET_HAS_BEEN_LOADED_	/* special order dependency */
#include <net.h>
#endif


#define NET__C_NODE_VERIFY      7909
#define NET__C_LINK_VERIFY      7808

#define BOGUS_NODE_TYPE         0 /* for dead end asserts */

#define INVALID_POSITION	-1 /* for lhs variable arrays  */


/*
**	Links between nodes are annotated with the direction
**	from which they will enter the target nodes
*/
typedef struct net_link *Net_Link;


/*
**	Subsystem private functions
*/
Boolean rul___net_node_is_valid (Net_Node node);
Boolean rul___net_link_is_valid (Net_Link lnk);


/*
**
**	Net_Node's have been organized into an object class
**	hierarchy as shown below.  The typedefs in this file fake
**	that hierarchy given the limited facilities available in C.
**
**			             Net_Node
**		                   ___/    \___
**		               ___/            \___
**		              /                    \
**                     Net_Nonterm_Node           Net_CSE_Node
**	         _____/    /    \  \________________________
**         _____/         /      \_________                 \
**        /              /                 \                 \
** Net_Decl_Node   Net_Class_Node       Net_1_In_Node    Net_2_In_Node
**                                                          /     \
**                                           ______________/       \
**                                          /                       \
**                                   Net_2_In_Pos_Node       Net_2_In_Neg_Node
**
*/


#define BASE	      012000
/*
**	First level types
*/
#define NONTERM		0600
#define NODE_CSE	0700
/*
**	Second level types
*/
#define NODE_DECL	0030
#define NODE_CLASS	0050
#define NODE_1_IN	0060
#define NODE_2_IN	0070
/*
**	Third Level
*/
#define NODE_2_IN_POS	0004
#define NODE_2_IN_NEG	0005
/*
**	Level Masks
*/
#define MASK1 0700
#define MASK2 0770
#define MASK3 0777


typedef enum {
	NET__E_INVALID = 0,

	NET__E_CSE	= (BASE | NODE_CSE),
	NET__E_DECL	= (BASE | NONTERM | NODE_DECL),
	NET__E_CLASS	= (BASE | NONTERM | NODE_CLASS),
	NET__E_1_IN 	= (BASE | NONTERM | NODE_1_IN),
	NET__E_2_IN_POS	= (BASE | NONTERM | NODE_2_IN | NODE_2_IN_POS),
	NET__E_2_IN_NEG	= (BASE | NONTERM | NODE_2_IN | NODE_2_IN_NEG)
} Net_Node_Type;


#define is_net_node(x)		(((int)x->type & ~MASK3) == BASE)
#define is_nonterm_node(x)	(((int)x->type & MASK1) == NONTERM)
#define is_cse_node(x)		(x->type == NET__E_CSE)
#define is_decl_node(x)		(x->type == NET__E_DECL)
#define is_class_node(x)	(x->type == NET__E_CLASS)
#define is_1_in_node(x)		(x->type == NET__E_1_IN)
#define is_2_in_node(x)		(((int)x->type & MASK2) == (NONTERM|NODE_2_IN))
#define is_2_in_pos_node(x)	(x->type == NET__E_2_IN_POS)
#define is_2_in_neg_node(x)	(x->type == NET__E_2_IN_NEG)


#ifdef	NDEBUG
#define NET_DEBUG_COMMON
#else
#define NET_DEBUG_COMMON	\
		long			verification;
#endif



#define NET_NODE_COMMON		\
		Net_Node_Type		type; 			\
		long			node_number;		\
		Boolean			visited_tag;		\
		NET_DEBUG_COMMON				\
								\
		Net_Node		global_next_node; 	\
		Net_Node		global_prev_node; 	\
								\
		Dynamic_Array		source_nodes;		\
								\
		Mol_Symbol		source_rule; 		\
		long			rule_index;

struct net_node {
	NET_NODE_COMMON
};



#define NET_NONTERM_COMMON	\
		Net_Link		first_link;		\
		Net_Link		last_link;		\
		long			ce_index;

typedef struct net_nonterm_node {
	NET_NODE_COMMON
	NET_NONTERM_COMMON
} *Net_Nonterm_Node;


#define DECL_NODE_COMMON	\
		Decl_Block	  	declaring_block;

typedef struct net_decl_node {
	NET_NODE_COMMON
	NET_NONTERM_COMMON
	DECL_NODE_COMMON
} *Net_Decl_Node;



#define CLASS_NODE_COMMON	\
		Class			this_class;

typedef struct net_class_node {
	NET_NODE_COMMON
	NET_NONTERM_COMMON
	CLASS_NODE_COMMON
} *Net_Class_Node;



#define ONE_IN_NODE_COMMON	\
		Class			ce_class;		\
		Net_Test	  	first_test;		\
		Dynamic_Array		value_bindings;

typedef struct net_1_in_node {
	NET_NODE_COMMON
	NET_NONTERM_COMMON
	ONE_IN_NODE_COMMON
} *Net_1_In_Node;



#define TWO_IN_NODE_COMMON	\
		Net_Test	  	first_test; 		\
		Dynamic_Array		value_bindings;		\
		Dynamic_Array		vars_used_here;		\
								\
		long			left_entry_obj_count;	\
		Dynamic_Array		left_entry_aux_values;	\
		Dynamic_Array		left_entry_hsh_values;	\
								\
		long			right_entry_obj_count;	\
		Dynamic_Array		right_entry_aux_values;	\
		Dynamic_Array		right_entry_hsh_values;


typedef struct net_2_in_node {
	NET_NODE_COMMON
	NET_NONTERM_COMMON
	TWO_IN_NODE_COMMON
} *Net_2_In_Node;



#define CSE_NODE_COMMON	\
		long			class_specificity;	\
		long			test_specificity;	\
		long			object_count;

typedef struct net_cse_node {
        NET_NODE_COMMON
        CSE_NODE_COMMON
} *Net_CSE_Node;





struct net_link {
	struct net_link		*next_link;

	Net_Link_Type		type;

	Net_Node		source_node;
	Net_Node		target_node;

	struct net_link		*global_next_link;
	struct net_link		*global_prev_link;
#ifndef NDEBUG
	long			verification;
#endif
};



struct net_test {
	Pred_Type		 predicate;
	Pred_Type		 opt_pred;
	Value			 value_1;
	Value			 value_2;
	struct net_test		*next_test;
};


/*
**  Functions private to the NET subsystem and it's close buddies
*/

Net_Link	rul___net_get_root_link (void);

long		rul___net_get_var_position (
				Dynamic_Array val_array,
				long lhs_var_id);

void		rul___net_add_lhs_var_to_list (
				Dynamic_Array *sites_ptr,
				Value lhs_var_val);

Boolean 	rul___net_is_var_visible (
				Net_Node node, 
				long lhs_var_id);

Boolean 	rul___net_is_var_bound_here (
				Net_Node node, 
				long lhs_var_id);

Boolean 	rul___net_is_var_in_hsh_values (
				Net_Node node, 
				long lhs_var_id);

Net_Node 	rul___net_find_right_source (Net_Node cur_node);

void		rul___net_increment_test_count (void);


#ifndef NDEBUG
void		rul__net_print (void);
#endif
