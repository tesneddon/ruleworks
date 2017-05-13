/****************************************************************************
**                                                                         **
**                   C M P _ G E N _ C A T C H E R . C                     **
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
 *	This module provides the code generation for catchers.
 *
 * MODIFIED BY:
 *	DEC	Digital Equipment Corporation
 *	CPQ	Compaq Compuetr Corporation
 *
 * MODIFICATION HISTORY:
 *
 *	 1-Jul-1993	DEC	Chuck Doucette
 *	01-Dec-1999	CPQ	Release with GPL
 */

#include <common.h>
#include <cmp_comm.h>
#include <mol.h>
#include <ast.h>
#include <val.h>
#include <emit.h>
#include <gen.h>
#include <conrg.h>
#include <sem.h>

#define RUL__C_MAX_FUNC_NAME_SIZE 31

static char *
gen_catcher_func_name (Mol_Symbol catcher_name_sym)
{
  static char catcher_func_name[RUL__C_MAX_FUNC_NAME_SIZE];

  sprintf (catcher_func_name, "%s%ld_catch_%ld", GEN_NAME_PREFIX,
	   rul__conrg_get_cur_block_index (),
	   rul__conrg_get_construct_index (catcher_name_sym));

  return catcher_func_name;
}


/***************************
**                        **
**  RUL__GEN_CATCHER_FUNC **
**                        **
***************************/

void rul__gen_catcher_func (Ast_Node ast)
{
  Mol_Symbol	catcher_name_sym;

  /* Emit catcher function entry point */

  rul__emit_blank_line();

  catcher_name_sym = rul__ast_get_value (rul__ast_get_child (ast));
  rul__emit_entry_point_str (gen_catcher_func_name (catcher_name_sym),
			     EMIT__E_SCOPE_STATIC,
			     EMIT__E_HAS_RETURN,
			     ext_type_long,
			     EXT__C_NOT_ARRAY,
			     ext_mech_value);
  rul__emit_entry_arg_internal (CMP__E_INT_ENTRY_DATA,
				"eb_data",
				EXT__C_NOT_ARRAY,
				ext_mech_value);
  rul__emit_entry_args_done (EMIT__E_ENTRY_DEFINITION);

  /* Emit catcher function local declarations and initializations */

  rul__gen_rhs_tmp_decl ();
  rul__gen_rhs_var_decl ();
  rul__gen_rhs_tmp_mol_inits ();
  rul__gen_rhs_var_inits ();

  rul__gen_on_rhs (ast); /* use same routine as the ON-* clauses */

  rul__emit_entry_done ();
}

void rul__gen_catcher_tables (long block_num)
{
  void *construct = NULL;
  Mol_Symbol cname;
  Construct_Type ctype;
  long catcher_count = 0;
  Value assignment;
  char catcher_names_array[RUL_C_MAX_SYMBOL_SIZE];
  char catcher_funcs_array[RUL_C_MAX_SYMBOL_SIZE];

  sprintf (catcher_names_array, "%s%ld_catch_names", 
		GEN_NAME_PREFIX, block_num);
  sprintf (catcher_funcs_array, "%s%ld_catch_funcs", 
		GEN_NAME_PREFIX, block_num);

  while (rul__conrg_get_next_construct (&construct, &cname, &ctype))
    if (ctype == RUL__C_CATCH) {
      assignment = rul__val_create_assignment
	(rul__val_create_blk_or_loc_vec (CMP__E_INT_MOLECULE,
					 catcher_names_array,
					 catcher_count),
	 rul__val_create_rul_constant (cname));
      rul__emit_value_statement (assignment);
      rul__val_free_value (assignment);

      assignment = rul__val_create_assignment
	(rul__val_create_blk_or_loc_vec (CMP__E_INT_MOLECULE,
					 catcher_funcs_array,
					 catcher_count++),
	 rul__val_create_blk_or_loc (CMP__E_INT_CATCHER_FUNC,
				     gen_catcher_func_name (cname)));
      rul__emit_value_statement (assignment);
      rul__val_free_value (assignment);
    }

  assert (catcher_count == rul__conrg_get_catcher_count ());
}
				       
