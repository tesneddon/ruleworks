/*
 * cmp_gen_decl.c - misc. declarations generation
 */
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
 * FACILITY:
 *	RULEWORKS compiler
 *
 * ABSTRACT:
 *	This module is used only by the compiler.
 *
 * MODIFIED BY:
 *	DEC	Digital Equipment Corporation
 *	CPQ	Compaq Computer corporation
 *
 * MODIFICATION HISTORY:
 *
 *	16-Nov-1990	DEC	Initial version.
 *
 *	20-Oct-1992	DEC	Finished adapting it from V4.
 *
 *  27-Oct-1992 DEC	Misc. V5 changes
 *
 *	17-Mar-1993	DEC	Add support for External Routines
 *	
 *	16-Feb-1998	DEC	class type changed to rclass 
 *
 *	01-Dec-1999	CPQ	Release with GPL
*/

#include <common.h>		/* declarations/definitions for RULEWORKS   */
#include <cmp_comm.h>
#include <hash.h>		/* used by attribute declarations           */
#include <mol.h>		/* declarations of molecule functions	    */
#include <decl.h>		/* functions to declare object classes etc. */
#include <decl_c_p.h> 		/* class and decl_block struct defns        */
#include <decl_e_p.h>		/* external routines struct defns           */
#include <indexf.h>		/* Indexfile declarations                   */
#include <msg.h>
#include <cmp_msg.h>
#include <gen.h>
#include <emit.h>
#include <list.h>
#include <conrg.h>
#include <dyar.h>
#include <cli.h>


/*  File Pointers  */

static  Dynamic_Array SA_use_file_names;

static  FILE *out_fptr = NULL;

static  long pattern_count;
static  int  stringsize;	   /* Number of chars "printed" into string */

/*  I/O data buffers  */

/* String big enough to hold largest possible line:                         */
/* ^<attr_name> COMPOUND  (DEFAULT <compound_value>) (FILL <scalar_value>)  */
static  char out_string_chars[LONGEST_LINE];
static  char *out_string = out_string_chars;

#define print_to_usefile(out_string) fputs(out_string,out_fptr)

/* Forward declarations */

static void rul__decl_export_class (Class c);
static void rul__decl_export_method (void * method);
static void print_ext_type_to_usefile (Ext_Type ext_type);
static void print_ext_mech_to_usefile (Ext_Mech ext_mech);
static void print_shape_to_usefile (Decl_Shape shape);
static void print_domain_to_usefile (Decl_Domain domain);
static void rul__decl_export_ext_rtn (Ext_Rt_Decl ext_rt);
extern Boolean rul__decl_idx_cont_other_decls (char *file_spec,
					       Mol_Symbol blk_name_sym,
					       char *blk_name);
static void gen_declare_ext_rtn_alias (Ext_Alias_Decl alias);
static void gen_declare_ext (Ext_Rt_Decl ext_rt, long arg_cnt, Boolean extras);
static void gen_export_method (Method meth);
static void gen_open_use_file (char *file_spec);


/*

 FUNCTIONAL DESCRIPTION:

 Puts the OBJECT-CLASS declarations into the USE file.
 It dumps them with the following format:

|---- (USE-VERSION V5.0-0 )
|----
|---- (DECLARATION-BLOCK <blk_nam> )
||---   (PATTERN-COUNT <inh-pattern-size>)
||---   (COMPILATION-ID <block_id> )
||---   (OBJECT-CLASS <classname>
|||--     (INHERITS-FROM <parentname> )
|||--     (IMPORT-DEFS <inh-mask-len> <inh-pattern-part>... )
|||--     (<attr> ...)
|||--     (<attr> ...)
||---   )
||---   (GENERIC-METHOD <methodname>
|||--     (ACCEPTS <par> WHEN INSTANCE-ID OF <classname>
|||--              <par> ...)
|||--     (RETURNS <par> ...)
||---   )
||---   (EXTERNAL-ROUTINE... 
||---   )
|---- (END-BLOCK <blk_nam> )
|----
|---- (END-USE)
*/

void
rul__gen_decl_block_usefile (Decl_Block block, char *file_spec,
			       char *block_id)
{
  Mol_Symbol	blk_nam_sym;
  char		blk_nam[RUL_C_MAX_READFORM_SIZE];
  Class		rclass;
  Ext_Rt_Decl	ext_rtn;

  blk_nam_sym = rul__decl_get_decl_name(block);
  
  rul__mol_use_readform (blk_nam_sym, blk_nam, sizeof(blk_nam));

    /*
     * copy other declaration block data with identical use file name
     *  (if any) to this new file 
     */
    if (rul__decl_idx_cont_other_decls (file_spec, blk_nam_sym, blk_nam))
	{
	rul__msg_print (CMP_INXDUPBLK, file_spec);
	return;
	}

  gen_open_use_file (file_spec);

  if (out_fptr) {
    /*
     * Store the USE version number 
     */
    print_to_usefile (USE_VERSION);
    print_to_usefile (USE_SPACE);
    print_to_usefile (VERSION_NUMBER);
    print_to_usefile (CONSTRUCT_END);
    /*
     ** store the decl block name
     */
    print_to_usefile (DECLARATION_STR);
    print_to_usefile (USE_SPACE);
    print_to_usefile (blk_nam);
    print_to_usefile (USE_NEWLINE);
    /*
     ** Print number of longwords in inheritance patterns.
     */
    print_to_usefile (USE_TAB_1);
    print_to_usefile (PATT_SIZ);
    print_to_usefile (USE_SPACE);
    pattern_count = rul__decl_get_inh_pattern_size(
			rul__decl_get_decl_name(block));
    sprintf(out_string, "%ld", pattern_count);
    print_to_usefile (out_string);
    print_to_usefile (CONSTRUCT_END);
    /*
     * Print unique compilation identifier
     */
    print_to_usefile (USE_TAB_1);
    print_to_usefile (COMPILATION_ID);
    print_to_usefile (USE_SPACE);
    print_to_usefile ("|");
    print_to_usefile (block_id);
    print_to_usefile ("|");
    print_to_usefile (CONSTRUCT_END);
    /*
     * Close declaration-block params
     */
    print_to_usefile (CONSTRUCT_END);

    /*
     * Print class declaration information
     */
    for (rclass = rul__decl_get_block_classes(block)->next;
		 /* skip the first class - $ROOT */
	 rclass != NULL;
	 rclass = rclass->next) {
      rul__decl_export_class (rclass);
    }
    
    /* Export generic method defs */
    rul__hash_for_each_entry (rul__decl_get_block_method_ht (block),
			      (void (*) (void *)) gen_export_method);


    /* Export external routine declaration parameters */

    for (ext_rtn = rul__decl_get_block_routines(block);
	 ext_rtn != NULL;
	 ext_rtn = rul__decl_ext_rt_next_routine(ext_rtn)) {
      rul__decl_export_ext_rtn (ext_rtn);
    }
    
    print_to_usefile (END_BLOCK);
    print_to_usefile (USE_SPACE);
    print_to_usefile (blk_nam);
    print_to_usefile (CONSTRUCT_END);
    print_to_usefile (END_USE);
    print_to_usefile (CONSTRUCT_END);
    fclose(out_fptr);
    return;
  }
  rul__msg_print (CMP_INXOPENERR, file_spec);
}




static void
rul__decl_export_class (Class c)
{
  int         i, comp_len;
  Attribute   attr;		/* To iterate over attributes */
  Molecule    def_val;
  Boolean     need_space;

  /*
   ** Format the line into the variable out_string.
   **  (OBJECT-CLASS <class-name>
   **    (INHERITS-FROM <parentname>)
   **    (IMPORT-DEFS <mask-length> <pattern-part>...)
   **    (<attr> <offset> <shape> <domain> [(OF <class>)]
   **           [<len>] [(DEFAULT <val>)] [(FILL <val>)])
   **      ...
   **  )
   */

  print_to_usefile (USE_TAB_1);
  print_to_usefile (OBJECT_CLASS);
  print_to_usefile (USE_SPACE);
  rul__mol_use_readform (c->name, out_string, sizeof(out_string_chars));
  print_to_usefile (out_string);
  print_to_usefile (USE_NEWLINE);

  print_to_usefile (USE_TAB_2);
  print_to_usefile (INHERIT_FROM);
  print_to_usefile (USE_SPACE);
  rul__mol_use_readform (c->parent->name, out_string,sizeof(out_string_chars));
  print_to_usefile (out_string);
  print_to_usefile (CONSTRUCT_END);
  
  print_to_usefile (USE_TAB_2);
  print_to_usefile (IMPORT_DEFS);
  sprintf(out_string, " %ld", rul__decl_get_inh_mask_length (c));
  stringsize = strlen (out_string);
  /* Append pattern parts to out_string. */
  for (i = 0; i < pattern_count; i++) {
    sprintf (out_string+stringsize," %ld",
	     rul__decl_get_inh_pattern_part(c, i));
    stringsize += strlen (out_string + stringsize);
  }
  print_to_usefile (out_string);
  print_to_usefile (CONSTRUCT_END);
  
  /*
   ** Print line for each attribute.
   */
  for (attr = c->local_attributes; attr != NULL; attr = attr->next) {

    need_space = FALSE;

    if (attr->name != rul__mol_symbol_instance_of () &&
	attr->name != rul__mol_symbol_id ()) {

      /* Store attribute name */
      print_to_usefile (USE_ATTR_TAB);
      print_to_usefile (CLAUSE_BEGIN);
      rul__mol_use_readform (attr->name, out_string, sizeof(out_string_chars));
      print_to_usefile (out_string);

      /* Store attribute offset */
      sprintf(out_string, " %ld ", rul__decl_get_attr_offset (c, attr->name));
      print_to_usefile (out_string);

      print_shape_to_usefile (attr->shape);
      print_domain_to_usefile (attr->domain);

      /* set domain default domain */
      if (attr->domain == dom_any)
	def_val = rul__mol_symbol_nil ();
      else if (attr->domain == dom_symbol)
	def_val = rul__mol_symbol_nil ();
      else if (attr->domain == dom_instance_id)
	def_val = rul__mol_instance_id_zero ();
      else if (attr->domain == dom_opaque)
	def_val = rul__mol_opaque_null ();
      else if (attr->domain == dom_number)
	def_val = rul__mol_integer_zero ();
      else if (attr->domain == dom_int_atom)
	def_val = rul__mol_integer_zero ();
      else if (attr->domain == dom_dbl_atom)
	def_val = rul__mol_double_zero ();
      
      if (attr->domain == dom_instance_id && attr->class_restrict) {
	print_to_usefile (INSTANCE_OF);
	rul__mol_use_readform (rul__decl_get_class_name (attr->class_restrict),
			       out_string, sizeof(out_string_chars));
	print_to_usefile (out_string);
	print_to_usefile (CLAUSE_END);
	need_space = TRUE;
      }

      if (attr->shape == shape_compound) {

	/* Store default value length and the default value if not nil */
	if (attr->default_value != rul__mol_compound_empty ()) {
	  
	  if (need_space)
	    print_to_usefile (USE_SPACE);
	  print_to_usefile (DEFAULT);
	  print_to_usefile (OPEN_COMPOUND);
	  comp_len = rul__mol_get_poly_count (attr->default_value);
	  for (i = 1; i <= comp_len; i++) {
	    print_to_usefile (USE_SPACE);
	    rul__mol_use_readform (
			   rul__mol_get_comp_nth (attr->default_value, i),
				   out_string, sizeof(out_string_chars));
	    print_to_usefile (out_string);
	  }
	  print_to_usefile (CLAUSE_END); /* compound */
	  print_to_usefile (CLAUSE_END); /* default  */
	  need_space = TRUE;
	}

	if (attr->fill_value != def_val) {
	  if (need_space)
	    print_to_usefile (USE_SPACE);
	  /* store compound fill value if not nil */
	  print_to_usefile (FILL);
	  rul__mol_use_readform (attr->fill_value, out_string,
				 sizeof(out_string_chars));
	  print_to_usefile (out_string);
	  print_to_usefile (CLAUSE_END);
	  need_space = TRUE;
	}
      }
      
      /*
      else if (attr->shape == shape_table) {
	if (attr->default_value != def_val) {
	  if (need_space)
	    print_to_usefile (USE_SPACE);
	  print_to_usefile (DEFAULT);
	  print_to_usefile (OPEN_TABLE);
	  table_len = rul__mol_get_poly_count (attr->default_value);
	  for (i = 1; i <= table_len; i++) {
	    print_to_usefile (USE_SPACE);
	    print_to_usefile (out_string);
	  }
	  print_to_usefile (CLAUSE_END);
	  print_to_usefile (CLAUSE_END);
	  need_space = TRUE;
        }
      }
      */

      else { /* attr->shape == shape_atom */

	/* Store attribute default value */
	if (attr->default_value != def_val) {
	  if (need_space)
	    print_to_usefile (USE_SPACE);
	  print_to_usefile (DEFAULT); 
	  rul__mol_use_readform (attr->default_value, out_string,
				 sizeof(out_string_chars));
	  print_to_usefile (out_string);
	  print_to_usefile (CLAUSE_END);
	}
      }
      print_to_usefile (CLAUSE_END); /* close attr */
      print_to_usefile (USE_NEWLINE);
    }
  }
  print_to_usefile (USE_TAB_1);
  print_to_usefile (CONSTRUCT_END);
}



static void gen_export_method (Method meth)
{
  long i;

  /*   (GENERIC-METHOD <methodname>
   **    (ACCEPTS shape domain class
   **             ...)
   **    (RETURNS ...)
   **  )
   */

  while (meth) {
    if (rul__decl_is_generic_method (meth) &&
	rul__decl_is_system_method (meth->name) == 0) {
      print_to_usefile (USE_TAB_1);
      print_to_usefile (GENERIC_METHOD);
      print_to_usefile (USE_SPACE);
      rul__mol_use_readform (meth->name, out_string, sizeof(out_string_chars));
      print_to_usefile (out_string);

      print_to_usefile (USE_SPACE);
      sprintf (out_string, "%ld", meth->param_cnt);
      print_to_usefile (out_string);

      for (i = 0; i < meth->param_cnt; i++) {

	print_to_usefile (USE_NEWLINE);

	if (i == 0) {
	  print_to_usefile (USE_TAB_2);
	  print_to_usefile (ACCEPTS);
	  print_to_usefile (USE_SPACE);
	}
	else
	  print_to_usefile (EXT_RT_TAB);

	print_shape_to_usefile (meth->params[i].shape);
	print_domain_to_usefile (meth->params[i].domain);
	if (meth->params[i].rclass) {
	  rul__mol_use_readform (meth->params[i].rclass->name,
				 out_string, sizeof(out_string_chars));
	  print_to_usefile (out_string);
	}
	else
	  print_to_usefile (ROOT);
	
      }
      print_to_usefile (CLAUSE_END);
      print_to_usefile (USE_NEWLINE);
      i = meth->param_cnt;
      print_to_usefile (USE_TAB_2);
      print_to_usefile (RETURNS);
      print_to_usefile (USE_SPACE);
      print_shape_to_usefile (meth->params[i].shape);
      print_domain_to_usefile (meth->params[i].domain);
      if (meth->params[i].rclass) {
	rul__mol_use_readform (meth->params[i].rclass->name,
			       out_string, sizeof(out_string_chars));
	print_to_usefile (out_string);
      }
      else
	print_to_usefile (ROOT);
      
      print_to_usefile (CLAUSE_END);
      print_to_usefile (USE_NEWLINE);
      print_to_usefile (USE_TAB_1);
      print_to_usefile (CONSTRUCT_END);
    }
    meth = meth->next;
  }
}


static void print_ext_type_to_usefile (Ext_Type ext_type)
{
    char *type_str;

    switch (ext_type)
	{
	case ext_type_long:
	    type_str = "LONG";
	    break;
	case ext_type_short:
	    type_str = "SHORT";
	    break;
	case ext_type_byte:
	    type_str = "BYTE";
	    break;
	case ext_type_uns_long:
	    type_str = "UNSIGNED-LONG";
	    break;
	case ext_type_uns_short:
	    type_str = "UNSIGNED-SHORT";
	    break;
	case ext_type_uns_byte:
	    type_str = "UNSIGNED-BYTE";
	    break;
	case ext_type_float:
	    type_str = "SINGLE-FLOAT";
	    break;
	case ext_type_double:
	    type_str = "DOUBLE-FLOAT";
	    break;
	case ext_type_asciz:
	    type_str = "ASCIZ";
	    break;
	case ext_type_ascid:
	    type_str = "ASCID";
	    break;
	case ext_type_void_ptr:
	    type_str = "OPAQUE";
	    break;
	case ext_type_atom:
	    type_str = "ATOM";
	    break;
	default:
	    assert (FALSE /* Bogus Type */);
	    type_str = "INVALID-TYPE";
	    break;
	}
    print_to_usefile (type_str);
}

static void
print_ext_mech_to_usefile (Ext_Mech ext_mech)
{
    char *mech_str;

    switch (ext_mech)
	{
	case ext_mech_value:
	    mech_str = "BY-VALUE";
	    break;
	case ext_mech_ref_rw:
	    mech_str = "BY-REFERENCE-READ-WRITE";
	    break;
	case ext_mech_ref_ro:
	    mech_str = "BY-REFERENCE-READ-ONLY";
	    break;
	default:
	    assert (FALSE /* Bogus Mech */);
	    mech_str = "INVALID-MECHANISM";
	    break;
	}
    print_to_usefile (mech_str);
}

static void
rul__decl_export_ext_rtn (Ext_Rt_Decl ext_rt)
{
  long        i, p;
  Cardinality a_len;
  Mol_Symbol  a_arg;

  /**
   ** An external routine indexfile record will be in the format:
   **
   **  (EXTERNAL-ROUTINE <routine_name>
   **    (ALIAS <other_name> )
   **    (ACCEPTS <type> <array_length> <passing_mech>
   **             <type> <array_length> <passing_mech>
   **             ...)
   **    (RETURNS <type> <array_length> <passing_mech> ) 
   **  )
   **/

  print_to_usefile (USE_TAB_1);
  print_to_usefile (EXTERNAL_ROUTINE);
  print_to_usefile (USE_SPACE);
  rul__mol_use_readform (rul__decl_ext_rt_name (ext_rt), out_string,
			 sizeof(out_string_chars));
  print_to_usefile (out_string);

  /* Write ALIAS clause */
  if (rul__decl_ext_rt_alias (ext_rt) != rul__decl_ext_rt_name (ext_rt)) {
    print_to_usefile (USE_NEWLINE);
    print_to_usefile (USE_TAB_2);
    print_to_usefile (ALIAS);
    print_to_usefile (USE_SPACE);
    rul__mol_use_readform (rul__decl_ext_rt_alias (ext_rt), out_string,
			   sizeof(out_string_chars));
    print_to_usefile (out_string);
    print_to_usefile (CLAUSE_END);
  }

  /* Write ACCEPTS clause */
  for (i = 1; i <= rul__decl_ext_rt_num_args (ext_rt); i++) {
    print_to_usefile (USE_NEWLINE);
    if (i == 1) {
      print_to_usefile (USE_TAB_2);
      print_to_usefile (ACCEPTS);
      print_to_usefile (USE_SPACE);
    }
    else
      print_to_usefile (EXT_RT_TAB);
    print_ext_type_to_usefile (rul__decl_ext_rt_param_type (ext_rt, i));
    a_len = rul__decl_ext_rt_param_a_len (ext_rt, i);
    a_arg = rul__decl_ext_rt_param_a_arg (ext_rt, i);
    if (a_len != EXT__C_IMPLICIT_ARRAY || a_arg == NULL) {
      sprintf (out_string, " %ld ", a_len);
      print_to_usefile (out_string);
    }
    else {
      p = rul__decl_ext_rt_param_index (ext_rt, a_arg);
      sprintf (out_string, "P%ld", p-1);
      print_to_usefile (USE_SPACE);
      print_to_usefile (out_string);
      print_to_usefile (USE_SPACE);
    }

    print_ext_mech_to_usefile (rul__decl_ext_rt_param_mech (ext_rt, i));

    if (i == rul__decl_ext_rt_num_args (ext_rt))
      print_to_usefile (CONSTRUCT_END);
  }

  /* Write RETURNS clause */
  if (rul__decl_ext_rt_ret_val (ext_rt)) {
    print_to_usefile (USE_TAB_2);
    print_to_usefile (RETURNS);
    print_to_usefile (USE_SPACE);
    print_ext_type_to_usefile (rul__decl_ext_rt_ret_type (ext_rt));
    a_len = rul__decl_ext_rt_ret_a_len (ext_rt);
    a_arg = rul__decl_ext_rt_ret_a_arg (ext_rt);
    if (a_len != EXT__C_IMPLICIT_ARRAY || a_arg == NULL) {
      sprintf (out_string, " %ld ", a_len);
      print_to_usefile (out_string);
    }
    else {
      p = rul__decl_ext_rt_param_index (ext_rt, a_arg);
      sprintf (out_string, "P%ld", p-1);
      print_to_usefile (USE_SPACE);
      print_to_usefile (out_string);
      print_to_usefile (USE_SPACE);
    }
    print_ext_mech_to_usefile (rul__decl_ext_rt_mech (ext_rt));
    print_to_usefile (CONSTRUCT_END);
  }
  print_to_usefile (USE_TAB_1);
  print_to_usefile (CONSTRUCT_END);
}


/*
 * declare the block specific static varaibles
 */

void rul__gen_declare_dblock_vars (void)
{
  List        block_list;
  Mol_Symbol  block_name;	/* name of various declaring blocks	     */
  long	      block_index;	/*index of this block; used to generate names*/
  long        num_of_decling_blocks;/*# of decl blocks USESed plus this one*/
  char        buffer[RUL_C_MAX_SYMBOL_SIZE+1],*int_param_name=buffer;


  rul__emit_blank_line ();
  rul__emit_comment ("Declare this blocks static dblock variables");

  block_index = rul__conrg_get_cur_block_index ();
  num_of_decling_blocks = 0;	/* Number of decl blocks USEd */

  /* Count: num_of_decling_blocks */
  for (block_list = rul__decl_get_visible_decl_blks ();
       !rul__list_is_empty (block_list);
       block_list = rul__list_rest (block_list)) {
    num_of_decling_blocks += 1;
  }
  
  /*
   * Define array containing names of visible declaring blocks,
   * a long containing the number of declaring blocks,
   * and array containing addresses of this ruling block's propagate
   * functions.
   */
  sprintf(buffer, "%s%ld_db_names", GEN_NAME_PREFIX, block_index);
  rul__emit_stack_internal(CMP__E_INT_MOLECULE, buffer,
			   num_of_decling_blocks);
  
  sprintf(buffer, "%s%ld_db_count", GEN_NAME_PREFIX, block_index);
  rul__emit_stack_internal(CMP__E_INT_LONG, buffer, 0);
  
  sprintf(buffer, "%s%ld_db_funcs", GEN_NAME_PREFIX, block_index);
  rul__emit_stack_internal(CMP__E_INT_PROP_FUNC, buffer,
			   num_of_decling_blocks);
  rul__emit_blank_line ();
}



/*
 * declare the external routine
 */

void rul__gen_declare_ext_rtns (void)
{
  List           decl_blk_list;
  Ext_Rt_Decl    ext_rt;
  Mol_Symbol     alias_name;
  Ext_Alias_Decl alias;
  Hash_Table     ext_alias_ht = rul__decl_get_alias_ht ();

  rul__emit_blank_line ();
  rul__emit_comment ("external-routine declarations");

  for (decl_blk_list = rul__decl_get_visible_decl_blks ();
       ext_alias_ht && (decl_blk_list) && !rul__list_is_empty (decl_blk_list);
       decl_blk_list = rul__list_rest (decl_blk_list)) {
    for (ext_rt = rul__decl_get_block_routines (rul__decl_get_block (
					rul__list_first (decl_blk_list)));
	 ext_rt;
	 ext_rt = rul__decl_get_next_ext_rt (ext_rt)) {

      alias_name = rul__decl_ext_rt_alias (ext_rt);
      alias = (Ext_Alias_Decl) rul__hash_get_entry (ext_alias_ht, alias_name);
      if (alias && alias->func)
	gen_declare_ext_rtn_alias (alias);
    }
  }
  
  if (ext_alias_ht)
    rul__decl_delete_alias_ht ();
}

static void gen_declare_ext_rtn_alias (Ext_Alias_Decl alias)
{
  Ext_Rt_Decl    ext_rt, next_rt;
  Ext_Alias_Decl next_alias;
  long           max_pars, min_pars, ext_pars;

  ext_rt = alias->func;
  next_alias = rul__decl_get_next_alias (alias);

  /* if only on definition, no checks needed */

  if (next_alias == NULL) {
    gen_declare_ext (ext_rt, rul__decl_ext_rt_num_args (ext_rt), FALSE);
  }
  else {
    /* multiple function definitions with same alias */
    /* match up the paramiters */
    /*??? only matches the count of parameters now...*/

    ext_pars = min_pars = max_pars = rul__decl_ext_rt_num_args (ext_rt);
    do {
      next_rt = next_alias->func;
      ext_pars = rul__decl_ext_rt_num_args (next_rt);
      max_pars = MAX (max_pars, ext_pars);
      min_pars = MIN (min_pars, ext_pars);
      next_alias = rul__decl_get_next_alias (next_alias);
    }  while (next_alias);
    
    if (max_pars != min_pars)
      gen_declare_ext (ext_rt, min_pars, TRUE);
  }
  alias->func = NULL;	/* already done is func=null */
}

static void gen_declare_ext (Ext_Rt_Decl ext_rt, long arg_cnt, Boolean extras)
{
  char           buffer[50];
  long           i;

  rul__emit_entry_point (rul__decl_ext_rt_alias (ext_rt),
			 EMIT__E_SCOPE_GLOBAL,
			 rul__decl_ext_rt_ret_val (ext_rt),
			 rul__decl_ext_rt_ret_type (ext_rt),
			 rul__decl_ext_rt_ret_a_len (ext_rt),
			 rul__decl_ext_rt_mech (ext_rt));
  /* do params... */

  for (i = 1; i <= arg_cnt; i++) {
    sprintf (buffer, "p%ld", i);
    rul__emit_entry_arg_external (rul__decl_ext_rt_param_type (ext_rt, i),
				  buffer,
				  rul__decl_ext_rt_param_a_len (ext_rt, i),
				  rul__decl_ext_rt_param_mech (ext_rt, i));
  }
  if (extras)
    rul__emit_entry_arg_external (ext_type_none, "...",
				  EXT__C_NOT_ARRAY, ext_mech_value);

  rul__emit_entry_args_done (EMIT__E_ENTRY_DECLARATION);
}

static void print_shape_to_usefile (Decl_Shape shape)
{
  /* Store shape */
  if (shape == shape_atom) {
    print_to_usefile (SHAPE_SCALAR);
  }
  else if (shape == shape_compound) {
    print_to_usefile (SHAPE_COMPOUND);
  }
  else if (shape == shape_table) {
    print_to_usefile (SHAPE_TABLE);
  }
}

static void print_domain_to_usefile (Decl_Domain domain)
{
  /* Store domain */
  if (domain == dom_any) {
    print_to_usefile (DOMAIN_ANY);
  }
  else if (domain == dom_symbol) {
    print_to_usefile (DOMAIN_SYMBOL);
  }
  else if (domain == dom_instance_id) {
    print_to_usefile (DOMAIN_INSTANCE);
  }
  else if (domain == dom_opaque) {
    print_to_usefile (DOMAIN_OPAQUE);
  }
  else if (domain == dom_number) {
    print_to_usefile (DOMAIN_NUMBER);
  }
  else if (domain == dom_int_atom) {
    print_to_usefile (DOMAIN_INTEGER);
  }
  else if (domain == dom_dbl_atom) {
    print_to_usefile (DOMAIN_FLOAT);
  }
}

static void gen_open_use_file (char *file_spec)
{
  char *str;

  out_fptr = fopen (file_spec, "w");
  if (out_fptr) {
    str = rul__mem_malloc (strlen (file_spec) + 1);
    strcpy (str, file_spec);
    if (SA_use_file_names == NULL)
      SA_use_file_names = rul__dyar_create_array (10);
    rul__dyar_append (SA_use_file_names, str);
  }
}

void rul__gen_cleanup_usefiles (Boolean remove_use_files)
{
  char *use_file;

  if (SA_use_file_names) {
    use_file = (char *) rul__dyar_pop_first (SA_use_file_names);
    while (use_file != NULL) {
      if (remove_use_files)
	remove (use_file);
      rul__mem_free (use_file);
      use_file = (char *) rul__dyar_pop_first (SA_use_file_names);
    }
  }
}
