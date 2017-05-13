/****************************************************************************
**                                                                         **
**                     C M P _ S E M _ S Q L . C                           **
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
 *	This module provides semantic checks for RHS sql actions.
 *
 * MODIFIED BY:
 *	DEC	Digital Equipment Corporation
 *	CPQ	Compaq Computer Corporation
 *
 * MODIFICATION HISTORY:
 *	15-Jan-1993	DEC	Initial version
 *	01-Dec-1999	CPQ	Releasew ith GPL
 *
 */

#include <common.h>
#include <cmp_comm.h>
#include <mol.h>
#include <ios.h>
#include <decl.h>
#include <ast.h>
#include <val.h>
#include <sem.h>
#include <msg.h>
#include <cmp_msg.h>
#include <conrg.h>
#include <emit.h>


static void display_check_error (char *err, Ast_Node node);

/* sql actions */


Value rul__sem_check_sql_attach (Ast_Node ast)
{
  Ast_Node	node;
  Ast_Node	pathnode;
  Ast_Node_Type node_type;
  Value		func_val, arg_val;
  char		buffer[RUL_C_MAX_SYMBOL_SIZE+1];

  /* SQL-ATTACH db-spec db-scope */

  node = rul__ast_get_child(ast);
  node_type = rul__ast_get_type(node);
  if (node_type != AST__E_CONSTANT &&
      node_type != AST__E_SQL_PATHNAME) {
    display_check_error (CMP_INVATHPAR, node);
    return NULL;
  }

  func_val = rul__val_create_function_str ("rul__sql_attach", 3);

  if (node_type == AST__E_CONSTANT) {
    /* filename used */
    rul__val_set_nth_subvalue (func_val, 1,
			       rul__val_create_long_animal (1));
    pathnode = node;
  }
  else {
    /* pathname used */
    rul__val_set_nth_subvalue (func_val, 1,
			       rul__val_create_long_animal (0));
    pathnode = rul__ast_get_child (node);
    node_type = rul__ast_get_type(pathnode);
    if (node_type != AST__E_CONSTANT) {
      display_check_error (CMP_INVPATPAR, pathnode);
      rul__val_free_value (func_val);
      return NULL;
    }
  }

  rul__mol_use_printform (rul__ast_get_value (pathnode),
			  buffer, RUL_C_MAX_SYMBOL_SIZE);
  arg_val = rul__val_create_asciz_animal (buffer);
  rul__val_set_nth_subvalue (func_val, 2, arg_val);

  /* now check the db-scope */
  node = rul__ast_get_sibling (node);
  if (node != NULL) {
    if (rul__ast_get_type (node) != AST__E_CONSTANT) {
      display_check_error (CMP_INVSCOPAR, node);
      rul__val_free_value (func_val);
      return NULL;
    }
    rul__mol_use_printform (rul__ast_get_value (node),
			    buffer, RUL_C_MAX_SYMBOL_SIZE);
  }
  else
    buffer[0] = '\0';
  
  arg_val = rul__val_create_asciz_animal (buffer);
  rul__val_set_nth_subvalue (func_val, 3, arg_val);

  return func_val;
}


Value rul__sem_check_sql_commit (Ast_Node ast)
{
  /* SQL-COMMIT */

  return rul__val_create_function_str ("rul__sql_commit", 0);
}


Value rul__sem_check_sql_delete (Ast_Node ast)
{
  Ast_Node	node, tab_node;
  Ast_Node_Type node_type;
  Value		func_val, arg_val, ret_val, curr_val;
  char		buffer[RUL_C_MAX_SYMBOL_SIZE+1];
  Boolean	quoted;

  /* SQL-DELETE db-table opt-rse */

  /* now check the db-table */
  tab_node = rul__ast_get_child (ast);
  if (rul__ast_get_type (tab_node) != AST__E_CONSTANT) {
    display_check_error (CMP_INVTBLPAR, tab_node);
    return NULL;
  }

  ret_val = curr_val = rul__val_create_function_str ("rul__sql_rse_init", 0);

  /* now check the opt-rse */
  node = rul__ast_get_sibling (tab_node);
  if (node != NULL) {
    assert (rul__ast_get_type (node) == AST__E_SQL_RSE);
    
    for (node = rul__ast_get_child (node);
	 node != NULL;
	 node = rul__ast_get_sibling (node)) {
      
      quoted = FALSE;
      node_type = rul__ast_get_type (node);
      if (node_type == AST__E_CONSTANT)
	arg_val = rul__sem_check_rul_constant (node, SEM__C_RETURN_RUL_TYPE);
      else if (node_type == AST__E_VARIABLE)
        arg_val = rul__sem_check_rhs_var (node, SEM__C_VALUE_ACCESS);
      else if (node_type == AST__E_QUOTED_VAR) {
        arg_val = rul__sem_check_rhs_var (node, SEM__C_VALUE_ACCESS);
	quoted = TRUE;
      }
      else {
	display_check_error (CMP_INVRSEPAR, node);
	rul__val_free_value (ret_val);
	return NULL;
      }
      
      if (arg_val == NULL) {
	rul__val_free_value (ret_val);
	return NULL;
      }
      
      func_val = rul__val_create_function_str ("rul__sql_rse_symbol", 2);
      rul__val_set_nth_subvalue (func_val, 1, arg_val);
      if (quoted)
	rul__val_set_nth_subvalue (func_val, 2,
			       rul__val_create_long_animal (1));
      else
	rul__val_set_nth_subvalue (func_val, 2,
			       rul__val_create_long_animal (0));
      curr_val = rul__val_set_next_value (curr_val, func_val);
    }
  }

  func_val = rul__val_create_function_str ("rul__sql_delete", 3);

  rul__mol_use_printform (rul__ast_get_value (tab_node),
			  buffer, RUL_C_MAX_SYMBOL_SIZE);
  arg_val = rul__val_create_asciz_animal (buffer);
  rul__val_set_nth_subvalue (func_val, 1, arg_val);
  rul__val_set_nth_subvalue (func_val, 2,
			     rul__val_create_function_str ("rul__sql_rse", 0));
  rul__val_set_nth_subvalue (func_val, 3,
			     rul__val_create_blk_or_loc (CMP__E_INT_ENTRY_DATA,
							 "eb_data"));
  rul__val_set_next_value (curr_val, func_val);

  return ret_val;
}


Value rul__sem_check_sql_detach (Ast_Node ast)
{
  /* SQL-DETACH */

  return rul__val_create_function_str ("rul__sql_detach", 0);
}


Value rul__sem_check_sql_fetch_each (Ast_Node ast)
{
  Ast_Node	node, rse_node, first_var, act_node;
  Ast_Node_Type node_type;
  Value		func_val, arg_val, curr_val, ret_val = NULL;
  Value		wida_ptr_val, wida_val, widc_val, cmplx_val, loop_val;
  long		var_count, i;
  Boolean	quoted;

  /* SQL-FETCH-EACH db-vars ( res ) actions */

  /* count the number of variables to use */
  first_var = node = rul__ast_get_child (ast);
  var_count = 0;
  while (rul__ast_get_type (node) == AST__E_VARIABLE) {
    node = rul__ast_get_sibling (node);
    var_count += 1;
  }

  if (rul__ast_get_type(node) != AST__E_SQL_RSE) {
    display_check_error (CMP_INVFTEPAR, node);
    return NULL;
  }

  loop_val = curr_val = NULL;

  /*
   * do the inside of the while loop first
   * make all the assignments from the mol array into the rhs_vars
   */

  /* create the atom-array variable */
  wida_val = rul__val_create_unnamed_ext_var (
		      rul__sem_register_ext_temp (ext_type_atom,
						  EXT__C_IMPLICIT_ARRAY,
						  ext_mech_value,
						  SEM__C_ON_RHS),
			      ext_type_atom, EXT__C_IMPLICIT_ARRAY,
			      ext_mech_value, SEM__C_NO_MEMORY_ALLOCATED);

  for (node = first_var, i = 0;
       rul__ast_get_type (node) == AST__E_VARIABLE;
       node = rul__ast_get_sibling (node), i++) {

    arg_val = rul__sem_check_rhs_var (node, SEM__C_VALUE_ASSIGNMENT);
    rul__sem_reset_var_binding_site ((Mol_Symbol) rul__ast_get_value (node),
				     arg_val);
    if (arg_val == NULL) {
      if (loop_val)
	rul__val_free_value (loop_val);
      return NULL;
    }

#ifndef NDEBUG
    func_val = rul__val_create_function_str ("rul__mol_decr_uses", 1);
#else
    func_val = rul__val_create_function_str ("MDU", 1);
#endif
    rul__val_set_nth_subvalue (func_val, 1, arg_val);
    if (curr_val == NULL) {
      curr_val = func_val;
      loop_val = func_val;
    }
    else
      curr_val = rul__val_set_next_value (curr_val, func_val);

    rul__val_set_ext_var_index (wida_val, i);
    func_val = rul__val_create_assignment (rul__val_copy_value (arg_val),
					   rul__val_copy_value (wida_val));
    curr_val = rul__val_set_next_value (curr_val, func_val);
  }

  /* assign the mol array = mol array ptr */
  /* create the atom-array ptr variable */
  wida_ptr_val = rul__val_create_unnamed_ext_var (
		      rul__sem_register_ext_temp (ext_type_atom_ptr,
						  EXT__C_NOT_ARRAY,
						  ext_mech_value,
						  SEM__C_ON_RHS),
			      ext_type_atom_ptr, EXT__C_NOT_ARRAY,
			      ext_mech_value, SEM__C_NO_MEMORY_ALLOCATED);
  rul__val_set_ext_var_index (wida_val, -1);
  func_val = rul__val_create_assignment (wida_val, wida_ptr_val);
  rul__val_set_next_value (func_val, loop_val);

  /* attach the loop code to the rse node */
  rse_node = node;
  rul__ast_attach_value (rse_node, AST__E_TYPE_VALUE, func_val);

  curr_val = ret_val = rul__val_create_function_str ("rul__sql_rse_init", 0);

  for (node = rul__ast_get_child (node);
       node != NULL;
       node = rul__ast_get_sibling (node)) {

    quoted = FALSE;
    node_type = rul__ast_get_type (node);
    if (node_type == AST__E_CONSTANT)
      arg_val = rul__sem_check_rul_constant (node, SEM__C_RETURN_RUL_TYPE);
    else if (node_type == AST__E_VARIABLE)
      arg_val = rul__sem_check_rhs_var (node, SEM__C_VALUE_ACCESS);
    else if (node_type == AST__E_QUOTED_VAR) {
      arg_val = rul__sem_check_rhs_var (node, SEM__C_VALUE_ACCESS);
      quoted = TRUE;
    }
    else {
      display_check_error (CMP_INVFETPAR, node);
      rul__val_free_value (ret_val);
      return NULL;
    }

    if (arg_val == NULL) {
      rul__val_free_value (ret_val);
      return NULL;
    }

    func_val = rul__val_create_function_str ("rul__sql_rse_symbol", 2);
    rul__val_set_nth_subvalue (func_val, 1, arg_val);
    if (quoted)
      rul__val_set_nth_subvalue (func_val, 2,
			       rul__val_create_long_animal (1));
    else
      rul__val_set_nth_subvalue (func_val, 2,
			       rul__val_create_long_animal (0));
    curr_val = rul__val_set_next_value (curr_val, func_val);
  }

  func_val = rul__val_create_function_str ("rul__sql_fetch_setup", 1);
  rul__val_set_nth_subvalue (func_val, 1,
			     rul__val_create_function_str ("rul__sql_rse", 0));
  curr_val = rul__val_set_next_value (curr_val, func_val);
  
  /* create the wid-count variable, mech is rw and assign to var count */
  widc_val = rul__val_create_unnamed_ext_var (
			      rul__sem_register_ext_temp (ext_type_long,
							  EXT__C_NOT_ARRAY,
							  ext_mech_value,
							  SEM__C_ON_RHS),
			      ext_type_long, EXT__C_NOT_ARRAY,
			      ext_mech_value, SEM__C_NO_MEMORY_ALLOCATED);

  func_val = rul__val_create_assignment (widc_val,
				 rul__val_create_long_animal (var_count));
  curr_val = rul__val_set_next_value (curr_val, func_val);

  func_val = rul__val_create_assignment (rul__val_copy_value (wida_ptr_val),
				 rul__val_create_null (CMP__E_INT_MOLECULE));
  curr_val = rul__val_set_next_value (curr_val, func_val);

  /* function for doing the fetchs */
  wida_ptr_val = rul__val_copy_value (wida_ptr_val);
  rul__val_set_ext_var_mech (wida_ptr_val, ext_mech_ref_rw);
  func_val = rul__val_create_function_str  ("rul__sql_fetch_each", 3);
  rul__val_set_nth_subvalue (func_val, 1, wida_ptr_val);
  rul__val_set_nth_subvalue (func_val, 2, rul__val_copy_value (widc_val));
  rul__val_set_nth_subvalue (func_val, 3,
	     rul__val_create_blk_or_loc (CMP__E_INT_ENTRY_DATA, "eb_data"));

  /* define the while statement */
  cmplx_val = rul__val_create_complex (3);
  rul__val_set_nth_subvalue (cmplx_val, 1,
		     rul__val_create_long_animal ((long)EMIT__E_REL_EQ));
  rul__val_set_nth_subvalue (cmplx_val, 2, func_val);
  rul__val_set_nth_subvalue (cmplx_val, 3,
			     rul__val_create_long_animal (1));

  rul__ast_attach_value (first_var, AST__E_TYPE_VALUE, cmplx_val);

  /*
   * loop for each fetch-each action
   */

  act_node = rul__ast_get_sibling (rse_node);

  for (node = rul__ast_get_child (act_node);
       node != NULL;
       node = rul__ast_get_sibling(node)) {

    arg_val = rul__sem_check_rhs_action (node, 1);
    if (arg_val == NULL) {
      rul__val_free_value (ret_val);
      return NULL;
    }
    else {
      rul__ast_attach_value (node, AST__E_TYPE_VALUE, arg_val);
    }
  }

  /* make a copy of the wme-id array value and change passing mech to value */
  wida_val = rul__val_copy_value (wida_val);
/*  rul__val_set_ext_var_mech (wida_val, ext_mech_value); */

  func_val = rul__val_create_function_str ("rul__sql_fetch_cleanup", 1);
  rul__val_set_nth_subvalue (func_val, 1, wida_val);
  rul__ast_attach_value (act_node, AST__E_TYPE_VALUE, func_val);

  return ret_val;
}


Value rul__sem_check_sql_fetch_obj (Ast_Node ast, Boolean return_required, 
				    Value return_to_value)
{
  Ast_Node	node;
  Ast_Node_Type node_type;
  Value		curr_val, func_val, arg_val, ret_val;
  Value		wida_val, widc_val;	/* wme id array and count values */
  Boolean	quoted;

  /* SQL-FETCH-AS-OBJECT res */

  /* produce the following:
   *
   *   rul__sql_rse_init ();
   *   rul__sql_rse_symbol (rse_param, flag);
   *   ...
   *   rul__sql_fetch_setup (rul__sql_rse ());
   *   rul__sql_fetch_to_wme (&tmp_id_array, &tmp_long);
   *   [ ret_val = rul__cvt_a_ta_c_at (tmp_long, tmp_id_array) ]
   *   rul__sql_fetch_cleanup (tmp_mol);
   */
  
  node = rul__ast_get_child (ast);
  assert (rul__ast_get_type (node) == AST__E_SQL_RSE);

  ret_val = curr_val = rul__val_create_function_str ("rul__sql_rse_init", 0);

  for (node = rul__ast_get_child (node);
       node != NULL;
       node = rul__ast_get_sibling (node)) {
    
    quoted = FALSE;
    node_type = rul__ast_get_type (node);
    if (node_type == AST__E_CONSTANT)
      arg_val = rul__sem_check_rul_constant (node, SEM__C_RETURN_RUL_TYPE);
    else if (node_type == AST__E_VARIABLE)
      arg_val = rul__sem_check_rhs_var (node, SEM__C_VALUE_ACCESS);
    else if (node_type == AST__E_QUOTED_VAR) {
      arg_val = rul__sem_check_rhs_var (node, SEM__C_VALUE_ACCESS);
      quoted = TRUE;
    }
    else {
      display_check_error (CMP_INVFETPAR, node);
      rul__val_free_value (ret_val);
      return NULL;
    }

    if (arg_val == NULL) {
      rul__val_free_value (ret_val);
      return NULL;
    }

    func_val = rul__val_create_function_str ("rul__sql_rse_symbol", 2);
    rul__val_set_nth_subvalue (func_val, 1, arg_val);
    if (quoted)
      rul__val_set_nth_subvalue (func_val, 2,
			      rul__val_create_long_animal (1));
    else
      rul__val_set_nth_subvalue (func_val, 2,
			      rul__val_create_long_animal (0));
    curr_val = rul__val_set_next_value (curr_val, func_val);
  }

  func_val = rul__val_create_function_str ("rul__sql_fetch_setup", 1);
  rul__val_set_nth_subvalue (func_val, 1,
			     rul__val_create_function_str ("rul__sql_rse", 0));
  curr_val = rul__val_set_next_value (curr_val, func_val);
  
  /* create the wid-array variable, mech is rw */
  wida_val = rul__val_create_unnamed_ext_var (
			      rul__sem_register_ext_temp (ext_type_atom_ptr,
							  EXT__C_NOT_ARRAY,
							  ext_mech_value,
							  SEM__C_ON_RHS),
			      ext_type_atom_ptr, EXT__C_NOT_ARRAY,
			      ext_mech_value, SEM__C_NO_MEMORY_ALLOCATED);
  func_val = rul__val_create_assignment (wida_val,
				 rul__val_create_null (CMP__E_INT_MOLECULE));
  curr_val = rul__val_set_next_value (curr_val, func_val);

  /* create the wid-count variable, mech is rw */
  widc_val = rul__val_create_unnamed_ext_var (
			      rul__sem_register_ext_temp (ext_type_long,
							  EXT__C_NOT_ARRAY,
							  ext_mech_value,
							  SEM__C_ON_RHS),
			      ext_type_long, EXT__C_NOT_ARRAY,
			      ext_mech_ref_rw, SEM__C_NO_MEMORY_ALLOCATED);

  wida_val = rul__val_copy_value (wida_val);
  rul__val_set_ext_var_mech (wida_val, ext_mech_ref_rw);

  func_val = rul__val_create_function_str ("rul__sql_fetch_to_wme", 3);
  rul__val_set_nth_subvalue (func_val, 1, wida_val);
  rul__val_set_nth_subvalue (func_val, 2, widc_val);
  rul__val_set_nth_subvalue (func_val, 3,
			     rul__val_create_blk_or_loc (CMP__E_INT_ENTRY_DATA,
							 "eb_data"));
  curr_val = rul__val_set_next_value (curr_val, func_val);

  /* make a copy of the wme-id array value and change passing mech to value */
  wida_val = rul__val_copy_value (wida_val);
  rul__val_set_ext_var_mech (wida_val, ext_mech_value);

  if (return_required) {
    func_val = rul__val_create_function_str ("rul__cvt_a_ta_c_at", 2);
    rul__val_set_nth_subvalue (func_val, 1, rul__val_copy_value (wida_val));

    /* make a copy of the wme countvalue and change passing mech to value */
    widc_val = rul__val_copy_value (widc_val);
    rul__val_set_ext_var_mech (widc_val, ext_mech_value);
    rul__val_set_nth_subvalue (func_val, 2, widc_val);

    /* set the shape and domain of the return value */
    rul__val_set_shape_and_domain (return_to_value, shape_compound, dom_any);
    func_val = rul__val_create_assignment (return_to_value, func_val);
    curr_val = rul__val_set_next_value (curr_val, func_val);
  }

  func_val = rul__val_create_function_str ("rul__sql_fetch_cleanup", 1);
  rul__val_set_nth_subvalue (func_val, 1, wida_val);
  rul__val_set_next_value (curr_val, func_val);

  return ret_val;
}


Value rul__sem_check_sql_insert (Ast_Node ast)
{
  Ast_Node	node, tab_node;
  Ast_Node_Type node_type;
  Value		func_val, arg_val, ret_val, curr_val;
  char		buffer[RUL_C_MAX_SYMBOL_SIZE+1];
  Boolean	quoted;

  /* SQL-INSERT db-table rse */

  /* now check the db-table */
  tab_node = rul__ast_get_child (ast);
  if (rul__ast_get_type (tab_node) != AST__E_CONSTANT) {
    display_check_error (CMP_INVTBLPAR, tab_node);
    return NULL;
  }

  ret_val = curr_val = rul__val_create_function_str ("rul__sql_rse_init", 0);

  /* now check the opt-rse */
  node = rul__ast_get_sibling (tab_node);
  assert (rul__ast_get_type (node) == AST__E_SQL_RSE);
    
  for (node = rul__ast_get_child (node);
       node != NULL;
       node = rul__ast_get_sibling (node)) {
      
    quoted = FALSE;
    node_type = rul__ast_get_type (node);
    if (node_type == AST__E_CONSTANT)
      arg_val = rul__sem_check_rul_constant (node, SEM__C_RETURN_RUL_TYPE);
    else if (node_type == AST__E_VARIABLE)
      arg_val = rul__sem_check_rhs_var (node, SEM__C_VALUE_ACCESS);
    else if (node_type == AST__E_QUOTED_VAR) {
      arg_val = rul__sem_check_rhs_var (node, SEM__C_VALUE_ACCESS);
      quoted = TRUE;
    }
    else {
      display_check_error (CMP_INVRSEPAR, node);
      rul__val_free_value (ret_val);
      return NULL;
    }

    if (arg_val == NULL) {
      rul__val_free_value (ret_val);
      return NULL;
    }

    func_val = rul__val_create_function_str ("rul__sql_rse_symbol", 2);
    rul__val_set_nth_subvalue (func_val, 1, arg_val);
    if (quoted)
      rul__val_set_nth_subvalue (func_val, 2,
			       rul__val_create_long_animal (1));
    else
      rul__val_set_nth_subvalue (func_val, 2,
			       rul__val_create_long_animal (0));
    curr_val = rul__val_set_next_value (curr_val, func_val);
  }

  func_val = rul__val_create_function_str ("rul__sql_insert", 3);

  rul__mol_use_printform (rul__ast_get_value (tab_node),
			  buffer, RUL_C_MAX_SYMBOL_SIZE);
  arg_val = rul__val_create_asciz_animal (buffer);
  rul__val_set_nth_subvalue (func_val, 1, arg_val);
  rul__val_set_nth_subvalue (func_val, 2,
			     rul__val_create_function_str ("rul__sql_rse", 0));
  rul__val_set_nth_subvalue (func_val, 3,
			     rul__val_create_blk_or_loc (CMP__E_INT_ENTRY_DATA,
							 "eb_data"));
  rul__val_set_next_value (curr_val, func_val);

  return ret_val;
}

Value rul__sem_check_sql_insert_obj (Ast_Node ast)
{
  Ast_Node	node;
  Value		func_val, arg_val;

  /* SQL-INSERT-FROM-OBJECT var */

  /* get and verify element id binding */
  node = rul__ast_get_child(ast);
  if (rul__ast_get_type(node) != AST__E_VARIABLE) {
    display_check_error (CMP_INVINSPAR, node);
    return NULL;
  }

  arg_val = rul__sem_check_rhs_var(node, SEM__C_VALUE_ACCESS);
  if (arg_val == NULL)
    return NULL;

  if (rul__val_get_domain(arg_val) != dom_instance_id) {
    display_check_error (CMP_INVINSVAR, node);
    rul__val_free_value (arg_val);
    return NULL;
  }

  /*
   * create the modify function value
   * store the instance id (bound valiable) in the functions 1st par slot
   */
  func_val = rul__val_create_function_str ("rul__sql_insert_from_wme", 2);
  rul__val_set_nth_subvalue (func_val, 1, arg_val);
  rul__val_set_nth_subvalue (func_val, 2,
			     rul__val_create_blk_or_loc (CMP__E_INT_ENTRY_DATA,
							 "eb_data"));

  return func_val;
}

Value rul__sem_check_sql_rollback (Ast_Node ast)
{
  /* SQL-ROLLBACK */

  return rul__val_create_function_str ("rul__sql_rollback", 0);
}


Value rul__sem_check_sql_start (Ast_Node ast)
{
  Ast_Node	node;
  Ast_Node_Type node_type;
  Value		func_val, arg_val, ret_val, curr_val;
  Boolean	quoted;

  /* SQL-START opt-rse */

  ret_val = curr_val = rul__val_create_function_str ("rul__sql_rse_init", 0);

  /* now check the opt-rse */
  node = rul__ast_get_child (ast);
  if (node) {
    assert (rul__ast_get_type (node) == AST__E_SQL_RSE);
    
    for (node = rul__ast_get_child (node);
	 node != NULL;
	 node = rul__ast_get_sibling (node)) {
      
      quoted = FALSE;
      node_type = rul__ast_get_type (node);
      if (node_type == AST__E_CONSTANT)
	arg_val = rul__sem_check_rul_constant (node, SEM__C_RETURN_RUL_TYPE);
      else if (node_type == AST__E_VARIABLE)
	arg_val = rul__sem_check_rhs_var (node, SEM__C_VALUE_ACCESS);
      else if (node_type == AST__E_QUOTED_VAR) {
        arg_val = rul__sem_check_rhs_var (node, SEM__C_VALUE_ACCESS);
	quoted = TRUE;
      }
      else {
	display_check_error (CMP_INVRSEPAR, node);
	rul__val_free_value (ret_val);
	return NULL;
      }

      if (arg_val == NULL) {
	rul__val_free_value (ret_val);
	return NULL;
      }

      func_val = rul__val_create_function_str ("rul__sql_rse_symbol", 2);
      rul__val_set_nth_subvalue (func_val, 1, arg_val);
      if (quoted)
	rul__val_set_nth_subvalue (func_val, 2,
			       rul__val_create_long_animal (1));
      else
	rul__val_set_nth_subvalue (func_val, 2,
			       rul__val_create_long_animal (0));
      curr_val = rul__val_set_next_value (curr_val, func_val);
    }
  }

  func_val = rul__val_create_function_str ("rul__sql_start", 1);
  rul__val_set_nth_subvalue (func_val, 1,
			     rul__val_create_function_str ("rul__sql_rse", 0));
  rul__val_set_next_value (curr_val, func_val);

  return ret_val;
}


Value rul__sem_check_sql_update (Ast_Node ast)
{
  Ast_Node	node, tab_node;
  Ast_Node_Type node_type;
  Value		func_val, arg_val, ret_val, curr_val;
  char		buffer[RUL_C_MAX_SYMBOL_SIZE+1];
  Boolean	quoted;

  /* SQL-UPDATE db-table rse */

  /* now check the db-table */
  tab_node = rul__ast_get_child (ast);
  if (rul__ast_get_type (tab_node) != AST__E_CONSTANT) {
    display_check_error (CMP_INVTBLPAR, tab_node);
    return NULL;
  }

  ret_val = curr_val = rul__val_create_function_str ("rul__sql_rse_init", 0);

  /* now check the rse */
  node = rul__ast_get_sibling (tab_node);
  assert (rul__ast_get_type (node) == AST__E_SQL_RSE);
    
  for (node = rul__ast_get_child (node);
       node != NULL;
       node = rul__ast_get_sibling (node)) {
      
    quoted = FALSE;
    node_type = rul__ast_get_type (node);
    if (node_type == AST__E_CONSTANT)
      arg_val = rul__sem_check_rul_constant (node, SEM__C_RETURN_RUL_TYPE);
    else if (node_type == AST__E_VARIABLE)
      arg_val = rul__sem_check_rhs_var (node, SEM__C_VALUE_ACCESS);
    else if (node_type == AST__E_QUOTED_VAR) {
      arg_val = rul__sem_check_rhs_var (node, SEM__C_VALUE_ACCESS);
      quoted = TRUE;
    }
    else {
      display_check_error (CMP_INVRSEPAR, node);
      rul__val_free_value (ret_val);
      return NULL;
    }

    if (arg_val == NULL) {
      rul__val_free_value (ret_val);
      return NULL;
    }

    func_val = rul__val_create_function_str ("rul__sql_rse_symbol", 2);
    rul__val_set_nth_subvalue (func_val, 1, arg_val);
    if (quoted)
      rul__val_set_nth_subvalue (func_val, 2,
			       rul__val_create_long_animal (1));
    else
      rul__val_set_nth_subvalue (func_val, 2,
			       rul__val_create_long_animal (0));
    curr_val = rul__val_set_next_value (curr_val, func_val);
  }

  func_val = rul__val_create_function_str ("rul__sql_update", 3);

  rul__mol_use_printform (rul__ast_get_value (tab_node),
			  buffer, RUL_C_MAX_SYMBOL_SIZE);
  arg_val = rul__val_create_asciz_animal (buffer);
  rul__val_set_nth_subvalue (func_val, 1, arg_val);
  rul__val_set_nth_subvalue (func_val, 2,
			     rul__val_create_function_str ("rul__sql_rse", 0));
  rul__val_set_nth_subvalue (func_val, 3,
			     rul__val_create_blk_or_loc (CMP__E_INT_ENTRY_DATA,
							 "eb_data"));
  rul__val_set_next_value (curr_val, func_val);

  return ret_val;
}

Value rul__sem_check_sql_update_obj (Ast_Node ast)
{
  Ast_Node	node;
  Ast_Node_Type	node_type;
  Value		func_val, arg_val, curr_val, ret_val, wid_val;
  Boolean	quoted;

  /* SQL-UPDATE-FROM-OBJECT var opt-rse */

  /* get and verify element id binding */
  node = rul__ast_get_child(ast);
  if (rul__ast_get_type(node) != AST__E_VARIABLE) {
    display_check_error (CMP_INVUPDPAR, node);
    return NULL;
  }

  wid_val = rul__sem_check_rhs_var(node, SEM__C_VALUE_ACCESS);
  if (wid_val == NULL)
    return NULL;

  if (rul__val_get_domain(wid_val) != dom_instance_id) {
    display_check_error (CMP_INVUPDVAR, node);
    rul__val_free_value (wid_val);
    return NULL;
  }

  ret_val = curr_val = rul__val_create_function_str ("rul__sql_rse_init", 0);

  /* now check the opt-rse */
  node = rul__ast_get_sibling (node);
  if (node) {
    assert (rul__ast_get_type (node) == AST__E_SQL_RSE);
    
    for (node = rul__ast_get_child (node);
	 node != NULL;
	 node = rul__ast_get_sibling (node)) {
      
      quoted = FALSE;
      node_type = rul__ast_get_type (node);
      if (node_type == AST__E_CONSTANT)
	arg_val = rul__sem_check_rul_constant (node, SEM__C_RETURN_RUL_TYPE);
      else if (node_type == AST__E_VARIABLE)
	arg_val = rul__sem_check_rhs_var (node, SEM__C_VALUE_ACCESS);
      else if (node_type == AST__E_QUOTED_VAR) {
        arg_val = rul__sem_check_rhs_var (node, SEM__C_VALUE_ACCESS);
	quoted = TRUE;
      }
      else {
	display_check_error (CMP_INVRSEPAR, node);
	rul__val_free_value (ret_val);
	rul__val_free_value (wid_val);
	return NULL;
      }

      if (arg_val == NULL) {
	rul__val_free_value (ret_val);
	rul__val_free_value (wid_val);
	return NULL;
      }

      func_val = rul__val_create_function_str ("rul__sql_rse_symbol", 2);
      rul__val_set_nth_subvalue (func_val, 1, arg_val);
      if (quoted)
	rul__val_set_nth_subvalue (func_val, 2,
			       rul__val_create_long_animal (1));
      else
	rul__val_set_nth_subvalue (func_val, 2,
			       rul__val_create_long_animal (0));
      curr_val = rul__val_set_next_value (curr_val, func_val);
    }
  }

  /*
   * create the update function value
   * store the instance id (bound valiable) in the functions 1st par slot
   */
  func_val = rul__val_create_function_str ("rul__sql_update_from_wme", 3);
  rul__val_set_nth_subvalue (func_val, 1, wid_val);
  rul__val_set_nth_subvalue (func_val, 2,
			     rul__val_create_function_str ("rul__sql_rse", 0));
  rul__val_set_nth_subvalue (func_val, 3,
			     rul__val_create_blk_or_loc (CMP__E_INT_ENTRY_DATA,
							 "eb_data"));
  rul__val_set_next_value (curr_val, func_val);

  return ret_val;
}

static void display_check_error (char *err_code, Ast_Node node)
{
  Molecule   mol = NULL;
  Ast_Node   child_node;
  Ast_Node   parent_node;

  if (rul__ast_get_value_type (node) == AST__E_TYPE_MOL) {
    mol = rul__ast_get_value (node);
  }
  else if ((child_node = rul__ast_get_child (node)) != NULL) {
    if (rul__ast_get_value_type (child_node) == AST__E_TYPE_MOL)
      mol = rul__ast_get_value (child_node);
  }
  else if ((parent_node = rul__ast_get_parent (node)) != NULL) {
    if (rul__ast_get_value_type (parent_node) == AST__E_TYPE_MOL)
      mol = rul__ast_get_value (parent_node);
  }

  if (mol)
    rul__msg_cmp_print_w_atoms(err_code, node, 1, mol);
  else
    rul__msg_cmp_print (err_code, node, "");
  
  return;
}


