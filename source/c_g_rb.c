/* cmp_gen_rule_block.c - generate code for a rule block		*/
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
**	The function corresponding to the rule block is generated.
**
**  MODIFIED BY:
**	DEc	Digital Euqipment Corporation
*	CPQ	Compaq Computer Corporation
**
**  MODIFICATION HISTORY:
**
**	 3-Jun-1993	DEC	Initial version
*	01-Dec-1999	CPQ	Release with GPL
*/

#include <common.h>
#include <cmp_comm.h>
#include <mol.h>
#include <list.h>
#include <conrg.h>
#include <val.h>
#include <emit.h>
#include <decl.h>
#include <gen.h>
#include <net.h>
#include <cli.h>
#include <sem.h>

/* NOTE: most of the code here was lifted from rul__gen_entry_block(). */

void
rul__gen_rule_block (void)
{
  Mol_Symbol	function_name,block_name;
  long		block_index;
  long		num_of_decling_blocks = 1; /* Number decl blocks USESed + 1 */
  List		block_list;
  Name_Scope	scope;
  char		buffer[RUL_C_MAX_SYMBOL_SIZE+1];
  char		buffer1[RUL_C_MAX_SYMBOL_SIZE+1];
  Value		func,assign_val;
  long		i;
  Strategy	rb_strategy = rul__decl_get_eb_strategy ();
  Boolean       rules_seen = rul__sem_rules_seen ();

  /*
   * Determine the entry block's name (e.g. "count_to") and the index of
   * functions created for this block.
   */
  function_name = rul__conrg_get_cur_block_name();
  block_index = rul__conrg_get_cur_block_index();

  /* Count: num_of_decling_blocks */
  for (block_list = rul__decl_get_visible_decl_blks ();
       !rul__list_is_empty (block_list);
       block_list = rul__list_rest (block_list)) {
    block_name = rul__list_first (block_list);
    if ((block_name != function_name) &&
	(rul__decl_block_has_classes (block_name))) {
      num_of_decling_blocks = num_of_decling_blocks + 1;
      /* Generate a function prototype for the "used" decl block */
      rul__mol_use_printform (block_name, buffer, sizeof (buffer));
      rul__emit_entry_point_str(buffer, EMIT__E_SCOPE_GLOBAL,
				EMIT__E_NO_RETURN, ext_type_invalid,
				EXT__C_NOT_ARRAY, ext_mech_invalid);
      rul__emit_entry_args_done(EMIT__E_ENTRY_DECLARATION);
    }
  }	 

  /*
   * Generate forward declarations for declaring block routines.
   */
  if (rul__conrg_get_block_type (function_name) == RUL__C_DECL_BLOCK) {
    scope = EMIT__E_SCOPE_GLOBAL;
    rul__mol_use_printform (function_name, buffer, sizeof (buffer));
  }
  else {
    scope = EMIT__E_SCOPE_STATIC;
    sprintf(buffer, "%s____db%ld", GEN_NAME_PREFIX, block_index);
  }

  rul__emit_entry_point_str(buffer, scope,
			    EMIT__E_NO_RETURN, ext_type_invalid,
			    EXT__C_NOT_ARRAY, ext_mech_invalid);
  rul__emit_entry_args_done(EMIT__E_ENTRY_DECLARATION);


  /*
   * Generate declarations for dblock variables.
   */
  rul__gen_declare_dblock_vars ();

  /*
   * Generate forward declarations for propogation functions.
   */
  if (rules_seen)
    rul__gen_prop_func_forw_decls(num_of_decling_blocks);

  /*
   * Generate the function header.
   */
  rul__emit_page_break();

  rul__emit_entry_point(function_name,
			EMIT__E_SCOPE_GLOBAL,
			EMIT__E_NO_RETURN,
			ext_type_invalid,
			EXT__C_NOT_ARRAY,
			ext_mech_invalid);
  rul__emit_entry_args_done(EMIT__E_ENTRY_DEFINITION);

  /* 
   * Call the declaring blocks used by this entry block.
   */
  for (block_list = rul__decl_get_visible_decl_blks ();
       !rul__list_is_empty (block_list);
       block_list = rul__list_rest (block_list)) {
    block_name = rul__list_first (block_list);
    if ((block_name != function_name) &&
	(rul__decl_block_has_classes (block_name))) {
      func = rul__val_create_function_str (
				   rul__mol_get_printform (block_name), 0);
      rul__emit_value_statement (func);
      rul__val_free_value(func);
    }
  }	 

  /*
   * Call the declaring block associated with this entry block.
   */
  sprintf(buffer, "%s____db%ld", GEN_NAME_PREFIX, block_index);
  func = rul__val_create_function_str(buffer, 0);
  rul__emit_value_statement(func);
  rul__val_free_value(func);
  
  /*
   * Emit assignment for number of decling blocks
   */
  sprintf(buffer, "%s%ld_db_count", GEN_NAME_PREFIX, block_index);
  assign_val = rul__val_create_assignment (
		   rul__val_create_blk_or_loc (CMP__E_INT_LONG, buffer),
		       rul__val_create_long_animal (num_of_decling_blocks));
  rul__emit_value_statement (assign_val);
  rul__val_free_value (assign_val);
  
  /*
   * Loop through declaring blocks, assigning block names and propogate
   * function addresses to these arrays.
   */
  for (block_list = rul__decl_get_visible_decl_blks (), i=0;
       !rul__list_is_empty (block_list);
       block_list = rul__list_rest (block_list)) {
    block_name = rul__list_first (block_list);
    if ((block_name == function_name) ||
	(rul__decl_block_has_classes (block_name))) {
      (void) rul__net_create_decl_node (block_name);
      sprintf(buffer, "%s%ld_db_names", GEN_NAME_PREFIX, block_index);
      assign_val = rul__val_create_assignment (
	       rul__val_create_blk_or_loc_vec (CMP__E_INT_MOLECULE, buffer, i),
			       rul__val_create_rul_constant (block_name));
      rul__emit_value_statement (assign_val);
      rul__val_free_value (assign_val);

      if (rules_seen) {
	sprintf(buffer, "%s%ld_pr_db%ld", GEN_NAME_PREFIX, block_index, i + 1);
	sprintf(buffer1, "%s%ld_db_funcs", GEN_NAME_PREFIX, block_index);
	assign_val =
	  rul__val_create_assignment 
	    (rul__val_create_blk_or_loc_vec (CMP__E_INT_PROP_FUNC, buffer1, i),
	     rul__val_create_blk_or_loc (CMP__E_INT_PROP_FUNC, buffer));
	rul__emit_value_statement(assign_val);
	rul__val_free_value(assign_val);
      }
      i = i + 1;
    }
  }

  /*
   * Emit a call to register the ruling block associated with this entry
   * block.
   */
  func = rul__val_create_function_str("rul__rbs_register_rblock", 16);
  rul__val_set_nth_subvalue(func, 1,
			    rul__val_create_rul_constant (function_name));
  rul__val_set_nth_subvalue(func, 2,
			    rul__val_create_long_animal (rb_strategy));
  rul__val_set_nth_subvalue(func, 3,
			    rul__val_create_null (CMP__E_INT_MOLECULE));
  if (rules_seen) {
    sprintf(buffer, "%s%ld_cs_subset", GEN_NAME_PREFIX, block_index);
    rul__val_set_nth_subvalue(func, 4,
			      rul__val_create_blk_or_loc_addr (CMP__E_INT_CS,
							      buffer));
    sprintf(buffer, "%s%ld_bc", GEN_NAME_PREFIX, block_index);
    rul__val_set_nth_subvalue(func, 5,
		      rul__val_create_blk_or_loc_addr (CMP__E_INT_BETA_SET,
						       buffer));
    sprintf(buffer, "%s%ld_matches", GEN_NAME_PREFIX, block_index);
    rul__val_set_nth_subvalue(func, 6,
		      rul__val_create_blk_or_loc (CMP__E_INT_MATCHES_FUNC,
						  buffer));
  }
  else {
      rul__val_set_nth_subvalue (func, 4,
				 rul__val_create_null (CMP__E_INT_MOLECULE));
      rul__val_set_nth_subvalue (func, 5,
				 rul__val_create_null (CMP__E_INT_MOLECULE));
      rul__val_set_nth_subvalue (func, 6,
				 rul__val_create_null (CMP__E_INT_MOLECULE));
  }

  /* default = SPACE = FALSE = Don't retain memories */
  rul__val_set_nth_subvalue(func, 7,
			    rul__val_create_long_animal (
		 (rul__cli_optimize_option () == CLI_OPTIMIZE_REINVOCATION)));

  /*
   * Pass number of declaring blocks, array of names, and array of
   * propagation functions.
   */
  rul__val_set_nth_subvalue(func, 8,
		    rul__val_create_long_animal (num_of_decling_blocks));
  sprintf(buffer, "%s%ld_db_names", GEN_NAME_PREFIX, block_index);
  rul__val_set_nth_subvalue(func, 9,
			    rul__val_create_blk_or_loc (
					 CMP__E_INT_MOLECULE, buffer));
  sprintf(buffer, "%s%ld_db_funcs", GEN_NAME_PREFIX, block_index);
  rul__val_set_nth_subvalue(func, 10,
			    rul__val_create_blk_or_loc (
					 CMP__E_INT_PROP_FUNC, buffer));

  /*
   * Pass number of catchers, array of names, and array of functions.
   */
  sprintf(buffer, "%s%ld_catch_count", GEN_NAME_PREFIX, block_index);
  rul__val_set_nth_subvalue(func, 11,
			    rul__val_create_blk_or_loc (CMP__E_INT_LONG,
							buffer));
  rul__val_set_nth_subvalue(func, 12,
			    rul__val_create_null (CMP__E_INT_MOLECULE));
  rul__val_set_nth_subvalue(func, 13,
			    rul__val_create_null (CMP__E_INT_CATCHER_FUNC));

  /*
   * Pass number of constructs, array of names, and array of types.
   * A runtime variable should be added to contain number of constructs,
   * since it isn't known until after the entry block is generated.  nyi
   */
  sprintf(buffer, "%s%ld_cons_count", GEN_NAME_PREFIX, block_index);
  rul__val_set_nth_subvalue(func, 14,
			      rul__val_create_blk_or_loc(CMP__E_INT_LONG,
							 buffer));
  sprintf(buffer, "%s%ld_cons", GEN_NAME_PREFIX, block_index);

  rul__val_set_nth_subvalue (func, 15,
			     rul__val_create_blk_or_loc (CMP__E_INT_MOLECULE,
							 buffer));
  sprintf(buffer, "%s%ld_cons_type", GEN_NAME_PREFIX, block_index);
  rul__val_set_nth_subvalue (func, 16,
			     rul__val_create_blk_or_loc (CMP__E_INT_MOLECULE,
							 buffer));
  rul__emit_value_statement (func);
  rul__val_free_value (func);

  rul__emit_entry_done ();
}
