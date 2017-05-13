/****************************************************************************
**                                                                         **
**                            B E T A . H                                  **
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
**	This file contains the exported definitions for the 
**	beta memory and beta token subsystem.
**
**	Each beta memory collection is owned by some ruling-block, and
**	maintains the partial match information for all the rules in
**	that ruling-block.
**
**
**  MODIFIED BY:
**	DEC		Digital Equipment Corporation
**	CPQ		Compaq Computer Corporation
**
**  MODIFICATION HISTORY:
**
**	 9-Sep-1992	DEC	Initial version
**   1-Dec-1999 CPQ Release with GPL 
*/


/* INCLUDE-IN-GEND.H  *********************************************** */
#ifdef RUL__C_IN_GENERATED_CODE
#define BPPM    rul__beta_print_partial_matches
#define BGNT    rul__beta_get_next_token
#define BMM     rul__beta_modify_memory
#define BRFM    rul__beta_remove_from_memory
#define BATM    rul__beta_add_to_memory
#define BGFT    rul__beta_get_first_token
#define BMMC    rul__beta_modify_match_count
#endif

#define BETA_MEMBER_HT_SIZE  16


struct beta_token {
	/*
	**  The first set of fields should ONLY be accessed
	**  by code inside the beta memories subsystem.
	*/
	Class_Member		curr_member;
        unsigned long           hash_num;	/*  Composite hash number    */
        long                    node_id;	/*  Unique node/side ident   */
        Beta_Token		hash_next;	/*  Bucket overflow chain    */
        Beta_Token		hash_prev;	/*  Inverse overflow chain   */
        Class_Member		first_member;	/*  Equivalence class chain  */
	Class_Member	        hash_members[BETA_MEMBER_HT_SIZE];

#ifdef DEBUG_BETA
        long                    verification;
	long			hash_val_count; /*  Number of values hashed  */
	long			aux_val_count;  /*  Number of unhashed values*/
#endif

	/*
	**  The following are the only fields that should be
	**  accessed directly from generated code.
	*/
	long			inst_count;
	Mol_Instance_Id	       *inst_array;	/*  Array of instances       */
	Molecule	       *aux_array;	/*  Array of 2-in tested     */
	Molecule		val_array[1];	/*  Array of molecules       */
					/*
					**  There is always at least 1 value
					**  slot allocated, and normally more.
					*/
}; /*  a.k.a.  *Beta_Token  */

/* END-INCLUDE-IN-GEND.H  *********************************************** */


Beta_Collection rul__beta_make_collection (void);
void 		rul__beta_free_collection (
			Molecule block_name,
			Beta_Collection bc);
void		rul__beta_suspend_collection (
			Molecule block_name,
			Beta_Collection bc);


/* INCLUDE-IN-GEND.H  *********************************************** */
void 		rul__beta_modify_memory ( 
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
			Molecule val_array[]);	/* molecular eq TINT values  */

Beta_Token 	rul__beta_get_first_token (
			Beta_Collection bc,	/* which beta memory set     */
			long tin_id,		/* which 2-input node        */
			unsigned long tin_hsh,	/* hash number for above     */
			long count,		/* number of hash values     */
			Molecule mol_array[]);	/* molecular hash values     */

Beta_Token 	rul__beta_get_next_token (Beta_Token prev_bt);

void 		rul__beta_print_partial_matches (
			Beta_Collection b,	/* which beta collection     */
			long num,		/* number of nodes to print  */
			... );			/* node-ids/title pairs      */


/*	For negated nodes only:		*/

long    	rul__beta_modify_match_count (Beta_Token bt, long diff);
			/*
			**  Returns the new value of the match count
			**  after applying the supplied difference.
			*/

long 		rul__beta_remove_from_memory (
			/*
			**  Returns the cross match count in the entry
			**  being deleted.
			*/
			Beta_Collection bc,	/* which beta memory set     */
			long tin_id,		/* which 2-input node-side   */
			unsigned long tin_hsh,	/* hash number for above     */
			long inst_count,	/* number of instance-id's   */
			Mol_Instance_Id inst_array[],
						/* partial instantiation     */
			long hash_val_count,	/* number of hash values     */
			Molecule val_array[]);	/* molecular eq TINT values  */

void 		rul__beta_add_to_memory (
			/*
			**  Adds an entry, and sets its cross match count
			*/
			Beta_Collection bc,	/* which beta memory set     */
			long tin_id,		/* which 2-input node-side   */
			unsigned long tin_hsh,	/* hash number for above     */

			long cross_matches,	/* num of right-mem matches  */
			long inst_count,	/* number of instance-id's   */
			Mol_Instance_Id inst_array[],
						/* partial instantiation     */
			long aux_val_count,	/* number of TINT values     */
			Molecule aux_array[],	/* non-eq TINT molecules     */
			long hash_val_count,	/* number of hash values     */
			Molecule val_array[]);	/* molecular eq TINT values  */

/* END-INCLUDE-IN-GEND.H  *********************************************** */
