/****************************************************************************
**                                                                         **
**                     C M P _ S E M _ O N _ C L A U S E . C               **
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
 *	This module provides semantic checks for ON-clauses.
 *
 * MODIFIED BY:
 *	DEC	Digital Equipment Corporation
 *	CPQ	Compaq Computer Corporation
 *
 * MODIFICATION HISTORY:
 *	 4-Mar-1993	DEC	Initial version
 *	01-Jul-1998	DEC	call to rul__sem_check_rhs_action now has 
 *					second param to indicate not in loop.
 *	01-Dec-1999	CPQ	Releasdew ith GPL
 */

#include <common.h>
#include <cmp_comm.h>
#include <ast.h>
#include <sem.h>
#include <decl.h>
#include <msg.h>
#include <cmp_msg.h>
#include <conrg.h>

static Boolean sql_each;        /* flag for checking on sql nexting */
static Boolean ret_done;        /* flag for a return action         */
static Boolean quit_done;       /* flag for a quit action         */
 
/*******************************
**                            **
**  RUL__SEM_CHECK_ON_CLAUSE  **
**                            **
*******************************/

Boolean
rul__sem_check_on_clause (Ast_Node ast)
{
  Ast_Node_Type	ast_type = rul__ast_get_type(ast);
  Boolean     status = TRUE;          /* TRUE for success */
  Ast_Node    action_node;
  Value	      action_value;
  On_Clause_Type on_type;

  /*This is redundant assuming we were called from rul__sem_check_construct()*/
  assert ((ast_type == AST__E_ON_ENTRY) ||
	  (ast_type == AST__E_ON_EXIT)  ||
	  (ast_type == AST__E_ON_EVERY) ||
	  (ast_type == AST__E_ON_EMPTY));

  if (rul__conrg_get_block_type (
		 rul__conrg_get_cur_block_name ()) != RUL__C_ENTRY_BLOCK) {
    rul__msg_cmp_print (CMP_ONBADBLK, ast,
			rul___ast_type_to_string (ast_type));
    return FALSE;
  }

  if (ast_type == AST__E_ON_ENTRY)
    on_type = DECL_ON_ENTRY;
  else if (ast_type == AST__E_ON_EXIT)
    on_type = DECL_ON_EXIT;
  else if (ast_type == AST__E_ON_EMPTY)
    on_type = DECL_ON_EMPTY;
  else if (ast_type == AST__E_ON_EVERY)
    on_type = DECL_ON_EVERY;

  if (rul__decl_eb_was_on_clause_seen (on_type)) {
    rul__msg_cmp_print (CMP_ONDUPCLS, ast,
			rul___ast_type_to_string (ast_type));
    return FALSE;
  }

  sql_each = FALSE;
  ret_done = FALSE;
  quit_done = FALSE;

  rul__sem_use_entry_vars (TRUE);

  /* get first action node based on type... */
  action_node = rul__ast_get_child (ast);

  for (;
       action_node != NULL;
       action_node = rul__ast_get_sibling (action_node)) {

    ast_type = rul__ast_get_type (action_node);

    if (ret_done == TRUE) {
      rul__msg_cmp_print(CMP_RETACTEXE, action_node);
      break;
    }
    
    if (quit_done == TRUE) {
      rul__msg_cmp_print(CMP_QUTACTEXE, action_node);
      break;
    }

    action_value = rul__sem_check_rhs_action (action_node, 0);

    if (ast_type == AST__E_RETURN_ACTION)
      ret_done = TRUE;
    else if (ast_type == AST__E_QUIT_ACTION)
      quit_done = TRUE;
    
    /* reset external storage requirements for next action... */
    rul__sem_rhs_action_end_vars ();

    /* store code generation value as action AST node value */
    if (action_value == NULL) 
      status = FALSE;
    else {
      rul__ast_attach_value (action_node, AST__E_TYPE_VALUE, action_value);
    }
  }

  if (status == TRUE)
    rul__decl_eb_on_clause_was_seen (on_type);
  else
    rul__sem_init_temp_rhs_vars ();

  return status;
}

