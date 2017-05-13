/****************************************************************************
**                                                                         **
**                C M P _ G E N _ O N _ C L A U S E . C                    **
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
 *	This module provides the code generation for an ON-clause.
 *
 * MODIFIED BY:
 *	DEC	Digital Equipment Corporation
 *	CPQ	Compaq Compuetr Corporation
 * MODIFICATION HISTORY:
 *
 *	 5-Mar-1993	DEC	Chuck Doucette
 *	01-Dec-1999	CPQ	Release with GPL
 */

#include <common.h>
#include <cmp_comm.h>
#include <ast.h>
#include <val.h>
#include <emit.h>
#include <gen.h>
#include <conrg.h>
#include <sem.h>

Ast_Node      return_node;
Value         return_value;

#define RUL__C_MAX_FUNC_NAME_SIZE 31

char *
rul___gen_on_clause_func_name (Ast_Node_Type ast_type)
{
  static char on_clause_func_name[RUL__C_MAX_FUNC_NAME_SIZE];
  unsigned int eb_index = rul__conrg_get_cur_block_index();
  char *ast_type_name = "?";

  switch (ast_type) {
  case AST__E_ON_ENTRY :
    ast_type_name = "on_entry";
    break;
  case AST__E_ON_EVERY :
    ast_type_name = "on_every";
    break;
  case AST__E_ON_EMPTY :
    ast_type_name = "on_empty";
    break;
  case AST__E_ON_EXIT  :
    ast_type_name = "on_exit";
    break;
  default :
    assert (FALSE); /* we shouldn't get here */
    break;
  }

  sprintf (on_clause_func_name, "%s____%d_%s", GEN_NAME_PREFIX,
	   eb_index, ast_type_name);

  return on_clause_func_name;
}


/************************
**                     **
**  RUL__GEN_ON_CLAUSE **
**                     **
************************/

void
rul__gen_on_clause (Ast_Node ast)
{
  Ast_Node_Type	ast_type = rul__ast_get_type(ast);
  Ast_Node	action_node;

  /*This is redundant assuming we were called from rul__gen_constructs_code()*/
  assert ((ast_type == AST__E_ON_ENTRY) ||
	  (ast_type == AST__E_ON_EXIT)  ||
	  (ast_type == AST__E_ON_EVERY) ||
	  (ast_type == AST__E_ON_EMPTY));

  /* Emit on-clause function entry point */

  rul__emit_blank_line();

  rul__emit_entry_point_str (rul___gen_on_clause_func_name (ast_type),
			     EMIT__E_SCOPE_STATIC,
			     EMIT__E_HAS_RETURN,
			     ext_type_long,
			     EXT__C_NOT_ARRAY,
			     ext_mech_value);
  rul__emit_entry_arg_internal (CMP__E_INT_MOLECULE,
				GEN__C_RHS_VARIABLE_STR, 
				EXT__C_NOT_ARRAY,
				ext_mech_ref_rw);
  rul__emit_entry_arg_internal (CMP__E_INT_ENTRY_DATA,
				"eb_data",
				EXT__C_NOT_ARRAY,
				ext_mech_value);
  rul__emit_entry_args_done (EMIT__E_ENTRY_DEFINITION);

  /* Emit RHS function local declarations */

  rul__gen_rhs_tmp_decl ();

  rul__gen_on_rhs (ast);

  rul__emit_entry_done ();
}

