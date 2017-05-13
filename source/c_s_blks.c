/* cmp_sem_blocks.c - semantic routine for entry/decl/rule blocks */
/****************************************************************************
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
**  FACILITY:
**	RULEWORKS compiler
**
**  ABSTRACT:
**	The entry/decl/rule block semantic routines are passed the AST of
**	the construct.  The routines do semantic checks and set up DECL
**	subsystem data structures.
**
**  MODIFIED BY:
**	DEC	Digital Equipment Corporation
 *	CPQ	Compaq Computer Corporation
**
**  MODIFICATION HISTORY:
**
**	20-Jan-1993	DEC	Initial version
**
**	 3-Mar-1994 DEC <argv> [<argc>] support
**
**	 3-Jun-1994	DEC	Move MAIN -> |main| hack down to just codegen.
**
**	23-Sep-1994	DEC	Avoid illegal characters and collisions in
**					generated argument names.
*
*	01-Dec-1999	CPQ	Release with GPL
*/

#include <ctype.h>
#include <common.h>
#include <cmp_comm.h>
#include <sem.h>		/* Declarations of these functions */
#include <ast.h>
#include <decl.h>
#include <mol.h>
#include <conrg.h>		/* Construct registration */
#include <cons.h>		/* Constant management */
#include <msg.h>
#include <cmp_msg.h>
#include <val.h>
#include <lvar.h>
#include <dyar.h>
#include <ver_msg.h>		/* for RUL__C_MSG_PREFIX */

/*
 * Forward declarations.
 */

static void new_decl_block (Mol_Symbol block_name);

static void process_eb_param (Ast_Node param_node,
			      Mol_Symbol block_name,
			      Ext_Rt_Decl ext_rt,
			      Boolean is_the_return_value,
			      long current_par_index);

static Dynamic_Array ent_var_bindings;

/*****************************************************************************/

Boolean
rul__sem_entry_block (Ast_Node ast)
{
  Ast_Node	  eb_clause, param_node, temp_node,
    		  name_node  = rul__ast_get_child (ast); /* Block name */
  Mol_Symbol	  block_name = rul__ast_get_value (name_node);
  Boolean	  accepts_declared = FALSE, returns_declared = FALSE,
		  activates_declared=FALSE, uses_declared    = FALSE,
		  strategy_declared =FALSE;
  Strategy	  eb_strategy = CS__C_MEA;
  char           *block_printname = rul__mol_get_printform (block_name);
  long            par_index, num_par;
  Ext_Rt_Decl     ext_rt;
  Mol_Symbol      par_sym;

  /* 
   * Check for a valid linkable block name
   */
  if (! rul__sem_check_linkable (block_name)) {
    rul__msg_cmp_print_w_atoms (CMP_INVBLKNAME, name_node, 1, block_name);
    return FALSE;
  }
  
  /*
   * Check to see if there's already a block with this name and
   * complain if so.
   */
  if (rul__conrg_get_block_type (block_name) != RUL__C_NOISE) {
    rul__msg_cmp_print_w_atoms(CMP_DUPBLOCK, name_node, 1, block_name);
    return FALSE;		/* Unsuccessful */
  }

  /* Register the block with the construct registration subsystem. */
  rul__conrg_register_block (block_name,
			     RUL__C_ENTRY_BLOCK,
			     rul__ast_nearest_line_number (name_node));

  /* Declare that this is the current entry block. */
  rul__decl_set_entry_block (block_name);

  /* initialize variables mapped to the entry-block; those parameters
     initialized from the accepts clause and/or used in the on-clauses */
  rul__sem_use_entry_vars (TRUE);
  rul__sem_initialize_rhs_vars ();

  /* Check the entry block clauses for uses (before implicit decl. block) */
  for (eb_clause = rul__ast_get_sibling (name_node);
       eb_clause != NULL;
       eb_clause = rul__ast_get_sibling (eb_clause)) {

    if (rul__ast_get_type(eb_clause) == AST__E_USES)
      if (uses_declared) {
	rul__msg_cmp_print_w_atoms(CMP_EBDUPUSES, eb_clause, 1, block_name);
	temp_node = eb_clause;
	eb_clause = rul__ast_get_previous_sibling (eb_clause);
	rul__ast_delete_node(temp_node);
      }

      else {
	uses_declared = TRUE;
	for (param_node = rul__ast_get_child (eb_clause);
	     param_node != NULL;
	     param_node = rul__ast_get_sibling (param_node)) {
	  if (! rul__sem_check_decl_usefile (
			       rul__ast_get_value (param_node), param_node)) {
	    return FALSE;
	  }
	}
      }
  }

  /* Create the (implicit) declaring block for this entry block. */
  new_decl_block (block_name);

  /* begin to declare an external routine for this entry-block */
  rul__decl_create_ext_rt (block_name);

  /* Set default alias to |main| for main block */
  if (rul__conrg_is_main_block())
      rul__decl_add_ext_rt_alias(rul__mol_make_symbol ("main"));

  ext_rt = rul__decl_get_current_rt ();

  for (eb_clause = rul__ast_get_sibling (name_node);
       eb_clause != NULL;
       eb_clause = rul__ast_get_sibling (eb_clause)) {

    switch (rul__ast_get_type (eb_clause)) {

    case AST__E_USES :
      break;

    case AST__E_ACTIVATES :
      if (activates_declared) {
	rul__msg_cmp_print_w_atoms(CMP_EBDUPACT, eb_clause, 1, block_name);
	temp_node = eb_clause;
	eb_clause = rul__ast_get_previous_sibling (eb_clause);
	rul__ast_delete_node (temp_node);
      }
      else {
	activates_declared = TRUE;
	temp_node = rul__ast_get_child(eb_clause);
	while (temp_node != NULL) {
	  rul__decl_add_rule_block_to_eb (rul__ast_get_value(temp_node));
	  temp_node = rul__ast_get_sibling (temp_node);
	}
      }
      break;
      
    case AST__E_STRATEGY : /* only LEX or MEA - verified by grammar */
      if (strategy_declared) {
	rul__msg_cmp_print_w_atoms (CMP_EBDUPSTR, eb_clause, 1, block_name);
	temp_node = eb_clause;
	eb_clause = rul__ast_get_previous_sibling (eb_clause);
	rul__ast_delete_node(temp_node);
      }
      else {
	strategy_declared = TRUE;
	assert (rul__ast_get_value_type (eb_clause) == AST__E_TYPE_STRATEGY);
	eb_strategy = (Strategy) rul__ast_get_long_value (eb_clause);
      }
      break;

    case AST__E_ACCEPTS_DECL :
      if (accepts_declared) {
	rul__msg_cmp_print_w_atoms(CMP_EBDUPACT, eb_clause, 1, block_name);
	temp_node = eb_clause;
	eb_clause = rul__ast_get_previous_sibling (eb_clause);
	rul__ast_delete_node(temp_node);
      }
      else {
	accepts_declared = TRUE;
	for (param_node = rul__ast_get_child (eb_clause), par_index = 1;
	     param_node != NULL;
	     param_node = rul__ast_get_sibling (param_node), par_index++) {
	  assert (rul__ast_get_type (param_node) == AST__E_ACCEPTS_PARAM);
	  if (!rul__sem_check_ext_rt_param (rul__ast_get_child (param_node),
					    block_name,
					    SEM__C_ROUTINE_ARGUMENT)) {
	    rul__decl_destroy_ext_rt (rul__decl_get_current_rt ());
	    return FALSE;
	  }
	  else 
	    process_eb_param (param_node, block_name, ext_rt,
			      SEM__C_ROUTINE_ARGUMENT, par_index);
	}
      }
      break;

    case AST__E_RETURNS_DECL :
      if (returns_declared) {
	rul__msg_cmp_print_w_atoms(CMP_EBDUPRET, eb_clause, 1, block_name);
	temp_node = eb_clause;
	eb_clause = rul__ast_get_previous_sibling (eb_clause);
	rul__ast_delete_node (temp_node);
      }
      else {
	returns_declared = TRUE;
	if (!rul__sem_check_ext_rt_param (rul__ast_get_child(eb_clause),
					  block_name,
					  SEM__C_RETURN_PARAMETER)) {
	  rul__decl_destroy_ext_rt (rul__decl_get_current_rt ());
	  return FALSE;
	}
	/* entry block return params are handled a little differently
	  else
	  process_eb_param (eb_clause, block_name, ext_rt,
	                    SEM__C_RETURN_PARAMETER, 0);
	 */
      }
      break;
      
    default:
      assert (FALSE);	/* invalid AST - parser shouldn't allow this */
      break;
    }
  }

  if (rul__conrg_is_main_block () && !returns_declared)
    rul__decl_add_ext_rt_ret_val (ext_type_long, 0, ext_mech_value, NULL);
  
  rul__decl_set_eb_strategy (eb_strategy);
  
  /* finish routine decls and save bindings */

  rul__decl_finish_ext_rt ();

  if (ent_var_bindings == NULL)
    ent_var_bindings = rul__dyar_create_array (10);
  else
    rul__dyar_set_array_empty (ent_var_bindings);

  num_par = rul__decl_ext_rt_num_args (ext_rt);
  for (par_index = 1; par_index <= num_par; par_index++) {
    par_sym = rul__decl_ext_rt_param_name (ext_rt, par_index);
    if (par_sym) {
      rul__dyar_append (ent_var_bindings,
			rul__sem_curr_rhs_var_binding (par_sym));
    }
  }

  /* Cause declarations declared in this declaring block to be visible
     to this entry block. */
  rul__decl_make_block_visible (block_name);
  
  return TRUE;		/* Successful */
}

void
rul__sem_set_ent_var_bindings (Ext_Rt_Decl ext_rt, long args)
{
  long       i;
  Mol_Symbol par_sym;
  Value      bind_val;

  for (i = 1; i <= args; i++) {
    par_sym = rul__decl_ext_rt_param_name (ext_rt, i);
    bind_val = (Value) rul__dyar_pop_first (ent_var_bindings);
    if (par_sym != NULL && bind_val != NULL)
      rul__sem_reset_var_binding_site (par_sym, bind_val);
  }
}

/*****************************************************************************/
Boolean
rul__sem_decl_block(Ast_Node ast)
{
  Ast_Node	name_node = rul__ast_get_child(ast); /* Block name */
  Mol_Symbol	block_name = rul__ast_get_value(name_node);

  /* 
   * Check for a valid linkable block name
   */
  if (! rul__sem_check_linkable (block_name)) {
    rul__msg_cmp_print_w_atoms (CMP_INVBLKNAME, name_node, 1, block_name);
    return FALSE;
  }

  /*
   * Check to see if there's already a block with this name and
   * complain if so.
   */
  if (rul__conrg_get_block_type(block_name) != RUL__C_NOISE) {
    rul__msg_cmp_print_w_atoms(CMP_DUPBLOCK, name_node, 1, block_name);
    return FALSE;		/* Unsuccessful */
  }

  /* Register the block with the construct registration subsystem. */
  rul__conrg_register_block (block_name,
			     RUL__C_DECL_BLOCK,
			     rul__ast_nearest_line_number(name_node));

  /* Create the declaring block. */
  new_decl_block(block_name);

  /* Cause declarations in this declaraion block to be visible here. */
  rul__decl_set_entry_block (block_name);
  rul__decl_make_block_visible (block_name);

  return TRUE;		/* Successful */
}


/*****************************************************************************/
Boolean
rul__sem_rule_block (Ast_Node ast)
{
  Ast_Node	rb_clause, param_node, temp_node,
    		name_node = rul__ast_get_child(ast); /* Block name */
  Mol_Symbol	block_name = rul__ast_get_value(name_node);
  Boolean	uses_declared = FALSE, strategy_declared = FALSE;
  Strategy	rb_strategy = CS__C_MEA;

  /* 
   * Check for a valid linkable block name
   */
  if (! rul__sem_check_linkable (block_name)) {
    rul__msg_cmp_print_w_atoms (CMP_INVBLKNAME, name_node, 1, block_name);
    return FALSE;
  }

  /*
   * Check to see if there's already a block with this name and
   * complain if so.
   */
  if (rul__conrg_get_block_type(block_name) != RUL__C_NOISE) {
    rul__msg_cmp_print_w_atoms(CMP_DUPBLOCK, name_node, 1, block_name);
    return FALSE;		/* Unsuccessful */
  }

  /* Register the block with the construct registration subsystem. */
  rul__conrg_register_block (block_name,
			     RUL__C_RULE_BLOCK,
			     rul__ast_nearest_line_number (name_node));

  /* Declare that this is the current (rule) block. */
  rul__decl_set_entry_block(block_name);

  /* Check the rule block clauses for uses (before implicit decl. block) */
  for (rb_clause = rul__ast_get_sibling (name_node);
       rb_clause != NULL;
       rb_clause = rul__ast_get_sibling (rb_clause)) {
    if (rul__ast_get_type(rb_clause) == AST__E_USES)
      if (uses_declared) {
	rul__msg_cmp_print_w_atoms(CMP_EBDUPUSES, rb_clause, 1, block_name);
	temp_node = rb_clause;
	rb_clause = rul__ast_get_previous_sibling (rb_clause);
	rul__ast_delete_node(temp_node);
      }
      else {
	uses_declared = TRUE;
	for (param_node = rul__ast_get_child (rb_clause);
	     param_node != NULL;
	     param_node = rul__ast_get_sibling (param_node)) {
	  if (! rul__sem_check_decl_usefile
	      (rul__ast_get_value (param_node), param_node))
	    return FALSE;
	}
      }
  }

  /* Create the implicit declaring block for this rule block. */
  new_decl_block(block_name);

  for (rb_clause = rul__ast_get_sibling (name_node);
       rb_clause != NULL;
       rb_clause = rul__ast_get_sibling (rb_clause))

    switch (rul__ast_get_type (rb_clause)) {

    case AST__E_USES :
      break;

    case AST__E_STRATEGY : /* only LEX or MEA - verified by grammar */
      if (strategy_declared) {
	rul__msg_cmp_print_w_atoms (CMP_EBDUPSTR, rb_clause, 1, block_name);
	temp_node = rb_clause;
	rb_clause = rul__ast_get_previous_sibling (rb_clause);
	rul__ast_delete_node(temp_node);
      }
      else {
	strategy_declared = TRUE;
	assert (rul__ast_get_value_type (rb_clause) == AST__E_TYPE_STRATEGY);
	rb_strategy = (Strategy) rul__ast_get_long_value (rb_clause);
      }
      break;

    default:
      assert (FALSE); /* invalid AST; parser shouldn't allow this */
      break;
    }

  rul__decl_set_eb_strategy (rb_strategy);
  
  /* Cause declarations declared in this implicit declaring block
     to be visible to this rule block. */
  rul__decl_make_block_visible(block_name);

  return TRUE;		/* Successful */
}

/*****************************************************************************/
Boolean
rul__sem_end_block (Ast_Node ast)
{
  Ast_Node	name_node;
  Mol_Symbol	end_block_name;


  if (ast != NULL) {

    name_node = rul__ast_get_child(ast); /* Block name */
    if (name_node) {		/* If this END-BLOCK has a name */

      end_block_name = rul__ast_get_value(name_node);

      if (end_block_name != rul__conrg_get_cur_block_name ()) {
	rul__msg_cmp_print_w_atoms (CMP_BADENDBLK, name_node,
				    2, end_block_name,
				    rul__conrg_get_cur_block_name ());
      }
      rul__conrg_register_construct (end_block_name, RUL__C_END_BLOCK,
				     rul__ast_nearest_line_number (name_node));
    }
    else {
      rul__conrg_register_construct (rul__conrg_get_cur_block_name (),
				     RUL__C_END_BLOCK,
				     rul__ast_nearest_line_number (ast));
    }
  }

  if (ast && rul__conrg_get_cur_group_name () != rul__mol_symbol_nil ()) {
    rul__conrg_register_construct (rul__conrg_get_cur_group_name (),
				   RUL__C_END_GROUP,
				   rul__ast_nearest_line_number (ast));
    /* ??? return as error ??? */
  }

  rul__decl_finish_decl_block();
  rul__lvar_clear_block ();
  rul__sem_use_entry_vars (TRUE);
  rul__sem_initialize_rhs_vars ();
  rul__sem_use_entry_vars (FALSE);
  rul__sem_initialize_rhs_vars ();
  rul__sem_initialize_oin_ht ();

  return TRUE;		/* Successful */
}

/*****************************************************************************/
static void
new_decl_block (Mol_Symbol block_name)
{
  /*
   * Create a new declaring block.
   */
  rul__decl_create_decl_block (block_name);
  rul__cons_init ();		/* Init constant table for new block */
  rul__class_init ();		/* Init class table */
}

/*****************************************************************************/
Boolean
rul__sem_check_linkable (Mol_Symbol block_name)
{
  char  blk_nam[RUL_C_MAX_SYMBOL_SIZE + 1];
  long  i;

  rul__mol_use_printform (block_name, blk_nam, sizeof(blk_nam));

  for (i = 0; i < RUL_C_MAX_SYMBOL_SIZE; i++) {
    if (blk_nam[i] == '\0') {
      if (i == 0) return FALSE; else return TRUE;
    }
    else if (blk_nam[i] == '_') {
      if (i == 0) return FALSE;
    }
    else if (! ((blk_nam[i] >= 'A' && blk_nam[i] <= 'Z') ||
		(blk_nam[i] >= 'a' && blk_nam[i] <= 'z') ||
		(blk_nam[i] >= '0' && blk_nam[i] <= '9'))) {
      return FALSE;
    }
  }
  return TRUE;
}

/*****************************************************************************/
Boolean
rul__sem_rule_group (Ast_Node ast)
{
  Ast_Node	name_node = rul__ast_get_child(ast); /* Group name */
  Mol_Symbol	group_name = rul__ast_get_value(name_node);

  if (rul__conrg_get_cur_group_name () != rul__mol_symbol_nil ()) {
    rul__msg_cmp_print_w_atoms (CMP_NESGROUP, name_node, 2,
				group_name, rul__conrg_get_cur_group_name ());
    return FALSE;		/* Unsuccessful */
  }

  /*
   * Check to see if there's already a group with this name and
   * complain if so.
   */
  if (rul__conrg_get_construct_type(group_name) != RUL__C_NOISE) {
    rul__msg_cmp_print_w_atoms (CMP_DUPGROUP, name_node, 1, group_name);
    return FALSE;		/* Unsuccessful */
  }

  /* Register the group with the construct registration subsystem. */
  rul__conrg_register_construct (group_name,
				 RUL__C_RULE_GROUP,
				 rul__ast_nearest_line_number(name_node));

  return TRUE;		/* Successful */
}

/*****************************************************************************/
Boolean
rul__sem_end_group (Ast_Node ast)
{
  Ast_Node	  name_node = rul__ast_get_child(ast); /* Group name */
  Mol_Symbol	  end_group_name;

  if (name_node) {		/* If this END-GROUP has a name */
    end_group_name = rul__ast_get_value(name_node);

    if (end_group_name != rul__conrg_get_cur_group_name ()) {
      rul__msg_cmp_print_w_atoms (CMP_BADENDGRP, name_node,
				  2, end_group_name,
				  rul__conrg_get_cur_group_name ());
      return FALSE;
    }
    rul__conrg_register_construct (end_group_name, RUL__C_END_GROUP,
				   rul__ast_nearest_line_number(name_node));
  }
  else {
    rul__conrg_register_construct (rul__conrg_get_cur_group_name (),
				   RUL__C_END_GROUP,
				   rul__ast_nearest_line_number (ast));
  }

  return TRUE;		/* Successful */
}

/*****************************************************************************/
void
rul__sem_get_ext_param_name(char *buffer, Mol_Symbol argname,
			    Ext_Rt_Decl ext_rt)
/* Write generated variable name for external (animal) form of parameter
   e.g., <FOO>, into buffer.  argname includes <>s, e.g., <FOO>.
   Example result: RUL_arg3_FOO */
{
      char char_arg_name[RUL_C_MAX_SYMBOL_SIZE+1]; /* Printform of symbol */
      char alpha_name[RUL_C_MAX_SYMBOL_SIZE+1];	/* alphanumeric prefix */
      long arg_index = rul__decl_ext_rt_param_index(ext_rt, argname);
				/* position of this arg */

      rul__mol_use_printform(argname, char_arg_name, sizeof char_arg_name);
      sscanf(
	 char_arg_name,
	 "<%[0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz]",
	 alpha_name);		/* Will get leading alphanumerics */
      sprintf(buffer, "%s_arg%ld_%.50s",
	      RUL__C_MSG_PREFIX, arg_index-1, alpha_name);
}

/*****************************************************************************/
static void
process_eb_param (Ast_Node param_node,
		  Mol_Symbol block_name,
		  Ext_Rt_Decl ext_rt,
		  Boolean is_the_return_value,
		  long par_index)
{
  Ast_Node     node, arg_node;
  Ext_Type     ext_type;
  Cardinality  a_len;
  Ext_Mech     ext_mech;
  Mol_Symbol   a_arg = NULL;
  Value        rul_var_val, rul_arg_val;
  Value        ext_var_val, cvt_ext_to_rul_val;
  Mol_Symbol   par_sym = NULL, ext_sym;
  static char  ext_name[RUL_C_MAX_SYMBOL_SIZE];
  Value        a_val = NULL, tmp_val;

  if (is_the_return_value) {
    ext_type = rul__decl_ext_rt_ret_type (ext_rt);
    ext_mech = rul__decl_ext_rt_mech (ext_rt);
    a_len = rul__decl_ext_rt_ret_a_len (ext_rt);
    a_arg = rul__decl_ext_rt_ret_a_arg (ext_rt);
  }
  else {
    par_sym = rul__decl_ext_rt_param_name (ext_rt, par_index);
    if (par_sym == NULL)
      return;
    ext_type = rul__decl_ext_rt_param_type (ext_rt, par_index);
    ext_mech = rul__decl_ext_rt_param_mech (ext_rt, par_index);
    a_len = rul__decl_ext_rt_param_a_len (ext_rt, par_index);
    a_arg = rul__decl_ext_rt_param_a_arg (ext_rt, par_index);
  }

  node = rul__ast_mol_node (AST__E_VARIABLE, par_sym);
  rul_var_val = rul__sem_check_rhs_var (node, SEM__C_VALUE_ASSIGNMENT);
  rul__ast_free (node);
  rul__sem_get_ext_param_name(ext_name, par_sym, ext_rt);
  ext_sym = rul__mol_make_symbol (ext_name);
  ext_var_val = rul__val_create_named_ext_var (ext_sym, ext_type,
					       a_len,   ext_mech,
					       SEM__C_NO_MEMORY_ALLOCATED);

  if (a_len == EXT__C_IMPLICIT_ARRAY && a_arg != NULL) {
    arg_node = rul__ast_mol_node (AST__E_VARIABLE, a_arg);
    rul_arg_val = rul__sem_check_rhs_var (arg_node, SEM__C_VALUE_ACCESS);
    a_val = rul__sem_convert_to_ext_value (rul_arg_val, ext_type_long,
					   EXT__C_NOT_ARRAY, NULL,
					   ext_mech_value, NULL, 
					   SEM__C_ON_RHS, param_node);
    if (rul__val_is_assignment (a_val)) {
      tmp_val = a_val;
      a_val = rul__val_copy_value (rul__val_get_assignment_from (tmp_val));
      rul__val_free_value (tmp_val);
    }
    rul__ast_free (arg_node);
  }

  cvt_ext_to_rul_val = rul__sem_convert_to_rul_value (ext_var_val,
						      rul_var_val,
						      param_node,
						      a_val);

  rul__sem_reset_var_binding_site (par_sym, cvt_ext_to_rul_val);
}

