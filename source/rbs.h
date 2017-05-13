/****************************************************************************
**                                                                         **
**                         R B S . H                               	   **
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
**	This file contains the exported definitions for the live ruling
**	block subsystem, to be used elsewhere in the system.
**
**
**  MODIFIED BY:
**	DEC	Digital Equipment Corporation
 *	CPQ	Compaq Computer Corporation
**
**  MODIFICATION HISTORY:
**
**	23-Jul-1992	DEC	Initial version
**	 2-Sep-1992	DEC	Added ruling-block operation codes
**	 4-Dec-1992	DEC	Added constructs for registration
**	25-Feb-1993	DEC	Pulled ruling-block from gen'd code into RTL
**	01-Dec-1999	CPQ	Release with GPL
*/



/* INCLUDE-IN-GEND.H  *********************************************** */
/*
**	All ruling-blocks are instances of this type
*/
typedef void (*Matches_Function)   (Mol_Symbol);
typedef void (*Propagate_Function) (Delta_Token);
typedef long (*Catcher_Function)   (Entry_Data eb_data);

/*
**	3.2.1 Registering and Deregistering Ruling Blocks
*/

void	rul__rbs_register_rblock (
		Mol_Symbol 		 block_name, 
		Strategy		 block_strategy,
		Molecule		*return_value_loc,
		Conflict_Subset		*css_loc,
		Beta_Collection		*beta_mem_loc,
		Matches_Function	 matches_func,
		long			 retain_memories_on_exit,

		long			 dblock_count,
		Mol_Symbol		*dblock_names,
		Propagate_Function	*dblock_prop_funcs,

		long			 catcher_count,
		Mol_Symbol		*catcher_names,
		Catcher_Function	*catcher_funcs,

		long			 cons_count,
		Mol_Symbol 		*cons_names,
		Construct_Type 		*cons_types);

void	rul__rbs_unregister_rblock (Mol_Symbol block_name);

/* END-INCLUDE-IN-GEND.H  *********************************************** */


/*
**	Getting information to and from ruling blocks:
*/

Conflict_Subset	 rul__rbs_get_css (Mol_Symbol rb_name);

Strategy	 rul__rbs_get_strategy (Mol_Symbol rb_name);

Molecule	 rul__rbs_get_return_val (Mol_Symbol rb_name);

long		 rul__rbs_constructs (Mol_Symbol rb_name,
				      Molecule **rb_cons,
				      Construct_Type **rb_typs);

long
		 rul__rbs_catchers (Mol_Symbol rb_name,
				    Mol_Symbol **rb_catcher_names);


long
		 rul__rbs_dblocks (Mol_Symbol rb_name,
				   Mol_Symbol **rb_dblock_names);


void 		 rul__rbs_print_matches (Mol_Symbol rb_name, 
					 Mol_Symbol rule_name);

long 		 rul__rbs_invoke_catcher (Mol_Symbol catcher_name,
					  Mol_Symbol catcher_rb_name,
					  Entry_Data eb_data);

void		 rul__rbs_notify_wm_delta (Object inst_ptr, Token_Sign sign);

void		 rul__rbs_update_match_network (Mol_Symbol rb_name, 
						long target_tt);

void		 rul__rbs_invalidate_ref_caches (Refraction_Set ref_set);
