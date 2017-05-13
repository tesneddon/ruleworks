/* rts_dbg.c - RULEWORKS rts debugger */
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
****************************************************************************/


/*
**  FACILITY:
**	RULEWORKS Command Interpreter
**
**  ABSTRACT:
**	DBG API - Contains routines needed for debugging.
**      Used with rts_cmd_parser_tab.[ch].
**
**  MODIFIED BY:
**	DEC	Digital Equipment Corporation
**	CPQ	Compaq Computer Corporation
**
**  MODIFICATION HISTORY:
**
**	 1-Nov-1992	DEC	Major enhancements
**	 3-Nov-1994	DEC	Add space before ^ when printing wbreaks.
**	16-Feb-1998	DEC	class type changed to rclass
**	01-Dec-1999	CPQ	Release with GPL
*/

#include <ctype.h>	/* define toupper 				*/
#include <stdarg.h>
#include <common.h>	/* declares Molecule, Mol_Compound, Mol_Symbol  */
#include <cli_msg.h>	/* message defs				        */
#include <msg.h>	/* message routines                             */
#include <scan.h>	/* defines Token_Type and Token_Value & scanner	*/
#include <rbs.h>	/* declares Ruling blocks 			*/
#include <cs.h>		/* declares Conflict Set			*/
#include <ver_msg.h>	/* get prompt definition			*/
#include <wm.h>		/* declares Work Memory				*/
#include <mol.h>	/* declares molecules				*/
#include <decl.h>	/* declares declarations			*/
#include <decl_c_p.h>	/* declares class struct			*/
#include <list.h>	/* declares lists				*/
#include <ios.h>	/* io streams					*/
#include <rac.h>	/* declares entry_data struct                   */
#include <states.h>	/* declares *states funcs                       */
#include <dbg.h>	/* declares debugger (interp) info		*/
#include <dbg_tab.h>	/* declares scan tokens				*/
#include <dbg_p.h>	/* declares debugger private info		*/
#ifdef __VMS
#include <descrip.h>
#include <rmsdef.h>
#include <iodef.h>
#include <starlet.h>
#include <lib$routines.h>
#include <libclidef.h>
#endif

extern int rul__cmd_yyparse ();

extern Boolean    rul__scan_switch_file (IO_Stream);

EXTERNAL Mol_Symbol    rul__rac_active_rule_name;

/* debugger/interpreter */
static long rul__dbg_interp (Entry_Data eb_data);

#ifdef __VMS
static void rul__dbg_control_c_ast (void);
#endif

/*
 ** debugger (interpreter-rts) interface varaibles 
 */


static Pat_Pattern        *Dbg_pat;  /* pattern for ppwm  */
static long                Dbg_exit_value = EXIT_SUCCESS;
static Boolean             Dbg_init = FALSE;
static Boolean             Dbg_ebreak_exit = FALSE;
static Boolean             Dbg_exit_flag = FALSE;
static Boolean             Dbg_return_flag = FALSE;
static Boolean             Dbg_run_flag = FALSE;
static long                Dbg_changes = 0; /* 0 = no change, 1 = changes */
static Mol_Symbol          Dbg_action_name;
static Mol_Symbol          Dbg_name;
static long                Dbg_run_count = -1;
static long                Dbg_after_count = 0;
static Mol_Symbol          Dbg_catcher_name;
static Mol_Symbol	   Dbg_catcher_rb_name;
static Entry_Data          Dbg_ebdata;
static Conflict_Subset    *Dbg_css;
static Molecule           *Dbg_rbnames;
static long                Dbg_rbname_count;
static Molecule           *Dbg_dbnames;
static long                Dbg_dbname_count;
static Conflict_Set_Entry  Dbg_rbreak;
static Conflict_Set_Entry  Dbg_winner;
static Refraction_Set      Dbg_rs;
static Molecule           *Dbg_return;
static Param_List          Dbg_elist;
static Param_List          Dbg_rlist;
static Param_List          Dbg_wlist;
static long               *Dbg_cycle_ptr;

#ifdef __VMS
/* for control-c handling on vms... */
static int	    rul_gl_input_channel = 0;
static int	    rul_gl_ctrlc_code = IO$_SETMODE + IO$M_CTRLCAST;
static int	    rul_gq_iostat[2];
#endif

static void dbg_set_trace (Boolean on, Molecule mol);
static void dbg_init_trace_mols (void);
static Mol_Symbol SM_trace_off = NULL;
static Mol_Symbol SM_trace_on;
static Mol_Symbol SM_trace_star;
static Mol_Symbol SM_trace_entry_block;
static Mol_Symbol SM_trace_eb;
static Mol_Symbol SM_trace_rule_group;
static Mol_Symbol SM_trace_rg;
static Mol_Symbol SM_trace_r;
static Mol_Symbol SM_trace_wm;
static Mol_Symbol SM_trace_cs;

#define RBREAK_MSG_LEN 1000

/**********************************************************************/


void rul__debug_init ()
{
  IO_Stream ios;

  if (!Dbg_init) {
    /*
    **  If the debugger has never been initialized, do so.
    */

    rul__ios_init();
    rul__dbg_ga_interp = rul__dbg_interp;

#ifdef __VMS
    if (rul__smg_init()) {
        Mol_Symbol stream_name;

        stream_name = rul__mol_make_symbol ("SMG-INPUT");
	rul__mol_mark_perm (stream_name);
        ios = rul__ios_open_bizzare (stream_name, 0, 0, 0,
				   (Gets_Function) rul__smg_input,
				   RUL__C_PRODUCT_PROMPT, 0);
    }
    else
      ios = RUL__C_STD_IN;
#else
    ios = RUL__C_STD_IN;
#endif
    rul__ios_set_source_stream (ios);

    /* Defaults: */
    Dbg_action_name = rul__mol_make_symbol (RUL__C_MSG_PREFIX);
    rul__mol_mark_perm (Dbg_action_name);
    
    /* set prompt */
    Dbg_name = rul__mol_make_symbol (RUL__C_PRODUCT_PROMPT);
    rul__mol_mark_perm (Dbg_name);
    rul__scan_set_prompt (Dbg_name);

    /* set a control-c handler... */
/*    rul__dbg_enable_control_y (); */

    /* set break on all entry-blocks if none have been set */
    if (rul__dbg_gl_ebreak == 0)
      rul__dbg_gl_ebreak |= DBG_M_EBREAK_ALL;

    /* enable wmhistory - default is OFF */
/*    rul__dbg_gl_enable |= DBG_M_ENABLE_WMH; */

    /* and don't forget to enable DEBUG and TRACE*/
    rul__dbg_gl_enable |= DBG_M_ENABLE_DBG;
    rul__dbg_gl_enable |= DBG_M_ENABLE_TRA;

    Dbg_init = TRUE;

    /*  set for break after initialization  */
    rul__dbg_gl_break |= DBG_M_BREAK;
  }
}


void rul__debug (void)
{
  /*  First, make sure that the debugger has been initialized */
  rul__debug_init ();  

  /*  Set the break reason to the API/DEBUG-action value  */
  rul__dbg_gl_break |= DBG_M_BREAK_API;
}


void rul__trace (long arg_cnt, ...)
{
   va_list   ap;
   Molecule  mol, out_mol, cmol;
   long      i, j, k, len;
   Boolean   on = FALSE;

   if ((rul__dbg_gl_enable & DBG_M_ENABLE_TRA) == 0)
     return;

   dbg_init_trace_mols ();

   if (!Dbg_init) {
      rul__debug ();
      rul__dbg_gl_break = 0;
      rul__dbg_gl_ebreak = 0;
      rul__dbg_gl_enable = DBG_M_ENABLE_TRA;
   }

   va_start (ap, arg_cnt);

   mol = va_arg (ap, Molecule);
   if (mol == SM_trace_on)
     on = TRUE;

   for (i = 1; i < arg_cnt; i++) {
      mol = va_arg (ap, Molecule);
      
      assert (rul__mol_is_valid (mol));
      
      if (rul__mol_is_atom (mol))
	dbg_set_trace (on, mol);
      else {
	 k = rul__mol_get_poly_count (mol);
	 for (j = 1; j <= k; j++) {
	    cmol = rul__mol_get_comp_nth (mol, j);
	    dbg_set_trace (on, cmol);
	 }
      }
   }
   va_end (ap);
}


Entry_Data rul__dbg_get_ebdata (void)
{
  return Dbg_ebdata;
}

static long rul__dbg_interp (Entry_Data eb_data)
{
  long      status;
  long      j;
  List      list;
  char      buf[RUL_C_MAX_SYMBOL_SIZE+1] = "";

  Dbg_ebdata = eb_data;
  Dbg_rbnames = eb_data->rb_names;
  Dbg_rbname_count = eb_data->rb_name_count;
  Dbg_dbnames = eb_data->db_names;
  Dbg_dbname_count = eb_data->db_name_count;
  Dbg_winner = *eb_data->winning_cse_ptr;
  Dbg_css = eb_data->conflict_subset_array;
  Dbg_rs = eb_data->refraction_set;
  Dbg_return = eb_data->return_value_ptr;
  Dbg_cycle_ptr = eb_data->cycle_count_ptr;

  Dbg_ebreak_exit = FALSE;
  Dbg_run_flag    = FALSE;
  Dbg_changes     = 0;
  Dbg_exit_flag   = FALSE;
  Dbg_return_flag = FALSE;

  rul__msg_set_in_debugger (TRUE);

  if (rul__dbg_gl_enable & DBG_M_ENABLE_BKN) {
     if (Dbg_rbname_count && Dbg_rbnames[0]) {
	rul__mol_use_readform (Dbg_rbnames[0], buf,
			       RUL_C_MAX_SYMBOL_SIZE+1);
	strcat (buf, "~");
     }
  }

  if (rul__dbg_gl_break & DBG_M_BREAK_EXIT)
    Dbg_ebreak_exit = TRUE;

  Dbg_rbreak = NULL;
  rul__rac_active_rule_name = Dbg_action_name;
  eb_data->active_rb_name = NULL;

  if (rul__dbg_gl_trace & DBG_M_TRACE_RG) {
    rul__dbg_ga_group = rul__dbg_get_group (Dbg_winner);
  }

  if (rul__dbg_gl_break) {

    if (rul__dbg_gl_break & DBG_M_BREAK_RULE)
      Dbg_rbreak = Dbg_winner;

    if (rul__dbg_gl_break & DBG_M_BREAK_WMO)
      rul__msg_print (CLI_BREAKNOTED);
    else if (rul__dbg_gl_break & DBG_M_BREAK_RULE)
      rul__msg_print (CLI_BREAKNOTED);
    else if (rul__dbg_gl_break & DBG_M_BREAK_ENTRY) {
      rul__mol_use_readform (Dbg_rbnames[0], buf, RUL_C_MAX_SYMBOL_SIZE+1);
      rul__msg_print (CLI_EBREAK, "ON-ENTRY", buf);
    } else if (rul__dbg_gl_break & DBG_M_BREAK_EXIT) /* before empty */ {
      rul__mol_use_readform (Dbg_rbnames[0], buf, RUL_C_MAX_SYMBOL_SIZE+1);
      rul__msg_print (CLI_EBREAK, "ON-EXIT", buf);
    }
#if 0
    else if (rul__dbg_gl_break & DBG_M_BREAK_EVERY)
      rul__msg_print (CLI_EBREAK, buf, "ON-EVERY");
    else if (rul__dbg_gl_break & DBG_M_BREAK_EMPTY)
      rul__msg_print (CLI_EBREAK, buf, "ON-EMPTY");
#endif
    else if (rul__dbg_gl_break & DBG_M_BREAK_RUN) {
      rul__msg_print (CLI_PAUSE);
    } else if (rul__dbg_gl_break & DBG_M_BREAK_API) {
      rul__msg_print (CLI_DEBPAUSE);
    }
#ifndef NDEBUG
/*?*    else
      rul__ios_printf (RUL__C_STD_ERR,
		"\nINTERNAL ERROR:  Paused for no apparent reason = %d\n",
		rul__dbg_gl_break);
*?*/
#endif

    rul__dbg_gl_break = 0;

    /*
     * reset run count on break
     */
    if (*eb_data->run_count_ptr == -1)
       *eb_data->run_count_ptr = 0;

    /*
     * Main loop here...
     */
    do {
      status = rul__cmd_yyparse ();
    }
    while (!Dbg_exit_flag && !Dbg_run_flag &&
	   !Dbg_return_flag && !Dbg_changes);
  }

  if (Dbg_exit_flag)
    exit (Dbg_exit_value);
  
  /* set run count if a run command was given */
  if (Dbg_run_flag)
    *eb_data->run_count_ptr = rul__rac_rule_firing_cycle + Dbg_run_count;

  if (Dbg_after_count) {
    *eb_data->after_count_ptr = *eb_data->cycle_count_ptr + Dbg_after_count;
    *eb_data->catcher_name_ptr = Dbg_catcher_name;
    *eb_data->catcher_rb_name_ptr = Dbg_catcher_rb_name;
    Dbg_after_count = 0;
  }

  if (Dbg_changes)
    rul__dbg_gl_break |= DBG_M_BREAK;

  if (Dbg_elist == NULL)
    rul__dbg_ga_ebreak = NULL;
  else
    rul__dbg_ga_ebreak = rul__dbg_match_ebreak;

  if (Dbg_wlist == NULL)
    rul__dbg_ga_wbreak = NULL;
  else
    rul__dbg_ga_wbreak = rul__dbg_match_wbreak;
  
  if (Dbg_rlist == NULL)
    rul__dbg_ga_rbreak = NULL;
  else
    rul__dbg_ga_rbreak = rul__dbg_match_rbreak;
  
  rul__msg_set_in_debugger (FALSE);

  if (Dbg_return_flag)
    return 2;

  return (Dbg_changes);
}

void rul__dbg_print (Object wmo)
{
  rul__ios_printf (RUL__C_STD_ERR, "   ");
  rul__wm_print_readform (wmo, RUL__C_STD_ERR);
  rul__ios_printf (RUL__C_STD_ERR, "\n");
}

void rul__dbg_set_pp_pat (Pat_Pattern *pat)
{
  Dbg_pat = pat;
}

static void dbg_print_pred_test (IO_Stream ios, long pred)
{

  switch (pred & 65535) {
  case TOK_EQUAL_EQUAL:
   /* rul__ios_printf (ios, "== "); */
    break;
  case TOK_APPROX_EQUAL:
    rul__ios_printf (ios, "~= ");
    break;
  case TOK_NOT_APPROX_EQUAL:
    rul__ios_printf (ios, "-~= ");
    break;
  case TOK_EQUAL:
    rul__ios_printf (ios, "= ");
    break;
  case TOK_NOT_EQ:
    rul__ios_printf (ios, "<> ");
    break;
  case TOK_NOT_EQUAL:
    rul__ios_printf (ios, "-= ");
    break;
  case TOK_GREATER:
    rul__ios_printf (ios, "> ");
    break;
  case TOK_GREATER_EQUAL:
    rul__ios_printf (ios, ">= ");
    break;
  case TOK_LESS:
    rul__ios_printf (ios, "< ");
    break;
  case TOK_LESS_EQUAL:
    rul__ios_printf (ios, "<= ");
    break;
    
  case TOK_SAME_TYPE:
    rul__ios_printf (ios, "<=> ");
    break;
  case TOK_DIFF_TYPE:
    rul__ios_printf (ios, "<-> ");
    break;
    
  case TOK_LENGTH_LESS:
    rul__ios_printf (ios, "[<] ");
    break;
  case TOK_LENGTH_LESS_EQUAL:
    rul__ios_printf (ios, "[<=] ");
    break;
  case TOK_LENGTH_EQUAL:
    rul__ios_printf (ios, "[=] ");
    break;
  case TOK_LENGTH_GREATER_EQUAL:
    rul__ios_printf (ios, "[>=] ");
    break;
  case TOK_LENGTH_GREATER:
    rul__ios_printf (ios, "[>] ");
    break;
    
  case TOK_CONTAINS:
    rul__ios_printf (ios, "[+] ");
    break;
  case TOK_DOES_NOT_CONTAIN:
    rul__ios_printf (ios, "[-] ");
    break;

  default:
    rul__ios_printf (ios, "? ");
    break;
  }

  pred = pred >> 16;
  if (pred) {
    switch (pred & 65535) {
    case TOK_EQUAL_EQUAL:
    /*  rul__ios_printf (ios, "== "); */
      break;
    case TOK_APPROX_EQUAL:
      rul__ios_printf (ios, "~= ");
      break;
    case TOK_NOT_APPROX_EQUAL:
      rul__ios_printf (ios, "-~= ");
      break;
    case TOK_EQUAL:
      rul__ios_printf (ios, "= ");
      break;
    case TOK_NOT_EQ:
      rul__ios_printf (ios, "<> ");
      break;
    case TOK_NOT_EQUAL:
      rul__ios_printf (ios, "-= ");
      break;
    case TOK_GREATER:
      rul__ios_printf (ios, "> ");
      break;
    case TOK_GREATER_EQUAL:
      rul__ios_printf (ios, ">= ");
      break;
    case TOK_LESS:
      rul__ios_printf (ios, "< ");
      break;
    case TOK_LESS_EQUAL:
      rul__ios_printf (ios, "<= ");
      break;
      
    case TOK_SAME_TYPE:
      rul__ios_printf (ios, "<=> ");
      break;
    case TOK_DIFF_TYPE:
      rul__ios_printf (ios, "<-> ");
      break;
      
    case TOK_CONTAINS:
      rul__ios_printf (ios, "[+] ");
      break;
    case TOK_DOES_NOT_CONTAIN:
      rul__ios_printf (ios, "[-] ");
      break;
    }
  }
}

static Boolean dbg_get_match_results (Molecule v1, long pred, Molecule v2)
{
  Boolean        match = FALSE;
  long		 opred, tpred;

  opred = pred & 65535;
  pred = pred >> 16;
  if ((opred == TOK_CONTAINS || opred == TOK_DOES_NOT_CONTAIN) &&
      pred != 0) {
    tpred = opred;
    opred = pred;
    pred = tpred;
  }

  switch (opred) {
  case TOK_EQUAL_EQUAL:	/* ident equal */
    if (pred == TOK_CONTAINS)
      match = rul__mol_contains_pred (v1, v2, rul__mol_eq_pred);
    else if  (pred == TOK_DOES_NOT_CONTAIN)
      match = rul__mol_not_contains_pred (v1, v2, rul__mol_eq_pred);
    else
      match = (v1 == v2);
    break;
  case TOK_APPROX_EQUAL:
    if (pred == TOK_CONTAINS)
      match = rul__mol_contains_pred (v1, v2, rul__mol_approx_eq_pred);
    else if  (pred == TOK_DOES_NOT_CONTAIN)
      match = rul__mol_not_contains_pred (v1, v2, rul__mol_approx_eq_pred);
    else
      match = rul__mol_approx_eq_pred (v1, v2);
    break;
  case TOK_NOT_APPROX_EQUAL:
    if (pred == TOK_CONTAINS)
      match = rul__mol_contains_pred (v1, v2, rul__mol_not_approx_eq_pred);
    else if  (pred == TOK_DOES_NOT_CONTAIN)
      match = rul__mol_not_contains_pred (v1, v2, rul__mol_not_approx_eq_pred);
    else
      match = rul__mol_not_approx_eq_pred (v1, v2);
    break;
  case TOK_EQUAL:
    if (pred == TOK_CONTAINS)
      match = rul__mol_contains_pred (v1, v2, rul__mol_equal_pred);
    else if  (pred == TOK_DOES_NOT_CONTAIN)
      match = rul__mol_not_contains_pred (v1, v2, rul__mol_equal_pred);
    else
      match = rul__mol_equal_pred (v1, v2);
    break;
  case TOK_NOT_EQ:
    if (pred == TOK_CONTAINS)
      match = rul__mol_contains_pred (v1, v2, rul__mol_not_eq_pred);
    else if  (pred == TOK_DOES_NOT_CONTAIN)
      match = rul__mol_not_contains_pred (v1, v2, rul__mol_not_eq_pred);
    else
      match = !(v1 == v2);
    break;
  case TOK_NOT_EQUAL:
    if (pred == TOK_CONTAINS)
      match = rul__mol_contains_pred (v1, v2, rul__mol_not_equal_pred);
    else if  (pred == TOK_DOES_NOT_CONTAIN)
      match = rul__mol_not_contains_pred (v1, v2, rul__mol_not_equal_pred);
    else
      match = !rul__mol_equal_pred (v1, v2);
    break;
  case TOK_GREATER:
    if (pred == TOK_CONTAINS)
      match = rul__mol_contains_pred (v1, v2, rul__mol_gt_pred);
    else if  (pred == TOK_DOES_NOT_CONTAIN)
      match = rul__mol_not_contains_pred (v1, v2, rul__mol_gt_pred);
    else
      match = rul__mol_gt_pred (v1, v2);
    break;
  case TOK_GREATER_EQUAL:
    if (pred == TOK_CONTAINS)
      match = rul__mol_contains_pred (v1, v2, rul__mol_gte_pred);
    else if  (pred == TOK_DOES_NOT_CONTAIN)
      match = rul__mol_not_contains_pred (v1, v2, rul__mol_gte_pred);
    else
      match = rul__mol_gte_pred (v1, v2);
    break;
  case TOK_LESS:
    if (pred == TOK_CONTAINS)
      match = rul__mol_contains_pred (v1, v2, rul__mol_lt_pred);
    else if  (pred == TOK_DOES_NOT_CONTAIN)
      match = rul__mol_not_contains_pred (v1, v2, rul__mol_lt_pred);
    else
      match = rul__mol_lt_pred (v1, v2);
    break;
  case TOK_LESS_EQUAL:
    if (pred == TOK_CONTAINS)
      match = rul__mol_contains_pred (v1, v2, rul__mol_lte_pred);
    else if  (pred == TOK_DOES_NOT_CONTAIN)
      match = rul__mol_not_contains_pred (v1, v2, rul__mol_lte_pred);
    else
      match = rul__mol_lte_pred (v1, v2);
    break;
    
  case TOK_SAME_TYPE:
    if (pred == TOK_CONTAINS)
      match = rul__mol_contains_pred (v1, v2, rul__mol_same_type_pred);
    else if  (pred == TOK_DOES_NOT_CONTAIN)
      match = rul__mol_not_contains_pred (v1, v2, rul__mol_same_type_pred);
    else
      match = rul__mol_same_type_pred (v1, v2);
    break;
  case TOK_DIFF_TYPE:
    if (pred == TOK_CONTAINS)
      match = rul__mol_contains_pred (v1, v2, rul__mol_diff_type_pred);
    else if  (pred == TOK_DOES_NOT_CONTAIN)
      match = rul__mol_not_contains_pred (v1, v2, rul__mol_diff_type_pred);
    else
      match = rul__mol_diff_type_pred (v1, v2);
    break;
    
  case TOK_CONTAINS:
    match = rul__mol_contains_pred (v1, v2, NULL);
    break;
  case TOK_DOES_NOT_CONTAIN:
    match = rul__mol_not_contains_pred (v1, v2, NULL);
    break;
    
  case TOK_LENGTH_LESS:
    match = rul__mol_len_lt_pred (v1, v2);
    break;
  case TOK_LENGTH_LESS_EQUAL:
    match = rul__mol_len_lte_pred (v1, v2);
    break;
  case TOK_LENGTH_EQUAL:
    match = rul__mol_len_eq_pred (v1, v2);
    break;
  case TOK_LENGTH_GREATER_EQUAL:
    match = rul__mol_len_gte_pred (v1, v2);
    break;
  case TOK_LENGTH_GREATER:
    match = rul__mol_len_gt_pred (v1, v2);
    break;
  }
  return match;
}

void rul__dbg_pp_print (Object wmo)
{
  Boolean match = TRUE;
  Pat_Classes   *pc;
  Pat_Attrs     *pa;
  Pat_Tests     *pt, *pdt;
  Molecule       att_val, tst_val;
  long		 pred, opred, tpred;
  long          idx;

  if (Dbg_pat != NULL) {

    for (pc = Dbg_pat->pc; pc && match == TRUE; pc = pc->next) {

      if (!(rul__decl_is_subclass_of (rul__wm_get_class (wmo), pc->cls))) {
	match = FALSE;
      }
      else {

	for (pa = pc->pa; pa && match == TRUE; pa = pa->next) {
	  att_val = rul__wm_get_attr_val (wmo, pa->attr);
	  if (pa->idx) {
	    idx = pa->idx;
	    if (idx == -1)
	      idx = rul__mol_get_poly_count (att_val);
	    if (idx <= rul__mol_get_poly_count (att_val))
	      att_val = rul__mol_get_comp_nth (att_val, idx);
	    else {
	      match = FALSE;
	      break;
	    }
	  }

	  pt = pa->pt;
	  tst_val = pt->u.value;
	  pred = pt->pred;
	  
	  if (pt->ttype == PAT_DISJ) {
	    pdt = pt->u.pt;
	    tst_val = pdt->u.value;
	    pred = pdt->pred;
	  }
	  else {
	    if (pt->ttype == PAT_CONJ) {
	      pt = pt->next;
	      if (pt->ttype == PAT_DISJ) {
		pdt = pt->u.pt;
		tst_val = pdt->u.value;
		pred = pdt->pred;
	      }
	      else {
		tst_val = pt->u.value;
		pred = pt->pred;
	      }
	    }
	  }
	      
	  while (pt) {
	    
	    match = dbg_get_match_results (att_val, pred, tst_val);
	    
	    if (pt->ttype == PAT_DISJ && match == FALSE && pdt->next) {
	      pdt = pdt->next;
	      tst_val = pdt->u.value;
	      pred = pdt->pred;
	    }
	    else if (match == FALSE) {
	      break;
	    }
	    else if (pt->next) {
	      pt = pt->next;
	      if (pt->ttype == PAT_DISJ) {
		pdt = pt->u.pt;
		tst_val = pdt->u.value;
		pred = pdt->pred;
	      }
	      else {
		tst_val = pt->u.value;
		pred = pt->pred;
	      }
	    }
	    else
	      break;
	  }
	}
      }
      if (pc->ctype == PAT_CE_NEG) {
	if (match == FALSE)
	  match = TRUE;
	else
	  match = FALSE;
      }
    }
  }
  
  if (match) {
    rul__ios_printf (RUL__C_STD_ERR, "   ");
    rul__wm_print_readform (wmo, RUL__C_STD_ERR);
    rul__ios_printf (RUL__C_STD_ERR, "\n");
  }
}

void rul__dbg_remove_class_wmes (Class cls)
{
  if (rul__decl_get_class_name (cls) == rul__mol_symbol_root ()) {
    rul__wm_destroy_all_class_var (rul__mol_symbol_root (), Dbg_ebdata);
  }
  else
    rul__wm_destroy_and_notify_all (cls, Dbg_ebdata);
}


void rul__dbg_show_ebreaks (void)
{
   Param_List pl = Dbg_elist;
   long       i = 1;

   if ((rul__dbg_gl_ebreak & DBG_M_EBREAK_ALL) ||
       (pl != NULL)) {
      rul__ios_printf(RUL__C_STD_ERR, CLI_STR_EBREAK_HEADER);
      if (pl == NULL) {
	 rul__ios_printf (RUL__C_STD_ERR, CLI_STR_EBREAK_ALL);
      }
      else {
	 while (pl != NULL) {
	    rul__ios_printf (RUL__C_STD_ERR, "\n   %-3d", i++);
	    rul__mol_print_readform ((Molecule) pl->data, RUL__C_STD_ERR);
	    pl = pl->next;
	 }
      }
   }
   else
     rul__ios_printf(RUL__C_STD_ERR, CLI_STR_EBREAK_NONE);

   rul__ios_printf(RUL__C_STD_ERR, "\n"); 
}


void rul__dbg_free_param_list (Param_List pl)
{
   Param_List n;
   RuleName   rn;

   if (pl != (void *) -1) {
      while (pl) {
	 n = pl->next;
	 if (pl->type == DBG__E_PL_MOL) {
	    rul__mol_decr_uses ((Molecule) pl->data);
	 }
	 else if (pl->type == DBG__E_PL_RUL) {
	    rn = (RuleName) pl->data;
	    rul__mol_decr_uses (rn->rb_name);
	    rul__mol_decr_uses (rn->rule_name);
	    rul__mem_free (pl->data);
	 }
	 else if (pl->type == DBG__E_PL_PAT) {
	    rul__dbg_free_pattern ((Pat_Pattern **) &pl->data);
	 }
	 rul__mem_free (pl);
	 pl = n;
      }
   }
}

static Boolean dbg_match_pattern (Pat_Pattern *p1, Pat_Pattern *p2)
{
   Pat_Classes   *pc, *pc2;
   Pat_Attrs     *pa, *pa2;
   Pat_Tests     *pt, *pt2;
   Pat_Tests     *pdt, *pdt2;
   Molecule       vl1, vl2;
   long           pred, prd2;

   if (p1->ptype != p2->ptype)
     return FALSE;

   for (pc = p1->pc, pc2 = p2->pc;
	pc && pc2;
	pc = pc->next, pc2 = pc2->next) {
	  
      if ((pc->cls != pc2->cls) || (pc->ctype != pc2->ctype))
	return FALSE;
	 
      for (pa = pc->pa, pa2 = pc2->pa;
	   pa && pa2;
	   pa = pa->next, pa2 = pa2->next) {
	    
	 if ((pa->attr != pa2->attr) || (pa->idx != pa2->idx))
	   return FALSE;
		  
	 for (pt = pa->pt, pt2 = pa2->pt;
	      pt && pt2;
	      pt = pt->next, pt2 = pt2->next) {

	    if (pt->ttype != pt2->ttype)
	      return FALSE;

	    if (pt->ttype != PAT_DISJ) {
	       if (pt->u.value != pt2->u.value ||
		   pt->pred != pt2->pred)
		 return FALSE;
	    }
	    else {
	       for (pdt = pt->u.pt, pdt2 = pt2->u.pt;
		    pdt && pdt2;
		    pdt = pdt->next, pdt2 = pdt2->next) {
		  
		  if (pdt->ttype != pdt2->ttype ||
		      pdt->u.value != pdt2->u.value ||
		      pdt->pred != pdt2->pred)
		    return FALSE;
	       }
	       if (pdt || pdt2)
		 return FALSE;
	    }
	 }
	 if (pt || pt2)
	   return FALSE;
      }
      if (pa || pa2)
	return FALSE;
   }
   if (pc || pc2)
     return FALSE;

   return TRUE;
}

static Param_List dbg_is_in_list (Param_List pl, Param_Type type,
				  void *data, Param_List *prev,
				  Boolean match_with_no_rb)
{
   RuleName   rn, rn1;

   if (prev)
     *prev = NULL;
   while (pl) {
      if (pl->type == type) {
	 if (type == DBG__E_PL_RUL) {
	    rn = (RuleName) data;
	    rn1 = (RuleName) pl->data;
	    if (rn->rb_name == rn1->rb_name &&
		rn->rule_name == rn1->rule_name)
	      return pl;
	    if (match_with_no_rb && rn->rb_name == NULL &&
		rn->rule_name == rn1->rule_name)
	      return pl;	
	 }
	 else if (type == DBG__E_PL_PAT) {
	    if (dbg_match_pattern ((Pat_Pattern*) pl->data,
				   (Pat_Pattern*) data))
	      return pl;
	 }
	 else {
	    if (pl->data == data)
	      return pl;
	 }
      }
      if (prev)
	*prev = pl;
      pl = pl->next;
   }
   return NULL;
}

static Pat_Pattern *dbg_copy_pattern (Pat_Pattern *pat)
{
   Pat_Pattern   *p = NULL;
   Pat_Classes   *pc, **pcn;
   Pat_Attrs     *pa, **pan;
   Pat_Tests     *pt, **ptn, *pdt, **pdtn;

   if (pat) {
      p = RUL_MEM_ALLOCATE (Pat_Pattern, 1);
      p->ptype = pat->ptype;
      pcn = &(p->pc);
      
      for (pc = pat->pc; pc; pc = pc->next) {
	 
	 *pcn = RUL_MEM_ALLOCATE (Pat_Classes, 1);
	 (*pcn)->cls   = pc->cls;
	 (*pcn)->ctype = pc->ctype;
	 pan = &((*pcn)->pa);
	 pcn = &((*pcn)->next);
	 
	 for (pa = pc->pa; pa; pa = pa->next) {

	    *pan = RUL_MEM_ALLOCATE (Pat_Attrs, 1);
	    (*pan)->attr = pa->attr;
	    (*pan)->idx  = pa->idx;
	    ptn = &((*pan)->pt);
	    pan = &((*pan)->next);

	    for (pt = pa->pt; pt; pt = pt->next) {
	       
	       *ptn = RUL_MEM_ALLOCATE (Pat_Tests, 1);
	       (*ptn)->ttype    = pt->ttype;
	       (*ptn)->pred     = pt->pred;
	       (*ptn)->u.value  = pt->u.value;
	       if (pt->ttype != PAT_DISJ)
		 rul__mol_incr_uses (pt->u.value);
	       pdtn = &((*ptn)->u.pt);
	       ptn = &((*ptn)->next);

	       if (pt->ttype == PAT_DISJ) {

		  for (pdt = pt->u.pt; pdt; pdt = pdt->next) {
		     
		     *pdtn = RUL_MEM_ALLOCATE (Pat_Tests, 1);
		     (*pdtn)->ttype    = pdt->ttype;
		     (*pdtn)->pred     = pdt->pred;
		     (*pdtn)->u.value  = pdt->u.value;
		     rul__mol_incr_uses (pdt->u.value);
		     pdtn = &((*pdtn)->next);
		  }
	       }
	    }
	 }
      }
   }
   return p;
}

void * dbg_copy_data (Param_Type type, void *data)
{
   RuleName rn;
   Pat_Pattern *p;

   if (type == DBG__E_PL_RUL) {
      rn = rul__mem_malloc (sizeof (struct rulename));
      rn->rb_name = ((RuleName) data)->rb_name;
      rul__mol_incr_uses (rn->rb_name);
      rn->rule_name = ((RuleName) data)->rule_name;
      rul__mol_incr_uses (rn->rule_name);
      return rn;
   }
   else if (type == DBG__E_PL_PAT) {
      p = dbg_copy_pattern ((Pat_Pattern*) data);
      return p;
   }
   else if (type == DBG__E_PL_MOL) {
      rul__mol_incr_uses ((Molecule) data);
   }
   
   return data;
}

static void dbg_copy_data_to_list (Param_List *pl, Param_Type type, void *data)
{
   Param_List n, p = NULL;
   
   if (dbg_is_in_list (*pl, type, data, &p, FALSE) == NULL) {
      n = rul__mem_malloc (sizeof (struct _pl));
      n->next = NULL;
      n->type = type;
      n->data = dbg_copy_data (type, data);
      if (p)
	p->next = n;
      else
	*pl = n;
   }
}

void rul__dbg_add_data_to_list (Param_List *pl, Param_Type type, void *data)
{
   Param_List n, p = NULL;
   
   if (dbg_is_in_list (*pl, type, data, &p, FALSE) == NULL) {
      n = rul__mem_malloc (sizeof (struct _pl));
      n->next = NULL;
      n->type = type;
      n->data = data;
      if (p)
	p->next = n;
      else
	*pl = n;
   }
}

void rul__dbg_ebreak (Boolean on, Param_List plist)
{
   Param_List    p, p1, p2, p3, p4;
   long		 i;

   if (on) {
      if (plist) {  	/* add to list */
	 rul__dbg_gl_ebreak = DBG_M_EBREAK_NAME;
	 p = plist;
	 while (p) {
	    dbg_copy_data_to_list (&Dbg_elist, p->type, p->data);
	    p = p->next;
	 }
      }
      else {		/* all entry-blocks on */
	 rul__dbg_gl_ebreak = DBG_M_EBREAK_ALL;
	 rul__dbg_free_param_list (Dbg_elist);
	 Dbg_elist = NULL;
      }
   }
   else {		/* remove from list */
      if (plist) {
	 if (Dbg_elist) {
	    p = plist;
	    p1 = NULL;
	    while (p) {
	       if (p->type == DBG__E_PL_MOL) {
		  dbg_copy_data_to_list (&p1, p->type, p->data);
	       }
	       else {
		  p2 = Dbg_elist;
		  for (i = 1; i < (long) p->data && p2 != NULL; i++) {
		     p2 = p2->next;
		  }
		  if (p2) {
		     dbg_copy_data_to_list (&p1, p2->type, p2->data);
		  }
	       }
	       p = p->next;
	    }
	    p = p1;
	    while (p1 && Dbg_elist) {
	       p2 = dbg_is_in_list (Dbg_elist, p1->type, p1->data, &p3, FALSE);
	       if (p2) {
		  if (p3)
		    p3->next = p2->next;
		  else
		    Dbg_elist = p2->next;
		  rul__mol_decr_uses ((Molecule) p2->data);
		  rul__mem_free (p2);
	       }
	       p1 = p1->next;
	    }
	    if (p)
	      rul__dbg_free_param_list (p);
	 }
      }
      else {		/* all off */
	 rul__dbg_gl_ebreak = 0;
	 rul__dbg_free_param_list (Dbg_elist);
	 Dbg_elist = NULL;
      }
   }
}

Boolean rul__dbg_match_ebreak (Mol_Symbol ebname)
{
   Param_List    p = Dbg_elist;
   
   while (p) {
      if (ebname == (Molecule) p->data) {
	 return TRUE;
      }
      p = p->next;
   }
   return FALSE;
}

void rul__dbg_show_wbreaks (void)
{
   Pat_Pattern   *pat;
   Pat_Classes   *pc;
   Pat_Attrs     *pa;
   Pat_Tests     *pt, *pdt;
   long		  pred;
   Molecule       tst_val;
   IO_Stream      ios = RUL__C_STD_ERR;
   Boolean        conj, need_space;
   Param_List     p = Dbg_wlist;
   long           i = 1;

   if (p) {

      rul__ios_printf (ios, CLI_STR_WBREAK_HEADER);
      while (p) {

	 pat = (Pat_Pattern*) p->data;
	 for (pc = pat->pc; pc; pc = pc->next) {

	    rul__ios_printf (ios, "   %-3d", i++);

	    if (pc->ctype == PAT_CE_NEG)
	      rul__ios_printf (ios, "-(");
	    else
	      rul__ios_printf (ios, "(");

	    rul__mol_print_readform (rul__decl_get_class_name (pc->cls), ios);
	    need_space = TRUE;

	    for (pa = pc->pa; pa; pa = pa->next) {

	       if (need_space)
		 rul__ios_printf (ios, " ");
	       rul__ios_printf (ios, "^");
	       rul__mol_print_readform (pa->attr, ios);
	       
	       if (pa->idx) {
		  if (pa->idx == -1)
		    rul__ios_printf (ios, "[$LAST]");
		  else
		    rul__ios_printf (ios, "[%ld]", pa->idx);
	       }
	       rul__ios_printf (ios, " ");
	       need_space = FALSE;

	       conj = FALSE;
	       pt = pa->pt;
	       tst_val = pt->u.value;
	       pred = pt->pred;
	       
	       if (pt->ttype == PAT_DISJ) {
		  rul__ios_printf (ios, "<< ");
		  pdt = pt->u.pt;
		  tst_val = pdt->u.value;
		  pred = pdt->pred;
	       }
	       else if (pt->ttype == PAT_CONJ) {
		  rul__ios_printf (ios, "{ ");
		  conj = TRUE;
		  pt = pt->next;
		  if (pt->ttype == PAT_DISJ) {
		     rul__ios_printf (ios, "<< ");
		     pdt = pt->u.pt;
		     tst_val = pdt->u.value;
		     pred = pdt->pred;
		  }
		  else {
		     tst_val = pt->u.value;
		     pred = pt->pred;
		  }
	       }
	       
	       while (pt) {
		  
		  if (need_space)
		    rul__ios_printf (ios, " ");
		  dbg_print_pred_test (ios, pred);
		  rul__mol_print_readform (tst_val, ios);
		  
		  if ((pt->ttype == PAT_DISJ) && pdt->next) {
		     pdt = pdt->next;
		     tst_val = pdt->u.value;
		     pred = pdt->pred;
		     need_space= TRUE;
		  }
		  else {
		     if (pt->ttype == PAT_DISJ)
		       rul__ios_printf (ios, " >>");
		     
		     if (pt->next) {
			need_space = TRUE;
			pt = pt->next;
			if (pt->ttype == PAT_DISJ) {
			   rul__ios_printf (ios, " <<");
			   pdt = pt->u.pt;
			   tst_val = pdt->u.value;
			   pred = pdt->pred;
			}
			else {
			   if (pt->ttype == PAT_CONJ) {
			      rul__ios_printf (ios, " {");
			      conj = TRUE;
			      pt = pt->next;
			   }
			   tst_val = pt->u.value;
			   pred = pt->pred;
			}
		     }
		     else {
			if (conj == TRUE)
			  rul__ios_printf (ios, " }");
			pt = pt->next;
		     }
		  }
	       }
	       need_space = TRUE;
	    }
	 }
	 rul__ios_printf (ios, ")\n");
	 p = p->next;
      }
   }
   else {
     rul__ios_printf (ios, CLI_STR_WBREAK_NONE);
   }
}

void rul__dbg_wbreak (Boolean on, Param_List plist)
{
   Param_List    p, p1, p2, p3;
   long		 i;
   RuleName      rn;
   Molecule      rb = NULL, rule = NULL;
   char          buf[RUL_C_MAX_SYMBOL_SIZE+1];
   long          k;
   Boolean       list_valid = TRUE;

   if (on) {
      p = plist;
      while (p) {
	 dbg_copy_data_to_list (&Dbg_wlist, p->type, p->data);
	 p = p->next;
      }
   }
   else {
      if (plist) {
	 if (Dbg_wlist) {
	    p = plist;
	    p1 = NULL;
	    while (p) {
	       if (p->type == DBG__E_PL_PAT) {
		  dbg_copy_data_to_list (&p1, p->type, p->data);
	       }
	       else {
		  p2 = Dbg_wlist;
		  for (i = 1; i < (long) p->data && p2 != NULL; i++) {
		     p2 = p2->next;
		  }
		  if (p2) {
		     dbg_copy_data_to_list (&p1, p2->type, p2->data);
		  }
	       }
	       p = p->next;
	    }
	    p = p1;
	    while (p1 && Dbg_wlist) {
	       p2 = dbg_is_in_list (Dbg_wlist, p1->type, p1->data, &p3, FALSE);
	       if (p2) {
		  if (p3)
		    p3->next = p2->next;
		  else
		    Dbg_wlist = p2->next;
		  rul__dbg_free_pattern ((Pat_Pattern **) &p2->data);
		  rul__mem_free (p2);
	       }
	       p1 = p1->next;
	    }
	    if (p)
	      rul__dbg_free_param_list (p);
	 }
      }
      else {		/* all off */
	 rul__dbg_gl_ebreak = 0;
	 rul__dbg_free_param_list (Dbg_wlist);
	 Dbg_wlist = NULL;
      }
   }

   if (Dbg_wlist != NULL)
     rul__dbg_ga_wbreak = NULL;
   else
     rul__dbg_ga_wbreak = rul__dbg_match_wbreak;

}

Boolean rul__dbg_match_wbreak (Object wmo)
{
  Boolean       match = FALSE;
  Param_List    p = Dbg_wlist;
  Pat_Pattern  *pat;
  Pat_Classes  *pc;
  Pat_Attrs    *pa;
  Pat_Tests    *pt, *pdt;
  Molecule      att_val, tst_val;
  long		pred;
  long          idx;

  while (p && match == FALSE) {
     
     pat = (Pat_Pattern*) p->data;
    
     if (pat) {

	for (pc = pat->pc, match = TRUE; pc && match == TRUE; pc = pc->next) {

	   if (!(rul__decl_is_subclass_of (rul__wm_get_class (wmo),
					   pc->cls))) {
	      match = FALSE;
	}
	else {

	   for (pa = pc->pa; pa && match == TRUE; pa = pa->next) {
	      att_val = rul__wm_get_attr_val (wmo, pa->attr);
	      if (pa->idx) {
		 idx = pa->idx;
		 if (idx == -1)
		   idx = rul__mol_get_poly_count (att_val);
		 if (idx <= rul__mol_get_poly_count (att_val))
		   att_val = rul__mol_get_comp_nth (att_val, idx);
		 else {
		    match = FALSE;
		    break;
		 }
	      }

	      pt = pa->pt;
	      tst_val = pt->u.value;
	      pred = pt->pred;
	    
	      if (pt->ttype == PAT_DISJ) {
		 pdt = pt->u.pt;
		 tst_val = pdt->u.value;
		 pred = pdt->pred;
	      }
	      else {
		 if (pt->ttype == PAT_CONJ) {
		    pt = pt->next;
		    if (pt->ttype == PAT_DISJ) {
		       pdt = pt->u.pt;
		       tst_val = pdt->u.value;
		       pred = pdt->pred;
		    }
		    else {
		       tst_val = pt->u.value;
		       pred = pt->pred;
		    }
		 }
	      }
	    
	      while (pt) {
	      
		 match = dbg_get_match_results (att_val, pred, tst_val);

		 if (pt->ttype == PAT_DISJ && match == FALSE && pdt->next) {
		    pdt = pdt->next;
		    tst_val = pdt->u.value;
		    pred = pdt->pred;
		 }
		 else if (match == FALSE) {
		    break;
		 }
		 else if (pt->next) {
		    pt = pt->next;
		    if (pt->ttype == PAT_DISJ) {
		       pdt = pt->u.pt;
		       tst_val = pdt->u.value;
		       pred = pdt->pred;
		    }
		    else {
		       tst_val = pt->u.value;
		       pred = pt->pred;
		    }
		 }
		 else
		   pt = pt->next;
	      }
	   }
	}
	   if (pc->ctype == PAT_CE_NEG) {
	      if (match == FALSE)
		match = TRUE;
	      else
		match = FALSE;
	   }
	}
     }
     p = p->next;
  }

  return (match);
}

void rul__dbg_free_pattern (Pat_Pattern **pat_adr)
{
   Pat_Pattern   *pat = *pat_adr;
   Pat_Classes   *pc,  *pcn;
   Pat_Attrs     *pa,  *pan;
   Pat_Tests     *pt,  *ptn, *pdt, *pdtn;
   
   if (pat) {
      pc = pat->pc;
      while (pc) {
	 pcn = pc->next;
	 pa = pc->pa;
	 while (pa) {
	    pan = pa->next;
	    pt = pa->pt;
	    while (pt) {
	       ptn = pt->next;
	       if (pt->ttype == PAT_DISJ) {
		  pdt = pt->u.pt;
		  while (pdt) {
		     pdtn = pdt->next;
		     rul__mol_decr_uses (pdt->u.value);
		     rul__mem_free (pdt);
		     pdt = pdtn;
		  }
	       }
	       else {
		  rul__mol_decr_uses (pt->u.value);
	       }
	       rul__mem_free (pt);
	       pt = ptn;
	    }
	    rul__mem_free (pa);
	    pa = pan;
	 }
	 rul__mem_free (pc);
	 pc = pcn;
      }
      rul__mem_free (pat);
      *pat_adr = NULL;
   }
}

void rul__dbg_set_prompt (Molecule prompt)
{
  rul__scan_set_prompt (prompt);
  Dbg_name = prompt;
}

void rul__dbg_set_exit (long exit_value)
{
  Dbg_exit_value = exit_value;
  Dbg_exit_flag = TRUE;
}

void rul__dbg_flag_changes (void)
{
  if (Dbg_ebreak_exit == FALSE)
    Dbg_changes = 1;
  else
    rul__ios_printf (RUL__C_STD_ERR, CLI_STR_NOMOREFIRINGS);
}

void rul__dbg_print_cs (void)
{
  if (Dbg_winner) {
    rul__cs_print_cs (Dbg_css, Dbg_rbname_count,
		    Dbg_rs, RUL__C_STD_ERR);
  } else {
    rul__ios_printf (RUL__C_STD_ERR, CLI_STR_EMPTY_CS);
  }
}

static void print_class_list (Molecule blk_name)
{
   Class rclass;

   for (rclass = rul__decl_get_block_classes (rul__decl_get_block (blk_name));
	rclass != NULL;
	rclass = rclass->next) {
      if (rclass->name != rul__mol_symbol_root ()) {
	 rul__ios_printf (RUL__C_STD_ERR, "       ");
	 rul__mol_print_readform (rclass->name, RUL__C_STD_ERR);
	 rul__ios_printf (RUL__C_STD_ERR, "\n");
      }
   }
}

void rul__dbg_print_class_list (void)
{
   long  i, j, db_cnt;
   Class rclass;
   Molecule *db_names;
   
   for (i = 0; i < Dbg_rbname_count; i++) {
      if (i == 0)
	rul__ios_printf (RUL__C_STD_ERR, "   Entry block ");
      else
	rul__ios_printf (RUL__C_STD_ERR, "   Rule block ");
      rul__mol_print_readform (Dbg_rbnames[i], RUL__C_STD_ERR);
      rul__ios_printf (RUL__C_STD_ERR, "\n");
      if (rul__decl_block_has_classes (Dbg_rbnames[i])) {
	 print_class_list (Dbg_rbnames[i]);
      }
      
      db_cnt = rul__rbs_dblocks (Dbg_rbnames[i], &db_names);
      
      for (j = 0; j < db_cnt; j++) {
	 if (db_names[j] != Dbg_rbnames[i]) {
	    if (rul__decl_block_has_classes (db_names[j])) {
	       rul__ios_printf (RUL__C_STD_ERR, "     Declaration block ");
	       rul__mol_print_readform (db_names[j], RUL__C_STD_ERR);
	       rul__ios_printf (RUL__C_STD_ERR, "\n");
	       print_class_list (db_names[j]);
	    }
	 }
      }
   }
}

Boolean rul__dbg_is_visible_class (Mol_Symbol class_name)
{
   return (rul__decl_get_visible_class_rt (class_name, Dbg_dbnames,
					   Dbg_dbname_count) != NULL);
}


void rul__dbg_show_amb_class (Mol_Symbol class_name)
{
   long       i;

   for (i = 0; i < Dbg_dbname_count; i++) {
      if (rul__decl_get_class (rul__decl_get_block (Dbg_dbnames[i]),
			       class_name) != NULL) {
	 rul__ios_printf (RUL__C_STD_ERR, "   ");
	 rul__mol_print_readform (Dbg_dbnames[i], RUL__C_STD_ERR);
	 rul__ios_printf (RUL__C_STD_ERR, "~");
	 rul__mol_print_readform (class_name, RUL__C_STD_ERR);
	 rul__ios_printf (RUL__C_STD_ERR, "\n");
      }
   }
}

void rul__dbg_show_matches (Param_List plist)
{
   Boolean    found = FALSE;
   long       k = -1;
   char       buf[RUL_C_MAX_SYMBOL_SIZE+1];
   Param_List p = plist;
   RuleName   rn;
   Molecule   rb = NULL, rule = NULL;

   while (p) {

      rn = (RuleName) p->data;
      rb = rn->rb_name;
      rule = rn->rule_name;
      if (!rul__dbg_is_valid_rb (&k, rb)) {
	 rul__mol_use_readform (rb, buf, RUL_C_MAX_SYMBOL_SIZE+1);
	 rul__ios_printf (RUL__C_STD_ERR, CLI_STR_NOSUCHBLOCK, buf);
	 return;
      }

      while (rul__dbg_is_valid_rule (&k, rule)) {
	 found = TRUE;
	 rul__ios_printf (RUL__C_STD_ERR, "   >>> ");
	 if (rul__dbg_gl_enable & DBG_M_ENABLE_BKN) {
	    rul__mol_print_readform (Dbg_rbnames[k], RUL__C_STD_ERR);
	    rul__ios_printf (RUL__C_STD_ERR, "~");
	 }
	 rul__mol_print_readform (rule, RUL__C_STD_ERR);
	 rul__ios_printf (RUL__C_STD_ERR, " <<<");
	 rul__rbs_print_matches (Dbg_rbnames[k], rule);
	 rul__cs_print_matches (Dbg_rbnames[k], rule,
				Dbg_css, Dbg_rbname_count,
				Dbg_rs, RUL__C_STD_ERR);
	 if (rb != NULL)
	   break;
      }
      if (found == FALSE) {
	 rul__mol_use_readform (rule, buf, RUL_C_MAX_SYMBOL_SIZE+1);
	 rul__ios_printf (RUL__C_STD_ERR, CLI_STR_NOSUCHRULE, buf);
      }
      p = p->next;
   }
}

void rul__dbg_show_next (void)
{
   long j;

   if (Dbg_winner) {
   
      rul__ios_printf (RUL__C_STD_ERR, "   %ld (%ld): ",
		    rul__rac_rule_firing_cycle, *Dbg_cycle_ptr);

      if (rul__dbg_gl_enable & DBG_M_ENABLE_BKN) {
	 rul__mol_print_readform (rul__cs_get_cse_rb_name (Dbg_winner),
				  RUL__C_STD_ERR);
	 rul__ios_printf (RUL__C_STD_ERR, "~");
      }
      rul__mol_print_readform (rul__cs_get_cse_rule_name (Dbg_winner),
			       RUL__C_STD_ERR);
      for (j = 1; j <= rul__cs_get_cse_object_count (Dbg_winner); j++) {
	 rul__ios_printf (RUL__C_STD_ERR, " ");
	 rul__mol_print_readform (rul__cs_get_cse_nth_instance (Dbg_winner,
								j), 
				  RUL__C_STD_ERR);
      }
      rul__ios_printf (RUL__C_STD_ERR, "\n");
   }
   else
     rul__ios_printf (RUL__C_STD_ERR, CLI_STR_NO_NEXT);
}


void rul__dbg_set_return (Molecule ret_val)
{
   Molecule_Type  ret_type;
   char          *asciz_type;
   
   if (ret_val == NULL) {
      if (Dbg_return == NULL) {
	 Dbg_return_flag = TRUE;
      } else {
         /* error, value required */
         rul__msg_print (CLI_RETVALREQ);
      }
   }
   else {
      if (Dbg_return == NULL) {
	 rul__msg_print (CLI_RETNOTREQ);
      }
      else {
	 /* validate return value type */
	 ret_type = rul__mol_get_value_type (*Dbg_return);
	 if (ret_type != rul__mol_get_value_type (ret_val)) {
	    switch (ret_type) {
	     case int_atom:
	       asciz_type = CLI_STR_DATA_TYP_INT;
	       break;
	     case dbl_atom:
	       asciz_type = CLI_STR_DATA_TYP_FLOAT;
	       break;
	     case opaque:
	       asciz_type = CLI_STR_DATA_TYP_OPAQUE;
	       break;
	     case symbol:
	       asciz_type = CLI_STR_DATA_TYP_SYMBOL;
	       break;
	     case instance_id:
	       asciz_type = CLI_STR_DATA_TYP_ID;
	       break;
	     case compound:
	       asciz_type = CLI_STR_DATA_TYP_COMPOUND;
	       break;
	     case table:
	       asciz_type = CLI_STR_DATA_TYP_TABLE;
	       break;
	    }
	    rul__msg_print (CLI_RETWRGTYP, asciz_type);
	    rul__mol_decr_uses (ret_val);
	 }
	 else {
	    /* all ok */
	    rul__mol_decr_uses (*Dbg_return);
	    *Dbg_return = ret_val;
	    Dbg_return_flag = TRUE;
	 }
      }
   }
}

void rul__dbg_set_run (long run_count)
{
   long       rbreak_msg_idx;
   Mol_Symbol rbreak_rule;
   char       rbreak_msg[RBREAK_MSG_LEN+1];
   
   Dbg_run_count = run_count;
   Dbg_run_flag = TRUE;

   /* check to see if the next rule to fire has a rbreak set on it
    * - did we break on this winner already?
    * -- are there rbreaks set and a next?
    * --- does the winner have a rbreak set on it?
    * ---- display rbreak messages
    * ---- reset run flag and set last Dbg_rbreak
    */
   if (Dbg_rbreak != Dbg_winner) {
      if (Dbg_rlist && Dbg_winner) {
	 if (rul__dbg_match_rbreak (Dbg_winner)) {
	    rbreak_msg_idx = 0;
	    if (rul__dbg_gl_enable & DBG_M_ENABLE_BKN) {
	       rbreak_rule = rul__cs_get_cse_rb_name (Dbg_winner);
	       rul__mol_use_readform (rbreak_rule, rbreak_msg, RBREAK_MSG_LEN);
	       rbreak_msg_idx = rul__mol_get_readform_length (rbreak_rule);
	       rbreak_msg[rbreak_msg_idx++] = '~';
	    }
	    rul__mol_use_readform (rul__cs_get_cse_rule_name (Dbg_winner),
				   &(rbreak_msg[rbreak_msg_idx]),
				   RBREAK_MSG_LEN-(rbreak_msg_idx+1));
	    rbreak_msg_idx += rul__mol_get_readform_length (
				    rul__cs_get_cse_rule_name (Dbg_winner));
	    rbreak_msg[rbreak_msg_idx] = '\0';
	    rul__msg_print(CLI_RBREAK, rbreak_msg);
	    rul__msg_print (CLI_BREAKNOTED);
	    Dbg_run_flag = FALSE;
	    Dbg_rbreak = Dbg_winner;
	 }
      }
   }
}



void rul__dbg_show_rbreaks (void)
{
   Param_List     p = Dbg_rlist;
   RuleName       rn;
   long           i = 1;

   if (p) {
      rul__ios_printf (RUL__C_STD_ERR, CLI_STR_RBREAK_HEADER);
      while (p) {
	 rul__ios_printf (RUL__C_STD_ERR, "   %-3d", i++);
	 rn = (RuleName) p->data;
	 if (rul__dbg_gl_enable & DBG_M_ENABLE_BKN && rn->rb_name) {
	    rul__mol_print_readform (rn->rb_name, RUL__C_STD_ERR);
	    rul__ios_printf (RUL__C_STD_ERR, "~");
	 }
	 rul__mol_print_readform (rn->rule_name, RUL__C_STD_ERR);
	 rul__ios_printf (RUL__C_STD_ERR, "\n");
	 p = p->next;
      }
   }
   else
     rul__ios_printf (RUL__C_STD_ERR, CLI_STR_RBREAK_NONE);
}

Boolean rul__dbg_match_rbreak (Conflict_Set_Entry cse)
{
   RuleName      rn;
   Param_List    p = Dbg_rlist;

   while (p != NULL) {
      rn = (RuleName) p->data;
      if ((rn->rb_name == NULL ||
	   rul__cs_get_cse_rb_name (cse) == rn->rb_name) &&
	  rul__cs_get_cse_rule_name (cse) == rn->rule_name)
	return TRUE;
      p = p->next;
   }

  return (FALSE);
}

void rul__dbg_rbreak (Boolean on, Param_List plist)
{
   Param_List    p, p1, p2, p3;
   long		 i;
   RuleName      rn;
   Molecule      rb = NULL, rule = NULL;
   char          buf[RUL_C_MAX_SYMBOL_SIZE+1];
   long          k;
   Boolean       list_valid = TRUE;

   if (on) {
      /* validate entire list */
      p = plist;
      while (p) {
	 k = -1;
	 rn = (RuleName) p->data;
	 rb = (Molecule) rn->rb_name;
	 rule = (Molecule) rn->rule_name;
	 if (!rul__dbg_is_valid_rb (&k, rb)) {
	    rul__mol_use_readform (rb, buf, RUL_C_MAX_SYMBOL_SIZE+1);
	    rul__ios_printf (RUL__C_STD_ERR, CLI_STR_NOSUCHBLOCK, buf);
	    list_valid = FALSE;
	 }
	 else if (rb == NULL && rul__dbg_is_dup_rule (rule)) {
	    rul__mol_use_readform (rule, buf, RUL_C_MAX_SYMBOL_SIZE+1);
	    rul__ios_printf (RUL__C_STD_ERR, CLI_STR_AMBIGRULE, buf);
	    rul__dbg_show_amb_rule (rule);
	    list_valid = FALSE;
	 }
	 else if (! rul__dbg_is_valid_rule (&k, rule)) {
	    rul__ios_printf (RUL__C_STD_ERR, CLI_STR_NOSUCHRULE2);
	    if (rb != NULL) {
	       rul__mol_use_readform (rb, buf, RUL_C_MAX_SYMBOL_SIZE+1);
	       rul__ios_printf (RUL__C_STD_ERR, "%s~", buf);
	    }
	    rul__mol_use_readform (rule, buf, RUL_C_MAX_SYMBOL_SIZE+1);
	    rul__ios_printf (RUL__C_STD_ERR, "%s\n", buf);
	    list_valid = FALSE;
	 }
	 else if (rb == NULL) {
	    rn->rb_name = Dbg_rbnames[k];
	    rul__mol_incr_uses (rn->rb_name);
	 }
	 p = p->next;
      }
      
      /* add to list */
      if (list_valid) {
	 p = plist;
	 while (p) {
	    dbg_copy_data_to_list (&Dbg_rlist, p->type, p->data);
	    p = p->next;
	 }
      }
   }
   else {
      if (plist) {
	 if (Dbg_rlist) {
	    p = plist;
	    p1 = NULL;
	    while (p) {
	       if (p->type == DBG__E_PL_RUL) {
		  dbg_copy_data_to_list (&p1, p->type, p->data);
	       }
	       else {
		  p2 = Dbg_rlist;
		  for (i = 1; i < (long) p->data && p2 != NULL; i++) {
		     p2 = p2->next;
		  }
		  if (p2) {
		     dbg_copy_data_to_list (&p1, p2->type, p2->data);
		  }
	       }
	       p = p->next;
	    }
	    p = p1;
	    while (p1 && Dbg_rlist) {
	       p2 = dbg_is_in_list (Dbg_rlist, p1->type, p1->data, &p3, TRUE);
	       if (p2) {
		  if (p3)
		    p3->next = p2->next;
		  else
		    Dbg_rlist = p2->next;
		  p2->next = NULL;
		  rul__dbg_free_param_list (p2);
	       }
	       p1 = p1->next;
	    }
	 }
	 if (p)
	   rul__dbg_free_param_list (p);
      }
      else { /* remove all rbreaks */
	 rul__dbg_free_param_list (Dbg_rlist);
	 Dbg_rlist = NULL;
      }
   }
}

Boolean rul__dbg_is_valid_rb (long *rb_index, Mol_Symbol rb)
{
  long     i;

  if (rb == NULL) {
   *rb_index = -1;
   return TRUE;
 }

  for (i = 0; i < Dbg_rbname_count; i++) {
    if (rb == Dbg_rbnames[i]) {
      *rb_index = i - 1;
      return TRUE;
    }
  }
  return FALSE;
}


Boolean rul__dbg_is_valid_db (Mol_Symbol db)
{
  long     i;

  for (i = 0; i < Dbg_dbname_count; i++) {
    if (db == Dbg_dbnames[i]) {
      return TRUE;
    }
  }
  return FALSE;
}


Boolean rul__dbg_is_valid_rule (long *rb_index, Mol_Symbol rule)
{
  long     i, j, k = 0;
  Rb_Cons  cons;

  if (rb_index == NULL)
    j = 0;
  else
    j = *rb_index;

  for (j += 1; j < Dbg_rbname_count; j++) {
    cons.rb_count = rul__rbs_constructs (Dbg_rbnames[j],
					 &(cons.rb_names),
					 &(cons.rb_types));
    for (i = 0; i < cons.rb_count; i++) {    
      if ((cons.rb_names[i] == rule) && (cons.rb_types[i] == RUL__C_RULE)) {
	if (rb_index != NULL)
	  *rb_index = j;
	return (TRUE);
      }
    }
  }
  return (FALSE);
}

Boolean rul__dbg_is_dup_rule (Mol_Symbol rule)
{
  long     i, j;
  Rb_Cons  cons;
  Boolean  found = FALSE;

  for (j = 0; j < Dbg_rbname_count; j++) {
    cons.rb_count = rul__rbs_constructs (Dbg_rbnames[j],
					 &(cons.rb_names),
					 &(cons.rb_types));
    for (i = 0; i < cons.rb_count; i++) {    
      if ((cons.rb_names[i] == rule) && (cons.rb_types[i] == RUL__C_RULE)) {
	if (found)
	  return (TRUE);
	found = TRUE;
      }
    }
  }
  
  return (FALSE);
}

void rul__dbg_show_amb_rule (Mol_Symbol rule)
{
  long     i, j;
  Rb_Cons  cons;

  for (j = 0; j < Dbg_rbname_count; j++) {
    cons.rb_count = rul__rbs_constructs (Dbg_rbnames[j],
					 &(cons.rb_names),
					 &(cons.rb_types));
    for (i = 0; i < cons.rb_count; i++) {    
      if ((cons.rb_names[i] == rule) && (cons.rb_types[i] == RUL__C_RULE)) {
	rul__ios_printf (RUL__C_STD_ERR, "   ");
	rul__mol_print_readform (Dbg_rbnames[j], RUL__C_STD_ERR);
	rul__ios_printf (RUL__C_STD_ERR, "~");
	rul__mol_print_readform (rule, RUL__C_STD_ERR);
	rul__ios_printf (RUL__C_STD_ERR, "\n");
	break;
      }
    }
  }
}

Mol_Symbol rul__dbg_get_catcher_rb_name (Mol_Symbol catcher_name)
{
  long i,j;
  Mol_Symbol *catcher_name_array;
  long catcher_array_len;

  for (i = 0; i < Dbg_rbname_count; i++) {
    catcher_array_len = rul__rbs_catchers (Dbg_rbnames[i],
					   &catcher_name_array);
    for (j = 0; j < catcher_array_len; j++) {
      if (catcher_name_array[j] == catcher_name)
	return Dbg_rbnames[i];
    }
  }
  return NULL;
}

void rul__dbg_set_catcher (Molecule after_count, Mol_Symbol catcher_name)
{
  long i, j;
  Rb_Cons cons;

  for (j = 0; j < Dbg_rbname_count; j++) {
    cons.rb_count = rul__rbs_constructs (Dbg_rbnames[j],
					 &(cons.rb_names),
					 &(cons.rb_types));
    for (i = 0; i < cons.rb_count; i++) {    
      if ((cons.rb_names[i] == catcher_name) &&
	  (cons.rb_types[i] == RUL__C_CATCH)) {
	Dbg_catcher_name = catcher_name;
	Dbg_after_count = rul__mol_int_atom_value (after_count);
	Dbg_catcher_rb_name = rul__dbg_get_catcher_rb_name (catcher_name);
	rul__msg_print_w_atoms(CLI_SETCATCH, 2, catcher_name, after_count);
	return;
      }
    }
  }
  rul__msg_print_w_atoms (CLI_INVCATCH, 1, catcher_name);
  return;
}

void rul__dbg_addstate (Mol_Symbol filename)
{
  rul__addstate (filename, Dbg_ebdata);
  Dbg_changes = 1;
}

void rul__dbg_restorestate (Mol_Symbol filename)
{
   rul__restorestate (filename, Dbg_ebdata);
   Dbg_changes = 1;
}

void rul__dbg_savestate (Mol_Symbol filename)
{
   rul__savestate (filename, Dbg_ebdata);
}

void rul__dbg_set_group (void)
{
   rul__dbg_ga_group = rul__dbg_get_group (Dbg_winner);
}


Mol_Symbol rul__dbg_get_group (Conflict_Set_Entry cse)
{
   long        i, j;
   Rb_Cons     cons;
   Mol_Symbol  group = NULL;
   Mol_Symbol  block;
   
   if (cse) {
      for (j = 0; j < Dbg_rbname_count; j++) {
	 cons.rb_count = rul__rbs_constructs (Dbg_rbnames[j],
					      &(cons.rb_names),
					      &(cons.rb_types));
	 block = rul__cs_get_cse_rb_name (cse);
	 if (cons.rb_count > 0) {
	    if (block == cons.rb_names[0]) {
	       for (i = 1; i < cons.rb_count; i++) {    
		 if (cons.rb_types[i] == RUL__C_RULE_GROUP)
		   group = cons.rb_names[i];
		 else if (cons.rb_types[i] == RUL__C_END_GROUP)
		   group = NULL;
		 else if (cons.rb_names[i] == rul__cs_get_cse_rule_name(cse) &&
			  cons.rb_types[i] == RUL__C_RULE)
		   return (group);
	      }
	    }
	 }
      }
   }
   return (group);
}

static void dbg_init_trace_mols (void)
{

   if (SM_trace_off == NULL) {
      SM_trace_off = rul__mol_make_symbol ("OFF");
      rul__mol_mark_perm (SM_trace_off);
      SM_trace_on = rul__mol_make_symbol ("ON");
      rul__mol_mark_perm (SM_trace_on);
      SM_trace_star = rul__mol_make_symbol ("*");
      rul__mol_mark_perm (SM_trace_star);
      SM_trace_entry_block = rul__mol_make_symbol ("ENTRY-BLOCK");
      rul__mol_mark_perm (SM_trace_entry_block);
      SM_trace_eb = rul__mol_make_symbol ("EB");
      rul__mol_mark_perm (SM_trace_eb);
      SM_trace_rule_group = rul__mol_make_symbol ("RULE-GROUP");
      rul__mol_mark_perm (SM_trace_rule_group);
      SM_trace_rg = rul__mol_make_symbol ("RG");
      rul__mol_mark_perm (SM_trace_rg);
      SM_trace_r = rul__mol_make_symbol ("RULE");
      rul__mol_mark_perm (SM_trace_r);
      SM_trace_wm = rul__mol_make_symbol ("WM");
      rul__mol_mark_perm (SM_trace_wm);
      SM_trace_cs = rul__mol_make_symbol ("CS");
      rul__mol_mark_perm (SM_trace_cs);
   }
}

static void dbg_set_trace (Boolean on, Molecule mol)
{

   if (mol == SM_trace_star)
     if (on)
       rul__dbg_gl_trace |= DBG_M_TRACE_ALL;
     else
       rul__dbg_gl_trace &= ~DBG_M_TRACE_ALL;
   
   else if (mol == SM_trace_eb || mol == SM_trace_entry_block)
     if (on)
       rul__dbg_gl_trace |= DBG_M_TRACE_EB;
     else
       rul__dbg_gl_trace &= ~DBG_M_TRACE_EB;
   
   else if (mol == SM_trace_rg || mol == SM_trace_rule_group)
     if (on)
       rul__dbg_gl_trace |= DBG_M_TRACE_RG;
     else
       rul__dbg_gl_trace &= ~DBG_M_TRACE_RG;
   
   else if (mol == SM_trace_r)
     if (on)
       rul__dbg_gl_trace |= DBG_M_TRACE_R;
     else
       rul__dbg_gl_trace &= ~DBG_M_TRACE_R;
   
   else if (mol == SM_trace_wm)
     if (on)
       rul__dbg_gl_trace |= DBG_M_TRACE_WM;
     else
       rul__dbg_gl_trace &= ~DBG_M_TRACE_WM;
   
   else if (mol == SM_trace_cs)
     if (on)
       rul__dbg_gl_trace |= DBG_M_TRACE_CS;
     else
       rul__dbg_gl_trace &= ~DBG_M_TRACE_CS;
}

char *rul__dbg_toupper (char *str)
{
  int i;
  int str_len = strlen(str);
  static char *upstr;
  static long  upstr_len;

  if (!upstr || (upstr_len <= str_len)) {
    if (upstr)
      rul__mem_free (upstr);
    upstr = rul__mem_malloc (str_len + 1);
  }

  for (i = 0; i < str_len; i++)
    upstr[i] = toupper(str[i]);

  upstr[str_len] = '\0';

  return (upstr);
}


#ifdef __VMS
/*
 **  enable default control Y (as DCL command 'set control=Y')
 */
void rul__dbg_enable_control_y ()
{
  long arg1 = LIB$M_CLI_CTRLY;

  if (!rul_gl_input_channel)
    if (lib$enable_ctrl (&arg1, 0) & 1)
      rul__dbg_enable_control_c ();
  
  return;
}

/************************************************************************
			ENABLE_CONTROL_c
*************************************************************************/
void rul__dbg_enable_control_c ()
{
  struct dsc$descriptor  input;
  char                  *input_name = "SYS$INPUT";

  /*
   ** assign a channel to sys$input
   */
  if (!rul_gl_input_channel) {
    input.dsc$w_length = strlen (input_name);
    input.dsc$b_dtype =  DSC$K_DTYPE_T;
    input.dsc$b_class =  DSC$K_CLASS_S;
    input.dsc$a_pointer = &(input_name[0]);
    if (!(sys$assign (&input, &rul_gl_input_channel, 0, 0) & 1))
      return;
  }
  /*
   ** set the AST on control C
   */
  sys$qiow (0, rul_gl_input_channel, rul_gl_ctrlc_code,
	    rul_gq_iostat, 0, 0, rul__dbg_control_c_ast, 0, 0, 0, 0, 0);
  
  return;
}

/************************************************************************
			DISABLE_CONTROL_c
*************************************************************************/
void rul__dbg_disable_control_c ()
{
  /*
   ** if channel assigned to sys$input
   */
  if (rul_gl_input_channel)
    /*
     ** Reset the AST on control C
     */
    sys$qiow (0, rul_gl_input_channel, rul_gl_ctrlc_code,
	      rul_gq_iostat, 0, 0, 0, 0, 0, 0, 0, 0);
  
  return;
}

/*
 ** Received on control Y or control C
 */
static void rul__dbg_control_c_ast ()
{
  /*
   ** reset the AST on control C
   */
  sys$qiow (0, rul_gl_input_channel, rul_gl_ctrlc_code,
	    rul_gq_iostat, 0, 0, rul__dbg_control_c_ast, 0, 0, 0, 0, 0);

  rul__dbg_gl_break |= DBG_M_BREAK_CRC;
  return;
}
#else
void        rul__dbg_enable_control_y () {}
void        rul__dbg_enable_control_c () {}
void        rul__dbg_disable_control_c () {}
#endif

