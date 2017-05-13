/****************************************************************************
**                                                                         **
**                         R T S _ R B S . C                               **
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
**	RULEWORKS run time system
**
**  ABSTRACT:
**	This file contains the live ruling block subsystem.
**
**  MODIFIED BY:
**	DEC	Digital Equipment Corporation
 *	CPQ	Compaq Computer Corporation
**
**  MODIFICATION HISTORY:
**
**	23-Jul-1992	DEC	Initial version
**	 4-Dec-1996	DEC	Added construct data
**	01-Dec-1999	CPQ	Release with GPL
*/



#include <common.h>
#include <beta.h>
#include <cs.h>
#include <decl.h>
#include <delta.h>
#include <dyar.h>
#include <mol.h>
#include <wm.h>
#include <rbs.h>

#define NOT_FOUND -1
#define RBLOCK_VERIFY 11931


typedef struct ruling_block {

	/*  Ruling Block:  */
	Molecule		 rb_name;
	Strategy		 rb_strategy;

	/*  Addresses of things manipulated by RHS's and RTL  */
	Molecule		*rb_return_value_loc;

	/*  Addresses of things manipulated by LHS's and RTL  */
	Conflict_Subset	 	*rb_css_loc;
	Beta_Collection		*rb_beta_loc;

	/*  Ruling Block Matches Function  */
	Matches_Function	 rb_matches_func;

	/*  Associated Set of Declaring Blocks  */
	long			 rb_dblock_count;
	Molecule		*rb_dblock_names;
	Propagate_Function	*rb_dblock_prop_funcs;

	/*  Ruling Block's Catcher List  */
	long			 rb_catcher_count;
	Molecule		*rb_catcher_names;
	Catcher_Function	*rb_catcher_funcs;

	/*  Ruling Block's Construct List  */
	long			 rb_const_count;
	Molecule		*rb_const_names;
	Construct_Type		*rb_const_types;

	/*  Info Private to this Subsystem  */
	long			 rb_use_count;
	Delta_Queue		 rb_delta_queue;
	long			 rb_last_propagated_tt;
	long			 rb_last_seen_tt;

	long			 rb_retain_memories;

#ifndef NDEBUG
	long			 verification;
#endif

} *Ruling_Block;


static	Ruling_Block		*SA__rblock_vector = NULL;
static	long			 SL__vector_length = 0;
static	long			 SL__rblocks_stored = 0;



static  Boolean is_valid_ruling_block (Ruling_Block rblock);
static  long  	position_of_name_in_vector (Molecule block_name);
static  void  	ensure_rblock_vector_has_room (void);
static  void  	do_initial_wm_propagation (Ruling_Block rblock);
static  void	do_one_propagation (Object inst_ptr);


#ifndef NDEBUG


/****************************
**                         **
**  IS_VALID_RULING_BLOCK  **
**                         **
****************************/

static Boolean is_valid_ruling_block (Ruling_Block rblock)
	/*  Use only in asserts  */
{
	return (rblock  &&  rblock->verification == RBLOCK_VERIFY);
}
#endif



/*********************************
**                              **
**  POSITION_OF_NAME_IN_VECTOR  **
**                              **
*********************************/

static long position_of_name_in_vector (Molecule block_name)
{
	long i = 0;
  
	while (i < SL__rblocks_stored) {
    	    if (block_name == SA__rblock_vector[i]->rb_name) {
		assert (is_valid_ruling_block (SA__rblock_vector[i]));
		return (i);
	    }
	    i++;
	}
	return (NOT_FOUND);
}



/************************************
**                                 **
**  ENSURE_RBLOCK_VECTOR_HAS_ROOM  **
**                                 **
************************************/

static void  ensure_rblock_vector_has_room (void)
{
	long i, start;

	if (SA__rblock_vector == NULL) {

	    /*  Never been allocated  */
	    SL__vector_length = 100;
	    SA__rblock_vector =
	    	rul__mem_malloc (SL__vector_length * sizeof(Ruling_Block));
	    for (i=0; i<SL__vector_length; i++) SA__rblock_vector[i] = NULL;

	} else if (SL__rblocks_stored == SL__vector_length) {

	    /*  Alocation wasn't big enough */
	    start = SL__vector_length;
	    SL__vector_length *= 3;
	    SA__rblock_vector = 
	    rul__mem_realloc (SA__rblock_vector, 
			  SL__vector_length * sizeof(Ruling_Block));
	    for (i=start; i<SL__vector_length; i++) SA__rblock_vector[i] = NULL;
	}

	assert (SA__rblock_vector != NULL);
}




/*******************************
**                            **
**  RUL__RBS_REGISTER_RBLOCK  **
**                            **
*******************************/

void   rul__rbs_register_rblock (
		Molecule 		 block_name, 
		Strategy		 block_strategy,
		Molecule		*return_value_loc,
		Conflict_Subset		*css_loc,
		Beta_Collection		*beta_mem_loc,
		Matches_Function	 matches_func,
		long			 retain_memories_on_exit,

		long			 dblock_count,
		Molecule		*dblock_names,
		Propagate_Function	*dblock_prop_funcs,

		long			 catcher_count,
		Molecule		*catcher_names,
		Catcher_Function	*catcher_funcs,

		long			 cons_count,
		Molecule 		*cons_names,
		Construct_Type 		*cons_types)
{
	Ruling_Block rblock;
	long i;

	i = position_of_name_in_vector (block_name);

	if (i != NOT_FOUND) {
	    /*
	    **  If this ruling block is already is use,
	    **	then increment its use count.
	    */
	    SA__rblock_vector[i]->rb_use_count += 1;
    
	} else {
	    /*
	    **  If this ruling block is not already is use,
	    **  then add it to the ruling block vector, after first
	    **  ensuring that there is enough room allocated there.
	    */
	    ensure_rblock_vector_has_room ();
    
	    /*  Now that there's enough room, create a new ruling block */
	    /*  Use calloc or init entries to NULL */
	    rblock = rul__mem_calloc (1, sizeof (struct ruling_block));
	    SA__rblock_vector[SL__rblocks_stored]= rblock;
	    SL__rblocks_stored++;

	    /*  Fill in the new Ruling_Block structure  */
	    rblock->rb_name 		 = block_name;
	    rblock->rb_strategy 	 = block_strategy;

	    rblock->rb_return_value_loc	 = return_value_loc;

	    rblock->rb_css_loc 		 = css_loc;
	    rblock->rb_beta_loc 	 = beta_mem_loc;

	    rblock->rb_dblock_count	 = dblock_count;
	    rblock->rb_dblock_names	 = dblock_names;
	    rblock->rb_dblock_prop_funcs = dblock_prop_funcs;

	    rblock->rb_matches_func	 = matches_func;

	    rblock->rb_catcher_count	 = catcher_count;
	    rblock->rb_catcher_names	 = catcher_names;
	    rblock->rb_catcher_funcs	 = catcher_funcs;

	    rblock->rb_const_count 	 = cons_count;
	    rblock->rb_const_names 	 = cons_names;
	    rblock->rb_const_types 	 = cons_types;

	    rblock->rb_retain_memories	 = retain_memories_on_exit;

	    /*  Setup activities for private info  */
    	    rblock->rb_use_count 	 = 1;
	    rblock->rb_last_propagated_tt = rul__wm_get_cur_time_tag();
	    rblock->rb_last_seen_tt	 = rblock->rb_last_propagated_tt;

	    if (css_loc) {
	      rblock->rb_delta_queue 	 = rul__delta_make_queue ();
	      *(rblock->rb_beta_loc)	 = rul__beta_make_collection ();
	      *(rblock->rb_css_loc) 	 = rul__cs_make_conflict_subset ();
	    }

#ifndef NDEBUG
	    rblock->verification 	 = RBLOCK_VERIFY; 
#endif
	    do_initial_wm_propagation (rblock);
	}
}



/*********************************
**                              **
**  RUL__RBS_UNREGISTER_RBLOCK  **
**                              **
*********************************/

void   rul__rbs_unregister_rblock (Molecule block_name)
{
	long i, j;
	Ruling_Block rblock;

	i = position_of_name_in_vector (block_name);
	assert (i != NOT_FOUND);
	assert (SA__rblock_vector[i]->rb_use_count >= 0);
	rblock = SA__rblock_vector[i];

	if (rblock->rb_use_count > 1) {
	    /*  If there were multiple uses of this block, decrement uses  */
	    rblock->rb_use_count -= 1;
      
	} else {

	    if (rblock->rb_retain_memories) {
		/*  Retain beta memories, even on exit of this block  */
		rblock->rb_use_count -= 1;
#ifndef NDEBUG
		/*  print out beta statistics if appropriate  */
		if (rblock->rb_beta_loc) {
		    rul__beta_suspend_collection (
				block_name, 
				*(rblock->rb_beta_loc));
		}
#endif
	    } else {
		/*
		**  This is the only use of this block (and this
		**  block is not tagged as one to be retained 
		**  between invocations of this block), so remove it.
		*/

	        if (rblock->rb_css_loc) {
		    /*  Do other cleanup/shutdown activities  */
		    rul__delta_free_queue (rblock->rb_delta_queue);
		    rblock->rb_delta_queue = NULL;
		    rul__beta_free_collection (block_name, 
					       *(rblock->rb_beta_loc));
		    *(rblock->rb_beta_loc) = NULL;
		    rul__cs_free_conflict_subset (*(rblock->rb_css_loc));
		    *(rblock->rb_css_loc) = NULL;
		}
#ifndef NDEBUG
		rblock->verification = RBLOCK_VERIFY; 
#endif

		/*  Now get rid of the Ruling_Block structure  */
	    	rul__mem_free (SA__rblock_vector[i]);
	    	j = i;
	    	while (j + 1 < SL__rblocks_stored) {
		    SA__rblock_vector[j] = SA__rblock_vector[j+1];
		    j++;
	    	}
	    	SL__rblocks_stored--;
	    	SA__rblock_vector[SL__rblocks_stored] = NULL;
	    }
	}
}



/***********************
**                    **
**  RUL__RBS_GET_CSS  **
**                    **
***********************/

Conflict_Subset rul__rbs_get_css (Molecule rb_name)
{
	long i = position_of_name_in_vector (rb_name);
  
	if ((i != NOT_FOUND) && (SA__rblock_vector[i]->rb_css_loc)) {
	    return (*(SA__rblock_vector[i]->rb_css_loc));
	}
	return (NULL);
}




/****************************
**                         **
**  RUL__RBS_GET_STRATEGY  **
**                         **
****************************/

Strategy rul__rbs_get_strategy (Molecule rb_name)
{
	long i = position_of_name_in_vector (rb_name);
  
	if (i != NOT_FOUND) {
	    return (SA__rblock_vector[i]->rb_strategy);
	}
	return (-1);
}




/******************************
**                           **
**  RUL__RBS_GET_RETURN_VAL  **
**                           **
******************************/

Molecule rul__rbs_get_return_val (Molecule rb_name)
{
	long i = position_of_name_in_vector (rb_name);
  
	if ((i != NOT_FOUND) && (SA__rblock_vector[i]->rb_return_value_loc)) {
	    return (*(SA__rblock_vector[i]->rb_return_value_loc));
	}
	return (NULL);
}




/**************************
**                       **
**  RUL__RBS_CONSTRUCTS  **
**                       **
**************************/

long
rul__rbs_constructs   (Molecule rb_name,
		       Molecule **rb_const_names,
		       Construct_Type **rb_const_types)
{
	long i = position_of_name_in_vector (rb_name);
  
	if (i != NOT_FOUND) {
	    *rb_const_names = SA__rblock_vector[i]->rb_const_names;
	    *rb_const_types = SA__rblock_vector[i]->rb_const_types;
	    return (SA__rblock_vector[i]->rb_const_count);
	}
	return (0);
}



/**************************
**                       **
**  RUL__RBS_CATCHERS	 **
**                       **
**************************/

long
rul__rbs_catchers  (Mol_Symbol rb_name,
		    Mol_Symbol **rb_catcher_names)
{
	long i = position_of_name_in_vector (rb_name);
  
	if (i != NOT_FOUND) {
	    *rb_catcher_names = SA__rblock_vector[i]->rb_catcher_names;
	    return (SA__rblock_vector[i]->rb_catcher_count);
	}
	return (0);
}



/***********************
**                    **
**  RUL__RBS_DBLOCKS  **
**                    **
***********************/

long
rul__rbs_dblocks (Mol_Symbol rb_name,
		  Mol_Symbol **rb_dblock_names)
{
	long i = position_of_name_in_vector (rb_name);
  
	if (i != NOT_FOUND) {
	    *rb_dblock_names = SA__rblock_vector[i]->rb_dblock_names;
	    return (SA__rblock_vector[i]->rb_dblock_count);
	}
	return (0);
}




/*****************************
**                          **
**  RUL__RBS_PRINT_MATCHES  **
**                          **
*****************************/

void rul__rbs_print_matches (Molecule rb_name, Molecule rule_name)
{
	long i = position_of_name_in_vector (rb_name);
  
	if (i != NOT_FOUND) {
	    if (SA__rblock_vector[i]->rb_matches_func != NULL) {
	        /*  invoke the supplied matches function  */
	        (SA__rblock_vector[i]->rb_matches_func) (rule_name);
	    }
	}
}




/******************************
**                           **
**  RUL__RBS_INVOKE_CATCHER  **
**                           **
******************************/

long rul__rbs_invoke_catcher (Mol_Symbol catcher_name,
			      Mol_Symbol rb_name,
			      Entry_Data eb_data)
{
  long i, j;
  Ruling_Block rblock;
  long status = RUL__C_SUCCESS;

  i = position_of_name_in_vector (rb_name);
  if (i != NOT_FOUND) {

    rblock = SA__rblock_vector[i];
    
    for (j = 0; j < rblock->rb_catcher_count; j++) {
      if (catcher_name == rblock->rb_catcher_names[j]) {
	if (rblock->rb_catcher_funcs[j] != NULL) {
	  status = (rblock->rb_catcher_funcs[j]) (eb_data);
	}
      }
    }
  }
  return status;
}




/*******************************
**                            **
**  RUL__RBS_NOTIFY_WM_DELTA  **
**                            **
*******************************/

void rul__rbs_notify_wm_delta (Object inst_ptr, Token_Sign sign)
{
	long i, j;
	Ruling_Block rblock;
	Molecule dblock_name;

	/*  Fetch the declaring block for this instance  */
	dblock_name = rul__decl_get_class_block_name (
				      rul__wm_get_class (inst_ptr));

	/*  Now, for all ruling-blocks, check each dblock_name  */
	for (i = 0; i < SL__rblocks_stored; i++) {

	    rblock = SA__rblock_vector[i];

	    for (j=0; j<rblock->rb_dblock_count; j++) {
	        if (dblock_name == rblock->rb_dblock_names[j] &&
		    rblock->rb_delta_queue != NULL) {
		    /*
		    **  Found a ruling block that cares about changes
		    **  to WMO's like the one received.  Add it to that
		    **  ruling block's local delta queue.
		    */
		    rul__delta_add_to_queue (
				rblock->rb_delta_queue,
				rblock->rb_last_propagated_tt,
				sign,
				rul__wm_get_instance_id (inst_ptr),
				rul__wm_get_time_tag (inst_ptr));

		    if (sign == DELTA__C_SIGN_POSITIVE) {
			rblock->rb_last_seen_tt =
				rul__wm_get_time_tag (inst_ptr);
		    }
		}
	    }
	}
}




/************************************
**                                 **
**  RUL__RBS_UPDATE_MATCH_NETWORK  **
**                                 **
************************************/

void rul__rbs_update_match_network (Mol_Symbol rb_name, long target_tt)
{
	long i, j;
	Ruling_Block rblock;
	Molecule dblock_name;
	Delta_Token delta_tok;

	i = position_of_name_in_vector (rb_name);
	if (i != NOT_FOUND) {

	    rblock = SA__rblock_vector[i];

	    /*  Get first Delta_Token  */
	    if (rblock->rb_delta_queue)
	        delta_tok = rul__delta_get_from_queue (rblock->rb_delta_queue, 
						       target_tt);
	    else
	        delta_tok = NULL;

	    while (delta_tok) {

		dblock_name = rul__decl_get_class_block_name (
						    delta_tok->instance_class);

		for (j=0; j<rblock->rb_dblock_count; j++) {
		    if (dblock_name == rblock->rb_dblock_names[j]) {
		        if (rblock->rb_dblock_prop_funcs[j] != NULL) {
			    /*  Invoke the supplied propagate function  */
			    (rblock->rb_dblock_prop_funcs[j]) (delta_tok);
			}
			break; /* kick out of the for-loop once it's found */
		    }
		}
		if (rul__wm_is_snapshot (delta_tok->instance)) {
		    rul__wm_destroy_snapshot (delta_tok->instance);
		}

		/*  Free old delta token  */
		rul__mol_decr_uses (delta_tok->instance_id);
		rul__mem_free (delta_tok);

		/*  Get next Delta_Token  */
		delta_tok = rul__delta_get_from_queue (
					rblock->rb_delta_queue, 
					target_tt);
	    }
	}
}


/*
 * Invalidate cached refraction info for all conflict subsets which
 * are caching info for this refraction set.
 */
void
rul__rbs_invalidate_ref_caches (Refraction_Set ref_set)
{
    long i;
    Conflict_Subset *css_ptr;

    for (i = 0; i < SL__rblocks_stored; i++) {
	css_ptr = SA__rblock_vector[i]->rb_css_loc;
	if (css_ptr != NULL)
	    rul__cs_invalidate_ref_cache(ref_set, *css_ptr);
    }
}



/*
**  The following static variables are used to communicate 
**  extra information between the following two functions.
*/
static long 		SL_cur_dblock_index;
static Ruling_Block 	SA_cur_rblock = NULL;

typedef struct prop_context {
	long 		current_dblock_index;
	Ruling_Block	current_ruling_block;
} * Prop_Context;

static Prop_Context 	SA_cur_context = NULL;
static Dynamic_Array	SA_prev_contexts = NULL;


/********************************
**                             **
**  DO_INITIAL_WM_PROPAGATION  **
**                             **
********************************/

static void do_initial_wm_propagation (Ruling_Block rblock)
{
	long i, end;

	assert (is_valid_ruling_block (rblock));

	if (SA_cur_context != NULL) {
	    /*  If necessary, stack up the previous context  */
	    if (SA_prev_contexts == NULL) {
		SA_prev_contexts = rul__dyar_create_array (10);
	    }
	    rul__dyar_append (SA_prev_contexts, SA_cur_context);
	}

	/*  Now set up the current context */
	SA_cur_context = (Prop_Context) 
			rul__mem_malloc (sizeof (struct prop_context));
	SA_cur_context->current_ruling_block = rblock;

	/*  Now do all the propagations within this context  */
	end = rblock->rb_dblock_count;
	for (i=0; i<end; i++) {
	    SA_cur_context->current_dblock_index = i;
	    rul__wm_for_each_object_from_db (
			rblock->rb_dblock_names[i],
			do_one_propagation);
	}

	/* Now cleanup */
	rul__mem_free (SA_cur_context);
	if (SA_prev_contexts != NULL  &&  
		rul__dyar_get_length (SA_prev_contexts) > 0) {
	    /*  If necessary, restore previous context  */
	    SA_cur_context = rul__dyar_pop_last (SA_prev_contexts);
	    assert (
		is_valid_ruling_block (SA_cur_context->current_ruling_block));
	} else {
	    SA_cur_context = NULL;
	}
}


/*************************
**                      **
**  DO_ONE_PROPAGATION  **
**                      **
*************************/

static void do_one_propagation (Object inst_ptr)
{
	struct delta_token 	delta;
	Ruling_Block		rblock;
	long 			index;

	delta.sign = DELTA__C_SIGN_POSITIVE;
    	delta.instance = inst_ptr;
    	delta.instance_class = rul__wm_get_class (inst_ptr);
    	delta.instance_id = rul__wm_get_instance_id (inst_ptr);

	rblock = SA_cur_context->current_ruling_block;
	assert (is_valid_ruling_block (rblock));
	index = SA_cur_context->current_dblock_index;

	if (rblock->rb_dblock_prop_funcs[index]) {
	    (rblock->rb_dblock_prop_funcs[index]) (&delta);
	}
}
