/* rts_ref.c - REFraction set routines */
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
 *	This module contains the routines that maintain the refraction
 *	set for an entry block.  The refraction set contains
 *	instantiations (represented as conflict set entries) which
 *	have already fired, and are therefor not eligible for firing
 *	again.  Since removal or modification of an object causes any
 *	instantiations which reference that object to be removed from the
 *	confict set, any instantiations referencing that object may be
 *	removed from the refraction set (via
 *	rul__ref_remove_refs_to_timetag).  
 *
 *  MODIFIED BY:
 *	DEC	Digital Equipment Corporation
 *	CPQ	Compaq Computer Corporation
 *
 *  MODIFICATION HISTORY:
 *
 *	14-Sep-1992	DEC	Initial version
 *
 *	14-Sep-1994	DEC	When clearing refraction set, invalidate
 *					cached refraction info in each conflict set.
 *
 *	01-Dec-1999	CPQ	Release with GPL
 */

#include <common.h>
#include <cs.h>
#include <ios.h>
#include <ref.h>
#include <callback.h>
#include <wm.h>
#include <mol.h>
#include <rbs.h>


#define REF_SET_VERIFY	 	6710
#define REF_ENT_VERIFY		6717

#define REF_SET_HT_SIZE 	373
#define HASH_NUM_TO_INDEX(hsh)  (hsh % REF_SET_HT_SIZE)



typedef struct refraction_entry {

#ifndef NDEBUG
        long			 verification ;
#endif
	unsigned long 		 hash_number;
	struct refraction_entry	*hash_chain;

	Mol_Symbol		 rb_name;
	Mol_Symbol		 rule_name;
	long			 inst_id_count;
	long			 *timetags;
	Mol_Instance_Id		 inst_ids[1];
} *Refraction_Entry;



struct refraction_set {

#ifndef NDEBUG
        long			 verification ;
#endif
	Refraction_Entry	 hash_vector[1];
};



#ifndef NDEBUG

/*
**  The following should ONLY appear inside assert()
*/

static Boolean is_valid_refraction_set (Refraction_Set ref)
{
	return (ref  &&  ref->verification == REF_SET_VERIFY);
}

static Boolean is_valid_refraction_entry (Refraction_Entry ent)
{
	return (ent  &&  ent->verification == REF_ENT_VERIFY);
}
#endif




/***********************************
**                                **
**  RUL__REF_MAKE_REFRACTION_SET  **
**                                **
***********************************/

Refraction_Set rul__ref_make_refraction_set (void)
{
	Refraction_Set ref;
	long i;

	ref = (Refraction_Set) 
		rul__mem_malloc (
			sizeof(struct refraction_set) +
			((REF_SET_HT_SIZE - 1) * sizeof (Refraction_Entry)));
	assert (ref != NULL);

	for (i=0; i<REF_SET_HT_SIZE; i++) ref->hash_vector[i] = NULL;

#ifndef NDEBUG
	ref->verification = REF_SET_VERIFY;
#endif
	return ref;
}





/***********************************
**                                **
**  RUL__REF_FREE_REFRACTION_SET  **
**                                **
***********************************/

void rul__ref_free_refraction_set (Refraction_Set ref)
{

	rul__ref_empty_refraction_set (ref);

#ifndef NDEBUG
	ref->verification = 0;
#endif

	rul__mem_free (ref);
}






/************************************
**                                 **
**  RUL__REF_EMPTY_REFRACTION_SET  **
**                                 **
************************************/

void rul__ref_empty_refraction_set (Refraction_Set ref)
{
	long i;
	Refraction_Entry ent, cur_ent;

	assert (is_valid_refraction_set (ref));

	for (i=0; i<REF_SET_HT_SIZE; i++) {
	    ent = ref->hash_vector[i];
	    ref->hash_vector[i] = NULL;
	    while (ent != NULL) {
		cur_ent = ent;
		ent = ent->hash_chain;
		rul__mem_free (cur_ent);
	    }
	}

	/* Invalidate cached refraction info for all conflict subsets
	   which have cached info for this refraction set. */
	rul__rbs_invalidate_ref_caches(ref);
}






/*************************
**                      **
**  RUL__REF_ADD_ENTRY  **
**                      **
*************************/

void rul__ref_add_entry (Refraction_Set ref, Conflict_Set_Entry cse)
{
	long i, id_count, index;
	Refraction_Entry ent;

	assert (is_valid_refraction_set (ref));

	id_count = rul__cs_get_cse_object_count (cse);
	ent = (Refraction_Entry)
		rul__mem_malloc (sizeof(struct refraction_entry) +
				 (sizeof(Mol_Instance_Id) * (id_count - 1)) +
				 (sizeof(long) * id_count));
	assert (ent != NULL);

	ent->rule_name		= rul__cs_get_cse_rule_name (cse);
	ent->rb_name		= rul__cs_get_cse_rb_name (cse);
	ent->hash_number	= rul__cs_get_cse_hash_num (cse);
	ent->inst_id_count	= id_count;

#ifndef NDEBUG
	ent->verification	= REF_ENT_VERIFY;
#endif
	ent->timetags = (long *) &(ent->inst_ids[id_count]);

	for (i=0; i<id_count; i++) {
	    ent->inst_ids[i]	= rul__cs_get_cse_nth_instance (cse, i+1);
	    ent->timetags[i]	= rul__cs_get_cse_nth_timetag  (cse, i+1);
	}

	/*  Now add it to the hash table  */
	index = HASH_NUM_TO_INDEX(ent->hash_number);
	ent->hash_chain = ref->hash_vector[index];
	ref->hash_vector[index] = ent;
}



/*******************************
**                            **
**  RUL__REF_ADD_STATE_ENTRY  **
**                            **
*******************************/

void rul__ref_add_state_entry (Refraction_Set ref, 
			       Mol_Symbol rule_name,
			       Mol_Symbol rb_name,
			       long id_count,
			       Molecule ids[])
{
	long i, index;
	Refraction_Entry ent;
	Object wmo;

	assert (is_valid_refraction_set (ref));

	ent = (Refraction_Entry)
		rul__mem_malloc (sizeof(struct refraction_entry) +
				 (sizeof(Mol_Instance_Id) * (id_count - 1)) +
				 (sizeof(long) * id_count));
	assert (ent != NULL);

	ent->rule_name		= rule_name;
	ent->rb_name		= rb_name;
	ent->inst_id_count	= id_count;

#ifndef NDEBUG
	ent->verification	= REF_ENT_VERIFY;
#endif

	ent->timetags = (long *) &(ent->inst_ids[id_count]);

	for (i=0; i<id_count; i++) {
	    ent->inst_ids[i]	= ids[i];
	    wmo = rul__mol_instance_id_value (ids[i]);
	    if (wmo != NULL) 
	      ent->timetags[i]	= rul__wm_get_time_tag (wmo);
	    else
	      ent->timetags[i]	= 0;
	}

	ent->hash_number = rul__cs_compute_cse_hash_num (rule_name,
							 id_count, ids);

	/*  Now add it to the hash table  */
	index = HASH_NUM_TO_INDEX(ent->hash_number);
	ent->hash_chain = ref->hash_vector[index];
	ref->hash_vector[index] = ent;
}





/****************************
**                         **
**  RUL__REF_IS_REFRACTED  **
**                         **
****************************/

Boolean rul__ref_is_refracted (Refraction_Set ref, Conflict_Set_Entry cse)
{
	Refraction_Entry ent;
	long index, i;
	unsigned long cse_hsh;
	Boolean found_ids, found_timetags;

	assert (is_valid_refraction_set (ref));

	cse_hsh = rul__cs_get_cse_hash_num (cse);
	index = HASH_NUM_TO_INDEX(cse_hsh);

	ent = ref->hash_vector[index];
	while (ent != NULL) {
	    if (ent->hash_number == cse_hsh) {
		if (ent->rule_name == rul__cs_get_cse_rule_name (cse) &&
		    ent->rb_name   == rul__cs_get_cse_rb_name (cse)) {

		    found_ids = TRUE;
		    for (i=ent->inst_id_count-1; i>=0; i--) {
			if (ent->inst_ids[i] != 
			    rul__cs_get_cse_nth_instance (cse, i+1))
			{
			    found_ids = FALSE;
			    break;
			}
		    }
		    if (found_ids) {
			found_timetags = TRUE;
			for (i=ent->inst_id_count-1; i>=0; i--) {
			    if (ent->timetags[i] !=
				rul__cs_get_cse_nth_timetag (cse, i+1))
			    {
				found_timetags = FALSE;
				break;
			    }
			}
			if (found_timetags) return (TRUE);
		    }
		}
	    }
	    ent = ent->hash_chain;
	}
	return (FALSE);
}




/**************************************
**                                   **
**  RUL__REF_REMOVE_REFS_TO_TIMETAG  **
**                                   **
**************************************/

void rul__ref_remove_refs_to_timetag (long old_timetag)
{
	/*  This function keeps the refraction sets small, for performance.  */
	/*  N Y I  */
	return;
}

/**********************************
**                               **
**  RUL__REF_PRINT_REFSET        **
**                               **
**********************************/
/*  This function prints the refractions set for a savestate */
void rul__ref_print_refset (IO_Stream ios, Refraction_Set rs)
{
  long i, j;
  Refraction_Entry ent;
  char rule[RUL_C_MAX_SYMBOL_SIZE+1];
  char rb[RUL_C_MAX_SYMBOL_SIZE+1];
  char id[RUL_C_MAX_SYMBOL_SIZE+1];

  assert (is_valid_refraction_set (rs));

  for (i = 0; i < REF_SET_HT_SIZE; i++) {
    ent = rs->hash_vector[i];
    while (ent != NULL) {
      rul__mol_use_readform (ent->rule_name, rule, RUL_C_MAX_SYMBOL_SIZE+1);
      rul__mol_use_readform (ent->rb_name, rb, RUL_C_MAX_SYMBOL_SIZE+1);
      rul__ios_printf (ios, "(%s %s %ld", rule, rb, ent->inst_id_count);
      for (j = 0; j < ent->inst_id_count; j++) {
	rul__mol_use_readform (ent->inst_ids[j], id, RUL_C_MAX_SYMBOL_SIZE+1);
	rul__ios_printf (ios, " %s", id);	
      }
      rul__ios_printf (ios, ")\n");
      ent = ent->hash_chain;
    }
  }
}

