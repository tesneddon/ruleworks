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
 * FACILITY:
 *	RULEWORKS compiler
 *
 * ABSTRACT:
 *	All the top-level SEM (semantic check) subsystem routines.
 *
 * MODIFIED BY:
 *	DEC	Digital Equipment Corporation
**	CPQ	Compaq Computer Corporation
 *
 * MODIFICATION HISTORY:
 *
 *	21-Dec-1992	DEC	Initial version
 *	19-Jan-1993	DEC	Added function prototypes for RHS
 *					variable list processing.
 *	 3-Jun-1994	DEC	Add rul__sem_eb_return_was_seen().
 *	01-Dec-1999	CPQ	Release with GPL
 */


/*
**  The following defines make the semantics of called
**  boolean parameters explicit
*/

#define SEM__C_MEMORY_ALLOCATED      TRUE
#define SEM__C_NO_MEMORY_ALLOCATED   FALSE

#define SEM__C_CONVERT_RUL_TO_EXT    TRUE
#define SEM__C_CONVERT_EXT_TO_RUL    FALSE

#define SEM__C_RETURN_VALUE_REQUIRED TRUE
#define SEM__C_NO_RETURN_VALUE_REQ   FALSE

#define SEM__C_RETURN_RUL_TYPE       TRUE
#define SEM__C_RETURN_EXT_TYPE       FALSE

#define SEM__C_ON_LHS                TRUE
#define SEM__C_ON_RHS                FALSE

#define SEM__C_RETURN_PARAMETER      TRUE
#define SEM__C_ROUTINE_ARGUMENT      FALSE

#define SEM__C_VALUE_ACCESS          TRUE
#define SEM__C_VALUE_ASSIGNMENT      FALSE

#define SEM__C_UNDEFINED_CE_INDEX    0

/* Blocks */

Boolean rul__sem_entry_block(Ast_Node ast);
void    rul__sem_set_ent_var_bindings (Ext_Rt_Decl ext_rt, long args);
Boolean rul__sem_decl_block(Ast_Node ast);
Boolean rul__sem_rule_block(Ast_Node ast);
Boolean rul__sem_end_block(Ast_Node ast);
void    rul__sem_prev_block_closed (Ast_Node ast);

Boolean rul__sem_check_decl_usefile (Mol_Symbol block_name, Ast_Node ast);

/* Groups */
Boolean rul__sem_rule_group(Ast_Node ast);
Boolean rul__sem_end_group(Ast_Node ast);

/* Misc Constructs */

Boolean	rul__sem_check_on_clause (Ast_Node ast);

Boolean	rul__sem_check_construct (Ast_Node ast);

Boolean	rul__sem_check_catch (Ast_Node ast);

void rul__sem_catcher_referenced(Molecule mol_name);

Boolean rul__sem_check_generic_method (Ast_Node ast);

Boolean rul__sem_check_method (Ast_Node ast);

Value rul__sem_check_method_call (Ast_Node ext_rt_node,
				  Boolean return_value_required,
				  Boolean on_lhs, 
				  long ce_index,
				  Boolean return_molecule,
				  Decl_Domain return_domain,
				  Decl_Shape return_shape,
				  Value return_to_value);

Boolean rul__sem_object_class (Ast_Node ast);

Boolean rul__sem_check_ext_rt_decl (Ast_Node ast);

/* The following routine will be used for a USES semantic check to verify that 
 * there are no duplicate external routine declarations visible in one
 * entry block. It returns void because it is not fatal to have duplicates,
 * but the user does need to be warned... 
 */
void rul__sem_check_ext_rts_in_eb (Mol_Symbol block_name, Ast_Node ast);


Boolean	rul__sem_check_rule (Ast_Node ast);

/* The following routine semantically checks a RHS.  It returns FALSE 
 * when the the RHS is semantically bogus.  Otherwise it returns TRUE 
 * and as a side effect attaches value trees to each RHS action ast node
 */
Boolean rul__sem_check_rhs (Ast_Node ast);

/* The following routine does a switch on the ast action
 * Called by rul__sem_check_rhs and rul__sem_check_for_each
 */
Value rul__sem_check_rhs_action (Ast_Node ast, Boolean in_loop);

void rul__sem_init_temp_rhs_vars (void);

/* The following routine initializes all the data structures used
 * for RHS variable code generation.  It is called before at the start of
 * semantic checking for each rule RHS.
 */
void rul__sem_initialize_rhs_vars (void);

/* The following routine resets temp assignment variables to allow
 * the amount of temporary value storage to be the maximum
 * needed by a single RHS action (rather than the total 
 * needed by all the RHS actions.  It is called at the end 
 * of semantic checking for Each RHS action in a rule RHS.
 */
void rul__sem_rhs_action_end_vars (void);

/* The following routine initializes all the data structures used
 * for LHS variable code generation.  It is called before at the start of
 * semantic checking for the first rule LHS in the entry block.
 */
void rul__sem_initialize_lhs_vars (void);

/* This routine sets a boolean for the usage of ENTRY block vars
 * versus the usage of normal rhs vars
 */
void rul__sem_use_entry_vars (Boolean use);

void rul__sem_initialize_oin_ht (void);

Boolean rul__sem_rhs_var_is_bound (Mol_Symbol name);

long rul__sem_number_of_rhs_vars (void);

Value rul__sem_curr_rhs_var_binding (Mol_Symbol name);

void rul__sem_reset_var_binding_site (Mol_Symbol name, Value var_value);

void rul__sem_set_var_initializer (Mol_Symbol name, Value var_value);

/* The following routine semantically checks RHS variables and records 
 * data to allow code generation of RHS variable initialization, 
 * assignment, and value access.  The second parameter (binding_required)
 * identifies whether this variable is used as a value (TRUE) or used
 * to store an assignment (FALSE) as in a bind action.
 * It returns NULL if the variable is semantically unusable, a value
 * otherwise.
 */
Value rul__sem_check_rhs_var (Ast_Node ast, 
			      Boolean binding_required);

Value rul__sem_check_lhs_var (Ast_Node ast, 
			      long ce_index,
			      Decl_Domain domain,
			      Decl_Shape shape,
			      char *usage);

/* The following routine returns the next index for temporary 
 * molecular (need to have reference count decremented at end
 * of RHS) variable.  It is assumed that registration occurs
 * at assigment time, value access will use the assignment. 
 */
long rul__sem_register_temp_var (Boolean on_lhs);

/* The following routine returns call the 'register_temp_var' 
 * routine and returns the Value from rul__val_create_unnamed_mol_var
 */
Value rul__sem_create_temp_var (Boolean on_lhs);

/* The following routine returns the next index for temporary 
 * external variable for the specified type.
 * It is assumed that registration occurs at assigment time,
 * value access will subsequently use the assignment. 
 */
long rul__sem_register_ext_temp (Ext_Type type,
				 Cardinality a_len,
				 Ext_Mech mech,
				 Boolean on_lhs);

Value rul__sem_return_value (Value return_to_val, Value func_val,
			     Decl_Shape shape, Decl_Domain domain,
			     Boolean which_side, Class class_id);

void rul__sem_add_lhs_tmp_var (Ext_Type type,
			       Cardinality a_len,
			       Ext_Mech mech,
			       long index);

/* ACTION semantic checkers */
Value rul__sem_check_addstate (Ast_Node ast);
Value rul__sem_check_after (Ast_Node ast);
Value rul__sem_check_at (Ast_Node ast);
Value rul__sem_check_bind (Ast_Node ast, Boolean in_loop);
Value rul__sem_check_build (Ast_Node ast);
Value rul__sem_check_call_inherited (Ast_Node ast,
				     Boolean return_required,
				     Value return_to_val);
Value rul__sem_check_closefile (Ast_Node ast);
Value rul__sem_check_copy (Ast_Node ast,
			   Boolean return_required,
			   Value return_to_val);
Value rul__sem_check_attr_value (Ast_Node ast, Class class_id,
				 Value object_val, Value next_val);
Value rul__sem_check_debug (Ast_Node ast);
Value rul__sem_check_default (Ast_Node ast);
Value rul__sem_check_for_each (Ast_Node ast);
Value rul__sem_check_if (Ast_Node ast);
Value rul__sem_check_make (Ast_Node ast, 
			   Boolean return_required,
			   Value return_to_val);
Value rul__sem_check_modify (Ast_Node ast, 
			     Boolean return_required, 
			     Value return_to_val);
Value rul__sem_check_openfile (Ast_Node ast);
Value rul__sem_check_quit (Ast_Node ast);
Value rul__sem_check_remove (Ast_Node ast);
Value rul__sem_check_remove_every (Ast_Node ast);
Value rul__sem_check_restorestate (Ast_Node ast);
Value rul__sem_check_return (Ast_Node ast);
Value rul__sem_check_savestate (Ast_Node ast);
Value rul__sem_check_specialize (Ast_Node ast, 
				 Boolean return_required,
				 Value return_to_val);
Value rul__sem_check_trace (Ast_Node ast);
Value rul__sem_check_while (Ast_Node ast);
Value rul__sem_check_write (Ast_Node ast);

/* SQL action semantic checkers */
Value rul__sem_check_sql_attach (Ast_Node ast);
Value rul__sem_check_sql_commit (Ast_Node ast);
Value rul__sem_check_sql_delete (Ast_Node ast);
Value rul__sem_check_sql_detach (Ast_Node ast);
Value rul__sem_check_sql_fetch_obj (Ast_Node ast, Boolean return_required,
				    Value return_to_value);
Value rul__sem_check_sql_fetch_each (Ast_Node ast);
Value rul__sem_check_sql_insert (Ast_Node ast);
Value rul__sem_check_sql_insert_obj (Ast_Node ast);
Value rul__sem_check_sql_rollback (Ast_Node ast);
Value rul__sem_check_sql_start (Ast_Node ast);
Value rul__sem_check_sql_update (Ast_Node ast);
Value rul__sem_check_sql_update_obj (Ast_Node ast);

/* The following routine returns a value for the arbitrary ast node
 * for LHS values or RHS values...
 *
 *   Parameters:
 *	ast       	      - the AST node
 *	on_lhs		      - Identifies whether this routine is walking
 *				an RHS or LHS external routine call
 *	ce_index	      - If this is a LHS value, this value identifies
 *                              the condition element from which its value
 *                              is being accessed.
 *	return_molecule       - Identifies whether the return value should
 *                              be converted into a molecule
 *	return_domain         - Type of rul molecule to convert return to
 *	return_shape          - Shape of rul molecule to convert return to
 *      return_to_value       - If non NULL, the variable value to hold the
 *			        function return value (for RHS BIND...)
 */
Value rul__sem_check_value (Ast_Node ast,
                            Boolean on_lhs, long ce_index,
                            Boolean return_molecule,
                            Decl_Domain domain, Decl_Shape shape,
                            Value return_to_value);

/* The following function, rul__sem_check_nested_value, is a variant of
** rul__sem_check_value that will create a temporary molecule variable for a
** return_to_value, if one is needed because of the nesting of expressions
*/
Value rul__sem_check_nested_value (Ast_Node ast,
                            Boolean on_lhs, long ce_index,
                            Boolean return_molecule,
                            Decl_Domain domain, Decl_Shape shape,
                            Value return_to_value);


/* The following routine returns a value for the external routine call 
 * including all the argument conversion values, argument values,
 * and call cleanup (replace read-write values; deallocate conversion
 * allocated memory) values.
 *
 *   Parameters:
 *	ext_rt_node	      - the AST node for the external routine call
 * 	return_value_required - Identifies whether this routine
 *				should return an assignment (TRUE)
 *				or function value (FALSE).
 *	on_lhs		      - Identifies whether this routine is walking
 *				an RHS or LHS external routine call
 *	ce_index	      - If this is a LHS value, this value identifies
 *                              the condition element from which its value
 *                              is being accessed.
 *	return_molecule       - Identifies whether the return value should
 *                              be converted into a molecule
 *	return_domain         - Type of rul molecule to convert return to
 *	return_shape          - Shape of rul molecule to convert return to
 *      return_to_value       - If non NULL, the variable value to hold the
 *			        function return value
 */
Value rul__sem_check_ext_rt_call (Ast_Node ext_rt_node,
				  Boolean return_value_required,
				  Boolean on_lhs, 
				  long ce_index,
				  Boolean return_molecule,
				  Decl_Domain return_domain,
				  Decl_Shape return_shape,
				  Value return_to_value);

/* The following routine semantically checks LHS values
 */
Value rul__sem_check_lhs_value (Ast_Node ast, long ce_index,
                                Boolean return_molecule,
                                Decl_Domain domain, Decl_Shape shape);

/* The following routine semantically checks RHS values
 * The second parameter (return_molecule) identifies whether this value
 * is expected to be an rul type (TRUE) or will used as an external (animal)
 * type (FALSE).  If the second parameter is TRUE the third and fourth
 * parameters further define the value.
 * The fifth parameter if non NULL identifies the value to assign the
 * value to (as from a bind action). 
 * It returns NULL if the value is semantically bogus, a value tree
 * otherwise.
 */
Value rul__sem_check_rhs_value (Ast_Node ast, 
				Boolean return_molecule,
				Decl_Domain domain,
				Decl_Shape shape,
				Value return_to_value);

/* The following will return a value expression that includes
 * the supplied rul_value, wrapped within the function(s)
 * necessary to convert it into the appropriate external type.
 */
Value rul__sem_convert_to_ext_value (Value rul_value, Ext_Type type,
				     Cardinality a_len, Value a_arg_val,
				     Ext_Mech mech, Value ext_val,
				     Boolean lhs_rt_call, Ast_Node ast);

/* The following will return a value expression that supplies the
 * function(s) necessary to convert it into the appropriate rul type.
 */
Value rul__sem_convert_to_rul_value (Value ext_value,
				     Value rul_var_value,
				     Ast_Node ast,
				     Value a_arg_value);

/* The following routine modifies a value tree to assure the given
 * value tree returns an external value.  If passed an external constant
 * or external routine call this routine or NULL this routine just 
 * returns it.
 */
Value rul__sem_scalar_val_to_ext (Ast_Node node, Value value,
				  Boolean on_lhs, Ext_Type type);

/* The following routine semantically checks arithmetic expressions
 * It returns NULL if the variable is semantically unusable, a value
 * otherwise.
 */
Value rul__sem_check_arith_expr (Ast_Node ast,
				 Boolean on_lhs, 
                                 long ce_index,
				 Decl_Domain domain,
				 Decl_Shape shape,
				 Value return_to_value);

/* The following routine semantically checks constant expressions
 * It returns an animal value or an rul constant value dependent of
 * the state of the second parameter
 */
Value rul__sem_check_rul_constant (Ast_Node ast,
				   Boolean return_molecule);

/* BTI (built_ins) semantic checkers */

Value rul__sem_check_bti_accept (Ast_Node ast, 
				 Decl_Shape accept_shape,
				 Value return_to_val);
Value rul__sem_check_bti_compound (Ast_Node ast, 
				   Boolean on_lhs,
				   long ce_index,
				   Value return_to_val);
Value rul__sem_check_bti_concat (Ast_Node ast, 
                                 Boolean on_lhs,
                                 long ce_index,
				 Value return_to_val);
Value rul__sem_check_bti_crlf (Ast_Node ast);
Value rul__sem_check_bti_every (Ast_Node ast, 
                                Boolean on_lhs,
                                long ce_index,
				Value return_to_val);
Value rul__sem_check_bti_float (Ast_Node ast, 
                                Boolean on_lhs,
                                long ce_index,
				Value return_to_val);
Value rul__sem_check_bti_genatom (Ast_Node ast, 
				  Value return_to_val);
Value rul__sem_check_bti_genint (Ast_Node ast, 
				  Value return_to_val);
Value rul__sem_check_bti_get (Ast_Node ast, 
			      Decl_Domain domain,
			      Decl_Shape shape,
			      Value return_to_val);
Value rul__sem_check_bti_integer (Ast_Node ast, 
                                  Boolean on_lhs,
                                  long ce_index,
				  Value return_to_val);
Value rul__sem_check_bti_is_open (Ast_Node ast, 
                                  Boolean on_lhs,
                                  long ce_index,
				  Value return_to_val);
Value rul__sem_check_bti_length (Ast_Node ast, 
                                 Boolean on_lhs,
                                 long ce_index,
				 Value return_to_val);
Value rul__sem_check_bti_nth (Ast_Node ast, 
                              Boolean on_lhs,
                              long ce_index,
			      Value return_to_val);
Value rul__sem_check_bti_max (Ast_Node ast,
			      Boolean is_max,
                              Boolean on_lhs,
                              long ce_index,
			      Value return_to_val);
Value rul__sem_check_bti_position (Ast_Node ast, 
                                   Boolean on_lhs,
                                   long ce_index,
				   Value return_to_val);
Value rul__sem_check_bti_rjust (Ast_Node ast);
Value rul__sem_check_bti_subcompound (Ast_Node ast, 
	                              Boolean on_lhs,
        	                      long ce_index,
				      Value return_to_val);
Value rul__sem_check_bti_subsymbol (Ast_Node ast, 
				    Boolean on_lhs,
				    long ce_index,
				    Value return_to_val);
Value rul__sem_check_bti_symbol (Ast_Node ast, 
                                 Boolean on_lhs,
                                 long ce_index,
				 Value return_to_val);
Value rul__sem_check_bti_symbol_len (Ast_Node ast, 
				     Boolean on_lhs,
				     long ce_index,
				     Value return_to_val);
Value rul__sem_check_bti_tabto (Ast_Node ast);

/* Miscellaneous Routines */

Boolean rul__sem_check_value_type (Ast_Node     ast,
				   Decl_Domain  required_domain,
				   Decl_Shape   required_shape,
				   Decl_Domain  return_domain,
				   Decl_Shape   return_shape,
				   char        *msgstring);

Boolean rul__sem_check_ext_rt_param (Ast_Node node, Mol_Symbol rt_name, 
				     Boolean is_the_return_value);

void rul__sem_clear_construct_name (void);

char *rul__sem_domain_to_string (Decl_Domain dom);
char *rul__sem_shape_to_string (Decl_Shape shape);

Boolean rul__sem_is_var_registered (Mol_Symbol var_name);

Boolean rul__sem_check_val_pred_val (Ast_Node node,
				     Pred_Type ptype, Pred_Type opt_ptype,
				     Value val1, Value val2);

Boolean rul__sem_rules_seen (void);

void rul__sem_eb_return_was_seen (void);

Boolean rul__sem_check_linkable (Mol_Symbol block_name);



void rul__sem_tag_rhs_use_of_lhs_var (Molecule var_name);

void rul__sem_set_inits_of_lhs_vars (void);

void rul__sem_set_var_type_from_lhs (Mol_Symbol name);


void rul__sem_get_ext_param_name(char *buffer, Mol_Symbol argname,
				 Ext_Rt_Decl ext_rt);
