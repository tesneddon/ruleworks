/* decl.h - External interface for RuleWorks class/routine declarations */
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
 *	RuleWorks run time system and compiler
 *
 *  ABSTRACT:
 *	Declaration subsystem interface.  Includes:
 *		entry blocks,
 *		declaring blocks,
 *		object classes
 *		external routines.
 *
 *  MODIFIED BY:
 *	DEC	Digital Equipment Corporation
**	CPQ	Compaq Computer Corporation
 *
 *  MODIFICATION HISTORY:
 *
 *	19-Aug-1992	DEC	Initial version
 *   1-Sept-1992     DEC      Added enumerated type for external types
 *	1-Dec-1992	DEC	Added function prototypes for external
 *					routine declaration functions
 *	01-Dec-1999	CPQ	Release with GPL
 */


/* INCLUDE-IN-GEND.H  *********************************************** */


#ifdef RUL__C_IN_GENERATED_CODE
#define DGC     rul__decl_get_class
#define DGB     rul__decl_get_block
#endif

/* END-INCLUDE-IN-GEND.H  *********************************************** */

Mol_Symbol      rul__decl_get_entry_block_name (void);

List            rul__decl_get_visible_decl_blks (void);

Hash_Table      rul__decl_get_entry_block_rt_ht (void);

void            rul__decl_set_entry_block (Mol_Symbol entry_block_name);

void            rul__decl_init_block_visibility (void);

void            rul__decl_make_block_visible (Mol_Symbol decl_block_name);


/*
 * Declaring blocks
 *	(Routines in RUL_DECL_DECL.C)
 */

/* INCLUDE-IN-GEND.H  *********************************************** */

void             rul__decl_create_decl_block (RulMol_Symbol block_name);

void             rul__decl_finish_decl_block (void);

RulDecl_Block    rul__decl_get_block (RulMol_Symbol block_name);

/* END-INCLUDE-IN-GEND.H  *********************************************** */


Decl_Block      rul__decl_get_curr_decl_block (void);

void            rul__decl_set_curr_decl_block (Decl_Block curr_decl_blk);

Boolean         rul__decl_is_decl_block (Mol_Symbol block_name);

Decl_Block      rul__decl_get_first_decl_block (void);

Decl_Block      rul__decl_get_next_decl_block (Decl_Block block);

Mol_Symbol      rul__decl_get_decl_name (Decl_Block block);

char          * rul__decl_get_decl_short_name (Decl_Block block);

char          * rul__decl_make_decl_short_name (Mol_Symbol block_name);

void            rul__decl_set_block_routines (Decl_Block block,
					      Ext_Rt_Decl rt);

Ext_Rt_Decl     rul__decl_get_block_routines (Decl_Block block);

Hash_Table      rul__decl_get_block_routine_ht (Decl_Block block);

Hash_Table      rul__decl_get_block_method_ht (Decl_Block block);

long            rul__decl_get_block_method_cnt (Decl_Block block);

Boolean         rul__decl_block_has_classes (Mol_Symbol block_name);

void            rul__decl_set_block_classes (Decl_Block block,
					     Class cl);

Class           rul__decl_get_block_classes (Decl_Block block);

void            rul__decl_set_block_classes_end (Decl_Block block,
						 Class cl);

Class           rul__decl_get_block_classes_end (Decl_Block block);

Hash_Table      rul__decl_get_block_class_ht (Decl_Block block);

List            rul__decl_get_block_wmo_list (Decl_Block block);

void            rul__decl_set_block_level_count (Decl_Block block,
						 long value);

long            rul__decl_get_block_level_count (Decl_Block block);

void            rul__decl_set_block_sca (Decl_Block block,
					 long *value);

void            rul__decl_set_block_sca_elt (Decl_Block block,
					     long index,
					     long value);

long          * rul__decl_get_block_sca (Decl_Block block);

long            rul__decl_get_block_sca_elt (Decl_Block block,
					     long index);

void            rul__decl_set_block_sib_a_len (Decl_Block block,
					       long value);

long            rul__decl_get_block_sib_a_len (Decl_Block block);

void            rul__decl_set_block_bpl (Decl_Block block,
					 long *value);

void            rul__decl_set_block_bpl_elt (Decl_Block block,
					     long index,
					     long value);

long          * rul__decl_get_block_bpl (Decl_Block block);

long            rul__decl_get_block_bpl_elt (Decl_Block block,
					     long index);

void            rul__decl_set_block_total_bits (Decl_Block block,
						long value);

long            rul__decl_get_block_total_bits (Decl_Block block);

void            rul__decl_set_block_code_size (Decl_Block block,
					       long value);

long            rul__decl_get_block_code_size (Decl_Block block);


/*
 * Object Classes
 *	(Routines defined in RUL_DECL_CLASS.C)
 */

Decl_Block      rul__decl_get_class_block (Class class_desc);

Mol_Symbol      rul__decl_get_class_block_name (Class class_desc);

Mol_Symbol      rul__decl_get_class_name (Class class_desc);

Class           rul__decl_get_class_next (Class class_desc);


/*
 * Incremental class constructors
 */

/* INCLUDE-IN-GEND.H  *********************************************** */
void            rul__decl_create_class (RulMol_Symbol block_name,
					RulMol_Symbol class_name);

void            rul__decl_set_cur_class_parent (RulMol_Symbol parent_name);

void            rul__decl_set_cur_class_patpart (long index,
						 long pattern_part);

void            rul__decl_set_cur_class_masklen (long mask_length);

void            rul__decl_add_attr_cur_class (RulMol_Symbol attr_name);

void            rul__decl_set_cur_attr_domain (RulDecl_Domain data_type);

void            rul__decl_set_cur_attr_compound (void);

void            rul__decl_set_cur_attr_class (RulClass instance_class);

void            rul__decl_set_cur_attr_default (RulMolecule default_value);

void            rul__decl_set_cur_attr_fill (RulMolecule fill_value);

void            rul__decl_set_cur_attr_offset (int offset);

Class           rul__decl_finish_cur_class (void);

/* END-INCLUDE-IN-GEND.H  *********************************************** */
void            rul__decl_create_root_class (Mol_Symbol block_name);


/*
 * Finding class identifiers
 */

Class           rul__decl_get_class_if_unique (Mol_Symbol class_name);

Class           rul__decl_get_visible_class (Mol_Symbol class_name);

void            rul__decl_for_each_subclass (Class rclass,
					     void (*func)(Class subclass));

/* INCLUDE-IN-GEND.H  *********************************************** */

RulClass           rul__decl_get_class (RulDecl_Block block,
					RulMol_Symbol class_name);

/* END-INCLUDE-IN-GEND.H  *********************************************** */

Class           rul__decl_get_visible_class_rt (Mol_Symbol class_name,
						Mol_Symbol blocks[],
						long blk_count);

Boolean         rul__decl_is_class_visible_rt  (Class rclass,
						Mol_Symbol blocks[],
						long blk_count);

/*
 * Querying class and attribute information
 */

Boolean         rul__decl_is_attr_in_class (Class class_desc,
					    Mol_Symbol attribute_name);

Boolean         rul__decl_is_locl_attr_in_class (Class class_desc,
						 Mol_Symbol attribute_name);

Boolean         rul__decl_is_leaf_class (Class class_desc);

Boolean         rul__decl_is_root_class (Class class_desc);

long            rul__decl_get_attr_offset (Class class_desc,
					   Mol_Symbol attribute_name);

Mol_Symbol      rul__decl_get_attr_name (Class class_desc,
					 long offset);

Decl_Domain     rul__decl_get_attr_domain (Class class_desc,
					   Mol_Symbol attribute_name);

Decl_Shape      rul__decl_get_attr_shape (Class class_desc,
					  Mol_Symbol attribute_name);

Class           rul__decl_get_attr_class (Class class_desc,
					  Mol_Symbol attribute_name);

Molecule        rul__decl_get_attr_default (Class class_desc,
					    Mol_Symbol attribute_name);

/* INCLUDE-IN-GEND.H  *********************************************** */

RulMolecule        rul__decl_get_attr_fill (RulClass class_desc,
					    RulMol_Symbol attribute_name);

/* END-INCLUDE-IN-GEND.H  *********************************************** */

Boolean         rul__decl_is_attr_builtin (Class class_desc,
					   Mol_Symbol attribute_name);

Molecule      * rul__decl_get_class_default (Class class_desc);
/*	Used only by WM subsystem for creating objects.  Returns array of
 *	molecules containing default values for all attributes.
 */

long            rul__decl_get_class_num_attrs (Class class_desc);
/*	Return the number of attributes declared for a given class.
 *	Inherited attributes are included in the count.
 */

int rul__decl_get_class_depth (Class class_desc);
/*	Returns distance from root node in the inheritance tree of specified
 *	class for class specificity.  If multiply-inherited, return 0.
 */

Class           rul__decl_get_class_parent (Class class_desc);
/*	Returns the parent of the specified class;
 *	returns NULL if the specified class is $root.
 */

Class           rul__decl_get_class_ancestor (Class class_desc);
/*	Returns the ancestor of the specified class whose 
 *	immediate parent is the class $root.
 */

Class           rul__decl_lowest_common_class (Class class_1,
					       Class class_2);


/*
 * Printing class and attribute information
 */

void            rul__decl_print_class_info (Class class_desc,
					    IO_Stream ios);


/*
 * Functions for inheritance codes
 *	(Routines defined in RUL_DECL_INH.C)
 */

/* INCLUDE-IN-GEND.H  *********************************************** */
RulBoolean         rul__decl_is_subclass_of (RulClass instance_class_desc,
					     RulClass parent_class_desc);

RulDecl_Domain     rul__decl_int_to_domain (long domain_as_int);

/* END-INCLUDE-IN-GEND.H  *********************************************** */

/* uses the class sibling/child chains; use before a finish_block */
Boolean         rul__decl_is_a_subclass (Class instance_class_desc,
					 Class parent_class_desc);

void            rul__decl_encode_inheritance (Mol_Symbol decl_block_name);

long            rul__decl_get_inh_pattern_size (Mol_Symbol decl_block_name);

void            rul__decl_set_inh_pattern_size (Mol_Symbol decl_block_name,
						long size);

unsigned long   rul__decl_get_inh_pattern_part (Class class_desc,
						long index);

void            rul__decl_set_inh_pattern_part (Class class_desc,
						long index,
						unsigned long pattern_part);

long            rul__decl_get_inh_mask_length (Class class_desc);

void            rul__decl_set_inh_mask_length (Class class_desc,
					       long mask_length);

unsigned long   rul__decl_get_inh_mask_part (Class class_desc,
					     long index);





/* 
 * External Routine Declarations 
 *	(Routines defined in RUL_DECL_EXT_RT.C)
 */


void            rul__decl_create_ext_rt (Mol_Symbol func_name);
/*
 * This routine assumes that the current declaration block has been set
 * and that the current declaration block does not contain a
 * declaration, for this routine and so the compile time data structures
 * for it need to be created.
 *
 * It allocated memory for an ext rt declaration struct, which then become
 * the "current external routine"
 * This routine sets the defaults (alias for itself, zero parameters,
 * no return value...)
 *
 * the ext rt declaration struct is not added to the current decl block
 */

void            rul__decl_destroy_ext_rt (Ext_Rt_Decl ext_rt);

void            rul__decl_add_ext_rt_alias (Mol_Symbol alias_name);
/*
 * This routine sets the alias slot of the cxurrent external routine
 * declaration.
 */

void            rul__decl_add_ext_rt_param (Ext_Type data_type,
					    long array_len,
					    Ext_Mech mechanism,
					    Mol_Symbol opt_param_name,
					    Mol_Symbol opt_arg_name);
/*
 * This routine creates a parameter declaration and adds it to the list
 * of input params in the current external routine declaration.  It also
 * check that more than the specified number of input args has not been 
 * specified.
 */

void            rul__decl_add_ext_rt_ret_val (Ext_Type data_type,
					      long array_len,
					      Ext_Mech mechanism,
					      Mol_Symbol rul_arg_name);
/*
 * This routine creates a parameter declaration adds it to the current
 * external routine declaration and set thge return value flag to true.
 */

void            rul__decl_finish_ext_rt (void);
/*
 * This routine adds the current external routine to the list of
 * external routines in the current declaration block and
 * the list of external routines visible to the current entry block.
 * it also clears the current external routine decl pointers
 */

Ext_Rt_Decl     rul__decl_get_current_rt (void);

Ext_Rt_Decl     rul__decl_get_next_ext_rt (Ext_Rt_Decl ext_rt);

Ext_Alias_Decl  rul__decl_get_next_alias (Ext_Alias_Decl alias);

Hash_Table      rul__decl_get_alias_ht (void);

void            rul__decl_delete_alias_ht (void);


void            rul__decl_register_ext_rt_in_eb (Ext_Rt_Decl ext_rt);
/* 
 * This routine add an external routine declaration to the 
 * entry block visible routines list
 */

Boolean         rul__decl_is_an_ext_rt (Mol_Symbol func);
/*
 * This routine returns true and sets the "current external routine"
 * when an external routine declaration exists for a given routine
 * name in the current declaration block.  It returns false when
 * no external routine declaration currently exists...
 */

Ext_Rt_Decl     rul__decl_get_ext_rt_decl (Decl_Block block,
					   Mol_Symbol func);
/* 
 * This routine returns the external routine declaration structure 
 * for the specified routine in the specified declaration block.
 */

Ext_Rt_Decl     rul__decl_get_visible_ext_rt (Mol_Symbol func);
/*
 * In the current entry block, this routine searches the
 * hash table of external routines to find a declaration for the specified
 * function.  If found, this routine returns the external routine
 * declaration block, else it returns NULL.
 */

Boolean         rul__decl_ext_rts_are_identical (Ext_Rt_Decl rt1,
						 Ext_Rt_Decl rt2);

/* This routine compares the given external routine declarations
 * If they are identical, this routine returns true, else it returns false.
 */



/*
 * The following routines hide the structure of the external routine
 * declaration data structure (Ext_Rt_Decl).
 */

Mol_Symbol      rul__decl_ext_rt_name (Ext_Rt_Decl ext_rt);

Mol_Symbol      rul__decl_ext_rt_alias (Ext_Rt_Decl ext_rt);

Decl_Block      rul__decl_ext_rt_decl_blk (Ext_Rt_Decl ext_rt);

Ext_Rt_Decl     rul__decl_ext_rt_next_routine (Ext_Rt_Decl ext_rt);

long            rul__decl_ext_rt_num_args (Ext_Rt_Decl ext_rt);

long            rul__decl_ext_rt_by_ref (Ext_Rt_Decl ext_rt);

long            rul__decl_ext_rt_param_index (Ext_Rt_Decl ext_rt,
					      Mol_Symbol param_name);

Ext_Type        rul__decl_ext_rt_param_type (Ext_Rt_Decl ext_rt,
					     long param_index);

Ext_Mech        rul__decl_ext_rt_param_mech (Ext_Rt_Decl ext_rt,
					     long param_index);

long            rul__decl_ext_rt_param_a_len (Ext_Rt_Decl ext_rt,
					      long param_index);

long            rul__decl_ext_rt_param_s_index (Ext_Rt_Decl ext_rt,
						long param_index);

Mol_Symbol      rul__decl_ext_rt_param_name (Ext_Rt_Decl ext_rt,
					     long param_index);

Mol_Symbol      rul__decl_ext_rt_param_a_arg (Ext_Rt_Decl ext_rt,
					    long param_index);

Boolean         rul__decl_ext_rt_ret_val (Ext_Rt_Decl ext_rt);

Ext_Type        rul__decl_ext_rt_ret_type (Ext_Rt_Decl ext_rt);

Ext_Mech        rul__decl_ext_rt_mech (Ext_Rt_Decl ext_rt);

Cardinality     rul__decl_ext_rt_ret_a_len (Ext_Rt_Decl ext_rt);

Mol_Symbol      rul__decl_ext_rt_ret_a_arg (Ext_Rt_Decl ext_rt);

/* 
 * These routines determine the conversion routines needed to convert
 * external routine parameters between external and rul types
 */

Boolean         rul__decl_ext_type_convertible (Ext_Type ext_type,
						long array_len,
						Decl_Domain rul_type,
						Decl_Shape rul_shape,
						long trans_vec_addr,
						char *cvt_name);

Boolean         rul__decl_rul_type_convertible (Decl_Domain rul_type,
						Decl_Shape rul_shape,
						Ext_Type ext_type,
						long array_len,
						long trans_vec_addr,
						char *cvt_name);

/*
 * This routine handles the selection of parameters for the message:
 * ---- External Routine Conversion from an <a1> <a2> <a3> to an
 *	<a4> <a5> <a6> is undefined.
 */

void            rul__decl_report_convert_error (Decl_Domain rul_type,
						Decl_Shape rul_shape,
						Ext_Type ext_type,
						long array_len,
						Boolean rul_to_external);

Boolean         rul__sem_check_decl_indexfile (Mol_Symbol block_name,
					       Ast_Node ast);

typedef enum on_clause_type {
  DECL_ON_ENTRY,
  DECL_ON_EVERY,
  DECL_ON_EMPTY,
  DECL_ON_EXIT
} On_Clause_Type;

void            rul__decl_eb_on_clause_was_seen (On_Clause_Type clause_seen);

Boolean         rul__decl_eb_was_on_clause_seen (On_Clause_Type clause_seen);

void            rul__decl_add_rule_block_to_eb (Mol_Symbol rule_block_name);

void            rul__decl_set_eb_strategy (Strategy eb_strategy);

Strategy        rul__decl_get_eb_strategy (void);

List            rul__decl_get_rule_block_list (void);




/*
 * Methods
 *       (routines in RUL_DECL_CLASS.C)
 */

#define SYSTEM_METHODS 8

typedef enum system_methods {
  BEFORE_MAKE = 0,
  BEFORE_MODIFY,
  BEFORE_REMOVE,
  AFTER_MAKE,
  AFTER_MODIFY,
  ON_ATTR_MODIFY,
  ON_COMP_ATTR_MODIFY,
  NO_METHOD_FOUND
} System_Method_Type;

#define METHOD_HT_SIZE 53	/* expected number of methods - block/class */

/* INCLUDE-IN-GEND.H  *********************************************** */

typedef RulMolecule (*RulMethod_Func) (long num_params,
				       RulMolecule *mole_array,
				       RulEntry_Data eb_data);

RulMethod          rul__decl_create_method (RulDecl_Block block,
					    RulMol_Symbol meth_name,
					    RulBoolean is_method,
					    long param_cnt);

void            rul__decl_set_method_param (long index,
					    RulDecl_Shape shape,
					    RulDecl_Domain domain,
					    RulClass rclass);

void            rul__decl_set_method_func (RulMethod_Func func);

void            rul__decl_finish_method (void);

void            rul__decl_define_sys_methods (RulDecl_Block block);

RulMolecule     rul__call_method (RulMol_Symbol meth_name, long arg_count,
				  RulMolecule args[], RulClass inherit_class,
				  RulEntry_Data eb_data);


/* END-INCLUDE-IN-GEND.H  *********************************************** */

void            rul__decl_destroy_methods (Method meth);

void            rul__decl_method_remove (Method meth);

Hash_Table      rul__decl_get_class_method_ht (Class rclass);

long            rul__decl_get_class_sys_methods (Class class_desc);

Boolean         rul__decl_is_generic_method (Method meth);

Method          rul__decl_get_class_method (Class rclass,
					    Mol_Symbol meth_name);

Method          rul__decl_get_generic_method (Class rclass,
					      Mol_Symbol meth_name);

Method          rul__decl_get_visible_method (Mol_Symbol meth_name);

long            rul__decl_get_method_num (Method meth);

Mol_Symbol      rul__decl_get_method_name (Method meth);

Method_Func     rul__decl_get_method_func (Method meth);

Method          rul__decl_get_method_next (Method meth);

long            rul__decl_get_method_num_params (Method meth);

Decl_Shape      rul__decl_get_method_par_shape (Method meth,
						long index);

Decl_Domain     rul__decl_get_method_par_domain (Method meth,
						 long index);

Class           rul__decl_get_method_par_class (Method meth,
						long index);

long            rul__decl_is_system_method (Mol_Symbol meth_name);

Mol_Symbol      rul__decl_get_sys_meth_name (System_Method_Type met_type);

