/* ref.h - REFraction set routines */
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
 *	Refraction set interface.
 *
 *  MODIFIED BY:
 *	DEC	Digital Equipment Corporation
 *	CPQ	Compaq Computer Corporation
 *
 *  MODIFICATION HISTORY:
 *
 *	14-Sep-1992	DEC	Initial version
 *	01-Dec-1999	CPQ	Release with GPL
 */

/* INCLUDE-IN-GEND.H  *********************************************** */

Refraction_Set
rul__ref_make_refraction_set (void);

void
rul__ref_free_refraction_set (Refraction_Set old_rs);

/* END-INCLUDE-IN-GEND.H  *********************************************** */

void
rul__ref_empty_refraction_set (Refraction_Set old_rs);

void
rul__ref_add_entry (Refraction_Set rs, Conflict_Set_Entry new_cse);

void
rul__ref_add_state_entry (Refraction_Set rs,
			  Mol_Symbol rule_name,
			  Mol_Symbol rb_name,
			  long id_count,
			  Molecule ids[]);

void
rul__ref_remove_refs_to_timetag (long old_timetag);

Boolean
rul__ref_is_refracted (Refraction_Set rs, Conflict_Set_Entry cse);

void
rul__ref_print_refset (IO_Stream ios, Refraction_Set rs);

