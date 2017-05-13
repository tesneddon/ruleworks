/****************************************************************************
**                                                                         **
**                            V A L . H                                    **
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
 * FACILITY:
 *	RULEWORKS compiler
 *
 * ABSTRACT:
 *	This module contains the exported functions for dealing with
 *	all the possible types of user program values.
 *
 * MODIFIED BY:
 *	DEC	Digital Equipment Corporation
**	CPQ	Compaq Computer Corporation
 *
 * MODIFICATION HISTORY:
 *
 *	15-Jan-1993	DEC	Initial version
 *	01-Feb-1993	DEC	Added functions for external routine values
 *	01-Dec-1999	CPQ	Release with GPL
 */



/*
**	For use with all Value types :
*/

void		rul__val_set_shape_and_domain (Value value, 
					       Decl_Shape shape,
					       Decl_Domain domain);

void		rul__val_set_class (Value value, Class rclass);

Class		rul__val_get_class (Value value);

Decl_Shape 	rul__val_get_shape (Value value);

Decl_Domain 	rul__val_get_domain (Value value);

Value		rul__val_copy_value (Value orig_value);
			/*  Creates a new value tree that is an exact
			**  copy of the supplied value-tree  */

void		rul__val_free_value (Value value);
			/*  Also frees any subvalues and subsequent values  */

Boolean		rul__val_is_valid (Value val);
			/*  Test if a thing is a value */

Value 		rul__val_build (Ast_Node val_expr, long ce_index);
			/*  Given a abstract syntax tree for some value
			**  or value expression, create the corresponding
			**  Value structure. */

Boolean		rul__val_is_simple_expr (Value val);
			/*  Test if a value has setup or cleanup required */

Boolean		rul__val_equivalent (Value val_1, Value val_2);
			/*  Test if two values are verifiably equivalent */



/*
** RULEWORKS constants 
*/

Boolean		rul__val_is_rul_constant (Value value);

Value		rul__val_create_rul_constant (Molecule con);
			/*  Run-time molecular constant  */

/*
** Animal Constants 
*/

Boolean		rul__val_is_animal_const (Value value);

Value		rul__val_create_long_animal (long long_val);

Value		rul__val_create_uns_long_animal (unsigned long long_val);

Value		rul__val_create_asciz_animal (char *val);

Value		rul__val_create_double_animal (double val);

Ext_Type	rul__val_get_animal_type (Value val);

long		rul__val_get_animal_long (Value val);

char		*rul__val_get_animal_asciz (Value val);

double		rul__val_get_animal_double (Value val);


/*
** RULEWORKS classes 
*/

Boolean		rul__val_is_class_constant (Value value);

Value		rul__val_create_class_constant (Class class_id);

/*
** RULEWORKS constructs and types
*/

Boolean		rul__val_is_construct (Value value);

Value		rul__val_create_construct (Molecule con);

Boolean		rul__val_is_construct_type (Value value);

Value		rul__val_create_construct_type (Mol_Symbol name,
						Construct_Type cons_type);



/*
** LHS Position Values 
*/

Boolean 	rul__val_is_position_elem (Value val);
Boolean 	rul__val_is_position (Value val);
long		rul__val_get_pos_ce_index (Value val);
long		rul__val_get_pos_attr_offset (Value val);
Value		rul__val_get_pos_elem_index (Value val);

Value		rul__val_create_position (Class ce_class,
					  Mol_Symbol attr_name,
					  long ce_index);
			/*  LHS attribute value reference */

Value 		rul__val_create_position_elem (Class ce_class,
					       Mol_Symbol attr_name, 
					       Value index, 
					       long ce_index);
			/*  LHS attribute element value reference */

/*
** LHS Variables 
*/

Boolean		rul__val_is_lhs_variable (Value val);
void            rul__val_set_lhs_variable_name (Value val,
						Mol_Symbol variable_name);
Value		rul__val_create_lhs_variable (Mol_Symbol variable_name,
					      long unique_lvar_id);
long 		rul__val_get_lhs_variable_id (Value val);
void		rul__val_mark_lhs_variables (Value value, Net_Node node);
			/*  Used for nested expression within the LHS  */



/*
**  References to stack, argument, or block scoped values
*/
Value 		rul__val_create_blk_or_loc_addr (
			Rul_Internal_Type arg_type, char *arg_name);

Value 		rul__val_create_blk_or_loc (
			Rul_Internal_Type arg_type, char *arg_name);

Value 		rul__val_create_blk_or_loc_vec (
			Rul_Internal_Type arg_type, char *arg_name,
			long array_index);

Value 		rul__val_create_blk_or_loc_fld (
			Rul_Internal_Type arg_type, char *arg_name,
			Rul_Internal_Field field);

Value		rul__val_create_blk_loc_fld_vec (
			Rul_Internal_Type arg_type, char *arg_name,
			Rul_Internal_Field field, long array_index);

Value		rul__val_create_null (Rul_Internal_Type arg_type);


void		rul__val_register_lhs_tmp_vars (Value value);

/*
** Named RHS variables (containing molecules) 
*/

Boolean		rul__val_is_rhs_variable (Value val);

Value		rul__val_create_rhs_variable (Mol_Symbol variable_name);
			/*  Explicit variable referenced within the RHS  */

void		rul__val_set_rhs_var_index (Value value, long index);

/*
** Unnamed molecule variables 
*/

Boolean		rul__val_is_mol_variable (Value val);

Value		rul__val_create_unnamed_mol_var (long cache_index);
			/*  Implicit molecular variable, at run-time 
			**  will be cached locally for each scope  */

void 		rul__val_for_each_unnamed_mol (Value value, 
			       void (*function_ptr)(Value unm_var_val));
			/*  Recursively traverses a value and call the
			**  specified function for each unnamed mol
			**  variable encountered.
			*/

Value	rul__val_create_named_ext_var (Mol_Symbol var_name,
				       Ext_Type type,
				       Cardinality array_len,
				       Ext_Mech mechanism,
				       Boolean memory_allocated);

/*
** External temp variables 
*/

Boolean		rul__val_is_ext_variable (Value val);

Value		rul__val_create_unnamed_ext_var (long cache_index,
						 Ext_Type type,
						 Cardinality array_len,
						 Ext_Mech mechanism, 
						 Boolean memory_allocated);
			/*  Implicit variable, at run-time will be cached
			**  locally, actual cache is dependent on type */

Ext_Type	rul__val_get_ext_var_type (Value value);
			/*  Access to type slot of an ext unnamed value */

Cardinality	rul__val_get_ext_var_arr_len (Value value);
			/*  Access to array len slot of an ext unnamed val */
long		rul__val_get_ext_var_index (Value value);
void		rul__val_set_ext_var_index (Value value, long index);
			/*  Access to array_index slot of an ext unnamed val */

Ext_Mech	rul__val_get_ext_var_mech (Value value);
void		rul__val_set_ext_var_mech (Value value, Ext_Mech mech);
			/*  Access to mechanism slot of an ext unnamed val */

Boolean		rul__val_get_ext_var_mem_dealoc (Value value);

/*
** Arithmetic Expression Values 
*/

Boolean		rul__val_is_arith_expr (Value val);

Value		rul__val_create_arith_expr (Arith_Oper_Type arith_operator,
					    Value operand_1, 
					    Value operand_2);

/*
** Conditional Expression Values 
*/

Boolean		rul__val_is_cond_expr (Value val);

Value		rul__val_create_cond_expr (Cond_Oper_Type cond_operator,
					   Value operand_1, 
					   Value operand_2);

/*
** Function Values 
*/

Boolean 	rul__val_is_function (Value func_value);
			/*  Test if a value is a function invocation  */

Value		rul__val_create_function (Mol_Symbol func_name, 
					  long arg_count);

Value		rul__val_create_function_str (char *func_name,
					      long arg_count);
			/*  Function invocation value  */

void		rul__val_set_func_mech (Value function_val, Ext_Mech mech);

void		rul__val_set_nth_subvalue (Value function_val,
					   long n,
					   Value subval);
			/*  Add argument to a function or a disjunction  */

Value		rul__val_set_prev_value (Value value, Value prev_value);
			/* Define setup sequence of values */

Value           rul__val_get_prev_value (Value value);
			/* Access sequence of function calls or assignments */

Value		rul__val_set_next_value (Value value, Value next_value);
			/* Define cleanup sequence of values */

Value           rul__val_get_next_value (Value value);
			/* Access sequence of function calls or assignments */

Value		rul__val_get_last_value (Value value);


/*
** Assignment Value routines 
*/

Boolean 	rul__val_is_assignment (Value assignment_value);
			/*  Test if a value is an assignment value  */

Value		rul__val_create_assignment (Value to_variable,
					    Value from_value);
			/*  Value assignments; usually used to assign a 
			**  function's return values into a variable */

Value		rul__val_get_assignment_to (Value value);
			/*  Access to to slot of an assignment value */

Value		rul__val_get_assignment_from (Value value);
			/*  Access to from slot of an assignment value */

/*
** Complex Values 
*/

Value		rul__val_create_complex (long cmplx_count);

Boolean		rul__val_is_complex (Value value);

long		rul__val_get_complex_count (Value value);

Value		rul__val_get_nth_complex (Value value, long n);

/*
** Disjunction Values 
*/

Value		rul__val_create_disjunction (long disj_count);
			/*  LHS value disjunction  */

Boolean		rul__val_is_disjunction (Value value);

long		rul__val_get_disjunct_count (Value value);

Value		rul__val_get_nth_disjunct (Value value, long n);

/*
** LHS Tests 
*/

Boolean 	rul__val_is_local (Value val, long ce_index);
			/*  For LHS when placing test on 1-in or 2-in nodes */

/*
** Miscellaneous support routines
*/

void		rul__val_print_animal_const (Value val, IO_Stream ios);

#ifndef NDEBUG
void		rul___val_print (Value val); /* for debugging only */
#endif
