/* cmp_gen_class.c - generate code for class declarations */
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
**	Code is generated to call routines which declare object classes.
**
**  MODIFIED BY:
**	DEC	Digital Equipment Corporation
**	CPQ	Compaq Computer Corporation
**
**  MODIFICATION HISTORY:
**
**	19-Feb-1993	DEC	Initial version
**	15-Apr-1993	DEC	changes for USES
**	16-Feb-1998	DEC	class type changed to rclass
**	01-Dec-1999	CPQ	Release with GPL
*/

#include <common.h>
#include <cmp_comm.h>
#include <ast.h>
#include <gen.h>		/* Declaration of this routine */
#include <val.h>
#include <emit.h>
#include <decl.h>
#include <decl_c_p.h>		/* Private class declarations */
#include <mol.h>
#include <conrg.h>
#include <cons.h>
#include <hash.h>

/* Forward declarations */

static void dump_class (Class cl);
static void add_method_data (Method meth);
static Decl_Block SP_method_block;
static Boolean    SB_system_methods = FALSE;

/*****************************************************************************/
void rul__gen_dump_classes (Decl_Block decl_block)
{
  Class	      rclass;
  Value	      func, sub_func;		/* For building function values */
  Molecule    class_name;
  void       *context = NULL;
  
  func = rul__val_create_function_str ("rul__decl_create_decl_block", 1);
  rul__val_set_nth_subvalue (func, 1,
			    rul__val_create_rul_constant (
				  rul__decl_get_decl_name (decl_block)));
  rul__emit_value_statement(func);
  rul__val_free_value(func);
  rul__emit_blank_line ();

  /* Iterate over each class in this declaring block, dumping each class. */
  for (rclass = rul__decl_get_block_classes(decl_block);
       rclass != NULL;
       rclass = rclass->next) {

    /* If this class isn't the root, dump it. */
    if (!rul__decl_is_root_class (rclass))
      dump_class (rclass);
  }

  func = rul__val_create_function_str ("rul__decl_finish_decl_block", 0);
  rul__emit_value_statement (func);
  rul__val_free_value (func);
  rul__emit_blank_line ();

  /* Initialize the class array */
  rul__emit_comment ("Class array initialization");
  while (rul__class_get_next (&context, &rclass)) {
    class_name = rul__decl_get_class_name (rclass);
#ifndef NDEBUG
    sub_func = rul__val_create_function_str ("rul__decl_get_block", 1);
#else
    sub_func = rul__val_create_function_str ("DGB", 1);
#endif
    rul__val_set_nth_subvalue(sub_func, 1,
			      rul__val_create_rul_constant (
				      rul__decl_get_class_block_name (rclass)));
#ifndef NDEBUG
    func = rul__val_create_function_str ("rul__decl_get_class", 2);
#else
    func = rul__val_create_function_str ("DGC", 2);
#endif
    rul__val_set_nth_subvalue(func, 1, sub_func);
    rul__val_set_nth_subvalue(func, 2,
			      rul__val_create_rul_constant (class_name));
    func = rul__val_create_assignment (rul__val_create_class_constant (rclass),
				       func);
    rul__emit_value_statement (func);
    rul__val_free_value(func);
  }
  rul__emit_blank_line();

  /* add the method info */
  SP_method_block = decl_block;
  SB_system_methods = FALSE;

  rul__emit_comment ("Method initialization");

  rul__hash_for_each_entry (rul__decl_get_block_method_ht (decl_block),
			    (void (*) (void *)) add_method_data);

}

/*****************************************************************************/
static void dump_class (Class cl)
{
  Value	        func, func2, func3;	/* For building function values */
  long	        i;			/* Loop index */
  Attribute	attr;
  Mol_Symbol    attr_id;
  Mol_Symbol	attr_instance_of;

  /*
   * Need to generate:
   *	Ex: rul__decl_create_class (eb1_molecules[EB1_MOL_0__COUNT_TO],
   *				    eb1_molecules[EB1_MOL_1__ITERATOR]);
   */
  func = rul__val_create_function_str ("rul__decl_create_class", 2);
  rul__val_set_nth_subvalue (func, 1,
			    rul__val_create_rul_constant (
					 rul__decl_get_class_block_name (cl)));
  rul__val_set_nth_subvalue (func, 2,
			    rul__val_create_rul_constant (
					  rul__decl_get_class_name (cl)));
  rul__emit_value_statement (func);
  rul__val_free_value (func);

  /*
   * Ex: rul__decl_set_cur_class_parent (eb1_molecules[EB1_MOL13__ROOT]);
   */
  func = rul__val_create_function_str ("rul__decl_set_cur_class_parent", 1);
  if (rul__decl_get_class_name (cl->parent) != rul__mol_symbol_root ())
    rul__val_set_nth_subvalue (func, 1,
			      rul__val_create_rul_constant (
				     rul__decl_get_class_name (cl->parent)));
  else
    rul__val_set_nth_subvalue(func, 1,
		      rul__val_create_function_str ("rul__mol_symbol_root",
						    0));

  rul__emit_value_statement(func);
  rul__val_free_value(func);

  /*
   * Iterate over attributes.
   *
   * check for $id and skip so its not defined again 
   * It should have been defined by the set_cur_class_parent
   */
  attr_id = rul__mol_symbol_id ();
  attr_instance_of = rul__mol_symbol_instance_of ();
  for (attr = cl->local_attributes; attr != NULL; attr = attr->next) {
    /*
     * Ex: rul__decl_add_attr_cur_class(eb1_molecules[EB1_MOL_2__COUNT]);
     */
    if ((attr->name != attr_id) && (attr->name != attr_instance_of)) {
      func = rul__val_create_function_str ("rul__decl_add_attr_cur_class", 1);
      rul__val_set_nth_subvalue (func, 1,
				 rul__val_create_rul_constant (attr->name));
      rul__emit_value_statement (func);
      rul__val_free_value (func);

      if (attr->domain != dom_any) {
	func2 = rul__val_create_function_str ("rul__decl_int_to_domain", 1);
	rul__val_set_nth_subvalue (func2, 1, 
				   rul__val_create_long_animal (
							(long) attr->domain));
	func = rul__val_create_function_str (
				     "rul__decl_set_cur_attr_domain", 1);
	rul__val_set_nth_subvalue (func, 1, func2);
	rul__emit_value_statement (func);
	rul__val_free_value (func);
      }

      if (attr->class_restrict != NULL) {
#ifndef NDEBUG
	func3 = rul__val_create_function_str ("rul__decl_get_block", 1);
#else
	func3 = rul__val_create_function_str ("DGB", 1);
#endif
	rul__val_set_nth_subvalue (func3, 1,
				   rul__val_create_rul_constant (
					 rul__decl_get_class_block_name (
						 attr->class_restrict)));
#ifndef NDEBUG
	func2 = rul__val_create_function_str ("rul__decl_get_class", 2);
#else
	func2 = rul__val_create_function_str ("DGC", 2);
#endif
	rul__val_set_nth_subvalue (func2, 1, func3);
	rul__val_set_nth_subvalue (func2, 2,
				   rul__val_create_rul_constant (
					 rul__decl_get_class_name (
						   attr->class_restrict)));
	func = rul__val_create_function_str (
				     "rul__decl_set_cur_attr_class", 1);
	rul__val_set_nth_subvalue (func, 1, func2);
	rul__emit_value_statement (func);
	rul__val_free_value (func);	    
      }

      if (attr->shape == shape_compound) {
	func = rul__val_create_function_str (
				    "rul__decl_set_cur_attr_compound", 0);
	rul__emit_value_statement (func);
	rul__val_free_value (func);

	if (attr->fill_value != rul__mol_symbol_nil ()) {
	  /* Set attr fill */
	  func = rul__val_create_function_str (
				       "rul__decl_set_cur_attr_fill", 1);
	  rul__val_set_nth_subvalue (func, 1,
			    rul__val_create_rul_constant (attr->fill_value));
	  rul__emit_value_statement (func);
	  rul__val_free_value (func);
	}
      }

      /* Set attr default.  (Could do only if non-NIL) */
      func = rul__val_create_function_str ("rul__decl_set_cur_attr_default",1);
      rul__val_set_nth_subvalue (func, 1,
				 rul__val_create_rul_constant (
						     attr->default_value));
      rul__emit_value_statement (func);
      rul__val_free_value (func);
    }
  }

  /*
   * Ex: rul__decl_set_cur_class_mask_len(9)
   */
  
  func = rul__val_create_function_str ("rul__decl_set_cur_class_masklen", 1);
  rul__val_set_nth_subvalue (func, 1,
			     rul__val_create_long_animal(
					 rul__decl_get_inh_mask_length (cl)));
  rul__emit_value_statement (func);
  rul__val_free_value (func);

  /* Set pattern_parts */
  for (i = 0;
       i < rul__decl_get_inh_pattern_size (rul__decl_get_class_block_name(cl));
       i++) {

    func = rul__val_create_function_str ("rul__decl_set_cur_class_patpart", 2);
    rul__val_set_nth_subvalue (func, 1,
			       rul__val_create_long_animal (i));
    rul__val_set_nth_subvalue (func, 2,
			       rul__val_create_long_animal (
				    rul__decl_get_inh_pattern_part (cl, i)));
    rul__emit_value_statement(func);
    rul__val_free_value(func);
  }

  func = rul__val_create_function_str ("rul__decl_finish_cur_class", 0);
  rul__emit_value_statement (func);
  rul__val_free_value (func);

  rul__emit_blank_line ();
}


static void add_method_data (Method meth)
{
  Value       func, sub_func;
  long        i, m;
  Mol_Symbol  class_name, par_name = rul__mol_symbol_nil ();
  char        buf[(RUL_C_MAX_SYMBOL_SIZE*2)+1];
  char        buf1[RUL_C_MAX_SYMBOL_SIZE+1];
  Boolean     is_generic, is_method;

  /*
   * generate all the methods
   * each generic contains the info for parameter checking
   */

  while (meth) {
    is_generic = rul__decl_is_generic_method (meth);
    is_method = TRUE;
    if (is_generic)
      is_method = FALSE;

    if ((rul__decl_is_system_method (meth->name) != 0) &&
	(!SB_system_methods)) {

      /* define the generic system methods */

      SB_system_methods = TRUE;
      rul__emit_comment("System methods are used, define system generics");
      func = rul__val_create_function_str ("rul__decl_define_sys_methods", 1);
#ifndef NDEBUG
      sub_func = rul__val_create_function_str ("rul__decl_get_block", 1);
#else
      sub_func = rul__val_create_function_str ("DGB", 1);
#endif
      rul__val_set_nth_subvalue(sub_func, 1,
			rul__val_create_rul_constant (
			      rul__decl_get_decl_name (SP_method_block)));
      rul__val_set_nth_subvalue (func, 1, sub_func);
      rul__emit_value_statement (func);
      rul__val_free_value (func);
      rul__emit_blank_line ();
    }

    if (is_method || rul__decl_is_system_method (meth->name) == 0) {
      
      /* define a method unless a system-generic method */
      rul__mol_use_printform (meth->name, buf1, RUL_C_MAX_SYMBOL_SIZE+1);
      if (is_generic)
	sprintf (buf, "Define generic method %s", buf1);
      else
	sprintf (buf, "Define method %s", buf1);
      rul__emit_comment (buf);
      func = rul__val_create_function_str ("rul__decl_create_method", 4);
#ifndef NDEBUG
      sub_func = rul__val_create_function_str ("rul__decl_get_block", 1);
#else
      sub_func = rul__val_create_function_str ("DGB", 1);
#endif
      rul__val_set_nth_subvalue(sub_func, 1,
			rul__val_create_rul_constant (
			      rul__decl_get_decl_name (SP_method_block)));
      rul__val_set_nth_subvalue (func, 1, sub_func);
      rul__val_set_nth_subvalue (func, 2,
				 rul__val_create_rul_constant (meth->name));
      rul__val_set_nth_subvalue (func, 3,
			 rul__val_create_long_animal ((long) is_method));
      rul__val_set_nth_subvalue (func, 4,
			 rul__val_create_long_animal (meth->param_cnt));
      rul__emit_value_statement (func);
      rul__val_free_value (func);
      
      for (i = 0; i < meth->param_cnt; i++) {
	func = rul__val_create_function_str ("rul__decl_set_method_param", 4);
	rul__val_set_nth_subvalue (func, 1,
				   rul__val_create_long_animal (i));
	rul__val_set_nth_subvalue (func, 2,
		   rul__val_create_long_animal (meth->params[i].shape));
	rul__val_set_nth_subvalue (func, 3,
		 rul__val_create_long_animal (meth->params[i].domain));
	if (meth->params[i].rclass)
	  rul__val_set_nth_subvalue (func, 4,
				     rul__val_create_class_constant (
						     meth->params[i].rclass));
	else
	  rul__val_set_nth_subvalue (func, 4,
			     rul__val_create_null (CMP__E_INT_MOLECULE));
      
	rul__emit_value_statement (func);
	rul__val_free_value (func);

	if (! is_generic) {

	  /* add real method info */

	  func = rul__val_create_function_str ("rul__decl_set_method_func", 1);
	  sprintf (buf, "%s%ld_meth_%ld", GEN_NAME_PREFIX,
		   rul__conrg_get_cur_block_index (), meth->method_id);
	  rul__val_set_nth_subvalue (func, 1,
				     rul__val_create_blk_or_loc (
						 CMP__E_INT_MOLECULE, buf));
	  rul__emit_value_statement (func);
	  rul__val_free_value (func);

	  func = rul__val_create_function_str ("rul__decl_finish_method", 0);
	  rul__emit_value_statement (func);
	  rul__val_free_value (func);

	  break;
	}
      }
      rul__emit_blank_line ();
    }
    meth = meth->next;
  }
}
