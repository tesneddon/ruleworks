/****************************************************************************
**                                                                         **
**                     C M P _ S E M _ R H S . C                           **
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
 *	This module provides semantic checks for Rhss.
 *
 * MODIFIED BY:
 *	DEC	Digital Equipment Corporation
 *	CPQ	Compaq Computer Corporation
 *
 * MODIFICATION HISTORY:
 *
 *	11-Jan-1993	DEC	Initial version
 *
 *	10-Jun-1994	DEC	Set shape and domain of constants, even for
 *					animal types.
 *
 *	01-Dec-1999	CPQ	Release with GPL
 */

#include <common.h>
#include <cmp_comm.h>
#include <mol.h>
#include <ast.h>
#include <sem.h>
#include <val.h>
#include <msg.h>
#include <cmp_msg.h>

static Boolean sql_each;	/* flag for checking on sql nexting */
static Boolean ret_done;	/* flag for a return action         */
static Boolean quit_done;	/* flag for a quit action         */

/**************************
**                       **
**  RUL__SEM_CHECK_RHS   **
**                       **
**************************/

Boolean
rul__sem_check_rhs (Ast_Node ast)
{
  Boolean     status = TRUE;          /* TRUE for success */
  Ast_Node    action_node = NULL;
  Value	      action_value;

  assert (rul__ast_get_type (ast) == AST__E_RULE_RHS ||
	  rul__ast_get_type (ast) == AST__E_CATCH ||
	  rul__ast_get_type (ast) == AST__E_METHOD_RHS);

  sql_each = FALSE;
  ret_done = FALSE;
  quit_done = FALSE;

  /* get first action node based on type... */
  if (rul__ast_get_type (ast) == AST__E_RULE_RHS) {
    rul__sem_use_entry_vars (FALSE);
    rul__sem_initialize_rhs_vars();
    action_node = rul__ast_get_child (ast);
  }
  else if (rul__ast_get_type (ast) == AST__E_CATCH) {
    rul__sem_use_entry_vars (FALSE);
    rul__sem_initialize_rhs_vars();
    action_node = rul__ast_get_sibling (rul__ast_get_child (ast));
  }
  else if (rul__ast_get_type (ast) == AST__E_METHOD_RHS) {
    action_node = rul__ast_get_child (ast);
  }

  for (;
       action_node != NULL;
       action_node = rul__ast_get_sibling (action_node)) {

    if (ret_done == TRUE) {
      rul__msg_cmp_print(CMP_RETACTEXE, action_node);
      break;
    }

    if (quit_done == TRUE) {
      rul__msg_cmp_print(CMP_QUTACTEXE, action_node);
      break;
    }

    action_value = rul__sem_check_rhs_action (action_node, 0);

    /* reset external storage requirements for next action... */
    rul__sem_rhs_action_end_vars();

    /* store code generation value as action AST node value */
    if (action_value == NULL) 
      status = FALSE;
    else {
      rul__ast_attach_value (action_node, AST__E_TYPE_VALUE, action_value);
    }
  }

  /*  
  **  At the end of the rule, now that we know the set of all
  **  LHS variables that are used on the RHS, set up all the 
  **  initializations of the RHS incarnations of those LHS variables,
  **  and treat this as a separate action for the sake of the allocation
  **  of any temporary variables used in those initializations.
  */
  if (status) {
    rul__sem_set_inits_of_lhs_vars ();
    rul__sem_rhs_action_end_vars ();
  }
  return status;
}

/*********************************
**                              **
**  RUL__SEM_CHECK_RHS_ACTION   **
**                              **
*********************************/

Value
rul__sem_check_rhs_action (Ast_Node action_node, Boolean in_loop)
{
  Value	      action_value;

  switch (rul__ast_get_type (action_node)) {
    
  case AST__E_ADDSTATE_ACTION:
    action_value = rul__sem_check_addstate (action_node);
    break;
    
  case AST__E_AFTER_ACTION:
    action_value = rul__sem_check_after (action_node);
    break;
    
  case AST__E_AT_ACTION:
    action_value = rul__sem_check_at (action_node);
    break;
    
  case AST__E_BIND_ACTION:
    action_value = rul__sem_check_bind (action_node, in_loop);
    break;
    
  case AST__E_BUILD_ACTION:
    action_value = rul__sem_check_build (action_node);
    break;
    
  case AST__E_CALL_INHERITED:
    action_value = rul__sem_check_call_inherited (action_node, 
					  SEM__C_NO_RETURN_VALUE_REQ, NULL);
    break;
    
  case AST__E_CLOSEFILE_ACTION:
    action_value = rul__sem_check_closefile (action_node);
    break;
    
  case AST__E_COPY_ACTION:
    action_value = rul__sem_check_copy (action_node, 
			  SEM__C_NO_RETURN_VALUE_REQ, NULL);
    break;
    
  case AST__E_DEBUG_ACTION:
    action_value = rul__sem_check_debug (action_node);
    break;
    
  case AST__E_DEFAULT_ACTION:
    action_value = rul__sem_check_default (action_node);
    break;
    
  case AST__E_EXT_RT_CALL:
    action_value = rul__sem_check_ext_rt_call (action_node,
					       SEM__C_NO_RETURN_VALUE_REQ,
				 	       SEM__C_ON_RHS,
					       SEM__C_UNDEFINED_CE_INDEX,
					       SEM__C_RETURN_EXT_TYPE,
					       dom_any, shape_molecule,
					       NULL);
    break;
    
  case AST__E_FOR_EACH_ACTION:
    action_value = rul__sem_check_for_each (action_node);
    break;
    
  case AST__E_IF_ACTION:
    action_value = rul__sem_check_if (action_node);
    break;
    
  case AST__E_MAKE_ACTION:
    action_value = rul__sem_check_make (action_node,
					SEM__C_NO_RETURN_VALUE_REQ, NULL);
    break;
    
  case AST__E_MODIFY_ACTION:
    action_value = rul__sem_check_modify (action_node, 
					  SEM__C_NO_RETURN_VALUE_REQ, NULL);
    break;
    
  case AST__E_OPENFILE_ACTION:
    action_value = rul__sem_check_openfile (action_node);
    break;
    
  case AST__E_QUIT_ACTION:
    action_value = rul__sem_check_quit (action_node);
    quit_done = TRUE;
    break;
    
  case AST__E_REMOVE_ACTION:
    action_value = rul__sem_check_remove (action_node);
    break;
    
  case AST__E_REMOVE_EVERY_ACTION:
    action_value = rul__sem_check_remove_every (action_node);
    break;
    
  case AST__E_RESTORESTATE_ACTION:
    action_value = rul__sem_check_restorestate (action_node);
    break;
    
  case AST__E_RETURN_ACTION:
    action_value = rul__sem_check_return (action_node);
    ret_done = TRUE;
    break;
    
  case AST__E_SAVESTATE_ACTION:
    action_value = rul__sem_check_savestate (action_node);
    break;
    
  case AST__E_SPECIALIZE_ACTION:
    action_value = rul__sem_check_specialize (action_node, 
					      SEM__C_NO_RETURN_VALUE_REQ,NULL);
    break;

  case AST__E_TRACE_ACTION:
    action_value = rul__sem_check_trace (action_node);
    break;
      
  case AST__E_WHILE_ACTION:
    action_value = rul__sem_check_while (action_node);
    break;
    
   case AST__E_WRITE_ACTION:
    action_value = rul__sem_check_write (action_node);
   break;

  case AST__E_SQL_ATTACH_ACTION:
    if (sql_each == TRUE) {
      rul__msg_cmp_print(CMP_INVSQLACT, action_node);
      action_value = NULL;
    }
    action_value = rul__sem_check_sql_attach (action_node);
    break;
    
  case AST__E_SQL_COMMIT_ACTION:
    if (sql_each == TRUE) {
      rul__msg_cmp_print(CMP_INVSQLACT, action_node);
      action_value = NULL;
    }
    action_value = rul__sem_check_sql_commit (action_node);
    break;
    
  case AST__E_SQL_DELETE_ACTION:
    if (sql_each == TRUE) {
      rul__msg_cmp_print(CMP_INVSQLACT, action_node);
      action_value = NULL;
    }
    action_value = rul__sem_check_sql_delete (action_node);
    break;
    
  case AST__E_SQL_DETACH_ACTION:
    if (sql_each == TRUE) {
      rul__msg_cmp_print(CMP_INVSQLACT, action_node);
      action_value = NULL;
    }
    action_value = rul__sem_check_sql_detach (action_node);
    break;
    
  case AST__E_SQL_FETCH_AS_OBJECT_ACTION:
    if (sql_each == TRUE) {
      rul__msg_cmp_print(CMP_INVSQLACT, action_node);
      action_value = NULL;
    }
    action_value = rul__sem_check_sql_fetch_obj (action_node,
						 SEM__C_NO_RETURN_VALUE_REQ,
						 NULL);
    break;
    
  case AST__E_SQL_FETCH_EACH_ACTION:
    if (sql_each == TRUE) {
      rul__msg_cmp_print(CMP_INVSQLACT, action_node);
      action_value = NULL;
    }
    else {
      sql_each = TRUE;
      action_value = rul__sem_check_sql_fetch_each (action_node);
    }
    break;
    
  case AST__E_SQL_INSERT_ACTION:
    if (sql_each == TRUE) {
      rul__msg_cmp_print(CMP_INVSQLACT, action_node);
      action_value = NULL;
    }
    action_value = rul__sem_check_sql_insert (action_node);
    break;
    
  case AST__E_SQL_INSERT_FROM_OBJECT_ACTION:
    if (sql_each == TRUE) {
      rul__msg_cmp_print(CMP_INVSQLACT, action_node);
      action_value = NULL;
    }
    action_value = rul__sem_check_sql_insert_obj (action_node);
    break;
    
  case AST__E_SQL_ROLLBACK_ACTION:
    if (sql_each == TRUE) {
      rul__msg_cmp_print(CMP_INVSQLACT, action_node);
      action_value = NULL;
    }
    action_value = rul__sem_check_sql_rollback (action_node);
    break;
    
  case AST__E_SQL_START_ACTION:
    if (sql_each == TRUE) {
      rul__msg_cmp_print(CMP_INVSQLACT, action_node);
      action_value = NULL;
    }
    action_value = rul__sem_check_sql_start (action_node);
    break;
    
  case AST__E_SQL_UPDATE_ACTION:
    if (sql_each == TRUE) {
      rul__msg_cmp_print(CMP_INVSQLACT, action_node);
      action_value = NULL;
    }
    action_value = rul__sem_check_sql_update (action_node);
    break;
    
  case AST__E_SQL_UPDATE_FROM_OBJECT_ACTION:
    if (sql_each == TRUE) {
      rul__msg_cmp_print(CMP_INVSQLACT, action_node);
      action_value = NULL;
    }
    action_value = rul__sem_check_sql_update_obj (action_node);
    break;
      
  default:
    rul__msg_cmp_print(CMP_INVRHSACT, action_node);
    action_value = NULL; /* should never get here??? */

  }

  return action_value;
}

Value
rul__sem_check_rul_constant (Ast_Node ast, Boolean return_molecule)
{
    Molecule    mole;
    Value	constant_value;

    mole = (Molecule) rul__ast_get_value (ast);
    if (return_molecule || rul__mol_get_shape (mole) == shape_compound)
	{
	constant_value = rul__val_create_rul_constant (mole);
	}
    else
	{
	/* return animal constant value */
	switch (rul__mol_get_value_type (mole))
	    {
	    case int_atom:
		constant_value = rul__val_create_long_animal (
					rul__mol_int_atom_value (
						(Mol_Int_Atom) mole));
		break;
					
	    case dbl_atom:
		constant_value = rul__val_create_double_animal (
					rul__mol_dbl_atom_value (
						(Mol_Dbl_Atom) mole));
		break;
       
	    default:
		constant_value = rul__val_create_asciz_animal (
					rul__mol_get_printform (mole));
		break;
	    }
	}
    rul__val_set_shape_and_domain (constant_value,
				   rul__mol_get_shape (mole),
				   rul__mol_get_domain (mole));
    return constant_value;
}
