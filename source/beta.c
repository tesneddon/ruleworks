/****************************************************************************
**                                                                         **
**                         R T S _ B E T A . C                             **
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
**	RULEWORKS run time system
**
**  ABSTRACT:
**	This file contains the Beta memory subsystem.
**
**	Each beta memory collection is owned by some ruling-block, and
**	maintains the partial match information for all the rules in
**	that ruling-block.
**
**	Each beta collection is in essence a hash table containing
**	beta token equivalence classes.  The hash table is keyed by the
**	node/side identification number, plus the set of all the 2-input
**	node eq-tested values.  Off of each beta token equivalence class
**	is a list of all the instantiations belonging to that class.
**
**
**  MODIFIED BY:
**	DEC	Digital Equipment Corporation
**	CPQ	Compaq Computer Corporation
** 
**  MODIFICATION HISTORY:
**
**	 9-Sep-1992	DEC	Initial version
**	21-Jul-1993	DEC	Added member-hash-table for each eq-class
**	11-Apr-1994	DEC	Added conditionalized analysis code
*/

#include <limits.h>
#include <stdarg.h>
#include <common.h>
#include <mol.h>
#include <ios.h>
#include <i18n.h>		/* For defined constant title strings */
#include <beta.h>

#ifdef DEBUG_BETA_INTERNALS
#define assert_beta(x)		assert(x)
#else
#define assert_beta(x)		;
#endif


#define MOLS_ORDERED_HASH_INCR(hsh,mol)   ((hsh + 1) ^ MOL_TO_HASH_NUMBER(mol))
                /* add 1, then bitwise XOR (left associative) */


/*
**	Define various table size control parameters
**	for the (per-block) beta memory collection
*/
#define INITIAL_HT_SIZE		1024		/*  Must be a power of 2    */
#define HT_TOO_FULL_FACTOR	1
#define BETA_COL_HASH_INDEX(bc,hsh)	(hsh & (bc->hash_vector_length - 1))

#define MEMBER_HASH_INDEX(hsh)		(hsh & (BETA_MEMBER_HT_SIZE - 1))

    
#define BT_VERIFY 31003
#define BC_VERIFY 31004



struct beta_collection {
	Beta_Token		*hash_vector;
	unsigned long		hash_vector_length;
	unsigned long		cur_eq_class_count;
	
#ifndef NDEBUG                  /* Used for beta memory analysis */
	unsigned long		max_class_count;
	unsigned long		cur_token_count;
	unsigned long		max_token_count;
	unsigned long		tot_token_count;
	unsigned long		max_of_class_lengths;
	unsigned long		sum_of_class_lengths;
	unsigned long		tot_class_traversals;
#endif
#ifdef DEBUG_BETA_INTERNALS
	long			verification;
#endif
};  /* a.k.a.  *Beta_Collection   */




struct class_member {
	struct class_member 	*next_member;
	struct class_member 	*prev_member;
	struct class_member 	*hash_chain;
	Molecule		*aux_array;
	long			 cross_match_count;
	unsigned long		 hash_number;
	Mol_Instance_Id		 inst_array[1];
                                        /*  There is always at least 1
                                        **  instance-id, and normally more.
                                        */
};  /*  a.k.a.  *Class_Member   */



/*
**	The following are all used for the optional
**	beta memory statistics gathering.
*/

#ifndef NDEBUG                  /* Used for beta memory analysis */

static Boolean do_beta_stats (void)
{
	static Boolean do_stats = FALSE;
	static Boolean do_stats_inited = FALSE;
	char *env_val;

	if (!do_stats_inited) {
	    do_stats_inited = TRUE;
	    env_val = getenv ("TIN_DEBUG_BETA");
	    if (env_val  &&  env_val[0] != '\0') do_stats = TRUE;
	}
	return do_stats;
}


#define UPDATE_BETA_TOKEN_STATS(bc,i)					\
	    if (do_beta_stats()) {					\
                if (i==1) bc->tot_token_count++;			\
		bc->cur_token_count += i;				\
		if (bc->cur_token_count > bc->max_token_count) 		\
			bc->max_token_count = bc->cur_token_count;	\
	    }

#define UPDATE_CLASS_STATS(bc)						\
	    if (do_beta_stats()) {					\
		if (bc->cur_eq_class_count > bc->max_class_count) 	\
			bc->max_class_count = bc->cur_eq_class_count;	\
	    }

#define UPDATE_MEMBER_STATS(bc,bt)					\
	    if (do_beta_stats()) {					\
		 long num = 0;						\
		 struct class_member *mem = bt->first_member;		\
		 while (mem != NULL) {num++; mem = mem->next_member;}	\
		 bc->tot_class_traversals += 1;				\
		 bc->sum_of_class_lengths += num;			\
		 if (num > bc->max_of_class_lengths) 			\
			bc->max_of_class_lengths = num;			\
	    }

#else
#define UPDATE_BETA_TOKEN_STATS(bc,i)
#define UPDATE_CLASS_STATS(bc)
#define UPDATE_MEMBER_STATS(bc,bt)
#endif







/*  Local Function Prototypes  */

static Beta_Token  find_beta_class (
                        Beta_Collection bc,     /* which beta memory set  */
                        long tin_id,            /* which 2-input node     */
			unsigned long tin_hsh,	/* hash number for above  */
                        long hash_val_count,    /* number of hash values  */
                        Molecule val_array[],   /* molecular hash values  */
			unsigned long *hsh_ptr);/* read/write result      */

static Boolean	remove_class_member (
			Beta_Token bt,
			long inst_count, 
			Mol_Instance_Id inst_array[]);

static Boolean	remove_class_member_w_count (
			Beta_Token bt, long *cross_match_count, 
			long inst_count, Mol_Instance_Id inst_array[]);

static void	add_class_member (
			Beta_Token bt,
			long inst_count, 
			Mol_Instance_Id inst_array[],
			long aux_val_count,
			Molecule aux_array[]);

static void	add_class_member_w_count (
			Beta_Token bt,
			long cross_matches,
			long inst_count, 
			Mol_Instance_Id inst_array[],
			long aux_val_count,
			Molecule aux_array[]);

static void	rehash_beta_collection (Beta_Collection bc);

static Boolean	instantiations_equal (
			long len, 
			Mol_Instance_Id array_1[],
			Mol_Instance_Id array_2[]);



#ifndef NDEBUG                  /* Used for beta memory analysis */

/*********************************
**                              **
**  RUL__BETA_PRINT_STATISTICS  **
**                              **
*********************************/

void
rul__beta_print_statistics (Molecule name, Beta_Collection bc)
{
	char *name_str = rul__mol_get_printform(name);

	printf ("\n\n  Beta Collection Statistics for block:  %s", name_str);
	printf ("\n        Total positive beta tokens ever seen: %9d", 
					bc->tot_token_count);
	printf ("\n        Max number of co-existing beta tokens:%9d", 
					bc->max_token_count);
	printf ("\n        Max number of equivalence classes:    %9d", 
					bc->max_class_count);
	printf ("\n        Largest equivalence class traversed:  %9d", 
					bc->max_of_class_lengths);
	printf ("\n        Total number of eq-class traversals:  %9d", 
					bc->tot_class_traversals);
	printf ("\n        Average size of eq-classes traversed: %9d", 
					bc->sum_of_class_lengths /
					bc->tot_class_traversals);
	rul__mem_free (name_str);

}
#endif



#ifdef DEBUG_BETA_INTERNALS

static	Boolean is_valid_beta_collection (Beta_Collection bc);
static	Boolean is_valid_beta_token (Beta_Token be);

void	rul___beta_print_collection (Beta_Collection bc);
void	rul___beta_print_beta_token (Beta_Token bt);



/*
**  The following should ONLY appear inside assert()
*/

static Boolean
is_valid_beta_collection (Beta_Collection bc)
{
	return (bc  &&  bc->verification == BC_VERIFY);
}

static Boolean
is_valid_beta_token (Beta_Token bt)
{
	return (bt && bt->verification == BT_VERIFY);
}

#endif




/********************************
**                             **
**  RUL__BETA_MAKE_COLLECTION  **
**                             **
********************************/

Beta_Collection
rul__beta_make_collection (void)
{
	Beta_Collection bc;
	long i;

	bc = rul__mem_malloc (sizeof (struct beta_collection));
	assert_beta (bc != NULL);

	bc->hash_vector =
		rul__mem_calloc (1, INITIAL_HT_SIZE * sizeof (Beta_Token));
	assert_beta (bc->hash_vector != NULL);
	bc->hash_vector_length = INITIAL_HT_SIZE;
	bc->cur_eq_class_count = 0;

#ifndef NDEBUG                  /* Used for beta memory analysis */
	bc->max_class_count = 0;
	bc->cur_token_count = 0;
	bc->max_token_count = 0;
	bc->tot_token_count = 0;
	bc->max_of_class_lengths = 0;
	bc->sum_of_class_lengths = 0;
	bc->tot_class_traversals = 0;
#endif
#ifdef DEBUG_BETA_INTERNALS
	bc->verification = BC_VERIFY;
#endif
	return (bc);
}




/********************************
**                             **
**  RUL__BETA_FREE_COLLECTION  **
**                             **
********************************/

void
rul__beta_free_collection (Molecule name, Beta_Collection bc)
{
	Beta_Token bt, bt_tmp;
	Class_Member cm, cm_tmp;
	long i;

	assert_beta (is_valid_beta_collection (bc));

	for (i=0; i<bc->hash_vector_length; i++) {

	    bt = bc->hash_vector[i];
	    while (bt) {
		cm = bt->first_member;
	        while (cm) {
	            cm_tmp = cm;
	            cm = cm->next_member;
	            rul__mem_free (cm_tmp);
		}
	        bt_tmp = bt;
	        bt = bt->hash_next;
	        rul__mem_free (bt_tmp);
	    }
	}

#ifndef NDEBUG                  /* Used for beta memory analysis */
	if (do_beta_stats()) rul__beta_print_statistics (name, bc);
#endif

#ifdef DEBUG_BETA_INTERNALS
	bc->verification = 0;
#endif
	rul__mem_free (bc->hash_vector);
	rul__mem_free (bc);
}





/*****************************
**                          **
**  REHASH_BETA_COLLECTION  **
**                          **
*****************************/

static void
rehash_beta_collection (Beta_Collection bc)
{
	long i, old_size, new_size, index;
	Beta_Token bt, next_bt, *old_hash_vector, *new_hash_vector;

	old_size = bc->hash_vector_length;
	old_hash_vector = bc->hash_vector;

	/*  Allocation sizes MUST be a factor of 2, but which one?  */
	if (old_size == INITIAL_HT_SIZE) {
	    new_size = 8 * INITIAL_HT_SIZE;
	} else {
	    new_size = 2 * old_size;
	}
	new_hash_vector = rul__mem_calloc(1, new_size * sizeof(Beta_Token));

	if (new_hash_vector == NULL) return; /* no room for expansion */

	/*  Change the size-based fields in the beta collection  */
	bc->hash_vector = new_hash_vector;
	bc->hash_vector_length = new_size;

#ifndef NDEBUG                  /* Used for beta memory analysis */
	if (do_beta_stats()) {
	    rul__ios_printf (RUL__C_STD_ERR,
		"\n  Resizing beta collection to: %6d\n", new_size);
	}
#endif

	for (i=0; i<old_size; i++) {
	    bt = old_hash_vector[i];
	    while (bt) {
		next_bt = bt->hash_next;

		/*
		**  Transplant each beta token into the new table
		*/
		index = BETA_COL_HASH_INDEX(bc,bt->hash_num);
		bt->hash_prev = NULL;
		bt->hash_next = bc->hash_vector[index];
		if (bc->hash_vector[index]) {
		    bc->hash_vector[index]->hash_prev = bt;
		}
		bc->hash_vector[index] = bt;

	        bt = next_bt;
	    }
	}
	rul__mem_free (old_hash_vector);
}


/***********************************
**                                **
**  RUL__BETA_SUSPEND_COLLECTION  **
**                                **
***********************************/

void
rul__beta_suspend_collection (Molecule name, Beta_Collection bc)
{
#ifndef NDEBUG                  /* Used for beta memory analysis */
	if (do_beta_stats())
	    rul__beta_print_statistics (name, bc);
#endif
}



/********************************
**                             **
**  RUL__BETA_GET_FIRST_TOKEN  **
**                             **
********************************/

Beta_Token
rul__beta_get_first_token (
                        Beta_Collection bc,     /* which beta memory set */
                        long tin_id,            /* which 2-input node    */
			unsigned long tin_hsh,	/* hash number for above */
                        long hash_val_count,    /* number of hash values */
                        Molecule val_array[])   /* molecular hash values */
{
	long i, failed;
	unsigned long hsh;
	Beta_Token bt;

	assert_beta (is_valid_beta_collection(bc));

	/* compute the composite hash number */
	hsh = tin_hsh;
	for (i=0; i<hash_val_count; i++) {
	    hsh = MOLS_ORDERED_HASH_INCR(hsh,val_array[i]);
	}
	bt = bc->hash_vector[BETA_COL_HASH_INDEX(bc,hsh)];

	while (bt) {
	    if (bt->hash_num == hsh  &&  bt->node_id == tin_id) {
		/*  likely match  */
		failed = FALSE;
		for (i=0; i<hash_val_count; i++) {
		    if (val_array[i] != bt->val_array[i]) {
			failed = TRUE;
			break;
		    }
		}
		if (!failed) {
		    /*  Found the correct eq-class chain  */

		    UPDATE_MEMBER_STATS(bc,bt)
		    bt->curr_member = bt->first_member;
		    bt->inst_array = bt->curr_member->inst_array;
		    bt->aux_array = bt->curr_member->aux_array;

		    return (bt);
		}
	    }
	    bt = bt->hash_next;
	}
	return (NULL);
}




/*******************************
**                            **
**  RUL__BETA_GET_NEXT_TOKEN  **
**                            **
*******************************/

Beta_Token
rul__beta_get_next_token (Beta_Token bt)
{
	assert_beta (is_valid_beta_token(bt) && bt->curr_member != NULL);

	if (bt->curr_member->next_member) {
	    /*  If there are any more members, return the next one.  */

	    bt->curr_member = bt->curr_member->next_member;
	    bt->inst_array = bt->curr_member->inst_array;
	    bt->aux_array = bt->curr_member->aux_array;
	    return (bt);

	} else {

#ifdef DEBUG_BETA_INTERNALS
	    bt->curr_member = NULL;
	    bt->inst_array = NULL;
	    bt->aux_array = NULL;
#endif
	    return (NULL);
	}
}



/******************************
**                           **
**  RUL__BETA_MODIFY_MEMORY  **
**                           **
******************************/

void
rul__beta_modify_memory ( 
			Beta_Collection bc,	/* which beta memory set     */
			long tin_id,		/* which 2-input node-side   */
			unsigned long tin_hsh,	/* hash number for above     */
			Token_Sign sign,	/* the token's sign          */
			long inst_count,	/* number of instance-id's   */
			Mol_Instance_Id inst_array[],
						/* partial instantiation     */
			long aux_val_count,	/* number of TINT values     */
			Molecule aux_array[],	/* non-eq TINT molecules     */
			long hash_val_count,	/* number of hash values     */
			Molecule val_array[])	/* eq TINT molecular values  */
{
	Beta_Token bt;
	unsigned long hsh;
	long i, index;

	assert_beta (is_valid_beta_collection(bc));

	bt = find_beta_class (bc, tin_id, tin_hsh, 
			      hash_val_count, val_array, &hsh);

	assert_beta (bt != NULL  ||  sign != DELTA__C_SIGN_NEGATIVE);
	
	if (sign == DELTA__C_SIGN_NEGATIVE) {

	    UPDATE_BETA_TOKEN_STATS(bc,-1);
	    if (remove_class_member (bt, inst_count, inst_array)) {

		/*  Last member was removed  */

		bc->cur_eq_class_count--;
		UPDATE_CLASS_STATS(bc);
		if (bt->hash_next) bt->hash_next->hash_prev = bt->hash_prev;
		if (bt->hash_prev) {
		    bt->hash_prev->hash_next = bt->hash_next;
		} else {
		    bc->hash_vector[BETA_COL_HASH_INDEX(bc,hsh)] = 
						bt->hash_next;
		}
		rul__mem_free (bt);
	    }

	} else {  /*  DELTA__C_SIGN_POSITIVE  */

	    UPDATE_BETA_TOKEN_STATS(bc,1);
	    if (bt) {
	        add_class_member (bt, inst_count, inst_array,
					aux_val_count, aux_array);

	    } else {

		bc->cur_eq_class_count++;
		UPDATE_CLASS_STATS(bc);
		
		/*  See if we should resize the hash table  */
		if (bc->cur_eq_class_count > 
			HT_TOO_FULL_FACTOR * bc->hash_vector_length) {
		    rehash_beta_collection (bc);
		}

		/*
		**  Now create the new equivalence class
		*/
		bt = rul__mem_calloc (1, sizeof(struct beta_token) +
				         (hash_val_count * sizeof(Molecule)));
		assert_beta (bt != NULL);

		bt->hash_num = hsh;
		bt->node_id = tin_id;

#ifdef DEBUG_BETA_INTERNALS
		bt->verification = BT_VERIFY;
		bt->hash_val_count = hash_val_count;
		bt->aux_val_count = aux_val_count;

		bt->curr_member = NULL;
		bt->inst_array = NULL;
		bt->aux_array = NULL;
#endif
		bt->inst_count = inst_count;
		for (i=0; i<hash_val_count; i++) {
		    bt->val_array[i] = val_array[i];
		}
		bt->first_member = NULL;
	        add_class_member (bt, inst_count, inst_array,
					aux_val_count, aux_array);

		index = BETA_COL_HASH_INDEX(bc,hsh);
		bt->hash_prev = NULL;
		bt->hash_next = bc->hash_vector[index];
		if (bc->hash_vector[index]) {
		    bc->hash_vector[index]->hash_prev = bt;
		}
		bc->hash_vector[index] = bt;
	    }
	}
}




/***********************************
**                                **
**  RUL__BETA_REMOVE_FROM_MEMORY  **
**                                **
***********************************/

long
rul__beta_remove_from_memory ( 
			Beta_Collection bc,	/* which beta memory set     */
			long tin_id,		/* which 2-input node-side   */
			unsigned long tin_hsh,	/* hash number for above     */
			long inst_count,	/* number of instance-id's   */
			Mol_Instance_Id inst_array[],
						/* partial instantiation     */
			long hash_val_count,	/* number of hash values     */
			Molecule val_array[])	/* eq TINT molecular values  */
{
	Beta_Token bt;
	unsigned long hsh;
	long cross_matches;

	assert_beta (is_valid_beta_collection(bc));

	bt = find_beta_class (bc, tin_id, tin_hsh, 
			      hash_val_count, val_array, &hsh);

	assert_beta (bt != NULL);
	
	if (remove_class_member_w_count (
			bt, &cross_matches, inst_count, inst_array)) {

	    /*  Last member was removed  */

	    if (bt->hash_next) bt->hash_next->hash_prev = bt->hash_prev;
	    if (bt->hash_prev) {
		bt->hash_prev->hash_next = bt->hash_next;
	    } else {
	        bc->hash_vector[BETA_COL_HASH_INDEX(bc,hsh)] = bt->hash_next;
	    }
	    rul__mem_free (bt);
	}
	return (cross_matches);
}




/******************************
**                           **
**  RUL__BETA_ADD_TO_MEMORY  **
**                           **
******************************/

void
rul__beta_add_to_memory ( 
			Beta_Collection bc,	/* which beta memory set     */
			long tin_id,		/* which 2-input node-side   */
			unsigned long tin_hsh,	/* hash number for above     */

			long cross_matches,	/* the num of cross matches  */
			long inst_count,	/* number of instance-id's   */
			Mol_Instance_Id inst_array[],
						/* partial instantiation     */
			long aux_val_count,	/* number of TINT values     */
			Molecule aux_array[],	/* non-eq TINT molecules     */
			long hash_val_count,	/* number of hash values     */
			Molecule val_array[])	/* eq TINT molecular values  */
{
	Beta_Token bt;
	unsigned long hsh;
	long i, index;

	assert_beta (is_valid_beta_collection(bc));

	bt = find_beta_class (bc, tin_id, tin_hsh,
			      hash_val_count, val_array, &hsh);

	if (bt) {
	    /*  An equivalence class already exists  */
	    add_class_member_w_count (bt, cross_matches, 
					inst_count, inst_array,
					aux_val_count, aux_array);

	} else {
	    bt = rul__mem_calloc (1, sizeof(struct beta_token) +
				     (hash_val_count * sizeof(Molecule)));
	    assert_beta (bt != NULL);

	    bt->hash_num = hsh;
	    bt->node_id = tin_id;

#ifdef DEBUG_BETA_INTERNALS
	    bt->verification = BT_VERIFY;
	    bt->hash_val_count = hash_val_count;
	    bt->aux_val_count = aux_val_count;

	    bt->curr_member = NULL;
	    bt->inst_array = NULL;
	    bt->aux_array = NULL;
#endif
	    bt->inst_count = inst_count;
	    for (i=0; i<hash_val_count; i++) {
		bt->val_array[i] = val_array[i];
	    }
	    bt->first_member = NULL;
	    add_class_member_w_count (bt, cross_matches,
					inst_count, inst_array,
					aux_val_count, aux_array);

	    index = BETA_COL_HASH_INDEX(bc,hsh);
	    bt->hash_prev = NULL;
	    bt->hash_next = bc->hash_vector[index];
	    if (bc->hash_vector[index]) {
		bc->hash_vector[index]->hash_prev = bt;
	    }
	    bc->hash_vector[index] = bt;
	}
}




/**********************
**                   **
**  FIND_BETA_CLASS  **
**                   **
**********************/

static Beta_Token
find_beta_class (
                        Beta_Collection bc,     /* which beta memory set  */
                        long tin_id,            /* which 2-input node     */
			unsigned long tin_hsh,	/* hash number for above   */
                        long hash_val_count,    /* number of hash values  */
                        Molecule val_array[],   /* molecular hash values  */
			unsigned long *hsh_ptr) /* read/write result      */
{
	long i, failed;
	unsigned long hsh;
	Beta_Token bt;

	assert_beta (is_valid_beta_collection(bc));

	/*  compute the composite hash number  */
	hsh = tin_hsh;
	for (i=0; i<hash_val_count; i++) {
	    hsh = MOLS_ORDERED_HASH_INCR(hsh,val_array[i]) ;
	}
	*hsh_ptr = hsh;

	bt = bc->hash_vector[BETA_COL_HASH_INDEX(bc,hsh)];
	while (bt) {
	    if (bt->node_id == tin_id  &&  bt->hash_num == hsh) {
		/*  likely match  */
		failed = FALSE;
		for (i=0; i<hash_val_count; i++) {
		    if (val_array[i] != bt->val_array[i]) {
			failed = TRUE;
			break;
		    }
		}
		if (!failed) {
		    /*  Found the correct eq-class chain  */
		    return (bt);
		}
	    }
	    bt = bt->hash_next;
	}
	return (NULL);
}




/**************************
**                       **
**  REMOVE_CLASS_MEMBER  **
**                       **
**************************/

static Boolean
remove_class_member (Beta_Token bt,
		     long inst_count, Mol_Instance_Id inst_array[])
	/*
	**  Removes a specific partial instantiation from
	**  a beta token equivalence class.
	**
	**  Returns TRUE if this was the last member of
	**  this class and thus the class is now empty.
	*/
{
	Class_Member cm, prev_cm;
	unsigned long hsh;
	long i, index;
	Boolean ret_stat;

	assert_beta (is_valid_beta_token(bt)  &&  bt->first_member != NULL);

	/*  Compute a composite hash number for this class member  */
	hsh = MOL_TO_HASH_NUMBER(inst_array[0]);
	for (i=1; i<inst_count; i++)
	    hsh = MOLS_ORDERED_HASH_INCR(hsh,inst_array[i]);
	index = MEMBER_HASH_INDEX(hsh);

	cm = bt->hash_members[index];
	prev_cm = NULL;

	while (cm != NULL) {
	    if (hsh == cm->hash_number  &&
		instantiations_equal (inst_count, inst_array, 
					cm->inst_array))
	    {
		/*  Remove it from the member hash table  */
		if (prev_cm == NULL) {
		    bt->hash_members[index] = cm->hash_chain;
		} else {
		    prev_cm->hash_chain = cm->hash_chain;		    
		}

		/*  Then remove it from the list  */
		if (cm->prev_member) {
		    cm->prev_member->next_member = cm->next_member;
		} else {
		    bt->first_member = cm->next_member;
		}
		if (cm->next_member) {
		    cm->next_member->prev_member = cm->prev_member;
		}
		rul__mem_free (cm);
		return (bt->first_member == NULL);
	    }
	    prev_cm = cm;
	    cm = cm->hash_chain;
	}
	assert (FALSE);  /*  This line should never be reached  */
	return (FALSE);
}




/**********************************
**                               **
**  REMOVE_CLASS_MEMBER_W_COUNT  **
**                               **
**********************************/

static Boolean
remove_class_member_w_count (
			Beta_Token bt, long *cross_match_count, 
			long inst_count, Mol_Instance_Id inst_array[])
	/*
	**  Removes a specific partial instantiation from
	**  a beta token equivalence class, and sets the
	**  read/write parameter, cross_match_count.
	**
	**  Returns TRUE if this was the last member of
	**  this class and thus the class is now empty.
	*/
{
	Class_Member cm, prev_cm;
	unsigned long hsh;
	long i, index;
	Boolean ret_stat;

	assert_beta (is_valid_beta_token(bt)  &&  bt->first_member != NULL);

	/*  Compute a composite hash number for this class member  */
	hsh = MOL_TO_HASH_NUMBER(inst_array[0]);
	for (i=1; i<inst_count; i++)
	    hsh = MOLS_ORDERED_HASH_INCR(hsh,inst_array[i]);
	index = MEMBER_HASH_INDEX(hsh);

	cm = bt->hash_members[index];
	prev_cm = NULL;

	while (cm != NULL) {
	    if (hsh == cm->hash_number  &&
		instantiations_equal (inst_count, inst_array, 
					cm->inst_array))
	    {
		/*  stash the x-match count  */
		*cross_match_count = cm->cross_match_count;

		/*  Remove it from the member hash table  */
		if (prev_cm == NULL) {
		    bt->hash_members[index] = cm->hash_chain;
		} else {
		    prev_cm->hash_chain = cm->hash_chain;		    
		}

		/*  Then remove it from the list  */
		if (cm->prev_member) {
		    cm->prev_member->next_member = cm->next_member;
		} else {
		    bt->first_member = cm->next_member;
		}
		if (cm->next_member) {
		    cm->next_member->prev_member = cm->prev_member;
		}
		rul__mem_free (cm);
		return (bt->first_member == NULL);
	    }
	    prev_cm = cm;
	    cm = cm->hash_chain;
	}
	assert (FALSE);  /*  This line should never be reached  */
	return (FALSE);
}



/***********************
**                    **
**  ADD_CLASS_MEMBER  **
**                    **
***********************/

static void
add_class_member (Beta_Token bt,
		  long inst_count, Mol_Instance_Id inst_array[],
		  long aux_val_count, Molecule aux_array[])
	/*
	**  Adds a new partial instantiation to an
	**  existing beta token equivalence class.
	*/
{
	Class_Member cm;
	unsigned long hsh;
	long i, index;

	assert_beta (is_valid_beta_token(bt));

	cm = rul__mem_malloc (sizeof(struct class_member)  +
				inst_count * sizeof(Mol_Instance_Id)  +
				aux_val_count * sizeof(Molecule));
	assert_beta (cm != NULL);
	cm->cross_match_count = 0;

	/*  Compute a hash number for this class member, and stash ids */
	hsh = MOL_TO_HASH_NUMBER(inst_array[0]);
	cm->inst_array[0] = inst_array[0];
	for (i=1; i<inst_count; i++) {
	    cm->inst_array[i] = inst_array[i];
	    hsh = MOLS_ORDERED_HASH_INCR(hsh,inst_array[i]);
	}
	cm->hash_number = hsh;

	if (aux_val_count > 0) {
	    cm->aux_array = (Molecule *) &(cm->inst_array[inst_count]);
	    for (i=0; i<aux_val_count; i++) cm->aux_array[i] = aux_array[i];
	} else {
	    cm->aux_array = NULL ;
	}

	/*
	**  Once it is created, insert the new member into the hash table
	*/
	index = MEMBER_HASH_INDEX(hsh);
	cm->hash_chain = bt->hash_members[index];
	bt->hash_members[index] = cm;

	/*
	**  Then insert the new member into the list
	*/
	cm->prev_member = NULL;
	if (bt->first_member) {
		bt->first_member->prev_member = cm;
	}
	cm->next_member = bt->first_member;
	bt->first_member = cm;
}





/*******************************
**                            **
**  ADD_CLASS_MEMBER_W_COUNT  **
**                            **
*******************************/

static void
add_class_member_w_count (Beta_Token bt, long cross_matches,
			  long inst_count, Mol_Instance_Id inst_array[],
			  long aux_val_count, Molecule aux_array[])
	/*
	**  Adds a new partial instantiation to an
	**  existing beta token equivalence class, and
	**  sets its cross match count (for negated nodes).
	*/
{
	Class_Member cm;
	unsigned long hsh;
	long i, index;

	assert_beta (is_valid_beta_token(bt));

	cm = rul__mem_malloc (sizeof(struct class_member)  +
				inst_count * sizeof(Mol_Instance_Id)  +
				aux_val_count * sizeof(Molecule));
	assert_beta (cm != NULL);
	cm->cross_match_count = cross_matches;

	/*  Compute a hash number for this class member, and stash ids */
	hsh = MOL_TO_HASH_NUMBER(inst_array[0]);
	cm->inst_array[0] = inst_array[0];
	for (i=1; i<inst_count; i++) {
	    cm->inst_array[i] = inst_array[i];
	    hsh = MOLS_ORDERED_HASH_INCR(hsh,inst_array[i]);
	}
	cm->hash_number = hsh;

	if (aux_val_count > 0) {
	    cm->aux_array = (Molecule *) &(cm->inst_array[inst_count]);
	    for (i=0; i<aux_val_count; i++) cm->aux_array[i] = aux_array[i];
	} else {
	    cm->aux_array = NULL ;
	}

	/*
	**  Once it is created, insert the new member into the hash table
	*/
	index = MEMBER_HASH_INDEX(hsh);
	cm->hash_chain = bt->hash_members[index];
	bt->hash_members[index] = cm;

	/*
	**  Then insert the new member into the list
	*/
	cm->prev_member = NULL;
	if (bt->first_member) {
		bt->first_member->prev_member = cm;
	}
	cm->next_member = bt->first_member;
	bt->first_member = cm;
}





/***********************************
**                                **
**  RUL__BETA_MODIFY_MATCH_COUNT  **
**                                **
***********************************/

long
rul__beta_modify_match_count (Beta_Token bt, long diff)
{
	assert_beta (is_valid_beta_token(bt));
	assert_beta (bt->curr_member != NULL);
	assert_beta (bt->curr_member->cross_match_count >= 0);

	bt->curr_member->cross_match_count += diff;

	assert_beta (bt->curr_member->cross_match_count >= 0);

	return (bt->curr_member->cross_match_count);
}




/***************************
**                        **
**  INSTANTIATIONS_EQUAL  **
**                        **
***************************/

static  Boolean
instantiations_equal (long len, Mol_Instance_Id array_1[],
		      Mol_Instance_Id array_2[])
{
        long    i;

        for (i=len-1; i>=0; i--) {
            if (array_1[i] != array_2[i]) {
		return (FALSE);
	    }
        }
        return (TRUE);
}



/**************************************
**                                   **
**  RUL__BETA_PRINT_PARTIAL_MATCHES  **
**                                   **
**************************************/

void
rul__beta_print_partial_matches (
		Beta_Collection bc,	/* which beta collection     */
		long num_nodes,		/* number of nodes to print  */
		... )			/* node-ids/title pairs      */
{
	Beta_Token bt;
	Class_Member cm;
	long i, j, curr_node, curr_index;
	char *curr_title;
	va_list ap;

	assert_beta (is_valid_beta_collection (bc)  &&  num_nodes > 0);

	va_start (ap, num_nodes);
	curr_index = 0;

	while (curr_index < num_nodes) {
	    curr_node =   va_arg (ap, long);
	    curr_title =  va_arg (ap, char *);

	    rul__ios_print_newline (RUL__C_STD_ERR);
	    rul__ios_printf (RUL__C_STD_ERR, 
				"   *** %s %s ***",
				RUL__C_MATCHES_STRING,
				curr_title);

	    for (i=0; i<bc->hash_vector_length; i++) {
		bt = bc->hash_vector[i];
		while (bt) {
		    if (bt->node_id == curr_node) {
			cm = bt->first_member;
		        while (cm) {
			    rul__ios_print_newline (RUL__C_STD_ERR);
			    rul__ios_printf (RUL__C_STD_ERR, "    ");
			    for (j=0; j<bt->inst_count; j++) {
				rul__mol_print_readform (
					cm->inst_array[j],
					RUL__C_STD_ERR);
			        rul__ios_printf (RUL__C_STD_ERR, "  ");
			    }
			    cm = cm->next_member ;
			}
		    }
		    bt = bt->hash_next;
		}
	    }
	    curr_index++;
	}
	va_end (ap);
}



#ifdef DEBUG_BETA_INTERNALS


/**********************************
**                               **
**  RUL___BETA_PRINT_COLLECTION  **
**                               **
**********************************/

void
rul___beta_print_collection (Beta_Collection bc)
{
	Beta_Token bt;
	long i;

	assert_beta (is_valid_beta_collection (bc));

	rul__ios_printf (RUL__C_STD_ERR, "\n  Beta_Collection contains --");

	for (i=0; i<bc->hash_vector_length; i++) {

	    bt = bc->hash_vector[i];
	    while (bt) {
		rul__ios_printf (RUL__C_STD_ERR, "\n\t[%ld]\t", i);
		rul___beta_print_beta_token (bt);
	        bt = bt->hash_next;
		rul__ios_printf (RUL__C_STD_ERR, "\n");
	    }
	}
}


/**********************************
**                               **
**  RUL___BETA_PRINT_BETA_TOKEN  **
**                               **
**********************************/

void
rul___beta_print_beta_token (Beta_Token bt)
{
	long i;
	Class_Member cm;

	rul__ios_printf (RUL__C_STD_ERR, "Node %ld", bt->node_id);

	rul__ios_printf (RUL__C_STD_ERR, "\n\t\tValues:\t{ ");
	for (i=0; i<bt->hash_val_count; i++) {
	    rul__mol_print_readform (bt->val_array[i], RUL__C_STD_ERR);
	    rul__ios_printf (RUL__C_STD_ERR, " ");
	}
	rul__ios_printf (RUL__C_STD_ERR, "}");

	cm = bt->first_member;
        while (cm) {
            rul__ios_printf (RUL__C_STD_ERR, "\n\t\t\tInstance :     { ");
	    for (i=0; i<bt->inst_count; i++) {
		rul__mol_print_readform (
			cm->inst_array[i],
			RUL__C_STD_ERR);
		rul__ios_printf (RUL__C_STD_ERR, " ");
	    }
	    rul__ios_printf (RUL__C_STD_ERR, "}   { ");
	    for (i=0; i<bt->aux_val_count; i++) {
		rul__mol_print_readform (cm->aux_array[i], RUL__C_STD_ERR);
		rul__ios_printf (RUL__C_STD_ERR, " ");
	    }
	    rul__ios_printf (RUL__C_STD_ERR, "}");

            cm = cm->next_member;
	}
}





/**********************
**                   **
**  RUL__BETA_PRINT  **
**                   **
**********************/


#ifdef __VMS

void
rul__beta_print (Beta_Collection *bc, long node_side_id)
{
	char buffer[50];

	sprintf (buffer, "node/side id %ld", node_side_id);
	rul__beta_print_partial_matches (
		*bc, 1, node_side_id, buffer);
}

#else

void
rul__beta_print (Beta_Collection bc, long node_side_id)
{
	char buffer[50];

	sprintf (buffer, "node/side id %ld", node_side_id);
	rul__beta_print_partial_matches (
		bc, 1, node_side_id, "By Request--");
}

#endif /* ifdef __VMS */


#endif /* #ifdef DEBUG_BETA_INTERNALS */


