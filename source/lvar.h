/****************************************************************************
**                                                                         **
**                            L V A R . H  

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
 *	RULEWORKS compiler
 *
 *  ABSTRACT:
 *	Exported routines for the table of LHS variables for the current rule.
 *
 *  MODIFIED BY:
 *	DEC	Digital Equipment Corporation
 *	CPQ	Compaq Computer Corporation
 *
 *  MODIFICATION HISTORY:
 *	16-Feb-1993	DEC	Initial version
 *	01-Dec-1999	CPQ	Release with GPL
 */



#define  LVAR__C_INVALID_VAR_NUMBER -1

typedef enum {
	    LVAR__E_POSITIVE = 193,
	    LVAR__E_NEGATIVE,
	    LVAR__E_START_DISJ_POS,
	    LVAR__E_START_DISJ_NEG,
	    LVAR__E_DISJ_MEMBER_POS,
	    LVAR__E_DISJ_MEMBER_NEG
} Condition_Element_Type;


/**********  Querying LHS Variable Information  **********/


Boolean 	rul__lvar_variable_is_known (Molecule var_name);
			/*  
			**  Returns TRUE if variable was bound in the LHS
			*/

Boolean 	rul__lvar_variable_is_visible (
			Molecule var_name, long in_ce_number);
			/*
			**  Returns TRUE if the variable was bound such that
			**  it's bining is visible in the specified CE  
			*/

Boolean 	rul__lvar_variable_is_local (
			Molecule var_name, long cur_ce_number);
			/*
			**  Determine whether this variable has a binding 
			**  site in this condition element
			*/

void 		rul__lvar_save_ce_type (
			long ce_index, Condition_Element_Type type);

Decl_Shape 	rul__lvar_get_shape (Molecule var_name);
Decl_Domain 	rul__lvar_get_domain (Molecule var_name);
Class 		rul__lvar_get_class (Molecule var_name);

Decl_Shape 	rul__lvar_get_rhs_shape (Molecule var_name);
Decl_Domain 	rul__lvar_get_rhs_domain (Molecule var_name);
Class 		rul__lvar_get_rhs_class (Molecule var_name);

long		rul__lvar_get_binding_count (Molecule var_name);
long		rul__lvar_get_id_number (Molecule var_name, long ce_index);
long 		rul__lvar_get_pos_var_number (long ce_index, Value pos_val);

void		rul__lvar_find_nested_lhs_vars (Mol_Symbol var_name);
Value		rul__lvar_build_rhs_value (Mol_Symbol var_name);
long  		rul__lvar_get_simple_ce_index (Mol_Symbol var_name);
long  		rul__lvar_get_simple_attr_ofset (Mol_Symbol var_name);



/**********  Creating and Mainitaining LHS Variables  **********/

void 		rul__lvar_clear_block (void);
			/*  Called at end of each rule or entry block  */

void		rul__lvar_clear_rule (void);
			/*  Called at end of each rule  */

void 		rul__lvar_create_bind_desc (
			Mol_Symbol var_name, 
			Ast_Node ast, Class ce_class, long ce_index, 
			Mol_Symbol attr_name, Value attr_elem,
			Ast_Node attr_elem_ast);
			/*
			**  Create a binding to the value of an attribute
			**  or of an attribute element.
			**
			**  Bindings must be created as part
			**  of the semantic check pass.
			*/

void 		rul__lvar_create_len_bind_desc (Mol_Symbol var_name, 
			Ast_Node ast, Class ce_class, 
			long ce_index, Mol_Symbol attr_name);
			/*
			**  Create a binding to the length of a
			**  compound  attribute.
			*/

void 		rul__lvar_create_last_bind_desc (Mol_Symbol var_name, 
			Ast_Node ast, Class ce_class, 
			long ce_index, Mol_Symbol attr_name);
			/*
			**  Create a binding to the last element of a
			**  compound  attribute.
			*/

void		rul__lvar_found_eq_test_on_var (Mol_Symbol var_name,
			Ast_Node ast, Class ce_class, long ce_index, 
			Mol_Symbol attr_name, Ast_Node attr_index_ast);
			/*
			**  Eq-test directly on a variable may affect
			**  the type/shape of that variable for the RHS
			*/

void 		rul__lvar_set_binding_node (
			Mol_Symbol var_name, long ce_index, Net_Node oin);
			/*
			**  Once match network nodes are created
			**  after the rule has been completely
			**  semantically validated, attach the
			**  var binding structures to those nodes.
			*/

long		rul__lvar_create_unnamed (void);
			/*
			**  Create a unique unnamed variable,
			**  and return the id for that variable.
			*/


Mol_Symbol	rul__lvar_get_name (long unique_index);
