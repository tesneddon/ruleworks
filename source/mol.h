/****************************************************************************
**                                                                         **
**                               M O L . H                                 **
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
**	RuleWorks run time system and compiler
**
**  ABSTRACT:
**	This file contains the exported definitions for the molecule
**	subsystem, to be used elsewhere in the system.
**
**
**  MODIFIED BY:
**	DEC	Digital Equipment Corporation
**	CPQ	Compaq Computer Corporation
**
**  MODIFICATION HISTORY:
**
**	6-Jun-1992	DEC	Initial version
**	01-Dec-1999	CPQ	Release with GPL
*/


/* INCLUDE-IN-GEND.H  *********************************************** */
#ifdef RUL__C_IN_GENERATED_CODE
#define MDU     rul__mol_decr_uses
#define MIU     rul__mol_incr_uses
#define MAA     rul__mol_arith_add
#define MAS     rul__mol_arith_subtract
#define MAM     rul__mol_arith_multiply
#define MAO     rul__mol_arith_modulo
#define MAN     rul__mol_arith_neg
#define MAD     rul__mol_arith_divide
#define MMP     rul__mol_mark_perm
#define MCA     rul__mol_concat_atoms
#define MMC     rul__mol_make_comp
#define MMI     rul__mol_make_int_atom
#define MMD     rul__mol_make_dbl_atom
#define MMS     rul__mol_make_symbol
#define MMON    rul__mol_opaque_null
#define MMIZ    rul__mol_instance_id_zero
#define MGCL    rul__mol_get_comp_last
#define MGCN    rul__mol_get_comp_nth
#define MGCNM   rul__mol_get_comp_nth_mol
#define MGCNR   rul__mol_get_comp_nth_rt
#define MGCNMR  rul__mol_get_comp_nth_mol_rt
#define MGPC    rul__mol_get_poly_count
#define MGPCA   rul__mol_get_poly_count_atom
#define MGPCL   rul__mol_get_poly_count_last
#define MGPF    rul__mol_get_printform
#define MUPF    rul__mol_use_printform
#define MSCN    rul__mol_set_comp_nth
#define MPEQ    rul__mol_eq_pred
#define MPEQL   rul__mol_equal_pred
#define MPNEQ   rul__mol_not_eq_pred
#define MPNEQL  rul__mol_not_equal_pred
#define MPAE    rul__mol_approx_eq_pred
#define MPNAE   rul__mol_not_approx_eq_pred
#define MPST    rul__mol_same_type_pred
#define MPDT    rul__mol_diff_type_pred
#define MPLT    rul__mol_lt_pred
#define MPLTE   rul__mol_lte_pred
#define MPGT    rul__mol_gt_pred
#define MPGTE   rul__mol_gte_pred
#define MPC     rul__mol_contains_pred
#define MPNC    rul__mol_not_contains_pred
#define MPLLE   rul__mol_len_lte_pred
#define MPLNE   rul__mol_len_neq_pred
#define MPLLT   rul__mol_len_lt_pred
#define MPLEQ   rul__mol_len_eq_pred
#define MPLGE   rul__mol_len_gte_pred
#define MPLGT   rul__mol_len_gt_pred
#define MPIV    rul__mol_index_valid_pred
#endif

typedef enum {
		invalid_molecule_type = 0,
		int_atom, dbl_atom, opaque, symbol, instance_id,
		compound, table
} RulMolecule_Type;



/*
**	Special Molecule Constants
*/
RulMol_Symbol rul__mol_symbol_nil (void);
RulMol_Symbol rul__mol_symbol_root (void);
RulMol_Symbol rul__mol_symbol_crlf (void);
RulMol_Symbol rul__mol_symbol_id (void);		/*  $ID  	  */
RulMol_Symbol rul__mol_symbol_instance_of (void);	/*  $INSTANCE-OF  */
RulMol_Int_Atom rul__mol_integer_zero (void);		/*  0	  	  */
RulMol_Dbl_Atom rul__mol_double_zero (void);		/*  0.0	  	  */
RulMol_Opaque rul__mol_opaque_null (void);		/*  %x0  	  */
RulMol_Instance_Id rul__mol_instance_id_zero (void);	/*  #0   	  */
RulMol_Compound rul__mol_compound_empty (void);		/*  (compound )   */


/* END-INCLUDE-IN-GEND.H  *********************************************** */


	 /*
	 **  RUL_C_MAX_READFORM_SIZE is the size needed to store the
	 **  printform for a symbol of all |'s, excluding the terminating
	 **  null character.
	 */
#define  RUL_C_MAX_READFORM_SIZE	(RUL_C_MAX_SYMBOL_SIZE*2+2)


/*
**  Note:	The following uses information from and depends upon
**  		the internals of the MOL subsystem.  Specifically,
**		it 'knows' that the first slot in a Molecule is the
**		hash number for that Molecule.  This macro should
**		only be used where performance is critical; elsewhere
**		use the rul__mol_to_hash_num function.
*/
#define MOL_TO_HASH_NUMBER(mol)  (*((unsigned long *)mol))



/*
**	2.2.1 Molecules:	Subsystem Initialization
*/

void		 rul__mol_init (void);

/*	For our debugging use -- 		*/
void		 rul__mol_print_remaining (void);

/*
**	2.2.2 Molecules:	Access to Print Forms
*/

/* INCLUDE-IN-GEND.H  *********************************************** */
char		*rul__mol_get_printform (Molecule mol);
Boolean		 rul__mol_use_printform (Molecule mol,
					 char *char_buffer, long max_chars);
long   		 rul__mol_get_printform_length (Molecule mol);
/* END-INCLUDE-IN-GEND.H  *********************************************** */
void		 rul__mol_print_printform (Molecule mol, IO_Stream ios);
char		*rul__mol_get_readform (Molecule mol);
Boolean		 rul__mol_use_readform (Molecule mol,
				char *char_buffer, long max_chars);
void		 rul__mol_print_readform (Molecule mol, IO_Stream ios);
long    	 rul__mol_get_readform_length (Molecule mol);

Boolean		 rul__mol_use_comp_raw_readform (Molecule mol,
				char *char_buffer, long max_chars);
long		 rul__mol_get_comp_raw_length (Molecule mol);

char 	        *rul__mol_format_double (double d);


/*
**	2.2.3 Molecules:	Access to Molecule Type
*/

Boolean		 rul__mol_is_valid (Molecule molecule_value);

Boolean		 rul__mol_is_atom (Molecule molecule_value);
Boolean		 rul__mol_is_polyatom (Molecule molecule_value);
Boolean		 rul__mol_is_number (Molecule molecule_value);
Boolean		 rul__mol_is_symbol (Molecule molecule_value);
Boolean		 rul__mol_is_instance_id (Molecule molecule_value);
Boolean		 rul__mol_is_opaque (Molecule molecule_value);
Boolean		 rul__mol_is_int_atom (Molecule molecule_value);
Boolean		 rul__mol_is_dbl_atom (Molecule molecule_value);
Boolean		 rul__mol_is_compound (Molecule molecule_value);
Boolean		 rul__mol_is_table (Molecule molecule_value);

Molecule_Type	 rul__mol_get_value_type (Molecule molecule_value);
Decl_Domain	 rul__mol_get_domain (Molecule molecule_value);
Decl_Shape	 rul__mol_get_shape (Molecule molecule_value);

Decl_Domain 	 rul__mol_domain_union (Decl_Domain dom1, Decl_Domain dom2);
Boolean 	 rul__mol_is_subdomain (Decl_Domain reqired_dom, 
					Decl_Domain actual_dom);



/*
**	2.2.4 Molecules:	Maintaining Unstable Molecules (Reference Count);
*/

/* INCLUDE-IN-GEND.H  *********************************************** */

void		 rul__mol_incr_uses (RulMolecule molecule_value);
void		 rul__mol_decr_uses (RulMolecule molecule_value);




/*
**	2.2.5 Molecules:	Comparing Molecules (Predicates);
*/

RulBoolean     rul__mol_eq_pred (RulMolecule mol1, RulMolecule mol2);
RulBoolean     rul__mol_equal_pred (RulMolecule mol1, RulMolecule mol2);
RulBoolean     rul__mol_not_eq_pred (RulMolecule mol1, RulMolecule mol2);
RulBoolean     rul__mol_not_equal_pred (RulMolecule mol1, RulMolecule mol2);
RulBoolean     rul__mol_lt_pred (RulMolecule mol1, RulMolecule mol2);
RulBoolean     rul__mol_lte_pred (RulMolecule mol1, RulMolecule mol2);
RulBoolean     rul__mol_gt_pred (RulMolecule mol1, RulMolecule mol2);
RulBoolean     rul__mol_gte_pred (RulMolecule mol1, RulMolecule mol2);
RulBoolean     rul__mol_same_type_pred (RulMolecule mol1, RulMolecule mol2);
RulBoolean     rul__mol_diff_type_pred (RulMolecule mol1, RulMolecule mol2);
RulBoolean     rul__mol_approx_eq_pred (RulMolecule mol1, RulMolecule mol2);
RulBoolean     rul__mol_not_approx_eq_pred (RulMolecule m1, RulMolecule m2);
RulBoolean     rul__mol_len_eq_pred (RulMolecule mol1, RulMolecule mol2);
RulBoolean     rul__mol_len_neq_pred (RulMolecule mol1, RulMolecule mol2);
RulBoolean     rul__mol_len_lt_pred (RulMolecule mol1, RulMolecule mol2);
RulBoolean     rul__mol_len_lte_pred (RulMolecule mol1, RulMolecule mol2);
RulBoolean     rul__mol_len_gt_pred (RulMolecule mol1, RulMolecule mol2);
RulBoolean     rul__mol_len_gte_pred (RulMolecule mol1, RulMolecule mol2);
RulBoolean     rul__mol_index_valid_pred (RulMolecule mol1, RulMolecule mol2);
RulBoolean     rul__mol_contains_pred (RulMolecule mol1, RulMolecule mol2,
			       Boolean (*pred_func) (Molecule, Molecule));
RulBoolean     rul__mol_not_contains_pred (RulMolecule mol1, RulMolecule mol2,
			       Boolean (*pred_func) (Molecule, Molecule));


/*
**	2.2.6 Atoms:	Creating Atoms
*/

RulMol_Int_Atom	 rul__mol_make_int_atom (long integer_value);
RulMol_Dbl_Atom	 rul__mol_make_dbl_atom (double double_float_value);
RulMol_Opaque	 rul__mol_make_opaque (void *opaque_value);
RulMol_Symbol	 rul__mol_make_symbol (char *string_value);
RulMol_Instance_Id	 rul__mol_make_instance_id (RulObject  instance_ptr);
RulMol_Instance_Id	 rul__mol_make_instance_atom (long id_value);

/*
**       x.x.x         Concatenation (in printforms)
*/
RulMol_Symbol       rul__mol_concat_atoms (long mol_count, ...);
RulMol_Symbol       rul__mol_gensym (RulMol_Symbol prefix);
RulMol_Int_Atom     rul__mol_genint (void);
RulMolecule         rul__mol_max_min (RulBoolean is_max, long mol_count, ...);
RulMol_Symbol       rul__mol_subsymbol (RulMol_Symbol sym,
					long start_chr, long end_chr);
/*
**	2.2.7 Atoms:	Making Atoms Permanent
*/

void	 rul__mol_mark_perm (RulMolecule mol);

/* END-INCLUDE-IN-GEND.H  *********************************************** */

void       rul__mol_set_gen_count (long new_count);
long       rul__mol_get_gen_count (void);

/*
**	2.2.8 Atoms:	Instance Id Access
*/

void		 rul__mol_set_instance_id_value (Mol_Instance_Id atom_value, 
						 Object instance_ptr);
void		 rul__mol_unmake_instance_id (Mol_Instance_Id atom_value);
Mol_Instance_Id  rul__mol_next_instance_id(Mol_Instance_Id atom_value);
Mol_Instance_Id	 rul__mol_id_num_to_instance_id (long instance_number);


/*
**	2.2.9 Atoms:	Value Queries
*/

long		 rul__mol_int_atom_value (Mol_Int_Atom atom_value);
double		 rul__mol_dbl_atom_value (Mol_Dbl_Atom atom_value);
char		*rul__mol_symbol_value (Mol_Symbol atom_value);
Object		 rul__mol_instance_id_value (Mol_Instance_Id atom_value);
void		*rul__mol_opaque_value (Mol_Opaque atom_value);



/*
**	2.2.10 Numbers:	Coercing Value Queries
*/

long		 rul__mol_number_as_integer (Mol_Number atom_value);
double	 	 rul__mol_number_as_double (Mol_Number atom_value);


/*
**	Numbers:	Arithmatic Operations
*/
/* INCLUDE-IN-GEND.H  *********************************************** */

RulMol_Number rul__mol_arith_add (RulMol_Number mol1, RulMol_Number mol2);
RulMol_Number rul__mol_arith_neg (RulMol_Number mol1);
RulMol_Number rul__mol_arith_subtract (RulMol_Number mol1, RulMol_Number mol2);
RulMol_Number rul__mol_arith_multiply (RulMol_Number mol1, RulMol_Number mol2);
RulMol_Number rul__mol_arith_divide (RulMol_Number mol1, RulMol_Number mol2);
RulMol_Number rul__mol_arith_modulo (RulMol_Number mol1, RulMol_Number mol2);


/*
**	2.2.11 Polyatoms:	Polyatom Queries
*/

long		 rul__mol_get_poly_count (RulMol_Polyatom polyatom_value);
RulMol_Int_Atom	 rul__mol_get_poly_count_atom (RulMol_Polyatom poly_val);
RulMol_Int_Atom	 rul__mol_get_poly_count_rt (RulMol_Polyatom poly_val);

long		 rul__mol_get_poly_count_last (RulMol_Polyatom polyatom_value);
RulBoolean	 rul__mol_poly_has_key (RulMol_Polyatom polyatom_value,
					RulMolecule key);
RulBoolean	 rul__mol_poly_has_no_key (RulMol_Polyatom polyatom_value,
					   RulMolecule key);
RulBoolean	 rul__mol_poly_values_are_all (RulMol_Polyatom polyatom_value,
					       RulMolecule_Type typ);


/*
**	2.2.12 Compounds:	Creating Compound Values
*/

RulMol_Compound	 rul__mol_make_comp (long molecule_count, ...);
RulMol_Compound	 rul__mol_subcomp (RulMol_Compound compound_value,
				   long start_index,
				   long end_index);
RulMol_Compound	 rul__mol_remove_comp_nth_rt (RulMol_Compound compound_value,
					      RulMol_Int_Atom mol_index);
RulMol_Compound	 rul__mol_remove_comp_nth (RulMol_Compound compound_value,
					   long index);
RulMol_Compound	 rul__mol_set_comp_nth (RulMol_Compound compound_value,
					long index, RulMolecule atom_val,
					RulMolecule fill_value);

/* END-INCLUDE-IN-GEND.H  *********************************************** */
	/*
	**  	Incremental creation of compounds
	*/
void 		 rul__mol_start_tmp_comp (long length);
void 		 rul__mol_set_tmp_comp_nth (long index, Molecule elem_value);
Mol_Compound 	 rul__mol_end_tmp_comp (void);
Mol_Compound 	 rul__mol_end_tmp_comp_w_decr (void);


/*
**	2.2.13 Compounds:	Accessing Elements of Compounds
**
**		The ..._rt routines increment the use count on their
**		return values before returning them.
*/
/* INCLUDE-IN-GEND.H  *********************************************** */

RulMol_Atom	 rul__mol_get_comp_nth_mol_rt  (RulMol_Compound compound_value,
					        RulMol_Int_Atom mol_index);
RulMol_Atom	 rul__mol_get_comp_nth_mol     (RulMol_Compound compound_value,
					        RulMol_Int_Atom mol_index);
RulMol_Atom	 rul__mol_get_comp_nth_rt      (RulMol_Compound compound_value,
					        long index);
RulMol_Atom	 rul__mol_get_comp_nth         (RulMol_Compound compound_value,
					        long index);

RulMol_Atom	 rul__mol_get_comp_last (RulMol_Compound compound_value);
RulMol_Atom	 rul__mol_get_comp_last_rt (RulMol_Compound compound_value);

long		 rul__mol_position_in_comp (RulMol_Compound compound_value,
				    Boolean (*pred_func) (Molecule, Molecule),
					    RulMolecule atom_val);
/* END-INCLUDE-IN-GEND.H  *********************************************** */



/*
**	2.2.14 Tables:	Creating Table Values
*/

Mol_Table	 rul__mol_make_table (long pair_count,
					Molecule key1, Molecule value1, ...);
Mol_Table	 rul__mol_subtable (Mol_Table table_value, 
					long keys_count, Molecule key1, ...);
Mol_Table	 rul__mol_merge_tables (long table_count, 
					Mol_Table table_value1, ...);
Mol_Table	 rul__mol_remove_table_pair (Mol_Table table_value, 
					Molecule key);
Mol_Table	 rul__mol_set_table_pair (Mol_Table table_value,
					Molecule key, Molecule atom_val);


/*
**	2.2.15 Tables:	Accessing Key-Value Pairs
*/

Molecule	 rul__mol_get_table_pair (Mol_Table table_value, Molecule key);
Boolean		 rul__mol_table_keys_are_all (Mol_Table table_value, 
					Molecule_Type type);


/*
**	2.2.16 Values:	General Purpose Hash Molecule Generation
*/

unsigned long	 rul__mol_string_to_hash_num (char *a_string);
unsigned long	 rul__mol_integer_to_hash_num (long an_integer);
unsigned long	 rul__mol_double_to_hash_num (double a_double_float);
unsigned long	 rul__mol_address_to_hash_num (void *an_opaque_address);
unsigned long	 rul__mol_to_hash_num (Molecule mol_value);
unsigned long	 rul__mol_mols_to_hash_num (long mol_count, ...);
unsigned long	 rul__mol_init_hash_num_incr (void);
unsigned long	 rul__mol_mols_to_hash_num_incr (unsigned long hsh,
					Molecule mol);

