/****************************************************************************
**                                                                         **
**                              A S T . H                                  **
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
 *	This module provides the compiler functions for 
 *	dealing with abstract syntax tree nodes.
 *
 * MODIFIED BY:
 *	DEC	Digital Equipment Corporation
 *	CPQ	Compaq Computer Corporation
 *
 * MODIFICATION HISTORY:
 *
 *	6-Dec-1992	DEC	Initial version
 *	1-Dec-1999	CPQ	Release with GPL
 */


/*
 * Node Types
 */
typedef enum {
		AST__E_INVALID = 0,

		AST__E_ENTRY_BLOCK = 8539,
		AST__E_DECL_BLOCK,
		AST__E_RULE_BLOCK,
		AST__E_END_BLOCK,
		AST__E_OBJ_CLASS,
		AST__E_EXT_RT_DECL,
		AST__E_METHOD,
		AST__E_GENERIC_METHOD,
		AST__E_RULE,
		AST__E_CATCH,
		AST__E_RULE_GROUP,
		AST__E_END_GROUP,
		AST__E_ON_ENTRY,
		AST__E_ON_EVERY,
		AST__E_ON_EMPTY,
		AST__E_ON_EXIT,

		/* Entry block clauses */

		AST__E_USES,
		AST__E_ACTIVATES,
		AST__E_STRATEGY,

		/* LHS AST node types */

		AST__E_RULE_LHS,
		AST__E_POS_CE,
		AST__E_NEG_CE,
		AST__E_CE_DISJ,
		AST__E_CE_NEG_CONJ,
		AST__E_CE,
		AST__E_ATTR_TEST,
		AST__E_ATTR_EXPR,
		AST__E_ATTR_INDEX,
		AST__E_PRED_TEST,
		AST__E_LHS_VAR_BIND,
		AST__E_VALUE_EXPR,
		AST__E_VALUE_DISJ,
		AST__E_EXT_RT_CALL,	/* External routine call */
		AST__E_BTI_RT_CALL,	/* Built in (stateless) routine call */

		AST__E_DOLLAR_LAST,
		AST__E_FAILURE,		/* QUIT $FAILURE */
		AST__E_SUCCESS,		/* QUIT $SUCCESS */

		AST__E_CONSTANT,
		AST__E_VARIABLE,
		AST__E_QUOTED_VAR,

		AST__E_COND_EXPR,
		AST__E_ARITH_EXPR,
		AST__E_OPR_PLUS,
		AST__E_OPR_MINUS,
		AST__E_OPR_TIMES,
		AST__E_OPR_DIVIDE,
		AST__E_OPR_MODULUS,
		AST__E_OPR_UMINUS,

		AST__E_OPR_AND,
		AST__E_OPR_OR,
		AST__E_OPR_NOT,

		/*
		 * Object class declaration nodes
		 */
		AST__E_OBJ_ATTRS,	/* Collector for attributes */
		AST__E_OBJ_CLASS_INH,	/* inherits-from */

		AST__E_ATTRIBUTE,	/* attribute */
		AST__E_ATTR_DOMAIN,
		AST__E_ATTR_CLASS,
		AST__E_ATTR_COMPOUND,	/* children are compound_default */
					/* and fill */
		AST__E_ATTR_DEFAULT,	/* Default value (for scalar) */
		AST__E_ATTR_COMPOUND_DEFAULT,
		AST__E_ATTR_COMPOUND_FILL,

		/* External Routine Declaration nodes */

		AST__E_ALIAS_DECL,
		AST__E_ACCEPTS_DECL,
		AST__E_ACCEPTS_PARAM,
		AST__E_RETURNS_DECL,
		AST__E_METHOD_WHEN,
		AST__E_METHOD_PARAM,
		AST__E_EXT_TYPE,
		AST__E_EXT_LEN,
		AST__E_EXT_MECH,

		/* RHS Action nodes */
		AST__E_RULE_RHS,
		AST__E_METHOD_RHS,
		AST__E_RHS_ACTION_TERM,

		AST__E_ADDSTATE_ACTION,
		AST__E_AFTER_ACTION,
		AST__E_AT_ACTION,
		AST__E_BIND_ACTION,
		AST__E_BUILD_ACTION,
		AST__E_CALL_INHERITED,
		AST__E_CLOSEFILE_ACTION,
		AST__E_COPY_ACTION,
		AST__E_DEBUG_ACTION,
		AST__E_DEFAULT_ACTION,
		AST__E_FOR_EACH_ACTION,
		AST__E_IF_ACTION,
		AST__E_IF_ELSE,
		AST__E_IF_THEN,
		AST__E_MAKE_ACTION,
		AST__E_MODIFY_ACTION,
		AST__E_OPENFILE_ACTION,
		AST__E_QUIT_ACTION,
		AST__E_REMOVE_ACTION,
		AST__E_REMOVE_EVERY_ACTION,
		AST__E_RESTORESTATE_ACTION,
		AST__E_RETURN_ACTION,
		AST__E_SAVESTATE_ACTION,
		AST__E_SPECIALIZE_ACTION,
		AST__E_TRACE_ACTION,
		AST__E_WHILE_ACTION,
		AST__E_WHILE_DO,
		AST__E_WRITE_ACTION,
		AST__E_SQL_ATTACH_ACTION,
		AST__E_SQL_COMMIT_ACTION,
		AST__E_SQL_DELETE_ACTION,
		AST__E_SQL_DETACH_ACTION,
		AST__E_SQL_FETCH_EACH_ACTION,
		AST__E_SQL_FETCH_AS_OBJECT_ACTION,
		AST__E_SQL_INSERT_ACTION,
		AST__E_SQL_INSERT_FROM_OBJECT_ACTION,
		AST__E_SQL_ROLLBACK_ACTION,
		AST__E_SQL_START_ACTION,
		AST__E_SQL_UPDATE_ACTION,
		AST__E_SQL_UPDATE_FROM_OBJECT_ACTION,
		AST__E_SQL_PATHNAME,
		AST__E_SQL_RSE,
		AST__E_SQL_VARS,
		AST__E_SQL_ACTIONS,

		/* Built in (stated) routine call */
		AST__E_BTI_RHS_CALL,
		AST__E_BTI_ACCEPT_ATOM,
		AST__E_BTI_ACCEPTLINE_COMPOUND,
		AST__E_BTI_COMPOUND,
		AST__E_BTI_CONCAT,
		AST__E_BTI_CRLF,
		AST__E_BTI_EVERY,
		AST__E_BTI_FLOAT,
		AST__E_BTI_GENATOM,
		AST__E_BTI_GENINT,
		AST__E_BTI_GET,
		AST__E_BTI_INTEGER,
		AST__E_BTI_IS_OPEN,
		AST__E_BTI_LENGTH,
		AST__E_BTI_MAX,
		AST__E_BTI_MIN,
		AST__E_BTI_NTH,
		AST__E_BTI_POSITION,
		AST__E_BTI_RJUST,
		AST__E_BTI_SUBCOMPOUND,
		AST__E_BTI_SUBSYMBOL,
		AST__E_BTI_SYMBOL,
		AST__E_BTI_SYMBOL_LENGTH,
		AST__E_BTI_TABTO


} Ast_Node_Type;


typedef enum {
		AST__E_TYPE_NULL = 7964,		/* No Value */
			/*  Pointer types  */
		AST__E_TYPE_MOL,			/* Molecule */
		AST__E_TYPE_OPAQUE,			/* void * value */
		AST__E_TYPE_NET_NODE,			/* Net_Node */
		AST__E_TYPE_NET_TEST,			/* Net_Test */
		AST__E_TYPE_VALUE,			/* Value */
		AST__E_TYPE_ASCIZ,			/* char * */
			/*  Enumerated types  */
		AST__E_TYPE_PRED,			/* Predicate test */
		AST__E_TYPE_EXT_TYPE,			/* Ext_Type */
		AST__E_TYPE_EXT_MECH,			/* Ext_Mech */
		AST__E_TYPE_CARDINALITY,		/* Cardinality */
		AST__E_TYPE_STRATEGY			/* Strategy */
} Ast_Value_Type;



/*
**	Ast_Node:  Creation and Modification Functions
*/

Ast_Node	rul__ast_node (Ast_Node_Type type);
Ast_Node	rul__ast_mol_node (Ast_Node_Type type,
				   Molecule attached_value);
Ast_Node	rul__ast_opaque_node (Ast_Node_Type type,
				      void *attached_value);
Ast_Node	rul__ast_str_node (char *str);
Ast_Node	rul__ast_pred_type_node (Pred_Type ptype);
Ast_Node	rul__ast_ext_type_node (Ext_Type etype);
Ast_Node	rul__ast_ext_mech_node (Ext_Mech mtype);
Ast_Node	rul__ast_cardinality_node (Cardinality a_len);

void		rul__ast_free (Ast_Node top_of_tree);
void		rul__ast_delete_node (Ast_Node top_of_subtree);

void		rul__ast_attach_children (Ast_Node parent, 
					  long child_count, ...);
void		rul__ast_attach_child (Ast_Node parent, Ast_Node child);
Ast_Node	rul__ast_append_sibling (Ast_Node child, Ast_Node next_child);
void		rul__ast_insert_sibling (Ast_Node old_child, 
					 Ast_Node new_child);
Ast_Node	rul__ast_append_opt_sibling (Ast_Node child1,
					     Ast_Node child2);
void		rul__ast_attach_value (Ast_Node node,
					Ast_Value_Type type,
					void *value);
void		rul__ast_attach_long_value (Ast_Node node,
					Ast_Value_Type type,
					long value);
void		rul__ast_reset_node_type (Ast_Node node, 
					Ast_Node_Type new_type);
void            rul__ast_build_string (Ast_Node node);

/*
**	Ast_Node:  Query Functions
*/
Ast_Node_Type	rul__ast_get_type (Ast_Node node);
void	       *rul__ast_get_value (Ast_Node node);
long	        rul__ast_get_long_value (Ast_Node node);
Ast_Value_Type  rul__ast_get_value_type (Ast_Node node);
Ast_Node	rul__ast_get_parent (Ast_Node node);
Ast_Node	rul__ast_get_child (Ast_Node node);
Ast_Node	rul__ast_get_sibling (Ast_Node node);
Ast_Node	rul__ast_get_previous_sibling (Ast_Node node);
long		rul__ast_get_child_count (Ast_Node node);
long		rul__ast_nearest_line_number (Ast_Node node);
char 	       *rul__ast_pred_to_symbol (Pred_Type predicate);
char           *rul__ast_get_ce_string (Ast_Node ast);


/*
**	For AST subsystem internal use only.
*/ 
char	       *rul___ast_type_to_string (Ast_Node_Type type);
char 	       *rul___ast_value_type_to_string (Ast_Value_Type type);

#ifndef NDEBUG
/*
**	For Debugging:
*/
void		rul__ast_print (
#ifdef __VMS
				Ast_Node *node);
#else
				Ast_Node node);
#endif

char 	       *rul__ast_pred_to_string (Pred_Type predicate);
#endif /* ifndef NDEBUG */
