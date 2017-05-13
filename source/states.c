#include <stdio.h>
/****************************************************************************
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

/*
 * FACILITY:
 *	RULEWORKS run time system
 *
 * ABSTRACT:
 *	This module provides routines used by the ADDSTATE,SAVESTATE,
 *	and RESTORESTATE directives.
 *
 * MODIFIED BY:
 *	DEC	Digital Equipment Corporation
**	CPQ	Compaq Computer Corporation
 *
 * MODIFICATION HISTORY:
 *
 *	 29-Sep-1993	DEC	Initial version
 *
 *	 25-Sep-1997	DEC	Check $VERSION in ADDSTATE
 *
 *	 21-Aug-1998	DEC	rul__at function added
 *
 *	 23-Nov-1998	DEC	rul__at modified to close current stream
 *
 *	01-Dec-1999		CPQ	Release with GPL
 */



#include <common.h>
#include <callback.h>
#include <states.h>
#include <atom.h>
#include <mol.h>
#include <wm.h>
#include <ios.h>
#include <rac.h>
#include <ref.h>
#include <rbs.h>
#include <msg.h>
#include <api_msg.h>

static void     rul___save_print_wm (Object wmo);
static Boolean  rul___validate_rule (Molecule rb, Molecule rule,
				     long rb_count, Molecule *rb_names);
 
static IO_Stream   SA_save_ios = NULL;
static char       *SC_save_header = "($VERSION 5)";
static char       *SC_save_refset = "($REF-SET)";
static char       *SC_save_end    = "($END-SAVE)";

static Molecule  *SM_ids = NULL;
static long       SL_ids_len = 0;


/*************************
**			**
** RUL__AT       	**
**			**
**************************/
void
rul__at (Mol_Symbol file_sym)
{
   IO_Stream fp;
  	
   /* 
   ** SB : 23-Nov-1998 
   ** Close the current file stream 
   */
   rul__scan_switch_file (NULL); 

   /* Open the new file stream */
   fp = rul__ios_open_file (file_sym, file_sym, IOS__E_IN);
   if (fp != NULL)
   {
      rul__scan_switch_file (fp); 
   }
}



/*************************
**			**
** RUL__ADDSTATE	**
**			**
**************************/

void
rul__addstate (Mol_Symbol file_sym, Entry_Data eb_data)
{
  volatile Molecule wme_id;
  char file_buf[RUL_C_MAX_SYMBOL_SIZE+1];
  FILE *file_ptr;
  char buf[RUL_C_MAX_SYMBOL_SIZE+1];

  rul__mol_use_printform (file_sym, file_buf, RUL_C_MAX_SYMBOL_SIZE);
  file_ptr = fopen (file_buf, "r");
  if (! file_ptr) {
    rul__msg_print (API_CANTOPEN, file_buf, "input");
    return;
  }

  rul__atom_file_setup (file_ptr);
  rul__ios_set_bizzare_args (RUL__C_ATOM, 0, 0,
			     (void **) &SC_save_refset,
			     (void **) &SC_save_end);


/* SB : 25-Sep-1997 - Get $VERSION line and check version */
  rul__ios_fgets (RUL__C_ATOM, buf, RUL_C_MAX_SYMBOL_SIZE);
  if (strcmp (buf, SC_save_header)) {
    rul__msg_print (API_INVRESFIL, file_buf, buf);
    return;
  }

  rul__start_addstate_trans ();

  do { wme_id = rul___make_wmos (eb_data->db_name_count,
				 eb_data->db_names);
     }
    while (wme_id);

  rul__end_addstate_trans ();

}




/*************************
**			**
** RUL__RESTORESTATE	**
**			**
**************************/

static void rul___rest_destroy (Object wmo)
{
  rul__wm_destroy_and_notify (wmo, NULL);
}


void
rul__restorestate (Mol_Symbol file_sym, Entry_Data eb_data)
{
  Boolean status = TRUE;
  volatile Molecule wme_id;
  char file_buf[RUL_C_MAX_SYMBOL_SIZE+1];
  FILE *file_ptr;
  char buf[RUL_C_MAX_SYMBOL_SIZE+1];
  Mol_Symbol rule_name, rb_name, lparen, rparen, icnt, id;
  long i, id_count;
  Token_Type tok_type;
  Molecule line_mol;

  rul__mol_use_printform (file_sym, file_buf, RUL_C_MAX_SYMBOL_SIZE);
  file_ptr = fopen (file_buf, "r");
  if (! file_ptr) {
    rul__msg_print (API_CANTOPEN, file_buf, "input");
    return;
  }

  rul__atom_file_setup (file_ptr);
  rul__ios_set_bizzare_args (RUL__C_ATOM, 0, 0,
			     (void **) &SC_save_refset, 0);

  rul__ios_fgets (RUL__C_ATOM, buf, RUL_C_MAX_SYMBOL_SIZE);
  if (strcmp (buf, SC_save_header)) {
    rul__msg_print (API_INVRESFIL, file_buf, buf);
    return;
  }

  rul__wm_for_each_known_object (eb_data->db_name_count,
				 eb_data->db_names,
				 rul___rest_destroy);

  rul__start_addstate_trans ();

  do { wme_id = rul___make_wmos (eb_data->db_name_count,
				 eb_data->db_names);
       if (wme_id == (Molecule) -1)
	 status = FALSE;
     }
    while (wme_id);

  /* should refraction set be restored if a wmo creation failed ? */

  if (status == TRUE) {

    rul__ios_set_bizzare_args (RUL__C_ATOM, 0, 0,
			       (void **) &SC_save_end, 0);
    rul__atom_restart (NULL);
    rul__ref_empty_refraction_set (eb_data->refraction_set);

    while (TRUE) {
      
      tok_type = rul__atom_get_atom (&lparen);
      
      if (tok_type == TOK_EOF)
	break;
      
      if (tok_type != TOK_LPAREN) {
	line_mol = rul__mol_make_int_atom (rul__atom_get_line_count ());
	rul__msg_print_w_atoms (API_REFDATERR, 2, lparen, line_mol);
	rul__mol_decr_uses (line_mol);
	break;
      }
      
      tok_type = rul__atom_get_atom (&rule_name);
      tok_type = rul__atom_get_atom (&rb_name);
      tok_type = rul__atom_get_atom (&icnt);
      status = rul___validate_rule (rb_name, rule_name,
				    eb_data->rb_name_count, eb_data->rb_names);
      if (status == TRUE) {
	id_count = rul__mol_int_atom_value (icnt);
	if (SL_ids_len < id_count) {
	  SM_ids = rul__mem_realloc (SM_ids, (sizeof (Molecule) * id_count));
	  SL_ids_len = id_count;
	}
	for (i = 0; i < id_count; i++) {
	  tok_type = rul__atom_get_atom (&id);
	  if (tok_type != TOK_INSTANCE_ID) {
	    line_mol = rul__mol_make_int_atom (rul__atom_get_line_count ());
	    rul__msg_print_w_atoms (API_REFERRID, 2, id, line_mol);
	    rul__mol_decr_uses (line_mol);
	    status = FALSE;
	    break;
	  }
	  SM_ids[i] = rul__translate_wme_id (id);
	  rul__mol_decr_uses (id);
	}
	if (status)
	  rul__ref_add_state_entry (eb_data->refraction_set, rule_name,
				    rb_name, id_count, SM_ids);
      }
      
      tok_type = rul__atom_get_atom (&rparen);
      
      if (status == FALSE) {
	rul__mol_decr_uses (rb_name);
	rul__mol_decr_uses (rule_name);
      }
      rul__mol_decr_uses (icnt);
      rul__mol_decr_uses (lparen);
      rul__mol_decr_uses (rparen);
    }
    rul__atom_restart (NULL);
    tok_type = rul__atom_get_atom (&icnt);
    if (tok_type == TOK_INTEGER_CONST) {
      rul__mol_set_gen_count (rul__mol_int_atom_value (icnt));
      rul__mol_decr_uses (icnt);
    }
  }

  rul__end_addstate_trans ();
}





/*************************
**			**
** RUL__SAVESTATE	**
**			**
**************************/

void
  rul__savestate (Mol_Symbol file_sym, Entry_Data eb_data)
{
  Mol_Symbol stream_sym;
  
  stream_sym = rul__mol_make_symbol ("$SAVESTATE-OUTPUT");
  SA_save_ios = rul__ios_open_file (stream_sym, file_sym, IOS__E_OUT);
  
  if (! SA_save_ios)
    return;

  /* output a header - "($VERSION 5)" */
  rul__ios_printf (SA_save_ios, SC_save_header);
  rul__ios_printf (SA_save_ios, "\n");

  /* save all the wmos */
  rul__wm_for_each_known_object (eb_data->db_name_count,
				 eb_data->db_names,
				 rul___save_print_wm);

  /* output a separator - "($REF-SET)" */
  rul__ios_printf (SA_save_ios, SC_save_refset);
  rul__ios_printf (SA_save_ios, "\n");

  /* output the refraction data */
  rul__ref_print_refset (SA_save_ios, eb_data->refraction_set);

  /* output end and close file */
  rul__ios_printf (SA_save_ios, SC_save_end);
  rul__ios_printf (SA_save_ios, "\n");
  rul__ios_printf (SA_save_ios, "%ld\n", rul__mol_get_gen_count ());
  rul__ios_close_file (stream_sym);
  rul__mol_decr_uses (stream_sym);
}



/*************************
**			**
** RUL___SAVE_PRINT_WM	**
**			**
**************************/

void
  rul___save_print_wm (Object wmo)
{
  rul__wm_print_saveform (wmo, SA_save_ios);
  rul__ios_printf (SA_save_ios, "\n");
}

static Boolean  rul___validate_rule (Molecule rb, Molecule rule,
				     long rb_count, Molecule *rb_names)
{
  long i, j, con_count;
  Molecule *constructs;
  Construct_Type *con_types;

  for (i = 0; i < rb_count; i++) {
    if (rb == rb_names[i]) {
      con_count = rul__rbs_constructs (rb_names[i], &constructs, &con_types);
      if (con_count) {
	for (j = 0; j < con_count; j++) {
	  if (rule == constructs[j] && con_types[j] == RUL__C_RULE)
	    return TRUE;
	}
	return FALSE;
      }
      else
	return TRUE;
    }
  }
  return FALSE;
}
 
