/* cmp_gen_entry.c - generate code for entry block */
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
**	The function corresponding to the entry block is generated.
**
**  MODIFIED BY:
**	DEC	Digital Equipment Corporation
*	CPQ	Compaq Computer Corporation
**
**  MODIFICATION HISTORY:
**
**	 5-Mar-1993	DEC	Initial version
**
**	 3-Jun-1994	DEC	Move MAIN -> |main| hack down to just codegen.
**
**	 1-Aug-1994	DEC	Don't generate uncompilable code for r/w array
**					length parameter - don't call
**					rul__sem_reset_var_binding_site()
**
**	 2-Aug-1994	DEC	Don't use rul__sem_curr_rhs_var_binding()
**					for array length, since it may have been
**					rebound and freed in ON- clause.
**
**	 3-Aug-1994	DEC	Correctly handle read-write array parameters
**					with constant length.
**
**	23-Sep-1994	DEC	Avoid illegal characters and collisions in
**					generated argument names.  Fix returning array
**					with variable length.
**
**	01-Dec-1999	CPQ	Release with GPL
*/

/*
 * Restrictions:
 *
 * Right now this module assumes no parameters, no return value, only
 * one declaring block and one ruling block (i.e., no USES or ACTIVATES),
 * and no catchers.
 */


#include <common.h>
#include <cmp_comm.h>
#include <ast.h>
#include <gen.h>		/* Declaration of this routine */
#include <decl.h>
#include <conrg.h>
#include <val.h>
#include <emit.h>
#include <mol.h>
#include <net.h>
#include <list.h>
#include <sem.h>
#include <cli.h>
#include <ver_msg.h>


void
rul__gen_entry_block (void)
{
  Mol_Symbol  par_sym;    	/*symbol representation of internal parameter*/
  Mol_Symbol  ext_sym;  	/*symbol representation of external parameter*/
  Mol_Symbol  block_name;	/* name of various declaring blocks	     */
  Mol_Symbol  function_name;	/* name of this entry block		     */
  long	      block_index;	/*index of this block; used to generate names*/
  Value	      func;		/* For building function values		     */
  Value	      assign_val;	/* For building assignment values	     */
  char	      buffer[RUL_C_MAX_SYMBOL_SIZE+1];
  char	      buffer1[RUL_C_MAX_SYMBOL_SIZE+1],*ext_nam=buffer1;
  long	      num_of_local_vars; /* # of vars in accepts and ON-constructs */
  long	      num_of_decling_blocks;/*# of decl blocks USESed plus this one*/
  long	      num_of_ruling_blocks; /*# of rule blocks ACTIVATEd + this one*/
  long	      i, a_len;	/* used to index through entry-block params  */
  Name_Scope  scope;		/* scope of implicit decl block for this EB  */
  List 	      block_list;	/* list of decling blocks assoc. w/ this EB  */
  Value	      on_clause_func_value[DECL_ON_EXIT+1];
  Value	      eb_scoped_vars_addr;
  Value       status_var;	/* the local variable "status" */
  Value	      status_const;	/* the constant integer RUL__C_RETURN */
  Ext_Rt_Decl ext_rt;		/* external routine declaration for this EB  */
  Boolean     ext_rt_returns;	/* does this routine return a value?	     */
  long	      num_args;	/* number of arguments to this entry-block   */
  Value	      cvt_return_value = NULL, rw_param_value = NULL;
				/*assignment stmt to convert return value   */
  Strategy    eb_strategy = rul__decl_get_eb_strategy();
  Value       default_ret_val, a_arg_val, ext_var_val, int_val;
  Value       a_val, tmp_val;
  Ast_Node    a_arg_ast, node;
  Mol_Symbol  a_arg;
  Boolean     rules_seen = rul__sem_rules_seen ();
  Ext_Type    ext_typ;

  /* This declaration should be in a header file; but, it's painful to do. */
  extern char *rul___gen_on_clause_func_name (Ast_Node_Type ast_type);

  /*
   * Determine the entry block's name (e.g. "count_to") and the index of
   * functions created for this block.
   */
  function_name = rul__conrg_get_cur_block_name ();
  block_index = rul__conrg_get_cur_block_index ();
  ext_rt = rul__decl_get_ext_rt_decl (rul__decl_get_block (function_name),
				      function_name);
  ext_rt_returns = rul__decl_ext_rt_ret_val (ext_rt);

  /*
   * Determine information about the entry block which will be used in
   * code generation.
   */
  rul__sem_use_entry_vars (TRUE);
  num_of_local_vars = rul__sem_number_of_rhs_vars(); /* vars in EB+ON cons */
  num_of_decling_blocks = 1;	/* Number of decl blocks USEd + implicit one */
  num_of_ruling_blocks = 1;	/* Should be num of ACTIVATES + implicit one */

  /* Count: num_of_decling_blocks */
  for (block_list = rul__decl_get_visible_decl_blks ();
       !rul__list_is_empty (block_list);
       block_list = rul__list_rest (block_list)) {

    block_name = rul__list_first (block_list);

    if (block_name != function_name) {
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
   * Generate a forward declaration for this entry block's
   * implicit decl block
   */
  if (rul__conrg_get_block_type (function_name) == RUL__C_DECL_BLOCK) {
    scope = EMIT__E_SCOPE_GLOBAL;
    rul__mol_use_printform (function_name, buffer, sizeof (buffer));
  }
  else {
    scope = EMIT__E_SCOPE_STATIC;
    sprintf(buffer, "%s____db%ld", GEN_NAME_PREFIX, block_index);
  }
  rul__emit_entry_point_str (buffer, scope, EMIT__E_NO_RETURN,
			     ext_type_invalid, EXT__C_NOT_ARRAY,
			     ext_mech_invalid);
  rul__emit_entry_args_done (EMIT__E_ENTRY_DECLARATION);
  
  /* generate a declaration for each ruling-block function */
  for (block_list = rul__decl_get_rule_block_list ();
       !rul__list_is_empty (block_list);
       block_list = rul__list_rest (block_list)) {

    block_name = rul__list_first (block_list);
    rul__emit_entry_point (block_name, EMIT__E_SCOPE_GLOBAL,
			   EMIT__E_NO_RETURN, ext_type_invalid,
			   EXT__C_NOT_ARRAY, ext_mech_invalid);
    rul__emit_entry_args_done (EMIT__E_ENTRY_DECLARATION);
    num_of_ruling_blocks++;
  }
      
  /*
   * Generate forward declarations for propogation functions.
   */
  if (rules_seen) {
    rul__gen_prop_func_forw_decls (num_of_decling_blocks);
    rul__emit_blank_line ();
  }

  /*
   * Generate the function header for this entry block.
   */
  rul__emit_page_break();
  rul__emit_entry_point (
      rul__decl_ext_rt_alias (ext_rt),
      EMIT__E_SCOPE_GLOBAL,
      ext_rt_returns,
      rul__decl_ext_rt_ret_type (ext_rt),
      rul__decl_ext_rt_ret_a_len (ext_rt),
      rul__decl_ext_rt_mech (ext_rt));
  
  /* Generate the parameters here for this entry-block. */
  num_args = rul__decl_ext_rt_num_args (ext_rt);
  for (i = 1; i <= num_args; i++) {
    par_sym = rul__decl_ext_rt_param_name (ext_rt, i);
    rul__sem_get_ext_param_name(ext_nam, par_sym, ext_rt);
    rul__emit_entry_arg_external (rul__decl_ext_rt_param_type (ext_rt, i),
				  ext_nam,
				  rul__decl_ext_rt_param_a_len (ext_rt, i),
				  rul__decl_ext_rt_param_mech (ext_rt, i));
  }
  rul__emit_entry_args_done (EMIT__E_ENTRY_DEFINITION);

  /*
   * Define local variables used by generated entry block.
   */
  rul__emit_stack_internal (CMP__E_INT_LONG, "eb_after_count", 0);
  rul__emit_stack_internal (CMP__E_INT_MOLECULE, "eb_catcher_name", 0);
  rul__emit_stack_internal (CMP__E_INT_MOLECULE, "eb_catcher_rb_name", 0);
  rul__emit_stack_internal (CMP__E_INT_REFRACTION_SET, "refracts", 0);

  /*
   * Define array of variables used in ON- constructs.
   */
  if (num_of_local_vars > 0) {
    rul__emit_stack_internal (CMP__E_INT_MOLECULE, GEN__C_RHS_VARIABLE_STR,
			      num_of_local_vars);
    /* note that we don't use rul__val_create_blk_or_loc_addr to avoid
       the & notation on an array name */
    eb_scoped_vars_addr = rul__val_create_blk_or_loc (CMP__E_INT_MOLECULE,
						      GEN__C_RHS_VARIABLE_STR);
  }
  else {
    eb_scoped_vars_addr = rul__val_create_null (CMP__E_INT_MOLECULE);
  }
    
  /*
   * Define array containing names of visible ruling blocks.
   */
  rul__emit_stack_internal (CMP__E_INT_MOLECULE, "eb_rblock_names",
			    num_of_ruling_blocks);
  
  /* if this entry block any read-write parameters or returns a value; 
   * determine any variables needed to convert the internal value to 
   * the external value and declare them.
   */
  for (i = 1; i <= num_args; i++) {
      
    par_sym = rul__decl_ext_rt_param_name (ext_rt, i);
    if (rul__decl_ext_rt_param_mech (ext_rt, i) == ext_mech_ref_rw &&
	par_sym != NULL) {

      a_val = NULL;
      ext_typ = rul__decl_ext_rt_param_type (ext_rt, i);
      a_len = rul__decl_ext_rt_param_a_len (ext_rt, i);
      a_arg = rul__decl_ext_rt_param_a_arg (ext_rt, i);
      rul__mol_incr_uses (par_sym);
      rul__sem_get_ext_param_name(ext_nam, par_sym, ext_rt);

      node = rul__ast_mol_node (AST__E_VARIABLE, par_sym);
      int_val = rul__sem_check_rhs_var (node, SEM__C_VALUE_ACCESS);

      ext_sym = rul__mol_make_symbol (ext_nam);
      ext_var_val = rul__val_create_named_ext_var (ext_sym,
						   ext_typ, a_len,
						   ext_mech_ref_rw,
						   SEM__C_NO_MEMORY_ALLOCATED);

      if (a_len == EXT__C_IMPLICIT_ARRAY && a_arg != NULL) {
	char ext_len_arg_name[RUL_C_MAX_SYMBOL_SIZE+1];
				/* e.g. RUL_arg3_LENGTH */
	Mol_Symbol ext_len_arg_sym; /* Symbol for external length, e.g., LEN */
	Ext_Mech ext_len_arg_mech; /* Mechanism of array length */

	rul__sem_get_ext_param_name(ext_len_arg_name, a_arg, ext_rt);
	ext_len_arg_sym = rul__mol_make_symbol(ext_len_arg_name);
	ext_len_arg_mech =
	    rul__decl_ext_rt_param_mech(
		ext_rt,
		rul__decl_ext_rt_param_index(ext_rt, a_arg));
	a_val = rul__val_create_named_ext_var (ext_len_arg_sym,
					       ext_type_long,
					       EXT__C_NOT_ARRAY,
					       ext_len_arg_mech,
					       SEM__C_NO_MEMORY_ALLOCATED);
	int_val = rul__sem_convert_to_ext_value (int_val, ext_typ,
						 a_len, a_val,
						 ext_mech_ref_rw,
						 ext_var_val,
						 SEM__C_ON_RHS, node);
      }
      else if (a_len > 0) {	/* Array with constant length */
	int_val = rul__sem_convert_to_ext_value (int_val, ext_typ,
						 a_len, a_val,
						 ext_mech_ref_rw,
						 ext_var_val,
						 SEM__C_ON_RHS, node);
      }
      else if (ext_typ == ext_type_asciz || ext_typ == ext_type_ascid) {
	int_val = rul__sem_convert_to_ext_value (int_val, ext_typ,
						 a_len, NULL,
						 ext_mech_ref_rw,
						 ext_var_val,
						 SEM__C_ON_RHS, node);
      }
      else {
	int_val = rul__sem_convert_to_ext_value (int_val, ext_typ,
						 a_len, a_val,
						 ext_mech_value, NULL,
						 SEM__C_ON_RHS, node);
	int_val = rul__val_create_assignment (ext_var_val, int_val);
      }
      rul__ast_free (node);

      if (rw_param_value != NULL)
	rul__val_set_next_value (rul__val_get_last_value (rw_param_value),
				 int_val);
      else
	rw_param_value = int_val;
    }
  }

  if (ext_rt_returns == EMIT__E_HAS_RETURN) {
    a_val = NULL;
    a_len = rul__decl_ext_rt_ret_a_len (ext_rt);
    a_arg = rul__decl_ext_rt_ret_a_arg (ext_rt);
    
    if (a_len == EXT__C_IMPLICIT_ARRAY && a_arg != NULL) {
      char ext_len_arg_name[RUL_C_MAX_SYMBOL_SIZE+1];
				/* e.g. RUL_arg3_LENGTH */
      Mol_Symbol ext_len_arg_sym; /* Symbol for external length, e.g., LEN */
      long index = rul__decl_ext_rt_param_index(ext_rt, a_arg);

      rul__sem_get_ext_param_name(ext_len_arg_name, a_arg, ext_rt);
      ext_len_arg_sym = rul__mol_make_symbol(ext_len_arg_name);

      a_val =
	  rul__val_create_named_ext_var(ext_len_arg_sym,
					rul__decl_ext_rt_param_type(ext_rt,
								    index),
					EXT__C_NOT_ARRAY,
					rul__decl_ext_rt_param_mech(ext_rt,
								    index),
					SEM__C_NO_MEMORY_ALLOCATED);
    }
    cvt_return_value =
      rul__sem_convert_to_ext_value (
			     rul__val_create_blk_or_loc (CMP__E_INT_MOLECULE,
					 rul__gen_get_cur_ret_val_name ()),
				     rul__decl_ext_rt_ret_type (ext_rt),
				     a_len,  a_val,
				     rul__decl_ext_rt_mech (ext_rt), NULL,
				     SEM__C_ON_RHS, NULL);
  }

  rul__sem_rhs_action_end_vars ();
  rul__gen_rhs_tmp_decl ();

  /*
   * Initialize variables used by the entry block.
   */
  
  /*  emit the assignment of the after counter to zero  */
  assign_val = rul__val_create_assignment (rul__val_create_blk_or_loc (
				       CMP__E_INT_LONG, "eb_after_count"),
					   rul__val_create_long_animal (0));
  rul__emit_value_statement (assign_val);
  rul__val_free_value (assign_val);

  assign_val = rul__val_create_assignment (rul__val_create_blk_or_loc (
				CMP__E_INT_MOLECULE, "eb_catcher_name"),
				   rul__val_create_null (CMP__E_INT_MOLECULE));
  rul__emit_value_statement (assign_val);
  rul__val_free_value (assign_val);
    
  assign_val = rul__val_create_assignment (rul__val_create_blk_or_loc (
				 CMP__E_INT_MOLECULE, "eb_catcher_rb_name"),
				   rul__val_create_null(CMP__E_INT_MOLECULE));
  rul__emit_value_statement (assign_val);
  rul__val_free_value (assign_val);

  /*
   * Call rul__ios_init(), it calls rul__mol_init().
   */
  func = rul__val_create_function_str ("rul__ios_init", 0);
  rul__emit_value_statement (func);
  rul__val_free_value (func);

  /* 
   * Call the explicit declaring blocks used by this entry block.
   */
  for (block_list = rul__decl_get_visible_decl_blks ();
       !rul__list_is_empty (block_list);
       block_list = rul__list_rest (block_list)) {

    block_name = rul__list_first (block_list);
    if (block_name != function_name) {
      func = rul__val_create_function_str (
				   rul__mol_get_printform (block_name), 0);
      rul__emit_value_statement (func);
      rul__val_free_value(func);
    }
  }
  
  /*
   * Call the implicit declaring block associated with this entry block.
   */
  sprintf(buffer, "%s____db%ld", GEN_NAME_PREFIX, block_index);
  func = rul__val_create_function_str(buffer, 0);
  rul__emit_value_statement(func);
  rul__val_free_value(func);
  
  /*
   * Initialize return value.
   */
  if (ext_rt_returns == EMIT__E_HAS_RETURN) {

    if (rul__decl_ext_rt_ret_a_len (ext_rt) != 0) {
      default_ret_val = rul__val_create_rul_constant (
					      rul__mol_compound_empty ());
    }
    else {
      switch (rul__decl_ext_rt_ret_type(ext_rt)) {
	  
      case ext_type_long:
      case ext_type_short:
      case ext_type_byte:
      case ext_type_uns_long:
      case ext_type_uns_short:
      case ext_type_uns_byte:
	default_ret_val = rul__val_create_rul_constant (
						rul__mol_integer_zero ());
	break;

      case ext_type_float:
      case ext_type_double:
	default_ret_val = rul__val_create_rul_constant (
						rul__mol_double_zero ());
	break;

      case ext_type_object:
	default_ret_val = rul__val_create_rul_constant (
						rul__mol_instance_id_zero ());
	break;

      case ext_type_asciz:
      case ext_type_ascid:
	default_ret_val = rul__val_create_rul_constant (
						rul__mol_make_symbol (""));
	break;

      case ext_type_void_ptr:
      case ext_type_atom_ptr:
	default_ret_val = rul__val_create_rul_constant (
						rul__mol_opaque_null ());
	break;

      case ext_type_atom:
	default_ret_val = rul__val_create_rul_constant (
						rul__mol_make_symbol (""));
	break;

      default:
	default_ret_val = rul__val_create_rul_constant (
						rul__mol_make_symbol (""));
	break;
      }
    }
    /*  set default return value based on return value type  */
    assign_val = rul__val_create_assignment (
		     rul__val_create_blk_or_loc (CMP__E_INT_MOLECULE,
					 rul__gen_get_cur_ret_val_name ()),
					     default_ret_val);
    /*  emit the assignment for the default return value  */
    rul__emit_value_statement (assign_val);
    rul__val_free_value (assign_val);
  }

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
    (void) rul__net_create_decl_node (block_name);
    sprintf(buffer, "%s%ld_db_names", GEN_NAME_PREFIX, block_index);
    assign_val = rul__val_create_assignment (
			     rul__val_create_blk_or_loc_vec (
					 CMP__E_INT_MOLECULE, buffer, i),
			     rul__val_create_rul_constant (block_name));
    rul__emit_value_statement (assign_val);
    rul__val_free_value (assign_val);

    if (rules_seen) {
      sprintf(buffer, "%s%ld_pr_db%ld", GEN_NAME_PREFIX, block_index, i + 1);
      sprintf(buffer1, "%s%ld_db_funcs", GEN_NAME_PREFIX, block_index);
      assign_val = rul__val_create_assignment (
			       rul__val_create_blk_or_loc_vec (
					 CMP__E_INT_PROP_FUNC, buffer1, i),
			       rul__val_create_blk_or_loc (
					   CMP__E_INT_PROP_FUNC, buffer));
      rul__emit_value_statement(assign_val);
      rul__val_free_value(assign_val);
    }
    i = i + 1;
  }

  /*
   * Emit a call to register the ruling block associated with this entry
   * block.
   */
  func = rul__val_create_function_str ("rul__rbs_register_rblock", 16);
  rul__val_set_nth_subvalue (func, 1,
			     rul__val_create_rul_constant (function_name));
  rul__val_set_nth_subvalue (func, 2,
			     rul__val_create_long_animal (eb_strategy));
  if (ext_rt_returns == EMIT__E_HAS_RETURN)
    rul__val_set_nth_subvalue (func, 3,
			       rul__val_create_blk_or_loc_addr (
					CMP__E_INT_MOLECULE,
					rul__gen_get_cur_ret_val_name()));
  else
    rul__val_set_nth_subvalue (func, 3,
			       rul__val_create_null (CMP__E_INT_MOLECULE));
  if (rules_seen) {
    sprintf(buffer, "%s%ld_cs_subset", GEN_NAME_PREFIX, block_index);
    rul__val_set_nth_subvalue (func, 4,
			 rul__val_create_blk_or_loc_addr (CMP__E_INT_CS,
							  buffer));
    sprintf(buffer, "%s%ld_bc", GEN_NAME_PREFIX, block_index);
    rul__val_set_nth_subvalue (func, 5,
			 rul__val_create_blk_or_loc_addr (CMP__E_INT_BETA_SET,
							  buffer));
    sprintf(buffer, "%s%ld_matches", GEN_NAME_PREFIX, block_index);
    rul__val_set_nth_subvalue (func, 6,
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
			    rul__val_create_long_animal(
					  num_of_decling_blocks));
  sprintf(buffer, "%s%ld_db_names", GEN_NAME_PREFIX, block_index);
  rul__val_set_nth_subvalue(func, 9,
			    rul__val_create_blk_or_loc(
					 CMP__E_INT_MOLECULE, buffer));
  sprintf(buffer, "%s%ld_db_funcs", GEN_NAME_PREFIX, block_index);
  rul__val_set_nth_subvalue(func, 10,
			    rul__val_create_blk_or_loc(
					 CMP__E_INT_PROP_FUNC, buffer));

  /*
   * Pass number of catchers, array of names, and array of functions.
   */
  sprintf (buffer, "%s%ld_catch_count", GEN_NAME_PREFIX, block_index);
  rul__val_set_nth_subvalue (func, 11,
			     rul__val_create_blk_or_loc (CMP__E_INT_LONG,
							 buffer));
  sprintf (buffer, "%s%ld_catch_names", GEN_NAME_PREFIX, block_index);
  rul__val_set_nth_subvalue (func, 12,
			     rul__val_create_blk_or_loc (CMP__E_INT_MOLECULE,
							 buffer));
  sprintf (buffer, "%s%ld_catch_funcs", GEN_NAME_PREFIX, block_index);
  rul__val_set_nth_subvalue (func, 13,
			     rul__val_create_blk_or_loc (
					 CMP__E_INT_CATCHER_FUNC, buffer));

  /*
   * Pass number of constructs, array of names, and array of types.
   */
  sprintf (buffer, "%s%ld_cons_count", GEN_NAME_PREFIX, block_index);
  rul__val_set_nth_subvalue (func, 14,
			     rul__val_create_blk_or_loc (
						   CMP__E_INT_LONG, buffer));
  sprintf (buffer, "%s%ld_cons", GEN_NAME_PREFIX, block_index);
  rul__val_set_nth_subvalue (func, 15,
			     rul__val_create_blk_or_loc (
					   CMP__E_INT_MOLECULE, buffer));
  sprintf (buffer, "%s%ld_cons_type", GEN_NAME_PREFIX, block_index);
  rul__val_set_nth_subvalue (func, 16,
			     rul__val_create_blk_or_loc (
					   CMP__E_INT_MOLECULE, buffer));
  rul__emit_value_statement (func);
  rul__val_free_value (func);

  /*
   * Initialize array of activated ruling blocks.
   */
  assign_val = rul__val_create_assignment (
		   rul__val_create_blk_or_loc_vec (CMP__E_INT_MOLECULE,
						   "eb_rblock_names", 0),
		   rul__val_create_rul_constant(function_name));
  rul__emit_value_statement(assign_val);
  rul__val_free_value(assign_val);

  /*
   * Loop through the list of activated rule-blocks, storing their names in
   * the array of Rblock names for this entry-block and then calling them.
   */
  for (block_list = rul__decl_get_rule_block_list (), i=1;
       !rul__list_is_empty (block_list);
       block_list = rul__list_rest (block_list)) {

    block_name = rul__list_first (block_list);
    assign_val = rul__val_create_assignment (
			     rul__val_create_blk_or_loc_vec (
			     CMP__E_INT_MOLECULE, "eb_rblock_names", i++),
			     rul__val_create_rul_constant (block_name));
    rul__emit_value_statement (assign_val);
    rul__val_free_value (assign_val);

    func = rul__val_create_function (block_name, 0);
    rul__emit_value_statement (func);
    rul__val_free_value (func);
  }

  /*
   * Initialize the refraction set.
   */
  assign_val = rul__val_create_assignment (rul__val_create_blk_or_loc (
				 CMP__E_INT_REFRACTION_SET, "refracts"),
					     rul__val_create_function_str (
					   "rul__ref_make_refraction_set", 0));
  rul__emit_value_statement(assign_val);
  rul__val_free_value(assign_val);


  /*
   * initialize each of the internal rhs variables and
   * assign the rhs_var which map to the named entry-block parameters
   */
  rul__sem_set_ent_var_bindings (ext_rt, num_args);
  for (i = 0; i < num_args; i++) {
    par_sym = rul__decl_ext_rt_param_name (ext_rt, i+1);
    if (par_sym != NULL)
      rul__emit_value_statement (rul__sem_curr_rhs_var_binding (par_sym));
  }
  for (i = num_args; i < num_of_local_vars; i++) {
    func = rul__val_create_assignment (
	       rul__val_create_blk_or_loc_vec (CMP__E_INT_MOLECULE,
					       GEN__C_RHS_VARIABLE_STR, i),
	       rul__val_create_null (CMP__E_INT_MOLECULE));
    rul__emit_value_statement (func);
    rul__val_free_value (func);
  }


  /*
   * if debug=yes then ensure rul__debug_init is called
   */
  if (rul__cli_debug_option () == CLI_DEBUG_YES) {
    func = rul__val_create_function_str ("rul__debug_init", 0);
    rul__emit_value_statement (func);
    rul__val_free_value (func);
  }


  /*
   * if an on-entry clause was declared, store its address (or NULL)
   */
  if (rul__decl_eb_was_on_clause_seen (DECL_ON_ENTRY))
    /* note that we don't use rul__val_create_blk_or_loc_addr to avoid
       the & notation on an array name */
    on_clause_func_value[DECL_ON_ENTRY] =
      rul__val_create_blk_or_loc (CMP__E_INT_FUNC_PTR,
			  rul___gen_on_clause_func_name (AST__E_ON_ENTRY));
  else
    on_clause_func_value[DECL_ON_ENTRY] =
      rul__val_create_null (CMP__E_INT_FUNC_PTR);

  /* if an on-every clause was declared, store its address (or NULL) */
  if (rul__decl_eb_was_on_clause_seen(DECL_ON_EVERY))
    /* note that we don't use rul__val_create_blk_or_loc_addr to avoid
       the & notation on an array name */
    on_clause_func_value[DECL_ON_EVERY] =
      rul__val_create_blk_or_loc (CMP__E_INT_FUNC_PTR,
			  rul___gen_on_clause_func_name (AST__E_ON_EVERY));
  else
    on_clause_func_value[DECL_ON_EVERY] =
      rul__val_create_null (CMP__E_INT_FUNC_PTR);

  /* if an on-empty clause was declared, store its address (or NULL) */
  if (rul__decl_eb_was_on_clause_seen(DECL_ON_EMPTY))
    /* note that we don't use rul__val_create_blk_or_loc_addr to avoid
       the & notation on an array name */
    on_clause_func_value[DECL_ON_EMPTY] =
      rul__val_create_blk_or_loc (CMP__E_INT_FUNC_PTR,
			  rul___gen_on_clause_func_name (AST__E_ON_EMPTY));
  else
    on_clause_func_value[DECL_ON_EMPTY] =
      rul__val_create_null (CMP__E_INT_FUNC_PTR);

  /* if an on-exit clause was declared, store its address (or NULL) */
  if (rul__decl_eb_was_on_clause_seen (DECL_ON_EXIT))
    /* note that we don't use rul__val_create_blk_or_loc_addr to avoid
       the & notation on an array name */
    on_clause_func_value[DECL_ON_EXIT] =
      rul__val_create_blk_or_loc (CMP__E_INT_FUNC_PTR,
			  rul___gen_on_clause_func_name (AST__E_ON_EXIT));
  else
    on_clause_func_value[DECL_ON_EXIT] =
      rul__val_create_null (CMP__E_INT_FUNC_PTR);

  /*
   * Emit a call to the recognize-act cycle.
   */
  func = rul__val_create_function_str("rul__rac_cycle", 13);
  rul__val_set_nth_subvalue (func, 1,
			     rul__val_create_blk_or_loc (
							 CMP__E_INT_MOLECULE, "eb_rblock_names"));
  rul__val_set_nth_subvalue (func, 2,
			     rul__val_create_long_animal (
							  num_of_ruling_blocks));
  rul__val_set_nth_subvalue (func, 3,
			     rul__val_create_blk_or_loc (
							 CMP__E_INT_REFRACTION_SET, "refracts"));
  rul__val_set_nth_subvalue (func, 4, /* ON-ENTRY function */
			     on_clause_func_value[DECL_ON_ENTRY]);
  rul__val_set_nth_subvalue (func, 5, /* ON-EVERY function */
			     on_clause_func_value[DECL_ON_EVERY]);
  rul__val_set_nth_subvalue (func, 6, /* ON-EMPTY function */
			     on_clause_func_value[DECL_ON_EMPTY]);
  rul__val_set_nth_subvalue (func, 7, /* ON-EXIT function */
			     on_clause_func_value[DECL_ON_EXIT]);
  rul__val_set_nth_subvalue (func, 8, /* Array of local variables */
			     eb_scoped_vars_addr);
  rul__val_set_nth_subvalue (func, 9,
			     rul__val_create_blk_or_loc_addr (
			      CMP__E_INT_LONG, "eb_after_count"));
  rul__val_set_nth_subvalue (func, 10, /* Catcher */
			     rul__val_create_blk_or_loc_addr (
			      CMP__E_INT_MOLECULE, "eb_catcher_name"));
  rul__val_set_nth_subvalue (func, 11, /* Catcher Rule-Block Name */
			     rul__val_create_blk_or_loc_addr (
			      CMP__E_INT_MOLECULE, "eb_catcher_rb_name"));
  if (ext_rt_returns == EMIT__E_HAS_RETURN)
    rul__val_set_nth_subvalue (func, 12, /* return_value address */
			       rul__val_create_blk_or_loc_addr (
					CMP__E_INT_MOLECULE,
					rul__gen_get_cur_ret_val_name ()));
  else
    rul__val_set_nth_subvalue (func, 12,
			       rul__val_create_null (CMP__E_INT_MOLECULE));
  rul__val_set_nth_subvalue (func, 13, /* debugging allowed flag */
			     rul__val_create_long_animal (
				  (rul__cli_debug_option () != CLI_DEBUG_NO)));
  rul__emit_value_statement (func);
  rul__val_free_value (func);
  
  /*
   * Emit a call to unregister all of the ruling blocks.
   */
  for (i = 0; i < num_of_ruling_blocks; i++) {
    func = rul__val_create_function_str("rul__rbs_unregister_rblock", 1);
    rul__val_set_nth_subvalue (func, 1,
			       rul__val_create_blk_or_loc_vec (
				 CMP__E_INT_MOLECULE, "eb_rblock_names", i));
    rul__emit_value_statement (func);
    rul__val_free_value (func);
  }

  /*
   * Emit a call to clean up entry-block data structures.
   */
  func = rul__val_create_function_str ("rul__ref_free_refraction_set", 1);
  rul__val_set_nth_subvalue (func, 1,
			     rul__val_create_blk_or_loc(
				  CMP__E_INT_REFRACTION_SET, "refracts"));
  rul__emit_value_statement(func);
  rul__val_free_value(func);


  /* update any read-write entry-block parameters */
  rul__emit_blank_line ();
  rul__emit_comment ("Update the read-write parameters");
  if (rw_param_value) {
    rul__emit_value (rw_param_value);
    rul__val_free_value (rw_param_value);
  }

  /*if this entry block returns a value; convert the molecule and return it*/
  if (ext_rt_returns == EMIT__E_HAS_RETURN) {
    rul__emit_value_statement (cvt_return_value);
#ifndef NDEBUG
    func = rul__val_create_function_str ("rul__mol_decr_uses", 1);
#else
    func = rul__val_create_function_str ("MDU", 1);
#endif
    rul__val_set_nth_subvalue (func, 1,
			       rul__val_create_blk_or_loc (
					    CMP__E_INT_MOLECULE,
					    rul__gen_get_cur_ret_val_name ()));
    rul__emit_value_statement (func);
    rul__val_free_value (func);
    /* clean up all of the temporary variables */
    rul__emit_blank_line ();
    rul__gen_decr_rhs_vars ();
    rul__emit_return_value (rul__val_get_assignment_to (cvt_return_value));
    rul__val_free_value (cvt_return_value);
  }
  else {
    /* clean up all of the temporary variables */
    rul__emit_blank_line ();
    rul__gen_decr_rhs_vars ();
  }
  
  rul__emit_entry_done ();
}
