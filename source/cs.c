/* rts_cs.c - Conflict Set routines */
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
 *  FACILITY:
 *	RULEWORKS run time system
 *
 *  ABSTRACT:
 *	This module contains the routines that maintain the conflict
 *	set.  The conflict set doesn't exist explicitly as a data
 *	structure, but instead is composed of the conflict subsets
 *	(CSSes) of all the ruling blocks associated with an entry
 *	block (i.e., the active ruling blocks), minus instantiations
 *	which are in the entry block's refraction set.
 *
 *	Instantiations are added to and removed from the conflict
 *	subsets by calls to rul__cs_modify from the match network
 *	terminal nodes.  The recognize-act cycle calls
 *	rul__cs_get_winner to determine the best instantiation which
 *	hasn't already fired.
 *	
 *
 *  MODIFIED BY:
 *	DEC	Digital Equipment Corporation
 *	CPQ	Compaq Computer Corporation
 *
 *  MODIFICATION HISTORY:
 *
 *	14-Sep-1992	DEC	Initial version
 *	21-Jul-1993	DEC	Switch to a hash table for quick removals
 *  25-Nov-1996 DEC  Fix read on CSE after it has been freed
 */

#include <common.h>
#include <mol.h>		/* For rul__mol_instance_id_value */
#include <rbs.h>		/* For Ruling_Block_Func */
#include <rac.h>		/* For RUL__C_RETURN */
#include <cs.h>			/* This module's external spec */
#include <ref.h>		/* For rul__ref_is_refracted() */
#include <wm.h>			/* For rul__wm_get_time_tag() */
#include <dbg.h>		/* declare debugger stuff */
#include <i18n.h>		/* For defined constant title strings */
#include <ios.h>		/* defines IO_Stream stuff */


#ifdef DEBUG_CS
#define assert_cs(x)          assert(x)
#define CSE_VERIFY      10671
#define CSS_VERIFY      10762

#else
#define assert_cs(x)          ;
#endif

#include <btree.h>

#define CS__C_POS_HT_SIZE 1327

#define MOLS_ORDERED_HASH_INCR(hsh,mol)   ((hsh + 1) ^ MOL_TO_HASH_NUMBER(mol))
		/* add 1, then bitwise XOR (left associative) */

#define HASH_NUM_TO_INDEX(hsh)  (hsh % CS__C_POS_HT_SIZE)

/* Value indicating cached refraction info is invalid */
#define INVALID_CACHE  ((Refraction_Set) -1)



/*
 * The conflict_set_entry represents an instantiation, and has information
 * needed for conflict resolution (specificities), and functions needed by
 * the recognize-act cycle.
 */
struct conflict_set_entry {
	/*
	**  The following fields are needed by the recognize act
	**  cycle when this instantiation has been selected for firing
	*/
	Rule_RHS_Func 		 rhs_func;		/* To fire rule */
	Mol_Symbol		 rb_name;		/* for RETURN value */
	/*
	**  These fields are used to maintain the interlocked
	**  list and hash table that make up a Conflict_Subset
	*/
	Conflict_Subset		   css;
	unsigned long		   hash_number;
	struct conflict_set_entry *hash_chain;
	Boolean			   is_refracted;
	struct bnode               cse_bnode;
#ifdef DEBUG_CS
        long			 verification ;
#endif
	/*
	**  This set of fields stores the instantiation plus
	**  associated information needed for conflict resolution
	*/
	unsigned long		 cse_number;
	Mol_Symbol		 rule_name;
	long			 inst_id_count;
	long			 class_specif;
	long			 test_specif;
	long			*timetags;
	long			*ordered_timetags;
	Mol_Instance_Id		 inst_ids[1];
};


struct conflict_subset {

	Conflict_Set_Entry	*hash_vector;
	Bnode                    treehead;
	Refraction_Set		 cached_ref_set; /* Refraction set which */
						 /* is_refracted fields of */
						 /* CSEs are caching */
#ifdef DEBUG_CS
	long			 verification;
#endif
};


static unsigned long SL_next_cse_number = 1;

/*
 * Forward declarations
 */

static Boolean		  better_cse (
				const Conflict_Set_Entry better, 
				const Conflict_Set_Entry worse);
static Conflict_Set_Entry cs_find_best_cse (
				const Conflict_Subset css,
				const Refraction_Set rs);
static void		  cs_free_cse (Conflict_Set_Entry cse);
static Conflict_Set_Entry cs_make_cse (
				const Conflict_Subset css,
				const Mol_Symbol rule_name,
				const Mol_Symbol rb_name,
				const Rule_RHS_Func rhs_func,
				const long class_specificity,
				const long test_specificity,
				const long inst_id_count,
				const Mol_Instance_Id inst_id_array []);
static Conflict_Set_Entry cs_remove_cse (
				const Conflict_Subset css,
				const Mol_Symbol rb_name,
				const Mol_Symbol rule_name,
				const long inst_id_count,
				const Mol_Instance_Id inst_id_array []);

#ifdef DEBUG_CS
#define DONT_FREE_CSES

static Boolean		  is_valid_conflict_subset (Conflict_Subset css);
static Boolean		  is_valid_conflict_set_entry (Conflict_Set_Entry cse);

#endif



#ifdef DEBUG_CS

/*
**  The following should ONLY appear inside assert_cs()
*/

static Boolean
is_valid_conflict_subset (Conflict_Subset css)
{
	return (css  &&  css->verification == CSS_VERIFY);
}

static Boolean
is_valid_conflict_set_entry (Conflict_Set_Entry cse)
{
	if (cse == NULL) {
	    rul__ios_printf (RUL__C_STD_ERR,
		"\n Internal Error:  null CSE received.\n");
	} else if ((cse->verification * -1) == CSE_VERIFY) {
	    rul__ios_printf (RUL__C_STD_ERR,
		"\n Internal Error:  access to already freed CSE, ");
	    rul__mol_print_readform (cse->rule_name, RUL__C_STD_ERR);
	    rul__ios_printf (RUL__C_STD_ERR, "\n");
	} 
	return (cse && cse->verification == CSE_VERIFY);
}

#endif


long
cse_alloc (void *key, Bnode *new_node, void *opt_data)
{
  Conflict_Set_Entry cse = (Conflict_Set_Entry) key;

  *new_node = &cse->cse_bnode;
  (*new_node)->node_ptr = key;
  return TRUE;
}

long
cse_dealloc (Bnode node, void *opt_data)
{
  Conflict_Set_Entry cse;

  cse = (Conflict_Set_Entry) node->node_ptr;
  cs_free_cse (cse);
  return TRUE;
}

long
cse_compare (void *key, Bnode new_node, void *opt_data)
{
  Conflict_Set_Entry cse, new_cse;
  long               i, num_timetags;

  new_cse = (Conflict_Set_Entry) new_node->node_ptr;
  cse = (Conflict_Set_Entry) key;

  if (new_cse == cse)
    return 0;

  /*
   * The timetags arrays in the CSEs are sorted according to the conflict
   * resolution strategy when they're created, so we just compare down the
   * arrays until we find one which is newer than the other.
   */
  num_timetags = MIN(new_cse->inst_id_count, cse->inst_id_count);

  for (i=0; i<num_timetags; i++) {
    if (new_cse->timetags[i] != cse->timetags[i]) {
      if (new_cse->timetags[i] > cse->timetags[i]) return  1;
      if (new_cse->timetags[i] < cse->timetags[i]) return -1;
    }
  }

  /*
   * If we made it here, the first  n  CEs matched the same WMOs.  Whoever
   * has more CEs wins.
   */
  if (new_cse->inst_id_count > cse->inst_id_count) return  1;
  if (new_cse->inst_id_count < cse->inst_id_count) return -1;

  /*
   * The two instances match exactly the same WMOs.  Check specificity.
   */
  if (new_cse->class_specif > cse->class_specif) return  1;
  if (new_cse->class_specif < cse->class_specif) return -1;

  if (new_cse->test_specif > cse->test_specif) return  1;
  if (new_cse->test_specif < cse->test_specif) return -1;

  /*
   * Wow, both rules match exactly the same WMOs, with the same
   * specificity.
   * Do something arbitrary, but repeatable and invertible.
   */
  if (new_cse->cse_number < cse->cse_number) return 1;
  return -1;
}

static Conflict_Set_Entry Best_tree_cse;
static IO_Stream          Print_cs_ios;
static Mol_Symbol         Print_rb_name;
static Mol_Symbol         Print_rule_name;
static Boolean            Printed_header;

static long
cse_find_best (Bnode node, void *data)
{
   Conflict_Set_Entry tmp_cse;

   tmp_cse = (Conflict_Set_Entry) node->node_ptr;
   if (!tmp_cse->is_refracted) {
      Best_tree_cse = tmp_cse;
      return FALSE;
   }
   return TRUE;
}

static long
cs_reset_cse_refractions (Bnode node, void *data)
{
   Conflict_Set_Entry cse = (Conflict_Set_Entry) node->node_ptr;
   Refraction_Set     rs = (Refraction_Set) data;

   cse->is_refracted = rul__ref_is_refracted (rs, cse);
   return TRUE;
}

static long
cs_print_cs (Bnode node, void *data)
{
   Conflict_Set_Entry cse = (Conflict_Set_Entry) node->node_ptr;
   Refraction_Set     rs = (Refraction_Set) data;

   assert_cs (is_valid_conflict_set_entry (cse));
   if (!rul__ref_is_refracted (rs, cse)) {
	       
      if (rul__ios_curr_column (Print_cs_ios))
	rul__ios_print_newline (Print_cs_ios);
      rul__ios_printf (Print_cs_ios, "   ");
      rul__cs_print_one_cse (cse, Print_cs_ios);
   }
   return TRUE;
}

static long
cs_print_matches (Bnode node, void *data)
{
   long j;
   Conflict_Set_Entry cse = (Conflict_Set_Entry) node->node_ptr;
   Refraction_Set     rs = (Refraction_Set) data;

   assert_cs (is_valid_conflict_set_entry (cse));
	       
   if (cse->rule_name == Print_rule_name &&
       cse->rb_name == Print_rb_name &&
       !rul__ref_is_refracted (rs, cse)) {

      /*  Found one to print  */

      if (!Printed_header) {
	 rul__ios_print_newline (Print_cs_ios);
	 rul__ios_printf (Print_cs_ios, "   *** complete instantiations ***");
	 rul__ios_print_newline (Print_cs_ios);
	 Printed_header = TRUE;
      }

      /*  So print it!  */
      rul__ios_printf (Print_cs_ios, "   ");
      for (j = 0; j < cse->inst_id_count; j++) {
	 rul__ios_printf (Print_cs_ios, "  ");
	 rul__mol_print_readform (cse->inst_ids[j], Print_cs_ios);
      }
      rul__ios_print_newline (Print_cs_ios);
   }
   return TRUE;
}


/***********************************
**                                **
**  RUL__CS_MAKE_CONFLICT_SUBSET  **
**                                **
***********************************/

Conflict_Subset
rul__cs_make_conflict_subset (void)
{
	Conflict_Subset css;
	long i;

	css = rul__mem_malloc (sizeof (struct conflict_subset));
	assert_cs (css != NULL);

	css->treehead = NULL;
	css->cached_ref_set =  NULL;

	css->hash_vector = rul__mem_calloc (1, 
			    CS__C_POS_HT_SIZE * sizeof (Conflict_Set_Entry));
	assert_cs (css->hash_vector != NULL);

	/* use calloc... 
	 * for (i = 0; i < CS__C_POS_HT_SIZE; i++)
	 *   css->hash_vector[i] = NULL;
	 */

#ifdef DEBUG_CS
	css->verification = CSS_VERIFY;
#endif
	return (css);
}




/***********************************
**                                **
**  RUL__CS_FREE_CONFLICT_SUBSET  **
**                                **
***********************************/

void
rul__cs_free_conflict_subset (Conflict_Subset css)
{
	Conflict_Set_Entry cse, cse_tmp;

	assert_cs (is_valid_conflict_subset (css));

	
	rul__btree_purge_tree (&css->treehead, cse_dealloc, NULL);

#ifdef DEBUG_CS
	css->verification = 0;
#endif
	rul__mem_free (css->hash_vector);
	rul__mem_free (css);
}




/***********************************
**                                **
**  RUL__CS_COMPUTE_CSE_HASH_NUM  **
**                                **
***********************************/

unsigned long
rul__cs_compute_cse_hash_num (const Mol_Symbol rule_name,
			      const long inst_id_count,
			      const Mol_Instance_Id inst_ids [])
{
	unsigned long hsh;
	long i;

	hsh = MOL_TO_HASH_NUMBER(rule_name);
	for (i=0; i<inst_id_count; i++) {
	    hsh = MOLS_ORDERED_HASH_INCR(hsh,inst_ids[i]);
	}
	return (hsh);
}




/******************
**               **
**  CS_FREE_CSE  **
**               **
******************/

static void
cs_free_cse (Conflict_Set_Entry cse)
{
#ifdef DEBUG_CS
	cse->verification = CSE_VERIFY * -1;
#endif
#ifndef DONT_FREE_CSES
	rul__mem_free (cse);
#endif
}



/******************
**               **
**  CS_MAKE_CSE  **
**               **
******************/

static Conflict_Set_Entry
cs_make_cse (const Conflict_Subset css,
	     const Mol_Symbol rule_name,
	     const Mol_Symbol rb_name,
	     const Rule_RHS_Func rhs_func,
	     const long class_specif,
	     const long test_specif,
	     const long inst_id_count,
	     const Mol_Instance_Id inst_id_array [])
{
	Strategy strategy = rul__rbs_get_strategy (rb_name);
	Conflict_Set_Entry cse;
	long i, j, start, higher, index;
	Object wmo;
	Boolean pending_deletion = FALSE;
	Bnode         cse_node;

	assert_cs (is_valid_conflict_subset (css));
	assert_cs (inst_id_count > 0);


	/*
	 * Because memory allocation is expensive, first
	 * check to see if we need to allocate any
	 */
	/*
	 **  If any of the WMO's was found not to exist, then this
	 **  instantiation would have been removed before conflict
	 **  resolution completed, so we won't even bother to create it.
	 */
        for (i=0; i<inst_id_count; i++) {
	  if (rul__mol_instance_id_value (inst_id_array[i]) == NULL)
	    return (NULL);
	}
	
	/*
	 **  We allocate the CSE, including the two timetag arrays
	 **  and the instance_id arrays, as a single allocation unit.
	 */
	cse = (Conflict_Set_Entry) rul__mem_malloc (
			      sizeof(struct conflict_set_entry) +
			      (2 * inst_id_count * sizeof(long)) +
			      ((inst_id_count - 1) * sizeof(Mol_Instance_Id)));
	/*
	 **  Set the timetag array pointers to point to the right
	 **  place within the single allocted block of memory.
	 */
	cse->timetags		= (long *) &(cse->inst_ids[inst_id_count]);
	cse->ordered_timetags	= &(cse->timetags[inst_id_count]);
	  
	/*
	 **  Next Copy all the instance ids, and fetch the timetags.
	 */
	for (i=0; i<inst_id_count; i++) {
	  cse->inst_ids[i] = inst_id_array[i];
	  wmo = rul__mol_instance_id_value (inst_id_array[i]);
	  cse->timetags[i] = rul__wm_get_time_tag (wmo);
	  cse->ordered_timetags[i] = cse->timetags[i];
	}


	/*
	 **  Now that we know this CSE is for real, fill in all 
	 **  the other values in the structure.
	 */
	cse->rule_name		  = rule_name;
	cse->rb_name		  = rb_name;
	cse->rhs_func		  = rhs_func;
	cse->class_specif	  = class_specif;
	cse->test_specif	  = test_specif;
	cse->inst_id_count	  = inst_id_count;
	cse->cse_bnode.node_left  = NULL;
	cse->cse_bnode.node_right = NULL;
	cse->cse_bnode.node_bal   = 0;
	cse->cse_bnode.node_ptr   = NULL;
	cse->css		  = css;
	cse->hash_number 	  = rul__cs_compute_cse_hash_num (rule_name,
						inst_id_count, inst_id_array);

	/*  Give this CSE a serial number  */
	cse->cse_number 	= SL_next_cse_number;
	SL_next_cse_number++;

#ifdef DEBUG_CS
	cse->verification	= CSE_VERIFY;
#endif

	/*
	**  Once this CSE is all set up, make sure that this CSE
	**  is not in the current refraction set (if any).
	*/
	/* If cache has been invalidated, do nothing. */
	if (css->cached_ref_set != INVALID_CACHE) {
	    if (css->cached_ref_set) {
		cse->is_refracted = rul__ref_is_refracted (css->cached_ref_set,
							   cse);
	    }
	    else {
		cse->is_refracted	= FALSE;
	    }
	}

	/*
	** Bubble sort the timetags of the objects, for conflict resolution.
	*/
	if (strategy == CS__C_MEA)
	  start = 1;		/* For MEA, sort all but MEA time tag */
	else
	  start = 0;		/* For LEX, sort all time tags */
    
	for (i = start; i < inst_id_count-1; i++) {
	  for (j = i+1; j < inst_id_count; j++) {
	    if (cse->timetags[j] > cse->timetags[i]) {
	      higher = cse->timetags[j];
	      cse->timetags[j] = cse->timetags[i];
	      cse->timetags[i] = higher;
	    }
	  }
	}

	/*
	 **  Insert the new CSE into the hash table
	 */
	index = HASH_NUM_TO_INDEX(cse->hash_number);
	cse->hash_chain = css->hash_vector[index];
	css->hash_vector[index] = cse;
	
	/*
	**  Insert the new CSE into the btree
	*/
	rul__btree_insert_tree (&css->treehead, cse, 0, cse_compare,
				cse_alloc, &cse_node, NULL);
	return (cse);
}





/********************
**                 **
**  CS_REMOVE_CSE  **
**                 **
********************/

static Conflict_Set_Entry
cs_remove_cse (const Conflict_Subset css,
	       const Mol_Symbol rb_name,
	       const Mol_Symbol rule_name,
	       const long inst_id_count,
	       const Mol_Instance_Id inst_ids [])
{
   unsigned long hsh;
   long index, i;
   Boolean matched;
   Conflict_Set_Entry cse, prev_cse, new_cse, new_prev_cse;

   assert_cs (is_valid_conflict_subset (css));

   hsh = rul__cs_compute_cse_hash_num (rule_name,inst_id_count,inst_ids);
   index = HASH_NUM_TO_INDEX(hsh);

   cse = css->hash_vector[index];
   prev_cse = NULL;

   while (cse != NULL) {
      /*
       **  Traverse the hash chain looking for the matching CSE
       */
      new_prev_cse = cse;
      new_cse = cse->hash_chain;
      if (hsh == cse->hash_number) {
	 if (rule_name == cse->rule_name && rb_name == cse->rb_name) {
	    matched = TRUE;
	    i = inst_id_count - 1;
	    while (matched  &&  i >= 0) {
	       if (inst_ids[i] != cse->inst_ids[i]) matched = FALSE;
	       i--;
	    }
	    if (matched) {
	       /*
		**  Debugger has TRACE on cs changes ?
		*/
	       if (rul__dbg_gl_enable & DBG_M_ENABLE_TRA) {
		  if (rul__dbg_gl_trace & DBG_M_TRACE_CS) {
		     if (rul__ios_curr_column (RUL__C_DEF_TRACE))
		       rul__ios_print_newline (RUL__C_DEF_TRACE);
		     rul__ios_printf (RUL__C_DEF_TRACE, "   <=CS:  ");
		     rul__cs_print_one_cse (cse, RUL__C_DEF_TRACE);
		  }
	       }
	       
	       /*
		**  Once the matching CSE is found, remove it
		**  from the hash table.
		*/
	       if (prev_cse == NULL) {
		  css->hash_vector[index] = cse->hash_chain;
	       } else {
		  prev_cse->hash_chain = cse->hash_chain;
	       }
	       /* remove from b-tree */
	       rul__btree_remove_tree (&css->treehead, cse,
				       cse_compare, cse_dealloc, NULL);
                   new_prev_cse = prev_cse;
 	    }
	 }
      }
      prev_cse = new_prev_cse;
      cse = new_cse;
   }
   return (NULL);
}




/**************************
**                       **
**  RUL__CS_REFRACT_CSE  **
**                       **
**************************/

void
rul__cs_refract_cse (Conflict_Set_Entry cse, Refraction_Set rs)
{
   Conflict_Subset css;

   assert_cs (is_valid_conflict_set_entry (cse));
   css = cse->css;
   assert_cs (is_valid_conflict_subset (css));

   /*  Stash which refraction set the cache is good for  */
   if (css->cached_ref_set != rs) {
      if (css->cached_ref_set != NULL) {
	 rul__btree_traverse_tree (&css->treehead, 
				   cs_reset_cse_refractions, rs);
      }
      css->cached_ref_set = rs;
   }

   /*  Always add it to the refraction set, and tag it  */
   rul__ref_add_entry (rs, cse);
   cse->is_refracted = TRUE;
}




/***********************************
**				  **
**  RUL__CS_INVALIDATE_REF_CACHE  **
**				  **
***********************************/
/*
 * If this conflict subset has cached refraction information for the given
 * refraction set, forget that refraction information.
 */
void
rul__cs_invalidate_ref_cache(Refraction_Set rs, Conflict_Subset css)
{
    if (rs == css->cached_ref_set) {
	css->cached_ref_set = INVALID_CACHE;
    }
}




/*********************
**                  **
**  RUL__CS_MODIFY  **
**                  **
*********************/

void 
rul__cs_modify (Conflict_Subset css, Token_Sign sign, Mol_Symbol rule_name,
		Mol_Symbol rb_name, Rule_RHS_Func rhs_func,
		long class_specif,	long test_specif,
		long inst_id_count, Mol_Instance_Id inst_id_array [])
{
   /*
    **  Terminal nodes of the network call this function to modify the
    **  conflict set.  For each call, one conflict set entry (cse) will
    **  be created or deleted.  Most of the parameters are used to make
    **  up new cse's.
    */
   Conflict_Set_Entry cse;

   assert_cs (is_valid_conflict_subset (css));

   if (sign == DELTA__C_SIGN_NEGATIVE) {
      
      /*  For removes, use hash table to find and remove the match  */
      cse = cs_remove_cse (css, rb_name, rule_name, 
			   inst_id_count, inst_id_array);
      
   } else {

      /*  For makes, create it and insert it into the list and table */
      cse = cs_make_cse ( css, rule_name, rb_name, rhs_func,
			 class_specif, test_specif,
			 inst_id_count, inst_id_array);
      if (cse == NULL)
	return;

      /*
       **  Debugger has TRACE on cs changes ?
       */
      if (rul__dbg_gl_enable & DBG_M_ENABLE_TRA) {
	 if (rul__dbg_gl_trace & DBG_M_TRACE_CS) {
	    if (rul__ios_curr_column (RUL__C_DEF_TRACE))
	      rul__ios_print_newline (RUL__C_DEF_TRACE);
	    rul__ios_printf (RUL__C_DEF_TRACE, "   =>CS:  ");
	    rul__cs_print_one_cse (cse, RUL__C_DEF_TRACE);
	 }
      }
   }
}




/***********************
**                    **
**  CS_FIND_BEST_CSE  **
**                    **
***********************/

static Conflict_Set_Entry
cs_find_best_cse (const Conflict_Subset css,
		  const Refraction_Set rs)
{
	Conflict_Set_Entry next_best, tmp_cse;

	assert_cs (is_valid_conflict_subset (css));

	if (css->cached_ref_set == NULL) {
	    css->cached_ref_set = rs;
	} else {
	    if (rs != css->cached_ref_set) {
	       rul__btree_traverse_tree (&css->treehead, 
					 cs_reset_cse_refractions, rs);
	    }
	}

	Best_tree_cse = NULL;
	rul__btree_traverse_tree (&css->treehead, cse_find_best, NULL);
	return (Best_tree_cse);
}





/*************************
**                      **
**  RUL__CS_GET_WINNER  **
**                      **
*************************/

Conflict_Set_Entry
rul__cs_get_winner (Conflict_Subset css_array[], 
		    long css_count,
		    Refraction_Set rs)
{
    	/*
     	**  We're passed an array of conflict subsets (one per
	**  associated ruling block) and we need to find the 
     	**  best of the best cse.
	*/

    	Conflict_Set_Entry best_cse, tmp_cse;
    	long	i;

	best_cse = NULL;
	for (i = 0; i < css_count; i++) {
	    if (css_array[i]) {
	        tmp_cse = cs_find_best_cse (css_array[i], rs);
		if (tmp_cse != NULL) {
		    if (best_cse == NULL) {
		        best_cse = tmp_cse;
		    } else {
		        if (better_cse (tmp_cse, best_cse)) {
			    best_cse = tmp_cse;
			}
		    }
		}
	    }
	}
        return (best_cse);
}




/*****************
**              **
**  BETTER_CSE  **
**              **
*****************/

static Boolean
better_cse (const Conflict_Set_Entry better, 
	    const Conflict_Set_Entry worse)
{
    /* Return TRUE iff cse in first param is better than second param. */
    long i, num_timetags;

    assert_cs (is_valid_conflict_set_entry (better));
    assert_cs (is_valid_conflict_set_entry (worse));

    /*
     * The timetags arrays in the CSEs are sorted according to the conflict
     * resolution strategy when they're created, so we just compare down the
     * arrays until we find one which is newer than the other.
     */
    num_timetags = MIN(better->inst_id_count, worse->inst_id_count);

    for (i=0; i<num_timetags; i++) {
	if (better->timetags[i] != worse->timetags[i]) {
	    if (better->timetags[i] > worse->timetags[i]) return TRUE;
	    if (better->timetags[i] < worse->timetags[i]) return FALSE;
	}
    }

    /*
     * If we made it here, the first  n  CEs matched the same WMOs.  Whoever
     * has more CEs wins.
     */
    if (better->inst_id_count > worse->inst_id_count) return TRUE;
    if (better->inst_id_count < worse->inst_id_count) return FALSE;

    /*
     * The two instances match exactly the same WMOs.  Check specificity.
     */
    if (better->class_specif > worse->class_specif) return TRUE;
    if (better->class_specif < worse->class_specif) return FALSE;

    if (better->test_specif > worse->test_specif) return TRUE;
    if (better->test_specif < worse->test_specif) return FALSE;

    /*
     * Wow, both rules match exactly the same WMOs, with the same
     * specificity.
     * Do something arbitrary, but repeatable and invertible.
     */
    return (better->cse_number < worse->cse_number);
}





/****************************
**                         **
**  RUL__CS_CALL_RHS_FUNC  **
**                         **
****************************/

long
rul__cs_call_rhs_func (Conflict_Set_Entry cse, Entry_Data eb_data)
{
    /*
    **  Call the RHS functions for the rule in the cse.  If it executes 
    **  the RETURN action, store the return value in the second parameter.
    */
    long status;
    
    assert_cs (is_valid_conflict_set_entry (cse));

    /* Call the RHS. */
    status = (*cse->rhs_func) (cse, eb_data);

    return status;
}





/***********************
**                    **
**  RUL__CS_PRINT_CS  **
**                    **
***********************/

void 
rul__cs_print_cs (Conflict_Subset css_array[], long css_count,
		  Refraction_Set rs, IO_Stream ios)
{
   /*
    **  It would be nice to print the conflict set in order.  Currently,
    **  it's printed in order within CSSes.  We could print it in order by 
    **  creating our own refraction set and passing that to 
    **  RUL__cs_get_winner() and rul__ref_add_entry() repeatedly, and 
    **  then freeing the refraction set.
    */
   long i;
   Conflict_Set_Entry cse;

   for (i = 0; i < css_count; i++) {
      
      if (css_array[i]) {
	 /* We have an index into css_array */

	 Print_cs_ios = ios;
	 rul__btree_traverse_tree (&css_array[i]->treehead, cs_print_cs, rs);
      }
   }
}





/****************************
**                         **
**  RUL__CS_PRINT_ONE_CSE  **
**
**
****************************/

void
rul__cs_print_one_cse (Conflict_Set_Entry cse, IO_Stream ios)
{
    /*
    **	Prints the specified confict set entry.
    **	Note, it may be preceeded by a <= or a >= when called
    **	as a result of the trace cs command.
    */
    long j, tt;

    assert_cs (is_valid_conflict_set_entry (cse));

    if (rul__dbg_gl_enable & DBG_M_ENABLE_BKN) {
       rul__mol_print_readform (cse->rb_name, ios);
       rul__ios_printf (ios, "~");
    }
    rul__mol_print_readform (cse->rule_name, ios);

    for (j = 0; j < cse->inst_id_count; j++) {

	if (j==0) {
	    rul__ios_printf (ios, " ");
	} else {
	    rul__ios_printf (ios, "  ");
	}
	rul__mol_print_readform (cse->inst_ids[j], ios);
	tt = cse->ordered_timetags[j];
	if (tt != 0) {
	    rul__ios_printf (ios, " %ld", tt);
	}
    }
    rul__ios_print_newline (ios);
}



#ifndef NDEBUG


/*********************
**                  **
**  RUL__CSE_PRINT  **
**                  **
*********************/

#ifdef __VMS
void
rul__cse_print (Conflict_Set_Entry *cse)
{
 	rul__cs_print_one_cse (*cse, RUL__C_STD_ERR);
}
#else
void
rul__cse_print (Conflict_Set_Entry cse)
{
 	rul__cs_print_one_cse (cse, RUL__C_STD_ERR);
}
#endif
#endif /* ifndef NDEBUG */





/****************************
**                         **
**  RUL__CS_PRINT_MATCHES  **
**                         **
****************************/

void
rul__cs_print_matches (Mol_Symbol rb_name, Mol_Symbol rule_name,
		       Conflict_Subset css_array[],
		       long css_count, Refraction_Set rs, IO_Stream ios)
{
   /*
    **  This routine prints the complete matches for a rule. 
    **  It's to be executed after the rul__rbs_print_matches call.
    */
   long i, j, printed_header = 0;
   Conflict_Set_Entry cse;
   Printed_header = FALSE;
   
   for (i = 0; i < css_count; i++) {
      
      if (css_array[i]) {
	 
	 Print_rb_name = rb_name;
	 Print_rule_name = rule_name;
	 Print_cs_ios = ios;
	 rul__btree_traverse_tree (&css_array[i]->treehead,
				   cs_print_matches, rs);
      }
   }
   
   if (!Printed_header)
     rul__ios_print_newline (ios);
}


/*
**  CSE accessors:
**
**  	For all the rul__get_cse_nth_... functions, indexes are 1-based
*/



/********************************
**                             **
**  RUL__CS_GET_CSE_RULE_NAME  **
**                             **
********************************/

Mol_Symbol
rul__cs_get_cse_rule_name (Conflict_Set_Entry cse)
{
	assert_cs (is_valid_conflict_set_entry (cse));
	return (cse->rule_name);
}



/******************************
**                           **
**  RUL__CS_GET_CSE_RB_NAME  **
**                           **
******************************/

Mol_Symbol
rul__cs_get_cse_rb_name (Conflict_Set_Entry cse)
{
	assert_cs (is_valid_conflict_set_entry (cse));
	return (cse->rb_name);
}



/***********************************
**                                **
**  RUL__CS_GET_CSE_OBJECT_COUNT  **
**                                **
***********************************/

long
rul__cs_get_cse_object_count (Conflict_Set_Entry cse)
{
	assert_cs (is_valid_conflict_set_entry (cse));
	return (cse->inst_id_count);
}



/*******************************
**                            **
**  RUL__CS_GET_CSE_HASH_NUM  **
**                            **
*******************************/

unsigned long
rul__cs_get_cse_hash_num (Conflict_Set_Entry cse)
{
	assert_cs (is_valid_conflict_set_entry (cse));
	return (cse->hash_number);
}



/***********************************
**                                **
**  RUL__CS_GET_CSE_NTH_INSTANCE  **
**                                **
***********************************/

Mol_Instance_Id
rul__cs_get_cse_nth_instance (
			Conflict_Set_Entry cse, long index)
{
	assert_cs (is_valid_conflict_set_entry (cse));
	assert_cs (index > 0);
	assert_cs (cse->inst_id_count >= index);

	return (cse->inst_ids[index-1]);
}



/**********************************
**                               **
**  RUL__CS_GET_CSE_NTH_TIMETAG  **
**                               **
**********************************/

long
rul__cs_get_cse_nth_timetag (Conflict_Set_Entry cse, long index)
{
	assert_cs (is_valid_conflict_set_entry (cse));
	assert_cs (index > 0);
	assert_cs (cse->inst_id_count >= index);

	return (cse->ordered_timetags[index-1]);
}



/*********************************
**                              **
**  RUL__CS_GET_CSE_NTH_OBJECT  **
**                              **
*********************************/

Object
rul__cs_get_cse_nth_object (Conflict_Set_Entry cse, long index)
{
	assert_cs (is_valid_conflict_set_entry (cse));
	assert_cs (index > 0);
	assert_cs (cse->inst_id_count >= index);

	return (rul__mol_instance_id_value (cse->inst_ids[index-1]));
}
