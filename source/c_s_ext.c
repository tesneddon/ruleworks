/* cmp_ext_rt_sem.c - RULEWORKS External Routine Semantic Checking */
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
 *  FACILITY:
 *	RULEWORKS compiler
 *
 *  ABSTRACT:
 *	External Routine Semantic checking routines.  Includes semantic
 *	checks for external routine declarations and routine calls.
 *
 *  MODIFIED BY:
 *	DEC	Digital Equipment Corporation
 *	CPQ	Compaq Computer Corporation
 *
 *  MODIFICATION HISTORY:
 *
 *	22-Dec-1992	DEC	Initial version.
 *
 *	10-Jun-1994	DEC	Set shape and domain of function return, even
 *					for animal types.
 *
 *	16-Feb-1998	DEC	class type changed to rclass
 *
 *	01-Dec-1999	CPQ	Releasew ith GPL
 */


#include <common.h>
#include <cmp_comm.h>
#include <mol.h>
#include <sem.h>
#include <ast.h>
#include <val.h>
#include <decl.h>
#include <gen.h>
#include <msg.h>
#include <cmp_msg.h>


static char *rul__sem_get_type_converter (Decl_Domain rul_type,
					  Decl_Shape rul_shape,
					  Ext_Type ext_type,
					  Cardinality array_len,
					  Boolean converting_rul_to_external,
					  Decl_Domain *ret_dom);

Boolean rul__sem_check_ext_rt_decl (Ast_Node ext_rt_node)
{
  /* 
   * This routine handles the pass 2 (semantic checking) for External
   * routine declarations
   */
  Ast_Node	node, dup_node, param_node;
  Ast_Node_Type type;
  Mol_Symbol    rt_name;
  Boolean	alias_declared  = FALSE;
  Boolean	input_declared  = FALSE;
  Boolean	return_declared = FALSE;
  Ext_Rt_Decl   ext_rt = NULL;

  node = rul__ast_get_child (ext_rt_node);
  assert (node != NULL /* A child is always expected here... */);
  rt_name = (Mol_Symbol) rul__ast_get_value (node);
  assert (rt_name != NULL /* A symbol is  always expected here... */);
  
  /* Check to see if an ext rt declaration already exists in this 
   * declaration block */
  if (rul__decl_is_an_ext_rt (rt_name)) {
    rul__msg_cmp_print_w_atoms (CMP_EXTDUPDEF, ext_rt_node, 2, rt_name,
				(Molecule) rul__decl_get_decl_name (
					    rul__decl_get_curr_decl_block ()));
    return FALSE;
  }
  
  else if (rul__decl_get_visible_method (rt_name) != NULL) {
    rul__msg_cmp_print_w_atoms (CMP_EXTDUPDEF, ext_rt_node, 2, rt_name,
				(Molecule) rul__decl_get_decl_name (
					    rul__decl_get_curr_decl_block ()));
    return FALSE;
  }
  
  rul__decl_create_ext_rt (rt_name);
  
  /* Check the three valid types of child nodes (alias, input parameters,
   * or return parameter) */  
  for (node = rul__ast_get_sibling (node);
       node != NULL;
       node = rul__ast_get_sibling (node)) {

    type = rul__ast_get_type (node);
    if (type == AST__E_ALIAS_DECL) {
      if (alias_declared) {
	rul__msg_cmp_print_w_atoms (CMP_EXTDUPALIAS, node, 1, rt_name);
      }
      else {
	alias_declared = TRUE;
	rul__decl_add_ext_rt_alias ((Mol_Symbol) rul__ast_get_value (
					     rul__ast_get_child (node)));
      }
    }

    else if (type == AST__E_ACCEPTS_DECL) {
      if (input_declared) {
	rul__msg_cmp_print_w_atoms (CMP_EXTDUPINP, node, 1, rt_name);
      }
      else {
	input_declared = TRUE;
	for (param_node = rul__ast_get_child (node);
	     param_node != NULL;
	     param_node = rul__ast_get_sibling (param_node)) {
	  assert (rul__ast_get_type(param_node) == AST__E_ACCEPTS_PARAM);
	  if (!rul__sem_check_ext_rt_param (rul__ast_get_child (param_node),
					    rt_name,
					    SEM__C_ROUTINE_ARGUMENT)) {
	    rul__decl_destroy_ext_rt (rul__decl_get_current_rt ());
	    return FALSE;
	  }
	}
      }
    }
    else if (type == AST__E_RETURNS_DECL) {
      if (return_declared) {
	rul__msg_cmp_print_w_atoms (CMP_EXTDUPRET, node, 1, rt_name);
      }
      else {
	return_declared = TRUE;
	if (!rul__sem_check_ext_rt_param (rul__ast_get_child (node), 
					  rt_name,
					  SEM__C_RETURN_PARAMETER)) {
	  rul__decl_destroy_ext_rt (rul__decl_get_current_rt ());
	  return FALSE; /* bogus return value found */
	}
      }
    }
    else {
      assert (FALSE /* Invalid node in External Routine Decl */);
      rul__decl_destroy_ext_rt (rul__decl_get_current_rt ());
      return FALSE;
    }
  }
  return TRUE;
}

Boolean rul__sem_check_ext_rt_param (Ast_Node node,
				     Mol_Symbol rt_name, 
				     Boolean is_the_return_value)
{
  Ast_Node      node1, node2, node3;
  Ast_Node_Type node_type;
  Ext_Type	ext_type = ext_type_long, par_type;
  Ext_Mech      ext_mech = ext_mech_value;	/* default value */
  Cardinality	a_len = 0;			/* default value */
  Mol_Symbol    a_arg = NULL;
  Mol_Symbol    param_name = NULL;
  long          i;
  Ext_Rt_Decl   ext_rt;

  /* get param type */
  assert (rul__ast_get_value_type (node) == AST__E_TYPE_EXT_TYPE);
  ext_type = (Ext_Type) rul__ast_get_long_value (node);

  /* adjust mechanism default value */
  if (ext_type == ext_type_asciz || ext_type == ext_type_ascid)
    ext_mech = ext_mech_ref_ro;

  node1 = rul__ast_get_sibling (node);

  if (node1 != NULL) {

    node_type = rul__ast_get_type (node1);
    if (node_type == AST__E_EXT_LEN) {
      /* Array length was specified */
      
      assert (rul__ast_get_value_type(node1) == AST__E_TYPE_CARDINALITY ||
	      rul__ast_get_value_type(node1) == AST__E_TYPE_MOL);

      if (rul__ast_get_value_type (node1) == AST__E_TYPE_CARDINALITY) {
	a_len = (Cardinality) rul__ast_get_long_value (node1);
      }
      else {
	a_arg = rul__ast_get_value (node1);
	a_len = EXT__C_IMPLICIT_ARRAY;
      }
      if (a_len != EXT__C_NOT_ARRAY)
	ext_mech = ext_mech_ref_ro;

      node2 = rul__ast_get_sibling (node1);
      if (node2 != NULL) {

	node_type = rul__ast_get_type(node2);
	if (node_type == AST__E_EXT_MECH) {

	  /* Mechanism and array length were specified */
	  assert (rul__ast_get_value_type (node2) == AST__E_TYPE_EXT_MECH);
	  ext_mech = (Ext_Mech) rul__ast_get_long_value (node2);

	  node3 = rul__ast_get_sibling (node2);
	  if (node3 != NULL && rul__ast_get_type (node3) == AST__E_VARIABLE) {
	    param_name = rul__ast_get_value (node3);
	  }
	}
	else if (node_type == AST__E_VARIABLE) {
	  param_name = rul__ast_get_value (node2);
	}
      }
    }
    else if (node_type == AST__E_EXT_MECH) {
      /* Length was defaulted, mechanism was specified...
       * Add the default array length as an explicit node */

      assert (rul__ast_get_value_type (node1) == AST__E_TYPE_EXT_MECH);
      ext_mech = (Ext_Mech) rul__ast_get_long_value (node1);

      node3 = rul__ast_get_sibling (node1);
      if (node3 != NULL && rul__ast_get_type (node3) == AST__E_VARIABLE) {
	param_name = rul__ast_get_value (node3);
      }
    }
    else if (node_type == AST__E_VARIABLE) {
      param_name = rul__ast_get_value (node1);
    }
  }

  if (a_len == EXT__C_IMPLICIT_ARRAY) {
    if (a_arg == NULL) {
      if (is_the_return_value || ext_mech == ext_mech_ref_rw) {
	rul__msg_cmp_print_w_atoms (CMP_EXTINVARR, node1, 1, rt_name);
	return FALSE;
      }
    }
    else {
      ext_rt = rul__decl_get_current_rt ();
      i = rul__decl_ext_rt_param_index (ext_rt, a_arg);
      if (i == 0) {
	rul__msg_cmp_print_w_atoms (CMP_EXTUNDPAR, node1, 1, a_arg);
	return FALSE;
      }
      par_type = rul__decl_ext_rt_param_type (ext_rt, i);
      if ((par_type != ext_type_long &&
	   par_type != ext_type_short &&
	   par_type != ext_type_byte &&
	   par_type != ext_type_uns_long &&
	   par_type != ext_type_uns_short &&
	   par_type != ext_type_uns_byte) ||
	  (rul__decl_ext_rt_param_a_len (ext_rt, i) != EXT__C_NOT_ARRAY)) {
	rul__msg_cmp_print_w_atoms (CMP_EXTINVTYP, node1, 1, a_arg);
	return FALSE;
      }
    }
  }

  if ((ext_mech == ext_mech_value) && (a_len != EXT__C_NOT_ARRAY)) {
    rul__msg_cmp_print_w_atoms (CMP_EXTINVBYVAL, node, 1, rt_name);
    return FALSE;
  }
  
  if (is_the_return_value) {
    rul__decl_add_ext_rt_ret_val (ext_type, a_len, ext_mech, a_arg);
  }
  else {
    rul__decl_add_ext_rt_param (ext_type, a_len, ext_mech,
				param_name, a_arg);
  }
  
  return TRUE;
}




Boolean rul__sem_ext_type_convertible (Ext_Type ext_type,
				       Cardinality array_len,
				       Decl_Domain rul_type,
				       Decl_Shape rul_shape,
				       Boolean converting_rul_to_external)
{

  if ((rul_shape != shape_molecule) &&
      ((array_len != 0 && rul_shape == shape_atom) ||
       (array_len == 0 && rul_shape != shape_atom)))
    /* scaler/multivalue type mismatch */
    return (FALSE);

  else if ((rul_shape == shape_atom) ||
	   (rul_shape == shape_molecule && array_len == 0)) {
	/* Scalar type conversion */
    if ((ext_type == ext_type_long) ||
	(ext_type == ext_type_short) ||
	(ext_type == ext_type_byte) ||
	(ext_type == ext_type_uns_long) ||
	(ext_type == ext_type_uns_short) ||
	(ext_type == ext_type_uns_byte) ||
	(ext_type == ext_type_float) ||
	(ext_type == ext_type_double)) {
      if ((rul_type == dom_int_atom) ||
	  (rul_type == dom_dbl_atom) ||
	  (rul_type == dom_any) ||
	  (rul_type == dom_number))
	return (TRUE);
      else
	return (FALSE);
    }
    else if (ext_type == ext_type_atom)
      return TRUE;
    else if ((ext_type == ext_type_asciz) || (ext_type == ext_type_ascid)) {
      if ((rul_type == dom_symbol) ||
	  (rul_type == dom_any) ||
	  (converting_rul_to_external))
	return (TRUE);
      else return (FALSE);
    }
    else if (ext_type == ext_type_void_ptr)
      if (rul_type == dom_opaque || rul_type == dom_any) return (TRUE); 
      else return (FALSE);
    else
      /* System Bug -- Undefined external type */
      return (FALSE);
  }
  else if ((rul_shape == shape_compound) ||
	   (rul_shape == shape_molecule && array_len != 0)) {
    /* Compound type conversion */
    
    if (rul_type == dom_any)
      return (TRUE);

    if ((((ext_type == ext_type_long) ||
	  (ext_type == ext_type_short) ||
	  (ext_type == ext_type_byte) ||
	  (ext_type == ext_type_uns_long) ||
	  (ext_type == ext_type_uns_short) ||
	  (ext_type == ext_type_uns_byte) ||
	  (ext_type == ext_type_float) ||
	  (ext_type == ext_type_double)) && 
	 ((rul_type == dom_number) ||
	  (rul_type == dom_int_atom) ||
	  (rul_type == dom_dbl_atom))) ||
	(((ext_type == ext_type_asciz) ||
	  (ext_type == ext_type_ascid) ||
	  (ext_type == ext_type_atom)) &&
	 ((rul_type == dom_number) ||
	  (rul_type == dom_int_atom) ||
	  (rul_type == dom_dbl_atom) ||
	  (rul_type == dom_symbol) ||
	  (rul_type == dom_instance_id) ||
	  (rul_type == dom_opaque))))
      /*
	((ext_type == ext_type_void_ptr) ||
	(ext_type == ext_type_atom_ptr) ||
	(ext_type == ext_type_object)) */
      return (TRUE);
    else
      /* System Bug -- Undefined external type */
      return (FALSE);
  }
  else
    /* Undefined conversion */
    return (FALSE);
}

static char *rul__sem_get_type_converter (Decl_Domain rul_type,
					  Decl_Shape rul_shape,
					  Ext_Type ext_type,
					  Cardinality array_len,
					  Boolean converting_rul_to_external,
					  Decl_Domain *ret_dom)
{
  char *invalid_converter = "Invalid Conversion";

  if ((rul_shape == shape_molecule) && (array_len == 0))
    rul_shape = shape_atom;

  if ((rul_shape == shape_molecule) && (array_len != 0))
    rul_shape = shape_compound;

  *ret_dom = rul_type;

  if (rul_shape == shape_atom) {
	/*
	 * Scalar type conversion
	 */
    switch (ext_type)
      {
      case ext_type_long:
	*ret_dom = dom_int_atom;
	switch (rul_type)
	  {
	  case dom_any:
	    if (converting_rul_to_external)
	      return "rul__cvt_s_at_s_lo";
	    else
	      return "rul__cvt_s_lo_s_in";
	  case dom_number:
	    if (converting_rul_to_external)
	      return "rul__cvt_s_at_s_lo";
	    else
	      return "rul__cvt_s_lo_s_in";
	  case dom_int_atom:
	    if (converting_rul_to_external)
	      return "rul__cvt_s_in_s_lo";
	    else
	      return "rul__cvt_s_lo_s_in";
	  case dom_dbl_atom:
	    *ret_dom = dom_dbl_atom;
	    if (converting_rul_to_external)
	      return "rul__cvt_s_db_s_lo";
	    else
	      return "rul__cvt_s_lo_s_db";
	  default:
	    return invalid_converter;
	  }
      case ext_type_short:
	*ret_dom = dom_int_atom;
	switch (rul_type)
	  {
	  case dom_any:
	    if (converting_rul_to_external)
	      return "rul__cvt_s_at_s_sh";
	    else
	      return "rul__cvt_s_sh_s_in";
	  case dom_number:
	    if (converting_rul_to_external)
	      return "rul__cvt_s_at_s_sh";
	    else
	      return "rul__cvt_s_sh_s_in";
	  case dom_int_atom:
	    if (converting_rul_to_external)
	      return "rul__cvt_s_in_s_sh";
	    else
	      return "rul__cvt_s_sh_s_in";
	  case dom_dbl_atom:
	    *ret_dom = dom_dbl_atom;
	    if (converting_rul_to_external)
	      return "rul__cvt_s_db_s_sh";
	    else
	      return "rul__cvt_s_sh_s_db";
	  default:
	    return invalid_converter;
	  }
      case ext_type_byte:
	*ret_dom = dom_int_atom;
	switch (rul_type)
	  {
	  case dom_any:
	    if (converting_rul_to_external)
	      return "rul__cvt_s_at_s_by";
	    else
	      return "rul__cvt_s_by_s_in";
	  case dom_number:
	    if (converting_rul_to_external)
	      return "rul__cvt_s_at_s_by";
	    else
	      return "rul__cvt_s_by_s_in";
	  case dom_int_atom:
	    if (converting_rul_to_external)
	      return "rul__cvt_s_in_s_by";
	    else
	      return "rul__cvt_s_by_s_in";
	  case dom_dbl_atom:
	    *ret_dom = dom_dbl_atom;
	    if (converting_rul_to_external)
	      return "rul__cvt_s_db_s_by";
	    else
	      return "rul__cvt_s_by_s_db";
	  default:
	    return invalid_converter;
	  }
      case ext_type_uns_long:
	*ret_dom = dom_int_atom;
	switch (rul_type)
	  {
	  case dom_any:
	    if (converting_rul_to_external)
	      return "rul__cvt_s_at_s_ul";
	    else
	      return "rul__cvt_s_ul_s_in";
	  case dom_number:
	    if (converting_rul_to_external)
	      return "rul__cvt_s_at_s_ul";
	    else
	      return "rul__cvt_s_ul_s_in";
	  case dom_int_atom:
	    if (converting_rul_to_external)
	      return "rul__cvt_s_in_s_ul";
	    else
	      return "rul__cvt_s_l__s_in";
	  case dom_dbl_atom:
	    *ret_dom = dom_dbl_atom;
	    if (converting_rul_to_external)
	      return "rul__cvt_s_db_s_ul";
	    else
	      return "rul__cvt_s_ul_s_db";
	  default:
	    return invalid_converter;
	  }
      case ext_type_uns_short:
	*ret_dom = dom_int_atom;
	switch (rul_type)
	  {
	  case dom_any:
	    if (converting_rul_to_external)
	      return "rul__cvt_s_at_s_us";
	    else
	      return "rul__cvt_s_us_s_in";
	  case dom_number:
	    if (converting_rul_to_external)
	      return "rul__cvt_s_at_s_us";
	    else
	      return "rul__cvt_s_us_s_in";
	  case dom_int_atom:
	    if (converting_rul_to_external)
	      return "rul__cvt_s_in_s_us";
	    else
	      return "rul__cvt_s_us_s_in";
	  case dom_dbl_atom:
	    *ret_dom = dom_dbl_atom;
	    if (converting_rul_to_external)
	      return "rul__cvt_s_db_s_us";
	    else
	      return "rul__cvt_s_us_s_db";
	  default:
	    return invalid_converter;
	  }
      case ext_type_uns_byte:
	*ret_dom = dom_int_atom;
	switch (rul_type)
	  {
	  case dom_any:
	    if (converting_rul_to_external)
	      return "rul__cvt_s_at_s_ub";
	    else
	      return "rul__cvt_s_ub_s_in";
	  case dom_number:
	    if (converting_rul_to_external)
	      return "rul__cvt_s_at_s_ub";
	    else
	      return "rul__cvt_s_ub_s_in";
	  case dom_int_atom:
	    if (converting_rul_to_external)
	      return "rul__cvt_s_in_s_ub";
	    else
	      return "rul__cvt_s_ub_s_in";
	  case dom_dbl_atom	:
	    *ret_dom = dom_dbl_atom;
	    if (converting_rul_to_external)
	      return "rul__cvt_s_db_s_ub";
	    else
	      return "rul__cvt_s_ub_s_db";
	  default:
	    return invalid_converter;
	  }
      case ext_type_float:
	*ret_dom = dom_dbl_atom;
	switch (rul_type)
	  {
	  case dom_any:
	    if (converting_rul_to_external)
	      return "rul__cvt_s_at_s_sf";
	    else
	      return "rul__cvt_s_sf_s_db";
	  case dom_number:
	    if (converting_rul_to_external)
	      return "rul__cvt_s_at_s_sf";
	    else
	      return "rul__cvt_s_sf_s_db";
	  case dom_int_atom:
	    *ret_dom = dom_int_atom;
	    if (converting_rul_to_external)
	      return "rul__cvt_s_in_s_sf";
	    else
	      return "rul__cvt_s_sf_s_in";
	  case dom_dbl_atom:
	    if (converting_rul_to_external)
	      return "rul__cvt_s_db_s_sf";
	    else
	      return "rul__cvt_s_sf_s_db";
	  default:
	    return invalid_converter;
	  }
      case ext_type_double:
	*ret_dom = dom_dbl_atom;
	switch (rul_type)
	  {
	  case dom_any:
	    if (converting_rul_to_external)
	      return "rul__cvt_s_at_s_df";
	    else
	      return "rul__cvt_s_df_s_db";
	  case dom_number:
	    if (converting_rul_to_external)
	      return "rul__cvt_s_at_s_df";
	    else
	      return "rul__cvt_s_df_s_db";
	  case dom_int_atom:
	    if (converting_rul_to_external)
	      return "rul__cvt_s_in_s_df";
	    else {
	      *ret_dom = dom_int_atom;
	      return "rul__cvt_s_df_s_in"; }
	  case dom_dbl_atom:
	    if (converting_rul_to_external)
	      return "rul__cvt_s_db_s_df";
	    else
	      return "rul__cvt_s_df_s_db";
	  default:
	    return invalid_converter;
	  }
      case ext_type_asciz:
	*ret_dom = dom_symbol;
	if ((rul_type == dom_symbol || rul_type == dom_any) &&
	    (! converting_rul_to_external))
	  return "rul__cvt_s_az_s_sy";
	else if (converting_rul_to_external)
	  return "rul__cvt_s_at_s_az";
	else 
	  return invalid_converter;
      case ext_type_ascid:
	*ret_dom = dom_symbol;
	if ((rul_type == dom_symbol || rul_type == dom_any) && 
	    (! converting_rul_to_external))
	  return "rul__cvt_s_ad_s_sy";
	else if (converting_rul_to_external)
	  return "rul__cvt_s_at_s_ad";
	else 
	  return invalid_converter;
      case ext_type_void_ptr:
	*ret_dom = dom_opaque;
	if ((rul_type == dom_opaque || rul_type == dom_any) &&
	    (! converting_rul_to_external))
	  return "rul__cvt_s_vd_s_op";
	else if (converting_rul_to_external)
	  return "rul__cvt_s_op_s_vd";
	else
	  return invalid_converter;
      case ext_type_atom:
	if (converting_rul_to_external)
	  return "rul__cvt_s_at_s_ta";
	else
	  switch (rul_type)
	    {
	    case dom_any:
	      return "rul__cvt_s_ta_s_at";
	    case dom_number:
	      return "rul__cvt_s_ta_s_na";
	    case dom_int_atom:
	      return "rul__cvt_s_ta_s_in";
	    case dom_dbl_atom:
	      return "rul__cvt_s_ta_s_db";
	    case dom_symbol:
	      return "rul__cvt_s_ta_s_sy";
	    case dom_opaque: 
	      return "rul__cvt_s_ta_s_op";
	    case dom_instance_id:
	      return "rul__cvt_s_ta_s_id";
	    }
      }
  }
  else if (rul_shape == shape_compound) {
    /*
     * Compound type conversion
     */
    switch (ext_type)
      {
      case ext_type_long:
	*ret_dom = dom_int_atom;
	if (converting_rul_to_external)
	  return "rul__cvt_c_at_a_lo";
	else
	  return "rul__cvt_a_lo_c_in";
      case ext_type_short:
	*ret_dom = dom_int_atom;
	if (converting_rul_to_external)
	  return "rul__cvt_c_at_a_sh";
	else
	  return "rul__cvt_a_sh_c_in";
      case ext_type_byte:
	*ret_dom = dom_int_atom;
	if (converting_rul_to_external)
	  return "rul__cvt_c_at_a_by";
	else
	  return "rul__cvt_a_by_c_in";
      case ext_type_uns_long:
	*ret_dom = dom_int_atom;
	if (converting_rul_to_external)
	  return "rul__cvt_c_at_a_ul";
	else
	  return "rul__cvt_a_ul_c_in";
      case ext_type_uns_short:
	*ret_dom = dom_int_atom;
	if (converting_rul_to_external)
	  return "rul__cvt_c_at_a_us";
	else
	  return "rul__cvt_a_us_c_in";
      case ext_type_uns_byte:
	*ret_dom = dom_int_atom;
	if (converting_rul_to_external)
	  return "rul__cvt_c_at_a_ub";
	else
	  return "rul__cvt_a_ub_c_in";
      case ext_type_float:
	*ret_dom = dom_dbl_atom;
	if (converting_rul_to_external)
	  return "rul__cvt_c_at_a_sf";
	else
	  return "rul__cvt_a_sf_c_db";
      case ext_type_double:
	*ret_dom = dom_dbl_atom;
	if (converting_rul_to_external)
	  return "rul__cvt_c_at_a_df";
	else
	  return "rul__cvt_a_df_c_db";
      case ext_type_asciz:
	*ret_dom = dom_symbol;
	if (converting_rul_to_external)
	  return "rul__cvt_c_at_a_az";
	else
	  return "rul__cvt_a_az_c_sy";
      case ext_type_ascid:
	*ret_dom = dom_symbol;
	if (converting_rul_to_external)
	  return "rul__cvt_c_at_a_ad";
	else
	  return "rul__cvt_a_ad_c_sy";
      case ext_type_void_ptr:
	*ret_dom = dom_opaque;
	if (converting_rul_to_external)
	  return "rul__cvt_c_at_a_vd";
	else
	  return "rul__cvt_a_vd_c_op";
      case ext_type_atom_ptr:
      case ext_type_atom:
	if (converting_rul_to_external)
	  return "rul__cvt_c_at_a_ta";
	else
	  return "rul__cvt_a_ta_c_at";
      default:
	/* System Bug -- Undefined external type */
	return invalid_converter;
      }
  }
  else
    /* Undefined conversion */
    return invalid_converter;

  return invalid_converter;
}

/*
 * This routine handles the selection of parameters for the message:
 * ---- External Routine Conversion from an <a1> <a2> <a3> to an
 *	<a4> <a5> <a6> is undefined.
 */

void rul__sem_report_convert_error (Ext_Type ext_type, Cardinality array_len,
				    Decl_Domain rul_type, Decl_Shape rul_shape,
				    Boolean converting_rul_to_external,
				    Ast_Node ast)
{
  char *a1 = "Internal";
  char *a2;
  char *a3;
  char *a4 = "External";
  char *a5;
  char *a6;

  switch (rul_shape)
    {
    case shape_molecule:
      a2 = "molecule typed to";
      break;
    case shape_atom:
      a2 = "scalar";
      break;
    case shape_compound:
      a2 = "compound typed to";
      break;
    case shape_table:
      a2 = "table typed to";
      break;
    default:
      a2 = "undefined collector of";
    }
  switch (rul_type)
    {
    case dom_any:
      a3 = "untyped atom";
      break;
    case dom_symbol:
      a3 = "symbol";
      break;
    case dom_instance_id:
      a3 = "instance id atom";
      break;
    case dom_opaque:
      a3 = "opaque atom";
      break;
    case dom_number:
      a3 = "numeric atom";
      break;
    case dom_int_atom:
      a3 = "integer atom";
      break;
    case dom_dbl_atom:
      a3 = "float atom";
      break;
    default:
      a3 = "undefined atomic type";
    }

  if (array_len == 0)
    a5 = "type";
  else 
    a5 = "array of type";

  switch (ext_type)
    {
    case ext_type_long:
      a6 = "long";
      break;
    case ext_type_short:
      a6 = "short";
      break;
    case ext_type_byte:
      a6 = "byte";
      break;
    case ext_type_uns_long:
      a6 = "unsigned long";
      break;
    case ext_type_uns_short:
      a6 = "unsigned short";
      break;
    case ext_type_uns_byte:
      a6 = "unsigned byte";
      break;
    case ext_type_float:
      a6 = "float";
      break;
    case ext_type_double:
      a6 = "double";
      break;
    case ext_type_asciz:
      a6 = "asciz";
      break;
    case ext_type_ascid:
      a6 = "ascid";
      break;
    case ext_type_void_ptr:
      a6 = "void *";
      break;
    case ext_type_atom:
      a6 = "atom";
      break;
    default:
      a6 = "undefined";
    }
  if (converting_rul_to_external)
    rul__msg_cmp_print (CMP_EXTBADCVT, ast, a1, a2, a3, a4, a5, a6);
  else
    rul__msg_cmp_print (CMP_EXTBADCVT, ast, a4, a5, a6, a1, a2, a3);
}

Value rul__sem_convert_to_rul_value (Value ext_value,
				     Value rul_var_value,
				     Ast_Node ast,
				     Value arg_value)
{
  Decl_Domain   domain, ret_dom;
  Decl_Shape    shape;
  Ext_Type 	type;
  Cardinality	a_len;
  Value	        ext_var_value = 0,
		func_value;

  assert (rul__val_is_mol_variable (rul_var_value) ||
	  rul__val_is_rhs_variable (rul_var_value));

  if (rul__val_is_ext_variable (ext_value))
    ext_var_value = ext_value;

  else if (rul__val_is_assignment (ext_value)) {
    ext_var_value = rul__val_get_assignment_to (ext_value);
    assert (rul__val_is_ext_variable (ext_var_value));
  }
  else
    assert (FALSE /* Bogus external value to convert to RUL type */);

  domain = rul__val_get_domain (rul_var_value);
  shape  = rul__val_get_shape (rul_var_value);
  type   = rul__val_get_ext_var_type (ext_var_value);    
  a_len  = rul__val_get_ext_var_arr_len (ext_var_value);    

  if (!rul__sem_ext_type_convertible (type, a_len, domain, shape, 
				      SEM__C_CONVERT_EXT_TO_RUL)) {
    rul__sem_report_convert_error (type, a_len, domain, shape, 
				   SEM__C_CONVERT_EXT_TO_RUL, ast);
    rul__val_free_value (ext_value);
    rul__val_free_value (rul_var_value);
    return NULL; /* Unable to generate code! */
  }
  else {
    /* build value to do the conversion */
    if (a_len == EXT__C_NOT_ARRAY) {
      /* scalar conversion */
      func_value = rul__val_create_function_str (
			 rul__sem_get_type_converter (
				      domain, shape, type, a_len, 
				      SEM__C_CONVERT_EXT_TO_RUL, &ret_dom),
						 1);
      rul__val_set_nth_subvalue (func_value, 1, ext_value);
    }
    else {
      /* conversion to a compound */
      func_value = rul__val_create_function_str (
			 rul__sem_get_type_converter (
				      domain, shape, type, a_len, 
				      SEM__C_NO_MEMORY_ALLOCATED, &ret_dom),
						 2);
      rul__val_set_nth_subvalue (func_value, 1, ext_value);
      if (a_len != EXT__C_IMPLICIT_ARRAY || arg_value == NULL)
	rul__val_set_nth_subvalue (func_value, 2,
				   rul__val_create_long_animal (a_len));
      else {
	assert (arg_value != NULL);
	rul__val_set_nth_subvalue (func_value, 2, arg_value);
      }
    }

    /*  Set the shape and domain for the resulting value expression  */
    if (shape == shape_molecule) {
      if (a_len == EXT__C_NOT_ARRAY) 
        shape = shape_atom;
      else
        shape = shape_compound;
    }
    rul__val_set_shape_and_domain (func_value, shape, ret_dom);

    return rul__val_create_assignment (rul_var_value, func_value);
  }
}

Value rul__sem_convert_to_ext_value (Value rul_value, Ext_Type type,
				     Cardinality a_len, Value a_arg_val,
				     Ext_Mech mech, Value ext_val,
				     Boolean lhs_rt_call, Ast_Node ast)
{
  Decl_Domain domain, ret_dom;
  Decl_Shape  shape;
  Ast_Node    arg_ast;
  Value	      var_val, func_value, cvt_value, arg_val;

  if (rul__val_is_assignment (rul_value))
    var_val = rul__val_get_assignment_to (rul_value);

  else if (rul__val_is_rhs_variable (rul_value) ||
	   rul__val_is_mol_variable (rul_value))
    var_val = rul_value;

  else /* if (rul__val_is_blk_or_local (rul_value)) */
    var_val = rul_value;
#if 0
  else
    assert (FALSE /* Bogus value to convert to external value... */);
#endif

  domain = rul__val_get_domain (var_val);
  shape = rul__val_get_shape (var_val);

  if (!rul__sem_ext_type_convertible (type, a_len, domain, shape, 
				      SEM__C_CONVERT_RUL_TO_EXT)) {
    rul__sem_report_convert_error (type, a_len, domain, shape, 
				   SEM__C_CONVERT_RUL_TO_EXT, ast);
    return NULL; /* Unable to generate code! */
  }

  else {
    /* build value to do the conversion */
    if ((a_len == 0) && 
	(type != ext_type_ascid) &&
	(type != ext_type_asciz)) {
      /* scalar conversion */
      func_value = rul__val_create_function_str (
			 rul__sem_get_type_converter (
				    domain, shape, type, a_len, 
				    SEM__C_CONVERT_RUL_TO_EXT, &ret_dom),
						 1);
      cvt_value = rul__val_create_unnamed_ext_var (
			   rul__sem_register_ext_temp (type, a_len, mech,
						       lhs_rt_call),
						   type, a_len, mech,
						   SEM__C_NO_MEMORY_ALLOCATED);
      rul__val_set_nth_subvalue (func_value, 1, rul_value);
    }
    else if ((a_len == 0) &&
	     ((type == ext_type_ascid) || (type == ext_type_asciz))) {

      /* conversion to string */
      func_value = rul__val_create_function_str (
			   rul__sem_get_type_converter (
					domain, shape, type, a_len, 
					SEM__C_CONVERT_RUL_TO_EXT, &ret_dom),
						   2);
      cvt_value  = rul__val_create_unnamed_ext_var (
			   rul__sem_register_ext_temp (type, a_len, mech,
						       lhs_rt_call),
						    type, a_len, mech,
						    SEM__C_MEMORY_ALLOCATED);
      rul__val_set_nth_subvalue (func_value, 1, rul_value);
      if (ext_val != NULL)
	rul__val_set_nth_subvalue (func_value, 2, ext_val);
      else
	rul__val_set_nth_subvalue (func_value, 2,
				   rul__val_create_null (CMP__E_INT_MOLECULE));
    }
    else {
      /* conversion to an array */
      func_value = rul__val_create_function_str (
			 rul__sem_get_type_converter (
				      domain, shape, type, a_len, 
				      SEM__C_CONVERT_RUL_TO_EXT, &ret_dom),
						 3);
      cvt_value  = rul__val_create_unnamed_ext_var (
			    rul__sem_register_ext_temp (type,
							EXT__C_IMPLICIT_ARRAY,
							mech, lhs_rt_call),
						    type, a_len, mech,
						    SEM__C_MEMORY_ALLOCATED);
      rul__val_set_nth_subvalue (func_value, 1, rul_value);
      if (a_len != EXT__C_IMPLICIT_ARRAY || a_arg_val == NULL) {
	rul__val_set_nth_subvalue (func_value, 2,
				   rul__val_create_long_animal (a_len));
      }
      else {
        rul__val_set_nth_subvalue (func_value, 2, a_arg_val);
      }
      if (ext_val != NULL)
	rul__val_set_nth_subvalue (func_value, 3, ext_val);
      else
	rul__val_set_nth_subvalue (func_value, 3,
				   rul__val_create_null (CMP__E_INT_MOLECULE));
    }
	    
    rul__val_set_shape_and_domain (func_value, shape, ret_dom);
    return rul__val_create_assignment (cvt_value, func_value);
  }
}

Value rul__add_mem_free_step (Value last_step, Value ext_arg_value)
{
  Value func_val;

  if (rul__val_get_ext_var_mem_dealoc (ext_arg_value)) {
    func_val = rul__val_create_function_str ("rul__cvt_free", 1);
    rul__val_set_nth_subvalue (func_val, 1, 
			       rul__val_copy_value (ext_arg_value));
    return rul__val_set_next_value (last_step, func_val);
  }
  else
    return last_step;
}


static Boolean invalid_constant_value (Ext_Type const_type, 
				       Ext_Type ext_type,
				       Cardinality a_len) 
{
  if (a_len != 0) return TRUE;

  if (const_type == ext_type_long)
    switch (ext_type)
      {
      case ext_type_long:
      case ext_type_short:
      case ext_type_byte:
      case ext_type_uns_long:
      case ext_type_uns_short:
      case ext_type_uns_byte:
	return FALSE;
      default:
	return TRUE;
      }

  if (const_type == ext_type_double)
    switch (ext_type)
      {
      case ext_type_float:
      case ext_type_double:
	return FALSE;
      default:
	return TRUE;
      }

  if (const_type == ext_type_asciz)
    switch (ext_type)
      {
      case ext_type_asciz:
	return FALSE;
      default:
	return TRUE;
      }
  return TRUE;
}


Value rul__sem_check_ext_rt_call (Ast_Node    ext_rt_node,
				  Boolean     return_value_required,
				  Boolean     lhs_rt_call,
				  long	      ce_index,
				  Boolean     return_molecule,
				  Decl_Domain return_domain,
				  Decl_Shape  return_shape,
				  Value       return_to_value)
     /*
      * This routine returns a value for the external routine call 
      * including all the argument conversion values, argument values,
      * and call cleanup (replace read-write values; deallocate conversion
      * allocated memory) values.
      *
      *   Parameters:
      *	ext_rt_node	      - the AST node for the external routine call
      * return_value_required - Identifies whether this routine
      *				should return an assignment (TRUE)
      *				or function value (FALSE).
      *	lhs_rt_call	      - Identifies whether this routine is walking
      *				an RHS or LHS external routine call
      *	ce_index	      - If this is a LHS value, this value identifies
      *                         the condition element from which its value
      *                         is being accessed.
      *	return_molecule       - Identifies whether the return value should
      *                         be converted into a molecule
      *	return_domain         - Type of rul molecule to convert return to
      *	return_shape          - Shape of rul molecule to convert return to
      * return_to_value       - If non NULL, the variable value to hold the
      *			        function return value
      */
{
  Mol_Symbol	  rt_name,      /* The name of the routine (atomized) */
                  ext_name,     /* The name of the external routine to call */
                  a_arg;        /* the array argument param name */
  Method          meth;	        /* Could be a method */
  Ext_Rt_Decl	  rt_decl;	/* The compiler data struct for the function */
  long	          num_args,	/* number of arguments to this routine */
                  i;		/* current parameter index */
  Value	          ret_val,	/* THE value for this routine call */
		  rul_val,	/* Temporary value to hold rul conversion */
		  func_val,	/* the function value for this routine call */
		  arg_value,	/* the assignment value for the current arg */
		  a_arg_val,	/* the value for the array arg */
		  ext_arg_val,	/* the assignment value for the external
				 * value for the current argument */
		  ext_var_val,	/* the ext variable value from the assignment
				 * value for the convert to external value
				 * for the current argument (whew!) */
		  first_step,	/* the value pointer to attach the first
				 * pre-process value list call */
		  first_prev,	/* the list pointer for the first pre-process
				 * value identified for this routine call */
		  last_step,	/* the list pointer for the last post-process
				 * value identified for this routine call */
		  next_step,	/* the list pointer for the next post-process
				 * value identified for this routine call */
		  prev_step,	/* the list pointer for the prev pre-process
				 * value identified for this routine call */
		  tmp_value;	/* the assignment value for the current arg */
  Ast_Node	  param_node;	/* the AST node for the current parameter */
  Ast_Node	  arg_node;	/* the AST node for the current parameter */
  Ast_Node	  a_arg_node;	/* temp AST node for the array parameter */
  Ast_Node_Type   node_type; 	/* the type of node of the current parameter */
  Ext_Type	  type;		/* external type of the current parameter */
  Ext_Type	  tmp_type;	/* external type of the current parameter */
  Cardinality     a_len,	/* array length (0 = scalar) of curr. param */
                  tmp_len;
  Ext_Mech	  mech;		/* mechanism of the current parameter */
  Boolean         ret_par_type; /* return type, ext or rul */
  Decl_Shape      val_shape;
  long            par_index;

  /* Get the routine name from the AST */

  param_node = rul__ast_get_child (ext_rt_node);
  rt_name = (Mol_Symbol) rul__ast_get_value (param_node);

  /* CHECK to verify we know of such a routine... */

  rt_decl = rul__decl_get_visible_ext_rt (rt_name);
  if (rt_decl == NULL) {
    meth = rul__decl_get_visible_method (rt_name);
    if (meth == NULL) {
      rul__msg_cmp_print_w_atoms (CMP_EXTUNDEF, ext_rt_node, 1, rt_name);
      return NULL;
    }
    else {
      return rul__sem_check_method_call (ext_rt_node,
					 return_value_required,
					 lhs_rt_call,
					 ce_index,
					 return_molecule,
					 return_domain,
					 return_shape,
					 return_to_value);
    }
  }

  /* get the routine name to use in the generated code */
  ext_name = rul__decl_ext_rt_alias (rt_decl);
  
  /* CHECK the number of parameters to this call */

  num_args = rul__decl_ext_rt_num_args (rt_decl);
  if (num_args != (rul__ast_get_child_count (ext_rt_node) - 1))	{
    rul__msg_cmp_print_w_atoms (CMP_EXTINVCALL, ext_rt_node, 1, rt_name);
    return NULL; /* Unable to generate code! */
  }

  /* CHECK for return value */

  if ((return_value_required) && (!rul__decl_ext_rt_ret_val (rt_decl)))	{
    rul__msg_cmp_print_w_atoms (CMP_EXTNORETURN, ext_rt_node, 1, rt_name);
    return NULL;
  }

  /* Make a function value. */

  first_step = func_val = rul__val_create_function (ext_name, num_args);
  first_prev = prev_step = NULL;

  if (return_value_required) {
    /* Record the return value to be handled by caller of this routine */
    type = rul__decl_ext_rt_ret_type (rt_decl);
    mech = rul__decl_ext_rt_mech (rt_decl);
    tmp_len = a_len = rul__decl_ext_rt_ret_a_len (rt_decl);
    rul__val_set_func_mech (func_val, ext_mech_value);
    if (a_len > 0)
      tmp_len = EXT__C_IMPLICIT_ARRAY;

    ret_val = rul__val_create_unnamed_ext_var (
	       rul__sem_register_ext_temp (type, tmp_len, mech, lhs_rt_call),
			       type, a_len, mech, SEM__C_NO_MEMORY_ALLOCATED);
				 
    first_step = rul__val_create_assignment (ret_val, func_val);
    ret_val = first_step;

    /* If the function doesn't return the correct type, complain.  The message
     * isn't entirely accurate, but it points the programmer in the right
     * direction and doing it right would be hard.
     */
    if (!rul__sem_ext_type_convertible(type, /* routine returns this type */
				       a_len, /* and this length. */
				       return_domain, /* Construct needs this*/
				       return_shape, /* type and shape. */
				       SEM__C_CONVERT_EXT_TO_RUL))
	rul__sem_report_convert_error (type, a_len,
				       return_domain, return_shape,
				       SEM__C_CONVERT_EXT_TO_RUL, ext_rt_node);
  }
  else    /* No Return Value required */
    ret_val = func_val;
    
  /* Initialize post processing step list */

  last_step = ret_val;


  /* CHECK each of the called routine's parameters */

  for (i = 1; i <= num_args; i++) {

    /* Identify the parameters of this parameter :-) */

    ext_arg_val = NULL;
    param_node = rul__ast_get_sibling (param_node);
    node_type = rul__ast_get_type (param_node);
    val_shape = shape_atom;
    if (node_type == AST__E_CONSTANT &&
	rul__ast_get_value_type (param_node) == AST__E_TYPE_MOL)
      val_shape = rul__mol_get_shape (rul__ast_get_value (param_node));
    type = rul__decl_ext_rt_param_type (rt_decl, i);
    if (type == ext_type_atom)
      ret_par_type = SEM__C_RETURN_RUL_TYPE;
    else
      ret_par_type = SEM__C_RETURN_EXT_TYPE;
    mech = rul__decl_ext_rt_param_mech (rt_decl, i);
    tmp_len = a_len = rul__decl_ext_rt_param_a_len (rt_decl, i);
    a_arg = rul__decl_ext_rt_param_a_arg (rt_decl, i);
    if (a_len > 0)
      tmp_len = EXT__C_IMPLICIT_ARRAY;

    /* Walk next argument to function call (Get the value for this arg) */
    arg_value = rul__sem_check_nested_value (param_node, lhs_rt_call,
					     ce_index, ret_par_type,
					     dom_any, shape_molecule, NULL);
    if (arg_value == NULL) {
      rul__val_free_value (ret_val);
      return NULL;
    }

    /* Setup function call to convert the rul value
     * to the proper external type (unless its a nested external call) */

    if (node_type == AST__E_EXT_RT_CALL) {
      tmp_value = ext_arg_val = arg_value;
      if (rul__val_is_assignment (arg_value))
	tmp_value = rul__val_get_assignment_to (ext_arg_val);
      if ((type  != rul__val_get_ext_var_type (tmp_value)) ||
	  (a_len != rul__val_get_ext_var_arr_len (tmp_value))) {
	/* Maybe we should do more in the case where external
	 * types don't match.   ,Later... */
	rul__msg_cmp_print_w_atoms (CMP_EXTINVNESTING, param_node, 1, rt_name);
      }
      rul__val_set_ext_var_mech (tmp_value, mech);
    }

    else if (node_type == AST__E_CONSTANT &&
	     (mech == ext_mech_value ||
	      (mech == ext_mech_ref_ro && type == ext_type_asciz)) &&
	     val_shape == shape_atom) {
      if (rul__val_is_animal_const (arg_value))	{
	if (invalid_constant_value (
			    rul__val_get_animal_type (arg_value), type, a_len))
	  rul__msg_cmp_print_w_atoms (CMP_EXTINVNESTING, 
				      param_node, 1, rt_name);
      }
      rul__val_set_nth_subvalue (func_val, i, arg_value);
    }

    else if (node_type == AST__E_CONSTANT && mech != ext_mech_value &&
	     val_shape == shape_atom) {
      ext_arg_val = rul__val_create_unnamed_ext_var (
	     rul__sem_register_ext_temp (type, tmp_len, mech, lhs_rt_call),
					     type, a_len, mech,
					     SEM__C_NO_MEMORY_ALLOCATED);
      tmp_value = rul__val_create_assignment (
				      rul__val_copy_value (ext_arg_val),
					      arg_value);
      if (prev_step)
	prev_step = rul__val_set_next_value (prev_step, tmp_value);
      else
	first_prev = prev_step = tmp_value;
    }

    else {

      a_arg_val = NULL;
      if (a_len == EXT__C_IMPLICIT_ARRAY && a_arg != NULL) {
	par_index = rul__decl_ext_rt_param_index (rt_decl, a_arg);
	a_arg_val = rul__val_get_nth_complex (func_val, par_index);
	if (rul__val_is_assignment (a_arg_val))
	  a_arg_val = rul__val_copy_value (
				   rul__val_get_assignment_to (a_arg_val));
	else
	  a_arg_val = rul__val_copy_value (a_arg_val);
	rul__val_set_ext_var_mech (a_arg_val, ext_mech_value);
      }
      ext_arg_val = rul__sem_convert_to_ext_value (arg_value, type,
						   a_len, a_arg_val,
						   mech, NULL,
						   lhs_rt_call, param_node);
      if (ext_arg_val == NULL) {
	/* Bogus parameter found, get rid of values made so far*/
	rul__val_free_value (ret_val);
	return NULL;
      }
    }

    /* Add post processing if it's a read-write parameter */

    if (ext_arg_val != NULL) {
      /* need to setup assignment of read write value after call */
      ext_var_val = NULL;

      if (mech == ext_mech_ref_ro) {
	if (rul__val_is_assignment (ext_arg_val))
	  ext_var_val = rul__val_get_assignment_to (ext_arg_val);
      }

      if (mech == ext_mech_ref_rw) {
	if (lhs_rt_call) {
	  rul__msg_cmp_print_w_atoms (CMP_EXTINVLHSARG, param_node,
				      1, rt_name);
	}

	else if (node_type != AST__E_VARIABLE) {
	  rul__msg_cmp_print_w_atoms (CMP_EXTINVRWPAR, param_node,
				      1, rt_name);
	}

	else {
	  /* Decrement the reference count on the current value of 
	   * the variable to be rebound */
#ifndef NDEBUG
	  next_step = rul__val_create_function_str ("rul__mol_decr_uses", 1);
#else
	  next_step = rul__val_create_function_str ("MDU", 1);
#endif
	  rul__val_set_nth_subvalue (next_step, 1, 
				     rul__val_copy_value (arg_value));
	  last_step = rul__val_set_next_value (last_step, next_step);

	  /* convert the external value back to an rul type */
	  ext_var_val = rul__val_copy_value (
				     rul__val_get_assignment_to (ext_arg_val));

	  /* conversion routine always accepts value BY VALUE */
	  rul__val_set_ext_var_mech (ext_var_val, ext_mech_value);
	  a_arg_val = NULL;
	  if (a_len == EXT__C_IMPLICIT_ARRAY && a_arg != NULL) {
	    par_index = rul__decl_ext_rt_param_index (rt_decl, a_arg);
	    a_arg_val = rul__val_get_nth_complex (func_val, par_index);
	    if (rul__val_is_assignment (a_arg_val))
	      a_arg_val = rul__val_copy_value (
				     rul__val_get_assignment_to (a_arg_val));
	    else
	      a_arg_val = rul__val_copy_value (a_arg_val);
	    rul__val_set_ext_var_mech (a_arg_val, ext_mech_value);
	  }
	  next_step = rul__sem_convert_to_rul_value (ext_var_val, 
					     rul__val_copy_value (arg_value), 
						     param_node, a_arg_val);
	  if (next_step == NULL) {
	    /* Can't convert read write value back to rul type */
	    rul__val_free_value (ret_val);
	    return NULL;
	  }
	  last_step = rul__val_set_next_value (last_step, next_step);
	}
      }

      /* Add arg conversion memory deallocation step (if any) */

      if (ext_var_val)
	last_step = rul__add_mem_free_step (last_step, ext_var_val);

      rul__val_set_nth_subvalue (func_val, i, ext_arg_val);
    }
    /* Done with this arg... continue loop for next arg */
  }

  /* Convert return value if necessary */
  if (return_molecule) {
    if (return_to_value == NULL)
      rul_val = rul__val_create_unnamed_mol_var (
				 rul__sem_register_temp_var (lhs_rt_call));
    else
      rul_val = return_to_value;

    rul__val_set_shape_and_domain (rul_val, return_shape, return_domain);

    a_arg_val = NULL;
    a_arg = rul__decl_ext_rt_ret_a_arg (rt_decl);
    a_len = rul__decl_ext_rt_ret_a_len (rt_decl);
    if (a_len == EXT__C_IMPLICIT_ARRAY && a_arg != NULL) {
      par_index = rul__decl_ext_rt_param_index (rt_decl, a_arg);
      a_arg_val = rul__val_get_nth_complex (func_val, par_index);
      if (rul__val_is_assignment (a_arg_val))
	a_arg_val = rul__val_copy_value (
				 rul__val_get_assignment_to (a_arg_val));
      else
	a_arg_val = rul__val_copy_value (a_arg_val);
      rul__val_set_ext_var_mech (a_arg_val, ext_mech_value);
    }

    ret_val = rul__sem_convert_to_rul_value (first_step, rul_val,
					     ext_rt_node, a_arg_val);
    if (ret_val == NULL)
      return NULL;
  }

  rul__val_set_prev_value (first_step, first_prev);

  /* The parameters are OK; return this mess... */

  return ret_val;
}

void rul__sem_check_ext_rts_in_eb (Mol_Symbol block_name, Ast_Node ast)
{
/*
 * This routine checks the external routines in the given
 * declaration block against the declaration block(s) "used"
 * by the current entry block.  If an offending external
 * routine declaration is found, a warning is displayed and
 * it is not added to the entry block.
 */

  Ext_Rt_Decl rt, other_rt;
  Decl_Block  block = rul__decl_get_block (block_name);
  Mol_Symbol  rt_name;

  for (rt = rul__decl_get_block_routines (block);
       rt != NULL;
       rt = rul__decl_ext_rt_next_routine (rt)) {

    rt_name = rul__decl_ext_rt_name (rt);
    other_rt = rul__decl_get_visible_ext_rt (rt_name);
    if (other_rt != NULL)  {
      if (!rul__decl_ext_rts_are_identical (other_rt, rt))
	rul__msg_cmp_print_w_atoms (CMP_EXTMULDEF, ast, 3, rt_name, block_name,
				    rul__decl_get_decl_name (
				     rul__decl_ext_rt_decl_blk (other_rt)));
    }
    else
      rul__decl_register_ext_rt_in_eb (rt);
  }                    
}


Value rul__sem_scalar_val_to_ext (Ast_Node node, Value value,
				  Boolean on_lhs, Ext_Type type)
{
  Ast_Node_Type node_type = rul__ast_get_type (node);

  if (value == NULL) 
    return value;

  if (node_type == AST__E_EXT_RT_CALL) {
    if ((rul__val_get_ext_var_type (rul__val_get_assignment_to (value))
	 != type) ||
	(rul__val_get_ext_var_arr_len (rul__val_get_assignment_to (value))
	 != EXT__C_NOT_ARRAY)) {
      /* Maybe we should do more in the case where external
       * types don't match.   ,Later... */
      rul__msg_cmp_print (CMP_INVCONVER, node);
    }
    return value;
  }
    
  if (node_type == AST__E_CONSTANT) {
    if (invalid_constant_value (rul__val_get_animal_type (value), 
				type, EXT__C_NOT_ARRAY))
      rul__msg_cmp_print (CMP_INVCONVER, node);
    return value;
  }

  return rul__sem_convert_to_ext_value (value, type, EXT__C_NOT_ARRAY, NULL,
					ext_mech_value, NULL, on_lhs, node);
}
    
Value rul__sem_check_method_call (Ast_Node ast,
				  Boolean return_value_required,
				  Boolean on_lhs, 
				  long ce_index,
				  Boolean return_molecule,
				  Decl_Domain return_domain,
				  Decl_Shape return_shape,
				  Value return_to_value)
{
  Mol_Symbol    meth_name, arg_name;
  Method        meth;
  long          par_cnt, arg_cnt, i;
  Value         func_val, arg_val, aar_val, loop_val;
  Ast_Node      node;
  Ast_Node_Type node_type;
  Class         rclass;

  /* (method-name <id> params...) */

  node = rul__ast_get_child (ast);
  meth_name = rul__ast_get_value (node);
  meth = rul__decl_get_visible_method (meth_name);

  if (meth == NULL) {
    rul__msg_cmp_print_w_atoms (CMP_EXTUNDEF, ast, 1, meth_name);
    return NULL;
  }

  /* child count is name + parameters */
  arg_cnt = rul__ast_get_child_count (ast) - 1;
  if (arg_cnt < 1) { /* must have at least one parameter */
    rul__msg_cmp_print_w_atoms (CMP_METPARCNT, ast, 1, meth_name);
    return NULL;
  }

  aar_val = rul__val_create_unnamed_ext_var (
                     rul__sem_register_ext_temp (ext_type_atom,
						 arg_cnt,
						 ext_mech_value,
						 SEM__C_ON_RHS),
  				   ext_type_atom, arg_cnt,
				   ext_mech_value, SEM__C_NO_MEMORY_ALLOCATED);

  func_val = rul__val_create_function_str ("rul__call_method", 5);
  rul__val_set_nth_subvalue (func_val, 1,
			     rul__val_create_rul_constant (meth_name));
  rul__val_set_nth_subvalue (func_val, 2,
			     rul__val_create_long_animal (arg_cnt));
  aar_val = rul__val_copy_value (aar_val);
  rul__val_set_ext_var_index (aar_val, EXT__C_IMPLICIT_ARRAY);
  rul__val_set_nth_subvalue (func_val, 3, aar_val);
  rul__val_set_nth_subvalue (func_val, 4,
			     rul__val_create_blk_or_loc (CMP__E_INT_MOLECULE,
							 "NULL"));
  rul__val_set_nth_subvalue (func_val, 5,
		     rul__val_create_blk_or_loc (CMP__E_INT_ENTRY_DATA,
						 GEN__C_EB_DATA_STR));

  loop_val = func_val;

  for (node = rul__ast_get_sibling (node), i = 0;
       node != NULL; node = rul__ast_get_sibling (node), i++) {

    /* set index for assignment */
    aar_val = rul__val_copy_value (aar_val);
    rul__val_set_ext_var_index (aar_val, i);

    node_type = rul__ast_get_type (node);
    arg_name = rul__ast_get_value (node);

    /* special code for when instance-id of class*/
    /* first parameter - must be a variable of instance_id */
    if (i == 0) {
      if (node_type != AST__E_VARIABLE) {
	if (rul__mol_is_valid (arg_name))
	  rul__msg_cmp_print_w_atoms (CMP_INVMETNPAR, node, 1, arg_name);
	else
	  rul__msg_cmp_print (CMP_INVMETPAR, node);
      }
      arg_val = rul__sem_check_rhs_var (node, SEM__C_VALUE_ACCESS);

      if (arg_val != NULL) {
	/* if it's got a known class, specialize the method we're using */
	rclass = rul__val_get_class (arg_val);
	if (rclass) {
	  meth = rul__decl_get_generic_method (rclass, meth_name);
	  if (meth == NULL) {
	    rul__msg_cmp_print_w_atoms (CMP_NOCLSMET, node, 2, meth_name,
					rul__decl_get_class_name (rclass));
	    rul__val_free_value (arg_val);
	    arg_val = NULL;
	  }
	  else if (arg_cnt != rul__decl_get_method_num_params (meth)) {
	    rul__msg_cmp_print_w_atoms (CMP_METPARCNT, node, 1, meth_name);
	    rul__val_free_value (arg_val);
	    arg_val = NULL;
	  }
	}
      }
    }
    else { /* rest of the params... */
      arg_val = rul__sem_check_nested_value (node, on_lhs, ce_index,
					     SEM__C_RETURN_RUL_TYPE,
					     dom_any, shape_molecule, NULL);
    }
    
    if (arg_val == NULL) {
      rul__val_free_value (func_val);
      return NULL;
    }
    
    arg_val = rul__val_create_assignment (aar_val, arg_val);
    rul__val_set_prev_value (loop_val, arg_val);
    loop_val = arg_val;
  }
  
  if (return_value_required) {
    return (rul__sem_return_value (return_to_value,
				   func_val, return_shape, return_domain,
				   on_lhs, NULL));
  }

  return func_val;
}



Value rul__sem_check_call_inherited (Ast_Node ast, 
				     Boolean return_required,
				     Value return_to_val)
{
  Ast_Node       node, meth_node;
  Ast_Node_Type  node_type;
  Value          arg_val, ret_val, aar_val, loop_val, func_val;
  Value		 ret_val_assignment = NULL;
  Value		 ret_val_func = NULL;
  Decl_Shape     shape;
  Decl_Domain    domain;
  Boolean        in_method = FALSE;
  Mol_Symbol     meth_name, class_name, arg_name;
  Method         meth;
  Class          rclass, par_class;
  Molecule       ret_mol;
  long           arg_cnt, i;

  /* call-inherited <var> ... */

  meth_node = rul__ast_get_parent (ast);
  while (meth_node) {
    if (rul__ast_get_type (meth_node) == AST__E_METHOD_RHS) {
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
      in_method = TRUE;
      break;
    }
    meth_node = rul__ast_get_parent (meth_node);
  }

  if (!in_method) {
    rul__msg_cmp_print (CMP_INVCALINH, ast);
    return NULL;
  }

  if (meth == NULL) {
    rul__msg_cmp_print_w_atoms (CMP_EXTUNDEF, ast, 1, meth_name);
    return NULL;
  }

  arg_cnt = rul__ast_get_child_count (ast);
  if (arg_cnt != rul__decl_get_method_num_params (meth)) {
    rul__msg_cmp_print_w_atoms (CMP_METPARCNT, ast, 1, meth_name);
    return NULL;
  }

  par_class = rul__decl_get_class_parent (rclass);
  if (par_class == NULL ||
      rul__decl_get_class_name (par_class) == rul__mol_symbol_root ()) {
    rul__msg_cmp_print_w_atoms (CMP_NOINHCLS, ast, 1, class_name);
    return NULL;
  }

  meth = rul__decl_get_generic_method (par_class, meth_name);
  if (meth == NULL) {
    rul__msg_cmp_print_w_atoms (CMP_NOINHMET, ast, 1, class_name);
    return NULL;
  }

  aar_val = rul__val_create_unnamed_ext_var (
                     rul__sem_register_ext_temp (ext_type_atom,
						 arg_cnt,
						 ext_mech_value,
						 SEM__C_ON_RHS),
  				   ext_type_atom, arg_cnt,
				   ext_mech_value, SEM__C_NO_MEMORY_ALLOCATED);

  func_val = rul__val_create_function_str ("rul__call_method", 5);
  rul__val_set_nth_subvalue (func_val, 1,
			     rul__val_create_rul_constant (meth_name));
  rul__val_set_nth_subvalue (func_val, 2,
			     rul__val_create_long_animal (arg_cnt));
  aar_val = rul__val_copy_value (aar_val);
  rul__val_set_ext_var_index (aar_val, EXT__C_IMPLICIT_ARRAY);
  rul__val_set_nth_subvalue (func_val, 3, aar_val);
  rul__val_set_nth_subvalue (func_val, 4,
			     rul__val_create_class_constant (par_class));
  rul__val_set_nth_subvalue (func_val, 5,
		     rul__val_create_blk_or_loc (CMP__E_INT_ENTRY_DATA,
						 GEN__C_EB_DATA_STR));

  loop_val = func_val;

  for (node = rul__ast_get_child (ast), i = 0;
       node != NULL; node = rul__ast_get_sibling (node), i++) {

    /* set index for assignment */
    aar_val = rul__val_copy_value (aar_val);
    rul__val_set_ext_var_index (aar_val, i);

    node_type = rul__ast_get_type (node);
    arg_name = rul__ast_get_value (node);

    /* special code for when instance-id of class*/
    /* first parameter - must be a variable of instance_id */
    if (i == 0) {
      if (node_type != AST__E_VARIABLE) {
	if (rul__mol_is_valid (arg_name))
	  rul__msg_cmp_print_w_atoms (CMP_INVMETNPAR, node, 1, arg_name);
	else
	  rul__msg_cmp_print (CMP_INVMETPAR, node);
      }
      arg_val = rul__sem_check_rhs_var (node, SEM__C_VALUE_ACCESS);

      if (arg_val != NULL) {
	/* if it's got a known class, specialize the method we're using */
	rclass = rul__val_get_class (arg_val);
	if (rclass) {
	  meth = rul__decl_get_generic_method (rclass, meth_name);
	  if (meth == NULL) {
	    rul__msg_cmp_print_w_atoms (CMP_NOCLSMET, node, 2, meth_name,
					rul__decl_get_class_name (rclass));
	    rul__val_free_value (arg_val);
	    arg_val = NULL;
	  }
	  else if (arg_cnt != rul__decl_get_method_num_params (meth)) {
	    rul__msg_cmp_print_w_atoms (CMP_METPARCNT, node, 1, meth_name);
	    rul__val_free_value (arg_val);
	    arg_val = NULL;
	  }
	}
      }
    }

    else { /* rest of the params... */
      arg_val = rul__sem_check_nested_value (node, SEM__C_ON_RHS,
					     SEM__C_UNDEFINED_CE_INDEX,
					     SEM__C_RETURN_RUL_TYPE,
					     dom_any, shape_molecule, NULL);
    }
    
    if (arg_val == NULL) {
      rul__val_free_value (func_val);
      return NULL;
    }
    
    arg_val = rul__val_create_assignment (aar_val, arg_val);
    rul__val_set_prev_value (loop_val, arg_val);
    loop_val = arg_val;
  }
  
  if (return_required) {
    return (rul__sem_return_value (return_to_val, func_val,
			   rul__decl_get_method_par_shape (meth, arg_cnt),
			   rul__decl_get_method_par_domain (meth, arg_cnt),
			   SEM__C_ON_RHS,
			   rul__decl_get_method_par_class (meth, arg_cnt)));
  }

  return func_val;
}

