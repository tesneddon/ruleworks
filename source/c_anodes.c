/****************************************************************************
**                                                                         **
**                  C M P _ A S T _ N O D E S . C                          **
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
 *	dealing with abstract syntax tree nodes.
 *
 * MODIFIED BY:
 *	DEC	Digital Equipment Corporation
 *	CPQ	Compaq Computer Corporation
 *
 * MODIFICATION HISTORY:
 *
 *	7-Dec-1992	DEC	Initial version
 *	1-Dec-1999	CPQ	Release with GPL
 */

#include <common.h>
#include <stdarg.h>
#include <ctype.h>		/* For toupper */
#include <cmp_comm.h>
#include <ast.h>
#include <mol.h>
#include <ios.h>
#include <val.h>

#define AST__C_MAGIC_NUMBER	50831


struct ast_node {
	Ast_Node_Type		type;

	struct ast_node  	*parent;
	struct ast_node  	*next_sibling;
	struct ast_node  	*prev_sibling;
	struct ast_node  	*child;

	Ast_Value_Type		value_type;
	union { long l_val;
		void *a_val; }	value;

	long			source_line;
#ifndef NDEBUG
	unsigned short		verification;
#endif
};

static char    *SC_ast_string = NULL;
static long     SC_ast_string_length = 0;
static long     SC_ast_string_size = 0;
#define AST_STR_INCR 100
static void     ast_build_node_subtree (Ast_Node ast);
static void     ast_build_node (Ast_Node ast);
static void     ast_get_ce_subtree (Ast_Node ast, Ast_Node node);



#ifndef NDEBUG


/*******************
**                **
**  AST_IS_VALID  **
**                **
*******************/

static	Boolean ast_is_valid (Ast_Node node)
	/*
	**	Use ONLY inside assert statements
	*/
{
	return (node != NULL  &&  node->verification == AST__C_MAGIC_NUMBER);
}
#endif



/********************
**                 **
**  RUL__AST_NODE  **
**                 **
********************/

Ast_Node	rul__ast_node (Ast_Node_Type type)
{
	Ast_Node ast;

	ast = rul__mem_malloc (sizeof(struct ast_node));
	assert (ast != NULL);

	ast->type = type;
	ast->parent = NULL;
	ast->child = NULL;
	ast->next_sibling = NULL;
	ast->prev_sibling = NULL;
	ast->value_type = AST__E_TYPE_NULL;
	ast->value.a_val = NULL;
	ast->source_line = IOS__C_NOT_A_LINE_NUM;

#ifndef NDEBUG
	ast->verification = AST__C_MAGIC_NUMBER;
#endif

	return (ast);
}



/************************
**                     **
**  RUL__AST_MOL_NODE  **
**                     **
************************/

Ast_Node  rul__ast_mol_node (Ast_Node_Type type, Molecule mol)
	/*
	**	Convienence function that creates an Ast_Node and 
	**	then attaches a Molecule value to that node.
	*/
{
	Ast_Node ast;
	assert (rul__mol_is_valid (mol));

	ast = rul__ast_node (type);
	rul__ast_attach_value ( ast, AST__E_TYPE_MOL, mol);
	ast->source_line = rul__ios_curr_line_num (RUL__C_SOURCE);
	return (ast);
}



/***************************
**                        **
**  RUL__AST_OPAQUE_NODE  **
**                        **
***************************/

Ast_Node  rul__ast_opaque_node (Ast_Node_Type type, void *val)
	/*
	**	Convienence function that creates an Ast_Node and 
	**	then attaches an opaque value to that node.
	*/
{
	Ast_Node ast;

	ast = rul__ast_node (type);
	rul__ast_attach_value (ast, AST__E_TYPE_OPAQUE, val);
	ast->source_line = rul__ios_curr_line_num (RUL__C_SOURCE);
	return (ast);
}




/******************************
**                           **
**  RUL__AST_PRED_TYPE_NODE  **
**                           **
******************************/

Ast_Node  rul__ast_pred_type_node (Pred_Type type)
	/*
	**	Convienence function that creates an AST__E_PRED_TEST
	**	Ast_Node and then attaches the predicate type to that node.
	*/
{
	Ast_Node ast;

	ast = rul__ast_node (AST__E_PRED_TEST);
	rul__ast_attach_long_value (ast, AST__E_TYPE_PRED, (long) type);
	ast->source_line = rul__ios_curr_line_num (RUL__C_SOURCE);
	return (ast);
}



/*****************************
**                          **
**  RUL__AST_EXT_TYPE_NODE  **
**                          **
*****************************/

Ast_Node  rul__ast_ext_type_node (Ext_Type type)
	/*
	**	Convienence function that creates an AST__E_EXT_TYPE
	**	Ast_Node and then attaches the external type to that node.
	*/
{
	Ast_Node ast;

	ast = rul__ast_node (AST__E_EXT_TYPE);
	rul__ast_attach_long_value (ast, AST__E_TYPE_EXT_TYPE, (long) type);
	ast->source_line = rul__ios_curr_line_num (RUL__C_SOURCE);
	return (ast);
}




/*****************************
**                          **
**  RUL__AST_EXT_MECH_NODE  **
**                          **
*****************************/

Ast_Node  rul__ast_ext_mech_node (Ext_Mech type)
	/*
	**	Convienence function that creates an AST__E_EXT_MECH
	**	Ast_Node and then attaches the mechanism type to that node.
	*/
{
	Ast_Node ast;

	ast = rul__ast_node (AST__E_EXT_MECH);
	rul__ast_attach_long_value (ast, AST__E_TYPE_EXT_MECH, (long) type);
	ast->source_line = rul__ios_curr_line_num (RUL__C_SOURCE);
	return (ast);
}



/********************************
**                             **
**  RUL__AST_CARDINALITY_NODE  **
**                             **
********************************/

Ast_Node  rul__ast_cardinality_node (Cardinality a_len)
	/*
	**	Convienence function that creates an AST__E_EXT_LEN
	**	Ast_Node and then attaches the length to that node.
	*/
{
	Ast_Node ast;

	ast = rul__ast_node (AST__E_EXT_LEN);
	rul__ast_attach_long_value (ast, AST__E_TYPE_CARDINALITY, (long) a_len);
	ast->source_line = rul__ios_curr_line_num (RUL__C_SOURCE);
	return (ast);
}




/****************************
**                         **
**  RUL__AST_STR_NODE	   **
**                         **
****************************/

Ast_Node rul__ast_str_node (char *str)
	/*
	 * Create a molecule from str, create and return a node with that 
	 * molecule as its value.
	 */
{
	long i;
	Ast_Node ast;
	char buff[RUL_C_MAX_SYMBOL_SIZE+1];

	ast = rul__ast_node (AST__E_CONSTANT);

	i = 0;
	while (str[i] != '\0') {
	    buff[i] = toupper (str[i]);
	    i++;
	}
	buff[i] = '\0';

	rul__ast_attach_value ( ast, AST__E_TYPE_MOL, 
				rul__mol_make_symbol (buff));
	ast->source_line = rul__ios_curr_line_num (RUL__C_SOURCE);
	return (ast);
}




/*******************************
**                            **
**  RUL__AST_RESET_NODE_TYPE  **
**                            **
*******************************/

void rul__ast_reset_node_type (Ast_Node node, Ast_Node_Type new_type)
{
	assert (ast_is_valid (node));

	node->type = new_type;
}




/********************
**                 **
**  RUL__AST_FREE  **
**                 **
********************/

void	rul__ast_free (Ast_Node ast)
{
	assert (ast_is_valid (ast));

	if (ast->child) {
	    rul__ast_free (ast->child);
	}
	if (ast->next_sibling) {
	    rul__ast_free (ast->next_sibling);
	}
	if (ast->value_type != AST__E_TYPE_NULL) {
	    /*  For some value types there may be special freeing required  */
	    if (ast->value_type == AST__E_TYPE_MOL) {
		rul__mol_decr_uses ((Molecule) ast->value.a_val);
	    }
	    else if (ast->value_type == AST__E_TYPE_VALUE) {
	        rul__val_free_value ((Value) ast->value.a_val);
	    }
	    else if (ast->value_type == AST__E_TYPE_ASCIZ) {
	        rul__mem_free ((char *) ast->value.a_val);
	    }
	}
	rul__mem_free (ast);
}



/*******************************
**                            **
**  RUL__AST_ATTACH_CHILDREN  **
**                            **
*******************************/

void	rul__ast_attach_children (Ast_Node parent, long child_count, ...)
{
	 va_list  ap;
	 Ast_Node children[30];
volatile Ast_Node next;
	 long     found_count = 0;
	 long     i = 0;

	assert (ast_is_valid (parent)  &&  parent->child == NULL);

	va_start (ap, child_count);

	while (i < child_count) {
	    next = va_arg (ap, Ast_Node);
	    if (next != NULL) {
		children[found_count] = next;
		found_count++;
	    }
	    i++;
	}
	va_end (ap);

	for (i=0; i<found_count; i++) {

	    assert (ast_is_valid (children[i])  && 
	    	    children[i]->parent == NULL);

	    if (i == 0) {
		rul__ast_attach_child (parent, children[i]);
	    } else {
		rul__ast_append_sibling (children[i-1], children[i]);
	    }
	    next = children[i];
	}

	if (found_count) {
	    while (next->next_sibling) {
	        next = next->next_sibling;
	        if (next->parent == NULL)
	            next->parent = parent;
	        else
	            assert (parent == next->parent);
	    }
	}
}




/****************************
**                         **
**  RUL__AST_ATTACH_CHILD  **
**                         **
****************************/

void	rul__ast_attach_child (Ast_Node parent, Ast_Node child)
{
        volatile Ast_Node sibling;

	assert (ast_is_valid (parent)  &&  parent->child == NULL);

	if (child != NULL) {
	    assert (ast_is_valid (child)  &&  child->parent == NULL);

	    parent->child = child;

	    /* set parent of all siblings */
	    sibling = child;
	    while (sibling != NULL) {
	      sibling->parent = parent;
	      sibling = sibling->next_sibling;
	    }
	}
}




/******************************
**                           **
**  RUL__AST_APPEND_SIBLING  **
**                           **
******************************/

Ast_Node rul__ast_append_sibling (Ast_Node child, Ast_Node next_child)
{
	Ast_Node tmp;

	assert (ast_is_valid (child));
	if (next_child == NULL) return (child);

	assert (ast_is_valid (next_child) &&
		next_child->prev_sibling == NULL);

	for (tmp = child; tmp->next_sibling != NULL; tmp = tmp->next_sibling)
	    ;

	tmp->next_sibling = next_child;
	next_child->prev_sibling = tmp;
	next_child->parent = tmp->parent;

	return (child);
}




/**********************************
**                               **
**  RUL__AST_APPEND_OPT_SIBLING  **
**                               **
**********************************/

Ast_Node rul__ast_append_opt_sibling (Ast_Node child1, Ast_Node child2)
{
	Ast_Node tmp;

	if (child1 == NULL) {
	    assert (child2 == NULL  ||  ast_is_valid (child2));
	    return (child2);
	} else {
	    assert (ast_is_valid (child1));
	    if (child2 == NULL) return (child1);
	}

	/*  Both args are valid Ast_Node's  */
	assert (ast_is_valid (child2));

	tmp = child1;
	while (tmp->next_sibling != NULL) tmp = tmp->next_sibling;

	tmp->next_sibling = child2;
	child2->prev_sibling = tmp;
	child2->parent = tmp->parent;
	return (child1);
}





/******************************
**                           **
**  RUL__AST_INSERT_SIBLING  **
**                           **
******************************/

void	rul__ast_insert_sibling (Ast_Node old_child, Ast_Node new_child)
	/*
	**	Inserts the new_child into the list 
	**	immediately after the old_child.
	*/
{
	Ast_Node tmp;

	assert (ast_is_valid (new_child)  &&  ast_is_valid (old_child));

	tmp = old_child->next_sibling;
	old_child->next_sibling = new_child;
	new_child->next_sibling = tmp;
	new_child->parent = old_child->parent;

	if (tmp) {
	    tmp->prev_sibling = new_child;
	}
	new_child->prev_sibling = old_child;
}


/***************************
**                        **
**  RUL__AST_DELETE_NODE  **
**                        **
***************************/

void	rul__ast_delete_node (Ast_Node node)
	/*
	**	Removes the node from sibling list (if any)
	**	then frees it and all its children.
	*/
{
	Ast_Node parent,prev_sib,next_sib;

	assert (ast_is_valid (node));

	parent   = node->parent;
	next_sib = node->next_sibling;
	prev_sib = node->prev_sibling;

	if (parent && parent->child == node)
	    parent->child = next_sib;
	if (prev_sib != NULL)
	    prev_sib->next_sibling = next_sib;
	if (next_sib != NULL)
	    next_sib->prev_sibling = prev_sib;

	node->next_sibling = NULL;
	
	rul__ast_free (node);
}




/****************************
**                         **
**  RUL__AST_ATTACH_VALUE  **
**                         **
****************************/

void	rul__ast_attach_value (Ast_Node node, 
			       Ast_Value_Type type, void *value)
{
   char *bufptr;

   assert (ast_is_valid (node));

   /* deal with the previous value, if needed  */
   if (node->value_type == AST__E_TYPE_MOL)
     rul__mol_decr_uses (node->value.a_val);
   else if (node->value_type == AST__E_TYPE_ASCIZ && type != AST__E_TYPE_ASCIZ)
     rul__mem_free (node->value.a_val);

   assert (type != AST__E_TYPE_PRED);
   assert (type != AST__E_TYPE_EXT_TYPE);
   assert (type != AST__E_TYPE_EXT_MECH);
   assert (type != AST__E_TYPE_CARDINALITY);
   assert (type != AST__E_TYPE_STRATEGY);

   if (type == AST__E_TYPE_ASCIZ) {
      assert (node->value_type == AST__E_TYPE_NULL ||
	      node->value_type == AST__E_TYPE_ASCIZ);
      if (node->value_type == AST__E_TYPE_NULL) {
	 node->value.a_val = rul__mem_malloc (strlen ((char*) value) + 1);
	 strcpy (node->value.a_val, value);
	 node->value_type = AST__E_TYPE_ASCIZ;
      }
      else {
	 node->value.a_val = rul__mem_realloc (node->value.a_val,
					       strlen (node->value.a_val) +
					       strlen (value) + 1);
	 strcat (node->value.a_val, value);
      }
   }
   else if (type == AST__E_TYPE_VALUE && value == (void *) -1) {
      node->value_type = AST__E_TYPE_NULL;
      node->value.a_val = NULL;
   }
   else {
      node->value_type = type;
      node->value.a_val = value;
   }
}



/*********************************
**                              **
**  RUL__AST_ATTACH_LONG_VALUE  **
**                              **
*********************************/

void	rul__ast_attach_long_value (Ast_Node node, 
				Ast_Value_Type type, long value)
{
	assert (ast_is_valid (node));

	/* deal with the previous value, if needed  */
	if (node->value_type == AST__E_TYPE_MOL)
	  rul__mol_decr_uses (node->value.a_val);

	assert (type != AST__E_TYPE_MOL);
	assert (type != AST__E_TYPE_OPAQUE);
	assert (type != AST__E_TYPE_NET_NODE);
	assert (type != AST__E_TYPE_NET_TEST);
	assert (type != AST__E_TYPE_VALUE);

	node->value_type = type;
	node->value.l_val = value;
}




/************************
**                     **
**  RUL__AST_GET_TYPE  **
**                     **
************************/

Ast_Node_Type	rul__ast_get_type (Ast_Node node)
{
	assert (ast_is_valid (node));

	return (node->type);
}



/*************************
**                      **
**  RUL__AST_GET_VALUE  **
**                      **
*************************/

void	*rul__ast_get_value (Ast_Node node)
{
	assert (ast_is_valid (node));

	assert (node->value_type != AST__E_TYPE_PRED);
        assert (node->value_type != AST__E_TYPE_EXT_TYPE);
        assert (node->value_type != AST__E_TYPE_EXT_MECH);
        assert (node->value_type != AST__E_TYPE_CARDINALITY);
        assert (node->value_type != AST__E_TYPE_STRATEGY);

	return (node->value.a_val);
}



/******************************
**                           **
**  RUL__AST_GET_LONG_VALUE  **
**                           **
******************************/

long	rul__ast_get_long_value (Ast_Node node)
{
	assert (ast_is_valid (node));

        assert (node->value_type != AST__E_TYPE_MOL);
        assert (node->value_type != AST__E_TYPE_OPAQUE);
        assert (node->value_type != AST__E_TYPE_NET_NODE);
        assert (node->value_type != AST__E_TYPE_NET_TEST);
        assert (node->value_type != AST__E_TYPE_VALUE);

	return (node->value.l_val);
}




/******************************
**                           **
**  RUL__AST_GET_VALUE_TYPE  **
**                           **
******************************/

Ast_Value_Type  rul__ast_get_value_type (Ast_Node node)
{
	assert (ast_is_valid (node));

	return (node->value_type);
}



/**************************
**                       **
**  RUL__AST_GET_PARENT  **
**                       **
**************************/

Ast_Node	rul__ast_get_parent (Ast_Node node)
{
	assert (ast_is_valid (node));

	return (node->parent);
}




/*************************
**                      **
**  RUL__AST_GET_CHILD  **
**                      **
*************************/

Ast_Node	rul__ast_get_child (Ast_Node node)
{
	assert (ast_is_valid (node));

	return (node->child);
}



/***************************
**                        **
**  RUL__AST_GET_SIBLING  **
**                        **
***************************/

Ast_Node	rul__ast_get_sibling (Ast_Node node)
{
	assert (ast_is_valid (node));

	return (node->next_sibling);
}



/************************************
**                                 **
**  RUL__AST_GET_PREVIOUS_SIBLING  **
**                                 **
************************************/

Ast_Node	rul__ast_get_previous_sibling (Ast_Node node)
{
	assert (ast_is_valid (node));

	return (node->prev_sibling);
}



/*******************************
**                            **
**  RUL__AST_GET_CHILD_COUNT  **
**                            **
*******************************/

long	rul__ast_get_child_count (Ast_Node node)
{
	long i = 0;
	Ast_Node ast;

	assert (ast_is_valid (node));

	ast = node->child;
	while (ast) {
	    i++;
	    ast = ast->next_sibling;
	}
	return (i);
}



/***********************************
**                                **
**  RUL__AST_NEAREST_LINE_NUMBER  **
**                                **
***********************************/

long	rul__ast_nearest_line_number (Ast_Node ast)
{
	long line;

	if (ast == NULL) return (IOS__C_NOT_A_LINE_NUM);

	assert (ast_is_valid (ast));

	if (ast->source_line != IOS__C_NOT_A_LINE_NUM) {
	    return (ast->source_line);
	} else {
	    line = rul__ast_nearest_line_number (ast->child);
	    if (line == IOS__C_NOT_A_LINE_NUM) {
	        line = rul__ast_nearest_line_number (ast->next_sibling);
	    }
	    return (line);
	}
}


/****************************
**                         **
**  rul__ast_build_string  **
**                         **
****************************/

void rul__ast_build_string (Ast_Node ast)
{
   char     buf[RUL_C_MAX_SYMBOL_SIZE+1];

   if (SC_ast_string_size)
     *SC_ast_string = '\0';
   ast_build_node_subtree (ast->child);
   rul__ast_attach_value (ast, AST__E_TYPE_ASCIZ, SC_ast_string);
}

static void ast_build_node (Ast_Node ast)
{
   char     buf[RUL_C_MAX_SYMBOL_SIZE+1];
   char    *buf_ptr;
   long     len;

   if (ast->value_type != AST__E_TYPE_NULL) {
      switch (ast->value_type) {
       case AST__E_TYPE_MOL:
	 buf_ptr = buf;
	 rul__mol_use_readform ((Molecule) ast->value.a_val,
				buf, RUL_C_MAX_SYMBOL_SIZE+1);
	 break;

       case AST__E_TYPE_PRED:
	 buf_ptr = rul__ast_pred_to_symbol ((Pred_Type) ast->value.l_val);
	 break;

       default:
	 return;
      }
   }
   else
     return;

   len = strlen (buf_ptr);
   if ((SC_ast_string_length + len) >= SC_ast_string_size) {
      SC_ast_string_size = SC_ast_string_size + len + AST_STR_INCR;
      SC_ast_string = rul__mem_realloc (SC_ast_string, SC_ast_string_size);
   }
   strcat (SC_ast_string, buf_ptr);
   SC_ast_string_length += len;
}

static void ast_build_node_subtree (Ast_Node ast)
{
   while (ast) {
      ast_build_node (ast);
      ast_build_node_subtree (ast->child);
      ast = ast->next_sibling;
   }
}




/*****************************
**                          **
**  rul__ast_get_ce_string  **
**                          **
*****************************/

char *rul__ast_get_ce_string (Ast_Node ast)
{
   char     buf[RUL_C_MAX_SYMBOL_SIZE+1];

   assert (ast->child->type == AST__E_CE);

   rul__mol_use_readform ((Molecule) ast->child->child->value.a_val,
			  buf, RUL_C_MAX_SYMBOL_SIZE+1);
   rul__ast_attach_value (ast, AST__E_TYPE_ASCIZ, buf);
   ast_get_ce_subtree (ast, ast->child->child->next_sibling);
   return ((char *) ast->value.a_val);
}   

static void ast_get_ce_subtree (Ast_Node ast, Ast_Node node)
{

   while (node) {
      /* add a T for test and a B for a bind */
      if (node->type == AST__E_PRED_TEST)
	rul__ast_attach_value (ast, AST__E_TYPE_ASCIZ, "T");
      else if (node->type == AST__E_LHS_VAR_BIND)
	rul__ast_attach_value (ast, AST__E_TYPE_ASCIZ, "B");

      if (node->value_type == AST__E_TYPE_ASCIZ)
	rul__ast_attach_value (ast, AST__E_TYPE_ASCIZ, node->value.a_val);

      ast_get_ce_subtree (ast, node->child);
      node = node->next_sibling;
   }
}



#ifndef NDEBUG

void rul___ast_print_node (Ast_Node node, long depth);



/******************************
**                           **
**  RUL___AST_PRINT_SUBTREE  **
**                           **
******************************/

void	rul___ast_print_subtree (Ast_Node node, long depth)
{
	if (node == NULL) return;
	if (! ast_is_valid (node)) {
	    fprintf (stderr,"ERROR:  rul___ast_print_subtree got invalid node\n");
	    return;
	}

	/*
	**  Print this node, then its children, then its siblings.
	*/
	rul___ast_print_node (node, depth);

	rul___ast_print_subtree (node->child, depth +1);
	rul___ast_print_subtree (node->next_sibling, depth);
}




/***************************
**                        **
**  RUL___AST_PRINT_NODE  **
**                        **
***************************/

void rul___ast_print_node (Ast_Node node, long depth)
{
	long i, col;
	char *indent = "   ";
	char *type_str;
	char whitespace[256];
	Ast_Node construct_name_node;

	if (node == NULL) return;
	if (! ast_is_valid (node)) {
	    fprintf (stderr,"ERROR:  rul___ast_print_node got invalid node\n");
	    return;
	}

	/*
	**	Print this node.
	*/
	fprintf (stderr, "\n");
	whitespace[0] = '\0';
	for (i=0; i<depth; i++) {
	    strcat (whitespace, indent);
	}
	type_str = rul___ast_type_to_string (node->type);
	fprintf (stderr, "%s%s  ", whitespace, type_str);

	if (node->type == AST__E_RULE		||  
	    node->type == AST__E_CATCH  	||
	    node->type == AST__E_ENTRY_BLOCK	||
	    node->type == AST__E_DECL_BLOCK	||
	    node->type == AST__E_RULE_BLOCK	||
	    node->type == AST__E_OBJ_CLASS	||
	    node->type == AST__E_EXT_RT_DECL)
	{
	    /*  If the node is a named construct, print the name too  */
	    construct_name_node = node->child;
	    assert (construct_name_node->type == AST__E_CONSTANT);
	    assert (construct_name_node->value_type == AST__E_TYPE_MOL);
	    rul__mol_print_readform (
		(Molecule) construct_name_node->value.a_val, RUL__C_STD_ERR);

	} else if (node->value_type != AST__E_TYPE_NULL) {
	    col = (depth * strlen (indent)) + strlen (type_str) + 2 ;
	    whitespace[0] = '\0';
	    while (col < 49) {
		if (col % 3 == 0) {
		    strcat (whitespace, ".");
		} else {
		    strcat (whitespace, " ");
		}
		col++;
	    }
	    fprintf (stderr, "%s  ", whitespace);

	    switch (node->value_type) {
	      case AST__E_TYPE_MOL:
		fprintf (stderr, "Mol = ");
	        rul__mol_print_readform ((Molecule) node->value.a_val, 
					 RUL__C_STD_ERR);
		break;
	      case AST__E_TYPE_PRED:
		fprintf (stderr, "Pred = %s",
			 rul__ast_pred_to_string (
				(Pred_Type) node->value.l_val));
		break;
	      case AST__E_TYPE_ASCIZ:
		fprintf (stderr, "Asciz = %s", (char *) node->value.a_val);
		break;
	      default:
		fprintf (stderr, "%s", 
			 rul___ast_value_type_to_string (node->value_type));
	    }
	}
}




/*********************
**                  **
**  RUL__AST_PRINT  **
**                  **
*********************/

void	rul__ast_print (
#ifdef __VMS
		Ast_Node *node)
#else
		Ast_Node node)
#endif
{
	Ast_Node ast;
	fprintf (stderr, "\n");

#ifdef __VMS
	ast = *node;
#else
	ast = node;
#endif

	if (ast_is_valid (ast)) {
	    rul___ast_print_subtree (ast, 1);
	} else {
	    fprintf (stderr, 
		"ERROR:    rul__ast_print got invalid Ast_Node = %p", ast);
	}
	fprintf (stderr, "\n");
}

#endif /* ifndef NDEBUG */
