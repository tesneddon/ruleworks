/****************************************************************************
**                                                                         **
**                        C M P _ L V A R . C                              **
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
 *	Maintain a table of LHS variables for the current rule.
 *
 *  MODIFIED BY:
 *	DEC	Digiatl Equipment Corporation
 *	CPQ	Compaq Computer Corporation
 *
 *  MODIFICATION HISTORY:
 *	16-Feb-1993	DEC	Initial version
 *	01-Dec-1999	CPQ	Release fwith GPL
 */

#include <common.h>
#include <cmp_comm.h>
#include <ast.h>
#include <decl.h>
#include <dyar.h>
#include <gen.h>		/* here for rhs init of lhs variables */
#include <hash.h>
#include <mol.h>
#include <msg.h>
#include <cmp_msg.h>
#include <net.h>
#include <sem.h>
#include <val.h>
#include <lvar.h>

#ifdef DEBUG_LVAR
#include <ios.h>
#endif

#ifdef __MSDOS
#define  LVAR_HT_SIZE 		37
#else 
#define  LVAR_HT_SIZE 		73
#endif

#define  LVAR_DESC_VERIFY 	9001
#define  LVAR_BIND_VERIFY 	9002


typedef struct binding_site {
	Net_Node		node;
	long			ce_index;
	Mol_Symbol		attr_name;
	long			attr_offset;
	Value			attr_elem_index;
	Ast_Node		attr_elem_expr_ast;
	Boolean			is_length_bind;
	Boolean			is_last_elem_bind;
	struct binding_site 	*next_bind_site;
#ifndef NDEBUG
	long			verification;
#endif
} *Binding_Site;



typedef struct lhs_var_desc {
	Mol_Symbol		variable_name;
	long			unique_index;

	Decl_Domain		domain;		/* Type on LHS */
	Decl_Shape		shape;
	Class			is_a;
	Decl_Domain		rhs_domain;	/* Type on RHS, may be more  */
	Decl_Shape		rhs_shape;	/* specific than on LHS by   */
	Class			rhs_is_a;	/* info from LHS EQ tests    */

	Binding_Site		first_bind_site;
#ifndef NDEBUG
	long			verification;
#endif
} *Lhs_Var_Desc;


static Hash_Table 	SA_lhs_var_table = NULL;
static long		SL_lhs_var_index = 1;
static Dynamic_Array	SA_lhs_ce_signs = NULL;

static void 		lvar_init (void);
static Lhs_Var_Desc	lvar_create_var_desc (Mol_Symbol var_name);
static Boolean 		lvar_desc_is_valid (Lhs_Var_Desc desc);
static Boolean 		lvar_bind_site_is_valid (Binding_Site bind);
static Binding_Site 	lvar_get_nth_binding (Mol_Symbol var_name, long n);
static void 		lvar_free_var_desc (void *desc);
static Binding_Site	lvar_create_bind_site (Class ce_class, long ce_index, 
					Mol_Symbol attr_name, Value attr_elem);
static void 		lvar_attach_var_binding (Lhs_Var_Desc desc, 
					Binding_Site site);
static long 		lvar_ce_index_to_cse_index (long ce_index);
static Condition_Element_Type 
			lvar_get_ce_type (long ce_index);
static void 		lvar_add_subsequent_bind_site (Lhs_Var_Desc desc, 
					Binding_Site site, 
					Decl_Domain new_dom, 
					Decl_Shape new_shape,
					Class new_class,
					Ast_Node ast);
void 			lvar_set_rhs_type_from_lhs (Lhs_Var_Desc desc);
Decl_Domain 		lvar_more_specific_domain (Decl_Domain dom_1, 
					Decl_Domain dom_2);
static void  		lvar_tag_each_nested_lvar (Ast_Node cur_ast_node);


#ifndef NDEBUG


/*************************
**                      **
**  LVAR_DESC_IS_VALID  **
**                      **
*************************/

static Boolean lvar_desc_is_valid (Lhs_Var_Desc desc)
{
	return (desc  &&  desc->verification == LVAR_DESC_VERIFY);
}



/******************************
**                           **
**  LVAR_BIND_SITE_IS_VALID  **
**                           **
******************************/

static Boolean lvar_bind_site_is_valid (Binding_Site site)
{
	return (site  &&  site->verification == LVAR_BIND_VERIFY);
}

#endif




/***************************
**                        **
**  LVAR_CREATE_VAR_DESC  **
**                        **
***************************/

static Lhs_Var_Desc lvar_create_var_desc (Mol_Symbol var_name)
{
	Lhs_Var_Desc desc;

	assert (SA_lhs_var_table != NULL);

	desc = (Lhs_Var_Desc) 
		rul__mem_malloc (sizeof (struct lhs_var_desc));
	assert (desc != NULL);

	desc->first_bind_site = NULL;

	desc->variable_name = var_name;
	if (var_name != NULL) rul__mol_incr_uses (var_name);

	desc->unique_index = SL_lhs_var_index++;

	rul__hash_add_entry (SA_lhs_var_table, var_name, desc);

	desc->domain		= dom_invalid;
	desc->shape		= shape_invalid;
	desc->is_a		= NULL;
	desc->rhs_domain	= dom_invalid;
	desc->rhs_shape		= shape_invalid;
	desc->rhs_is_a		= NULL;

#ifndef NDEBUG
	desc->verification = LVAR_DESC_VERIFY;
#endif
	return (desc);
}



/*******************************
**                            **
**  RUL__LVAR_CREATE_UNNAMED  **
**                            **
*******************************/

long	rul__lvar_create_unnamed (void)
{
	long tmp = SL_lhs_var_index;

	SL_lhs_var_index++;
	return (tmp);
}



/****************************
**                         **
**  LVAR_CREATE_BIND_SITE  **
**                         **
****************************/

static Binding_Site lvar_create_bind_site (
				Class ce_class, long ce_index, 
				Mol_Symbol attr_name, Value attr_elem)
{
	Binding_Site site;

	site = (Binding_Site) rul__mem_malloc (sizeof (struct binding_site));
	assert (site != NULL);

	site->ce_index = ce_index;
	site->attr_name = attr_name;
	rul__mol_incr_uses (attr_name);
	site->attr_offset = rul__decl_get_attr_offset (ce_class, attr_name);
	site->attr_elem_index = attr_elem;
	site->next_bind_site = NULL;
	site->attr_elem_expr_ast = NULL;

	site->is_length_bind = FALSE;
	site->is_last_elem_bind = FALSE;

	site->node = NULL;  /*  set later, in sem_build phase  */

#ifndef NDEBUG
	site->verification = LVAR_BIND_VERIFY;
#endif
#ifdef DEBUG_LVAR
	rul__ios_printf (RUL__C_STD_ERR,
			 "\n**  LVAR_CREATE_BIND_SITE  ** CE: %d  attr: ",
			 ce_index);
	rul__mol_print_printform (attr_name, RUL__C_STD_ERR);
#endif
	return (site);
}



/****************
**             **
**  LVAR_INIT  **
**             **
****************/

static void lvar_init (void)
{
	/*  Create the new table  */
	if (SA_lhs_var_table == NULL) {
	    SA_lhs_var_table = rul__hash_create_table (LVAR_HT_SIZE);
	}
}




/****************************
**                         **
**  RUL__LVAR_CLEAR_BLOCK  **
**                         **
****************************/

void rul__lvar_clear_block (void)
	/*  Called at end of each rule or entry block  */
{
	rul__lvar_clear_rule ();

	SL_lhs_var_index = 1;
}



/***************************
**                        **
**  RUL__LVAR_CLEAR_RULE  **
**                        **
***************************/

void rul__lvar_clear_rule (void)
	/*  Called before semantic checks start for each rule  */
{
	/*  Free all the old table entries  */

	if (SA_lhs_var_table) {
	    rul__hash_for_each_entry (SA_lhs_var_table, 
				      lvar_free_var_desc);
	    rul__hash_delete_table (SA_lhs_var_table);
	    SA_lhs_var_table = NULL;
	    if (SA_lhs_ce_signs)
	      rul__dyar_set_array_empty (SA_lhs_ce_signs);
	}
}




/*****************************
**                          **
**  RUL__LVAR_SAVE_CE_TYPE  **
**                          **
*****************************/

void rul__lvar_save_ce_type (long ce_index, Condition_Element_Type type)
{
	assert (ce_index > 0);
	assert (type == LVAR__E_POSITIVE	||
		type == LVAR__E_NEGATIVE	||
		type == LVAR__E_START_DISJ_POS	||
		type == LVAR__E_START_DISJ_NEG	||
		type == LVAR__E_DISJ_MEMBER_POS	||
		type == LVAR__E_DISJ_MEMBER_NEG);

	if (SA_lhs_ce_signs == NULL) {
	    SA_lhs_ce_signs = rul__dyar_create_array (MAX(20,ce_index));
	}
	rul__dyar_set_nth (SA_lhs_ce_signs, ce_index, (void *) type);
}




/***********************
**                    **
**  LVAR_GET_CE_TYPE  **
**                    **
***********************/

static Condition_Element_Type lvar_get_ce_type (long ce_index)
{
	assert (ce_index > 0);
	assert (SA_lhs_ce_signs != NULL);
	assert (rul__dyar_get_length (SA_lhs_ce_signs) >= ce_index);

	return ((Condition_Element_Type) 
			rul__dyar_get_nth (SA_lhs_ce_signs, ce_index));
}




/*********************************
**                              **
**  LVAR_CE_INDEX_TO_CSE_INDEX  **
**                              **
*********************************/

static long lvar_ce_index_to_cse_index (long ce_index)
{
	long i, cse_index;
	Condition_Element_Type type;

	cse_index = 0;
	for (i=1; i<=ce_index; i++) {
	    type = (Condition_Element_Type)
			rul__dyar_get_nth (SA_lhs_ce_signs, i);
	    if (type == LVAR__E_POSITIVE) cse_index++;
	    /*?*  what about ce disjunctions!  */
	}
	assert (cse_index > 0);

	return (cse_index);
}




/**********************************
**                               **
**  RUL__LVAR_VARIABLE_IS_KNOWN  **
**                               **
**********************************/

Boolean rul__lvar_variable_is_known (Molecule var_name)
{
	Lhs_Var_Desc desc;

	if (SA_lhs_var_table == NULL) lvar_init();

	desc = rul__hash_get_entry (SA_lhs_var_table, var_name);
	if (desc) {
	    return (TRUE);
	}
	return (FALSE);
}



/************************************
**                                 **
**  RUL__LVAR_VARIABLE_IS_VISIBLE  **
**                                 **
************************************/

Boolean rul__lvar_variable_is_visible (Molecule var_name, long in_ce_number)
{
	Lhs_Var_Desc desc;
	Condition_Element_Type type;
	long variables_ce;
	Binding_Site site;

	if (SA_lhs_var_table == NULL) lvar_init();

	/*?*  Shouldn't be this simple:  ignores ce disjunctions */

	desc = rul__hash_get_entry (SA_lhs_var_table, var_name);
	if (desc) {
	    site = desc->first_bind_site;
	    while (site) {
		variables_ce = site->ce_index;

		/*  Check for if this binding site is same CE  */
		if (variables_ce == in_ce_number) return (TRUE);

		/*  Check if this binding site is visible  */
		type = lvar_get_ce_type (variables_ce);
		if (type == LVAR__E_POSITIVE) return (TRUE);

		/*  If this site was not visible, try the next site  */
		site = site->next_bind_site;
	    }
	}
	return (FALSE);
}




/**********************************
**                               **
**  RUL__LVAR_VARIABLE_IS_LOCAL  **
**                               **
**********************************/

Boolean rul__lvar_variable_is_local (Molecule var_name, long cur_ce_number)
	/*
	**  Determine whether this variable has a binding 
	**  site in this condition element
	*/
{
	Lhs_Var_Desc desc;
	Binding_Site site;

	assert (SA_lhs_var_table != NULL);

	desc = rul__hash_get_entry (SA_lhs_var_table, var_name);
	assert (desc != NULL);

	site = desc->first_bind_site;
	while (site) {
	    assert (lvar_bind_site_is_valid (site));
	    if (site->ce_index == cur_ce_number) {

		if (rul__net_node_is_1_input (site->node)) {
		    /*
		    **  Normally, if the variable was bound in the
		    **  same CE where it is being used, it is a local
		    **  reference.
		    */
	            return (TRUE);

		} else {
		    /*
		    **  Whenever the binding site is a 2-input node
		    **  (for example:  ^list[<var-from-prev-ce>] <new-var> )
		    **  then this variable forces all references to it
		    **  to be 2-input tests, even when they occur within
		    **  the same CE where the variable is bound.
		    */
		    return (FALSE);
		}
	    }
	    site = site->next_bind_site;
	}
	return (FALSE);
}




/**********************************
**                               **
**  RUL__LVAR_GET_BINDING_COUNT  **
**                               **
**********************************/

long rul__lvar_get_binding_count (Molecule var_name)
{
	Lhs_Var_Desc desc;
	Binding_Site site;
	Condition_Element_Type type;
	long i;

	if (SA_lhs_var_table == NULL) lvar_init();

	desc = rul__hash_get_entry (SA_lhs_var_table, var_name);
	if (desc) {
	    i = 0;
	    site = desc->first_bind_site;
	    while (site) {
		assert (lvar_bind_site_is_valid (site));
		
		/*  Check if this binding site is visible  */
		type = lvar_get_ce_type (site->ce_index);
		if (type == LVAR__E_POSITIVE) i++;
		site = site->next_bind_site;
	    }
	    if (i == 0) return (-1);	/*  Bound only in negated ce's  */
	    return (i);			/*?* not handling ce disjunctions */
	}
	return (0);			/*  No bindings found  */
}




/******************************
**                           **
**  RUL__LVAR_GET_ID_NUMBER  **
**                           **
******************************/

long rul__lvar_get_id_number (Molecule var_name, long ce_index)
	/*?*  will we really need the ce_index for ce disjunctions ?  */
{
	Lhs_Var_Desc desc;

	assert (SA_lhs_var_table != NULL);

	desc = rul__hash_get_entry (SA_lhs_var_table, var_name);
	assert (desc != NULL);

	if (desc != NULL) return (desc->unique_index);

	return (LVAR__C_INVALID_VAR_NUMBER);
}





/**************************
**                       **
**  RUL__LVAR_GET_SHAPE  **
**                       **
**************************/

Decl_Shape rul__lvar_get_shape (Molecule var_name)
{
	Lhs_Var_Desc desc;

	if (SA_lhs_var_table == NULL) lvar_init();

	desc = rul__hash_get_entry (SA_lhs_var_table, var_name);
	assert (desc != NULL);

	return (desc->shape);
}



/***************************
**                        **
**  RUL__LVAR_GET_DOMAIN  **
**                        **
***************************/

Decl_Domain rul__lvar_get_domain (Molecule var_name)
{
	Lhs_Var_Desc desc;

	if (SA_lhs_var_table == NULL) lvar_init();

	desc = rul__hash_get_entry (SA_lhs_var_table, var_name);
	assert (desc != NULL);

	return (desc->domain);
}



/**************************
**                       **
**  RUL__LVAR_GET_CLASS  **
**                       **
**************************/

Class rul__lvar_get_class (Molecule var_name)
{
	Lhs_Var_Desc desc;

	if (SA_lhs_var_table == NULL) lvar_init();

	desc = rul__hash_get_entry (SA_lhs_var_table, var_name);
	assert (desc != NULL);

	return (desc->is_a);
}



/******************************
**                           **
**  RUL__LVAR_GET_RHS_SHAPE  **
**                           **
******************************/

Decl_Shape rul__lvar_get_rhs_shape (Molecule var_name)
{
	Lhs_Var_Desc desc;

	if (SA_lhs_var_table == NULL) lvar_init();

	desc = rul__hash_get_entry (SA_lhs_var_table, var_name);
	assert (desc != NULL);
	assert (desc->rhs_shape != shape_invalid);

	return (desc->rhs_shape);
}



/*******************************
**                            **
**  RUL__LVAR_GET_RHS_DOMAIN  **
**                            **
*******************************/

Decl_Domain rul__lvar_get_rhs_domain (Molecule var_name)
{
	Lhs_Var_Desc desc;

	if (SA_lhs_var_table == NULL) lvar_init();

	desc = rul__hash_get_entry (SA_lhs_var_table, var_name);
	assert (desc != NULL);
	assert (desc->rhs_domain != dom_invalid);

	return (desc->rhs_domain);
}



/******************************
**                           **
**  RUL__LVAR_GET_RHS_CLASS  **
**                           **
******************************/

Class rul__lvar_get_rhs_class (Molecule var_name)
{
	Lhs_Var_Desc desc;

	if (SA_lhs_var_table == NULL) lvar_init();

	desc = rul__hash_get_entry (SA_lhs_var_table, var_name);
	assert (desc != NULL);

	return (desc->rhs_is_a);
}





/*********************************
**                              **
**  RUL__LVAR_CREATE_BIND_DESC  **
**                              **
*********************************/

void rul__lvar_create_bind_desc (Mol_Symbol var_name, 
	Ast_Node ast, Class ce_class, long ce_index, Mol_Symbol attr_name, 
	Value attr_elem, Ast_Node attr_elem_ast)
{
	Lhs_Var_Desc desc;
 	Binding_Site site;
	Decl_Domain new_dom;
	Decl_Shape  new_shape;
	Class new_class;
	Condition_Element_Type ce_type;	

	if (SA_lhs_var_table == NULL) lvar_init();

	/*  Get (or create) the variable descriptor  */
	desc = rul__hash_get_entry (SA_lhs_var_table, var_name);
	if (desc == NULL) desc = lvar_create_var_desc (var_name);
	assert (lvar_desc_is_valid (desc));

	/*  Get the attribute info  */
	if (attr_elem == NULL) {
	    new_shape =  rul__decl_get_attr_shape  (ce_class, attr_name);
	} else {
	    new_shape =  shape_atom;
	}
	new_dom = rul__decl_get_attr_domain (ce_class, attr_name);
	new_class = rul__decl_get_attr_class (ce_class, attr_name);

	/*  Create the new Binding_Site  */
	site = lvar_create_bind_site (ce_class, ce_index, attr_name, attr_elem);
	site->attr_elem_expr_ast = attr_elem_ast;

	if (desc->first_bind_site == NULL) {
	    /*
	    **  If this is the first site, just set the shape and domain
	    */
	    desc->shape =      new_shape;
	    desc->domain =     new_dom;
	    desc->is_a =       new_class;

	    ce_type = lvar_get_ce_type (ce_index);
	    if (ce_type == LVAR__E_POSITIVE  ||
		ce_type == LVAR__E_START_DISJ_POS) {
		lvar_set_rhs_type_from_lhs (desc);
	    }
	    desc->first_bind_site = site;

	} else {
	    /*
	    **  If this is not the first site, check the shape and domain
	    **  against the previous binding sites.
	    */
	    lvar_add_subsequent_bind_site (
			desc, site, new_dom, new_shape, new_class, ast);
	}

#ifdef DEBUG_LVAR
	rul__ios_printf (RUL__C_STD_ERR,
		 "\n**  RUL__LVAR_CREATE_BIND_DESC  ** CE: %d  attr: ",
			 ce_index);
	rul__mol_print_printform (attr_name, RUL__C_STD_ERR);
	rul__ios_printf (RUL__C_STD_ERR, "  Var: ");
	rul__mol_print_printform (var_name, RUL__C_STD_ERR);
	rul__ios_printf (RUL__C_STD_ERR, "  ID: %d", desc->unique_index);
#endif

}



/*************************************
**                                  **
**  RUL__LVAR_CREATE_LEN_BIND_DESC  **
**                                  **
*************************************/

void rul__lvar_create_len_bind_desc (Mol_Symbol var_name, 
	Ast_Node ast, Class ce_class, long ce_index, Mol_Symbol attr_name)
{
	Lhs_Var_Desc desc;
 	Binding_Site site;
	Decl_Domain new_dom, old_dom;

	if (SA_lhs_var_table == NULL) lvar_init();

	/*  Get (or create) the variable descriptor  */
	desc = rul__hash_get_entry (SA_lhs_var_table, var_name);
	if (desc == NULL) desc = lvar_create_var_desc (var_name);
	assert (lvar_desc_is_valid (desc));

	/*  Create the new Binding_Site  */
	site = lvar_create_bind_site (ce_class, ce_index, attr_name, NULL);
	site->is_length_bind = TRUE;

	if (desc->first_bind_site == NULL) {
	    /*
	    **  If this is the first site, set the shape and domain
	    */
	    desc->shape =  shape_atom;
	    desc->domain = dom_int_atom;
	    desc->is_a =   NULL;

	    lvar_set_rhs_type_from_lhs (desc);
	    desc->first_bind_site = site;

	} else {
	    /*
	    **  If this is not the first site, check the shape and domain
	    **  against the previous binding sites.
	    */
	    lvar_add_subsequent_bind_site (
			desc, site, dom_int_atom, shape_atom, NULL, ast);
	}
#ifdef DEBUG_LVAR
	rul__ios_printf (RUL__C_STD_ERR,
		 "\n**  RUL__LVAR_CREATE_LEN_BIND_DESC  ** CE: %d  attr: ",
			 ce_index);
	rul__mol_print_printform (attr_name, RUL__C_STD_ERR);
	rul__ios_printf (RUL__C_STD_ERR, "  Var: ");
	rul__mol_print_printform (var_name, RUL__C_STD_ERR);
	rul__ios_printf (RUL__C_STD_ERR, "  ID: %d", desc->unique_index);
#endif
}




/**************************************
**                                   **
**  RUL__LVAR_CREATE_LAST_BIND_DESC  **
**                                   **
**************************************/

void rul__lvar_create_last_bind_desc (Mol_Symbol var_name, 
	Ast_Node ast, Class ce_class, long ce_index, Mol_Symbol attr_name)
{
	Lhs_Var_Desc desc;
 	Binding_Site site;
	Decl_Domain new_dom;
	Class new_class;

	if (SA_lhs_var_table == NULL) lvar_init();

	/*  Get (or create) the variable descriptor  */
	desc = rul__hash_get_entry (SA_lhs_var_table, var_name);
	if (desc == NULL) desc = lvar_create_var_desc (var_name);
	assert (lvar_desc_is_valid (desc));

	new_dom = rul__decl_get_attr_domain (ce_class, attr_name);
	new_class = rul__decl_get_attr_class (ce_class, attr_name);

	/*  Create the new Binding_Site  */
	site = lvar_create_bind_site (ce_class, ce_index, attr_name, NULL);
	site->is_last_elem_bind = TRUE;

	if (desc->first_bind_site == NULL) {
	    /*
	    **  If this is the first site, set the shape and domain
	    */
	    desc->shape =  shape_atom;
	    desc->domain = new_dom;
	    desc->is_a =   new_class;

	    lvar_set_rhs_type_from_lhs (desc);
	    desc->first_bind_site = site;

	} else {
	    /*
	    **  If this is not the first site, check the shape and domain
	    **  against the previous binding sites.
	    */
	    lvar_add_subsequent_bind_site (
			desc, site, new_dom, shape_atom, new_class, ast);
	}
#ifdef DEBUG_LVAR
	rul__ios_printf (RUL__C_STD_ERR,
		 "\n**  RUL__LVAR_CREATE_LAST_BIND_DESC  ** CE: %d  attr: ",
			 ce_index);
	rul__mol_print_printform (attr_name, RUL__C_STD_ERR);
	rul__ios_printf (RUL__C_STD_ERR, "  Var: ");
	rul__mol_print_printform (var_name, RUL__C_STD_ERR);
	rul__ios_printf (RUL__C_STD_ERR, "  ID: %d", desc->unique_index);
#endif
}




/************************************
**                                 **
**  LVAR_ADD_SUBSEQUENT_BIND_SITE  **
**                                 **
************************************/

static void lvar_add_subsequent_bind_site (Lhs_Var_Desc desc, 
		Binding_Site site, Decl_Domain new_dom, 
		Decl_Shape new_shape, Class new_class, Ast_Node ast)
{
	Decl_Domain old_dom;
	Condition_Element_Type ce_type;

	assert (lvar_bind_site_is_valid (site));
	assert (lvar_bind_site_is_valid (desc->first_bind_site));

	/*
	**  Since this is not the first site, if the previous bind site 
	**  was a negated condition, just use the new domain and shape.
	**
	**  Once we have CE disjunctions, then adjust the domain
	**  and shape depending on the union of the current and the previous
	**  domains.
	*/

	if (desc->rhs_domain == dom_invalid) {
	    /*
	    **  This will be true if all previous binding sites
	    **  have been in negated conditions.  Print a message
	    **  saying this is a dubious thing to be doing.
	    */
	    if (desc->shape != new_shape) {
		rul__msg_cmp_print_w_atoms (CMP_VARDIFTYP, ast, 1,
					desc->variable_name);
	    } else {
	        rul__msg_cmp_print_w_atoms (CMP_VARMULBIND, ast, 1, 
					desc->variable_name);
	    }

	    ce_type = lvar_get_ce_type (site->ce_index);
	    if (ce_type == LVAR__E_POSITIVE  || 
		ce_type == LVAR__E_START_DISJ_POS) {
		/*
		**  For bindings in positive CE's, set the RHS domain and shape
		*/
		desc->rhs_domain  = new_dom;
		desc->rhs_shape   = new_shape;
		desc->rhs_is_a 	  = new_class;
	    }

	} else {

	    assert (FALSE);  /*?* should never happen until we do CE disj's  */
	    /*
	    **  For variables bound within the arms of a CE disjunction,
	    **  the RHS shape and domain will be the lowest common shape
	    **  and domain of each of the binding sites.
	    */

	    /*  Update the shape  */
            if (desc->shape != new_shape) {
	        rul__msg_cmp_print_w_atoms (CMP_VARDIFTYP, ast, 1,
					desc->variable_name);
	        desc->shape = shape_molecule;
	    }

	    /*  Update the domain  */
	    desc->domain = rul__mol_domain_union (desc->domain, new_dom);

	    /*  Update the class  */
	    if (desc->domain != dom_instance_id) {
	        desc->is_a = NULL;
	    } else if (desc->is_a != new_class) {
	        desc->is_a = rul__decl_lowest_common_class (
					desc->is_a, new_class);
	    }
	    lvar_set_rhs_type_from_lhs (desc);
	}

	site->next_bind_site = desc->first_bind_site;
	desc->first_bind_site = site;
}




/*************************************
**                                  **
**  RUL__LVAR_FOUND_EQ_TEST_ON_VAR  **
**                                  **
*************************************/

void	rul__lvar_found_eq_test_on_var (Mol_Symbol var_name,
			Ast_Node ast, Class ce_class, long ce_index,
			Mol_Symbol attr_name, Ast_Node attr_index_ast)
{
	Lhs_Var_Desc desc;
	Decl_Shape new_shape;
	Decl_Domain new_dom, low_domain;
	Class new_class;
	/*
	**  An EQ-test directly on a variable can affect
	**  the type/shape of that variable for the RHS code
	*/

	if (lvar_get_ce_type (ce_index) != LVAR__E_POSITIVE) return;

	/*  Get the variable descriptor  */
	assert (SA_lhs_var_table != NULL);
	desc = rul__hash_get_entry (SA_lhs_var_table, var_name);
	assert (desc != NULL);
	assert (lvar_desc_is_valid (desc));
	assert (desc->rhs_domain != dom_invalid);
	assert (desc->rhs_shape != shape_invalid);


	/*  Get all the attribute info  */
	if (attr_index_ast == NULL) {
	    new_shape =  rul__decl_get_attr_shape  (ce_class, attr_name);
	} else {
	    new_shape =  shape_atom;
	}
	new_dom = rul__decl_get_attr_domain (ce_class, attr_name);
	new_class = rul__decl_get_attr_class (ce_class, attr_name);

	if (new_shape != desc->rhs_shape) {
	    if (new_shape != shape_molecule  &&  
		desc->rhs_shape == shape_molecule) {

		desc->rhs_shape = new_shape;
	    }
	}

	if (new_dom != desc->rhs_domain) {
	    low_domain = lvar_more_specific_domain (new_dom, desc->rhs_domain);
	    if (low_domain != dom_invalid) {

		desc->rhs_domain = low_domain;

	    }
	}

	if (desc->rhs_domain == dom_instance_id) {
	    if (new_class != NULL  &&  new_class != desc->rhs_is_a) {
		if (desc->rhs_is_a == NULL ||
		    rul__decl_is_subclass_of (new_class, desc->rhs_is_a)) {

		    /*  refine rhs class knowledge...  */
		    desc->rhs_is_a = new_class;

		} else if (desc->rhs_is_a != NULL  &&  new_class != NULL  &&
			 !rul__decl_is_subclass_of(desc->rhs_is_a, new_class)){

		    /*  signal warning  */
		    rul__msg_cmp_print_w_atoms (CMP_MATCHNEVER, ast, 2,
					        attr_name, var_name);
		}
	    }
	}
}




/*********************************
**                              **
**  RUL__LVAR_SET_BINDING_NODE  **
**                              **
*********************************/

void rul__lvar_set_binding_node (Mol_Symbol var_name, 
	long ce_index, Net_Node node)
{
	Lhs_Var_Desc desc;
	Binding_Site site;

	assert (SA_lhs_var_table != NULL);

	desc = rul__hash_get_entry (SA_lhs_var_table, var_name);
	assert (desc != NULL);

	site = desc->first_bind_site;
	while (site) {
	    assert (lvar_bind_site_is_valid (site));
	    if (site->ce_index == ce_index) {
		site->node = node;
		return;
	    }
	    site = site->next_bind_site;
	}
}





/********************************
**                             **
**  RUL__LVAR_BUILD_RHS_VALUE  **
**                             **
********************************/

Value rul__lvar_build_rhs_value (Mol_Symbol var_name)
{
	Lhs_Var_Desc desc;
	Binding_Site site, tmp;
	Value	final_val = NULL;
	Value	cse_wm_val, wm_attr_val, get_nth_val, index_val;
	
	assert (SA_lhs_var_table != NULL);

	desc = rul__hash_get_entry (SA_lhs_var_table, var_name);
	assert (desc != NULL);

	/*
	**  Find the positive-CE that was the binding site visible to the RHS.
	**  As long as CE disjunctions are NYI, there can be only one such.
	*/
	site = desc->first_bind_site;
	assert (lvar_bind_site_is_valid (site));
	while (lvar_get_ce_type (site->ce_index) != LVAR__E_POSITIVE) {
	    site = site->next_bind_site;
	    assert (lvar_bind_site_is_valid (site));
	}

#ifndef NDEBUG
	/*?* while CE-disj NYI, this should be the only positive-CE binding */
	tmp = site->next_bind_site;
	while (tmp != NULL) {
	    assert (lvar_get_ce_type (tmp->ce_index) != LVAR__E_POSITIVE);
	    tmp = tmp->next_bind_site;
	}
#endif


	/*
	**  Now create the RHS value access expression
	*/
#ifdef NDEBUG
	cse_wm_val = rul__val_create_function_str (
			       "CGCNO", 2);
#else
	cse_wm_val = rul__val_create_function_str (
			       "rul__cs_get_cse_nth_object", 2);
#endif
	rul__val_set_nth_subvalue (
		    cse_wm_val, 1,
		    rul__val_create_blk_or_loc (
			CMP__E_INT_CS_ENTRY, GEN__C_CS_ENTRY_PTR_STR));
	rul__val_set_nth_subvalue (
		    cse_wm_val, 2, 
		    rul__val_create_long_animal (
			lvar_ce_index_to_cse_index (site->ce_index)));
	wm_attr_val = rul__val_create_function_str (
#ifndef NDEBUG
			       "rul__wm_get_offset_val", 2);
#else
			       "WGOV", 2);
#endif
	rul__val_set_nth_subvalue (wm_attr_val, 1, cse_wm_val);
	rul__val_set_nth_subvalue (
		    wm_attr_val, 2, 
		    rul__val_create_long_animal (site->attr_offset));

	if (site->is_length_bind) {
	    /*  Compound attribute length bind  */
	    final_val = rul__val_create_function_str (
#ifndef NDEBUG
				"rul__mol_get_poly_count_atom", 1);
#else
				"MGPCA", 1);
#endif
	    rul__val_set_nth_subvalue (final_val, 1, wm_attr_val);
	    rul__val_set_shape_and_domain (final_val,
				shape_atom, dom_int_atom);

	} else if (site->is_last_elem_bind) {
	    /*  Compound attribute last element bind  */
	    final_val = rul__val_create_function_str (
#ifndef NDEBUG
				"rul__mol_get_comp_last", 1);
#else
				"MGCL", 1);
#endif
	    rul__val_set_nth_subvalue (final_val, 1, wm_attr_val);
	    rul__val_set_shape_and_domain (final_val,
				shape_atom, desc->rhs_domain);

	} else if (site->attr_elem_index != NULL) {
	    /*  Compound attribute element bind  */
	    index_val = rul__sem_check_rhs_value (
				site->attr_elem_expr_ast, 
				TRUE, dom_int_atom, shape_atom, NULL);
	    final_val = rul__val_create_function_str (
#ifndef NDEBUG
				"rul__mol_get_comp_nth_mol", 2);
#else
				"MGCNM", 2);
#endif
	    rul__val_set_nth_subvalue (final_val, 1, wm_attr_val);
	    rul__val_set_nth_subvalue (final_val, 2, index_val);
	    rul__val_set_shape_and_domain (final_val, 
				shape_atom, desc->rhs_domain);
	    if (desc->rhs_is_a != NULL) {
		rul__val_set_class (final_val, desc->rhs_is_a);
	    }

	} else {
	    /*  Simple attribute bind  */
	    final_val = wm_attr_val;
	    rul__val_set_shape_and_domain (final_val, 
				desc->rhs_shape, desc->rhs_domain);
	    if (desc->rhs_is_a != NULL) {
		rul__val_set_class (final_val, desc->rhs_is_a);
	    }
	}
	assert (final_val != NULL);
	return (final_val);
}



/*************************************
**                                  **
**  RUL__LVAR_FIND_NESTED_LHS_VARS  **
**                                  **
*************************************/

void rul__lvar_find_nested_lhs_vars (Mol_Symbol var_name)
{
	Lhs_Var_Desc desc;
	Binding_Site site, tmp;
	
	assert (SA_lhs_var_table != NULL);

	desc = rul__hash_get_entry (SA_lhs_var_table, var_name);
	assert (desc != NULL);

	/*
	**  Find the positive-CE that was the binding site visible to the RHS.
	**  As long as CE disjunctions are NYI, there can be only one such.
	*/
	site = desc->first_bind_site;
	assert (lvar_bind_site_is_valid (site));
	while (lvar_get_ce_type (site->ce_index) != LVAR__E_POSITIVE) {
	    site = site->next_bind_site;
	    assert (lvar_bind_site_is_valid (site));
	}

#ifndef NDEBUG
	/*?* while CE-disj NYI, this should be the only positive-CE binding */
	tmp = site->next_bind_site;
	while (tmp != NULL) {
	    assert (lvar_get_ce_type (tmp->ce_index) != LVAR__E_POSITIVE);
	    tmp = tmp->next_bind_site;
	}
#endif

	/*
	**  Attribute element expressions are the only place where
	**  a nested reference to another LHS variable could occur.
	**  If this is a compound element binding site, then
	**  call the routine to check if this attribute element index
	**  expression contains any references to other LHS variables.
	*/

	if (site->attr_elem_index != NULL) {
	    lvar_tag_each_nested_lvar (site->attr_elem_expr_ast);
	}
}


static void  lvar_tag_each_nested_lvar (Ast_Node cur_ast_node)
{
	Mol_Symbol var_name;
	Ast_Node tmp;

	if (rul__ast_get_type (cur_ast_node) == AST__E_VARIABLE) {
	    var_name = (Mol_Symbol) rul__ast_get_value (cur_ast_node);
	    if (rul__lvar_get_binding_count (var_name)) {
		rul__sem_tag_rhs_use_of_lhs_var (var_name);
	    }
	}

	/* depth first traversal */
	tmp = rul__ast_get_child (cur_ast_node);
	if (tmp != NULL) lvar_tag_each_nested_lvar (tmp);

	tmp = rul__ast_get_sibling (cur_ast_node);
	if (tmp != NULL) lvar_tag_each_nested_lvar (tmp);
}





/***************************
**                        **
**  LVAR_GET_NTH_BINDING  **
**                        **
***************************/

static Binding_Site lvar_get_nth_binding (Mol_Symbol var_name, long n)
{
	Lhs_Var_Desc desc;
	Binding_Site site;
	long i;
	
	assert (SA_lhs_var_table != NULL);

	desc = rul__hash_get_entry (SA_lhs_var_table, var_name);
	assert (desc != NULL);

	i = 0;
	site = desc->first_bind_site;
	while (site) {
	    assert (lvar_bind_site_is_valid (site));
	    i++;
	    if (i == n) {
	        return (site);
	    }
	    site = site->next_bind_site;
	}
	return (NULL);
}




/*************************
**                      **
**  LVAR_FREE_VAR_DESC  **
**                      **
*************************/

static void lvar_free_var_desc (void *var_desc)
{
	Lhs_Var_Desc desc;
	Binding_Site bind, tmp_bind;

	desc = (Lhs_Var_Desc) var_desc;
	assert (lvar_desc_is_valid (desc));

	rul__hash_replace_entry (SA_lhs_var_table, desc->variable_name, NULL);

	bind = desc->first_bind_site;
	while (bind) {
	    assert (lvar_bind_site_is_valid (bind));
	    rul__mol_decr_uses(bind->attr_name);
	    if (bind->attr_elem_index != NULL)
	      rul__val_free_value(bind->attr_elem_index);
	    tmp_bind = bind;
	    bind = bind->next_bind_site;
	    rul__mem_free (tmp_bind);
	}

	if (desc->variable_name != NULL) {
		rul__mol_decr_uses (desc->variable_name);
	}
#ifndef NDEBUG
	desc->verification = 0;
#endif

	rul__mem_free (desc);
}


/*
**	These statics are used only in the following two routines
*/
static long SA_target_ce_index;
static long SA_target_attr_offset;
static Value SA_target_attr_index_val;
#define INVALID_ELEM_INDEX -1

static long SA_found_lhs_var_id;
static Molecule SM_found_lhs_var_name;

/*************************
**                      **
**  LVAR_IS_BOUND_POSN  **
**                      **
*************************/

static void lvar_is_bound_posn (void *lvar_desc)
{
	Lhs_Var_Desc desc;
	Binding_Site site;

	if (SA_found_lhs_var_id != LVAR__C_INVALID_VAR_NUMBER) return;

	desc = (Lhs_Var_Desc) lvar_desc;
	site = desc->first_bind_site;

	while (site != NULL) {
	    if (site->ce_index    == SA_target_ce_index  	&&
		site->attr_offset == SA_target_attr_offset	&&
		! site->is_length_bind  			&&
		! site->is_last_elem_bind) {

		if (SA_target_attr_index_val == NULL  &&
		    site->attr_elem_index == NULL) {

		    /*  Found matching complete attribute bind!  */
		    SA_found_lhs_var_id = desc->unique_index;
		    return;

		} else if (rul__val_equivalent (SA_target_attr_index_val,
						site->attr_elem_index)) {

		    /*  Found matching attribute element bind!  */
		    SA_found_lhs_var_id = desc->unique_index;
		    return;
		}
	    }
	    site = site->next_bind_site;
	}
}


/***********************
**                    **
**  LVAR_MATCH_INDEX  **
**                    **
***********************/

static void lvar_match_index (void *lvar_desc)
{
	Lhs_Var_Desc desc;

	if (SM_found_lhs_var_name != NULL) return;

	desc = (Lhs_Var_Desc) lvar_desc;
	if (SA_found_lhs_var_id == desc->unique_index)
	   SM_found_lhs_var_name = desc->variable_name;
	return;
}



/*************************
**                      **
**  RUL__LVAR_GET_NAME  **
**                      **
*************************/

Mol_Symbol rul__lvar_get_name (long unique_index)
{
        SM_found_lhs_var_name = NULL;
	SA_found_lhs_var_id = unique_index;
	/*  Now search the variable table  */

	if (SA_lhs_var_table) {
	    /*  ... rather inefficient, but does the job...  */
	    rul__hash_for_each_entry (SA_lhs_var_table, 
				      lvar_match_index);
	}
	return (SM_found_lhs_var_name);
}






/***********************************
**                                **
**  RUL__LVAR_GET_POS_VAR_NUMBER  **
**                                **
***********************************/

long rul__lvar_get_pos_var_number (long ce_index, Value pos_val)
{
	Value element_index;
	assert (rul__val_is_position (pos_val));

	/*  Set the parameters for the search  */

	SA_found_lhs_var_id = LVAR__C_INVALID_VAR_NUMBER;
	SA_target_ce_index = ce_index;
	SA_target_attr_offset = rul__val_get_pos_attr_offset (pos_val);

	if (rul__val_is_position_elem (pos_val)) {
	    SA_target_attr_index_val = rul__val_get_pos_elem_index (pos_val);
	} else {
	    SA_target_attr_index_val = NULL;
	}

	/*  Now search the variable table  */

	if (SA_lhs_var_table) {
	    /*  ... rather inefficient, but does the job...  */
	    rul__hash_for_each_entry (SA_lhs_var_table, 
				      lvar_is_bound_posn);
	}
	return (SA_found_lhs_var_id);
}




/*********************************
**                              **
**  LVAR_SET_RHS_TYPE_FROM_LHS  **
**                              **
*********************************/

void lvar_set_rhs_type_from_lhs (Lhs_Var_Desc desc)
{
	assert (lvar_desc_is_valid (desc));

	desc->rhs_shape =   desc->shape;
	desc->rhs_domain =  desc->domain;
	desc->rhs_is_a =    desc->is_a;
}



/********************************
**                             **
**  LVAR_MORE_SPECIFIC_DOMAIN  **
**                             **
********************************/

Decl_Domain lvar_more_specific_domain (Decl_Domain dom_1, Decl_Domain dom_2)
{
	Decl_Domain dom_out;

	assert (dom_1 != dom_invalid);
	assert (dom_1 != dom_invalid);

	if (dom_1 != dom_2) {
	    if (dom_1 == dom_any) {
		dom_out = dom_2;
	    } else if (dom_2 == dom_any) {
		dom_out = dom_1;
	    } else if (dom_1 == dom_number  &&
			(dom_2 == dom_int_atom || dom_2 == dom_dbl_atom)) {
		dom_out = dom_2;
	    } else if (dom_2 == dom_number  &&
			(dom_1 == dom_int_atom || dom_1 == dom_dbl_atom)) {
		dom_out = dom_1;
	    } else {
		dom_out = dom_invalid;
	    }
	} else {
	    dom_out = dom_1;
	}
	return (dom_out);
}
