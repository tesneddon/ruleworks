/****************************************************************************
**                                                                         **
**                            G E N . H                                    **
**                                                                         **
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
 *	All the top-level GEN subsystem routines.
 *
 * MODIFIED BY:
 *	DEC	Digital Equipment Corporation
 *	CPQ	Compaq Computer Corporation
 *
 * MODIFICATION HISTORY:
 *
 *	21-Dec-1992	DEC	Initial version
 *	01-Dec-1999	CPQ	Release with GPL
 */



#define GEN__C_EB_DATA_STR      "eb_data"
#define GEN__C_METHOD_PARAM_STR "method_params"
#define GEN__C_METHOD_PARAM_CNT "param_cnt"
#define GEN__C_METHOD_RETURN    "method_return"
#define GEN__C_RHS_VARIABLE_STR	"rhs_var"
#define GEN__C_CS_ENTRY_PTR_STR	"cse"

void    rul__gen_cleanup_usefiles (Boolean remove_files);

void    rul__gen_decl_block_usefile (Decl_Block block, char *file_spec,
				       char *block_id);

void	rul__gen_constructs_code (Ast_Node ast);

void	rul__gen_dump_classes(Decl_Block decl_block);

void	rul__gen_rule_rhs (Ast_Node ast);

void	rul__gen_on_rhs (Ast_Node ast);

void	rul__gen_entry_block (void);

void	rul__gen_rule_block (void);

void	rul__gen_end_block (void);

void	rul__gen_match_top_decls (void);
		/*  Should be called when starting a new
		**  rule-block or entry-block  */

void 	rul__gen_prop_func_forw_decls (long number_of_assoc_dblocks);

void	rul__gen_match_network (void);
		/*  Should be called for the end-block of a
		**  rule-block or entry-block  */

void	rul__gen_net_rules_matches (Mol_Symbol rule_name);
		/*  Register the named rule for use in matches command  */
/*

**	Return the names for generated functions
*/
char   *rul__gen_get_cur_ret_val_name (void);
char   *rul__gen_get_cur_rhs_func (void);
		/*
		**  ..._cur_rhs_func should ONLY be used while
		**  generating the function for the rhs of a rule.
		*/

void rul__gen_rhs_var_decl (void);
void rul__gen_rhs_var_inits (void);
void rul__gen_decr_rhs_vars (void);
void rul__gen_decr_rhs_vars_simple (void);

void rul__gen_rhs_tmp_decl (void);
void rul__gen_rhs_tmp_mol_inits (void);
long rul__gen_decr_rhs_tmps (void);
void rul__gen_decr_rhs_tmps_count (long count);

void rul__gen_lhs_tmp_decl (void);
void rul__gen_lhs_tmp_mol_inits (void);
void rul__gen_decr_lhs_tmps (void);

void rul__gen_lhs_var_reference (Value lhs_var);

void rul__gen_on_clause (Ast_Node ast);

void rul__gen_declare_ext_rtns (void);
void rul__gen_declare_dblock_vars (void);

void rul__gen_catcher_func (Ast_Node ast);
void rul__gen_catcher_tables (long block_num);

void rul__gen_generic_method (Ast_Node ast);
void rul__gen_method (Ast_Node ast);

char *rul__gen_pred_func_name (Pred_Type pred);


