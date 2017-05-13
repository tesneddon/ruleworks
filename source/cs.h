/* cs.h - Conflict Set routines */
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
 *	RuleWorks run time system
 *
 *  ABSTRACT:
 *	Conflict set interface.
 *
 *  MODIFIED BY:
 *	DEC	Digital Equipment Corporation
 *	CPQ	Compaq Computer Corporation
 *
 *  MODIFICATION HISTORY:
 *
 *	14-Sep-1992	DEC	Initial version
 *	01-Dec-1999	CPQ	Releasew ith GPL
 */

/* INCLUDE-IN-GEND.H  *********************************************** */
#ifdef RUL__C_IN_GENERATED_CODE
#define CGCNO   rul__cs_get_cse_nth_object
#define CMO     rul__cs_modify
#endif

/* Define Rule_RHS_Func as a pointer to a function accepting a
   Conflict_Set_Entry returning a long status */
typedef long (*Rule_RHS_Func)(Conflict_Set_Entry cse, Entry_Data eb_data);

/*
 * Functions
 */

/* Terminal nodes of the network call this function to modify the conflict
 * set.  For each call, one conflict set entry (cse) will be created or
 * deleted.  Most of the parameters are used to make up the cse.
 */

void
rul__cs_modify (Conflict_Subset mod_css, Token_Sign sign, Mol_Symbol rule_name,
		Mol_Symbol rb_name, Rule_RHS_Func rhs_func,
		long class_specificity,	long test_specificity,
		long inst_id_count, Mol_Instance_Id inst_id_array []);


Object
rul__cs_get_cse_nth_object (Conflict_Set_Entry cse, long index);

/* END-INCLUDE-IN-GEND.H  *********************************************** */

Conflict_Subset
rul__cs_make_conflict_subset (void);

void
rul__cs_free_conflict_subset (Conflict_Subset old_css);

unsigned long
rul__cs_compute_cse_hash_num (const Mol_Symbol rule_name,
			      const long inst_id_count,
			      const Mol_Instance_Id inst_id_array []);

Conflict_Set_Entry
rul__cs_get_winner (Conflict_Subset css_array[], long css_array_length,
		    Refraction_Set rs);


/* Call the RHS functions for the rule in the cse.  If it executes the RETURN
 * action, store the return value in the second parameter.
 */
long
rul__cs_call_rhs_func (Conflict_Set_Entry cse, Entry_Data eb_data);

void
rul__cs_print_cs (Conflict_Subset css_array[], long css_array_length,
		  Refraction_Set rs, IO_Stream ios);

void
rul__cs_print_one_cse (Conflict_Set_Entry cse, IO_Stream ios);

void
rul__cs_print_matches (Mol_Symbol rb_name, Mol_Symbol rule_name,
		       Conflict_Subset css_array[], long css_array_length,
		       Refraction_Set rs, IO_Stream ios);

void
rul__cs_refract_cse (Conflict_Set_Entry cse, Refraction_Set rs);

void
rul__cs_invalidate_ref_cache(Refraction_Set rs, Conflict_Subset css);

/*
**  CSE accessors:
**
**  	For the rul__get_cse_nth_... functions, indexes are 1-based
*/

Mol_Symbol
rul__cs_get_cse_rule_name (Conflict_Set_Entry cse);

Mol_Symbol
rul__cs_get_cse_rb_name (Conflict_Set_Entry cse);

unsigned long 
rul__cs_get_cse_hash_num (Conflict_Set_Entry cse);

long
rul__cs_get_cse_object_count (Conflict_Set_Entry cse);

Mol_Instance_Id
rul__cs_get_cse_nth_instance (Conflict_Set_Entry cse, long index);

long
rul__cs_get_cse_nth_timetag (Conflict_Set_Entry cse, long index);


