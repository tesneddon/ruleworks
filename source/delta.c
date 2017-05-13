/****************************************************************************
**                                                                         **
**                        R T S _ D E L T A . C                            **
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
**	This file contains the delta queue subsystem.
**
**	Each delta queue is owned by some ruling-block, and maintains
**	the list of changes to working memory that need to be propagated
**	through that ruling-block.  The delta queue, where possible,
**	compresses multiple changes into a single event.  The compression
**	rules are as follows:
**
**		1)	A negative event for an object without a positive
**			event already in the queue must always be saved with
**			an object snapshot.
**
**		2)	A negative event for an object with a positive
**			event already in the queue results in the elimination
**			of both events from the queue.
**
**		3)	If a negative event for an object with a positive
**			but non-compressible event in the queue requires that
**			a snapshot be added to both events.
**
**		Note:	The phrase "already in the queue" implies one added
**			constraint to work in a threaded environment.  To be
**			considered for event compression, the earlier event
**			must have a timetag newer than the ruling-block's
**			last propagated timetag.  This assumes that the
**			generated code correctly set the last propagated
**			timetag to the target timetag immediately upon
**			starting to do a RBS__C_OP_DELTA operation in the
**			ruling-block owning this delta queue.
**
**  MODIFIED BY:
**	DEC	Digital Equipment Corporation
**	CPQ	Compaq Computer Corporation
**
**  MODIFICATION HISTORY:
**
**	23-Jul-1992	DEC	Initial version
**	01-Dec-1999	CPQ	Releasew ith GPL
*/

#include <common.h>
#include <mol.h>
#include <wm.h>
#include <delta.h>
#include <ios.h>


/*
**  Size of the hash-table for quick access to all the potentially
**  compressible positive events in he Delta_Queue.
*/
#define DELTA__C_POS_HT_SIZE 1013


#define DQ_VERIFY 11007
#define DE_VERIFY 11008


typedef struct delta_element {
	Token_Sign 		de_sign ;
	Mol_Instance_Id		de_id ;
	/*
	**  The following are used to stash information that may
	**  no longer be present in the WM subsystem when we
	**  finally need it.
	*/
	Object			de_snapshot ;
	long			de_timetag ;
	/*
	**  The last fields are used to maintain the queue of deltas
	**  and to allow random access to specific elements within the
	**  queue when the earlier information is now outdated.
	*/
	struct delta_element	*next_de ;
	struct delta_element	*prev_de ;
	struct delta_element	*hash_chain ;

#ifndef NDEBUG
	long			verification ;
#endif

} *Delta_Element ;


struct delta_queue {
	Delta_Element		first_de ;
	Delta_Element		last_de ;
	Delta_Element		*hash_vector ;
#ifndef NDEBUG
	long			verification ;
#endif
} ;  /* also  typedef'ed as  *Delta_Queue  (in common) */



/*  Local Function Prototypes  */

static Boolean is_valid_delta_queue (Delta_Queue dq) ;
static Boolean is_valid_delta_element (Delta_Element de) ;
static Delta_Element pop_elem_off_queue (Delta_Queue dq) ;
static void remove_elem_from_queue (Delta_Queue dq, Delta_Element de) ;
static void append_elem_to_queue (Delta_Queue dq, Delta_Element de) ;
static void remove_from_delta_hash (Delta_Queue dq, Delta_Element de) ;
static void add_to_delta_hash (Delta_Queue dq, Delta_Element de) ;
static Delta_Element find_in_delta_hash (Delta_Queue dq, Mol_Instance_Id id) ;




#ifndef NDEBUG

/*
**  The following should ONLY appear inside assert()
*/

static Boolean is_valid_delta_queue (Delta_Queue dq)
{
	return (dq  &&  dq->verification == DQ_VERIFY  &&
		(dq->first_de  ||  (!dq->first_de && !dq->last_de))) ;
}

static Boolean is_valid_delta_element (Delta_Element de)
{
	return (de && de->verification == DE_VERIFY) ;
}

#endif



/****************************
**                         **
**  RUL__DELTA_MAKE_QUEUE  **
**                         **
****************************/

Delta_Queue	rul__delta_make_queue (void)
{
	Delta_Queue dq ;
	long i ;

	dq = rul__mem_malloc (sizeof (struct delta_queue)) ;
	assert (dq != NULL) ;

	dq->first_de = NULL ;
	dq->last_de = NULL ;

	dq->hash_vector =
		rul__mem_malloc (DELTA__C_POS_HT_SIZE 
				 * sizeof (Delta_Element)) ;
	assert (dq->hash_vector != NULL) ;

	for (i=0; i<DELTA__C_POS_HT_SIZE; i++) dq->hash_vector[i] = NULL ;
	
#ifndef NDEBUG
	dq->verification = DQ_VERIFY ;
#endif
	return (dq) ;
}




/****************************
**                         **
**  RUL__DELTA_FREE_QUEUE  **
**                         **
****************************/

void	rul__delta_free_queue (Delta_Queue dq)
{
	Delta_Element de, de_tmp ;

	assert (is_valid_delta_queue (dq)) ;

	de = dq->first_de ;

	while (de) {
	    if (de->de_snapshot) rul__wm_destroy_snapshot (de->de_snapshot) ;
	    rul__mol_decr_uses (de->de_id) ;

	    de_tmp = de ;
	    de = de->next_de ;
	    rul__mem_free (de_tmp) ;
	}

#ifndef NDEBUG
	dq->verification = 0 ;
#endif
	rul__mem_free (dq->hash_vector) ;
	rul__mem_free (dq) ;
}




/*************************
**                      **
**  POP_ELEM_OFF_QUEUE  **
**                      **
*************************/

static Delta_Element pop_elem_off_queue (Delta_Queue dq)
	/*
	**  Removes the delta_element from the front of the specified 
	**  delta_queue, and returns that delta_element
	*/
{
	Delta_Element de ;

	assert (is_valid_delta_queue (dq)) ;

 	de = dq->first_de ;

	dq->first_de = de->next_de ; 
	if (dq->first_de) {
	    dq->first_de->prev_de = NULL ;
	} else {
	    dq->last_de = NULL ;
	}

	return (de) ;
}




/*****************************
**                          **
**  REMOVE_ELEM_FROM_QUEUE  **
**                          **
*****************************/

static void remove_elem_from_queue (Delta_Queue dq, Delta_Element de)
	/*
	**  Removes an delta_element from anywhere in the specified delta_queue
	*/
{
	assert (is_valid_delta_queue (dq)) ;
	assert (is_valid_delta_element (de)) ;

	if (de->prev_de) {
	    de->prev_de->next_de = de->next_de ;
	} else {
	    dq->first_de = de->next_de ;
	}
	if (de->next_de) {
	    de->next_de->prev_de = de->prev_de ;
	} else {
	    dq->last_de = de->prev_de ;
	}
}



/***************************
**                        **
**  APPEND_ELEM_TO_QUEUE  **
**                        **
***************************/

static void append_elem_to_queue (Delta_Queue dq, Delta_Element de)
	/*
	**  Adds a delta_element to the end of the specified delta_queue
	*/
{
        assert (is_valid_delta_queue (dq)) ;
        assert (is_valid_delta_element (de)) ;

	if (dq->last_de) {
	    de->next_de = NULL ;
	    de->prev_de = dq->last_de ;
	    dq->last_de->next_de = de ;
	    dq->last_de = de ;
	} else {
	    dq->first_de = de ;
	    dq->last_de = de ;
	    de->next_de = NULL ;
	    de->prev_de = NULL ;
	}
}




/*****************************
**                          **
**  REMOVE_FROM_DELTA_HASH  **
**                          **
*****************************/

static void remove_from_delta_hash (Delta_Queue dq, Delta_Element de)
	/*
	**  Removes a delta_element from the instance-id based
	**  hash table that is part of the delta_queue.
	*/
{
	Delta_Element tmp_de ;
	unsigned long index ;

        assert (is_valid_delta_queue (dq)) ;
        assert (is_valid_delta_element (de)  &&
		de->de_sign == DELTA__C_SIGN_POSITIVE) ;

	index = rul__mol_to_hash_num (de->de_id) % DELTA__C_POS_HT_SIZE ;

	if (dq->hash_vector[index] == de) {
	    dq->hash_vector[index] = de->hash_chain ;
	} else {
	    tmp_de = dq->hash_vector[index] ;
	    while (tmp_de  &&  tmp_de->hash_chain != de) {
		tmp_de = tmp_de->hash_chain ;
	    }
	    if (tmp_de) {
		/*  found it  */
		tmp_de->hash_chain = tmp_de->hash_chain->hash_chain ;
	    }
	}
}



/************************
**                     **
**  ADD_TO_DELTA_HASH  **
**                     **
************************/

static void add_to_delta_hash (Delta_Queue dq, Delta_Element de)
	/*
	**  Adds a delta_element to the instance-id based
	**  hash table that is part of the delta_queue.
	*/
{
	Delta_Element tmp_de ;
	unsigned long index ;

        assert (is_valid_delta_queue (dq)) ;
        assert (is_valid_delta_element (de)  &&
		de->de_sign == DELTA__C_SIGN_POSITIVE) ;

	index = rul__mol_to_hash_num (de->de_id) % DELTA__C_POS_HT_SIZE ;

	tmp_de = dq->hash_vector[index] ;
	dq->hash_vector[index] = de ;
	de->hash_chain = tmp_de ;
}


/*************************
**                      **
**  FIND_IN_DELTA_HASH  **
**                      **
*************************/

static Delta_Element find_in_delta_hash (
			Delta_Queue dq, Mol_Instance_Id inst_id)
	/*
	**  Return the delta_element indexed by the specified inst_id
	*/
{
	Delta_Element tmp_de ;
	unsigned long index ;

        assert (is_valid_delta_queue (dq)) ;
        assert (rul__mol_is_valid (inst_id)) ;

	index = rul__mol_to_hash_num (inst_id) % DELTA__C_POS_HT_SIZE ;

	tmp_de = dq->hash_vector[index] ;
	while (tmp_de  &&  tmp_de->de_id != inst_id) {
		tmp_de = tmp_de->hash_chain ;
	}
	return (tmp_de) ;
}



/******************************
**                           **
**  RUL__DELTA_ADD_TO_QUEUE  **
**                           **
******************************/

void	rul__delta_add_to_queue (
			Delta_Queue dq,
			long last_propagated_tt,
			Token_Sign sign,
			Mol_Instance_Id inst_id,
			long timetag)
{
	Delta_Element de ;

	assert (is_valid_delta_queue (dq)) ;
        assert (rul__mol_is_valid (inst_id)) ;

	de = find_in_delta_hash (dq, inst_id) ;

	if (sign == DELTA__C_SIGN_NEGATIVE  &&  
		de  &&  de->de_timetag > last_propagated_tt) {

	    assert (de->de_sign == DELTA__C_SIGN_POSITIVE) ;
	    /*  Event compression !  */

	    remove_from_delta_hash (dq, de) ;
	    remove_elem_from_queue (dq, de) ;
	    rul__mol_decr_uses (de->de_id) ;
	    rul__mem_free (de) ;

	} else {

	    if (de) {
		assert (sign == DELTA__C_SIGN_NEGATIVE  &&  
			de->de_sign == DELTA__C_SIGN_POSITIVE) ;
		/*
		**  If there was an old positive event that is now
		**  ineligible for compression because it falls before
		**  the current LHS propagation target timetag, then
		**  we need to remove that event from the hash table, and
		**  we need to attach a snapshot to that event before the
		**  current incarnation of the object vanishes.
		*/
		remove_from_delta_hash (dq, de) ;
		de->de_snapshot =
			rul__wm_create_snapshot (
				rul__mol_instance_id_value (de->de_id)) ;
	    }

	    /*
	    **  Now we need to create the new element.
	    */
	    de = rul__mem_malloc (sizeof (struct delta_element)) ;
	    assert (de != NULL) ;

	    de->de_sign = sign ;
	    de->de_id = inst_id ;
	    de->de_timetag = timetag ;
	    rul__mol_incr_uses (inst_id) ;

#ifndef NDEBUG
	    de->verification = DE_VERIFY ;
#endif

	    if (sign == DELTA__C_SIGN_NEGATIVE) {
	        /*  
		**  For non-compressible negative events,
		**  always save the object's snapshot.
		*/
		de->de_snapshot = 
			rul__wm_create_snapshot (
				rul__mol_instance_id_value (inst_id)) ;
	    } else {
		/*  for positive events only, add it to the hash table */
		de->de_snapshot = NULL ;
		add_to_delta_hash (dq, de) ;
	    }

	    /*  Last step, add it to the event queue  */
	    append_elem_to_queue (dq, de) ;
	}
}



/********************************
**                             **
**  RUL__DELTA_GET_FROM_QUEUE  **
**                             **
********************************/

Delta_Token	rul__delta_get_from_queue (
			Delta_Queue dq,
			long target_tt )
{
	Delta_Token dt ;
	Delta_Element de ;

	assert (is_valid_delta_queue (dq)) ;

	if (dq->first_de  &&  (dq->first_de->de_timetag <= target_tt)) {
	    /*
	    **  If there is an entry on the queue,
	    **  and it is not newer than the target_tt,
	    **  then pop it off the queue
	    */

	    de = pop_elem_off_queue (dq) ;
	    assert (is_valid_delta_element (de)) ;

	    /*  Fill in the Delta_Token  */

	    dt = rul__mem_malloc (sizeof (struct delta_token)) ;
	    assert (dt != NULL) ;

	    dt->sign = de->de_sign ;
	    if (de->de_snapshot) {
		dt->instance = de->de_snapshot ;
	    } else {
	        dt->instance = rul__mol_instance_id_value (de->de_id) ;
	    }
	    dt->instance_id = rul__wm_get_instance_id (dt->instance) ;
	    dt->instance_class = rul__wm_get_class (dt->instance) ;

	    /*  Clean up and then return the new Delta_Token  */

	    if (de->de_sign == DELTA__C_SIGN_POSITIVE) {
		remove_from_delta_hash (dq, de) ;
	    }
	    rul__mem_free (de) ;
	    return (dt) ;
	}
	return (NULL) ;
}



#ifndef NDEBUG


/******************************
**                           **
**  RUL___DELTA_PRINT_QUEUE  **
**                           **
******************************/

void	rul___delta_print_queue (Delta_Queue dq)
{
	Delta_Element de ;
	long i = 1 ;

	assert (is_valid_delta_queue (dq)) ;

	rul__ios_printf (RUL__C_STD_ERR, "\n  Delta_Queue contains --") ;
	de = dq->first_de ;

	while (de) {
	    rul__ios_printf (RUL__C_STD_ERR, "\n\t%3d :\t", i) ;
	    if (de->de_sign == DELTA__C_SIGN_POSITIVE) {
		rul__ios_printf (RUL__C_STD_ERR, "+ ") ;
	    } else {
		rul__ios_printf (RUL__C_STD_ERR, "- ") ;
	    }
	    rul__mol_print_readform (de->de_id, RUL__C_STD_ERR) ;
	    rul__ios_printf (RUL__C_STD_ERR, " @ %ld", de->de_timetag) ;
	    if (de->de_snapshot) {
		rul__ios_printf (RUL__C_STD_ERR, "\t  with object snapshot") ;
	    }
	    de = de->next_de ;
	    i++ ;
	}
	rul__ios_printf (RUL__C_STD_ERR, "\n      hashed ids = {") ;
	for (i=0; i<DELTA__C_POS_HT_SIZE; i++) {
	    de = dq->hash_vector[i] ;
	    while (de) {
		rul__ios_printf (RUL__C_STD_ERR, " %s", 
			(de->de_sign==DELTA__C_SIGN_NEGATIVE ? "-":""));
		rul__mol_print_readform (de->de_id, RUL__C_STD_ERR) ;
		de = de->hash_chain ;
	    }
	}
	rul__ios_printf (RUL__C_STD_ERR, " }\n\n") ;
}

#endif
