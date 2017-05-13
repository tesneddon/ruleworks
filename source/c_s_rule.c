/****************************************************************************
**                                                                         **
**                     C M P _ S E M _ R U L E . C                         **
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
 *	This module provides semantic checks for Rules.
 *
 * MODIFIED BY:
 *	DEC	Digital Equipment Corporation
 *	CPQ	Compaq Computer Corporation
 *
 * MODIFICATION HISTORY:
 *
 *	16-Dec-1992	DEC	Initial version
 *	01-Dec-1999	CPQ	Release with GPL
 */

#include <common.h>
#include <cmp_comm.h>
#include <ast.h>
#include <decl.h>
#include <val.h>
#include <lvar.h>
#include <mol.h>
#include <msg.h>
#include <cmp_msg.h>
#include <net.h>
#include <conrg.h>
#include <sem.h>


#define BOGUS_RULE_LHS_STRUCTURE 	0
#define BOGUS_CE_DISJ_STRUCTURE 	0


static Boolean sem_check_ce (Ast_Node ce, long ce_index, Boolean is_pos_ce);
static Boolean sem_check_ce_disj (Ast_Node disj, long *ce_index);
static Boolean sem_check_pred_test (
			Class ce_class, long ce_index, 
		    	Mol_Symbol attr_name, Ast_Node attr_elem,
			Ast_Node pred);
static Boolean sem_check_compound_index_expr (
			Ast_Node attr_elem_ast, long ce_index);
static Boolean sem_check_attr_vs_predicate (
			Ast_Node pred_ast, Pred_Type ptype,
		        Pred_Type opt_ptype,
			Class ce_class, Mol_Symbol attr_name,
			Ast_Node attr_index_ast,
			Decl_Domain *req_val_domain,
			Decl_Shape *req_val_shape);




/**************************
**                       **
**  RUL__SEM_CHECK_RULE  **
**                       **
**************************/

Boolean rul__sem_check_rule (Ast_Node ast)
{
   Ast_Node name, lhs, rhs, tmp, ce;
   Ast_Node_Type typ;
   Mol_Symbol rule_name;
   Boolean status = TRUE, clean;
   long ce_index = 0;

   assert (rul__ast_get_type (ast) == AST__E_RULE);
   assert (rul__ast_get_child_count (ast) == 3);	/* 3 = name,lhs, rhs */

   /*  At start of rule, clear any old lvar tables hanging around  */
   rul__lvar_clear_rule ();

   clean = TRUE;
   /*
    **  Check the rule's name against the known construct names.
    */
   name = rul__ast_get_child (ast) ;
   assert (rul__ast_get_type (name) == AST__E_CONSTANT);
   assert (rul__ast_get_value_type (name) == AST__E_TYPE_MOL);
   rule_name = (Molecule) rul__ast_get_value (name);
   
   if (rul__conrg_get_construct_type (rule_name) != RUL__C_NOISE) {
      clean = FALSE;
      rul__msg_cmp_print_w_atoms (CMP_DUPRULENAME, ast, 1, rule_name);
   }
   

   /*
    **  Check the rule's lhs
    */
   lhs = rul__ast_get_sibling (name);
   assert (rul__ast_get_type (lhs) == AST__E_RULE_LHS);
   tmp = rul__ast_get_child (lhs);
   while (tmp) {
      typ = rul__ast_get_type (tmp);
      ce_index ++;
      switch (typ) {
	 
       case AST__E_POS_CE :
	 rul__lvar_save_ce_type (ce_index, LVAR__E_POSITIVE);
	 ce = rul__ast_get_child (tmp);
	 status = sem_check_ce (ce, ce_index, TRUE);
	 break;
	 
       case AST__E_NEG_CE :
	 rul__lvar_save_ce_type (ce_index, LVAR__E_NEGATIVE);
	 ce = rul__ast_get_child (tmp);
	 status = sem_check_ce (ce, ce_index, FALSE);
	 break;
	 
       case AST__E_CE_DISJ :
	 status = sem_check_ce_disj (tmp, &ce_index);
	 break;
	 
	 default :
	   assert (BOGUS_RULE_LHS_STRUCTURE);
      }
      clean = clean && status;
      tmp = rul__ast_get_sibling (tmp);
   }
   
   /*
    **  Now check the right-hand side
    */
   rhs = rul__ast_get_sibling (lhs);
   assert (rul__ast_get_type (rhs) == AST__E_RULE_RHS);
   clean = clean && rul__sem_check_rhs (rhs); 
   
   if (clean) {
      /*  If there were no serious semantic errors  */
      rul__conrg_register_construct (rule_name, RUL__C_RULE,
				     rul__ast_nearest_line_number (ast));
      rul__net_build (lhs, rule_name);
   }
   
   return (clean);   /* True = Success */
}




/*******************
**                **
**  SEM_CHECK_CE  **
**                **
*******************/

static Boolean sem_check_ce (Ast_Node ce, long ce_index, Boolean is_pos_ce)
{
  Boolean    clean, check_value;
  Ast_Node   class_ast, attr_test_ast, attr_expr_ast,
             attr_name_ast, pred, attr_index_ast;
  Mol_Symbol class_name, attr_name, var_name;
  Class      ce_class;
  Decl_Shape attr_shape;
  char       buf[RUL_C_MAX_SYMBOL_SIZE+1];

  class_ast = rul__ast_get_child (ce);
  assert (rul__ast_get_type (class_ast) == AST__E_CONSTANT);
  assert (rul__ast_get_value_type (class_ast) == AST__E_TYPE_MOL);
  class_name = (Molecule) rul__ast_get_value (class_ast);

  ce_class = rul__decl_get_visible_class (class_name);
  if (ce_class == NULL) {
    rul__msg_cmp_print_w_atoms (CMP_UNDECCLASS, class_ast, 
				1, class_name);
    return (FALSE);
  }

  clean = TRUE;
  attr_test_ast = rul__ast_get_sibling (class_ast);
  while (attr_test_ast) {

    check_value = TRUE;

    /*
     **  First check the attribute
     */
    assert (rul__ast_get_type (attr_test_ast) == AST__E_ATTR_TEST);
    attr_expr_ast = rul__ast_get_child (attr_test_ast);
    assert (rul__ast_get_type (attr_expr_ast) == AST__E_ATTR_EXPR);
    attr_name_ast = rul__ast_get_child (attr_expr_ast);
    attr_index_ast = rul__ast_get_sibling (attr_name_ast);
    
    rul__ast_build_string (attr_test_ast);

    if (rul__ast_get_type (attr_name_ast) == AST__E_CONSTANT) {
      
      /*
       **  Was a typical constant attribute name:
       **				^ATTRIBUTE
       */
      assert (rul__ast_get_value_type (attr_name_ast) == AST__E_TYPE_MOL);
      attr_name = (Molecule) rul__ast_get_value (attr_name_ast);
      if (rul__decl_is_attr_in_class (ce_class, attr_name) == FALSE) {
	rul__msg_cmp_print_w_atoms (CMP_ATTRNOTDECL, attr_name_ast,
				    2, attr_name, class_name);
	check_value = FALSE;
	clean = FALSE;
      }
      if (clean  &&  attr_index_ast != NULL) {
	/*
	 **  There is an index into this attribute; check it.
	 **				^ATTRIBUTE[12]
	 */
	assert (rul__ast_get_type (attr_index_ast) == AST__E_ATTR_INDEX);
	attr_shape = rul__decl_get_attr_shape (ce_class, attr_name);
	if (attr_shape == shape_compound) {
	  clean = clean && 
	    sem_check_compound_index_expr (attr_index_ast, ce_index);

	  /*?* } else if (attr_shape == shape_table) {*/

	}
	else {
	  rul__msg_cmp_print_w_atoms (CMP_NOINDEXSCAL,
				      attr_index_ast, 1, attr_name);
	  check_value = FALSE;
	  clean = FALSE;
	}
      }
    }
    else {

      /*  Attribute name from a (presumably) bound variable  */
      assert (rul__ast_get_type (attr_name_ast) == AST__E_VARIABLE);
      var_name = (Molecule) rul__ast_get_value (attr_name_ast);
      
      if (! rul__lvar_variable_is_visible (var_name, ce_index)) {
	rul__msg_cmp_print_w_atoms (CMP_VARNOTBOUND,
				    attr_name_ast, 1, var_name);
	check_value = FALSE;
	clean = FALSE;
      }
      attr_name = var_name;
    }
	    
    if (check_value) {
      pred = rul__ast_get_sibling (attr_expr_ast);
      assert (rul__ast_get_type (pred) == AST__E_PRED_TEST);
      while (pred) {
	/*
	 **  One ATTR_EXPR may have multiple 
	 **  AST_E_PRED_TEST siblings 
	 */
	clean = clean && sem_check_pred_test (ce_class, ce_index, 
					      attr_name, attr_index_ast, pred);
	pred = rul__ast_get_sibling (pred);
      }
    }

    /*  Proceed to next attribute test  */
    attr_test_ast = rul__ast_get_sibling (attr_test_ast);
  }
  return (clean);
}




/************************
**                     **
**  SEM_CHECK_CE_DISJ  **
**                     **
************************/

static Boolean sem_check_ce_disj (Ast_Node disj, long *ce_index)
{
	Ast_Node ce_signed, ce;
	Ast_Node_Type typ;
	Mol_Symbol rule_name;
	Boolean status = TRUE, in_first, clean = TRUE, first = TRUE;

	ce_signed = rul__ast_get_child (disj);
	while (ce_signed) {
	    typ = rul__ast_get_type (ce_signed);
	    if (first) {
		first = FALSE;
		in_first = TRUE;
	    } else {
		*ce_index += 1;
		in_first = FALSE;
	    }

	    switch (typ) {

		case AST__E_POS_CE :
			rul__lvar_save_ce_type (*ce_index,
				(in_first ? LVAR__E_START_DISJ_POS :
					    LVAR__E_DISJ_MEMBER_POS));
			ce = rul__ast_get_child (ce_signed);
			status = sem_check_ce (ce, *ce_index, TRUE);
			break;

		case AST__E_NEG_CE :
			rul__lvar_save_ce_type (*ce_index,
				(in_first ? LVAR__E_START_DISJ_NEG :
					    LVAR__E_DISJ_MEMBER_NEG));
			ce = rul__ast_get_child (ce_signed);
			status = sem_check_ce (ce, *ce_index, FALSE);
			break;

		default:
			assert (BOGUS_CE_DISJ_STRUCTURE);
	    }
	    clean = clean && status;
	    ce_signed = rul__ast_get_sibling (ce_signed);
	}

	/*?* special disj variable binding checks -- NYI *?*/
	return (clean);
}




/**************************
**                       **
**  SEM_CHECK_PRED_TEST  **
**                       **
**************************/

static Boolean sem_check_pred_test (Class ce_class, long ce_index, 
				    Mol_Symbol attr_name,
				    Ast_Node attr_index_ast, 
				    Ast_Node pred_ast)
{
  Ast_Node        var_ast, attr_elem_ast, opt_ast;
  Ast_Node_Type   attr_elem_node_type;
  Pred_Type       ptype, optype;
  Boolean         is_a_bind;
  Molecule        var_name;
  Value           attr_elem_val, val;
  Decl_Domain     req_val_domain, match_val_domain;
  Decl_Shape      req_val_shape;

  assert (rul__ast_get_type (pred_ast) == AST__E_PRED_TEST);

  opt_ast = rul__ast_get_child (pred_ast);
  ptype = (Pred_Type) rul__ast_get_long_value (pred_ast);
  if (rul__ast_get_type (opt_ast) == AST__E_PRED_TEST) {
    optype = (Pred_Type) rul__ast_get_long_value (opt_ast);
    var_ast = rul__ast_get_sibling (opt_ast);
  }
  else {
    var_ast = rul__ast_get_child (pred_ast);
    optype = PRED__E_NOT_A_PRED;
  }

  /*  check if this is a variable binding */
  
  is_a_bind = FALSE;
  if (rul__ast_get_type (var_ast) == AST__E_VARIABLE) {
    var_name = (Molecule) rul__ast_get_value (var_ast);

    if (! rul__lvar_variable_is_visible (var_name, ce_index)) {
      /*  It's an unbound variable  */

      if (ptype == PRED__E_EQ && optype == PRED__E_NOT_A_PRED) {

	/*  It's a simple value bind  */
	rul__ast_reset_node_type (pred_ast, AST__E_LHS_VAR_BIND);
	is_a_bind = TRUE;

	if (attr_index_ast) {
	  attr_elem_val = (Value) rul__ast_get_value (attr_index_ast);
	  attr_elem_ast = rul__ast_get_child (attr_index_ast);
	  attr_elem_node_type = rul__ast_get_type (attr_elem_ast);
	}

	else {
	  attr_elem_val = NULL;
	  attr_elem_ast = NULL;
	  attr_elem_node_type = AST__E_INVALID;
	}

	if (attr_elem_node_type == AST__E_DOLLAR_LAST) {
	  /*  Add this last-elem-in-compound binding  */
	  rul__lvar_create_last_bind_desc (var_name,
					   pred_ast, ce_class, 
					   ce_index, attr_name);
	}

	else {
	  /*  Add this value binding to the variables table  */
	  rul__lvar_create_bind_desc (var_name, 
				      pred_ast, ce_class, 
				      ce_index, attr_name, 
				      rul__val_copy_value (attr_elem_val),
				      attr_elem_ast);
	}
      }

      else if (ptype == PRED__E_LENGTH_EQUAL  && attr_index_ast == NULL) {

	if (rul__decl_get_attr_shape(ce_class, attr_name) != shape_compound) {
	  rul__msg_cmp_print_w_atoms (CMP_ATTRNOLEN, 
				      pred_ast, 1, attr_name);
	  return (FALSE);
	}
	/*  It's a length-of-value binding  */
	rul__ast_reset_node_type (pred_ast, AST__E_LHS_VAR_BIND);
	is_a_bind = TRUE;
	rul__lvar_create_len_bind_desc (var_name, pred_ast,
					ce_class, ce_index, attr_name);
      }
    }
  }

  if (! is_a_bind) {
    /*
     **  Check the predicate type against the attribute's 
     **  type (and index) and the other argument's type
     */
    val = NULL;
    if (sem_check_attr_vs_predicate (pred_ast, ptype, optype, ce_class,
				     attr_name, attr_index_ast,
				     &req_val_domain, 
				     &req_val_shape)) {

      val = rul__sem_check_lhs_value (var_ast, ce_index,
				      SEM__C_RETURN_RUL_TYPE,
				      req_val_domain, req_val_shape);
    }
    if (val == NULL) return FALSE;
    rul__ast_attach_value (var_ast, AST__E_TYPE_VALUE, val);
    
    if (rul__ast_get_type (var_ast) == AST__E_VARIABLE) {
      if (ptype == PRED__E_EQ && optype == PRED__E_NOT_A_PRED) {
	
	/*  If it's a valid EQ test of an already bound variable */
	/*  see if we can refine the RHS type of this variable   */
	
	rul__lvar_found_eq_test_on_var (var_name, pred_ast, ce_class,
					ce_index, attr_name, attr_index_ast);
      }
    }
  }
  return TRUE;
}




/************************************
**                                 **
**  SEM_CHECK_COMPOUND_INDEX_EXPR  **
**                                 **
************************************/

static Boolean sem_check_compound_index_expr (
			Ast_Node attr_index_ast, long ce_index)
{
	/*
	**  Checks the element index expression for validity,
	**  and if it is valid it then attaches the value tree for
	**  that index expression to the AST__E_ATTR_INDEX node.
	*/
	Ast_Node val_ast;
	Molecule mol_val;
	long int_val;
	Value elem_expr_value = NULL;
	Boolean valid = FALSE;

	assert (rul__ast_get_type (attr_index_ast) == AST__E_ATTR_INDEX);

	val_ast = rul__ast_get_child (attr_index_ast);

	if (rul__ast_get_type (val_ast) == AST__E_CONSTANT) {

	    /*  The most common case:	an integer constant  */
	    mol_val = (Molecule) rul__ast_get_value (val_ast);
	    if (rul__mol_is_int_atom (mol_val)) {
		int_val = rul__mol_int_atom_value (mol_val);
		if (int_val > 0) {
		    elem_expr_value = rul__val_create_long_animal (int_val);
		    valid = TRUE;
		} else {
		    rul__msg_cmp_print_w_atoms (
				CMP_INVCMPIDX, val_ast, 1, mol_val);
		}
	    } else {
		rul__msg_cmp_print (CMP_INVINDEXTYP, val_ast);
	    }

	} else if (rul__ast_get_type (val_ast) == AST__E_VARIABLE) {

	    /*  A simple bound variable reference  */
	    elem_expr_value = rul__sem_check_lhs_var (
				    val_ast, ce_index,
				    dom_int_atom, shape_atom,
				    CMP__C_MSG_ATTR_INDEX);
	    if (elem_expr_value != NULL) valid = TRUE;

	} else if (rul__ast_get_type (val_ast) == AST__E_DOLLAR_LAST) {

	    /*  The special case, $last index */
	    elem_expr_value = NULL;
	    valid = TRUE;

	} else {

	    /*  For all other cases, call the generic LHS value sem checker  */
	    elem_expr_value = rul__sem_check_lhs_value (val_ast,ce_index,
							SEM__C_RETURN_RUL_TYPE,
							dom_int_atom,
							shape_atom);
	    if (elem_expr_value != NULL) valid = TRUE;
	}

	/*
	**  Now package up the index value expression (if any),
	**  and hook it to the index expression ast node.
	**
	**  If there is no valid index value, then return 
	**  FALSE to flag this rule as bogus.
	*/
	if (elem_expr_value != NULL) {
	    rul__ast_attach_value (attr_index_ast, AST__E_TYPE_VALUE, 
					elem_expr_value);
	}
	return (valid);
}





/**********************************
**                               **
**  SEM_CHECK_ATTR_VS_PREDICATE  **
**                               **
**********************************/

static Boolean	    sem_check_attr_vs_predicate (
			Ast_Node pred_ast, Pred_Type ptype, Pred_Type optype,
			Class ce_class, Mol_Symbol attr_name,
			Ast_Node attr_index_ast,
			Decl_Domain *req_val_domain,
			Decl_Shape *req_val_shape)
{
    /*
    **  This routine checks the validity of using the specified predicate
    **  expression (including the optional second predicate) with the 
    **  specified atribute expression (including the index, if any).
    **
    **  If they are valid together, it then sets the two read-write arguments
    **  with the required shape and domain for the value expression to be
    **  used with this attribute/predicate pair.
    */
    Decl_Domain  target_domain = 
			rul__decl_get_attr_domain (ce_class, attr_name);
    Decl_Shape   target_shape = shape_molecule;
    Decl_Shape   attr_expr_shape;
    Boolean	 is_a_composite_contain_test = (optype != PRED__E_NOT_A_PRED);

    if (attr_index_ast == NULL) {
	attr_expr_shape = rul__decl_get_attr_shape (ce_class, attr_name);
    } else {
	attr_expr_shape = shape_atom;
    }

    if (is_a_composite_contain_test) {
	if (ptype == PRED__E_CONTAINS  ||  ptype == PRED__E_DOES_NOT_CONTAIN) {
	    /*
	    **  Attribute expression better be compound!
	    */
	    if (attr_expr_shape != shape_compound) {
		rul__msg_cmp_print_w_atoms (
				CMP_CONTADJCOMP, pred_ast, 1, attr_name);
		return FALSE;
	    }
	    target_shape = shape_atom;
	    
	} else {
	    assert (optype == PRED__E_CONTAINS ||
		    optype == PRED__E_DOES_NOT_CONTAIN);
	    /*
	    **  Attribute expression better be scalar
	    */
	    if (attr_expr_shape != shape_atom) {
		rul__msg_cmp_print_w_atoms (
				CMP_CONTADJSCA, pred_ast, 1, attr_name);
		return FALSE;
	    }
	    target_shape = shape_compound;
	}	
    }

    switch (ptype) {

	case PRED__E_EQ:
	case PRED__E_NOT_EQ:
		if (! is_a_composite_contain_test) {
		    /*  for them to match, the shapes need to be the same */
		    target_shape = attr_expr_shape;
		}
		if (ptype == PRED__E_NOT_EQ) target_domain = dom_any;
		break;

	case PRED__E_SAME_TYPE:
	case PRED__E_DIFF_TYPE:
		if (! is_a_composite_contain_test) {
		    if (attr_expr_shape != shape_atom) {
			rul__msg_cmp_print_w_atoms (CMP_ATTRNOTSCA, pred_ast,
						    1, attr_name);
			return FALSE;
		    }
		    target_shape = shape_atom;
		}
		if (ptype == PRED__E_DIFF_TYPE) target_domain = dom_any;
		break;

	case PRED__E_EQUAL:
	case PRED__E_NOT_EQUAL:
	case PRED__E_APPROX_EQUAL:
	case PRED__E_NOT_APPROX_EQUAL:
	case PRED__E_LESS:
	case PRED__E_LESS_EQUAL:
	case PRED__E_GREATER:
	case PRED__E_GREATER_EQUAL:
		if (! is_a_composite_contain_test) {		
		    if (attr_expr_shape != shape_atom) {
			rul__msg_cmp_print_w_atoms (CMP_ATTRNOTSCA, pred_ast,
						    1, attr_name);
			return FALSE;
		    }
		    target_shape = shape_atom;
		}
		if (target_domain != dom_any) {
		    /*
		    **  If we know the domain of the attribute,
		    **  check it for reasonableness
		    */
		    if (    target_domain == dom_int_atom  ||
			    target_domain == dom_dbl_atom  ||
			    target_domain == dom_number) {
			target_domain = dom_number;
		    } else if (target_domain != dom_symbol) {
			rul__msg_cmp_print_w_atoms (
				    CMP_PREDBADTYPE, pred_ast, 1, attr_name);
			return FALSE;
		    }
		}
		break;

	case PRED__E_LENGTH_LESS_EQUAL:
	case PRED__E_LENGTH_NOT_EQUAL:
	case PRED__E_LENGTH_LESS:
	case PRED__E_LENGTH_EQUAL:
	case PRED__E_LENGTH_GREATER_EQUAL:
	case PRED__E_LENGTH_GREATER:
		assert (optype == PRED__E_NOT_A_PRED);
		if (attr_expr_shape != shape_compound) {
		    /*  Attribute better be compound  */
      		    rul__msg_cmp_print_w_atoms (CMP_ATTRNOTCOMP, pred_ast,
					        1, attr_name);
		    return FALSE;
		}
		target_shape  = shape_atom;
		target_domain = dom_int_atom;
		break;

	case PRED__E_CONTAINS:
	case PRED__E_DOES_NOT_CONTAIN:
		if (! is_a_composite_contain_test) {
		    if (attr_expr_shape == shape_compound) {
			/*  
			**  If attribute is an entire compound, 
			**  then the value needs to be atomic.
			*/
		        target_shape = shape_atom;
		    } else {
			/*  Otherwise, the value better be compound  */
			target_shape = shape_compound;
		    }
		} else {
		    /*
		    **  With a composite containment, see if we can further
		    **  refine the required domain based on the associated
		    **  scalar predicate type.
		    */
		    if (ptype == PRED__E_DOES_NOT_CONTAIN) {
			target_domain = dom_any;
		    } else {
		        switch (optype) {

			    case PRED__E_NOT_EQ:
			    case PRED__E_DIFF_TYPE:
				target_domain = dom_any;
				break;

			    case PRED__E_EQUAL:
			    case PRED__E_NOT_EQUAL:
			    case PRED__E_APPROX_EQUAL:
			    case PRED__E_NOT_APPROX_EQUAL:
			    case PRED__E_LESS:
			    case PRED__E_LESS_EQUAL:
			    case PRED__E_GREATER:
			    case PRED__E_GREATER_EQUAL:
				if (target_domain != dom_any) {
				    /*
				    **  If we know the domain of the attribute,
				    **  check it for reasonableness
				    */
				    if (    target_domain == dom_int_atom  ||
					    target_domain == dom_dbl_atom  ||
					    target_domain == dom_number) {
					target_domain = dom_number;
				    } else if (target_domain != dom_symbol) {
					rul__msg_cmp_print_w_atoms (
						    CMP_PREDBADTYPE, 
						    pred_ast, 1, attr_name);
					return FALSE;
				    }
				}
				break;		
			}
		    }
	        }
		break;
    }

    *req_val_domain = target_domain;
    *req_val_shape  = target_shape;
    return TRUE;
}





/**********************************
**                               **
**  RUL__SEM_CHECK_VAL_PRED_VAL  **
**                               **
**********************************/

Boolean	 rul__sem_check_val_pred_val (Ast_Node node,
				      Pred_Type ptype, Pred_Type optype,
				      Value val1, Value val2)
{
  Decl_Domain  dom1   = rul__val_get_domain (val1);
  Decl_Shape   shape1 = rul__val_get_shape (val1);
  Decl_Domain  dom2   = rul__val_get_domain (val2);
  Decl_Shape   shape2 = rul__val_get_shape (val2);
  char        *buf = NULL;

  switch (ptype) {
  case PRED__E_EQ:
  case PRED__E_NOT_EQ:

    if (optype != PRED__E_NOT_A_PRED) {

      if (shape1 != shape_atom)
	buf = "predicate requires a scalar value";

      else if (shape2 != shape_compound)
	buf = "predicate requires a compound value";

      if (buf) {
	rul__msg_cmp_print (CMP_INVVALPRED, node, buf, "","");
	return FALSE;
      }
    }
    break;

  case PRED__E_SAME_TYPE:
  case PRED__E_DIFF_TYPE:
    if (optype == PRED__E_NOT_A_PRED) {
      /* values must both be scalar */
      if (shape1 != shape_atom || shape2 != shape_atom) {
	rul__msg_cmp_print (CMP_INVVALPRED, node,
			    "predicate requires scalar values", "","");
	return FALSE;
      }
    }
    break;
      
  case PRED__E_EQUAL:
  case PRED__E_NOT_EQUAL:
  case PRED__E_APPROX_EQUAL:
  case PRED__E_NOT_APPROX_EQUAL:
  case PRED__E_LESS:
  case PRED__E_LESS_EQUAL:
  case PRED__E_GREATER:
  case PRED__E_GREATER_EQUAL:

    if ((dom1 != dom2 && dom1 != dom_any && dom2 != dom_any) &&
	(!((dom1 == dom_number || dom1 == dom_int_atom ||
	    dom1 == dom_dbl_atom) &&
	   (dom2 == dom_number || dom2 == dom_int_atom ||
	    dom2 == dom_dbl_atom)))) {
      rul__msg_cmp_print (CMP_INVVALPRED, node,
			  "predicate requires compatible value types", "","");
      return FALSE;
    }

    if (optype == PRED__E_NOT_A_PRED) {

      /* values must be scalar and of compatible domains */
      if (shape1 != shape_atom || shape2 != shape_atom) {
	rul__msg_cmp_print (CMP_INVVALPRED, node,
			    "predicate requires scalar values", "","");
	return FALSE;
      }
    }
    
    else {
      /* values are scalar and compound and of compatible domains */
      if (shape1 != shape_atom) {
	rul__msg_cmp_print (CMP_INVVALPRED, node,
			    "predicate requires scalar value", "","");
	return FALSE;
      }

      if (shape2 != shape_compound) {
	rul__msg_cmp_print (CMP_INVVALPRED, node,
			    "predicate requires compound value", "","");
	return FALSE;
      }
    }
    break;
    
  case PRED__E_LENGTH_LESS_EQUAL:
  case PRED__E_LENGTH_NOT_EQUAL:
  case PRED__E_LENGTH_LESS:
  case PRED__E_LENGTH_EQUAL:
  case PRED__E_LENGTH_GREATER_EQUAL:
  case PRED__E_LENGTH_GREATER:

    if (shape1 != shape_compound)
      buf = "predicate requires first value to be compound";

    else if (shape2 != shape_atom)
      buf = "predicate requires second value to be scalar";

    if (buf) {
      rul__msg_cmp_print (CMP_INVVALPRED, node, buf, "","");
      return FALSE;
    }
    break;

  case PRED__E_CONTAINS:
  case PRED__E_DOES_NOT_CONTAIN:
    if (optype == PRED__E_NOT_A_PRED) {

      if (shape1 != shape_atom && shape2 != shape_atom)
	buf = "predicate requires one scalar value";

      else if (shape1 != shape_compound && shape2 != shape_compound)
	buf = "predicate requires one compound value";

      if (buf) {
	rul__msg_cmp_print (CMP_INVVALPRED, node, buf, "","");
	return FALSE;
      }
    }
    else {
      switch (optype) {
      case PRED__E_EQ:
      case PRED__E_NOT_EQ:
      case PRED__E_SAME_TYPE:
      case PRED__E_DIFF_TYPE:
	if (shape1 != shape_compound)
	  buf = "predicate requires a compound value";
	else if (shape2 != shape_atom)
	  buf = "predicate requires a scalar value";
	if (buf) {
	  rul__msg_cmp_print (CMP_INVVALPRED, node, buf, "","");
	  return FALSE;
	}
	break;
	
      case PRED__E_EQUAL:
      case PRED__E_NOT_EQUAL:
      case PRED__E_APPROX_EQUAL:
      case PRED__E_NOT_APPROX_EQUAL:
      case PRED__E_LESS:
      case PRED__E_LESS_EQUAL:
      case PRED__E_GREATER:
      case PRED__E_GREATER_EQUAL:
	if ((dom1 != dom2 && dom1 != dom_any && dom2 != dom_any) &&
	    (!((dom1 == dom_number || dom1 == dom_int_atom ||
		dom1 == dom_dbl_atom) &&
	       (dom2 == dom_number || dom2 == dom_int_atom ||
		dom2 == dom_dbl_atom)))) {
	  rul__msg_cmp_print (CMP_INVVALPRED, node,
		      "predicate requires compatible value types", "","");
	  return FALSE;
	}

	/* values are scalar and compound and of compatible domains */
	if (shape1 != shape_compound) {
	  rul__msg_cmp_print (CMP_INVVALPRED, node,
			      "predicate requires compound value", "","");
	  return FALSE;
	}
	if (shape2 != shape_atom) {
	  rul__msg_cmp_print (CMP_INVVALPRED, node,
			      "predicate requires scalar value", "","");
	  return FALSE;
	}
      }
      break;
    }
  }
  return TRUE;
}

