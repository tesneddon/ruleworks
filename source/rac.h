/* rac.h - Recognize-Act Cycle routines */
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
 *	RuleWorks run time system
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
 *	01-Dec-1999	CPQ	Release with GPL
 */

/*
 * Values returned by generated RHSes.
 */

struct entry_data {
  long                 rb_name_count;
  Molecule            *rb_names;
  long                 db_name_count;
  Molecule            *db_names;
  Molecule             active_rb_name;
  Conflict_Set_Entry  *winning_cse_ptr;
  Conflict_Subset     *conflict_subset_array;
  Refraction_Set       refraction_set;
  Molecule            *entry_values;
  long                *cycle_count_ptr;
  long                *run_count_ptr;
  long                *after_count_ptr;
  Molecule            *catcher_name_ptr;
  Molecule            *catcher_rb_name_ptr;
  Molecule            *return_value_ptr;
};

/* INCLUDE-IN-GEND.H  *********************************************** */

void
rul__rac_cycle (RulMol_Symbol       rb_name_array[],
		long                rb_name_array_length,
		RulRefraction_Set   refraction_set,
		long on_entry_func (RulMolecule *eb_vars,
				    RulEntry_Data eb_data),
		long on_every_func (RulMolecule *eb_vars,
				    RulEntry_Data eb_data),
		long on_empty_func (RulMolecule *eb_vars,
				    RulEntry_Data eb_data),
		long on_exit_func  (RulMolecule *eb_vars,
				    RulEntry_Data eb_data),
		RulMolecule         eb_var_values[],
		long               *eb_after_count,
		RulMol_Symbol      *eb_catcher_name,
		RulMol_Symbol      *eb_catcher_rb_name,
		RulMolecule        *eb_return_value_ptr,
		RulBoolean          dbg_allowed);

void
rul__rac_after (long after_count, RulMol_Symbol catcher_name,
		RulMol_Symbol after_rb_name, RulEntry_Data eb_data);

void rul__debug (void);
void rul__debug_init (void);
void rul__trace (long arg_cnt, ...);

/* END-INCLUDE-IN-GEND.H  *********************************************** */

