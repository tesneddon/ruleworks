/****************************************************************************
**                                                                         **
**                     C M P _ S E M _ B T I . C                           **
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
 *	This module provides semantic checks for RHS built_ins.
 *
 * MODIFIED BY:
 *	DEC	Digital Equipment Corporation
 *	CPQ	Compaq Computer Corporation
 *
 * MODIFICATION HISTORY:
 *	15-Jan-1993	DEC	Initial version
 *	16-Fwb-1998	DEC	class type changed to rclass
 *	01-Dec-1999	CPQ	Release with GPL
 *
 */

#include <common.h>
#include <cmp_comm.h>
#include <mol.h>
#include <decl.h>
#include <sem.h>
#include <ast.h>
#include <val.h>
#include <msg.h>
#include <gen.h>
#include <cmp_msg.h>


/* define some static variables */
static Molecule bti_SM__crlf;
static Molecule bti_SM__tabto;
static Molecule bti_SM__rjust;

Decl_Domain  sem_union_of_comp_arg_domains (Value func_val);
Class  sem_union_of_comp_arg_classes (Value func_val);



/* built_ins */



/********************************
**                             **
**  RUL__SEM_CHECK_BTI_ACCEPT  **
**                             **
********************************/

Value rul__sem_check_bti_accept(Ast_Node ast, 
				Decl_Shape accept_shape,
				Value return_to_val)
{
   Ast_Node      node;
   Ast_Node_Type node_type;
   Value	 func_val, arg_val, def_val = NULL;
   long          arg_cnt = rul__ast_get_child_count(ast);
   Molecule      mol;
   Boolean       decmol = FALSE;

   if (accept_shape == shape_atom) 
     func_val = rul__val_create_function_str ("rul__ios_accept_atom", 1);
   else
     func_val = rul__val_create_function_str ("rul__ios_accept_line", 2);

   /* the FILE-ID check could be enhanced to accept any scalar value */
   /* currently must be a constant or variable                       */ 

   node = rul__ast_get_child(ast);
   if (node != NULL) {
      node_type = rul__ast_get_type(node);
      if (node_type != AST__E_CONSTANT && node_type != AST__E_VARIABLE) {
	 mol = rul__ast_get_value(node);
	 if (! mol && (rul__ast_get_type (node) == AST__E_BTI_COMPOUND)) {
	    decmol = TRUE;
	    mol = rul__mol_make_comp (0);
	 }
	 if (accept_shape == shape_compound) {
	    if (mol)
	      rul__msg_cmp_print_w_atoms (CMP_INVACLFID, node, 1, mol);
	    else
	      rul__msg_cmp_print (CMP_INVACLFID, node, "");
	 }
	 else {
	    if (mol)
	      rul__msg_cmp_print_w_atoms (CMP_INVACAFID, node, 1, mol);
	    else
	      rul__msg_cmp_print (CMP_INVACAFID, node, "");
	 }
	 if (decmol)
	   rul__mol_decr_uses (mol);
	 rul__val_free_value (func_val);
	 return NULL;
      }
    
      arg_val = rul__sem_check_value (node, SEM__C_ON_RHS,
				      SEM__C_UNDEFINED_CE_INDEX,
				      SEM__C_RETURN_RUL_TYPE,
				      dom_symbol, shape_atom, NULL);

      if (arg_val == NULL) {
	 rul__val_free_value (func_val);
	 return NULL;
      }
    
      rul__val_set_nth_subvalue (func_val, 1, arg_val);
    
      /* acceptline-compound may have an additional arg - default compound */

      if (accept_shape == shape_compound) {
	 if (arg_cnt == 2) {
	    node = rul__ast_get_sibling (node);
	    def_val = rul__sem_check_value (node, SEM__C_ON_RHS,
					    SEM__C_UNDEFINED_CE_INDEX,
					    SEM__C_RETURN_RUL_TYPE,
					    dom_any, shape_compound, NULL);
	    if (def_val == NULL) {
	       rul__val_free_value (func_val);
	       return NULL;
	    }
	
	    rul__val_set_nth_subvalue (func_val, 2, def_val);
	 }
	 else {
	    arg_val = rul__val_create_blk_or_loc (CMP__E_INT_MOLECULE, "NULL");
	    rul__val_set_nth_subvalue (func_val, 2, arg_val);
	 }
      }
   }
   else {
      arg_val = rul__val_create_blk_or_loc (CMP__E_INT_MOLECULE, "NULL");
      rul__val_set_nth_subvalue (func_val, 1, arg_val);
      if (accept_shape == shape_compound)
	rul__val_set_nth_subvalue (func_val, 2, rul__val_copy_value(arg_val));
   }

   return rul__sem_return_value (return_to_val, func_val, 
				 accept_shape, dom_any, SEM__C_ON_RHS, NULL);
}





/**********************************
**                               **
**  RUL__SEM_CHECK_BTI_COMPOUND  **
**                               **
**********************************/

Value rul__sem_check_bti_compound (Ast_Node ast, Boolean on_lhs,
				  long ce_index, Value return_to_val)
{
  Ast_Node      node;
  Value	        func_val, arg_val;
  long          arg_cnt = rul__ast_get_child_count(ast);
  long          i;
  Boolean       all_constants = TRUE;
  Molecule      comp_mol = NULL;
  Decl_Domain	comp_domain = dom_any;
  Class		comp_class = NULL;

  /* if all contained atoms are constants, create a constant compound */

  for (node = rul__ast_get_child(ast);
       node != NULL;
       node = rul__ast_get_sibling(node)) {
    if (rul__ast_get_type (node) != AST__E_CONSTANT) {
      all_constants = FALSE;
      break;
    }
  }

  if (all_constants == TRUE) {
    rul__mol_start_tmp_comp (arg_cnt);
    for (node = rul__ast_get_child(ast), i = 0;
	 node != NULL;
	 node = rul__ast_get_sibling(node), i++) {
      rul__mol_set_tmp_comp_nth (i, rul__ast_get_value (node));
    }
    comp_mol = rul__mol_end_tmp_comp ();
    arg_val = rul__val_create_rul_constant (comp_mol);

    comp_domain = rul__mol_get_domain (comp_mol);
    rul__val_set_shape_and_domain (arg_val, shape_compound, comp_domain);
    rul__mol_decr_uses (comp_mol);
    return rul__sem_return_value (return_to_val, arg_val, 
				  shape_compound, comp_domain, on_lhs, NULL);
  }
  else {

    /* create function_str with argument count */
#ifndef NDEBUG
    func_val = rul__val_create_function_str ("rul__mol_make_comp", arg_cnt+1);
#else
    func_val = rul__val_create_function_str ("MMC", arg_cnt+1);
#endif

    /* make a Value from the arg count */
    arg_val = rul__val_create_long_animal(arg_cnt);

    /* set arg count as first parameter to make_comp function */
    rul__val_set_nth_subvalue (func_val, 1, arg_val);

    for (i = 2, node = rul__ast_get_child(ast), arg_cnt += 2;
	 i < arg_cnt;
	 i++, node = rul__ast_get_sibling(node)) {
      arg_val = rul__sem_check_nested_value (
				node, on_lhs, ce_index, 
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
  }
  comp_domain = sem_union_of_comp_arg_domains (func_val);
  if (comp_domain == dom_instance_id)
    comp_class =  sem_union_of_comp_arg_classes (func_val);

  return rul__sem_return_value (return_to_val, func_val, 
				shape_compound, comp_domain, 
				SEM__C_ON_RHS, comp_class);
}




/********************************
**                             **
**  RUL__SEM_CHECK_BTI_CONCAT  **
**                             **
********************************/

Value rul__sem_check_bti_concat (Ast_Node ast, 
			 	 Boolean on_lhs, long ce_index,
				 Value return_to_val)
{
  Ast_Node      node;
  Value	        func_val, arg_val;
  long          arg_cnt = rul__ast_get_child_count(ast);
  long          i;

  /* create function_str with argument count */
#ifndef NDEBUG
  func_val = rul__val_create_function_str ("rul__mol_concat_atoms", arg_cnt+1);
#else
  func_val = rul__val_create_function_str ("MCA", arg_cnt+1);
#endif

  /* make a Value from the arg count */
  arg_val = rul__val_create_long_animal(arg_cnt);

  /* set arg count as first parameter to make_comp function */
  rul__val_set_nth_subvalue (func_val, 1, arg_val);

  for (i = 2, node = rul__ast_get_child(ast), arg_cnt += 2;
       i < arg_cnt;
       i++, node = rul__ast_get_sibling(node)) {
    arg_val = rul__sem_check_nested_value (
				node, on_lhs, ce_index,
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

  return rul__sem_return_value (return_to_val, func_val, 
				shape_atom, dom_symbol, on_lhs, NULL);
}




/******************************
**                           **
**  RUL__SEM_CHECK_BTI_CRLF  **
**                           **
******************************/

Value rul__sem_check_bti_crlf (Ast_Node ast)
{
  Value    func_val, arg_val;

  /* CRLF */

  if (rul__ast_get_type(rul__ast_get_parent(ast)) != AST__E_WRITE_ACTION) {
    rul__msg_cmp_print (CMP_INVCRLUSA, ast);	     
    return NULL;
  }

  if (bti_SM__crlf == NULL)
    bti_SM__crlf = rul__mol_make_comp (1, rul__mol_make_symbol ("$CRLF"));

  arg_val = rul__val_create_rul_constant (bti_SM__crlf);
  rul__val_set_shape_and_domain (arg_val, shape_compound, dom_any);

  return arg_val;
}




/*******************************
**                            **
**  RUL__SEM_CHECK_BTI_EVERY  **
**                            **
*******************************/

Value rul__sem_check_bti_every (Ast_Node ast,
				Boolean on_lhs, long ce_index,
				Value return_to_val)
{
  Ast_Node    node;
  Value       func_val, arg_val;
  Class       val_class = NULL;
  Mol_Symbol  class_name;

  node = rul__ast_get_child (ast);
  arg_val = rul__sem_check_nested_value (
				node, on_lhs, ce_index, SEM__C_RETURN_RUL_TYPE,
				dom_symbol, shape_atom, NULL);
  if (arg_val == NULL)
    return NULL;

#ifndef NDEBUG
  func_val = rul__val_create_function_str ("rul__wm_get_every", 2), 
#else
  func_val = rul__val_create_function_str ("WGE", 2), 
#endif
  rul__val_set_nth_subvalue (func_val, 1, arg_val);
  rul__val_set_nth_subvalue (func_val, 2,
			     rul__val_create_blk_or_loc (CMP__E_INT_ENTRY_DATA,
							 "eb_data"));

  /* validate class if a constant */
  if (rul__ast_get_type (node) == AST__E_CONSTANT) {
    class_name = rul__ast_get_value (node);
    val_class = rul__decl_get_visible_class (class_name);
    if (val_class == NULL) {
      rul__msg_cmp_print_w_atoms (CMP_INVCLSNAM, node, 1, class_name);
      return NULL;
    }
  }

  return rul__sem_return_value (return_to_val, func_val, shape_compound,
				dom_instance_id, on_lhs, val_class);
}




/*******************************
**                            **
**  RUL__SEM_CHECK_BTI_FLOAT  **
**                            **
*******************************/

Value rul__sem_check_bti_float (Ast_Node ast,
				Boolean on_lhs, long ce_index,
				Value return_to_val)
{
  Ast_Node node;
  Value func1_val, func2_val, arg_val;
  
  node = rul__ast_get_child(ast);
  arg_val = rul__sem_check_nested_value (node, on_lhs, ce_index,
					 SEM__C_RETURN_RUL_TYPE,
					 dom_number, shape_atom, NULL);
  if (arg_val == NULL)
    return NULL;

  func1_val = rul__val_create_function_str ("rul__cvt_s_at_s_df", 1), 
  rul__val_set_nth_subvalue (func1_val, 1, arg_val);

  func2_val = rul__val_create_function_str ("rul__cvt_s_df_s_db", 1), 
  rul__val_set_nth_subvalue (func2_val, 1, func1_val);
  return rul__sem_return_value (return_to_val, func2_val,
				shape_atom, dom_dbl_atom, on_lhs, NULL);
}




/*********************************
**                              **
**  RUL__SEM_CHECK_BTI_GENATOM  **
**                              **
*********************************/

Value rul__sem_check_bti_genatom (Ast_Node ast, Value return_to_val)
{
  Ast_Node node;
  Value    func, sym_val = NULL;

  node = rul__ast_get_child (ast);
  if (node) {
    sym_val = rul__sem_check_nested_value (node, SEM__C_ON_RHS,
					   SEM__C_UNDEFINED_CE_INDEX,
					   SEM__C_RETURN_RUL_TYPE,
					   dom_symbol, shape_atom, NULL);
    if (sym_val == NULL)
      return NULL;
  }

  func = rul__val_create_function_str ("rul__mol_gensym", 1); 
  if (sym_val)
    rul__val_set_nth_subvalue (func, 1, sym_val);
  else
    rul__val_set_nth_subvalue (func, 1,
			       rul__val_create_null (CMP__E_INT_MOLECULE));
  
  return rul__sem_return_value (return_to_val, func,
				shape_atom, dom_symbol, 
				SEM__C_ON_RHS, NULL);
}




/*********************************
**                              **
**  RUL__SEM_CHECK_BTI_GENINT  **
**                              **
*********************************/

Value rul__sem_check_bti_genint (Ast_Node ast, Value return_to_val)
{
  
  /* create function_str with argument count */
  return rul__sem_return_value (return_to_val,
			rul__val_create_function_str ("rul__mol_genint", 0),
				shape_atom, dom_int_atom, 
				SEM__C_ON_RHS, NULL);
}



/*****************************
**                          **
**  RUL__SEM_CHECK_BTI_GET  **
**                          **
*****************************/

Value rul__sem_check_bti_get (Ast_Node ast, 
			      Decl_Domain domain, Decl_Shape shape,
			      Value return_to_val)
{
  Ast_Node      node;
  Ast_Node_Type node_type;
  Value         func_val = NULL, inst_val, arg_val, att_val, incr_val;
  Ast_Node      att_node;
  Molecule      attr_name;
  Class	        class_id = NULL;
  Decl_Domain	val_domain = dom_any;
  Decl_Shape	val_shape = shape_molecule;
  Class		val_class = NULL;
  Boolean       var_is_id = FALSE;
  Boolean       eb_data_needed = FALSE;

  /* get <$ID> ^attr */

  node = rul__ast_get_child(ast);

  inst_val = rul__sem_check_nested_value (
				node, SEM__C_ON_RHS, SEM__C_UNDEFINED_CE_INDEX,
				SEM__C_RETURN_RUL_TYPE,
				dom_any, shape_atom, NULL);
  if (inst_val == NULL)
    return NULL;
  
  if (rul__val_get_domain (inst_val) == dom_instance_id) {
    class_id = rul__val_get_class (inst_val);
    if (class_id != NULL) {
      /*  If we know the exact type of the variable, then we can do more  */
      var_is_id = TRUE;
    }
  }
  else if (rul__val_get_domain (inst_val) != dom_any) {
    /*  bogus argument to GET  */
    rul__msg_cmp_print (CMP_INVGETIDARG, node);
    rul__val_free_value (inst_val);
    return NULL;
  }
  else {
    /*  domain was dom_any; tell user that we are inserting run-time checks  */
    rul__msg_cmp_print (CMP_IDARGNOVER, node, CMP_C_MSG_GET);
  }

  att_node = rul__ast_get_sibling (node);
  node_type = rul__ast_get_type (att_node);
  if (node_type != AST__E_CONSTANT &&
      node_type != AST__E_VARIABLE) {
    rul__msg_cmp_print_w_atoms (CMP_INVATTSPE, att_node,
			       1, rul__ast_get_value (att_node));
    rul__val_free_value (inst_val);
    return NULL;
  }

  attr_name = rul__ast_get_value (att_node);

  if (rul__val_get_domain (inst_val) == dom_instance_id && class_id == NULL) {
    if ((node_type != AST__E_CONSTANT) ||
	attr_name != rul__mol_symbol_id () &&
	attr_name != rul__mol_symbol_instance_of ()) {
      /*  un-verifiable class/attribute - give info on run-time checks */
      rul__msg_cmp_print (CMP_IDARGNOCLASS, node, CMP_C_MSG_GET);
    }
    else {
      class_id = rul__decl_get_visible_class (rul__mol_symbol_root ());
      var_is_id = TRUE;
    }
  }

  if (var_is_id == TRUE) {
    if (node_type == AST__E_CONSTANT) {
      if (!rul__decl_is_attr_in_class(class_id, attr_name)) {
	rul__msg_cmp_print_w_atoms(CMP_INVATTCLS, att_node, 2, attr_name,
				   rul__decl_get_class_name(class_id));
	rul__val_free_value (inst_val);
	return NULL;
      }
    
#ifndef NDEBUG
      func_val = rul__val_create_function_str ("rul__wm_get_offset_val_id", 2);
#else
      func_val = rul__val_create_function_str ("WGOVI", 2);
#endif
      arg_val = rul__val_create_long_animal(
			  rul__decl_get_attr_offset (class_id, attr_name));
      val_shape = rul__decl_get_attr_shape (class_id, attr_name);
      val_domain = rul__decl_get_attr_domain (class_id, attr_name);
      val_class = rul__decl_get_attr_class (class_id, attr_name);
    }
    else {

#ifndef NDEBUG
      func_val = rul__val_create_function_str ("rul__wm_get_attr_val_id", 3);
#else
      func_val = rul__val_create_function_str ("WGAVI", 3);
#endif
      eb_data_needed = TRUE;
      arg_val = rul__sem_check_rhs_var (att_node, SEM__C_VALUE_ACCESS);
    }
  }
  else {

#ifndef NDEBUG
    func_val = rul__val_create_function_str ("rul__wm_get_attr_val_var", 3);
#else
    func_val = rul__val_create_function_str ("WGAVV", 3);
#endif
    eb_data_needed = TRUE;
    if (node_type == AST__E_CONSTANT)
      arg_val = rul__sem_check_rul_constant (att_node, SEM__C_RETURN_RUL_TYPE);
    else
      arg_val = rul__sem_check_rhs_var (att_node, SEM__C_VALUE_ACCESS);
  }

  if (arg_val == NULL) {
    if (func_val)
      rul__val_free_value (func_val);
    return NULL;
  }

  if (rul__sem_check_value_type (ast, domain, shape,
				 val_domain, val_shape,
				 "GET's return value")) {

    rul__val_set_nth_subvalue (func_val, 1, inst_val);
    rul__val_set_nth_subvalue (func_val, 2, arg_val);
    if (eb_data_needed)
      rul__val_set_nth_subvalue (func_val, 3,
			 rul__val_create_blk_or_loc (CMP__E_INT_ENTRY_DATA,
						     "eb_data"));

    return rul__sem_return_value (return_to_val, func_val,
				val_shape, val_domain,
				SEM__C_ON_RHS, val_class);
  }
  return NULL;
}




/*********************************
**                              **
**  RUL__SEM_CHECK_BTI_INTEGER  **
**                              **
*********************************/

Value rul__sem_check_bti_integer (Ast_Node ast,
				  Boolean on_lhs, long ce_index,
				  Value return_to_val)
{
  Ast_Node node;
  Value func1_val, func2_val, arg_val;
  
  node = rul__ast_get_child(ast);
  arg_val = rul__sem_check_nested_value (node, on_lhs, ce_index,
					 SEM__C_RETURN_RUL_TYPE,
				  	 dom_number, shape_atom, NULL);
  if (arg_val == NULL)
    return NULL;

  func1_val = rul__val_create_function_str ("rul__cvt_s_at_s_lo", 1);
  rul__val_set_nth_subvalue (func1_val, 1, arg_val);
  
  func2_val = rul__val_create_function_str ("rul__cvt_s_lo_s_in", 1); 
  rul__val_set_nth_subvalue (func2_val, 1, func1_val);
  return rul__sem_return_value (return_to_val, func2_val,
				shape_atom, dom_int_atom, on_lhs, NULL);
}





/*********************************
**                              **
**  RUL__SEM_CHECK_BTI_IS_OPEN  **
**                              **
*********************************/

Value rul__sem_check_bti_is_open (Ast_Node ast,
				  Boolean on_lhs, long ce_index,
				  Value return_to_val)
{
  Ast_Node node;
  Value func_val, arg_val;
  
  node = rul__ast_get_child (ast);
  arg_val = rul__sem_check_nested_value (node, on_lhs, ce_index,
					 SEM__C_RETURN_RUL_TYPE,
				  	 dom_symbol, shape_atom, NULL);
  if (arg_val == NULL)
    return NULL;

  func_val = rul__val_create_function_str ("rul__ios_is_open", 1);
  rul__val_set_nth_subvalue (func_val, 1, arg_val);
  
  return rul__sem_return_value (return_to_val, func_val,
				shape_atom, dom_symbol, on_lhs, NULL);
}





/********************************
**                             **
**  RUL__SEM_CHECK_BTI_LENGTH  **
**                             **
********************************/

Value rul__sem_check_bti_length (Ast_Node ast,
				 Boolean on_lhs, long ce_index,
				 Value return_to_val)
{
  Ast_Node      node;
  Value         func_val, func1_val, arg_val;

  /* length value */

  node = rul__ast_get_child(ast);
  arg_val = rul__sem_check_nested_value (
			node, on_lhs, ce_index, SEM__C_RETURN_RUL_TYPE,
			dom_any, shape_compound, NULL);
  if (arg_val == NULL)
    return NULL;

#ifndef NDEBUG
  func_val = rul__val_create_function_str ("rul__mol_get_poly_count", 1), 
#else
  func_val = rul__val_create_function_str ("MGPC", 1), 
#endif
  rul__val_set_nth_subvalue (func_val, 1, arg_val);

  func1_val = rul__val_create_function_str ("rul__cvt_s_lo_s_in", 1), 
  rul__val_set_nth_subvalue (func1_val, 1, func_val);

  return rul__sem_return_value (return_to_val, func1_val,
				shape_atom, dom_int_atom, on_lhs, NULL);
}




/********************************
**                             **
**  RUL__SEM_CHECK_BTI_MAX     **
**                             **
********************************/

Value rul__sem_check_bti_max (Ast_Node ast, Boolean is_max,
			      Boolean on_lhs, long ce_index,
			      Value return_to_val)
{
  Ast_Node      node;
  Value	        func_val, arg_val;
  long          arg_cnt = rul__ast_get_child_count (ast);
  long          i;
  Decl_Domain   ret_dom = dom_invalid;

  if (arg_cnt == 0) {
    if (is_max)
      rul__msg_cmp_print (CMP_ZERMAXMIN, ast, "MAX");
    else
      rul__msg_cmp_print (CMP_ZERMAXMIN, ast, "MIN");
    func_val = rul__val_create_rul_constant (rul__mol_symbol_nil ());
    ret_dom = dom_symbol;
  }
  else {

    /* create function_str with argument count */
    func_val = rul__val_create_function_str ("rul__mol_max_min", arg_cnt + 2);

    /* make a Value for the boolean max/min */
    arg_val = rul__val_create_long_animal (is_max);
    rul__val_set_nth_subvalue (func_val, 1, arg_val);

    /* make a Value from the arg count */
    arg_val = rul__val_create_long_animal (arg_cnt);
    rul__val_set_nth_subvalue (func_val, 2, arg_val);

    for (i = 3, node = rul__ast_get_child(ast), arg_cnt += 3;
	 i < arg_cnt;
	 i++, node = rul__ast_get_sibling(node)) {
      arg_val = rul__sem_check_nested_value (node, on_lhs, ce_index,
					     SEM__C_RETURN_RUL_TYPE,
					     dom_any, shape_molecule, NULL);
    
      if (arg_val == NULL) {
	rul__val_free_value(func_val);
	return NULL;
      }
      else {
	if (ret_dom == dom_invalid)
	  ret_dom = rul__val_get_domain (arg_val);
	rul__val_set_nth_subvalue (func_val, i, arg_val);
      }
    }
  }

  return rul__sem_return_value (return_to_val, func_val, 
				shape_atom, ret_dom, on_lhs, NULL);
}




/*****************************
**                          **
**  RUL__SEM_CHECK_BTI_NTH  **
**                          **
*****************************/

Value rul__sem_check_bti_nth (Ast_Node ast,
			      Boolean on_lhs, long ce_index,
			      Value return_to_val)
{
  Ast_Node      node;
  Ast_Node_Type node_type;
  Value         func_val, arg_val = NULL, comp_val;
  Molecule	index;

  /* nth compound-value index */

  /* check the compound-value */
  node = rul__ast_get_child(ast);
  comp_val = rul__sem_check_nested_value (node, on_lhs, ce_index,
					   SEM__C_RETURN_RUL_TYPE,
					   dom_any, shape_compound, NULL);
  if (comp_val == NULL)
    return NULL;

  /* check the index */
  node = rul__ast_get_sibling(node);
  node_type = rul__ast_get_type (node);
  if (node_type == AST__E_CONSTANT ||
      node_type == AST__E_DOLLAR_LAST) {

#ifndef NDEBUG
    func_val = rul__val_create_function_str ("rul__mol_get_comp_nth_rt", 2), 
#else
    func_val = rul__val_create_function_str ("MGCNR", 2), 
#endif
    rul__val_set_nth_subvalue (func_val, 1, comp_val);

    if (node_type == AST__E_CONSTANT) {
      index = rul__ast_get_value (node);
      if (!rul__mol_is_int_atom (index)) {
	rul__msg_cmp_print_w_atoms (CMP_INVCMPIDX, node, 1, index);
	rul__val_free_value (comp_val);
	return NULL;
      }
      if (rul__mol_int_atom_value (index) < 1) {
	rul__msg_cmp_print_w_atoms (CMP_INVCMPIDX, node, 1, index);
	rul__val_free_value (comp_val);
	return NULL;
      }
      arg_val = rul__sem_check_nested_value (node, on_lhs, ce_index,
					      SEM__C_RETURN_EXT_TYPE,
					      dom_int_atom, shape_atom, NULL);
    }
    else {
#ifndef NDEBUG
      arg_val = rul__val_create_function_str ("rul__mol_get_poly_count_last",
#else
      arg_val = rul__val_create_function_str ("MGPCL",
#endif
					      1);
      rul__val_set_nth_subvalue (arg_val, 1, rul__val_copy_value (comp_val));
    }
  }
  /* some variable or expr... */
  else {
    func_val = rul__val_create_function_str (
#ifndef NDEBUG
					"rul__mol_get_comp_nth_mol_rt", 2);
#else
					"MGCNMR", 2);
#endif
    rul__val_set_nth_subvalue (func_val, 1, comp_val);
    arg_val = rul__sem_check_nested_value (node, on_lhs, ce_index, 
					SEM__C_RETURN_RUL_TYPE,
					dom_int_atom, shape_atom, NULL);
  }

  if (arg_val == NULL) {
    rul__val_free_value (func_val);
    return NULL;
  }

  rul__val_set_nth_subvalue (func_val, 2, arg_val);

  return rul__sem_return_value (return_to_val, func_val,
				shape_atom, 
				rul__val_get_domain (comp_val),
				on_lhs, 
				rul__val_get_class (comp_val));
}




/**********************************
**                               **
**  RUL__SEM_CHECK_BTI_POSITION  **
**                               **
**********************************/

Value rul__sem_check_bti_position (Ast_Node ast,
				   Boolean on_lhs, long ce_index,
				   Value return_to_val)
{
  Ast_Node      node;
  Value         func_val, func1_val, arg_val;
  Pred_Type     pred;

  /* position compound-var [predicate] scalar-value */

  /* check the compound-value */
  node = rul__ast_get_child(ast);
  arg_val = rul__sem_check_nested_value (node, on_lhs, ce_index,
					 SEM__C_RETURN_RUL_TYPE,
					 dom_any, shape_compound, NULL);
  if (arg_val == NULL)
    return NULL;

  func_val = rul__val_create_function_str ("rul__mol_position_in_comp", 3);
  rul__val_set_nth_subvalue (func_val, 1, arg_val);

  /* set the predicate function name */
  node = rul__ast_get_sibling(node);
  pred = (Pred_Type) rul__ast_get_long_value (node);
  if (pred == PRED__E_EQ || pred == PRED__E_NOT_A_PRED)
    arg_val = rul__val_create_blk_or_loc (CMP__E_INT_MOLECULE, "NULL");
  else
    arg_val = rul__val_create_blk_or_loc (CMP__E_INT_MOLECULE,
					  rul__gen_pred_func_name (pred));
  rul__val_set_nth_subvalue (func_val, 2, arg_val);
 

  /* check the scalar to look for */
  node = rul__ast_get_sibling(node);
  arg_val = rul__sem_check_nested_value (node, on_lhs, ce_index,
					 SEM__C_RETURN_RUL_TYPE,
					 dom_any, shape_atom, NULL);
  if (arg_val == NULL) {
    rul__val_free_value (func_val);
    return NULL;
  }

  rul__val_set_nth_subvalue (func_val, 3, arg_val);

  func1_val = rul__val_create_function_str ("rul__cvt_s_lo_s_in", 1), 
  rul__val_set_nth_subvalue (func1_val, 1, func_val);

  return rul__sem_return_value (return_to_val, func1_val,
				shape_atom, dom_int_atom, on_lhs, NULL);
}




/*******************************
**                            **
**  RUL__SEM_CHECK_BTI_RJUST  **
**                            **
*******************************/

Value rul__sem_check_bti_rjust (Ast_Node ast)
{
  Value         func_val, arg_val, mol_val;
  Ast_Node      node;

  /* rjust value */

  if (rul__ast_get_type(rul__ast_get_parent(ast)) != AST__E_WRITE_ACTION) {
    rul__msg_cmp_print (CMP_INVRJUUSA, ast);
    return NULL;
  }

  node = rul__ast_get_child(ast);
  arg_val = rul__sem_check_nested_value (
				node, SEM__C_ON_RHS, SEM__C_UNDEFINED_CE_INDEX,
				SEM__C_RETURN_RUL_TYPE,
				dom_int_atom, shape_atom, NULL);
  if (arg_val == NULL)
    return NULL;

  if (bti_SM__rjust == NULL)
    bti_SM__rjust = rul__mol_make_symbol ("$RJUST");

  mol_val = rul__val_create_unnamed_mol_var (
				rul__sem_register_temp_var (SEM__C_ON_RHS));
  rul__val_set_shape_and_domain (mol_val, shape_compound, dom_any);
  rul__ast_attach_value (ast, AST__E_TYPE_VALUE, mol_val);

#ifndef NDEBUG
  func_val = rul__val_create_function_str ("rul__mol_make_comp", 3);
#else
  func_val = rul__val_create_function_str ("MMC", 3);
#endif
  rul__val_set_nth_subvalue (func_val, 1,
			     rul__val_create_long_animal (2));
  rul__val_set_nth_subvalue (func_val, 2,
			     rul__val_create_rul_constant (bti_SM__rjust));
  rul__val_set_nth_subvalue (func_val, 3, arg_val);

  return rul__val_create_assignment (rul__val_copy_value (mol_val), func_val);
}



/*************************************
**                                  **
**  RUL__SEM_CHECK_BTI_SUBCOMPOUND  **
**                                  **
*************************************/

Value rul__sem_check_bti_subcompound (Ast_Node ast, Boolean on_lhs, 
				      long ce_index, Value return_to_val)
{
  Ast_Node      node;
  Ast_Node_Type node_type;
  Value         func_val, comp_val, arg_val, index_val;
  Mol_Int_Atom	index;

  /* subcompound compound-value index-1 index-2 */

  /* check the compound value */
  node = rul__ast_get_child(ast);
  comp_val = rul__sem_check_nested_value (
				node, on_lhs, ce_index, SEM__C_RETURN_RUL_TYPE,
				dom_any, shape_compound, NULL);
  if (comp_val == NULL)
    return NULL;
  
  func_val = rul__val_create_function_str ("rul__mol_subcomp", 3);
  rul__val_set_nth_subvalue (func_val, 1, comp_val);
  
  /* check index 1 */
  node = rul__ast_get_sibling(node);
  node_type = rul__ast_get_type (node);
  if (node_type == AST__E_CONSTANT) {
    index = rul__ast_get_value (node);
    if (!rul__mol_is_int_atom (index)) {
      rul__msg_cmp_print_w_atoms (CMP_INVCMPIDX, node, 1, index);
      rul__val_free_value (func_val);
      return NULL;
    }
    if (rul__mol_int_atom_value (index) < 1) {
      rul__msg_cmp_print_w_atoms (CMP_INVCMPIDX, node, 1, index);
      rul__val_free_value (func_val);
      return NULL;
    }
    arg_val = rul__val_create_long_animal (rul__mol_int_atom_value (index));
  }
  /* in case of $LAST .... */
  else if (node_type == AST__E_DOLLAR_LAST) {
#ifndef NDEBUG
    arg_val = rul__val_create_function_str ("rul__mol_get_poly_count_last", 1);
#else
    arg_val = rul__val_create_function_str ("MGPCL", 1);
#endif
    rul__val_set_nth_subvalue (arg_val, 1, rul__val_copy_value (comp_val));
  }
  /* some variable... */
  else {
    index_val = rul__sem_check_nested_value(
				node, on_lhs, ce_index, SEM__C_RETURN_RUL_TYPE,
				dom_int_atom, shape_atom, NULL);
    if (index_val == NULL) {
      rul__val_free_value (func_val);
      return NULL;
    }
    arg_val = rul__sem_convert_to_ext_value (index_val, ext_type_long,
					     0, NULL, ext_mech_value, NULL,
					     on_lhs, node);
    if (arg_val == NULL) {
      rul__val_free_value (func_val);
      return NULL;
    }
  }
  rul__val_set_nth_subvalue (func_val, 2, arg_val);
  
  /* check index 2 */
  node = rul__ast_get_sibling(node);
  node_type = rul__ast_get_type (node);
  if (node_type == AST__E_CONSTANT) {
    index = rul__ast_get_value (node);
    if (!rul__mol_is_int_atom (index)) {
      rul__msg_cmp_print_w_atoms (CMP_INVCMPIDX, node, 1, index);
      rul__val_free_value (func_val);
      return NULL;
    }
    if (rul__mol_int_atom_value (index) < 1) {
      rul__msg_cmp_print_w_atoms (CMP_INVCMPIDX, node, 1, index);
      rul__val_free_value (func_val);
      return NULL;
    }
    arg_val = rul__val_create_long_animal (rul__mol_int_atom_value (index));
  }
  /* in case of $LAST .... */
  else if (node_type == AST__E_DOLLAR_LAST) {
#ifndef NDEBUG
    arg_val = rul__val_create_function_str ("rul__mol_get_poly_count_last", 1);
#else
    arg_val = rul__val_create_function_str ("MGPCL", 1);
#endif
    rul__val_set_nth_subvalue (arg_val, 1, rul__val_copy_value (comp_val));
  }
  /* some variable... */
  else {
    index_val = rul__sem_check_nested_value (
				node, on_lhs, ce_index, SEM__C_RETURN_RUL_TYPE,
				dom_int_atom, shape_atom, NULL);
    if (index_val == NULL) {
      rul__val_free_value (func_val);
      return NULL;
    }
    arg_val = rul__sem_convert_to_ext_value (index_val, ext_type_long,
					     0, NULL, ext_mech_value, NULL,
					     on_lhs, node);
    if (arg_val == NULL) {
      rul__val_free_value (func_val);
      return NULL;
    }
  }
  rul__val_set_nth_subvalue (func_val, 3, arg_val);

  return rul__sem_return_value (return_to_val, func_val, 
				shape_compound, 
				rul__val_get_domain (comp_val), 
				on_lhs,
				rul__val_get_class (comp_val));
}



/*************************************
**                                  **
**  RUL__SEM_CHECK_BTI_SUBSYMBOL  **
**                                  **
*************************************/

Value rul__sem_check_bti_subsymbol (Ast_Node ast, Boolean on_lhs, 
				    long ce_index, Value return_to_val)
{
  Ast_Node      node;
  Ast_Node_Type node_type;
  Value         func_val, sym_val, arg_val, index_val;
  Mol_Int_Atom	index;

  /* subsymbol scalar-value index-1 index-2 */

  /* check the compound value */
  node = rul__ast_get_child(ast);
  sym_val = rul__sem_check_nested_value (node, on_lhs, ce_index,
					 SEM__C_RETURN_RUL_TYPE,
					 dom_any, shape_atom, NULL);
  if (sym_val == NULL)
    return NULL;
  
  func_val = rul__val_create_function_str ("rul__mol_subsymbol", 3);
  rul__val_set_nth_subvalue (func_val, 1, sym_val);
  
  /* check index 1 */
  node = rul__ast_get_sibling(node);
  node_type = rul__ast_get_type (node);

  if (node_type == AST__E_CONSTANT) {
    index = rul__ast_get_value (node);

    if (!rul__mol_is_int_atom (index) ||
	(rul__mol_int_atom_value (index) < 1)) {
      rul__msg_cmp_print_w_atoms (CMP_INVSUBSYMIDX, node, 1, index);
      rul__val_free_value (func_val);
      return NULL;
    }
    arg_val = rul__val_create_long_animal (rul__mol_int_atom_value (index));
  }

  /* in case of $LAST .... */
  else if (node_type == AST__E_DOLLAR_LAST) {
    arg_val = rul__val_create_long_animal (-1);
  }

  /* some variable... */
  else {
    index_val = rul__sem_check_nested_value (node, on_lhs, ce_index,
					     SEM__C_RETURN_RUL_TYPE,
					     dom_int_atom, shape_atom, NULL);
    if (index_val == NULL) {
      rul__val_free_value (func_val);
      return NULL;
    }
    arg_val = rul__sem_convert_to_ext_value (index_val, ext_type_long,
					     0, NULL, ext_mech_value, NULL,
					     on_lhs, node);
    if (arg_val == NULL) {
      rul__val_free_value (func_val);
      return NULL;
    }
  }
  rul__val_set_nth_subvalue (func_val, 2, arg_val);
  
  /* check index 2 */
  node = rul__ast_get_sibling(node);
  node_type = rul__ast_get_type (node);

  if (node_type == AST__E_CONSTANT) {
    index = rul__ast_get_value (node);

    if (!rul__mol_is_int_atom (index) ||
	(rul__mol_int_atom_value (index) < 1)) {
      rul__msg_cmp_print_w_atoms (CMP_INVCMPIDX, node, 1, index);
      rul__val_free_value (func_val);
      return NULL;
    }
    arg_val = rul__val_create_long_animal (rul__mol_int_atom_value (index));
  }

  /* in case of $LAST .... */
  else if (node_type == AST__E_DOLLAR_LAST) {
    arg_val = rul__val_create_long_animal (-1);
  }

  /* some variable... */
  else {
    index_val = rul__sem_check_nested_value (node, on_lhs, ce_index,
					     SEM__C_RETURN_RUL_TYPE,
					     dom_int_atom, shape_atom, NULL);
    if (index_val == NULL) {
      rul__val_free_value (func_val);
      return NULL;
    }
    arg_val = rul__sem_convert_to_ext_value (index_val, ext_type_long,
					     0, NULL,  ext_mech_value, NULL,
					     on_lhs, node);
    if (arg_val == NULL) {
      rul__val_free_value (func_val);
      return NULL;
    }
  }
  rul__val_set_nth_subvalue (func_val, 3, arg_val);

  return rul__sem_return_value (return_to_val, func_val, 
				shape_atom, dom_symbol, 
				on_lhs,	rul__val_get_class (sym_val));
}




/********************************
**                             **
**  RUL__SEM_CHECK_BTI_SYMBOL  **
**                             **
********************************/

Value rul__sem_check_bti_symbol (Ast_Node ast,
				 Boolean on_lhs, long ce_index,
				 Value return_to_val)
{
  Ast_Node      node;
  Value         func_val, arg_val;

  /* symbol value */

  node = rul__ast_get_child(ast);
  arg_val = rul__sem_check_nested_value (node, on_lhs, ce_index,
					 SEM__C_RETURN_RUL_TYPE,
					 dom_any, shape_molecule, NULL);
  if (arg_val == NULL)
    return NULL;

  if (rul__val_get_domain (arg_val) != dom_symbol) {
    func_val = rul__val_create_function_str ("rul__cvt_s_at_s_sy", 1);
    rul__val_set_nth_subvalue (func_val, 1, arg_val);
  }
  else
    func_val = arg_val;

  return rul__sem_return_value (return_to_val, func_val, 
				shape_atom, dom_symbol, on_lhs, NULL);
}




/************************************
**                                 **
**  RUL__SEM_CHECK_BTI_SYMBOL_LEN  **
**                                 **
************************************/

Value rul__sem_check_bti_symbol_len (Ast_Node ast,
				     Boolean on_lhs, long ce_index,
				     Value return_to_val)
{
  Ast_Node      node;
  Value         func_val, arg_val;

  /* symbol-length value */

  node = rul__ast_get_child(ast);
  arg_val = rul__sem_check_nested_value (node, on_lhs, ce_index,
					 SEM__C_RETURN_RUL_TYPE,
					 dom_any, shape_molecule, NULL);
  if (arg_val == NULL)
    return NULL;

  func_val = rul__val_create_function_str ("rul__mol_get_printform_length", 1);
  rul__val_set_nth_subvalue (func_val, 1, arg_val);

  return rul__sem_return_value (return_to_val, func_val, 
				shape_atom, dom_int_atom, on_lhs, NULL);
}




/*******************************
**                            **
**  RUL__SEM_CHECK_BTI_TABTO  **
**                            **
*******************************/

Value rul__sem_check_bti_tabto (Ast_Node ast)
{
  Value         func_val, arg_val, mol_val;
  Ast_Node      node;

  /* tabto value */

  if (rul__ast_get_type(rul__ast_get_parent(ast)) != AST__E_WRITE_ACTION) {
    rul__msg_cmp_print (CMP_INVTABUSA, ast);
    return NULL;
  }

  node = rul__ast_get_child(ast);
  arg_val = rul__sem_check_nested_value (node, SEM__C_ON_RHS,
					 SEM__C_UNDEFINED_CE_INDEX, 
					 SEM__C_RETURN_RUL_TYPE,
					 dom_int_atom, shape_atom, NULL);
  if (arg_val == NULL)
    return NULL;

  if (bti_SM__tabto == NULL)
    bti_SM__tabto = rul__mol_make_symbol ("$TABTO");

  mol_val = rul__val_create_unnamed_mol_var (
			     rul__sem_register_temp_var (SEM__C_ON_RHS));
  rul__val_set_shape_and_domain (mol_val, shape_compound, dom_any);
  rul__ast_attach_value (ast, AST__E_TYPE_VALUE, mol_val);

#ifndef NDEBUG
  func_val = rul__val_create_function_str ("rul__mol_make_comp", 3);
#else
  func_val = rul__val_create_function_str ("MMC", 3);
#endif
  rul__val_set_nth_subvalue (func_val, 1,
			     rul__val_create_long_animal (2));
  rul__val_set_nth_subvalue (func_val, 2,
			     rul__val_create_rul_constant (bti_SM__tabto));
  rul__val_set_nth_subvalue (func_val, 3, arg_val);

  return rul__val_create_assignment (rul__val_copy_value (mol_val), func_val);
}



/************************************
**                                 **
**  SEM_UNION_OF_COMP_ARG_DOMAINS  **
**                                 **
************************************/

Decl_Domain  sem_union_of_comp_arg_domains (Value func_val)
{
  Decl_Domain dom;
  long len, i;

  assert (rul__val_is_function (func_val));

  len = rul__val_get_complex_count (func_val);
  if (len < 2) return dom_any;

  dom = rul__val_get_domain (rul__val_get_nth_complex (func_val, 2));
  for (i=3; i<=len; i++) {
    dom = rul__mol_domain_union (dom,
			 rul__val_get_domain (
			      rul__val_get_nth_complex (func_val, i)));
  }
  return dom;
}



/************************************
**                                 **
**  SEM_UNION_OF_COMP_ARG_CLASSES  **
**                                 **
************************************/

Class  sem_union_of_comp_arg_classes (Value func_val)
{
  Class rclass;
  long len, i;

  assert (rul__val_is_function (func_val));
  assert (sem_union_of_comp_arg_domains (func_val) == dom_instance_id);

  len = rul__val_get_complex_count (func_val);
  if (len < 2) return NULL;

  rclass = rul__val_get_class (rul__val_get_nth_complex (func_val, 2));
  for (i=3; i<=len; i++) {
    rclass = rul__decl_lowest_common_class (rclass,
		   rul__val_get_class (
                              rul__val_get_nth_complex (func_val, i)));
  }
  return rclass;
}

