/*
 * Import declarations from modular-compilation index file
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
 *	This module is used only by the compiler.  It reads in
 *	information from the index file for modular compilation.
 *
 * MODIFIED BY:
 *	DEC	Digital Equipment Corporation
 *	CPQ	Compaq Computer Corporation
 *
 * MODIFICATION HISTORY:
 *
 *	16-Nov-1990	DEC	Initial version.
 *	20-Oct-1992	DEC	Finished adapting it from V4.
 *	19-Aug-1993	DEC	Changed all reads to use atomizer
 *	18-Dec-1997	DEC	Added call to rul__emit_global_external_ptr
 *					go that global externals are not optimzed away
 *	16-Feb-1998	DEC	class type changed to rclass
 *	01-Dec-1999	CPQ	Releasew ith GPL
 *
 */

#include <common.h>		/* declarations/definitions for RULEWORKS */
#include <cmp_comm.h>
#include <cons.h>
#include <ast.h>
#include <gen.h>
#include <mol.h>		/* declarations of molecule functions	    */
#include <atom.h>
#include <hash.h>		/* used as a field in the attribute struct  */
#include <decl.h>		/* functions to declare object classes etc. */
#include <emit.h>
#include <decl_c_p.h> 		/* class and decl_block struct defns    */
#include <indexf.h>		/* indexfile processing declarations */
#include <msg.h>
#include <cmp_msg.h>
#include <ios.h>
#include <ver_msg.h>
#include <cli.h>



static Boolean    SB__import_inited = FALSE;

static Molecule   SM__use_version;		/* "USE-VERSION" */
static Molecule   SM__use_ver_num;		/* "V5.0-0" */
static Molecule   SM__use_block;		/* "DECLARATION-BLOCK" */
static Molecule   SM__use_pat_size;		/* "PATTERN-SIZE" */
static Molecule   SM__use_com_id;		/* "COMPILATION-ID" */
static Molecule   SM__use_obj_class;		/* "OBJECT-CLASS" */
static Molecule   SM__use_inh_from;		/* "INHERITS-FROM" */
static Molecule   SM__use_import_def;		/* "IMPORT-DEFS" */
static Molecule   SM__use_scalar;		/* "SCALAR" */
static Molecule   SM__use_compound;		/* "COMPOUND" */
static Molecule   SM__use_table;		/* "TABLE" */
static Molecule   SM__use_any;			/* "ANY" */
static Molecule   SM__use_symbol;		/* "SYMBOL" */
static Molecule   SM__use_instance;		/* "INSTANCE" */
static Molecule   SM__use_opaque;		/* "OPAQUE" */
static Molecule   SM__use_number;		/* "NUMBER" */
static Molecule   SM__use_integer;		/* "INTEGER" */
static Molecule   SM__use_float;		/* "FLOAT" */
static Molecule   SM__use_of;			/* "OF" */
static Molecule   SM__use_default;		/* "DEFAULT" */
static Molecule   SM__use_fill;			/* "FILL" */
static Molecule   SM__use_generic_method;	/* "GENERIC-METHOD" */
static Molecule   SM__use_alias;		/* "ALIAS-FOR" */
static Molecule   SM__use_ext_rt;		/* "EXTERNAL-ROUTINE" */
static Molecule   SM__use_accepts;		/* "ACCEPTS" */
static Molecule   SM__use_returns;		/* "RETURNS" */
static Molecule   SM__use_end_block;		/* "END-BLOCK" */
static Molecule   SM__use_end_use;		/* "END-USE" */
static Molecule   SM__use_long;			/* "LONG" */
static Molecule   SM__use_short;		/* "SHORT" */
static Molecule   SM__use_byte;			/* "BYTE" */
static Molecule   SM__use_uns_long;		/* "UNSIGNED-LONG" */
static Molecule   SM__use_uns_short;		/* "UNSIGNED-SHORT" */
static Molecule   SM__use_uns_byte;		/* "UNSIGNED-BYTE" */
static Molecule   SM__use_sin_float;		/* "SINGLE-FLOAT" */
static Molecule   SM__use_dbl_float;		/* "DOUBLE-FLOAT" */
static Molecule   SM__use_asciz;		/* "ASCIZ" */
static Molecule   SM__use_ascid;		/* "ASCID" */
static Molecule   SM__use_atom;			/* "ATOM" */
static Molecule   SM__use_by_val;		/* "BY-VALUE" */
static Molecule   SM__use_by_ref_rw;		/* "BY-REFERENCE-READ-WRITE" */
static Molecule   SM__use_by_ref_ro;		/* "BY-REFERENCE-READ-ONLY" */
static char       SC_file_spec[RUL_C_MAX_READFORM_SIZE];
static char       SC_block_string[RUL_C_MAX_READFORM_SIZE];


static void     import_init (void);
static Boolean  find_token (Token_Type tokt);
static Boolean  find_block_named (Mol_Symbol block_name);
static Boolean  find_block (void);
static Boolean  atom_is (Molecule match);
static Boolean  use_version_verified (Mol_Symbol block_name);
static Boolean  find_block_named (Mol_Symbol block_name);
static Boolean  check_end_block (Mol_Symbol block_name);
static Boolean  find_pattern_size (Mol_Symbol block_name, long *pat_siz);
static Boolean  find_compilation_id (Mol_Symbol block_name, char *comp_id);
static Boolean  rul__decl_import_class (Mol_Symbol block_name, long patt_size);
static Boolean  rul__decl_import_ext_rt (Mol_Symbol block_name);
static Boolean  import_parameter (Mol_Symbol block_name, Boolean input);
static Boolean  rul__decl_import_generic_method (Mol_Symbol block_name);
static Ext_Type get_ext_type (Molecule atom);
static Ext_Mech get_ext_mech (Molecule atom);
static Boolean  get_shape_atom (Decl_Shape *shape);
static Boolean  get_domain_atom (Decl_Domain *domain);
static Boolean  decl_block_ok (Mol_Symbol block_name, Ast_Node ast);




Boolean  rul__sem_check_decl_usefile (Mol_Symbol block_name, Ast_Node ast)
{
  Decl_Block block;
  Token_Type tok_type;
  long       ptrn_size;
  Boolean    status;
  char       comp_id[RUL_C_MAX_READFORM_SIZE];
  Molecule   atom;
  long       line;
  FILE      *file_ptr;

  import_init ();
  
  block = rul__decl_get_block (block_name);
  if (block != NULL) {
    if (decl_block_ok (block_name, ast)) {
      rul__decl_make_block_visible (block_name);
      return TRUE;
    }
    else
      return FALSE;
  }

  /* Setup for index file reading */
  rul__mol_use_readform (block_name, SC_block_string, sizeof(SC_block_string));

  if (rul__cli_usedir_flag ())
    strcpy(SC_file_spec, rul__cli_usedir_option ());
  else
    SC_file_spec[0] = '\0';
  strcat(SC_file_spec, rul__decl_make_decl_short_name (block_name));
  strcat(SC_file_spec, RUL__C_DECL_FILE_EXT);

  file_ptr = fopen (SC_file_spec, "r");
  if (! file_ptr) {
    rul__msg_print (CMP_INXNOTFND, SC_file_spec, SC_block_string);
    return FALSE;
  }

  rul__atom_file_setup (file_ptr);

  /* Read use-version line */
  if (! use_version_verified (block_name))
    return FALSE;

  /* read declaration block line and make declaration block */
  if (! find_block_named (block_name)) {
    rul__msg_print_w_atoms (CMP_INXBLKNOTFND, 1, block_name);
    return FALSE;
  }

  rul__decl_create_decl_block (block_name);
  rul__cons_init ();
  rul__class_init ();

  /* get inheritance pattern size */
  if (! find_pattern_size (block_name, &ptrn_size))
    return FALSE; 

  rul__decl_set_inh_pattern_size (block_name, ptrn_size);

  /* read compilation id line */
  if (! find_compilation_id (block_name, comp_id))
    return FALSE;

  /* emit external reference for linkage mapping */
  rul__emit_blank_line ();
  rul__emit_comment (
     "The following external reference, defined when compiling the block:");
  rul__emit_comment (rul__mol_get_printform (block_name));
  rul__emit_comment (
     "will remain unresolved if linked with a different compilation.");
  rul__emit_global_external (ext_type_long, comp_id, 0, TRUE);
  rul__emit_global_external_ptr (ext_type_long, comp_id);
  /* read the rest of the file... */
  while (find_token (TOK_LPAREN)) {
    
    tok_type = rul__atom_get_atom (&atom);

    if (atom == SM__use_obj_class) {
      if (! rul__decl_import_class (block_name, ptrn_size))
	return FALSE;
    }

    else if (atom == SM__use_ext_rt) {
      if (! rul__decl_import_ext_rt (block_name))
	return FALSE;
    }

    else if (atom == SM__use_generic_method) {
      if (! rul__decl_import_generic_method (block_name))
	return FALSE;
    }

    else if (atom == SM__use_end_block) {
      if (check_end_block (block_name)) {
	rul__mol_mark_perm (block_name);
	rul__decl_make_block_visible (block_name);
	return TRUE;
      }
      else {
	return FALSE;
      }
    }
    else {
      rul__msg_print (CMP_INXINVFMT, SC_block_string,
		      rul__atom_get_line_count ());
      if (atom)
	rul__mol_decr_uses (atom);
      return FALSE;
    }
  }

  rul__msg_print (CMP_INXINVFMT, SC_block_string, rul__atom_get_line_count ());
  return FALSE;
}	



static Boolean rul__decl_import_class (Mol_Symbol block_name, long pat_size)
{
  Token_Type     tok_type;
  Molecule       atom;
  Class          rclass;
  long           i, pat_count;
  Boolean        status;
  static Boolean tmp_comp = FALSE;
  Decl_Shape     attr_shape;
  Decl_Domain    attr_dom;

  /*
   * get class name
   */

  if (tmp_comp) {
    atom = rul__mol_end_tmp_comp_w_decr ();
    rul__mol_decr_uses (atom);
    tmp_comp = FALSE;
  }

  tok_type = rul__atom_get_atom (&atom);
  if (tok_type != TOK_SYMBOL_CONST &&
      tok_type != TOK_QUOTED_SYMBOL &&
      tok_type != TOK_MIS_QUOTED_SYMBOL &&
      tok_type != TOK_COMPOUND) {
    rul__msg_print (CMP_INXINVFMT,
		    SC_block_string, rul__atom_get_line_count ());
    if (atom)
      rul__mol_decr_uses (atom);
    return FALSE;
  }

  rul__decl_create_class (block_name, atom);
  rclass = rul__decl_get_class (rul__decl_get_block (block_name), atom);

  /*
   * get parent class name
   */

  if (! find_token (TOK_LPAREN))
    return FALSE; 

  if (! atom_is (SM__use_inh_from)) {
    rul__msg_print (CMP_INXINVFMT,
		    SC_block_string, rul__atom_get_line_count ());
    if (atom)
      rul__mol_decr_uses (atom);
    return FALSE;
  }

  tok_type= rul__atom_get_atom (&atom);
  if (tok_type != TOK_SYMBOL_CONST &&
      tok_type != TOK_QUOTED_SYMBOL &&
      tok_type != TOK_MIS_QUOTED_SYMBOL &&
      tok_type != TOK_COMPOUND) {
    rul__msg_print (CMP_INXINVFMT,
		    SC_block_string, rul__atom_get_line_count ());
    if (atom)
      rul__mol_decr_uses (atom);
    return FALSE;
  }

  rul__decl_set_cur_class_parent (atom);

  /*
   * get inheritance mask length
   */

  if (! find_token (TOK_LPAREN))
    return FALSE; 

  if (! atom_is (SM__use_import_def)) {
    rul__msg_print (CMP_INXINVFMT,
		    SC_block_string, rul__atom_get_line_count ());
    if (atom)
      rul__mol_decr_uses (atom);
    return FALSE;
  }

  tok_type = rul__atom_get_atom (&atom);
  if (tok_type != TOK_INTEGER_CONST) {
    rul__msg_print (CMP_INXINVFMT,
		    SC_block_string, rul__atom_get_line_count ());
    if (atom)
      rul__mol_decr_uses (atom);
    return FALSE;
  }

  rul__decl_set_inh_mask_length (rclass, rul__mol_int_atom_value (atom));
  rul__mol_decr_uses (atom);

  /*
   * get inheritance patterns
   */

  for (i = 0; i < pat_size; i++) {
    tok_type= rul__atom_get_atom (&atom);
    if (tok_type != TOK_INTEGER_CONST) {
      rul__msg_print (CMP_INXINVFMT,
		      SC_block_string, rul__atom_get_line_count ());
      if (atom)
	rul__mol_decr_uses (atom);
      return FALSE;
    }
    rul__decl_set_inh_pattern_part(rclass, i, rul__mol_int_atom_value (atom));
    rul__mol_decr_uses (atom);
  }

  tok_type = rul__atom_get_atom (&atom);
  if (tok_type != TOK_RPAREN) {
    rul__msg_print (CMP_INXINVFMT,
		    SC_block_string, rul__atom_get_line_count ());
    if (atom)
      rul__mol_decr_uses (atom);
    return FALSE;
  }
  
  /*
   * for each attribute
   */

  while (TRUE) {

    /*
     * attribute name or end object-class
     */

    tok_type = rul__atom_get_atom (&atom);

    if (tok_type == TOK_RPAREN ||
	tok_type == TOK_EOF)
      break;

    if (tok_type == TOK_LPAREN) {
      rul__mol_decr_uses (atom);
      continue;
    }

    if (tok_type != TOK_SYMBOL_CONST &&
	tok_type != TOK_QUOTED_SYMBOL &&
	tok_type != TOK_MIS_QUOTED_SYMBOL &&
	tok_type != TOK_COMPOUND) {
      rul__msg_print (CMP_INXINVFMT,
		      SC_block_string, rul__atom_get_line_count ());
      if (atom)
	rul__mol_decr_uses (atom);
      return FALSE;
    }
  
    rul__decl_add_attr_cur_class (atom);
      
    /*
     * attribute offset
     */
    
    tok_type = rul__atom_get_atom (&atom);
    if (tok_type != TOK_INTEGER_CONST) {
      rul__msg_print (CMP_INXINVFMT,
		      SC_block_string, rul__atom_get_line_count ());
      if (atom)
	rul__mol_decr_uses (atom);
      return FALSE;
    }
    
    rul__decl_set_cur_attr_offset (rul__mol_int_atom_value (atom));
    rul__mol_decr_uses (atom);

    /*
     * attribute shape
     */

    if (!get_shape_atom (&attr_shape))
      return FALSE;

    if (attr_shape == shape_compound)
      rul__decl_set_cur_attr_compound ();

    /*
     *else if (attr_shape == shape_table)
     *  rul__decl_set_cur_attr_table ();
     */

    /*
     * attribute domain
     */

    if (!get_domain_atom (&attr_dom))
      return FALSE;

    while (TRUE) {

      /*
       * [(OF | (DEFAULT | (FILL ]
       */

      tok_type = rul__atom_get_atom (&atom);
      if (tok_type == TOK_LPAREN) {
	tok_type = rul__atom_get_atom (&atom);
      }

      if (tok_type == TOK_RPAREN) {     /* end of this attr's defs */
	rul__mol_decr_uses (atom);
	break;
      }

      if (atom == SM__use_of) {
	if (attr_dom != dom_instance_id) {
	  rul__msg_print (CMP_INXINVFMT,
			  SC_block_string, rul__atom_get_line_count ());
	  return FALSE;
	}
	tok_type = rul__atom_get_atom (&atom);
	rul__decl_set_cur_attr_class (rul__decl_get_visible_class(atom));
      }

      else if (atom == SM__use_default) {

	if (attr_shape == shape_atom) {
	  tok_type = rul__atom_get_atom (&atom);
	  rul__decl_set_cur_attr_default (atom);
	}

	else if (attr_shape == shape_compound) {
	  if (! find_token (TOK_COMPOUND)) {
	    rul__msg_print (CMP_INXINVFMT,
			    SC_block_string, rul__atom_get_line_count ());
	    return FALSE; 
	  }

	  if (tmp_comp) {
	    atom = rul__mol_end_tmp_comp_w_decr ();
	    rul__mol_decr_uses (atom);
	  }
	  rul__mol_start_tmp_comp (0);
	  tmp_comp = TRUE;
	  tok_type = rul__atom_get_atom (&atom); /* 1st after find compound */
	  for (i = 0; tok_type != TOK_RPAREN; i++) {
	    rul__mol_set_tmp_comp_nth (i, atom);
	    tok_type = rul__atom_get_atom (&atom);
	  }
	  rul__decl_set_cur_attr_default (rul__mol_end_tmp_comp_w_decr ());
	  tmp_comp = FALSE;
	}

	else if (attr_shape == shape_table) {
	}

	else {
	  rul__msg_print (CMP_INXINVFMT,
			  SC_block_string, rul__atom_get_line_count ());
	  if (atom)
	    rul__mol_decr_uses (atom);
	  return FALSE;
	}
      }

      else if (atom == SM__use_fill) {
	if (attr_shape != shape_compound) {
	  rul__msg_print (CMP_INXINVFMT,
			  SC_block_string, rul__atom_get_line_count ());
	  return FALSE;
	}
	tok_type = rul__atom_get_atom (&atom);
	rul__decl_set_cur_attr_fill (atom);
      }

      else {
	rul__msg_print (CMP_INXINVFMT,
			SC_block_string, rul__atom_get_line_count ());
	if (atom)
	  rul__mol_decr_uses (atom);
	return FALSE;
      }
    
      tok_type = rul__atom_get_atom (&atom); /* ) of (OF | (DEFAULT | (FILL */
      if (atom)
	rul__mol_decr_uses (atom);
    }
  }

  if (tok_type != TOK_RPAREN) {
    rul__msg_print (CMP_INXINVFMT,
		    SC_block_string, rul__atom_get_line_count ());
    if (atom)
      rul__mol_decr_uses (atom);
    return FALSE;
  }

  rul__decl_finish_cur_class ();
  return TRUE;
}

static Boolean rul__decl_import_ext_rt (Mol_Symbol block_name)
{
  Token_Type tok_type;
  Molecule   atom;

  /*
   * get external routine name
   */

  tok_type = rul__atom_get_atom (&atom);
  if (tok_type != TOK_SYMBOL_CONST &&
      tok_type != TOK_QUOTED_SYMBOL &&
      tok_type != TOK_MIS_QUOTED_SYMBOL) {
    rul__msg_print (CMP_INXINVFMT,
		    SC_block_string, rul__atom_get_line_count ());
    if (atom)
      rul__mol_decr_uses (atom);
    return FALSE;
  }

  rul__decl_create_ext_rt (atom);
  rul__mol_decr_uses (atom);
  
  /*
   * get alias, accepts, returns clauses
   */

  while (TRUE) {

    tok_type = rul__atom_get_atom (&atom);
    
    if (tok_type == TOK_RPAREN ||
	tok_type == TOK_EOF)
      break;
    
    tok_type = rul__atom_get_atom (&atom);

    if (atom == SM__use_alias) {
      tok_type = rul__atom_get_atom (&atom);
      if (tok_type != TOK_SYMBOL_CONST &&
	  tok_type != TOK_QUOTED_SYMBOL &&
	  tok_type != TOK_MIS_QUOTED_SYMBOL) {
	rul__msg_print (CMP_INXINVFMT,
			SC_block_string, rul__atom_get_line_count ());
	if (atom)
	  rul__mol_decr_uses (atom);
	return FALSE;
      }
      rul__decl_add_ext_rt_alias (atom);
      rul__mol_decr_uses (atom);
      tok_type = rul__atom_get_atom (&atom);
      if (tok_type != TOK_RPAREN) {
	rul__msg_print (CMP_INXINVFMT,
			SC_block_string, rul__atom_get_line_count ());
	if (atom)
	  rul__mol_decr_uses (atom);
	return FALSE;
      }
    }

    else if (atom == SM__use_accepts) {
      if (! import_parameter (block_name, TRUE))
	return FALSE;
    }

    else if (atom == SM__use_returns) {
      if (! import_parameter (block_name, FALSE))
	return FALSE;
    }
  }

  if (tok_type != TOK_RPAREN) {
    rul__msg_print (CMP_INXINVFMT,
		    SC_block_string, rul__atom_get_line_count ());
    if (atom)
      rul__mol_decr_uses (atom);
    return FALSE;
  }

  rul__decl_finish_ext_rt ();

  if (atom)
    rul__mol_decr_uses (atom);

  return TRUE;
}

static Boolean rul__decl_import_generic_method (Mol_Symbol block_name)
{
  Token_Type   tok_type;
  Molecule     atom, meth_name, class_name;
  long         i, par_cnt;
  Decl_Shape   shape;
  Decl_Domain  domain;
  Class        rclass;

  /*
   *  (GENERIC-METHOD <name> <parameter_count>
   *    (ACCEPTS  shape domain class
   *              ... )
   *    (RETURNS  shape domain class)
   *  )
   */

  /* get generic-method name */
  tok_type = rul__atom_get_atom (&atom);
  if (tok_type != TOK_SYMBOL_CONST &&
      tok_type != TOK_QUOTED_SYMBOL &&
      tok_type != TOK_MIS_QUOTED_SYMBOL) {
    rul__msg_print (CMP_INXINVFMT,
		    SC_block_string, rul__atom_get_line_count ());
    if (atom)
      rul__mol_decr_uses (atom);
    return FALSE;
  }

  meth_name = atom;

  /* get accepts parameter count */
  tok_type = rul__atom_get_atom (&atom);
  if (tok_type != TOK_INTEGER_CONST) {
    rul__msg_print (CMP_INXINVFMT,
		    SC_block_string, rul__atom_get_line_count ());
    if (atom)
      rul__mol_decr_uses (atom);
    return FALSE;
  }

  par_cnt = rul__mol_int_atom_value (atom);
  rul__mol_decr_uses (atom);

  rul__decl_create_method (rul__decl_get_block (block_name),
			   meth_name, FALSE, par_cnt);
  rul__mol_decr_uses (meth_name);
  
  if (!find_token (TOK_LPAREN)) {
    rul__msg_print (CMP_INXINVFMT,
		    SC_block_string, rul__atom_get_line_count ());
    return FALSE;
  }

  tok_type = rul__atom_get_atom (&atom);
  if (atom != SM__use_accepts) {
    rul__msg_print (CMP_INXINVFMT,
		    SC_block_string, rul__atom_get_line_count ());
    return FALSE;
  }

  for (i = 0; i < par_cnt; i++) {

    if (!get_shape_atom (&shape))
      return FALSE;

    if (!get_domain_atom (&domain))
      return FALSE;

    /* get class restriction */
    tok_type = rul__atom_get_atom (&atom);

    if (tok_type != TOK_SYMBOL_CONST &&
	tok_type != TOK_QUOTED_SYMBOL &&
	tok_type != TOK_COMPOUND &&
	tok_type != TOK_MIS_QUOTED_SYMBOL) {
      rul__msg_print (CMP_INXINVFMT,
		      SC_block_string, rul__atom_get_line_count ());
      if (atom)
	rul__mol_decr_uses (atom);
      return FALSE;
    }

    rclass = NULL;
    if (atom != rul__mol_symbol_root ())
      rclass = rul__decl_get_class (rul__decl_get_block (block_name), atom);

    rul__decl_set_method_param (i, shape, domain, rclass);
  }

  tok_type = rul__atom_get_atom (&atom);
  if (tok_type != TOK_RPAREN) {
    rul__msg_print (CMP_INXINVFMT,
		    SC_block_string, rul__atom_get_line_count ());
    return FALSE;
  }

  /* returns clause */

  if (!find_token (TOK_LPAREN)) {
    rul__msg_print (CMP_INXINVFMT,
		    SC_block_string, rul__atom_get_line_count ());
    return FALSE;
  }

  tok_type = rul__atom_get_atom (&atom);
  if (atom != SM__use_returns) {
    rul__msg_print (CMP_INXINVFMT,
		    SC_block_string, rul__atom_get_line_count ());
    return FALSE;
  }

  if (!get_shape_atom (&shape))
    return FALSE;

  if (!get_domain_atom (&domain))
    return FALSE;

  /* get class restriction */
  tok_type = rul__atom_get_atom (&atom);

  if (tok_type != TOK_SYMBOL_CONST &&
      tok_type != TOK_QUOTED_SYMBOL &&
      tok_type != TOK_COMPOUND &&
      tok_type != TOK_MIS_QUOTED_SYMBOL) {
    rul__msg_print (CMP_INXINVFMT,
		    SC_block_string, rul__atom_get_line_count ());
    if (atom)
      rul__mol_decr_uses (atom);
    return FALSE;
  }

  rclass = NULL;
  if (atom != rul__mol_symbol_root ())
    rclass = rul__decl_get_class (rul__decl_get_block (block_name), atom);

  rul__decl_set_method_param (i, shape, domain, rclass);

  tok_type = rul__atom_get_atom (&atom);
  if (tok_type != TOK_RPAREN) {
    rul__msg_print (CMP_INXINVFMT,
		    SC_block_string, rul__atom_get_line_count ());
    return FALSE;
  }

  return (find_token (TOK_RPAREN));
}

static Boolean check_end_block (Mol_Symbol block_name)
{
  Molecule   atom;
  Token_Type tok_type;

  if (! atom_is (block_name)) {
    rul__msg_print (CMP_INXINVFMT,
		    SC_block_string, rul__atom_get_line_count ());
    return FALSE;
  }

  return TRUE;
}

void import_init (void)
{


  if (!SB__import_inited) {
    SB__import_inited = TRUE;
    SM__use_version = rul__mol_make_symbol ("USE-VERSION");
    rul__mol_mark_perm (SM__use_version);
    SM__use_ver_num = rul__mol_make_symbol ("V5.0-0");
    rul__mol_mark_perm (SM__use_ver_num);
    SM__use_block = rul__mol_make_symbol ("DECLARATION-BLOCK");
    rul__mol_mark_perm (SM__use_block);
    SM__use_pat_size = rul__mol_make_symbol ("PATTERN-SIZE");
    rul__mol_mark_perm (SM__use_pat_size);
    SM__use_com_id = rul__mol_make_symbol ("COMPILATION-ID");
    rul__mol_mark_perm (SM__use_com_id);
    SM__use_obj_class = rul__mol_make_symbol ("OBJECT-CLASS");
    rul__mol_mark_perm (SM__use_obj_class);
    SM__use_inh_from = rul__mol_make_symbol ("INHERITS-FROM");
    rul__mol_mark_perm (SM__use_inh_from);
    SM__use_import_def = rul__mol_make_symbol ("IMPORT-DEFS");
    rul__mol_mark_perm (SM__use_import_def);
    SM__use_scalar = rul__mol_make_symbol ("SCALAR");
    rul__mol_mark_perm (SM__use_scalar);
    SM__use_compound = rul__mol_make_symbol ("COMPOUND");
    rul__mol_mark_perm (SM__use_compound);
    SM__use_table = rul__mol_make_symbol ("TABLE");
    rul__mol_mark_perm (SM__use_table);
    SM__use_any = rul__mol_make_symbol ("ANY");
    rul__mol_mark_perm (SM__use_any);
    SM__use_symbol = rul__mol_make_symbol ("SYMBOL");
    rul__mol_mark_perm (SM__use_symbol);
    SM__use_instance = rul__mol_make_symbol ("INSTANCE");
    rul__mol_mark_perm (SM__use_instance);
    SM__use_opaque = rul__mol_make_symbol ("OPAQUE");
    rul__mol_mark_perm (SM__use_opaque);
    SM__use_number = rul__mol_make_symbol ("NUMBER");
    rul__mol_mark_perm (SM__use_number);
    SM__use_integer = rul__mol_make_symbol ("INTEGER");
    rul__mol_mark_perm (SM__use_integer);
    SM__use_float = rul__mol_make_symbol ("FLOAT");
    rul__mol_mark_perm (SM__use_float);
    SM__use_of = rul__mol_make_symbol ("OF");
    rul__mol_mark_perm (SM__use_of);
    SM__use_default = rul__mol_make_symbol ("DEFAULT");
    rul__mol_mark_perm (SM__use_default);
    SM__use_fill = rul__mol_make_symbol ("FILL");
    rul__mol_mark_perm (SM__use_fill);
    SM__use_generic_method = rul__mol_make_symbol ("GENERIC-METHOD");
    rul__mol_mark_perm (SM__use_generic_method);
    SM__use_alias = rul__mol_make_symbol ("ALIAS-FOR");
    rul__mol_mark_perm (SM__use_alias);
    SM__use_ext_rt = rul__mol_make_symbol ("EXTERNAL-ROUTINE");
    rul__mol_mark_perm (SM__use_ext_rt);
    SM__use_accepts = rul__mol_make_symbol ("ACCEPTS");
    rul__mol_mark_perm (SM__use_accepts);
    SM__use_returns = rul__mol_make_symbol ("RETURNS");
    rul__mol_mark_perm (SM__use_returns);
    SM__use_end_block = rul__mol_make_symbol ("END-BLOCK");
    rul__mol_mark_perm (SM__use_end_block);
    SM__use_end_use = rul__mol_make_symbol ("END-USE");
    rul__mol_mark_perm (SM__use_end_use);
    SM__use_long = rul__mol_make_symbol ("LONG");
    rul__mol_mark_perm (SM__use_long);
    SM__use_short = rul__mol_make_symbol ("SHORT");
    rul__mol_mark_perm (SM__use_short);
    SM__use_byte = rul__mol_make_symbol ("BYTE");
    rul__mol_mark_perm (SM__use_byte);
    SM__use_uns_long = rul__mol_make_symbol ("UNSIGNED-LONG");
    rul__mol_mark_perm (SM__use_uns_long);
    SM__use_uns_short = rul__mol_make_symbol ("UNSIGNED-SHORT");
    rul__mol_mark_perm (SM__use_uns_short);
    SM__use_uns_byte = rul__mol_make_symbol ("UNSIGNED-BYTE");
    rul__mol_mark_perm (SM__use_uns_byte);
    SM__use_sin_float = rul__mol_make_symbol ("SINGLE-FLOAT");
    rul__mol_mark_perm (SM__use_sin_float);
    SM__use_dbl_float = rul__mol_make_symbol ("DOUBLE-FLOAT");
    rul__mol_mark_perm (SM__use_dbl_float);
    SM__use_asciz = rul__mol_make_symbol ("ASCIZ");
    rul__mol_mark_perm (SM__use_asciz);
    SM__use_ascid = rul__mol_make_symbol ("ASCID");
    rul__mol_mark_perm (SM__use_ascid);
    SM__use_atom = rul__mol_make_symbol ("ATOM");
    rul__mol_mark_perm (SM__use_atom);
    SM__use_by_val = rul__mol_make_symbol ("BY-VALUE");
    rul__mol_mark_perm (SM__use_by_val);
    SM__use_by_ref_rw = rul__mol_make_symbol ("BY-REFERENCE-READ-WRITE");
    rul__mol_mark_perm (SM__use_by_ref_rw);
    SM__use_by_ref_ro = rul__mol_make_symbol ("BY-REFERENCE-READ-ONLY");
    rul__mol_mark_perm (SM__use_by_ref_ro);
  }
}

static Boolean atom_is (Molecule match)
{
  Token_Type tok_type;
  Molecule atom;
  Boolean  status = FALSE;

  tok_type = rul__atom_get_atom (&atom);
  if (atom == match)
    status = TRUE;

  if (atom)
    rul__mol_decr_uses (atom);

  return status;
}

static Boolean find_token (Token_Type find_tok_typ)
{
  Token_Type tok_type;
  Molecule atom;

  tok_type = rul__atom_get_atom (&atom);

  while (tok_type != find_tok_typ) {
    if (tok_type == TOK_EOF)
      return FALSE;
    rul__mol_decr_uses (atom);
    tok_type = rul__atom_get_atom (&atom);
  }
  rul__mol_decr_uses (atom);
  return TRUE;
}



static Boolean use_version_verified (Mol_Symbol block_name)
{

  if (! find_token (TOK_LPAREN))
    return FALSE; 

  if (! atom_is (SM__use_version)) {
    rul__msg_print (CMP_INXINVFMT,
		    SC_block_string, rul__atom_get_line_count ());
    return FALSE;
  }

  if (! atom_is (SM__use_ver_num)) {
    rul__msg_print_w_atoms (CMP_INXOLDVER, 1, block_name);    
    return FALSE;
  }

  if (! find_token (TOK_RPAREN)) {
    rul__msg_print (CMP_INXINVFMT,
		    SC_block_string, rul__atom_get_line_count ());
    return FALSE;
  }

  return TRUE;
}

static Boolean find_block_named (Mol_Symbol block_name)
{

  while (find_block ()) {
    if (atom_is (block_name))
      return TRUE;
  }

  return FALSE;
}

static Boolean find_block (void)
{

  while (TRUE) {
    if (! find_token (TOK_LPAREN))
      break;
    if (atom_is (SM__use_block))
      return TRUE;
  }

  return FALSE;
}

static Boolean find_pattern_size (Mol_Symbol block_name, long *pat_size)
{
  Token_Type tok_type;
  Molecule   atom;

  if (! find_token (TOK_LPAREN))
    return FALSE; 

  if (! atom_is (SM__use_pat_size)) {
    rul__msg_print (CMP_INXINVFMT,
		    SC_block_string, rul__atom_get_line_count ());
    return FALSE;
  }

  tok_type = rul__atom_get_atom (&atom);
  if (tok_type != TOK_INTEGER_CONST) {
    rul__msg_print (CMP_INXINVFMT,
		    SC_block_string, rul__atom_get_line_count ());
    if (atom)
      rul__mol_decr_uses (atom);
    return FALSE;
  }

  *pat_size = rul__mol_int_atom_value (atom);
  rul__mol_decr_uses (atom);

  return TRUE;
}

static Boolean find_compilation_id (Mol_Symbol block_name, char *comp_id)
{
  Token_Type tok_type;
  Molecule   atom;

  if (! find_token (TOK_LPAREN))
    return FALSE; 

  if (! atom_is (SM__use_com_id)) {
    rul__msg_print (CMP_INXINVFMT,
		    SC_block_string, rul__atom_get_line_count ());
    return FALSE;
  }

  tok_type = rul__atom_get_atom (&atom);
  if (tok_type != TOK_SYMBOL_CONST) {
    rul__msg_print (CMP_INXINVFMT,
		    SC_block_string, rul__atom_get_line_count ());
    if (atom)
      rul__mol_decr_uses (atom);
    return FALSE;
  }

  rul__mol_use_printform (atom, comp_id, RUL_C_MAX_READFORM_SIZE);
  rul__mol_decr_uses (atom);

  return TRUE;
}

static Boolean import_parameter (Mol_Symbol block_name, Boolean input)
{
  Molecule    atom, line_mol;
  Token_Type  tok_type;
  Ext_Type    type;
  Ext_Mech    mech;
  Boolean     status;
  Cardinality a_len;
  Mol_Symbol  par, a_arg;
  long        i = 0;
  char        parbuf[RUL_C_MAX_SYMBOL_SIZE+1];

  /*
   * loop for all accepts parameters (once for returns)
   */

  while (TRUE) { 

    /*
     * get external type
     */
    
    a_len = 0;
    a_arg = NULL;
    sprintf (parbuf, "P%ld", i++);
    par = rul__mol_make_symbol (parbuf);

    tok_type = rul__atom_get_atom (&atom);

    if (tok_type == TOK_RPAREN ||
	tok_type == TOK_EOF)
      break;

    type = get_ext_type (atom);
    if (type == ext_type_invalid) {
      rul__msg_print (CMP_INXINVFMT,
		      SC_block_string, rul__atom_get_line_count ());
      if (atom)
	rul__mol_decr_uses (atom);
      return FALSE;
    }
    
    /*
     * get cardinality
     */
    
    tok_type = rul__atom_get_atom (&atom);
    if (tok_type != TOK_INTEGER_CONST &&
	tok_type != TOK_SYMBOL_CONST) {
      rul__msg_print (CMP_INXINVFMT,
		      SC_block_string, rul__atom_get_line_count ());
      if (atom)
	rul__mol_decr_uses (atom);
      return FALSE;
    }
    
    if (tok_type == TOK_INTEGER_CONST) {
      a_len = rul__mol_int_atom_value (atom);
      rul__mol_decr_uses (atom);
    }
    else {
      a_len = EXT__C_IMPLICIT_ARRAY;
      a_arg = atom;
    }
    
    /*
     * get mechanism
     */
    
    tok_type = rul__atom_get_atom (&atom);
    mech = get_ext_mech (atom);
    if (mech == ext_mech_invalid) {
      rul__msg_print (CMP_INXINVFMT,
		      SC_block_string, rul__atom_get_line_count ());
      if (atom)
	rul__mol_decr_uses (atom);
      return FALSE;
    }
    
    /* got parameter data add parameter */
    if (input)
      rul__decl_add_ext_rt_param (type, a_len, mech, par, a_arg);
    else
      rul__decl_add_ext_rt_ret_val (type, a_len, mech, a_arg);

    if (par)
      rul__mol_decr_uses (par);
    if (a_arg != NULL)
      rul__mol_decr_uses (a_arg);
  }

  if (tok_type != TOK_RPAREN) {
    rul__msg_print (CMP_INXINVFMT,
		    SC_block_string, rul__atom_get_line_count ());
    if (atom)
      rul__mol_decr_uses (atom);
    return FALSE;
  }

  if (atom)
    rul__mol_decr_uses (atom);

  return TRUE;
}



static Ext_Type get_ext_type (Molecule atom)
{
  if (atom == SM__use_long)
    return ext_type_long;
  else if (atom == SM__use_short)
    return ext_type_short;
  else if (atom == SM__use_byte)
    return ext_type_byte;
  else if (atom == SM__use_uns_long)
    return ext_type_uns_long;
  else if (atom == SM__use_uns_short)
    return ext_type_uns_short;
  else if (atom == SM__use_uns_byte)
    return ext_type_uns_byte;
  else if (atom == SM__use_sin_float)
    return ext_type_float;
  else if (atom == SM__use_dbl_float)
    return ext_type_double;
  else if (atom == SM__use_asciz)
    return ext_type_asciz;
  else if (atom == SM__use_ascid)
    return ext_type_ascid;
  else if (atom == SM__use_opaque)
    return ext_type_void;
  else if (atom == SM__use_atom)
    return ext_type_atom;
  else 
    return ext_type_invalid;
}

static Ext_Mech get_ext_mech (Molecule atom)
{
  if (atom == SM__use_by_val)
    return ext_mech_value;
  else if (atom == SM__use_by_ref_rw)
    return ext_mech_ref_rw;
  else if (atom == SM__use_by_ref_ro)
    return ext_mech_ref_ro;
  else
    return ext_mech_invalid;
}

static Boolean index_contains_other_decls (Mol_Symbol blk_name_sym,
					   char *blk_name)
{
  Boolean status = FALSE;

  while (find_block ()) {
    if (! atom_is (blk_name_sym))
      return TRUE;
  }
  return FALSE;
}

Boolean rul__decl_idx_cont_other_decls (char *file_spec,
					Mol_Symbol blk_name_sym,
					char *blk_name)
{
  Boolean status = FALSE;
  FILE    *fptr;

  fptr = fopen (file_spec, "r");

  if (fptr != NULL) {
    import_init ();
    rul__atom_file_setup (fptr);
    status = index_contains_other_decls (blk_name_sym, blk_name);
    rul__atom_file_close ();
  }
  return status;
}


static Boolean get_shape_atom (Decl_Shape *shape)
{
  Molecule   atom;
  Token_Type tok_type;

  /* get shape */
  tok_type = rul__atom_get_atom (&atom);

  if (tok_type != TOK_SYMBOL_CONST &&
      tok_type != TOK_COMPOUND) {
    rul__msg_print (CMP_INXINVFMT,
		    SC_block_string, rul__atom_get_line_count ());
    if (atom)
      rul__mol_decr_uses (atom);
    return FALSE;
  }
  
  if (atom == SM__use_scalar)
    *shape = shape_atom;
  else if (atom == SM__use_compound)
    *shape = shape_compound;
  else if (atom == SM__use_table)
    *shape = shape_table;
  else {
    rul__msg_print (CMP_INXINVFMT,
		    SC_block_string, rul__atom_get_line_count ());
    if (atom)
      rul__mol_decr_uses (atom);
    return FALSE;
  }
  rul__mol_decr_uses (atom);
  return TRUE;
}

static Boolean get_domain_atom (Decl_Domain *domain)
{
  Molecule   atom;
  Token_Type tok_type;

  /* get domain */
  tok_type = rul__atom_get_atom (&atom);

  if (tok_type != TOK_SYMBOL_CONST) {
    rul__msg_print (CMP_INXINVFMT,
		    SC_block_string, rul__atom_get_line_count ());
    if (atom)
      rul__mol_decr_uses (atom);
    return FALSE;
  }
  
  if (atom == SM__use_any)
    *domain = dom_any;
  else if (atom == SM__use_symbol)
    *domain = dom_symbol;
  else if (atom == SM__use_instance)
    *domain = dom_instance_id;
  else if (atom == SM__use_opaque)
    *domain = dom_opaque;
  else if (atom == SM__use_number)
    *domain = dom_number;
  else if (atom == SM__use_integer)
    *domain = dom_int_atom;
  else if (atom == SM__use_float)
    *domain = dom_dbl_atom;
  else {
    rul__msg_print (CMP_INXINVFMT,
		    SC_block_string, rul__atom_get_line_count ());
    if (atom)
      rul__mol_decr_uses (atom);
    return FALSE;
  }
  rul__mol_decr_uses (atom);
  return TRUE;
}

static Boolean decl_block_ok (Mol_Symbol block_name, Ast_Node ast)
{
  Decl_Block   decl_blk = rul__decl_get_block (block_name);
  Class        rclass, visible_class;
  Ext_Rt_Decl  ext_rt, dup_rt;
  Mol_Symbol   mol;

  /* check classes */

  if (rul__decl_block_has_classes (block_name)) {
    rclass = rul__decl_get_block_classes (decl_blk);
    while (rclass) {
      mol = rul__decl_get_class_name (rclass);
      if (mol != rul__mol_symbol_root ()) {
	visible_class = rul__decl_get_visible_class (mol);
	if (visible_class) {
	  rul__msg_cmp_print_w_atoms (CMP_MULDECCLS, ast, 3, mol,
			      rul__decl_get_class_block_name (visible_class),
				      block_name);
	  return FALSE;
	}
      }
      rclass = rul__decl_get_class_next (rclass);
    }
  }

  /* check external routines */

  ext_rt = rul__decl_get_block_routines (decl_blk);
  while (ext_rt) {
    mol = rul__decl_ext_rt_name (ext_rt);
    dup_rt = rul__decl_get_visible_ext_rt (mol);
    if (dup_rt) {
      if (! rul__decl_ext_rts_are_identical (ext_rt, dup_rt)) {
        rul__msg_cmp_print_w_atoms (CMP_EXTMULDEF, ast, 3, mol, block_name,
                       rul__decl_get_decl_name (
				rul__decl_ext_rt_decl_blk (dup_rt)));
      }
    }
    ext_rt = rul__decl_get_next_ext_rt (ext_rt);
  }

  /* check methods */


  return TRUE;
}
