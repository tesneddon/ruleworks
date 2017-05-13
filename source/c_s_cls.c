/* cmp_sem_class.c - object-class semantic routine */
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
**	rul__sem_object_class is passed the AST of an object class
**	declaration.  The tree is traversed checking for sementic
**	errors and calling decl RTL functions to create object-class
**	data structures.
**
**
**  MODIFIED BY:
**	DEC	Digital Equipment Corporation
 *	CPQ	Compaq Computer Corporation
**
**  MODIFICATION HISTORY:
**
**	15-Jan-1993	DEC	Initial version
**	22-Oct-1993	DEC	Added attribute typing
**	16-Feb-1998	DEC	class type changed to rclass
**	01-Dec-1999	CPQ	Release with GPL
*/

#include <common.h>
#include <cmp_comm.h>
#include <sem.h>
#include <ast.h>
#include <decl.h>
#include <mol.h>
#include <msg.h>
#include <cmp_msg.h>
#include <gen.h>
#include <val.h>
#include <cons.h>
#include <hash.h>

static Decl_Domain  sem_name_to_domain (Molecule domain_name);
static Boolean 	    sem_check_attr_value_domain (
			Class class_id, Mol_Symbol attribute_name,
			Molecule value, Boolean is_for_default,
			Ast_Node current_node);

static void         sem_method_add_param (long index, Ast_Node ast);
static Boolean      sem_method_check_param (long index, Method meth,
					    Ast_Node ast);
static Boolean      sem_method_check_ancestry (Mol_Symbol meth_name,
					       Class rclass, Boolean generic);




/****************************
**                         **
**  RUL__SEM_OBJECT_CLASS  **
**                         **
****************************/

Boolean rul__sem_object_class (Ast_Node ast)
{
  Class	        class_id;
  Mol_Symbol	class_name;
  Class	        parent_class_id; /* Used to ensure inherited shape matches */
  Mol_Symbol	parent_name;
  Mol_Symbol	attribute_name;
  Ast_Node	current_node;
  Ast_Node	attribute_child, domain_child;
  Decl_Block	current_decl_block = rul__decl_get_curr_decl_block();
  Boolean	error = FALSE;	/* TRUE if an error was found */
  Boolean	default_specified; /* True when this DEFAULT found for attr */
  Boolean	fill_specified;	/* True when FILL found or attr */
  Decl_Domain   attr_domain;
  Mol_Symbol    attr_class_name;
  Molecule	default_val, fill_val;
  Class	        attr_class;

  assert (rul__ast_get_type (ast) == AST__E_OBJ_CLASS);

  current_node = rul__ast_get_child (ast); /* Class name */
  class_name = rul__ast_get_value (current_node);

  /* If there's already a class visible with this name, we're in trouble. */
  class_id = rul__decl_get_visible_class (class_name);
  if (class_id != NULL) {
    rul__msg_cmp_print_w_atoms (CMP_DUPCLASS, current_node, 2, class_name,
				rul__decl_get_class_block_name (class_id));
    /* Since we can't create the class, we bail out here. */
    return FALSE;		/* Failure */
  }

  rul__decl_create_class (rul__decl_get_decl_name (current_decl_block),
			  class_name);
  class_id = rul__decl_get_class (current_decl_block, class_name);

  /* Switch from first child (name) to next (inherit, or attributes). */
  current_node = rul__ast_get_sibling (current_node);

  /*
   * Inherits-from
   */
  parent_class_id = NULL;

  if (rul__ast_get_type (current_node) == AST__E_OBJ_CLASS_INH) {
    parent_name = rul__ast_get_value (rul__ast_get_child (current_node));

    if (parent_name == class_name) {
      /*  If the parent is self, complain  */
      rul__msg_cmp_print (CMP_SELFPARENT, current_node);
    }

    else if (rul__decl_get_class (current_decl_block, parent_name)==NULL) {
      /* If parent isn't declared, complain. */
      rul__msg_cmp_print_w_atoms (CMP_NOPARENT, current_node, 1, parent_name);
    }
    else {
      rul__decl_set_cur_class_parent (parent_name);
      parent_class_id = rul__decl_get_class (current_decl_block, parent_name);
    }
    current_node = rul__ast_get_sibling (current_node); /* Attributes */
  }

  if (parent_class_id == NULL) {
    /* No valid inherits clause, so use $ROOT */
    rul__decl_set_cur_class_parent (rul__mol_symbol_root ());
    parent_class_id = rul__decl_get_class (current_decl_block,
					   rul__mol_symbol_root ());
  }

  /*
   * Attribute declarations
   */
  assert (rul__ast_get_type (current_node) == AST__E_OBJ_ATTRS);

  /* Iterate over the attributes. */
  for (current_node = rul__ast_get_child (current_node);
       current_node != NULL;
       current_node = rul__ast_get_sibling (current_node)) {

    /* attribute_child will iterate through the children of this
       attribute (name and modifiers). */
    attribute_child = rul__ast_get_child (current_node); /* Name */
    
    attribute_name = rul__ast_get_value (attribute_child);

    /* If duplicate attribute, complain, else process. */
    if (rul__decl_is_locl_attr_in_class (class_id, attribute_name)) {
      rul__msg_cmp_print_w_atoms (CMP_DUPATTR, current_node,
				  1, attribute_name);
    }

    else {			/* Not duplicate local attribute */

      rul__decl_add_attr_cur_class(attribute_name);

      /* Iterate over all the attribute modifiers. */
      for (attribute_child = rul__ast_get_sibling (attribute_child),
	   default_specified = fill_specified = FALSE;
	   attribute_child != NULL;
	   attribute_child = rul__ast_get_sibling (attribute_child)) {

	switch (rul__ast_get_type (attribute_child)) {

	case AST__E_ATTR_COMPOUND:
	  /* The syntax ensures that COMPOUND can't appear twice,
	     so we don't need to check. */
	  /* If this attribute is inherited, and it's not compound
	     in the parent, complain. */
	  if (rul__decl_is_attr_in_class (parent_class_id, attribute_name) &&
	      rul__decl_get_attr_shape (parent_class_id, attribute_name) !=
	      shape_compound)
	    rul__msg_cmp_print_w_atoms (CMP_INHCOMP, current_node,
					1, attribute_name);
	  else
	    rul__decl_set_cur_attr_compound();
	  break;
	  
	case AST__E_ATTR_DOMAIN:
	  /* The syntax ensures that there can't be 2 domains.  */
	  attr_domain = sem_name_to_domain ((Molecule)
				    rul__ast_get_value (attribute_child));
	  if (!rul__decl_is_attr_in_class (parent_class_id, attribute_name)) {
	    /*
	     **  If this is not an inherited attribute, set
	     **  the domain of the current attribute, and if
	     **  appropriate, the class of the attribute
	     */
	    
	    rul__decl_set_cur_attr_domain (attr_domain);
	    if (attr_domain == dom_instance_id) {
	      domain_child = rul__ast_get_child (attribute_child);
	      if (domain_child != NULL) {
		attr_class_name = (Molecule) rul__ast_get_value (domain_child);
		attr_class = rul__decl_get_visible_class (attr_class_name);
		if (attr_class != NULL) {
		  rul__decl_set_cur_attr_class (attr_class);
		}
		else {
		  rul__msg_cmp_print_w_atoms (CMP_UNKDOMCLASS,
					      domain_child, 2, attribute_name, 
					      attr_class_name);
		}
	      }
	    }
	  }
	  else {
	    /*
	     **  If this attribute is inherited, and it's domain 
	     **  (or class) isn't the same, complain
	     */
	    if (attr_domain != rul__decl_get_attr_domain (parent_class_id,
							  attribute_name)) {
	      rul__msg_cmp_print_w_atoms (CMP_INHDOM,
					  attribute_child, 1, attribute_name);
	    }
	    else if (attr_domain == dom_instance_id) {
	      domain_child = rul__ast_get_child (attribute_child);
	      if (domain_child != NULL) {
		attr_class_name = (Molecule) rul__ast_get_value (domain_child);
		attr_class = rul__decl_get_visible_class (attr_class_name);
		if (attr_class != rul__decl_get_attr_class (parent_class_id,
							    attribute_name)) {
		  rul__msg_cmp_print_w_atoms (CMP_INHDOM,
					      attribute_child, 1, 
					      attribute_name);
		}
	      }
	    }
	  }
	  break;
	  
	case AST__E_ATTR_DEFAULT:
	  if (default_specified)
	    rul__msg_cmp_print_w_atoms(CMP_MULTDEF, current_node,
				       1, attribute_name);
	  else if (rul__decl_get_attr_shape(class_id, attribute_name)
		   != shape_atom)
	    rul__msg_cmp_print_w_atoms(CMP_BADSCADEF, current_node,
				       1, attribute_name);
	  else {
	    default_val = rul__ast_get_value (
				      rul__ast_get_child (attribute_child));
	    if (sem_check_attr_value_domain (class_id, attribute_name,
					     default_val, TRUE,current_node)) {
	      rul__decl_set_cur_attr_default (default_val);
	      default_specified = TRUE;
	    }
	  }
	  break;
	  
	case AST__E_ATTR_COMPOUND_DEFAULT:
	  if (default_specified)
	    rul__msg_cmp_print_w_atoms(CMP_MULTDEF, current_node,
				       1, attribute_name);
	  else if (rul__decl_get_attr_shape(class_id, attribute_name)
		   != shape_compound)
	    rul__msg_cmp_print_w_atoms(CMP_BADCOMPDEF, current_node,
				       1, attribute_name);
	  else {
	    default_val = rul__ast_get_value (
				      rul__ast_get_child (attribute_child));
	    if (sem_check_attr_value_domain (class_id, attribute_name,
					     default_val, TRUE,
					     current_node)) {
	      rul__decl_set_cur_attr_default (default_val);
	      default_specified = TRUE;
	    }
	  }
	  break;
	  
	case AST__E_ATTR_COMPOUND_FILL:
	  if (fill_specified)
	    rul__msg_cmp_print_w_atoms(CMP_MULTFILL, current_node,
				       1, attribute_name);
	  else if (rul__decl_get_attr_shape(class_id, attribute_name)
		   != shape_compound)
	    rul__msg_cmp_print_w_atoms(CMP_SCALFILL, current_node,
				       1, attribute_name);
	  else {
	    
	    fill_val = rul__ast_get_value (
				   rul__ast_get_child (attribute_child));
	    if (sem_check_attr_value_domain (class_id, attribute_name,
					     fill_val, FALSE,
					     current_node)) {
	      rul__decl_set_cur_attr_fill (fill_val);
	      fill_specified = TRUE;
	    }
	  }
	  break;
	  
	default:
	  assert(FALSE); /* Can never happen */
	}		/* switch */
      }			/* Iterate over attribute modifiers */
    }			/* Not duplicate attribute */
  }			/* Iterate through attributes */
  return !error;
}





/*************************
**                      **
**  SEM_NAME_TO_DOMAIN  **
**                      **
*************************/

static Decl_Domain sem_name_to_domain (Molecule domain_name)
{
  static names_inited = FALSE;
  static Molecule mol_symbol, mol_instance, mol_opaque,
                  mol_number, mol_integer, mol_float, mol_any;

  if (names_inited == FALSE) {
    names_inited = TRUE;
    mol_symbol = rul__mol_make_symbol ("SYMBOL");
    rul__mol_mark_perm (mol_symbol);
    mol_instance = rul__mol_make_symbol ("INSTANCE");
    rul__mol_mark_perm (mol_instance);
    mol_opaque = rul__mol_make_symbol ("OPAQUE");
    rul__mol_mark_perm (mol_opaque);
    mol_number = rul__mol_make_symbol ("NUMBER");
    rul__mol_mark_perm (mol_number);
    mol_integer = rul__mol_make_symbol ("INTEGER");
    rul__mol_mark_perm (mol_integer);
    mol_float = rul__mol_make_symbol ("FLOAT");
    rul__mol_mark_perm (mol_float);
    mol_any = rul__mol_make_symbol ("ANY");
    rul__mol_mark_perm (mol_any);
  }

  if (domain_name == mol_symbol)	return dom_symbol;
  if (domain_name == mol_instance) 	return dom_instance_id;
  if (domain_name == mol_opaque)	return dom_opaque;
  if (domain_name == mol_number)	return dom_number;
  if (domain_name == mol_integer)	return dom_int_atom;
  if (domain_name == mol_float)		return dom_dbl_atom;
  if (domain_name == mol_any)		return dom_any;
  
  assert (FALSE); /* invalid domain name */
  return dom_invalid;
}




/**********************************
**                               **
**  SEM_CHECK_ATTR_VALUE_DOMAIN  **
**                               **
**********************************/

static Boolean sem_check_attr_value_domain (Class class_id,
					    Mol_Symbol attribute_name,
					    Molecule value,
					    Boolean is_for_default,
					    Ast_Node current_node)
{
  Decl_Domain attr_domain, val_domain;
  Mol_Symbol  attr_dom_name, val_dom_name;

  val_domain = rul__mol_get_domain (value);
  attr_domain = rul__decl_get_attr_domain (class_id, attribute_name);

  if (rul__mol_is_subdomain (attr_domain, val_domain))
    return TRUE;

  if (value == rul__mol_compound_empty())
    return TRUE;

  /*  If we got here, then we are in trouble  */
  val_dom_name = rul__mol_make_symbol (rul__sem_domain_to_string (val_domain));
  attr_dom_name = rul__mol_make_symbol (
				rul__sem_domain_to_string (attr_domain));
  if (is_for_default) {
    if (rul__mol_is_atom (value)) {
      rul__msg_cmp_print_w_atoms (CMP_INVATTRDEF, current_node, 3,
				  attr_dom_name, value, val_dom_name);
    }
    else {
      rul__msg_cmp_print_w_atoms (CMP_INVCOMPDEF, current_node, 1,
				  attr_dom_name);
    }
  }
  else {
    rul__msg_cmp_print_w_atoms (CMP_INVATTRFIL, current_node, 3,
				attr_dom_name, value, val_dom_name);
  }
  rul__mol_decr_uses (val_dom_name);
  rul__mol_decr_uses (attr_dom_name);
  return FALSE;
}

Boolean rul__sem_check_generic_method (Ast_Node ast)
{
  Decl_Block block = rul__decl_get_curr_decl_block ();
  Mol_Symbol meth_name, class_name;
  Ast_Node   node, acc_node, class_node, ret_node, param;
  long       i, par_cnt;
  Boolean    ret_flag = FALSE;
  Class      rclass = NULL, gen_class = NULL;
  Method     meth;

  node = rul__ast_get_child (ast); /* meth_name */
  meth_name = rul__ast_get_value (node);
  
  if (rul__decl_is_system_method (meth_name) != 0) {
    rul__msg_cmp_print_w_atoms (CMP_GENMETIGN, node, 2, meth_name,
				(Molecule) rul__decl_get_decl_name (block));
    return TRUE;
  }

  if (rul__decl_is_an_ext_rt (meth_name)) {
    rul__msg_cmp_print_w_atoms (CMP_EXTDUPDEF, node, 2, meth_name,
				(Molecule) rul__decl_get_decl_name (block));
    return FALSE;
  }
  
  acc_node = rul__ast_get_sibling (node); /* accepts_decl */
  assert (rul__ast_get_type (acc_node) == AST__E_ACCEPTS_DECL);

  par_cnt = rul__ast_get_child_count (acc_node);

  ret_node = rul__ast_get_sibling (acc_node); /* returns_decl */
  if (ret_node && rul__ast_get_type (ret_node) == AST__E_RETURNS_DECL) {
    ret_flag = TRUE;
    ret_node = rul__ast_get_child (ret_node);
  }

  acc_node = rul__ast_get_child (acc_node);

  assert (rul__ast_get_type (acc_node) == AST__E_METHOD_WHEN);

  /* checks for class in when_param */
  class_node = rul__ast_get_sibling (rul__ast_get_child (acc_node));

  if (class_node) {
    class_name = rul__ast_get_value (class_node);
    rclass = rul__decl_get_class (block, class_name);
    if (rclass == NULL ||  /* is $root ok??? */
	class_name == rul__mol_symbol_root ()) {
      /* error - invalid class */
      rul__msg_cmp_print_w_atoms (CMP_INVMETCLS, class_node, 1, class_name);
      return FALSE;
    }
  }
  if (rclass == NULL)
    class_name = rul__mol_symbol_root ();

  /* verify ancestry */
  if (! sem_method_check_ancestry (meth_name, rclass, TRUE)) {
    rul__msg_cmp_print_w_atoms (CMP_CNFMETDEF, class_node, 1, class_name);
    return FALSE;
  }

  /* create the generic method for this when-class */
  rul__decl_create_method (block, meth_name, FALSE, par_cnt);

  param = acc_node;
  for (i = 0; i < par_cnt; i++) {
    sem_method_add_param (i, param);
    param = rul__ast_get_sibling (param);
  }
  if (ret_flag)
    sem_method_add_param (par_cnt, ret_node);
  else
    rul__decl_set_method_param (par_cnt, shape_atom, dom_any, NULL);

  return TRUE;
}

Boolean rul__sem_check_method (Ast_Node ast)
{
  Decl_Block  block = rul__decl_get_curr_decl_block ();
  Mol_Symbol  meth_name, class_name = NULL;
  Ast_Node    node, acc_node, class_node, ret_node, param, act_node;
  long        i, j, par_cnt;
  Boolean     ret_flag = FALSE, gen_exists = FALSE;
  Class       rclass = NULL, gen_class;
  Method      meth, gen_meth;
  Value       arg_val, var_val, par_val;
  Mol_Symbol *par_array;

  node = rul__ast_get_child (ast); /* meth_name */
  meth_name = rul__ast_get_value (node);
  
  if (rul__decl_is_an_ext_rt (meth_name)) {
    rul__msg_cmp_print_w_atoms (CMP_EXTDUPDEF, node, 2, meth_name,
				(Molecule) rul__decl_get_decl_name (block));
    return FALSE;
  }
  
  if (rul__decl_is_system_method (meth_name) != 0)
    rul__decl_define_sys_methods (block);

  acc_node = rul__ast_get_sibling (node); /* accepts_decl */
  assert (rul__ast_get_type (acc_node) == AST__E_ACCEPTS_DECL);

  par_cnt = rul__ast_get_child_count (acc_node);

  ret_node = rul__ast_get_sibling (acc_node); /* returns_decl */
  if (ret_node && rul__ast_get_type (ret_node) == AST__E_RETURNS_DECL) {
    act_node = rul__ast_get_sibling (ret_node);
    ret_node = rul__ast_get_child (ret_node);
    ret_flag = TRUE;
  }
  else
    act_node = ret_node;

  acc_node = rul__ast_get_child (acc_node);

  assert (rul__ast_get_type (acc_node) == AST__E_METHOD_WHEN);

  /* checks for class in when_param */
  class_node = rul__ast_get_sibling (rul__ast_get_child (acc_node));

  if (class_node) {
    class_name = rul__ast_get_value (class_node);
    rclass = rul__decl_get_class (block, class_name);
    if (rclass == NULL || 
	rul__decl_get_class_name (rclass) == rul__mol_symbol_root ()) {
      /* error - invalid class */
      rul__msg_cmp_print_w_atoms (CMP_INVMETCLS, class_node, 1, class_name);
      return FALSE;
    }
  }
  else {
  /*
   * method with no class - syntax error in parser...
   */
  }

  /* see if theres a generic method defined for this class */

  gen_meth = rul__decl_get_generic_method (rclass, meth_name);

  if (! gen_meth) {     /* no generic method defined */

    /* verify ancestry of generic method definition */
    if (! sem_method_check_ancestry (meth_name, rclass, TRUE)) {
      rul__msg_cmp_print_w_atoms (CMP_CNFMETDEF, class_node, 1, class_name);
      return FALSE;
    }

    /* create the generic method definiion */
    gen_meth = rul__decl_create_method (block, meth_name, FALSE, par_cnt);
    
    /* define all the params */
    param = acc_node;
    for (i = 0; i < par_cnt; i++) {
      sem_method_add_param (i, param);
      param = rul__ast_get_sibling (param);
    }
    if (ret_flag)
      sem_method_add_param (par_cnt, ret_node);
    else
      rul__decl_set_method_param (par_cnt, shape_atom, dom_any, NULL);
  }

  else {

    gen_exists = TRUE;

    /* verify ancestry of method definition */
    if (! sem_method_check_ancestry (meth_name, rclass, FALSE)) {
      rul__msg_cmp_print_w_atoms (CMP_CNFMETDEF, class_node, 1, class_name);
      return FALSE;
    }
  }

  if (gen_exists) {

    /* verify that this methods params match the generic methods */
    if (rul__decl_get_method_num_params (gen_meth) != par_cnt) {
      /* error method - parameter count mismatch */
      rul__msg_cmp_print (CMP_INVMPRCNT, acc_node);
      return FALSE;
    }

    param = acc_node;
    for (i = 0; i < par_cnt; i++) {
      if (!sem_method_check_param (i, gen_meth, param))
	return FALSE;
      param = rul__ast_get_sibling (param);
    }

    if (ret_flag) {
      if (!sem_method_check_param (par_cnt, gen_meth, ret_node))
	return FALSE;
    }
  }

  /* one last check before creating method */
  /* check for duplicate parameter names */

  par_array = rul__mem_malloc (par_cnt * sizeof (void *));
  param = acc_node;
  for (i = 0; i < par_cnt; i++) {
    par_array[i] = rul__ast_get_value (rul__ast_get_child (param));
    for (j = 0; j < i; j++) {
      if (par_array[i] == par_array[j]) {
	rul__msg_cmp_print_w_atoms (CMP_METDUPPAR, param, 1, par_array[i]);
	rul__mem_free (par_array);
	return FALSE;
      }
    }
    param = rul__ast_get_sibling (param);
  }

  /* create the method */
  meth = rul__decl_create_method (block, meth_name, TRUE, par_cnt);
  
  /* on methods, only the when param is stored */
  param = acc_node;
  sem_method_add_param (0, param);

  /* initialize rhs vars here before the check_rhs */

  rul__sem_use_entry_vars (FALSE);
  rul__sem_initialize_rhs_vars ();

  /*  return param is not initialized... */

  param = acc_node;

  for (i = 0; i < par_cnt; i++) {
    
    /* create the initializer */
    par_val = rul__val_create_blk_or_loc_vec (CMP__E_INT_MOLECULE,
					      GEN__C_METHOD_PARAM_STR,
					      i);
    rul__val_set_shape_and_domain (par_val,
				   rul__decl_get_method_par_shape (gen_meth,
								   i),
				   rul__decl_get_method_par_domain (gen_meth,
								    i));
    /*
     * CALL-INHERITED could invoke this method,
     * but we still restrict to the method defined class...
     */
    if (i == 0)
      rul__val_set_class (par_val, rclass);
    else
      rul__val_set_class (par_val,
			  rul__decl_get_method_par_class (gen_meth, i));
    
    /* register the parameter name and set initializer */
    rul__sem_check_rhs_var (rul__ast_get_child (param),
			    SEM__C_VALUE_ASSIGNMENT);
    rul__sem_set_var_initializer (par_array[i], par_val);
    
    param = rul__ast_get_sibling (param);
  }

  rul__mem_free (par_array);

  /*
   * Process method body...
   */
  assert (rul__ast_get_type (act_node) == AST__E_METHOD_RHS);

  /* check the method body */
  if (rul__sem_check_rhs (act_node)) {

    /* finish of the method definition */
    rul__decl_finish_method ();

    return TRUE;
  }

  rul__decl_method_remove (meth);
  return FALSE;
}

static void sem_method_add_param (long index, Ast_Node ast)
{
  Decl_Block    block = rul__decl_get_curr_decl_block ();
  Ast_Node      node;
  Ast_Node_Type node_type;
  Mol_Symbol    par_name, class_name = NULL;
  Decl_Domain   domain = dom_any;
  Decl_Shape    shape = shape_atom;
  Class         rclass = NULL;

  node_type = rul__ast_get_type (ast);

  if (node_type == AST__E_METHOD_WHEN)
    domain = dom_instance_id;
    
  node = rul__ast_get_child (ast);
  par_name = rul__ast_get_value (node);
  node = rul__ast_get_sibling (node);

  /*
   * a WHEN node contains the name and [CLASS]
   * other params contain the name [shape] [domain] [class_restriction]
   */

  if (node_type == AST__E_METHOD_WHEN) {
    if (node) {
      class_name = rul__ast_get_value (node);
    }
  }

  else {
    if (node && rul__ast_get_type (node) == AST__E_ATTR_COMPOUND) {
      shape = shape_compound;
      node = rul__ast_get_sibling (node);
    }
    if (node) {
      domain = sem_name_to_domain (rul__ast_get_value (node));
      node = rul__ast_get_child (node);
    }
    if (node) {
      class_name = rul__ast_get_value (node);
    }
  }

  if (class_name == rul__mol_symbol_root ())
    class_name = NULL;
  if (class_name)
    rclass = rul__decl_get_visible_class (class_name);

  /* register so it's emmited in the class array */
  if (rclass)
    rul__class_create_class (rclass);
  else if (class_name) {
    rul__msg_cmp_print_w_atoms (CMP_METPARCLS, node, 2, par_name, class_name);
  }

  rul__decl_set_method_param (index, shape, domain, rclass);
}

static Boolean sem_method_check_param (long index, Method meth, Ast_Node ast)
{
  Decl_Block    block = rul__decl_get_curr_decl_block ();
  Ast_Node      node, last_node;
  Ast_Node_Type node_type;
  Mol_Symbol    class_name, par_name, par_class_name;
  Decl_Domain   domain = rul__decl_get_method_par_domain (meth, index);
  Decl_Shape    shape = rul__decl_get_method_par_shape (meth, index);
  Class         rclass = rul__decl_get_method_par_class (meth, index);
  Value         var;
  Decl_Domain   par_domain = dom_any;
  Decl_Shape    par_shape = shape_atom;
  Class         par_class = NULL;
  
  node_type = rul__ast_get_type (ast);
  node = rul__ast_get_child (ast);

  if (node_type == AST__E_METHOD_WHEN) {
    par_shape  = shape_atom;
    par_domain = dom_instance_id;
  }

  last_node = node;
  par_name = rul__ast_get_value (node);
  node = rul__ast_get_sibling (node);
  
  if (node && rul__ast_get_type (node) == AST__E_ATTR_COMPOUND) {
    par_shape = shape_compound;
    last_node = node;
    node = rul__ast_get_sibling (node);
  }

  if (node && rul__ast_get_type (node) == AST__E_ATTR_DOMAIN) {
    par_domain = sem_name_to_domain (rul__ast_get_value (node));
    last_node = node;
    node = rul__ast_get_child (node);
  }

  if (node && par_domain == dom_instance_id) {
    par_class_name = rul__ast_get_value (node);
    if (par_class_name != rul__mol_symbol_root ())
      par_class = rul__decl_get_class (block, par_class_name);
  }

  if (shape != par_shape) {
    /* error method invalid shape */
    rul__msg_cmp_print_w_atoms (CMP_METSHPPAR, last_node, 1, par_name);
    return FALSE;
  }

  if (domain != par_domain) {
    /* error method invalid domain */
    rul__msg_cmp_print_w_atoms (CMP_METDOMPAR, last_node, 1, par_name);
    return FALSE;
  }

  if (node_type == AST__E_METHOD_WHEN) {
    if (!rclass || rul__decl_is_a_subclass (par_class, rclass))
      return TRUE;
  }
  else {
    if ((rclass && !par_class) ||
	(!rclass && par_class) ||
	(rclass != par_class)) {
      /* error method invalid class */
      rul__msg_cmp_print_w_atoms (CMP_METCLSPAR, last_node, 1, par_name);
      return FALSE;
    }
  }

  return TRUE;
}

static Boolean sem_method_check_ancestry (Mol_Symbol meth_name,
					  Class rclass,
					  Boolean generic)
{
  Hash_Table meth_ht;
  Method     meth;
  Class      gen_class;
  Decl_Block block = rul__decl_get_curr_decl_block ();

  meth = rul__hash_get_entry (rul__decl_get_block_method_ht (block),
			      meth_name);

  while (meth) {
    if (rul__decl_is_generic_method (meth)) {
      gen_class = rul__decl_get_method_par_class (meth, 0);

      if (generic) {      /* new generic def can't conflict with another */
	if ((gen_class == rclass) ||
	    (!gen_class && rclass) ||
	    (gen_class && !rclass) ||
	    (rul__decl_get_class_ancestor (rclass) ==
	     rul__decl_get_class_ancestor (gen_class)))
	  return FALSE;
      }
      else { /* must have a valid generic defined for this method class */
	if (!gen_class ||
	    rul__decl_get_class_ancestor (rclass) ==
	    rul__decl_get_class_ancestor (gen_class))
	  return TRUE;
      }
    }
    meth = rul__decl_get_method_next (meth);
  }

  return TRUE;
}

