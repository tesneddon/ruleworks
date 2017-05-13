/* rts_rac.c - Recognize-Act Cycle routines */
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
 *  FACILITY:
 *	RULEWORKS run time system
 *
 *  ABSTRACT:
 *	Recognize-Act Cycle
 *
 *  MODIFIED BY:
 *	DEC	Digital Equipment Corporation
 *	CPQ	Compaq Computer Corporation
 *
 *  MODIFICATION HISTORY:
 *
 *	14-Sep-1992	DEC	Initial version
 *	10-Nov-1992	DEC	Add dbg stuff
 *	16-Feb-1998	DEC	class changed to rclass
 *	01-Dec-1999	CPQ	Release with GPL
 */

#include <common.h>
#include <dyar.h>
#include <rts_msg.h>		/* declares messages		        */
#include <msg.h>		/* declares message functions           */
#include <rbs.h>		/* For Ruling_Block_Func */
#include <rac.h>		/* This module's interface */
#include <cs.h>			/* For rul__cs_get_winner() */
#include <wm.h>			/* For current time tag */
#include <mol.h>		/* Molecule functions */
#include <ios.h>		/* For IO_Stream defs */
#include <decl.h>

#define IN_RTS_RAC	/* Note:  this changes behaviour of dbg.h */
#include <dbg.h>		/* For Debug data */ 

/* global data accessed by all modules... */
GLOBAL long     (*rul__dbg_ga_interp)(Entry_Data eb_data) = 0;
GLOBAL Boolean  (*rul__dbg_ga_ebreak)(Mol_Symbol ebname);
GLOBAL Boolean  (*rul__dbg_ga_wbreak)(Object wmo) = 0;
GLOBAL Boolean  (*rul__dbg_ga_rbreak)(Conflict_Set_Entry cse) = 0;
GLOBAL long       rul__dbg_gl_break = 0;
GLOBAL long       rul__dbg_gl_ebreak = 0;
GLOBAL long       rul__dbg_gl_trace = 0;
GLOBAL long       rul__dbg_gl_enable = 0;
GLOBAL Mol_Symbol rul__dbg_ga_group = 0;
GLOBAL long       rul__rac_rule_firing_cycle = 1;
GLOBAL Mol_Symbol rul__rac_active_rule_name;

#define MAX_RBREAK_MSG_LEN 2000
static long       rbreak_msg_idx;
static char       rbreak_msg[MAX_RBREAK_MSG_LEN]; /* 'blkname\rulename' */
static Mol_Symbol rbreak_rule;

/* use the same cycle and run counters nomatter how many entry-blocks */
static long       run_count = 0;

static Mol_Symbol SM_on_empty;
static Mol_Symbol SM_on_entry;
static Mol_Symbol SM_on_every;
static Mol_Symbol SM_on_exit;

static void  rac_save_timetags_for_refract (Conflict_Set_Entry cse,
					    Dynamic_Array timetags);
static void  rac_refract_cse_if_still_valid (Refraction_Set refraction_set,
					     Conflict_Set_Entry winning_cse,
					     Dynamic_Array timetags);
static long  rul___rac_create_decl_array (long rb_count,
					  Molecule *rb_names,
					  Molecule **db_names);



/*****************************************************************************/
/*
 * The main loop of RULEWORKS.  Repeatedly find the dominant instantiation and
 * fire it.  Return when there are no satisfied rules, or when the RHS
 * executes the RETURN action.
 */

void rul__rac_cycle (Molecule       rb_name_array[],
		     long           rb_name_array_length,
		     Refraction_Set refraction_set,
		     long (*on_entry_func) (Molecule eb_vars[],
					    Entry_Data eb_data),
		     long (*on_every_func) (Molecule eb_vars[],
					    Entry_Data eb_data),
		     long (*on_empty_func) (Molecule eb_vars[],
					    Entry_Data eb_data),
		     long (*on_exit_func)  (Molecule eb_vars[],
					    Entry_Data eb_data),
		     Molecule    eb_var_values[],
		     long       *eb_after_count,
		     Mol_Symbol *eb_catcher_name,
		     Mol_Symbol *eb_catcher_rb_name,
		     Molecule   *eb_return_value,
		     Boolean     dbg_allowed)
{
   Conflict_Subset   *conflict_subset_array; /* Array of CSSes, one per rb */
   long               time_tag, rb_index, status = RUL__C_SUCCESS;
   Conflict_Set_Entry winning_cse = NULL;    /* conflict resolution winner */
   Entry_Data         eb_data;
   Molecule          *dbg_cons;        	     /* used to verify debugging */
   Construct_Type    *dbg_cons_type;         /* used to verify debugging */
   long               dbg_cons_count;
   Boolean            done_looping = FALSE;
   long               dbg_status = 0;
   Mol_Symbol         current_group = 0;
   Dynamic_Array      old_timetags = rul__dyar_create_array (6);
   Boolean            dbg_flag = dbg_allowed;
   Boolean            old_dbg_enable = (rul__dbg_gl_enable & DBG_M_ENABLE_DBG);
   Boolean            old_tra_enable = (rul__dbg_gl_enable & DBG_M_ENABLE_TRA);
   Strategy           strat0 = 0, strat1;
   long               cycle_count = 1;

   if (SM_on_empty == NULL) {
      SM_on_empty = rul__mol_make_symbol ("ON-EMPTY");
      rul__mol_mark_perm (SM_on_empty);
      SM_on_entry = rul__mol_make_symbol ("ON-ENTRY");
      rul__mol_mark_perm (SM_on_entry);
      SM_on_every = rul__mol_make_symbol ("ON-EVERY");
      rul__mol_mark_perm (SM_on_every);
      SM_on_exit = rul__mol_make_symbol ("ON-EXIT");
      rul__mol_mark_perm (SM_on_exit);
   }

   /* Allocate CSS and rbf arrays, one entry for each rule block */
   conflict_subset_array = RUL_MEM_ALLOCATE (Conflict_Subset,
					     rb_name_array_length);
  
   /*
    * Store CSS and rbf from each ruling block used in this entry block
    * and also check for the strategies to be the same
    */
   for (rb_index = 0; rb_index < rb_name_array_length; rb_index++) {
      conflict_subset_array [rb_index] = rul__rbs_get_css (
						   rb_name_array [rb_index]);
      if (strat0 >= 0) {
	 strat1 = rul__rbs_get_strategy (rb_name_array [rb_index]);
	 if (strat0 > 0 && strat0 != strat1) {
	    rul__msg_print_w_atoms (RTS_STRMISMAT, 2,
				    rb_name_array [rb_index-1],
				    rb_name_array [rb_index]);
	    strat0 = -1;		/* to prevent mutiple messages */
	 }
	 else
	   strat0 = strat1;
      }
   }
  
   /* Allocate and init eb_data struct */
   eb_data = (Entry_Data) rul__mem_calloc (1, sizeof(struct entry_data));
   eb_data->rb_name_count = rb_name_array_length;
   eb_data->rb_names = rb_name_array;
   eb_data->db_name_count = rul___rac_create_decl_array (rb_name_array_length,
							 rb_name_array,
							 &eb_data->db_names);
   eb_data->winning_cse_ptr = &winning_cse;
   eb_data->refraction_set = refraction_set;
   eb_data->conflict_subset_array = conflict_subset_array;
   eb_data->entry_values = eb_var_values;
   eb_data->cycle_count_ptr = &cycle_count;
   eb_data->run_count_ptr = &run_count;
   eb_data->after_count_ptr = eb_after_count;
   eb_data->active_rb_name = rb_name_array[0];
   eb_data->catcher_name_ptr = eb_catcher_name;
   eb_data->catcher_rb_name_ptr = eb_catcher_rb_name;
   eb_data->return_value_ptr = eb_return_value;

   dbg_cons_count = 0;
   for (rb_index = 0; rb_index < rb_name_array_length; rb_index++) {
      dbg_cons_count += rul__rbs_constructs (rb_name_array[0],
					     &dbg_cons, &dbg_cons_type);
   }
   /* if /deb=no, no rul__debug () or no constructs then no debug */
   if (!dbg_allowed ||
       rul__dbg_ga_interp == NULL ||
       dbg_cons_count == 0) {
      dbg_flag = FALSE;
      rul__dbg_gl_enable &= ~DBG_M_ENABLE_DBG;
   }

   if (dbg_allowed && dbg_cons_count)
     rul__dbg_gl_enable |= DBG_M_ENABLE_TRA;

   /* check trace for entry-block */
   if (rul__dbg_gl_trace & DBG_M_TRACE_EB) {
      if (rul__ios_curr_column (RUL__C_DEF_TRACE))
	rul__ios_print_newline (RUL__C_DEF_TRACE);
      rul__ios_printf (RUL__C_DEF_TRACE, "-->Entry-Block: ");
      /* the first rb_name will always be the entry-block!!! */
      rul__mol_print_readform (rb_name_array[0], RUL__C_DEF_TRACE);
      rul__ios_print_newline (RUL__C_DEF_TRACE);
      
   }

   eb_data->active_rb_name = rb_name_array[0];
   rul__rac_active_rule_name = SM_on_entry;

   if (!done_looping && on_entry_func != NULL) {
      status = (*on_entry_func) (eb_var_values, eb_data);

      if (dbg_flag == FALSE  &&  dbg_allowed  &&  
	  rul__dbg_ga_interp != NULL  &&  dbg_cons_count != 0) {
	 /*
	 **  If DEBUG action or rul__debug API call happened in the on-entry,
	 **  or the debugger was otherwise enabled within an entry-block
	 **  called from within the on-entry clause, enable the debugger here.
	 */
         dbg_flag = TRUE;
      }
   }

   if (dbg_flag) {
      if (rul__dbg_gl_ebreak & DBG_M_EBREAK_ALL ||
	  (rul__dbg_ga_ebreak != NULL) &&
	  (*rul__dbg_ga_ebreak) (rb_name_array[0]))
	rul__dbg_gl_break |= DBG_M_BREAK_ENTRY;
   }

   if (status != RUL__C_RETURN) {

      /*
       * Loop, finding and firing dominant instantiations.
       */
  
      do {
	 /*
	  * Process delta queues: propogate working memory changes into
	  * match networks of ruling blocks.
	  */
	 time_tag = rul__wm_get_cur_time_tag ();
	 for (rb_index = 0; rb_index < rb_name_array_length; rb_index++) {
	    /* Update the ruling block's match network */
	    rul__rbs_update_match_network (rb_name_array[rb_index], time_tag);
	 }
      
	 /*
	  * Find best unrefracted instantiation from this entry block's ruling
	  * blocks.
	  */
	 winning_cse = rul__cs_get_winner (conflict_subset_array,
					   rb_name_array_length,
					   refraction_set);
      
	 /*
	  * check run break if debugging...
	  */
	 if (dbg_flag) {
	    if (run_count && run_count == rul__rac_rule_firing_cycle)
	      rul__dbg_gl_break |= DBG_M_BREAK_RUN;
	 }
      
#if 0
	 /*
	  * check for no satisfied rule and neither a debugger nor breakpoints
	  */
	 if (dbg_flag) {
	    if (!winning_cse && rul__dbg_gl_ebreak & DBG_M_EBREAK_EMPTY)
	      rul__dbg_gl_break |= DBG_M_BREAK_EMPTY;
	 }
#endif      
	 
	 if (!winning_cse && !(dbg_flag && rul__dbg_gl_break)) {
	    
	    /* check trace for entry-block empty */
	    if (rul__dbg_gl_enable & DBG_M_ENABLE_TRA) {
	       if (rul__dbg_gl_trace & DBG_M_TRACE_EB &&
		   rul__dbg_gl_trace & DBG_M_TRACE_R  &&
		   on_empty_func != NULL) {

#ifdef TRACE_FOR_ON_CLAUSES
		  if (rul__ios_curr_column (RUL__C_DEF_TRACE))
		    rul__ios_print_newline (RUL__C_DEF_TRACE);
		  rul__ios_printf (RUL__C_DEF_TRACE, "   %ld (%ld): ",
				   rul__rac_rule_firing_cycle,
				   cycle_count);
		  if (rul__dbg_gl_enable & DBG_M_ENABLE_BKN) {
		     rul__mol_print_readform (rb_name_array[0],
					      RUL__C_DEF_TRACE);
		     rul__ios_printf(RUL__C_DEF_TRACE, "~");
		  }
		  rul__mol_print_readform (SM_on_empty, RUL__C_DEF_TRACE);
		  rul__ios_print_newline (RUL__C_DEF_TRACE);
#endif /* TRACE_FOR_ON_CLAUSES */
	       }
	    }

	    if (on_empty_func != NULL) { /* Call eb's ON-EMPTY func */

	       eb_data->active_rb_name = rb_name_array[0];
	       rul__rac_active_rule_name = SM_on_empty;
	       status = (*on_empty_func) (eb_var_values,  eb_data);
	    }

	    if (status == RUL__C_RETURN || !(dbg_flag && rul__dbg_gl_break))
	      done_looping = TRUE;
	 }

	 else {		/* Some rule is satisfied (or debugger enabled) */
	    
	    /*
	     * check for rbreak
	     */
	    if (dbg_flag) {
	       if (rul__dbg_ga_rbreak &&
		   winning_cse &&
		   (*rul__dbg_ga_rbreak) (winning_cse)) {
		  rul__dbg_gl_break |= DBG_M_BREAK_RULE;
		  rbreak_msg_idx = 0;
		  if (rul__dbg_gl_enable & DBG_M_ENABLE_BKN) {
		     rbreak_rule = rul__cs_get_cse_rb_name (winning_cse);
		     rul__mol_use_readform(rbreak_rule, &(rbreak_msg[0]),
					   MAX_RBREAK_MSG_LEN);
		     rbreak_msg_idx = rul__mol_get_readform_length (
							    rbreak_rule);
		     rbreak_msg[rbreak_msg_idx++] = '~';
		  }
		  rul__mol_use_readform (
				 rul__cs_get_cse_rule_name (winning_cse),
				 &(rbreak_msg[rbreak_msg_idx]),
				 MAX_RBREAK_MSG_LEN-(rbreak_msg_idx+1));
		  rbreak_msg_idx += rul__mol_get_readform_length (
				  rul__cs_get_cse_rule_name (winning_cse));
		  rbreak_msg[rbreak_msg_idx] = '\0';
		  rul__msg_print(RTS_RBREAK, rbreak_msg);
	       }
	    }

	    /*
	     * call the debugger if requested...
	     */
	    if (dbg_flag) {
	       if (rul__dbg_gl_break ||
		   ((rul__dbg_gl_enable & DBG_M_ENABLE_TRA) &&
		    (rul__dbg_gl_trace & DBG_M_TRACE_RG) &&
		    (rul__dbg_ga_interp != NULL))) {
		  dbg_status = (*rul__dbg_ga_interp) (eb_data);
		  if (dbg_status == 2) {
		     done_looping = TRUE;
		     dbg_status = 1;
		  }
	       }
	    }
	    
	    if (dbg_status == 0 && winning_cse) {
	       
	       /*
		* set globals for wm system...
		*/
	       eb_data->active_rb_name = rul__cs_get_cse_rb_name (winning_cse);
	       rul__rac_active_rule_name = rul__cs_get_cse_rule_name (
							      winning_cse);
	       
	       /* check trace for rule-group */
	       if (rul__dbg_gl_enable & DBG_M_ENABLE_TRA) {
		  if (rul__dbg_gl_trace & DBG_M_TRACE_RG) {
		     if (rul__dbg_ga_group != current_group) {
			if (rul__ios_curr_column (RUL__C_DEF_TRACE))
			  rul__ios_print_newline (RUL__C_DEF_TRACE);
			rul__ios_printf (RUL__C_DEF_TRACE, "   %ld (%ld): ",
					 rul__rac_rule_firing_cycle,
					 cycle_count);
			/*rul__ios_printf(RUL__C_DEF_TRACE, "Rule-Group:  ");*/
			if (rul__dbg_gl_enable & DBG_M_ENABLE_BKN) {
			   rul__mol_print_readform (
				    rul__cs_get_cse_rb_name (winning_cse),
				    RUL__C_DEF_TRACE);
			   rul__ios_printf(RUL__C_DEF_TRACE, "~");
			}
			if (rul__dbg_ga_group)
			  rul__mol_print_printform (rul__dbg_ga_group,
						    RUL__C_DEF_TRACE);
			rul__ios_print_newline (RUL__C_DEF_TRACE);
			current_group = rul__dbg_ga_group;
		     }
		  }
		  
		  /*
		   * If traceing rules, display cycle_count,
		   * rule-name, matching ids
		   */
		  if (rul__dbg_gl_trace & DBG_M_TRACE_R) {
		     if (rul__ios_curr_column (RUL__C_DEF_TRACE))
		       rul__ios_print_newline (RUL__C_DEF_TRACE);
		     rul__ios_printf (RUL__C_DEF_TRACE, "   %ld (%ld):  ",
				      rul__rac_rule_firing_cycle, cycle_count);
		     rul__cs_print_one_cse (winning_cse, RUL__C_DEF_TRACE);
		  }
	       }
	       
	       /*
		* Save the winning timetags so we can check if they change
		*/
	       rac_save_timetags_for_refract (winning_cse, old_timetags);
	       
	       /*
		* increment cycle counters
		*/
	       rul__rac_rule_firing_cycle += 1;
	       cycle_count += 1;

	       /*
		* Call the RHS function for the winning rule, passing the
		* CSE so it has access to the WMOs which instantiated it.
		*/
	       
	       status = rul__cs_call_rhs_func (winning_cse, eb_data);

	       /*
		* update names after rule firing
		*/
	       eb_data->active_rb_name = rul__cs_get_cse_rb_name (winning_cse);
	       rul__rac_active_rule_name = rul__cs_get_cse_rule_name (
							      winning_cse);
	       if (status == RUL__C_RETURN) {
		  done_looping = TRUE;
	       }
	       
	       else { /* we haven't explicitly QUIT or RETURNed */
		  
		  /*
		   * Add this instantiation (if it's still valid) to this 
		   * entry block's refraction set so it won't fire again.
		   */
		  rac_refract_cse_if_still_valid (refraction_set, 
						  winning_cse, old_timetags);
		  
#if 0
		  /* Call entry block's ON-EVERY function */
		  if (dbg_flag) {
		     if (rul__dbg_gl_ebreak & DBG_M_EBREAK_EVERY)
		       rul__dbg_gl_break |= DBG_M_BREAK_EVERY;
		  }
#endif
		  if (on_every_func != NULL) {
		     eb_data->active_rb_name = rb_name_array[0];
		     rul__rac_active_rule_name = SM_on_every;
		     status = (*on_every_func) (eb_var_values, eb_data);
		  }
		  
		  if (status == RUL__C_RETURN)
		    done_looping = TRUE;
		  
		  else {
		     
		     /*
		      * check if catcher is enabled and after count to fire on
		      */
		     if ((*eb_catcher_name != NULL) &&
			 ((cycle_count - 1) == *eb_after_count)) {
			eb_data->active_rb_name = *eb_catcher_rb_name;
			rul__rac_active_rule_name = *eb_catcher_name;
			status = rul__rbs_invoke_catcher (*eb_catcher_name,
							  *eb_catcher_rb_name,
							  eb_data);
			if (status == RUL__C_RETURN)
			  done_looping = TRUE;
		     } /* catcher enabled; cycle count matches after count */
		  } /* continuing (on_every success), check for catcher */
	       } /* continuing (rhs success vs. QUIT or RETURN) */
	    } /* the conflict set is not empty or debug changes */
	 } /* there is a winning CSE or there is a debugger breakpoint set */
      } while (!done_looping);
   } /* continuing (on_entry success) */
   
   /* flag leaving entry-block */
   /* check trace for entry-block */
   if (rul__dbg_gl_enable & DBG_M_ENABLE_TRA) {
      if (rul__dbg_gl_trace & DBG_M_TRACE_EB) {
	 if (rul__ios_curr_column (RUL__C_DEF_TRACE))
	   rul__ios_print_newline (RUL__C_DEF_TRACE);
	 rul__ios_printf (RUL__C_DEF_TRACE, "<--Entry-Block: ");
	 /* the first rb_name will always be the entry-block!!! */
	 rul__mol_print_readform (rb_name_array[0], RUL__C_DEF_TRACE);
	 rul__ios_print_newline (RUL__C_DEF_TRACE);
      }
   }
   
   if (on_exit_func != NULL) {
      eb_data->active_rb_name = rb_name_array[0];
      rul__rac_active_rule_name = SM_on_exit;
      status = (*on_exit_func) (eb_var_values, eb_data);
   }
   
   if (dbg_flag) {
      if (rul__dbg_gl_ebreak & DBG_M_EBREAK_ALL ||
	  (rul__dbg_ga_ebreak != NULL) &&
	  (*rul__dbg_ga_ebreak) (rb_name_array[0])) {
	 rul__dbg_gl_break |= DBG_M_BREAK_EXIT;
	 dbg_status = (*rul__dbg_ga_interp) (eb_data);
      }
   }
   
   rul__dyar_free_array (old_timetags);
   rul__mem_free (conflict_subset_array);
   rul__mem_free (eb_data->db_names);
   rul__mem_free (eb_data);
   
   if (old_dbg_enable)
     rul__dbg_gl_enable |= DBG_M_ENABLE_DBG;
   if (old_tra_enable)
     rul__dbg_gl_enable |= DBG_M_ENABLE_TRA;
}



/************************************
**                                 **
**  RAC_SAVE_TIMETAGS_FOR_REFRACT  **
**                                 **
************************************/

void rac_save_timetags_for_refract (Conflict_Set_Entry cse,
				    Dynamic_Array old_timetags)
{
  long i, len;

  rul__dyar_set_array_empty (old_timetags);
  len = rul__cs_get_cse_object_count(cse);

  for (i = 0; i < len; i++) {
    rul__dyar_set_nth (old_timetags, i,
		       (void *) rul__wm_get_time_tag (
			     rul__cs_get_cse_nth_object (cse, i + 1)));
  }
}


/*************************************
**                                  **
**  RAC_REFRACT_CSE_IF_STILL_VALID  **
**                                  **
*************************************/

static void  rac_refract_cse_if_still_valid (Refraction_Set refraction_set,
					     Conflict_Set_Entry winning_cse,
					     Dynamic_Array old_timetags)
{
  Mol_Instance_Id id;
  Object inst_ptr;
  long i, count, old_timetag, new_timetag;

  count = rul__cs_get_cse_object_count (winning_cse);

  for (i = 0; i < count; i++) {

    /*
     **  If the RHS removed one of the instances
     **  this instantiation can't ever fire again,
     **  so just return.
     */
    id = rul__cs_get_cse_nth_instance (winning_cse, i+1);
    inst_ptr = rul__mol_instance_id_value (id);
    if (inst_ptr == NULL)
      return;

    /*
     **  If the RHS modified one of the instances
     **  this instantiation can't ever fire again,
     **  so just return.
     */
    old_timetag = (long) rul__dyar_get_nth (old_timetags, i);
    new_timetag = rul__wm_get_time_tag (inst_ptr);
    if (new_timetag != old_timetag) 
      return;
  }
  /*
   **  If none of the objects were modified or removed, then
   **  we need to ad this insantiation to the refraction set.
   */
  rul__cs_refract_cse (winning_cse, refraction_set);
}



void rul__rac_after (long after_count, Mol_Symbol catcher_name,
		     Mol_Symbol after_rb_name, Entry_Data eb_data)
{
  *eb_data->after_count_ptr = after_count;
  *eb_data->catcher_name_ptr = catcher_name;
  *eb_data->catcher_rb_name_ptr = after_rb_name;
}



static long rul___rac_create_decl_array (long rb_count, Molecule *rb_names,
					 Molecule **db_names)
{
  Boolean        found;
  long           i, j, k, db_count;
  long           array_len = 0;
  Dynamic_Array  array = rul__dyar_create_array (10);
  Molecule      *dblknams;

  for (i = 0; i < rb_count; i++) {
    db_count = rul__rbs_dblocks (rb_names[i], &dblknams);
    for (j = 0; j < db_count; j++) {
      found = FALSE;
      for (k = 0; k < array_len; k++) {
	if (dblknams[j] == (Molecule) rul__dyar_get_nth (array, k)) {
	  found = TRUE;
	  break;
	}
      }
      if (found == FALSE) {
	rul__dyar_append (array, dblknams[j]);
	array_len++;
      }
    }
  }
  *db_names = (Molecule *) rul__dyar_get_array (array);
  return array_len;
}



Molecule rul__call_method (Mol_Symbol meth_name, long arg_count,
			   Molecule args[], Class inherit_class,
			   Entry_Data eb_data)
{
  Method            meth, gen_meth;
  Method_Func       func;
  Molecule          ret, no_meth_args[2];
  Object            wmo;
  long              i, j, k;
  char              buf[RUL_C_MAX_SYMBOL_SIZE+1];
  Decl_Domain       domain;
  Decl_Shape        shape;
  Class             rclass = inherit_class;
  Boolean           status = TRUE;
  Molecule          tmp;
  static Mol_Symbol SM__no_method = NULL;

  if (!rul__mol_is_valid (args[0]) ||
      rul__mol_get_value_type (args[0]) != instance_id) {
    rul__msg_print_w_atoms (RTS_BADMETARG, 1, meth_name);
    return NULL;
  }

  wmo = rul__mol_instance_id_value (args[0]);
  if (wmo == NULL) {
    rul__msg_print_w_atoms (RTS_NOSUCHWM, 1, args[0]);
    return NULL;
  }

  if (rclass == NULL)
    rclass = rul__wm_get_class (wmo);

  meth = rul__decl_get_class_method (rclass, meth_name);

  if (meth == NULL) { /* no method found */

    /*
     * If a system method, just return NULL
     */
    if (rul__decl_is_system_method (meth_name) != 0)
      return NULL;

    /*
     * Check for user defined $NO-METHOD-FOUND
     */
    if (SM__no_method == NULL) {
      SM__no_method = rul__mol_make_symbol ("$NO-METHOD-FOUND");
      rul__mol_mark_perm (SM__no_method);
    }
    meth = rul__decl_get_class_method (rclass, SM__no_method);

    if (meth == NULL) {
      /*
       *  No user defined $NO-METHOD-FOUND
       */
      rul__msg_print_w_atoms (RTS_NOCLSMET, 2, meth_name,
			      rul__decl_get_class_name (rclass));
      return NULL;
    }

    /*
     * Call user defined $NO-METHOD-FOUND
     */
    no_meth_args[0] = args[0];
    no_meth_args[1] = meth_name;
    rul__mol_incr_uses (meth_name);
    return (rul__call_method (SM__no_method, 2, no_meth_args, NULL, eb_data));
  }

  rul__mol_use_printform (meth_name, buf, RUL_C_MAX_SYMBOL_SIZE);
  i = rul__decl_get_method_num_params (meth);
  if (i != arg_count) {
    rul__msg_print (RTS_METARGCNT, buf, arg_count, i);
    return NULL;
  }

  status = TRUE;

  /* get and use the generic method for parameter checks */
  gen_meth = rul__decl_get_generic_method (rclass, meth_name);

  for (i = 1; i < arg_count; i++) {

    shape = rul__decl_get_method_par_shape (gen_meth, i);
    domain = rul__decl_get_method_par_domain (gen_meth, i);
    rclass = rul__decl_get_method_par_class (gen_meth, i);

    if (!rul__mol_is_valid (args[i])) {
      rul__msg_print (RTS_METINVARG, buf, i, "");
      status = FALSE;
    }
    else if (rul__mol_get_shape (args[i]) != shape) {
      rul__msg_print (RTS_METINVARG, buf, i, "shape");
      status = FALSE;
    }
    else if (!rul__mol_is_subdomain (domain, rul__mol_get_domain (args[i]))) {
      rul__msg_print (RTS_METINVARG, buf, i, "domain");
      status = FALSE;
    }

    if (status == TRUE && domain == dom_instance_id && rclass) {

      
      if (shape == shape_atom) {
	tmp = args[i];
      }
      else if (shape == shape_compound) {
	k = rul__mol_get_poly_count (args[i]);
	j = 1;
	tmp = NULL;
	if (k > 0)
	  tmp = rul__mol_get_comp_nth (args[i], 1);
      }
      else {/* shape_table */
	tmp = NULL;
      }

      while (tmp) {
	wmo = rul__mol_instance_id_value (tmp);
	if (wmo == NULL) {
	  rul__msg_print_w_atoms (RTS_NOSUCHWM, 1, args[i]);
	  return NULL;
	}
	
	if (!rul__decl_is_subclass_of (rul__wm_get_class (wmo), rclass)) {
	  rul__msg_print (RTS_METINVARG, buf, i, "class");
	  status = FALSE;
	}

	if (status == TRUE && shape == shape_compound) {
	  if (j < k)
	    tmp = rul__mol_get_comp_nth (args[i], ++j);
	  else
	    tmp = NULL;
	}
      }
    }
  }

  if (status != TRUE)
    return NULL;

  func = rul__decl_get_method_func (meth);
  ret = (*func) (arg_count, args, eb_data);

  return (ret);
}

