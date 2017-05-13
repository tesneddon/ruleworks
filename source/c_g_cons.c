/****************************************************************************
**                                                                         **
**                C M P _ G E N _ C O N S T R U C T . C                    **
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
****************************************************************************/

#include <common.h>
#include <cmp_comm.h>
#include <ast.h>
#include <gen.h>
#include <mol.h>
#include <cons.h>
#include <conrg.h>
#include <net.h>
#include <decl.h>
#include <emit.h>
#include <val.h>
#include <time.h>
#include <cli.h>
#include <sem.h>
#include <ver_msg.h>

static void gen_end_decl_block (Mol_Symbol blk_sym);
static void gen_dump_all_constants (void);
static void gen_dump_all_constructs (void);
static void gen_decling_block_arrays (void);
static void gen_decling_block_func (void);




/*******************************
**                            **
**  RUL__GEN_CONSTRUCTS_CODE  **
**                            **
*******************************/

void rul__gen_constructs_code (Ast_Node ast)
{
	Ast_Node_Type typ;

	typ = rul__ast_get_type (ast);
	switch (typ) {

	case AST__E_OBJ_CLASS :
	  /*  Generation for these is delayed til end-block  */
	  break;

	case AST__E_GENERIC_METHOD :
	  rul__gen_generic_method (ast);
	  break;

	case AST__E_METHOD :
	  rul__gen_method (ast);
	  break;

	case AST__E_RULE :
	  rul__gen_rule_rhs (ast);
	  break;

	case AST__E_EXT_RT_DECL :
	  rul__decl_finish_ext_rt ();
	  break;

	case AST__E_ENTRY_BLOCK :
	  rul__gen_match_top_decls ();
	  gen_decling_block_arrays ();
	  break;

	case AST__E_RULE_BLOCK :
	  rul__gen_match_top_decls ();
	  gen_decling_block_arrays ();
	/*  rul__gen_rule_block (ast); */
	  break;

	case AST__E_DECL_BLOCK :
	  gen_decling_block_arrays ();
	  break;

	case AST__E_END_BLOCK :
	  rul__gen_end_block ();
	  break;
			
	case AST__E_RULE_GROUP :
	  /* no-op */
	  break;
			
	case AST__E_END_GROUP :
	  /* no-op */
	  break;
			
	case AST__E_ON_ENTRY :
	case AST__E_ON_EVERY :
	case AST__E_ON_EMPTY :
	case AST__E_ON_EXIT :
	  rul__gen_on_clause (ast);
	  break;

	case AST__E_CATCH :
	  rul__gen_catcher_func (ast);
	  break;
	  
	  default :
	    /*?*/ fprintf (stderr,
			   "\n  Un-generatable construct type:   %s\n",
			   rul___ast_type_to_string (typ));
	}
}





/*************************
**                      **
**  RUL__GEN_END_BLOCK  **
**                      **
*************************/

void	rul__gen_end_block (void)
{
	Mol_Symbol name;
	Construct_Type type;

	name = rul__conrg_get_cur_block_name ();
	type = rul__conrg_get_block_type(name);
	if (type == RUL__C_DECL_BLOCK) {
	  gen_end_decl_block (name);
	}
	else if (type == RUL__C_RULE_BLOCK) {
	  if (rul__sem_rules_seen ())
	    rul__gen_match_network ();
	  /*?*  gen ruling_block_func  *?*/
	  rul__net_clear_network ();
	}
	else {
	  assert(type == RUL__C_ENTRY_BLOCK);
	  if (rul__sem_rules_seen ())
	    rul__gen_match_network ();
	  /*?*  gen ruling_block_func  *?*/
	  /*?*  gen entry_block func  *?*/
	  rul__net_clear_network ();
	}
	gen_decling_block_func ();
}

static void gen_end_decl_block (Mol_Symbol blk_sym)
{
    Decl_Block	block = rul__decl_get_block (blk_sym);
    time_t	time_val;
    char        file_spec[(RUL_C_MAX_READFORM_SIZE + 1) * 2];
    char        blk_nam[RUL_C_MAX_READFORM_SIZE];
    char        blk_id[32]; /* unique symbol for this block compilation */

    /*
     ** get the name
     */
    rul__mol_use_readform (blk_sym, blk_nam, sizeof(blk_nam));

    /*
     ** build a unique symbol for this block compilation
     */
    strcpy (blk_id, GEN_NAME_PREFIX);
    strcat (blk_id, rul__decl_get_decl_short_name (block));
    time_val = time (NULL);
    strftime (blk_id + strlen(blk_id), 18, "_%b_%d_%y_%H%M%S",
	      localtime (&time_val));

    rul__emit_comment ("The following symbol uniquely identifes the");
    rul__emit_comment ("compilation of this declaration block");
    rul__emit_global_external (ext_type_long, blk_id, EXT__C_NOT_ARRAY, FALSE);

    /*
     ** build the file spec using the given directory, filename and suffix
     */
    if (rul__cli_usedir_flag ())
      strcpy(file_spec, rul__cli_usedir_option ());
    else
      file_spec[0] = '\0';
    strcat(file_spec, rul__decl_get_decl_short_name (block));
    strcat(file_spec, RUL__C_DECL_FILE_EXT);

    rul__gen_decl_block_usefile (block, file_spec, blk_id);
}


/*****************************************************************************/
static void gen_decling_block_arrays (void)
{
  /*
   * Declare arrays owned by this declaring block.  These arrays are defined
   * later, after we know what sizes they need to be.  There is an array
   * of all constants referenced in the block, an array of classes referenced
   * in the block, and an array of constructs and there types, used by the
   * debugger.
   */
  char buffer[RUL_C_MAX_SYMBOL_SIZE+1];
  long block_num;

  block_num = rul__conrg_get_cur_block_index ();

  /*
   * We need to declare arrays whose sizes we won't know until later.
   * We declare them as pointers here, and initialize the pointers to
   * point to the actual arrays later.  They can then be accessed as
   * arrays.
   */

  /* Emit the constant table declaration (definition comes later). */
  rul__emit_page_break ();
  sprintf (buffer, "%s%ld_m", GEN_NAME_PREFIX, block_num);
  rul__emit_comment ("The collected molecule constant table");
  rul__emit_stack_internal_ptr (CMP__E_INT_MOLECULE, buffer, 0);
  rul__emit_blank_line ();

  /* emit the class array declaration  */
  sprintf (buffer, "%s%ld_cl", GEN_NAME_PREFIX, block_num);
  rul__emit_comment ("The collected class table");
  rul__emit_stack_internal_ptr (CMP__E_INT_CLASS, buffer, 0);
  rul__emit_blank_line ();
    
  if (rul__conrg_get_block_type (
		 rul__conrg_get_cur_block_name ()) != RUL__C_DECL_BLOCK) {

    /* emit the catcher array declaration  */
    rul__emit_comment ("The collected catcher table");
    sprintf (buffer, "%s%ld_catch_count", GEN_NAME_PREFIX, block_num);
    rul__emit_stack_internal (CMP__E_INT_LONG, buffer, EXT__C_NOT_ARRAY);
    sprintf (buffer, "%s%ld_catch_names", GEN_NAME_PREFIX, block_num);
    rul__emit_stack_internal_ptr(CMP__E_INT_MOLECULE,buffer,EXT__C_NOT_ARRAY);
    sprintf (buffer, "%s%ld_catch_funcs", GEN_NAME_PREFIX, block_num);
    rul__emit_stack_internal_ptr(CMP__E_INT_CATCHER_FUNC,
				 buffer,EXT__C_NOT_ARRAY);
    rul__emit_blank_line ();
    
    /* emit the construct count declaration  */
    rul__emit_comment ("The collected construct table");
    sprintf (buffer, "%s%ld_cons_count", GEN_NAME_PREFIX, block_num);
    rul__emit_stack_internal (CMP__E_INT_LONG, buffer, 0);
    
    /* emit the construct array declaration  */
    sprintf (buffer, "%s%ld_cons", GEN_NAME_PREFIX, block_num);
    rul__emit_stack_internal_ptr (CMP__E_INT_MOLECULE, buffer, 0);
    
    /* emit the construct type array declaration  */
    sprintf (buffer, "%s%ld_cons_type", GEN_NAME_PREFIX, block_num);
    rul__emit_stack_internal_ptr (CMP__E_INT_CONSTRUCT_TYPE, buffer, 0);

    rul__emit_blank_line ();
  }
}    


/*****************************
**                          **
**  GEN_DECLING_BLOCK_FUNC  **
**                          **
*****************************/

static void gen_decling_block_func (void)
{
  char buffer[RUL_C_MAX_SYMBOL_SIZE+20];
  char buffer1[RUL_C_MAX_SYMBOL_SIZE+1];
  char buffer2[RUL_C_MAX_SYMBOL_SIZE+1];
  long block_num;
  Value	init_val;		/* For init variable values */
  Value	misc_val;		/* For misc values */
  Value	func;			/* For building function values */
  Value assign_val;		/* For building assignments */
  Mol_Symbol blk_name;
  Name_Scope scope;
  int cons_count;
  int class_count = 0;
  int conrg_count = 0;
  int catcher_count = rul__conrg_get_catcher_count();

  blk_name = rul__conrg_get_cur_block_name ();
  block_num = rul__conrg_get_cur_block_index ();

  rul__emit_page_break ();

  /* emit 'static long db%ld_init;' (declaring block initialization flag) */
  sprintf (buffer, "%s_%ld_init", GEN_NAME_PREFIX, block_num);
  rul__emit_stack_external (ext_type_long, buffer, EXT__C_NOT_ARRAY);
  rul__emit_blank_line ();

  /*
   * Declare the function which initializes the constant table for
   * this declaring block.
   */
  sprintf (buffer1, "%s%ld_const", GEN_NAME_PREFIX, block_num);
  rul__emit_entry_point_str (buffer1, EMIT__E_SCOPE_STATIC,
			     EMIT__E_NO_RETURN, ext_type_invalid,
			     EXT__C_NOT_ARRAY, ext_mech_invalid);
  rul__emit_entry_args_done (EMIT__E_ENTRY_DECLARATION);

  rul__emit_blank_line ();

  /*
   * Generate the function header for the declaring block function.
   */
  /* declaration block init functions have the same name as the block */
  if (rul__conrg_get_block_type (blk_name) == RUL__C_DECL_BLOCK) {
    scope = EMIT__E_SCOPE_GLOBAL;
    rul__mol_use_printform (blk_name, buffer, sizeof (buffer));
  }
  else {
    scope = EMIT__E_SCOPE_STATIC;
    sprintf(buffer, "%s____db%ld", GEN_NAME_PREFIX, block_num);
  }
  rul__emit_entry_point_str (buffer, scope, EMIT__E_NO_RETURN,
			     ext_type_invalid, EXT__C_NOT_ARRAY,
			     ext_mech_invalid);
  rul__emit_entry_args_done (EMIT__E_ENTRY_DEFINITION);
  rul__emit_blank_line ();

  /* emit the static init flag conditional */
  sprintf (buffer, "%s_%ld_init", GEN_NAME_PREFIX, block_num);
  init_val = rul__val_create_blk_or_loc (CMP__E_INT_MOLECULE, buffer);
  misc_val = rul__val_create_long_animal (FALSE);
  rul__emit_begin_conditional (EMIT__E_REL_EQ, init_val, misc_val);
  rul__val_free_value (misc_val);
  
  /* emit the static init flag assignment */
  assign_val = rul__val_create_assignment (init_val,
					   rul__val_create_long_animal(TRUE));
  rul__emit_value_statement (assign_val);
  rul__val_free_value (assign_val);
  rul__emit_blank_line ();

  /*
   * Call rul__ios_init(), it  calls rul__mol_init.
   */
  func = rul__val_create_function_str ("rul__ios_init", 0);
  rul__emit_value_statement (func);
  rul__val_free_value (func);

  /*
   * Call the function which initializes the constant table for
   * this declaring block.
   */
  func = rul__val_create_function_str (buffer1, 0);
  rul__emit_value_statement (func);
  rul__val_free_value (func);

  rul__emit_blank_line ();

  /*  Emit all the object-class declaration initializations  */
  rul__gen_dump_classes (rul__decl_get_block (blk_name));
  
  if (rul__conrg_get_block_type (
		 rul__conrg_get_cur_block_name ()) != RUL__C_DECL_BLOCK) {

    /*  Emit all the constructs initializations  */
    /* if debug=no then skip definitions */
    if (rul__cli_debug_option () != CLI_DEBUG_NO) {
      gen_dump_all_constructs();
      rul__emit_blank_line ();
    }

    rul__gen_catcher_tables(block_num);
  }

  rul__emit_end_conditional ();
  rul__emit_entry_done ();

  rul__emit_page_break ();

  /* Emit the constant table definition. */
  cons_count = rul__cons_get_count();
  if (cons_count > 0) {
    sprintf (buffer, "%s%ld_ma", GEN_NAME_PREFIX, block_num);
    rul__emit_comment ("The collected molecule constant table");
    rul__emit_stack_internal (CMP__E_INT_MOLECULE, buffer, cons_count);
    rul__emit_blank_line ();
  }

  /* emit the class array definition */
  class_count = rul__class_get_count ();
  if (class_count > 0) {
    sprintf (buffer, "%s%ld_cla", GEN_NAME_PREFIX, block_num);
    rul__emit_comment ("The collected class table");
    rul__emit_stack_internal (CMP__E_INT_CLASS, buffer, class_count);
    rul__emit_blank_line ();
  }

  if (rul__conrg_get_block_type (rul__conrg_get_cur_block_name ()) !=
      RUL__C_DECL_BLOCK) {

    /* emit the construct array definition */

    /* if debug=no then skip definitions */
    if (rul__cli_debug_option () == CLI_DEBUG_NO)
      conrg_count = 0;
    else
      conrg_count = rul__conrg_get_count ();

    if (conrg_count > 0) {
      sprintf (buffer, "%s%ld_cons_array", GEN_NAME_PREFIX, block_num);
      rul__emit_comment ("The collected construct table");
      rul__emit_stack_internal (CMP__E_INT_MOLECULE, buffer, conrg_count);
    
      /* emit the construct type array definition */
      sprintf (buffer, "%s%ld_cons_type_array", GEN_NAME_PREFIX, block_num);
      rul__emit_stack_internal (CMP__E_INT_CONSTRUCT_TYPE, buffer,conrg_count);
    }
    rul__emit_blank_line ();
  }

  /* emit the catcher tables */

  if (catcher_count > 0) {
    sprintf (buffer, "bl%ld_catcher_names_array", block_num);
    rul__emit_comment ("The collected catcher tables");
    rul__emit_stack_internal (CMP__E_INT_MOLECULE, buffer, catcher_count);
    sprintf (buffer, "bl%ld_catcher_funcs_array", block_num);
    rul__emit_stack_internal (CMP__E_INT_CATCHER_FUNC, buffer, catcher_count);
    rul__emit_blank_line ();
  }

  /*
   * Emit a function which initializes this declaring blocks's constants.
   * This function is called at the beginning of the declaring block
   * function.  This function is generated last so that any constants
   * created while generating other code will be included.
   */
  rul__emit_entry_point_str(buffer1, EMIT__E_SCOPE_STATIC, EMIT__E_NO_RETURN,
			    ext_type_invalid, EXT__C_NOT_ARRAY,
			    ext_mech_invalid);
  rul__emit_entry_args_done (EMIT__E_ENTRY_DEFINITION);
  rul__emit_blank_line ();

  /*
   * Initialize the pointers to the molecule, construct name, and construct
   * type arrays.
   */
  if (cons_count > 0) {
    sprintf (buffer, "%s%ld_m", GEN_NAME_PREFIX, block_num);
    sprintf (buffer2, "%s%ld_ma", GEN_NAME_PREFIX, block_num);
    assign_val =
      rul__val_create_assignment
	(rul__val_create_blk_or_loc(CMP__E_INT_MOLECULE,buffer),
	 rul__val_create_blk_or_loc(CMP__E_INT_MOLECULE,buffer2));
    rul__emit_value_statement(assign_val);
    rul__val_free_value(assign_val);
  }

  if (class_count > 0) {
    sprintf (buffer, "%s%ld_cl", GEN_NAME_PREFIX, block_num);
    sprintf (buffer2, "%s%ld_cla", GEN_NAME_PREFIX, block_num);
    assign_val = rul__val_create_assignment
      (rul__val_create_blk_or_loc (CMP__E_INT_CLASS, buffer),
       rul__val_create_blk_or_loc (CMP__E_INT_CLASS, buffer2));
    rul__emit_value_statement(assign_val);
    rul__val_free_value(assign_val);
  }

  if (rul__conrg_get_block_type (
		 rul__conrg_get_cur_block_name ()) != RUL__C_DECL_BLOCK) {

    sprintf (buffer, "%s%ld_catch_count", GEN_NAME_PREFIX, block_num);
    assign_val = rul__val_create_assignment (
		    rul__val_create_blk_or_loc (CMP__E_INT_MOLECULE, buffer),
		    rul__val_create_long_animal (catcher_count));
    rul__emit_value_statement (assign_val);
    rul__val_free_value (assign_val);

    if (catcher_count > 0) {
      sprintf (buffer, "%s%ld_catch_names", GEN_NAME_PREFIX, block_num);
      sprintf (buffer2, "bl%ld_catcher_names_array", block_num);
      assign_val =
	rul__val_create_assignment
	  (rul__val_create_blk_or_loc (CMP__E_INT_MOLECULE, buffer),
	   rul__val_create_blk_or_loc (CMP__E_INT_MOLECULE, buffer2));
      rul__emit_value_statement (assign_val);
      rul__val_free_value (assign_val);

      sprintf (buffer, "%s%ld_catch_funcs", GEN_NAME_PREFIX, block_num);
      sprintf (buffer2, "bl%ld_catcher_funcs_array", block_num);
      assign_val =
	rul__val_create_assignment
	  (rul__val_create_blk_or_loc (CMP__E_INT_CATCHER_FUNC, buffer),
	   rul__val_create_blk_or_loc (CMP__E_INT_CATCHER_FUNC, buffer2));
      rul__emit_value_statement (assign_val);
      rul__val_free_value (assign_val);
    }

    sprintf (buffer, "%s%ld_cons_count", GEN_NAME_PREFIX, block_num);
    assign_val = rul__val_create_assignment
      (rul__val_create_blk_or_loc (CMP__E_INT_MOLECULE, buffer),
       rul__val_create_long_animal (conrg_count));
    rul__emit_value_statement (assign_val);
    rul__val_free_value (assign_val);

    if (conrg_count > 0) {
      sprintf (buffer, "%s%ld_cons", GEN_NAME_PREFIX, block_num);
      sprintf (buffer2, "%s%ld_cons_array", GEN_NAME_PREFIX, block_num);
      assign_val =
	rul__val_create_assignment
	  (rul__val_create_blk_or_loc (CMP__E_INT_MOLECULE, buffer),
	   rul__val_create_blk_or_loc (CMP__E_INT_MOLECULE, buffer2));
      rul__emit_value_statement (assign_val);
      rul__val_free_value (assign_val);

      sprintf (buffer, "%s%ld_cons_type", GEN_NAME_PREFIX, block_num);
      sprintf (buffer2, "%s%ld_cons_type_array", GEN_NAME_PREFIX, block_num);
      assign_val =
	rul__val_create_assignment
	  (rul__val_create_blk_or_loc (CMP__E_INT_CONSTRUCT_TYPE, buffer),
	   rul__val_create_blk_or_loc (CMP__E_INT_CONSTRUCT_TYPE, buffer2));
      rul__emit_value_statement (assign_val);
      rul__val_free_value (assign_val);
    }
  }

  /*
   * Emit all the molecule constant initializations.  
   */
  gen_dump_all_constants();

  rul__emit_entry_done ();
}




/*****************************
**                          **
**  GEN_DUMP_ALL_CONSTANTS  **
**                          **
*****************************/

static void gen_dump_all_constants (void)
{
  Value     func_val = NULL, assign_val, mol_val;
  char      printform_str[RUL_C_MAX_SYMBOL_SIZE+1];
  Molecule  mol;
  long      idx, comp_len, i;
  void     *context = NULL;

  /*
   **  Generate the initialization for a symbolic constant
   **
   **  Example:
   **	SA_bl1_molecules[17] =  rul__mol_make_symbol ("LIMIT");
   **   rul__mol_mark_perm (SA_bl1_molecules[17]);
   */

  for (idx = rul__cons_get_next(&context, &mol);
       idx != 0; 
       idx = rul__cons_get_next(&context, &mol)) {

    assert (rul__mol_is_valid (mol));

    /*	Create the assignment target  */
    mol_val = rul__val_create_rul_constant (mol);
  
    /*	Create the appropriate function value  */
    if (rul__mol_is_symbol(mol)) {

#ifndef NDEBUG
      func_val = rul__val_create_function_str ("rul__mol_make_symbol", 1);
#else
      func_val = rul__val_create_function_str ("MMS", 1);
#endif
      rul__mol_use_printform (mol, printform_str, RUL_C_MAX_SYMBOL_SIZE + 1);
      rul__val_set_nth_subvalue (func_val, 1,
	       rul__val_create_asciz_animal (printform_str));

    } else if (rul__mol_is_int_atom(mol)) {

#ifndef NDEBUG
      func_val = rul__val_create_function_str ("rul__mol_make_int_atom", 1);
#else
      func_val = rul__val_create_function_str ("MMI", 1);
#endif
      rul__val_set_nth_subvalue (func_val, 1,
				 rul__val_create_long_animal(
				  rul__mol_int_atom_value (mol)));

    } else if (rul__mol_is_dbl_atom(mol)) {

#ifndef NDEBUG
      func_val = rul__val_create_function_str ("rul__mol_make_dbl_atom", 1);
#else
      func_val = rul__val_create_function_str ("MMD", 1);
#endif
      rul__val_set_nth_subvalue (func_val, 1,
	       rul__val_create_double_animal (
			rul__mol_dbl_atom_value (mol)));

    } else if (rul__mol_is_opaque (mol)) {

      assert (mol == rul__mol_opaque_null());
#ifndef NDEBUG
      func_val = rul__val_create_function_str ("rul__mol_opaque_null", 0);
#else
      func_val = rul__val_create_function_str ("MMON", 0);
#endif

    } else if (rul__mol_is_instance_id (mol)) {

      assert (mol == rul__mol_instance_id_zero());
#ifndef NDEBUG
      func_val = rul__val_create_function_str ("rul__mol_instance_id_zero", 0);
#else
      func_val = rul__val_create_function_str ("MMIZ", 0);
#endif

    } else if (rul__mol_is_compound(mol)) {

      comp_len = rul__mol_get_poly_count(mol);
#ifndef NDEBUG
      func_val = rul__val_create_function_str ("rul__mol_make_comp",
					       comp_len + 1);
#else
      func_val = rul__val_create_function_str ("MMC", comp_len + 1);
#endif
      rul__val_set_nth_subvalue (func_val, 1,
		 rul__val_create_long_animal (comp_len));

      for (i = 1; i <= comp_len; i++) {
	rul__val_set_nth_subvalue (func_val, i+1,
				   rul__val_create_rul_constant(
					      rul__mol_get_comp_nth(mol, i)));
      }
    }
    else /* a bad molecule... ??? */
      assert (rul__mol_is_symbol (mol));

    assert (func_val != NULL);

    /*	Create the assignment  */
    assign_val = rul__val_create_assignment (mol_val, func_val);
    
    /*	Emit the assignment */
    rul__emit_value_statement (assign_val);

    /* mark it as perminent */
#ifndef NDEBUG
    func_val = rul__val_create_function_str ("rul__mol_mark_perm", 1);
#else
    func_val = rul__val_create_function_str ("MMP", 1);
#endif
    rul__val_set_nth_subvalue (func_val, 1, rul__val_copy_value(mol_val));

    /*	Emit the perm func */
    rul__emit_value_statement (func_val);
    
    /* free the debris  */
    rul__val_free_value (assign_val);
    rul__val_free_value (func_val);
  }

}

/*****************************
**                          **
**  GEN_DUMP_ALL_CONSTRUCTS **
**                          **
*****************************/

static void gen_dump_all_constructs (void)
{
  Value           func_val, assign_val, mol_val, type_val;
  char            printform_str[RUL_C_MAX_SYMBOL_SIZE+1];
  char           *cons_type_str;
  char            buf[RUL_C_MAX_SYMBOL_SIZE+1];
  Molecule        mol;
  void           *context = NULL;
  Construct_Type  ctype;
  long            blk_num, idx = -1;

  /*
   **  Generate the initialization for a symbolic constructs
   **
   **  Example:
   **	SA_bl1_constructs[10] =  SA_bl1_molecules[0];
   **	SA_bl1_construct_type[10] = RUL__C_RULE;
   */

  blk_num = rul__conrg_get_cur_block_index ();

  while (rul__conrg_get_next_construct(&context, &mol, &ctype)) {

    assert (rul__mol_is_valid (mol));

    /* idx is used because of the special case (unnamed) constructs */
    /* the val_create_construct can only be used for named constructs */
    idx += 1;

    /*	Create the assignment target  */
    sprintf (buf, "%s%ld_cons", GEN_NAME_PREFIX, blk_num);
    mol_val = rul__val_create_blk_or_loc_vec (CMP__E_INT_MOLECULE, buf, idx);

    /*	Create the assignment  */
    assign_val = rul__val_create_assignment(mol_val,
					    rul__val_create_rul_constant(mol));
    
    /*	Emit the code  */
    rul__emit_value_statement (assign_val);

    /*	Free the debris  */
    rul__val_free_value (assign_val);

    switch (ctype) {
    case RUL__C_NOISE:
      cons_type_str = "RUL__C_NOISE";
      break;
    case RUL__C_RULE:
      cons_type_str = "RUL__C_RULE";
      break;
    case RUL__C_CATCH:
      cons_type_str = "RUL__C_CATCH";
      break;
    case RUL__C_COMMENT:
      cons_type_str = "RUL__C_COMMENT";
      break;
    case RUL__C_OBJ_CLASS:
      cons_type_str = "RUL__C_OBJ_CLASS";
      break;
    case RUL__C_EXT_ROUTINE:
      cons_type_str = "RUL__C_EXT_ROUTINE";
      break;
    case RUL__C_DECL_BLOCK:
      cons_type_str = "RUL__C_DECL_BLOCK";
      break;
    case RUL__C_ENTRY_BLOCK:
      cons_type_str = "RUL__C_ENTRY_BLOCK";
      break;
    case RUL__C_RULE_BLOCK:
      cons_type_str = "RUL__C_RULE_BLOCK";
      break;
    case RUL__C_END_BLOCK:
      cons_type_str = "RUL__C_END_BLOCK";
      break;
    case RUL__C_RULE_GROUP:
      cons_type_str = "RUL__C_RULE_GROUP";
      break;
    case RUL__C_END_GROUP:
      cons_type_str = "RUL__C_END_GROUP";
      break;
    case RUL__C_ON_ENTRY:
      cons_type_str = "RUL__C_ON_ENTRY";
      break;
    case RUL__C_ON_EVERY:
      cons_type_str = "RUL__C_ON_EVERY";
      break;
    case RUL__C_ON_EMPTY:
      cons_type_str = "RUL__C_ON_EMPTY";
      break;
    case RUL__C_ON_EXIT:
      cons_type_str = "RUL__C_ON_EXIT";
      break;
    case RUL__C_METHOD:
      cons_type_str = "RUL__C_METHOD";
      break;
    default:
      cons_type_str = "RUL__C_NOISE";
      break;
    }


    /*	Create the assignment target  */
    sprintf (buf, "%s%ld_cons_type", GEN_NAME_PREFIX, blk_num);
    mol_val = rul__val_create_blk_or_loc_vec (CMP__E_INT_MOLECULE, buf, idx);

    type_val = rul__val_create_blk_or_loc (ext_type_asciz, cons_type_str);
    
    /*	Create the assignment  */
    assign_val = rul__val_create_assignment (mol_val, type_val);
    
    /*	Emit the code, and free the debris  */
    rul__emit_value_statement (assign_val);
    rul__val_free_value (assign_val);
  }
}

