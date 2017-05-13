/****************************************************************************
**                                                                         **
**                     C M P _ S E M _ A C T I O N S. C                    **
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
 *	This module provides semantic checks for RHS actions.
 *
 * MODIFIED BY:
 *	DEC	Digital Equipment Corporation
 *	CPQ	Compaq Computer Corporation
 *
 * MODIFICATION HISTORY:
 *	15-Jan-1993	DEC	Initial version
 *
 *	 3-Jun-1994	DEC	Add message for missing RETURN
 *
 *	14-Jun-1994	DEC	Add $SUCCESS and $FAILURE for QUIT action
 *
 *	30-Aug-1994	DEC	Allow AFTERs to reference CATCHers not yet
 *					defined
 *
 *	16-Feb-1998	DEC	class type changed to rclass
 *	01-jul-1998	DEC	calls to rul__sem_check_rhs_action now have
 *					2nd parameter flag indicating whether in a loop
 *					(1) or not (0).
 *					rul__sem_check_bind now has 2nd parameter flag 
 *					indicating whether in a loop (1) or not (0).
 *					if (had_previous_binding) in rul__sem_check_bind
 *					is now if (had_previous_binding || in_loop)
 *	10-Aug-1998	DEC	MIU added when binding from variable 
 *					(rul__sem_check_bind)
 *
 *	01-Dec-1999	CPQ	Release with GPL
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
#include <gen.h>	/* for rul__gen_get_cur_ret_val_name */
#include <lvar.h>


static void display_check_error (char *err, Ast_Node node);
static Value rul__sem_check_all_attr_values (Ast_Node first_ast,
					     Class class_id,
					     Value object_val,
					     Value next_val);
static Value sem_check_method_return (Ast_Node ast, Ast_Node meth_node);




Value
rul__sem_check_addstate (Ast_Node ast)
{
  Ast_Node      node;
  Ast_Node_Type node_type;
  Value         func_val, arg_val;

  /* addstate filespec */

  node = rul__ast_get_child(ast);
  node_type = rul__ast_get_type(node);
  if (node_type != AST__E_CONSTANT &&
      node_type != AST__E_VARIABLE) {
    display_check_error (CMP_INVADDPAR, node);
    return NULL;
  }

  func_val = rul__val_create_function_str ("rul__addstate", 2);

  if (node_type == AST__E_CONSTANT)
    arg_val = rul__sem_check_rul_constant (node, SEM__C_RETURN_RUL_TYPE);
  else
    arg_val = rul__sem_check_rhs_var (node, SEM__C_VALUE_ACCESS);

  if (arg_val == NULL) {
    rul__val_free_value (func_val);
    return NULL;
  }

  rul__val_set_nth_subvalue (func_val, 1, arg_val);
  rul__val_set_nth_subvalue (func_val, 2,
			     rul__val_create_blk_or_loc (CMP__E_INT_ENTRY_DATA,
							 "eb_data"));

  return func_val;
}

Value
rul__sem_check_after (Ast_Node ast)
{
  Ast_Node      node;
  Value         func_val, arg_val;
  Molecule      mol;

  /* after cycles catcher */

  node = rul__ast_get_child(ast);

  /* check the cycle count */
  arg_val = rul__sem_scalar_val_to_ext (node,
			rul__sem_check_rhs_value (node,
						  SEM__C_RETURN_EXT_TYPE,
						  dom_int_atom, shape_atom,
						  NULL),
					SEM__C_ON_RHS, ext_type_uns_long);

  if (arg_val == NULL) {
    return NULL;
  }

  func_val = rul__val_create_function_str ("rul__rac_after", 4);
  rul__val_set_nth_subvalue (func_val, 1, arg_val);

  /* Do CATCHer checks */

  node = rul__ast_get_sibling(node);
  mol = rul__ast_get_value(node);

  if (rul__ast_get_type(node) != AST__E_CONSTANT) {
    display_check_error (CMP_INVCATSYM, node);
    rul__val_free_value (func_val);
    return NULL;
  }

  /* Remember that this catcher has been referenced, so we can be sure it
   * gets defined.
   */
  rul__sem_catcher_referenced(rul__ast_get_value(node));

  arg_val = rul__sem_check_rul_constant (node, SEM__C_RETURN_RUL_TYPE);

  if (arg_val == NULL) {
    rul__val_free_value (func_val);
    return NULL;
  }

  rul__val_set_nth_subvalue (func_val, 2, arg_val);
  rul__val_set_nth_subvalue (func_val, 3, rul__val_create_rul_constant (
					rul__conrg_get_cur_block_name()));
  rul__val_set_nth_subvalue(func_val, 4,
			    rul__val_create_blk_or_loc (CMP__E_INT_ENTRY_DATA,
							"eb_data"));

  return func_val;
}

Value
rul__sem_check_at (Ast_Node ast)
{
  Ast_Node      node;
  Ast_Node_Type node_type;
  Value         func_val, arg_val;

  /* @ filespec */

  node = rul__ast_get_child(ast);
  node_type = rul__ast_get_type(node);
  if (node_type != AST__E_CONSTANT &&
      node_type != AST__E_VARIABLE) {
    display_check_error (CMP_INVATPAR, node);
    return NULL;
  }

  /* create value for file id */

  if (node_type == AST__E_CONSTANT)
    arg_val = rul__sem_check_rul_constant (node, SEM__C_RETURN_RUL_TYPE);
  else
    arg_val = rul__sem_check_rhs_var (node, SEM__C_VALUE_ACCESS);

  if (arg_val == NULL) {
    return NULL;
  }

  func_val = rul__val_create_function_str ("rul__at", 1);
  rul__val_set_nth_subvalue (func_val, 1, arg_val);

  return func_val;
}


Value
rul__sem_check_bind (Ast_Node ast, Boolean in_loop)
{
    Ast_Node    node;
    Mol_Symbol	var_name;
    Boolean	had_previous_binding = FALSE;
    Value       decr_func_val, arg_val, rhs_var_val, tmp_var_val, assn_tmp_val;
    Value	last_val = NULL, func_val;

    /*  Found:	bind <var> value-expression */

    node = rul__ast_get_child (ast);
    var_name = (Mol_Symbol) rul__ast_get_value (node);

    /*  Verify that the first argument is a valid variable  */
    if (rul__ast_get_type (node) != AST__E_VARIABLE) {
	display_check_error (CMP_INVBINPAR, node);
	return NULL;
    }

    if (rul__sem_is_var_registered (var_name))
      had_previous_binding = rul__sem_rhs_var_is_bound (var_name);
    else 
      had_previous_binding = (rul__lvar_get_binding_count (var_name) != 0);
      
    if (had_previous_binding)
      rhs_var_val = rul__sem_check_rhs_var (node, SEM__C_VALUE_ACCESS);
    else
      rhs_var_val = rul__sem_check_rhs_var (node, SEM__C_VALUE_ASSIGNMENT);

    if (rhs_var_val == NULL)
      return NULL;

    if (had_previous_binding || in_loop) {
	/*
	**  If this variable had a previous binding, we need to be sure to
	**  decrement the reference count on the old binding.  However, since
	**  the value expression may contain references to that old value
	**  we can't do the decrement until after the actual expression is
	**  evaluated, and first we need to cache the old value.
	*/
	tmp_var_val = rul__sem_create_temp_var (SEM__C_ON_RHS);
	assn_tmp_val = rul__val_create_assignment (tmp_var_val,
					   rul__val_copy_value (rhs_var_val));
    }

    /*
    **  Get the value expression the variable is to be bound to
    */
    arg_val = rul__sem_check_nested_value (rul__ast_get_sibling(node),
					SEM__C_ON_RHS,
					SEM__C_UNDEFINED_CE_INDEX,
				        SEM__C_RETURN_RUL_TYPE,
				        dom_any, shape_molecule, rhs_var_val);
    if (arg_val == NULL) return NULL;

// SB: 10-Aug-1998 MIU missing when binding from varaible.

    if (rul__ast_get_type (rul__ast_get_sibling(node)) == AST__E_VARIABLE)
    {
	last_val = arg_val;
	while (rul__val_get_next_value (last_val) != NULL) {
	    last_val = rul__val_get_next_value (last_val);
	}

#ifndef NDEBUG
	func_val = rul__val_create_function_str ("rul__mol_incr_uses", 1);
#else
	func_val = rul__val_create_function_str ("MIU", 1);
#endif
	rul__val_set_nth_subvalue (func_val, 1, 
			       rul__val_copy_value (rhs_var_val));
	rul__val_set_next_value (last_val, func_val);
    }

    if (had_previous_binding || in_loop) {
	/*  Attach the caching of the old value  */
	rul__val_set_prev_value (arg_val, assn_tmp_val);

	/*  Create and attach the decrement of the cached value, and the
	**  reset of it's value to NULL.  It must be attached after all
	**  of the value expression fragments.
	*/
	last_val = arg_val;
	while (rul__val_get_next_value (last_val) != NULL) {
	    last_val = rul__val_get_next_value (last_val);
	}
	/*  First the decr_uses  */
#ifndef NDEBUG
	decr_func_val = rul__val_create_function_str ("rul__mol_decr_uses", 1);
#else
	decr_func_val = rul__val_create_function_str ("MDU", 1);
#endif
	rul__val_set_nth_subvalue (decr_func_val, 1, 
		rul__val_copy_value (tmp_var_val));
	rul__val_set_next_value (last_val, decr_func_val);

	/*  And then the set to NULL  */
	rul__val_set_next_value (
			decr_func_val,
			rul__val_create_assignment (
				rul__val_copy_value (tmp_var_val),
				rul__val_create_null (CMP__E_INT_MOLECULE)));
    }

    rul__sem_reset_var_binding_site (var_name, arg_val);
    return arg_val;
}



Value
rul__sem_check_build (Ast_Node ast)
{

  /* build (rule...) */

  rul__msg_cmp_print(CMP_NOBUILD, ast);

  return NULL;
}

Value
rul__sem_check_closefile (Ast_Node ast)
{
  Ast_Node      node;
  Ast_Node_Type node_type;
  Value         func_val, arg_val;
  long          arg_cnt = rul__ast_get_child_count(ast);
  long          i;

  /* closefile file-ids (constant|variable) ... */

  func_val = rul__val_create_function_str ("rul__ios_close_files", arg_cnt+1);

  /* make a Value from the arg count */
  arg_val = rul__val_create_long_animal(arg_cnt);

  /* set arg count as first parameter to make_comp function */
  rul__val_set_nth_subvalue (func_val, 1, arg_val);

  for (i = 2, node = rul__ast_get_child(ast), arg_cnt += 2;
       i < arg_cnt;
       i++, node = rul__ast_get_sibling(node)) {
    
    node_type = rul__ast_get_type(node);
    if (node_type != AST__E_CONSTANT &&
	node_type != AST__E_VARIABLE) {
      display_check_error (CMP_INVCLOPAR, node);
      rul__val_free_value (func_val);
      return NULL;
    }
    else {

      if (node_type == AST__E_CONSTANT)
	arg_val = rul__sem_check_rul_constant (node, SEM__C_RETURN_RUL_TYPE);
      else
	arg_val = rul__sem_check_rhs_var (node, SEM__C_VALUE_ACCESS);

      if (arg_val == NULL) {
	rul__val_free_value (func_val);
	return NULL;
      }
      rul__val_set_nth_subvalue (func_val, i, arg_val);
    }
  }

  return func_val;
}

Value
rul__sem_check_copy (Ast_Node ast, 
		     Boolean return_val_required,
		     Value return_to_val)
{
  Ast_Node      node;
  Value         inst_val,	/* instance id value */
		func_val,	/* current function value */
		first_val,	/* first assignment */
		obj_var,	/* rul__wm_copy_id return value */
                curr_val;	/* handle to last in sequence of set attrs */
  Class	        class_id = NULL;

  /* copy <$id-var> {^attr val-exp}...
   *
   * generated code will look like:
   *
   * obj_var = rul__wm_copy_id(inst_val);
   * rul__wm_set_attr_val(obj_var, mol[attr], value); <-- for each attr
   * rul__wm_update_and_notify(obj_var);
   * return (rul__wm_get_instance_id(obj_var);
   */

  /* get and verify element id binding */
  node = rul__ast_get_child(ast);
  if (rul__ast_get_type(node) != AST__E_VARIABLE) {
    display_check_error (CMP_INVCOPPAR, node);
    return NULL;
  }

  /* 1st parameter must be the <$ID> variable */
  inst_val = rul__sem_check_rhs_var(node, SEM__C_VALUE_ACCESS);

  if (inst_val == NULL)
    return NULL;

  if (rul__val_get_domain(inst_val) == dom_instance_id) {
    class_id = rul__val_get_class(inst_val);
    if (class_id == NULL)
      rul__msg_cmp_print (CMP_IDARGNOCLASS, node, CMP_C_MSG_COPY);
  }

  else if (rul__val_get_domain(inst_val) != dom_any) {
    display_check_error (CMP_INVCOPVAR, node);
    rul__val_free_value (inst_val);
    return NULL;
  }

  else
    rul__msg_cmp_print (CMP_IDARGNOVER, node, CMP_C_MSG_COPY);

  /*
   * create the copy function value
   * store the instance id (bound variable) in the functions 1st par slot
   */
#ifndef NDEBUG
  func_val = rul__val_create_function_str ("rul__wm_copy_id", 2);
#else
  func_val = rul__val_create_function_str ("WCOI", 2);
#endif
  rul__val_set_nth_subvalue (func_val, 1, inst_val);
  rul__val_set_nth_subvalue (func_val, 2,
			     rul__val_create_blk_or_loc (CMP__E_INT_ENTRY_DATA,
							 "eb_data"));

  /* create the object and assignment values */
  obj_var =
    rul__val_create_unnamed_ext_var(rul__sem_register_ext_temp(ext_type_object,
							       EXT__C_NOT_ARRAY
							       ,ext_mech_value,
							       SEM__C_ON_RHS),
				    ext_type_object, EXT__C_NOT_ARRAY,
				    ext_mech_value,SEM__C_NO_MEMORY_ALLOCATED);
  first_val = rul__val_create_assignment (obj_var, func_val);

  /* do all the attribute value pairs... */
  curr_val = rul__sem_check_all_attr_values (node, class_id,
					     obj_var, first_val);
  if (curr_val == NULL) {
    rul__val_free_value (first_val);
    return NULL;
  }

  /* all done with attribue values - do the update and notify */
#ifndef NDEBUG
  func_val = rul__val_create_function_str ("rul__wm_update_and_notify", 2);
#else
  func_val = rul__val_create_function_str ("WUN", 2);
#endif
  rul__val_set_nth_subvalue (func_val, 1, rul__val_copy_value(obj_var));
  rul__val_set_nth_subvalue (func_val, 2,
			     rul__val_create_blk_or_loc (CMP__E_INT_ENTRY_DATA,
							 "eb_data"));
  curr_val = rul__val_set_prev_value(func_val, curr_val);

  if (return_val_required) {
#ifndef NDEBUG
    func_val = rul__val_create_function_str ("rul__wm_get_instance_id", 1);
#else
    func_val = rul__val_create_function_str ("WGII", 1);
#endif
    rul__val_set_nth_subvalue (func_val, 1, rul__val_copy_value(obj_var));
    rul__val_set_prev_value(func_val, curr_val);
    return (rul__sem_return_value(return_to_val,
				  func_val,
				  shape_atom,
				  dom_instance_id,
				  SEM__C_ON_RHS,
				  class_id));
  }
  else {
    return curr_val;
  }
}

static Value
rul__sem_check_all_attr_values (Ast_Node first_ast,
				Class class_id,
				Value objvar,
				Value curr_val)
{
  Ast_Node	act_node;
  Value		ret_val, returned_val;
  Boolean	someone_failed = FALSE;

  ret_val = curr_val;

  act_node = rul__ast_get_sibling(first_ast);
  while (act_node != NULL) {

    /* node type must be an AST__E_RHS_ACTION_TERM */
    assert (rul__ast_get_type(act_node) == AST__E_RHS_ACTION_TERM);

    returned_val = rul__sem_check_attr_value (act_node, class_id,
					      objvar, curr_val);
    if (returned_val == NULL) {
      someone_failed = TRUE;
    }
    else {
      curr_val = rul__val_set_next_value (curr_val, returned_val);
    }

    act_node = rul__ast_get_sibling(act_node);
  }

  if (someone_failed)
    return NULL;

  return ret_val;
}


Value
rul__sem_check_attr_value (Ast_Node act_node, Class class_id,
			   Value objvar, Value curr_val)
{
  Ast_Node      node, attr_node, idx_node;
  Ast_Node_Type node_type;
  Value         func_val, arg_val, attoff_val;
  Value         fill_val, idx_val = NULL, idx_arg, new_val;
  Value         comp_val_1, comp_val_2, set_func_val, ret_val;
  Molecule      attr_name = NULL, index;
  Decl_Shape    attr_shape;
  Decl_Domain	attr_domain;
  Class		attr_class, val_class;
  Boolean       use_offset = FALSE;

  /*
   * 1st node is 1st RHS_ACTION_TERM
   * create a function value
   *  1st par - a copy of the returned object value
   *  2nd par - the attribute - could be a constant or a bound variable
   *  3rd par - the passed value
   */

  /* node type must be an AST__E_RHS_ACTION_TERM */
  assert (rul__ast_get_type(act_node) == AST__E_RHS_ACTION_TERM);

  /* for now the attribute must be a constant or a bound variable */
  attr_node = rul__ast_get_child(act_node);
  node_type = rul__ast_get_type(attr_node);
  if (node_type != AST__E_CONSTANT &&
      node_type != AST__E_VARIABLE) {
    display_check_error (CMP_INVATTSPE, attr_node);
    return NULL;
  }

  if (node_type == AST__E_CONSTANT) {
    
    attr_name = rul__ast_get_value(attr_node);
    /* verify atts are not system attributes */
    if (attr_name == rul__mol_symbol_id ()) {
      rul__msg_cmp_print (CMP_NOMODIFYID, attr_node);
      return (NULL);
    } else if (attr_name == rul__mol_symbol_instance_of ()) {
      rul__msg_cmp_print (CMP_NOMODIFYCLASS, attr_node);
      return (NULL);
    }

    /* verify atts belong to class if class_id != null */
    if (class_id) {
      if (!rul__decl_is_attr_in_class(class_id, attr_name)) {
        rul__msg_cmp_print_w_atoms(CMP_INVATTCLS, attr_node, 2, attr_name,
                                   rul__decl_get_class_name(class_id));
        return NULL;
      }
      use_offset = TRUE;
    }

    if (use_offset) {
      attoff_val = rul__val_create_long_animal(
                      rul__decl_get_attr_offset(class_id, attr_name));
      attr_shape = rul__decl_get_attr_shape (class_id, attr_name);
      attr_domain = rul__decl_get_attr_domain (class_id, attr_name);
      attr_class = rul__decl_get_attr_class (class_id, attr_name);
    }

    else {
      attr_shape = shape_molecule;
      attr_domain = dom_any;
      attr_class = NULL;
      attoff_val = rul__sem_check_rul_constant (attr_node, SEM__C_RETURN_RUL_TYPE);
    }
  }
  else {
    attr_shape = shape_molecule;
    attr_domain = dom_any;
    attr_class = NULL;
    attoff_val = rul__sem_check_rhs_var (attr_node, SEM__C_VALUE_ACCESS);
  }

  if (attoff_val == NULL) return NULL;
  

  /* check for a compound attribute index */
  idx_node = rul__ast_get_child (attr_node);
  if (idx_node != NULL) {

    assert (rul__ast_get_type(idx_node) == AST__E_ATTR_INDEX);

    /* get the index value */
    idx_node = rul__ast_get_child (idx_node);
    
    if (rul__ast_get_type (idx_node) == AST__E_CONSTANT) {
      index = rul__ast_get_value (idx_node);
      if (!rul__mol_is_int_atom (index)) {
        rul__msg_cmp_print_w_atoms (CMP_INVCMPIDX, idx_node, 1, index);
        rul__val_free_value (attoff_val);
        return NULL;
      }
      if (rul__mol_int_atom_value (index) < 1) {
        rul__msg_cmp_print_w_atoms (CMP_INVCMPIDX, idx_node, 1, index);
        rul__val_free_value (attoff_val);
        return NULL;
      }
      idx_val = rul__sem_check_rhs_value (idx_node, SEM__C_RETURN_EXT_TYPE,
                                          dom_int_atom, shape_atom, NULL);
      if (idx_val == NULL) {
        rul__val_free_value (attoff_val);
        return NULL;
      }
    }

    else if (rul__ast_get_type(idx_node) != AST__E_DOLLAR_LAST) {
      idx_arg = rul__sem_check_nested_value (idx_node, 
				SEM__C_ON_RHS, SEM__C_UNDEFINED_CE_INDEX,
				SEM__C_RETURN_RUL_TYPE,
                                dom_int_atom, shape_atom, NULL);
      if (idx_arg == NULL) {
        rul__val_free_value (attoff_val);
        return NULL;
      }
      idx_val = rul__val_create_function_str ("rul__cvt_s_in_s_lo", 1);
      rul__val_set_nth_subvalue (idx_val, 1, idx_arg);
    }

    /* get the new value, shape must be atom */
    node = rul__ast_get_sibling(attr_node);
    new_val = rul__sem_check_nested_value (node, 
				SEM__C_ON_RHS, SEM__C_UNDEFINED_CE_INDEX,
				SEM__C_RETURN_RUL_TYPE,
                                attr_domain, shape_atom, NULL);
    if (new_val == NULL) {
      rul__val_free_value (attoff_val);
      rul__val_free_value (idx_val);
      return NULL;
    }

    if (use_offset) { /* constants used */
      
      if (attr_shape != shape_compound) {
        rul__msg_cmp_print_w_atoms (CMP_NOINDEXSCAL, idx_node, 1, attr_name);
        rul__val_free_value (attoff_val);
        rul__val_free_value (idx_val);
        return NULL;
      }
      
      /* modifying ^compound [x] */
      /* arg_val =>   */
      /*   comp_val_1 = rul__wm_get_offset_val (objvar, attr_offset) */
      /*   rul__mol_incr_uses (comp_val_1) */
      
#ifndef NDEBUG
      func_val = rul__val_create_function_str ("rul__wm_get_offset_val", 2);
#else
      func_val = rul__val_create_function_str ("WGOV", 2);
#endif
      rul__val_set_nth_subvalue (func_val, 1,
                                 rul__val_copy_value (objvar));
      rul__val_set_nth_subvalue (func_val, 2,
                                 rul__val_copy_value (attoff_val));
      comp_val_1 = rul__val_create_unnamed_mol_var (
                                    rul__sem_register_temp_var (SEM__C_ON_RHS));
      arg_val = rul__val_create_assignment (comp_val_1, func_val);
#ifndef NDEBUG
      func_val = rul__val_create_function_str ("rul__mol_incr_uses", 1);
#else
      func_val = rul__val_create_function_str ("MIU", 1);
#endif
      rul__val_set_nth_subvalue (func_val, 1, 
                                 rul__val_copy_value (comp_val_1));
      func_val = rul__val_set_next_value (arg_val, func_val);


      /* in case of ^comp[$LAST] .... */
      if (rul__ast_get_type(idx_node) == AST__E_DOLLAR_LAST) {
        idx_val = rul__val_create_function_str (
#ifndef NDEBUG
                                        "rul__mol_get_poly_count_last", 1);
#else
                                        "MGPCL", 1);
#endif
        rul__val_set_nth_subvalue (idx_val, 1,
                                   rul__val_copy_value (comp_val_1));
      }


      /* fill_val = rul__decl_get_attr_fill (rclass, attr_name) */
      fill_val = rul__val_create_function_str("rul__decl_get_attr_fill", 2);
      rul__val_set_nth_subvalue (fill_val, 1,
                                 rul__val_create_class_constant (class_id));
      rul__val_set_nth_subvalue (fill_val, 2,
                                 rul__sem_check_rul_constant (attr_node,
                                                           SEM__C_RETURN_RUL_TYPE));


      /* set_func_val => */
      /* comp_val_2 = rul__mol_set_comp_nth (comp_val, idx, value, fill) */
      comp_val_2 = rul__val_create_unnamed_mol_var (
                                   rul__sem_register_temp_var (SEM__C_ON_RHS));

#ifndef NDEBUG
      set_func_val = rul__val_create_function_str("rul__mol_set_comp_nth", 4);
#else
      set_func_val = rul__val_create_function_str("MSCN", 4);
#endif
      rul__val_set_nth_subvalue (set_func_val, 1, arg_val);
      rul__val_set_nth_subvalue (set_func_val, 2,
                                 rul__val_copy_value (idx_val));
      rul__val_set_nth_subvalue (set_func_val, 3,
                                 new_val);
      rul__val_set_nth_subvalue (set_func_val, 4,
                                 fill_val);
      set_func_val = rul__val_create_assignment (comp_val_2, set_func_val);

#ifndef NDEBUG
      func_val = rul__val_create_function_str ("rul__wm_set_offset_val", 4);
#else
      func_val = rul__val_create_function_str ("WSOV", 4);
#endif
      rul__val_set_nth_subvalue (func_val, 1, rul__val_copy_value(objvar));
      rul__val_set_nth_subvalue (func_val, 2, attoff_val);
      rul__val_set_nth_subvalue (func_val, 3, set_func_val);
      rul__val_set_nth_subvalue (func_val, 4,
			 rul__val_create_blk_or_loc (CMP__E_INT_ENTRY_DATA,
						     "eb_data"));
    }

    else {
      /* not using offset, some non-constant used */
      /* all checks need to be done at runtime... */

      /* in case of ^comp[$LAST] .... */
      if (rul__ast_get_type(idx_node) != AST__E_DOLLAR_LAST) {
        func_val = rul__val_create_function_str (
#ifndef NDEBUG
					"rul__wm_set_comp_attr_val", 5);
#else
					"WSCAV", 5);
#endif
        rul__val_set_nth_subvalue (func_val, 1, rul__val_copy_value(objvar));
        rul__val_set_nth_subvalue (func_val, 2, attoff_val);
        rul__val_set_nth_subvalue (func_val, 3, new_val);
        rul__val_set_nth_subvalue (func_val, 4, idx_arg);
	rul__val_set_nth_subvalue (func_val, 5,
			 rul__val_create_blk_or_loc (CMP__E_INT_ENTRY_DATA,
						     "eb_data"));
	/* idx_arg was part of idx_val, so we should clean up idx_val here */
	rul__val_set_nth_subvalue (idx_val, 1, NULL);
	rul__val_free_value (idx_val);
      }
      else {
        func_val = rul__val_create_function_str(
#ifndef NDEBUG
					"rul__wm_set_comp_attr_last", 4);
#else
					"WSCAL", 4);
#endif
        rul__val_set_nth_subvalue (func_val, 1, rul__val_copy_value(objvar));
        rul__val_set_nth_subvalue (func_val, 2, attoff_val);
        rul__val_set_nth_subvalue (func_val, 3, new_val);
        rul__val_set_nth_subvalue (func_val, 4,
			 rul__val_create_blk_or_loc (CMP__E_INT_ENTRY_DATA,
						     "eb_data"));
      }
    }
  }
  else { /* non-indexed attribue */

    /* get the new value, match the known attr shape */
    node = rul__ast_get_sibling(attr_node);
    new_val = rul__sem_check_nested_value (node, 
				SEM__C_ON_RHS, SEM__C_UNDEFINED_CE_INDEX,
				SEM__C_RETURN_RUL_TYPE,
                                attr_domain, attr_shape, NULL);
    if (new_val == NULL) {
      rul__val_free_value (attoff_val);
      return NULL;
    }

    if (use_offset) { /* known valid values */
      
      if ((attr_shape != shape_molecule &&
	   rul__val_get_shape (new_val) != shape_molecule &&
	   attr_shape == rul__val_get_shape (new_val)) &&
	  ((attr_domain == dom_any) ||
	   (attr_domain == rul__val_get_domain (new_val))))
	func_val = rul__val_create_function_str (
#ifndef NDEBUG
					 "rul__wm_set_offset_val_simple", 4);
#else
					 "WSOVS", 4);
#endif
      else
#ifndef NDEBUG
	func_val = rul__val_create_function_str ("rul__wm_set_offset_val", 4);
#else
	func_val = rul__val_create_function_str ("WSOV", 4);
#endif

      rul__val_set_nth_subvalue (func_val, 1, rul__val_copy_value(objvar));
      rul__val_set_nth_subvalue (func_val, 2, attoff_val);
      rul__val_set_nth_subvalue (func_val, 3, new_val);
      rul__val_set_nth_subvalue (func_val, 4,
			 rul__val_create_blk_or_loc (CMP__E_INT_ENTRY_DATA,
						     "eb_data"));
    }
    else { /* attribute is a variable */
      
#ifndef NDEBUG
      func_val = rul__val_create_function_str ("rul__wm_set_var_attr_val", 4);
#else
      func_val = rul__val_create_function_str ("WSVAV", 4);
#endif
      rul__val_set_nth_subvalue (func_val, 1, rul__val_copy_value(objvar));
      rul__val_set_nth_subvalue (func_val, 2, attoff_val);
      rul__val_set_nth_subvalue (func_val, 3, new_val);
      rul__val_set_nth_subvalue (func_val, 4,
			 rul__val_create_blk_or_loc (CMP__E_INT_ENTRY_DATA,
						     "eb_data"));
    }
  }

  if (attr_domain == dom_instance_id  &&
      attr_class != NULL  &&
      rul__val_get_domain (new_val) == dom_instance_id  &&
      (val_class = rul__val_get_class (new_val)) != NULL) {
      
    if (!rul__decl_is_subclass_of (val_class, attr_class)) {
      rul__val_free_value (func_val);
      rul__msg_cmp_print_w_atoms (CMP_INVVALCLASS, act_node, 2, attr_name,
				  rul__decl_get_class_name (attr_class));
      return NULL;
    }
  }
  return func_val;
}


Value
rul__sem_check_debug (Ast_Node ast)
{

  /* DEBUG */

  return rul__val_create_function_str ("rul__debug", 0);
}

Value
rul__sem_check_default (Ast_Node ast)
{
  Ast_Node      node;
  Ast_Node_Type node_type;
  Value         func_val, arg_val;
  Molecule      mol;

  /* default file_id|NIL ACCEPT|TRACE|WRITE */

  /* get the file identifier */

  node = rul__ast_get_child(ast);
  node_type = rul__ast_get_type(node);
  if (node_type != AST__E_CONSTANT &&
      node_type != AST__E_VARIABLE) {
    display_check_error (CMP_INVDEFFID, node);
    return NULL;
  }

  /* create value for file id */

  if (node_type == AST__E_CONSTANT)
    arg_val = rul__sem_check_rul_constant (node, SEM__C_RETURN_RUL_TYPE);
  else
    arg_val = rul__sem_check_rhs_var (node, SEM__C_VALUE_ACCESS);

  if (arg_val == NULL) {
    return NULL;
  }

  func_val = rul__val_create_function_str ("rul__ios_set_default_rt", 2);
  rul__val_set_nth_subvalue (func_val, 1, arg_val);

  /* get the open mode keyword */

  node = rul__ast_get_sibling(node);
  node_type = rul__ast_get_type(node);
  if (node_type != AST__E_CONSTANT &&
      node_type != AST__E_VARIABLE) {
    display_check_error (CMP_INVDEFKTP, node);
    rul__val_free_value (func_val);
    return NULL;
  }

  /* can mode be a variable ??? */

  if (node_type == AST__E_CONSTANT) {
    /* check for a symbol and eq = ( ACCEPT | TRACE | WRITE ) */
    mol = rul__ast_get_value(node);
    if (mol != rul__mol_make_symbol("ACCEPT") &&
	mol != rul__mol_make_symbol("TRACE") &&
	mol != rul__mol_make_symbol("WRITE")) {
      rul__msg_cmp_print_w_atoms(CMP_INVDEFKEY, node, 1, mol);
      rul__val_free_value (func_val);
      return NULL;
    }
    arg_val = rul__sem_check_rul_constant (node, SEM__C_RETURN_RUL_TYPE);
  }
  else {
    arg_val = rul__sem_check_rhs_var (node, SEM__C_VALUE_ACCESS);
  }
  
  if (arg_val == NULL) {
    rul__val_free_value (func_val);
    return NULL;
  }
  
  rul__val_set_nth_subvalue (func_val, 2, arg_val);
  
  return func_val;
}

Value
rul__sem_check_for_each (Ast_Node ast)
{
  Ast_Node      node, var1_node, var2_node, comp_node;
  Value         ret_val,	/* returned value */
                asn_val,
		asn2_val,	/* value assignments inside loop */
                cmplx_val,	/* holds predicate and operators */
                func_val,	/* temporay function usage */
                last_val,	/* used to find last in a chain */
                arg_val,        /* temporay value usage */
                rhs_var,        /* the loop assigned value */
                comp_val,       /* the IN compound value */
                comp_var,       /* the compound for-each variable */
                incr_var,       /* local incrementor variable */
                curr_val;	/* most resently used value */
  Mol_Symbol	var_name, comp_name;
  char          buf[RUL_C_MAX_SYMBOL_SIZE];
  static long   for_each_level = 0;
  
  /* for-each <var> in compound-val (rhs-actions...) */

  /*
   * This routine sets up 3 values that are handled in the emit code
   * for executing a while loop.
   * 1st - initialization assignments 
   * 2nd - comparitor values (a complex_value is used)
   * 3rd - rhs variable assignment
   */

  var1_node = rul__ast_get_child (ast);
  var_name = (Mol_Symbol) rul__ast_get_value (var1_node);
  if (rul__ast_get_type (var1_node) != AST__E_VARIABLE) {
    display_check_error (CMP_INVFORPAR, var1_node);
    return NULL;
  }

  rhs_var = rul__sem_check_rhs_var (var1_node, SEM__C_VALUE_ASSIGNMENT);
  if (rhs_var == NULL)
    return NULL;

  /*
   * Create a named variable for storing the compound data.
   * Need to use a named var because they are decr'd at the end of
   * the rule, tmp_mol's after each action.
   * use the variable name or make one for the compound value
   */
  var2_node = rul__ast_get_sibling (var1_node);

  /* make up a fake ast node for the variable */
  sprintf (buf, "rul_foreach_%ld", for_each_level++);
  comp_name = rul__mol_make_symbol (buf);
  comp_node = rul__ast_mol_node (AST__E_VARIABLE, comp_name);
  comp_var = rul__sem_check_rhs_var (comp_node, SEM__C_VALUE_ASSIGNMENT);
  rul__ast_free (comp_node);
  if (comp_var == NULL) {
    for_each_level -= 1;
    return NULL;
  }

  func_val = NULL;
  if (rul__sem_rhs_var_is_bound (comp_name)) {
#ifndef NDEBUG
    func_val = rul__val_create_function_str ("rul__mol_decr_uses", 1);
#else
    func_val = rul__val_create_function_str ("MDU", 1);
#endif
    rul__val_set_nth_subvalue (func_val, 1, 
			       rul__val_copy_value (comp_var));
  }

  /*
   * Now check the compound value , must be a compound.
   * Also, assign it to the above named variable 
   */
  comp_val = rul__sem_check_nested_value (var2_node, SEM__C_ON_RHS, 
					  SEM__C_UNDEFINED_CE_INDEX,
					  SEM__C_RETURN_RUL_TYPE,
					  dom_any, shape_compound,
					  comp_var);
  if (comp_val == NULL) {
    for_each_level -= 1;
    return NULL;
  }

  /*
   * decr rul_for_each if previously bound (multiple for-eachs)
   */
  if (func_val)
    rul__val_set_prev_value (comp_val, func_val);

  rul__sem_reset_var_binding_site (comp_name, comp_val);

  if (rul__ast_get_type (var2_node) == AST__E_VARIABLE) {
#ifndef NDEBUG
    func_val = rul__val_create_function_str ("rul__mol_incr_uses", 1);
#else
    func_val = rul__val_create_function_str ("MIU", 1);
#endif
    rul__val_set_nth_subvalue (func_val, 1, 
			       rul__val_copy_value (comp_var));
    last_val = comp_val;
    while (rul__val_get_next_value (last_val) != NULL) {
      last_val = rul__val_get_next_value (last_val);
    }
    rul__val_set_next_value (last_val, func_val);
  }

  /*
   * create the incrementor and assign to 1 ( i = 1; )
   */
  incr_var = rul__val_create_unnamed_ext_var (
			      rul__sem_register_ext_temp (ext_type_for_each,
							  EXT__C_NOT_ARRAY,
							  ext_mech_value,
							  SEM__C_ON_RHS),
					      ext_type_for_each,
					      EXT__C_NOT_ARRAY,
					      ext_mech_value,
					      SEM__C_NO_MEMORY_ALLOCATED);
  curr_val = rul__val_create_assignment (incr_var,
					 rul__val_create_long_animal (1));

  /*
   * create the assignment for the comparator (max = poly_count (comp_val))
   */
#ifndef NDEBUG
  func_val = rul__val_create_function_str ("rul__mol_get_poly_count", 1);
#else
  func_val = rul__val_create_function_str ("MGPC", 1);
#endif
  rul__val_set_nth_subvalue (func_val, 1, comp_val);
  arg_val = rul__val_create_unnamed_ext_var (
			     rul__sem_register_ext_temp (ext_type_for_each,
							 EXT__C_NOT_ARRAY,
							 ext_mech_value,
							 SEM__C_ON_RHS),
					     ext_type_for_each,
					     EXT__C_NOT_ARRAY,
					     ext_mech_value,
					     SEM__C_NO_MEMORY_ALLOCATED);
  ret_val = rul__val_create_assignment (arg_val, func_val);
  curr_val = rul__val_set_prev_value (ret_val, curr_val);


  /*
   * define the while operators, oper oper1 oper2
   * while (incr_var <= arg_val)
   */
  cmplx_val = rul__val_create_complex (3);
  rul__val_set_nth_subvalue (cmplx_val, 1,
		     rul__val_create_long_animal ((long) EMIT__E_REL_LTE));
  rul__val_set_nth_subvalue (cmplx_val, 2, rul__val_copy_value (incr_var));
  rul__val_set_nth_subvalue (cmplx_val, 3, rul__val_copy_value (arg_val));
  rul__ast_attach_value (var1_node, AST__E_TYPE_VALUE, cmplx_val);

  /*
   * create the function/assignment(s) for setting the rhs_var
   * 1st, decrement the usage count on rhs_var
   */
#ifndef NDEBUG
  asn_val = rul__val_create_function_str ("rul__mol_decr_uses", 1);
#else
  asn_val = rul__val_create_function_str ("MDU", 1);
#endif
  rul__val_set_nth_subvalue (asn_val, 1, rhs_var);

  /* 2nd, make assignment -- rhs_var = comp_var[i] */
#ifndef NDEBUG
  func_val = rul__val_create_function_str ("rul__mol_get_comp_nth", 2);
#else
  func_val = rul__val_create_function_str ("MGCN", 2);
#endif
  rul__val_set_nth_subvalue (func_val, 1, rul__val_copy_value (comp_var));
  rul__val_set_nth_subvalue (func_val, 2, rul__val_copy_value (incr_var));
  rul__val_set_shape_and_domain (func_val, shape_atom,
				 rul__val_get_domain (comp_var));
  rul__val_set_class (func_val, rul__val_get_class (comp_var));

  asn2_val = rul__val_create_assignment (rul__val_copy_value (rhs_var),
					 func_val);
  curr_val = rul__val_set_next_value (asn_val, asn2_val);
  rul__sem_reset_var_binding_site (var_name, asn2_val);

  /* 3rd, increment the usage count on rhs_var after assignment */
#ifndef NDEBUG
  func_val = rul__val_create_function_str ("rul__mol_incr_uses", 1);
#else
  func_val = rul__val_create_function_str ("MIU", 1);
#endif
  rul__val_set_nth_subvalue (func_val, 1, rul__val_copy_value (rhs_var));
  curr_val = rul__val_set_next_value (curr_val, func_val);
  
  /*
   * i = i + 1
   */
  func_val = rul__val_create_arith_expr (CMP__E_ARITH_PLUS,
					 rul__val_copy_value(incr_var),
					 rul__val_create_long_animal (1));
  curr_val = rul__val_set_next_value (curr_val,
		      rul__val_create_assignment (
				  rul__val_copy_value(incr_var), func_val));
  rul__ast_attach_value (var2_node, AST__E_TYPE_VALUE, asn_val);

  /*
   * end rhs tmp mol usage...
   */
  rul__sem_rhs_action_end_vars ();

  /*
   * loop for each for-each action
   */
  node = rul__ast_get_sibling (var2_node);
  while (node != NULL) {

    arg_val = rul__sem_check_rhs_action (node, 1);
    if (arg_val == NULL) {
      rul__val_free_value (ret_val);
      for_each_level -= 1;
      return NULL;
    }

    else {
      rul__ast_attach_value (node, AST__E_TYPE_VALUE, arg_val);

      /* get next, end_action_vars if more. calling routine will
	 end_action_vars if no more */
      node = rul__ast_get_sibling(node);
      if (node != NULL)
	rul__sem_rhs_action_end_vars ();
    }
  }

  for_each_level -= 1;
  return ret_val;
}




static Value
sem_build_cmplx_exprs (Ast_Node node, Value *ret)
{
  Ast_Node       v1_node, v2_node, pred_node, opt_node, par_node;
  Ast_Node_Type  node_type;
  Value          v1_val = NULL, v2_val, cond_val, last, next, opr1, opr2;
  Pred_Type      pred, opt_pred, tmp_pred;
  Cond_Oper_Type cond_opr;

  node_type = rul__ast_get_type (node);

  if (node_type == AST__E_COND_EXPR) {

    /* get the individual cond-expr nodes... */
    v1_node = rul__ast_get_child (node);
    pred_node = rul__ast_get_sibling (v1_node);
    v2_node = rul__ast_get_sibling (pred_node);

    v1_val = rul__sem_check_nested_value (v1_node, SEM__C_ON_RHS,
					  SEM__C_UNDEFINED_CE_INDEX,
					  SEM__C_RETURN_RUL_TYPE,
					  dom_any, shape_molecule, NULL);
    if (v1_val == NULL)
      return FALSE;

    if (rul__val_is_assignment (v1_val)) {
      if (*ret) {
	last = rul__val_get_last_value (*ret);
	rul__val_set_next_value (last, v1_val);
      }
      else
	*ret = v1_val;

      v1_val = rul__val_copy_value (rul__val_get_assignment_to (v1_val));
    }

    v2_val = rul__sem_check_nested_value (v2_node, SEM__C_ON_RHS,
					  SEM__C_UNDEFINED_CE_INDEX,
					  SEM__C_RETURN_RUL_TYPE,
					  dom_any, shape_molecule, NULL);
    if (v2_val == NULL)
      return FALSE;
      
    if (rul__val_is_assignment (v2_val)) {
      if (*ret) {
	last = rul__val_get_last_value (*ret);
	rul__val_set_next_value (last, v2_val);
      }
      else
	*ret = v2_val;

      v2_val = rul__val_copy_value (rul__val_get_assignment_to (v2_val));
    }

    /* checks to assure the predicate can be used with the values */
    assert (rul__ast_get_type (pred_node) == AST__E_PRED_TEST);
    pred = (Pred_Type) rul__ast_get_long_value (pred_node);
    opt_node = rul__ast_get_child (pred_node);
    if (opt_node)
      opt_pred = (Pred_Type) rul__ast_get_long_value (opt_node);
    else
      opt_pred = PRED__E_NOT_A_PRED;

    if (! rul__sem_check_val_pred_val (node, pred, opt_pred, v1_val, v2_val)) {
      rul__val_free_value (v1_val);
      rul__val_free_value (v2_val);
      return NULL;
    }

    if (pred == PRED__E_CONTAINS ||
	pred == PRED__E_DOES_NOT_CONTAIN ||
	opt_pred == PRED__E_CONTAINS ||
	opt_pred == PRED__E_DOES_NOT_CONTAIN) {

      if (opt_pred == PRED__E_CONTAINS ||
	  opt_pred == PRED__E_DOES_NOT_CONTAIN) {

	tmp_pred = pred;
	pred = opt_pred;
	opt_pred = tmp_pred;
      }

      cond_val = rul__val_create_function_str (
				       rul__gen_pred_func_name (pred), 3);
      if (opt_pred == PRED__E_EQ || opt_pred == PRED__E_NOT_A_PRED)
	rul__val_set_nth_subvalue (cond_val, 3,
			   rul__val_create_blk_or_loc (CMP__E_INT_MOLECULE,
						       "NULL"));
      else
	rul__val_set_nth_subvalue (cond_val, 3,
			   rul__val_create_blk_or_loc (CMP__E_INT_MOLECULE,
					 rul__gen_pred_func_name (opt_pred)));
    }

    else {
      cond_val = rul__val_create_function_str (
				       rul__gen_pred_func_name (pred), 2);
    }
    rul__val_set_nth_subvalue (cond_val, 1, v1_val);
    rul__val_set_nth_subvalue (cond_val, 2, v2_val);
  }

  else {
    
    if (node_type == AST__E_OPR_AND)
      cond_opr = CMP__E_COND_AND;
    else if (node_type == AST__E_OPR_OR)
      cond_opr = CMP__E_COND_OR;
    else if (node_type == AST__E_OPR_NOT) {
      opr2 = NULL;
      cond_opr = CMP__E_COND_NOT;
    }
    else
      return NULL;

    node = rul__ast_get_child (node);
    opr1 = sem_build_cmplx_exprs (node, ret);
    if (opr1 == NULL)
      return NULL;

    if (cond_opr != CMP__E_COND_NOT) {
      opr2 = sem_build_cmplx_exprs (rul__ast_get_sibling (node), ret);
      if (opr2 == NULL)
	return NULL;
    }

    cond_val = rul__val_create_cond_expr (cond_opr, opr1, opr2);
  }

  return cond_val;
}



Value
rul__sem_check_if (Ast_Node ast)
{
  Ast_Node      node, act_node;
  Value         ret_val = NULL, arg_val;

  /* if expression
         then rhs-actions...
       [ else rhs-actions... ] */

  node = rul__ast_get_child (ast);
  arg_val = sem_build_cmplx_exprs (node, &ret_val);
  if (arg_val == NULL)
    return NULL;

  rul__ast_attach_value (node, AST__E_TYPE_VALUE, arg_val);

  /* now the actions... */
  node = rul__ast_get_sibling (node);
  act_node = rul__ast_get_child (node);

  if (act_node != NULL)
    rul__sem_rhs_action_end_vars ();

  while (act_node != NULL) {

    arg_val = rul__sem_check_rhs_action (act_node, 0);
    if (arg_val == NULL) {
      if (ret_val)
	rul__val_free_value (ret_val);
      return NULL;
    }

    else {

      rul__ast_attach_value (act_node, AST__E_TYPE_VALUE, arg_val);
      act_node = rul__ast_get_sibling (act_node);

      /*
       * get next, end_action_vars if more. calling routine will
       * end_action_vars if no more
       */
      if (act_node != NULL)
	rul__sem_rhs_action_end_vars ();
    }
  }


  /* Is there and ELSE? */
  node = rul__ast_get_sibling (node);
  if (node) {
    
    act_node = rul__ast_get_child (node);

    if (act_node != NULL)
      rul__sem_rhs_action_end_vars ();

    while (act_node != NULL) {

      arg_val = rul__sem_check_rhs_action (act_node, 0);
      if (arg_val == NULL) {
	rul__val_free_value (ret_val);
	return NULL;
      }
      
      else {
	
	rul__ast_attach_value (act_node, AST__E_TYPE_VALUE, arg_val);
	act_node = rul__ast_get_sibling (act_node);
	
	/*
	 * get next, end_action_vars if more. calling routine will
	 * end_action_vars if no more
	 */
	if (act_node != NULL)
	  rul__sem_rhs_action_end_vars ();
      }
    }
  }

  if (ret_val == NULL)
    return (Value) -1;

  return ret_val;
}


Value
rul__sem_check_make (Ast_Node ast,
		     Boolean return_val_required,
		     Value return_to_val)
{
  Ast_Node      node;
  Ast_Node_Type node_type;
  Value         obj_var,	/* rul__wm_create return value */
                class_val,      /* used when the class name is a var */
		func_val,	/* current function value */
		first_val,	/* first assignment */
                curr_val;	/* handle to last in sequence of set attrs */
  Class	        class_id = NULL;
  Molecule      class_name;
  char          buffer[RUL_C_MAX_SYMBOL_SIZE+1];
  long          blkidx;

  /* make class-name [{^attr value-expression}...]
   *
   * generated code will look like:
   *
   * obj_var = rul__wm_create(classes[x]);
   * rul__wm_set_attr_val(obj_var, mol[attr], value); <-- for each attr
   * rul__wm_update_and_notify(obj_var);
   *
   * return (rul__wm_get_instance_id(obj_var);
   *
   */

  /* get and verify constant class name */
  node = rul__ast_get_child(ast);
  node_type = rul__ast_get_type(node);
  if (node_type != AST__E_CONSTANT &&
      node_type != AST__E_VARIABLE &&
      node_type != AST__E_BTI_ACCEPT_ATOM &&
      node_type != AST__E_BTI_CONCAT &&
      node_type != AST__E_BTI_GET &&
      node_type != AST__E_BTI_NTH &&
      node_type != AST__E_BTI_SYMBOL &&
      node_type != AST__E_EXT_RT_CALL) {
    display_check_error (CMP_INVMAKPAR, node);
    return NULL;
  }

  if (node_type == AST__E_CONSTANT) {     /* is class defined? */
    class_name = rul__ast_get_value(node);
    if (class_name == rul__mol_symbol_root ()) {
      rul__msg_cmp_print_w_atoms(CMP_INVMAKCLS, node, 1, class_name);
      return NULL;
    }
    if (!(class_id = rul__decl_get_visible_class(class_name))) {
      rul__msg_cmp_print_w_atoms(CMP_INVCLSNAM, node, 1, class_name);
      return NULL;
    }
#ifndef NDEBUG
    func_val = rul__val_create_function_str ("rul__wm_create", 3);
#else
    func_val = rul__val_create_function_str ("WCR", 3);
#endif
    rul__val_set_nth_subvalue (func_val, 1,
			       rul__val_create_class_constant (class_id));
    rul__val_set_nth_subvalue (func_val, 2, 
	       rul__val_create_blk_or_loc (CMP__E_INT_MOLECULE, "NULL"));
    rul__val_set_nth_subvalue (func_val, 3,
		       rul__val_create_blk_or_loc (CMP__E_INT_ENTRY_DATA,
						   "eb_data"));
  }
  else {

    class_val = rul__sem_check_nested_value (node, 
					SEM__C_ON_RHS,
					SEM__C_UNDEFINED_CE_INDEX,
					SEM__C_RETURN_RUL_TYPE,
					dom_symbol, shape_atom, NULL);
    if (class_val == NULL)
      return NULL;

#ifndef NDEBUG
    func_val = rul__val_create_function_str ("rul__wm_create_var", 2);
#else
    func_val = rul__val_create_function_str ("WCRV", 2);
#endif
    rul__val_set_nth_subvalue (func_val, 1, class_val);
    rul__val_set_nth_subvalue (func_val, 2,
		       rul__val_create_blk_or_loc (CMP__E_INT_ENTRY_DATA,
						   "eb_data"));
  }
  
  /* create the wm and assign values */
  obj_var = rul__val_create_unnamed_ext_var (
		     rul__sem_register_ext_temp (ext_type_object,
						 EXT__C_NOT_ARRAY,
						 ext_mech_value,
						 SEM__C_ON_RHS),
			     ext_type_object, EXT__C_NOT_ARRAY,
			     ext_mech_value, SEM__C_NO_MEMORY_ALLOCATED);

  /* set up assignment to obj_var */
  first_val = rul__val_create_assignment (obj_var, func_val);

  /* do all the attribute value pairs... */
  curr_val = rul__sem_check_all_attr_values (node, class_id,
					     obj_var, first_val);
  if (curr_val == NULL) {
    rul__val_free_value (first_val);
    return NULL;
  }

  /* all done with attribue values - do the update and notify */
#ifndef NDEBUG
  func_val = rul__val_create_function_str ("rul__wm_update_and_notify", 2);
#else
  func_val = rul__val_create_function_str ("WUN", 2);
#endif
  rul__val_set_nth_subvalue (func_val, 1, rul__val_copy_value(obj_var));
  rul__val_set_nth_subvalue (func_val, 2,
			     rul__val_create_blk_or_loc (CMP__E_INT_ENTRY_DATA,
							 "eb_data"));
  curr_val = rul__val_set_prev_value(func_val, curr_val);

  if (return_val_required) {
#ifndef NDEBUG
    func_val = rul__val_create_function_str ("rul__wm_get_instance_id", 1);
#else
    func_val = rul__val_create_function_str ("WGII", 1);
#endif
    rul__val_set_nth_subvalue (func_val, 1, rul__val_copy_value(obj_var));
    rul__val_set_prev_value(func_val, curr_val);
    return (rul__sem_return_value(return_to_val,
				  func_val,
				  shape_atom,
				  dom_instance_id,
				  SEM__C_ON_RHS,
				  class_id));
  }
  else {
    return curr_val;
  }
}

Value
rul__sem_check_modify (Ast_Node ast,
		       Boolean return_val_required,
		       Value return_to_val)
{
  Ast_Node      node;
  Value         inst_val,	/* instance id value */
		func_val,	/* current function value */
		obj_var,	/* rul__wm_modify_id return value */
                first_val,	/* first in sequence of values */
                curr_val;	/* handle to last in sequence of set attrs */
  Class	        class_id = NULL;

  /* modify <$id-var> {^attr val-exp}... 
   *
   * generated code will look like:
   *
   * obj_var = rul__wm_modify_id(inst_val);
   * rul__wm_set_attr_val(obj_var, mol[attr], value); <-- for each attr
   * rul__wm_update_and_notify(obj_var);
   * return (rul__wm_get_instance_id(obj_var);
   */

  /* get and verify element id binding */
  node = rul__ast_get_child(ast);
  if (rul__ast_get_type(node) != AST__E_VARIABLE) {
    display_check_error (CMP_INVMODPAR, node);
    return NULL;
  }

  inst_val = rul__sem_check_rhs_var(node, SEM__C_VALUE_ACCESS);
  if (inst_val == NULL)
    return NULL;

  if (rul__val_get_domain(inst_val) == dom_instance_id) {
    class_id = rul__val_get_class(inst_val);
    if (class_id == NULL)
      rul__msg_cmp_print (CMP_IDARGNOCLASS, node, CMP_C_MSG_MODIFY);
  }

  else if (rul__val_get_domain(inst_val) != dom_any) {
    display_check_error (CMP_INVMODVAR, node);
    rul__val_free_value (inst_val);
    return NULL;
  }

  else
    rul__msg_cmp_print (CMP_IDARGNOVER, node, CMP_C_MSG_MODIFY);
 
  /*
   * create the modify function value
   * store the instance id (bound valiable) in the functions 1st par slot
   */
#ifndef NDEBUG
  func_val = rul__val_create_function_str ("rul__wm_modify_id", 2);
#else
  func_val = rul__val_create_function_str ("WMOI", 2);
#endif
  rul__val_set_nth_subvalue (func_val, 1, inst_val);
  rul__val_set_nth_subvalue (func_val, 2,
			     rul__val_create_blk_or_loc (CMP__E_INT_ENTRY_DATA,
							 "eb_data"));

  /* create the object var and assign values */
  obj_var =
    rul__val_create_unnamed_ext_var(rul__sem_register_ext_temp(ext_type_object,
							       EXT__C_NOT_ARRAY
							       ,ext_mech_value,
							       SEM__C_ON_RHS),
				    ext_type_object, EXT__C_NOT_ARRAY,
				    ext_mech_value,SEM__C_NO_MEMORY_ALLOCATED);
  first_val = rul__val_create_assignment (obj_var, func_val);

  /* do all the attribute value pairs... */
  curr_val = rul__sem_check_all_attr_values (node, class_id,
					     obj_var, first_val);
  if (curr_val == NULL) {
    rul__val_free_value (first_val);
    return NULL;
  }

  /* all done with attribue values - do the update and notify */
#ifndef NDEBUG
  func_val = rul__val_create_function_str ("rul__wm_update_and_notify", 2);
#else
  func_val = rul__val_create_function_str ("WUN", 2);
#endif
  rul__val_set_nth_subvalue (func_val, 1, rul__val_copy_value(obj_var));
  rul__val_set_nth_subvalue (func_val, 2,
			     rul__val_create_blk_or_loc (CMP__E_INT_ENTRY_DATA,
							 "eb_data"));
  curr_val = rul__val_set_prev_value(func_val, curr_val);
  
  if (return_val_required) {
#ifndef NDEBUG
    func_val = rul__val_create_function_str ("rul__wm_get_instance_id", 1);
#else
    func_val = rul__val_create_function_str ("WGII", 1);
#endif
    rul__val_set_nth_subvalue (func_val, 1, rul__val_copy_value(obj_var));
    rul__val_set_prev_value(func_val, curr_val);
    return (rul__sem_return_value(return_to_val,
				  func_val,
				  shape_atom,
				  dom_instance_id,
				  SEM__C_ON_RHS,
				  class_id));
  }
  else {
    return curr_val;
  }
}

Value
rul__sem_check_openfile (Ast_Node ast)
{
  Ast_Node       node;
  Ast_Node_Type  node_type;
  Value          func_val, arg_val;
  Molecule       mol;

  /* openfile file-id filespec keyword */

  /* file-id */
  node = rul__ast_get_child(ast);
  node_type = rul__ast_get_type(node);
  if (node_type != AST__E_CONSTANT &&
      node_type != AST__E_VARIABLE) {
    display_check_error (CMP_INVOPNFID, node);
    return NULL;
  }

  if (node_type == AST__E_CONSTANT)
    arg_val = rul__sem_check_rul_constant (node, SEM__C_RETURN_RUL_TYPE);
  else
    arg_val = rul__sem_check_rhs_var (node, SEM__C_VALUE_ACCESS);

  if (arg_val == NULL)
    return NULL;

  func_val = rul__val_create_function_str ("rul__ios_open_file_rt", 3);
  rul__val_set_nth_subvalue (func_val, 1, arg_val);

  /* filename */
  node = rul__ast_get_sibling(node);
  node_type = rul__ast_get_type(node);
  if (node_type != AST__E_CONSTANT &&
      node_type != AST__E_VARIABLE) {
    display_check_error (CMP_INVOPNFIL, node);
    rul__val_free_value (func_val);
    return NULL;
  }

  if (node_type == AST__E_CONSTANT)
    arg_val = rul__sem_check_rul_constant (node, SEM__C_RETURN_RUL_TYPE);
  else
    arg_val = rul__sem_check_rhs_var (node, SEM__C_VALUE_ACCESS);

  if (arg_val == NULL) {
    rul__val_free_value (func_val);
    return NULL;
  }

  rul__val_set_nth_subvalue (func_val, 2, arg_val);

  /* access mode */
  node = rul__ast_get_sibling(node);
  node_type = rul__ast_get_type(node);
  if (node_type != AST__E_CONSTANT &&
      node_type != AST__E_VARIABLE) {
    display_check_error (CMP_INVOPNMOD, node);
    rul__val_free_value (func_val);
    return NULL;
  }
  
  if (node_type == AST__E_CONSTANT) {
    mol = rul__ast_get_value(node);
    if (mol != rul__mol_make_symbol("IN") &&
	mol != rul__mol_make_symbol("OUT") &&
	mol != rul__mol_make_symbol("APPEND")) {
      display_check_error (CMP_INVOPNMDC, node);
      rul__val_free_value (func_val);
      return NULL;
    }
    arg_val = rul__sem_check_rul_constant (node, SEM__C_RETURN_RUL_TYPE);
  }
  else {
    arg_val = rul__sem_check_rhs_var (node, SEM__C_VALUE_ACCESS);
  }

  if (arg_val == NULL) {
    rul__val_free_value (func_val);
    return NULL;
  }

  rul__val_set_nth_subvalue (func_val, 3, arg_val);

  return func_val;
}

Value
rul__sem_check_quit (Ast_Node ast)
{
  Ast_Node      node;
  Value         func_val, arg_val = NULL;

  /* quit [value] */
  node = rul__ast_get_child(ast);  

  if (node == NULL || rul__ast_get_type (node) == AST__E_SUCCESS)
      arg_val = rul__val_create_blk_or_loc (CMP__E_INT_LONG, "EXIT_SUCCESS");
  else if (rul__ast_get_type(node) == AST__E_FAILURE)
      arg_val = rul__val_create_blk_or_loc (CMP__E_INT_LONG, "EXIT_FAILURE");
  else
      arg_val = rul__sem_check_rhs_value (node, SEM__C_RETURN_EXT_TYPE,
					  dom_int_atom, shape_atom, NULL);
  if (arg_val == NULL) {
      return NULL;
  }

  func_val = rul__val_create_function_str ("exit", 1);

  rul__val_set_nth_subvalue (func_val, 1, arg_val);

  return func_val;
}

Value
rul__sem_check_remove (Ast_Node ast)
{
  Ast_Node      node;
  Value         first_val, func_val, arg_val, curr_val;

  /* remove $id-var... */

  first_val = curr_val = NULL;

  for (node = rul__ast_get_child(ast);
       node != NULL;
       node = rul__ast_get_sibling(node)) {
    
    if (rul__ast_get_type (node) != AST__E_VARIABLE) {
      display_check_error (CMP_INVREMPAR, node);
      if (first_val)
	rul__val_free_value (first_val);
      return NULL;
    }
    
    arg_val = rul__sem_check_rhs_var (node, SEM__C_VALUE_ACCESS);
    
    if (arg_val == NULL) {
      if (first_val)
	rul__val_free_value (first_val);
      return NULL;
    }

    if (rul__val_get_domain(arg_val) == dom_any) {
      rul__msg_cmp_print (CMP_IDARGNOVER, node, CMP_C_MSG_REMOVE);
    } else if (rul__val_get_domain(arg_val) != dom_instance_id) {
      display_check_error (CMP_INVREMVAR, node);
      rul__val_free_value (arg_val);
      return NULL;
    }

    func_val = rul__val_create_function_str (
#ifndef NDEBUG
				     "rul__wm_destroy_and_notify_id", 2);
#else
				     "WDNI", 2);
#endif
    rul__val_set_nth_subvalue (func_val, 1, arg_val);
    rul__val_set_nth_subvalue (func_val, 2,
		       rul__val_create_blk_or_loc (CMP__E_INT_ENTRY_DATA,
						   "eb_data"));
    
    if (first_val == NULL) {
      first_val = func_val;
      curr_val = func_val;
    }
    else
      curr_val = rul__val_set_next_value (curr_val, func_val);
  }

  return first_val;
}

Value
rul__sem_check_remove_every (Ast_Node ast)
{
  Ast_Node      node;
  Ast_Node_Type node_type;
  Value         first_val, func_val, arg_val, curr_val;
  Molecule      class_name;
  Class         class_id;
  char          buffer[RUL_C_MAX_SYMBOL_SIZE+1];
  long          blkidx;

  /* remove-every class-name... */

  first_val = curr_val = NULL;

  for (node = rul__ast_get_child(ast);
       node != NULL;
       node = rul__ast_get_sibling(node)) {
    
    class_name = rul__ast_get_value(node);
    node_type = rul__ast_get_type (node);
    if (node_type != AST__E_CONSTANT &&
	node_type != AST__E_VARIABLE &&
	node_type != AST__E_BTI_ACCEPT_ATOM &&
	node_type != AST__E_BTI_CONCAT &&
	node_type != AST__E_BTI_GET &&
	node_type != AST__E_BTI_NTH &&
	node_type != AST__E_BTI_SYMBOL &&
	node_type != AST__E_EXT_RT_CALL) {
      display_check_error (CMP_INVRMEPAR, node);
      if (first_val)
	rul__val_free_value (first_val);
      return NULL;
    }

    if (node_type == AST__E_CONSTANT &&
	class_name != rul__mol_symbol_root()) {

      if (!(class_id = rul__decl_get_visible_class(class_name))) {
	rul__msg_cmp_print_w_atoms(CMP_INVCLSNAM, node, 1, class_name);
	if (first_val)
	  rul__val_free_value (first_val);
	return NULL;
      }

      func_val = rul__val_create_function_str (
#ifndef NDEBUG
			               "rul__wm_destroy_and_notify_all", 2);
#else
			               "WDNA", 2);
#endif
      arg_val = rul__val_create_class_constant (class_id);
    }

    else {

      if (node_type == AST__E_CONSTANT) { /* must be == $root */
	arg_val = rul__val_create_rul_constant (rul__mol_symbol_root ());
      }
      else {
	arg_val = rul__sem_check_nested_value (node, 
					SEM__C_ON_RHS,
					SEM__C_UNDEFINED_CE_INDEX,
					SEM__C_RETURN_RUL_TYPE,
					dom_symbol, shape_atom, NULL);
      }

      func_val = rul__val_create_function_str (
#ifndef NDEBUG
				       "rul__wm_destroy_all_class_var", 2);
#else
				       "WDACV", 2);
#endif
    }

    if (arg_val == NULL) {
      rul__val_free_value (func_val);
      if (first_val)
	rul__val_free_value (first_val);
      return NULL;
    }

    rul__val_set_nth_subvalue (func_val, 1, arg_val);
    rul__val_set_nth_subvalue (func_val, 2,
		       rul__val_create_blk_or_loc (CMP__E_INT_ENTRY_DATA,
						   "eb_data"));

    if (first_val == NULL) {
      first_val = func_val;
      curr_val = func_val;
    }
    else
      curr_val = rul__val_set_next_value(curr_val, func_val);
  }

  return first_val;
}

Value
rul__sem_check_restorestate (Ast_Node ast)
{
  Ast_Node      node;
  Ast_Node_Type node_type;
  Value         func_val, arg_val;

  /* restorestate filespec */

  node = rul__ast_get_child(ast);
  node_type = rul__ast_get_type(node);
  if (node_type != AST__E_CONSTANT &&
      node_type != AST__E_VARIABLE) {
    display_check_error (CMP_INVRESPAR, node);
    return NULL;
  }

  func_val = rul__val_create_function_str ("rul__restorestate", 2);

  if (node_type == AST__E_CONSTANT)
    arg_val = rul__sem_check_rul_constant (node, SEM__C_RETURN_RUL_TYPE);
  else
    arg_val = rul__sem_check_rhs_var (node, SEM__C_VALUE_ACCESS);

  if (arg_val == NULL) {
    rul__val_free_value (func_val);
    return NULL;
  }

  rul__val_set_nth_subvalue (func_val, 1, arg_val);
  rul__val_set_nth_subvalue (func_val, 2,
			     rul__val_create_blk_or_loc (CMP__E_INT_ENTRY_DATA,
							 "eb_data"));
  return func_val;
}

Decl_Domain
convert_ext_type_to_domain (Ext_Type type)
{
  register Decl_Domain domain = dom_invalid;

  switch (type) {
  case ext_type_invalid:
  case ext_type_none:
  case ext_type_atom_ptr:
  case ext_type_object:
    domain = dom_invalid;
    break;

  case ext_type_long:
  case ext_type_short:
  case ext_type_byte:
  case ext_type_uns_long:
  case ext_type_uns_short:
  case ext_type_uns_byte:
  case ext_type_for_each:
    domain = dom_int_atom;
    break;

  case ext_type_float:
  case ext_type_double:
    domain = dom_dbl_atom;
    break;

  case ext_type_void_ptr:
    domain = dom_opaque;
    break;


  case ext_type_atom:
    domain = dom_any;
    break;

  case ext_type_asciz:
  case ext_type_ascid:
    domain = dom_symbol;
    break;


  }

  return domain;
}



Value
rul__sem_check_return (Ast_Node ast)
{
    Ast_Node       arg_node, meth_node;
    Value          arg_val, ret_val, ret_val_assn, ret_val_return;
    Decl_Shape     shape;
    Decl_Domain    domain;
    Boolean	   ret_val_supplied;
    Boolean        ret_val_required;
    Boolean        method_return = FALSE;
    Mol_Symbol     cur_block_name;
    Construct_Type cur_block_type;
    Ext_Rt_Decl	   ext_rtn = NULL;
    Ext_Type 	   type;
    Cardinality    a_len;

    /*  If this return action is in a METHOD, then use the
    **  sem_check_method_return function instead of this one...
    */
    meth_node = rul__ast_get_parent (ast);
    while (meth_node) {
        if (rul__ast_get_type (meth_node) == AST__E_METHOD_RHS)
            return (sem_check_method_return (ast, meth_node));
        meth_node = rul__ast_get_parent (meth_node);
    }

    cur_block_name = rul__conrg_get_cur_block_name ();
    cur_block_type = rul__conrg_get_block_type (cur_block_name);
    if (cur_block_type != RUL__C_ENTRY_BLOCK) {
        /*
        **  The RETURN action is only allowed within methods or
        **  in constructs within an entry-block
        */
        rul__msg_cmp_print (CMP_INVRETURN, ast);
        return (NULL);
    }

    arg_node = rul__ast_get_child (ast);
    ret_val_supplied = arg_node != NULL;

    ext_rtn = rul__decl_get_visible_ext_rt (cur_block_name);
    assert (ext_rtn != NULL);
    ret_val_required = rul__decl_ext_rt_ret_val (ext_rtn);

    if (!ret_val_supplied && ret_val_required) {
        rul__msg_cmp_print (CMP_RETVALREQ, ast);
        return NULL;
    }
    if (!ret_val_required && ret_val_supplied) {
        rul__msg_cmp_print (CMP_RETVALIGN, arg_node);
        ret_val_supplied = FALSE;
    }

    if (ret_val_supplied) {

        /* get the entry-block's return values shape and domain */
        type = rul__decl_ext_rt_ret_type (ext_rtn);
        a_len = rul__decl_ext_rt_ret_a_len (ext_rtn);
        if (a_len == 0)
	    shape = shape_atom;
        else
	    shape = shape_compound;
        domain = convert_ext_type_to_domain(type);
    
        arg_val = rul__sem_check_nested_value (arg_node, 
					SEM__C_ON_RHS,
					SEM__C_UNDEFINED_CE_INDEX,
					SEM__C_RETURN_RUL_TYPE,
					domain, shape, NULL);
        if (arg_val == NULL) return NULL;

        /*  
        **  For a return action with an argument in a RULE, a CATCH, or 
        **  an ON-clause, we need to stash the returned value in the current
        **  blocks static variable for storing return values, increment the
	**  reference count on that value, and then generate a C return 
	**  statement passing the code that says to the RAC that a return 
	**  action was encountered.
        */
	ret_val_assn = rul__val_create_assignment (
			 rul__val_create_blk_or_loc (CMP__E_INT_MOLECULE,
					     rul__gen_get_cur_ret_val_name ()),
					     arg_val);
#ifndef NDEBUG
	ret_val = rul__val_create_function_str ("rul__mol_incr_uses", 1);
#else
	ret_val = rul__val_create_function_str ("MIU", 1);
#endif
	rul__val_set_nth_subvalue (ret_val, 1,
			   rul__val_create_blk_or_loc (CMP__E_INT_MOLECULE,
				       rul__gen_get_cur_ret_val_name ()));
	(void) rul__val_set_prev_value (ret_val, ret_val_assn);

	ret_val_return = rul__val_create_function_str ("return", 1);
	rul__val_set_nth_subvalue (ret_val_return, 1, 
			rul__val_create_long_animal (RUL__C_RETURN));
	(void) rul__val_set_next_value (ret_val, ret_val_return);

    } else {

        /*
	**  For a RETURN action with no argument, so just do a C return
	*/
	ret_val = rul__val_create_function_str ("return", 1);
	rul__val_set_nth_subvalue (ret_val, 1, 
			rul__val_create_long_animal (RUL__C_RETURN));
    }

    rul__sem_eb_return_was_seen(); /* This block does have a return */

    rul__ast_attach_value (ast, AST__E_TYPE_VALUE, ret_val);
    return ret_val;
}



static Value sem_check_method_return (Ast_Node ast, Ast_Node meth_node)
{
    Ast_Node       arg_node;
    Value          arg_val, ret_val;
    Value	   ret_val_assignment = NULL;
    Value	   ret_val_func = NULL;
    Decl_Shape     shape;
    Decl_Domain    domain;
    Boolean	   ret_val_supplied;
    Boolean        ret_val_required;
    Boolean        method_return = FALSE;
    Mol_Symbol     cur_block_name, meth_name, class_name;
    Construct_Type cur_block_type;
    Ext_Rt_Decl	   ext_rtn = NULL;
    Ext_Type 	   type;
    Cardinality    a_len;
    Method         meth;
    Class          rclass;
    Molecule       ret_mol;
    long           par_cnt;

      meth_node = rul__ast_get_child (rul__ast_get_parent (meth_node));
      meth_name = rul__ast_get_value (meth_node);
      class_name = rul__ast_get_value (
		       rul__ast_get_sibling (
			     rul__ast_get_child (
				 rul__ast_get_child (
				     rul__ast_get_sibling (meth_node)))));
      rclass = rul__decl_get_class (rul__decl_get_curr_decl_block (),
				   class_name);
      meth = rul__decl_get_generic_method (rclass, meth_name);
      method_return = TRUE;

    /* method return */
/*?*
    par_cnt = rul__decl_get_method_num_params (meth);
    shape   = rul__decl_get_method_par_shape (meth, par_cnt);
    domain  = rul__decl_get_method_par_domain (meth, par_cnt);
    rclass   = rul__decl_get_method_par_class (meth, par_cnt);

    if (!ret_val_supplied) {
      *?* return the named return value, default already set *?*
      ret_val_func = rul__val_create_function_str ("return", 1);
      rul__val_set_nth_subvalue (ret_val_func, 1,
			 rul__val_create_blk_or_loc (CMP__E_INT_MOLECULE,
						     GEN__C_METHOD_RETURN));
    }

    else {

      arg_val = rul__sem_check_nested_value (arg_node, 
					SEM__C_ON_RHS,
					SEM__C_UNDEFINED_CE_INDEX,
					SEM__C_RETURN_RUL_TYPE,
					domain, shape, NULL);
      if (arg_val == NULL)
	return NULL;
      
      ret_val = rul__val_create_blk_or_loc (CMP__E_INT_MOLECULE,
					    GEN__C_METHOD_RETURN);
      ret_val_assignment = rul__val_create_assignment (ret_val, arg_val);
#ifndef NDEBUG
      ret_val_func = rul__val_create_function_str ("rul__mol_incr_uses",1);
#else
      ret_val_func = rul__val_create_function_str ("MIU",1);
#endif
      rul__val_set_nth_subvalue (ret_val_func, 1,
				 rul__val_copy_value (ret_val));
      ret_val_assignment = rul__val_set_prev_value (ret_val_func,
						    ret_val_assignment);
      ret_val_func = rul__val_create_function_str ("return", 1);
      rul__val_set_nth_subvalue (ret_val_func, 1,
				 rul__val_copy_value (ret_val));
    }
  }
*?*/
      return (rul__val_create_function_str ("return", 0)); /*?*/
}



Value
rul__sem_check_savestate (Ast_Node ast)
{
  Ast_Node      node;
  Ast_Node_Type node_type;
  Value         func_val, arg_val;

  /* savestate filespec */

  node = rul__ast_get_child(ast);
  node_type = rul__ast_get_type(node);
  if (node_type != AST__E_CONSTANT &&
      node_type != AST__E_VARIABLE) {
    display_check_error (CMP_INVSAVPAR, node);
    return NULL;
  }

  func_val = rul__val_create_function_str ("rul__savestate", 2);

  if (node_type == AST__E_CONSTANT)
    arg_val = rul__sem_check_rul_constant (node, SEM__C_RETURN_RUL_TYPE);
  else
    arg_val = rul__sem_check_rhs_var (node, SEM__C_VALUE_ACCESS);

  if (arg_val == NULL) {
    rul__val_free_value (func_val);
    return NULL;
  }

  rul__val_set_nth_subvalue (func_val, 1, arg_val);
  rul__val_set_nth_subvalue (func_val, 2,
			     rul__val_create_blk_or_loc (CMP__E_INT_ENTRY_DATA,
							 "eb_data"));

  return func_val;
}

Value
rul__sem_check_specialize (Ast_Node ast, 
			   Boolean return_val_required,
			   Value return_to_val)
{
  Ast_Node      node;
  Ast_Node_Type node_type;
  Value         inst_val,	/* instance id value */
                class_val,      /* used when the class name is a var */
		func_val,	/* current function value */
		obj_var,	/* rul__wm_specid return value */
                first_val,	/* first in sequence of values */
                curr_val;	/* handle to last in sequence of set attrs */
  Class	        class_id = NULL, new_class_id = NULL;
  Molecule      class_name;
  char          buffer[RUL_C_MAX_SYMBOL_SIZE+1];
  long          blkidx;

  /* specialize <$id-var> class-name {^attr val-exp}... 
   *
   * generated code will look like:
   *
   * obj_var = rul__wm_specialize_id(new_class_id, bound_instance_id);
   * rul__wm_set_attr_val(obj_var, mol[attr], value); <-- for each attr
   * rul__wm_update_and_notify(obj_var);
   * return (rul__wm_get_instance_id(obj_var);
   *
   */

  /* get and verify element id binding */
  node = rul__ast_get_child(ast);
  if (rul__ast_get_type(node) != AST__E_VARIABLE) {
    display_check_error (CMP_INVSPEPAR, node);
    return NULL;
  }

  inst_val = rul__sem_check_rhs_var(node, SEM__C_VALUE_ACCESS);
  if (inst_val == NULL)
    return NULL;

  if (rul__val_get_domain(inst_val) == dom_instance_id) {
    class_id = rul__val_get_class(inst_val);
    if (class_id == NULL)
      rul__msg_cmp_print (CMP_IDARGNOCLASS, node, CMP_C_MSG_SPECIAL);
  }

  else if (rul__val_get_domain(inst_val) != dom_any) {
    display_check_error (CMP_INVSPEVAR, node);
    rul__val_free_value (inst_val);
    return NULL;
  }

  else
    rul__msg_cmp_print (CMP_IDARGNOVER, node, CMP_C_MSG_SPECIAL);


  /* check the new class */

  node = rul__ast_get_sibling(node);
  class_name = rul__ast_get_value(node);
  node_type = rul__ast_get_type(node);
  if (node_type != AST__E_CONSTANT &&
      node_type != AST__E_VARIABLE &&
      node_type != AST__E_BTI_ACCEPT_ATOM &&
      node_type != AST__E_BTI_CONCAT &&
      node_type != AST__E_BTI_GET &&
      node_type != AST__E_BTI_NTH &&
      node_type != AST__E_BTI_SYMBOL &&
      node_type != AST__E_EXT_RT_CALL) {
    display_check_error (CMP_INVSPECLS, node);
    rul__val_free_value (inst_val);
    return NULL;
  }

  /* check the new class name */
  if (node_type == AST__E_CONSTANT) {
    class_name = rul__ast_get_value(node);
    if (!(new_class_id = rul__decl_get_visible_class(class_name))) {
      rul__msg_cmp_print_w_atoms(CMP_INVCLSNAM, node, 1, class_name);
      rul__val_free_value (inst_val);
      return NULL;
    }

    /* the specialized class must be a sub-class of the current class */
    /* remove the "if (class_id)" when we can return a variables class */
    if (class_id) {
      if ((!rul__decl_is_subclass_of(new_class_id, class_id)) ||
	  (class_id == new_class_id)) {
	rul__msg_cmp_print_w_atoms(CMP_INVSPESCS, node, 1, class_name);
	rul__val_free_value (inst_val);
	return NULL;
      }
    }
#ifndef NDEBUG
    func_val = rul__val_create_function_str ("rul__wm_specialize_id", 3);
#else
    func_val = rul__val_create_function_str ("WSPI", 3);
#endif
    rul__val_set_nth_subvalue (func_val, 2,
			       rul__val_create_class_constant (new_class_id));
  }
  else {

    class_val = rul__sem_check_nested_value (node, 
					SEM__C_ON_RHS,
					SEM__C_UNDEFINED_CE_INDEX,
					SEM__C_RETURN_RUL_TYPE,
					dom_symbol, shape_atom, NULL);
    if (class_val == NULL) {
      rul__val_free_value (inst_val);
      return NULL;
    }
#ifndef NDEBUG
    func_val = rul__val_create_function_str ("rul__wm_specialize_var", 3);
#else
    func_val = rul__val_create_function_str ("WSPV", 3);
#endif
    rul__val_set_nth_subvalue (func_val, 2, class_val);
  }

  /* create the spec wm and assign values */
  obj_var =
    rul__val_create_unnamed_ext_var(rul__sem_register_ext_temp(ext_type_object,
							       EXT__C_NOT_ARRAY
							       ,ext_mech_value,
							       SEM__C_ON_RHS),
				    ext_type_object, EXT__C_NOT_ARRAY,
				    ext_mech_value,SEM__C_NO_MEMORY_ALLOCATED);
  rul__val_set_nth_subvalue (func_val, 1, inst_val);
  rul__val_set_nth_subvalue (func_val, 3,
			     rul__val_create_blk_or_loc (CMP__E_INT_ENTRY_DATA,
							 "eb_data"));

  /* assign the object var */
  first_val = rul__val_create_assignment (obj_var, func_val);

  /* do all the attribute value pairs... */
  curr_val = rul__sem_check_all_attr_values (node, new_class_id,
					     obj_var, first_val);
  if (curr_val == NULL) {
    rul__val_free_value (first_val);
    return NULL;
  }

  /* all done with attribue values - do the update and notify */
#ifndef NDEBUG
  func_val = rul__val_create_function_str ("rul__wm_update_and_notify", 2);
#else
  func_val = rul__val_create_function_str ("WUN", 2);
#endif
  rul__val_set_nth_subvalue (func_val, 1, rul__val_copy_value(obj_var));
  rul__val_set_nth_subvalue (func_val, 2,
			     rul__val_create_blk_or_loc (CMP__E_INT_ENTRY_DATA,
							 "eb_data"));
  curr_val = rul__val_set_prev_value(func_val, curr_val);
  
  if (return_val_required) {
#ifndef NDEBUG
    func_val = rul__val_create_function_str ("rul__wm_get_instance_id", 1);
#else
    func_val = rul__val_create_function_str ("WGII", 1);
#endif
    rul__val_set_nth_subvalue (func_val, 1, rul__val_copy_value(obj_var));
    rul__val_set_prev_value(func_val, curr_val);
    return (rul__sem_return_value(return_to_val,
				  func_val,
				  shape_atom,
				  dom_instance_id,
				  SEM__C_ON_RHS,
				  new_class_id));
  }
  else {
    return curr_val;
  }
}

Value
rul__sem_check_trace (Ast_Node ast)
{
  Ast_Node      node;
  Value         func_val, arg_val;
  long          arg_cnt = rul__ast_get_child_count(ast);
  long          i;

  /* trace trace-names... */

  func_val = rul__val_create_function_str ("rul__trace", arg_cnt + 1);

  /* make a Value from the arg count */
  arg_val = rul__val_create_long_animal (arg_cnt);
  rul__val_set_nth_subvalue (func_val, 1, arg_val);

  for (i = 2, node = rul__ast_get_child(ast), arg_cnt += 2;
       i < arg_cnt;
       i++, node = rul__ast_get_sibling(node)) {
    
    arg_val = rul__sem_check_nested_value (node, SEM__C_ON_RHS,
					   SEM__C_UNDEFINED_CE_INDEX,
					   SEM__C_RETURN_RUL_TYPE,
					   dom_any, shape_molecule, NULL);
    if (arg_val == NULL) {
      rul__val_free_value(func_val);
      return NULL;
    }
    else {
      rul__val_set_nth_subvalue (func_val, i, arg_val);
    }
  }

  return func_val;
}

Value
rul__sem_check_while (Ast_Node ast)
{
  Ast_Node      node, act_node;
  Value         ret_val = NULL, arg_val;

  /* while (expression) do actions... */

  node = rul__ast_get_child (ast);

  arg_val = sem_build_cmplx_exprs (node, &ret_val);
  if (arg_val == NULL)
    return NULL;

  rul__ast_attach_value (node, AST__E_TYPE_VALUE, arg_val);

  /* now the actions... */
  node = rul__ast_get_sibling (node);
  act_node = rul__ast_get_child (node);

  if (act_node != NULL)
    rul__sem_rhs_action_end_vars ();

  while (act_node != NULL) {

    arg_val = rul__sem_check_rhs_action (act_node, 1);
    if (arg_val == NULL) {
      if (ret_val)
	rul__val_free_value (ret_val);
      return NULL;
    }

    else {

      rul__ast_attach_value (act_node, AST__E_TYPE_VALUE, arg_val);
      act_node = rul__ast_get_sibling (act_node);

      /*
       * get next, end_action_vars if more. calling routine will
       * end_action_vars if no more
       */
      if (act_node != NULL)
	rul__sem_rhs_action_end_vars ();
    }
  }

  if (ret_val == NULL)
    return (Value) -1;

  return ret_val;
}

Value
rul__sem_check_write (Ast_Node ast)
{
  Ast_Node      node;
  Ast_Node_Type node_type;
  Value         func_val, arg_val, ret_val, curr_val = NULL;
  long          arg_cnt = rul__ast_get_child_count(ast);
  long          i;

  /* write [fil-id] rhs-exp */


  /* loop through all child nodes looking for and
   * setting up pre-write compounds for crlf's, tabto's and rjust's
   */
  ret_val = NULL;

  for (node = rul__ast_get_child(ast);
       node != NULL;
       node = rul__ast_get_sibling(node)) {
    
    node_type = rul__ast_get_type (node);
    if (node_type == AST__E_BTI_TABTO ||
	node_type == AST__E_BTI_RJUST) {
    
      if (node_type == AST__E_BTI_TABTO) {
	arg_val = rul__sem_check_bti_tabto (node);
      }
      else if (node_type == AST__E_BTI_RJUST) {
	arg_val = rul__sem_check_bti_rjust (node);
      }
      if (arg_val == NULL) {
	if (ret_val)
	  rul__val_free_value (ret_val);
	return NULL;
      }
      if (ret_val == NULL) {
	ret_val = arg_val;
	curr_val = arg_val;
      }
      else
	curr_val = rul__val_set_next_value (curr_val, arg_val);
    }
  }

  /* make the func and a value from the arg count */
  func_val = rul__val_create_function_str ("rul__ios_write", arg_cnt + 1);
  rul__val_set_nth_subvalue (func_val, 1,
		     rul__val_create_long_animal(arg_cnt));

  for (i = 2, node = rul__ast_get_child(ast);
       node != NULL;
       i++, node = rul__ast_get_sibling(node)) {
    
    node_type = rul__ast_get_type (node);
    if (node_type == AST__E_BTI_TABTO ||
	node_type == AST__E_BTI_RJUST) {
      
      arg_val = rul__ast_get_value (node);
      rul__ast_attach_value(node,AST__E_TYPE_NULL,NULL);
    }
    else {

      arg_val = rul__sem_check_nested_value (node, 
					SEM__C_ON_RHS,
					SEM__C_UNDEFINED_CE_INDEX,
					SEM__C_RETURN_RUL_TYPE,
					dom_any, shape_molecule, NULL);
      
      if (arg_val == NULL) {
	rul__val_free_value (ret_val);
	return NULL;
      }
    }
    rul__val_set_nth_subvalue (func_val, i, arg_val);
  }

  if (ret_val == NULL)
    ret_val = func_val;
  else
    rul__val_set_next_value (curr_val, func_val);
  
  return ret_val;
}

static void
display_check_error (char *err_code, Ast_Node node)
{
  Molecule   mol = NULL;
  Ast_Node   child_node;

  if (rul__ast_get_value_type (node) == AST__E_TYPE_MOL)
    mol = rul__ast_get_value (node);
  else {
    child_node = rul__ast_get_child (node);    
    if (child_node != NULL)
      if (rul__ast_get_value_type (child_node) == AST__E_TYPE_MOL)
	mol = rul__ast_get_value (child_node);
  }

  if (mol)
    rul__msg_cmp_print_w_atoms(err_code, node, 1, mol);
  else
    rul__msg_cmp_print (err_code, node, "");

  return;
}
