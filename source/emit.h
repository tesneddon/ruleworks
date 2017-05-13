/****************************************************************************
**                                                                         **
**                            E M I T . H                                  **
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
 *	This module describes the exported interface
 *	for ALL of the low-level code emitters.
 *
 * MODIFIED BY:
 *	DEC	Digital Equipment Corporation
 *	CPQ	Compaq Computer Corporation
 *
 * MODIFICATION HISTORY:
 *
 *	28-Jan-1993	DEC	Initial version
 *	01-Dec-1999	CPQ	Release with gPL
 */


typedef enum {
	EMIT__E_REL_NONE = 0,
	EMIT__E_REL_EQ = 3008,
	EMIT__E_REL_NEQ,
	EMIT__E_REL_LT,
	EMIT__E_REL_LTE,
	EMIT__E_REL_GT,
	EMIT__E_REL_GTE,
	EMIT__E_REL_AND,
	EMIT__E_REL_OR,
	EMIT__E_REL_NOT
} Relation_Operator;


typedef enum {
	EMIT__E_SCOPE_STATIC,
	EMIT__E_SCOPE_GLOBAL
} Name_Scope;


#define EMIT__E_NO_RETURN FALSE
#define EMIT__E_HAS_RETURN TRUE


typedef enum {
	EMIT__E_ENTRY_DECLARATION,
	EMIT__E_ENTRY_DEFINITION
} Entry_Type;



/*
**  Formatting the generated code
*/
void rul__emit_embedded_comment (char *arbitrary_str);
void rul__emit_comment (char *arbitrary_str);
void rul__emit_real_comment (char *arbitrary_str);
void rul__emit_blank_line (void);
void rul__emit_page_break (void);


/*
**  Creating Entry-point declarations and definitions
*/
void rul__emit_entry_point (Mol_Symbol entry_point_name, Name_Scope func_scope,
			    Boolean has_return, Ext_Type return_type,
			    Cardinality array_len, Ext_Mech return_mech);
void rul__emit_entry_point_str (char *entry_point_name, Name_Scope func_scope,
				Boolean has_return, Ext_Type return_type,
				Cardinality array_len, Ext_Mech return_mech);
	/*
	**  Both of the above generate the start of a
	**  routine.  The rul__emit_entry_point just calls
	**  the ..._str version after getting the printform.
	*/

void rul__emit_entry_arg_external (Ext_Type arg_type, char *arg_name,
				   Cardinality size, Ext_Mech arg_mech);
void rul__emit_entry_arg_internal (Rul_Internal_Type arg_type, char *arg_name,
				   Cardinality size, Ext_Mech arg_mech);
void rul__emit_entry_args_done (Entry_Type define_or_declare);

void rul__emit_return (void);
void rul__emit_return_value (Value val);

void rul__emit_entry_done (void);


/*
**  Creating memory allocation for static or stack space
*/

void    rul__emit_stack_tmp_start (Ext_Type type);

Boolean rul__emit_stack_tmp       (Ext_Type type, 
				   Cardinality a_len,
				   Ext_Mech mech,
				   long index,
				   Boolean first_done);

void    rul__emit_stack_tmp_end   (void);

void	rul__emit_stack_mol_init  (long var_index);


void rul__emit_global_external (Ext_Type var_type, 
				char *var_name,
				Cardinality size,
				Boolean referencing_symbol);

void rul__emit_global_external_ptr (Ext_Type var_type, 
				    char *var_name);

void rul__emit_stack_external (Ext_Type var_type, 
			       char *var_name,
			       Cardinality size);
void rul__emit_stack_internal (Rul_Internal_Type var_type,
			       char *var_name, 
			       Cardinality size);
void rul__emit_stack_internal_ptr (Rul_Internal_Type var_type,
				   char *var_name, 
				   Cardinality size);

void rul__emit_rhs_var_decl (long number_vars);
void rul__emit_rhs_tmp_decl (long number_vars);

void rul__emit_rhs_decr_tmp_mols (long number_vars);
void rul__emit_rhs_decr_var_mols (long number_vars);

/*
**  For function and assignment values, emit the value 
**  expression, and a statement terminator.
**
**  Note:  this function calls rul__emit_value
*/
void rul__emit_value_statement (Value value);


/*
**  For value generation:
**	use either the simple --
*/
void rul__emit_value (Value value);
/*
**      or the separated --
*/
void rul__emit_value_setup (Value value);
void rul__emit_value_reference (Value value, Boolean value_access);
void rul__emit_value_cleanup (Value value);



/*
**	Emission of simple and complex conditional (IF) statements
*/
void rul__emit_begin_conditional (
		Relation_Operator oper, Value operand_1, Value operand_2);
void rul__emit_else_if_condition (
		Relation_Operator oper, Value operand_1, Value operand_2);
void rul__emit_else_condition (void);

void rul__emit_begin_cplx_while (void);
void rul__emit_cplx_conditional (void);
void rul__emit_cplx_condition (
		Relation_Operator oper, Value operand_1, Value operand_2);
void rul__emit_close_cplx_conditions (void);

void rul__emit_end_conditional (void);


/*
**	Emission of simple loop (WHILE) statements
*/
void rul__emit_begin_iter_loop (char *iterator_name, long number_loops);
void rul__emit_begin_loop (
		Relation_Operator oper, Value operand_1, Value operand_2);
void rul__emit_end_loop (void);

void rul__emit_include_header_file (Boolean standard_library_file,
				    char *header_file_name);

void rul__emit_constant_definition (char *constant_name,
				    char *constant_value);


/*
**	Miscellaneous emissions
*/
void rul__emit_tab (unsigned int number_of_tabs);

void rul__emit_prologue (void);
