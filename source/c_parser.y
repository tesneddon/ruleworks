%{
/* cmp_parser.y - yacc input for RuleWorks language parser */
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
**                                                                         **
****************************************************************************/


/*
**  FACILITY:
**	RuleWorks compiler
**
**  ABSTRACT:
**	Parse source file.  Yacc processes this file and generates
**	cmp_parser_tab.[ch].
**	Each time yyparse (#defined to rul__cmp_yyparse) is called,
**	a construct is parsed and an abstract syntax tree (AST) is
**	built for the construct.  The AST is passed to the ast subsystem,
**	and this function returns 0 on success (via YYACCEPT) or 1 on
**	failure (via YYABORT).
**
**
**  MODIFIED BY:
**	DEC	Digital Equipment Corporation
**	CPQ	Compaq Compuetr Corporation
**
**  MODIFICATION HISTORY:
**
**	23-Dec-1992	DEC	Initial version
**	30-Mar-1994	DEC	Make yacc compatible: Don't define bogus
**					YYMAXDEPTH, don't use %pure_parser bisonism,
**					define yylex() with 0 parameters
**	16-Feb-1998	DEC	class type changed to rclass
**	01-Dec-1999	CPQ	Release with GPL
*/

#include <common.h>
#include <cmp_comm.h>	/* Ast_Node */
#include <scan.h>	/* defines Token_Type and Token_Value & scanner	*/
#include <ast.h>	/* Abstract syntax tree functions */
#include <mol.h>	/* For making molecules out of terminals */
#include <msg.h>
#include <cmp_msg.h>
#include <ctype.h>		/* For toupper() */

extern void rul__main_set_cur_construct (Ast_Node ast);
static void yyerror (char *msg);
static void set_error (void);
static void clear_error (void);
static Boolean in_error (void);
static char *upcase_in_place (char *str);
static Boolean in_construct (void);
static void set_in_construct (void);
void rul__parser_done_with_construct (void);
static void set_cur_construct (Ast_Node_Type typ, Ast_Node name_ast);
static Ast_Node_Type get_construct_type (void);
static char *construct_type_to_name (Ast_Node_Type con_type);
static char *get_construct_name (void);
static Ast_Node compound_is_a_constant (Ast_Node ast);

/*
 * Declare alloca, since Bison code calls it without declaring it.
 */
void *alloca(size_t);


#define yyparse rul__cmp_yyparse

#ifdef YYPURE
#define yylex(yylvalp)  rul__scan_get_token(yylvalp.tok, &SL_line_number)
#else
#define yylex() rul__scan_get_token(&yylval.tok, &SL_line_number)
#endif

#ifdef YYDEBUG
#define YYERROR_VERBOSE	/* be verbose about diagnosing parsing errors	*/
#define xmalloc malloc	/* where is xmalloc defined/declared? use malloc*/
#endif

/* bison.simple does #if YYDEBUG != 0 */
#ifndef YYDEBUG
#define YYDEBUG 0
#endif


static int	SL_line_number = 1; /* scanner position in the source file */

static Molecule	comp_const_val;	/* To build compile-time compound values */
static long	comp_const_index;

%}

/****************************************************************************/
/* DECLARATIONS SECTION							    */

/* %pure_parser not supported by yacc, only bison. */
/*%pure_parser*//* this avoids a clash with an external yylval and typing */

%start	program_construct


/* All nonterminals are of type Ast_Node, and all terminals are of type
 * Token_Value.
 */
%union {
  Ast_Node	node;		/* Nonterminals */
  Pred_Type	pred;		/* For predicate nonterminal */
  long		ival;
  Token_Value	tok;		/* Terminals (tokens) */
}

/* each %token declaration below is for a terminal
   symbol in the grammar recognized by the scanner */

/* First the value tokens are listed.  They should be referenced only in
 * special productions which make nodes out of their values. */
%token <tok> TOK_INTEGER_CONST	/* 3			*/
%token <tok> TOK_FLOAT_CONST	/* 3.1E2		*/
%token <tok> TOK_VARIABLE	/* <VAR>		*/
%token <tok> TOK_QUOTED_VAR	/* '<QVAR>'		*/
%token <tok> TOK_SYMBOL_CONST	/* FOO			*/
%token <tok> TOK_QUOTED_SYMBOL	/* |Quoted symbol|	*/
%token <tok> TOK_OPAQUE_CONST	/* %x0			*/
%token <tok> TOK_INSTANCE_CONST	/* #0			*/

/* The rest of the tokens have values which are constant for each type. */
/* First are special characters, then keywords. */

/* Magic characters */
%token TOK_LPAREN
%token TOK_RPAREN
%token TOK_LBRACE
%token TOK_RBRACE
%token TOK_LBRACKET
%token TOK_RBRACKET
%token TOK_HAT

/* Arithmetic operators, including precedence.  The first line is lowest
 * precedence, the last highest.  UMINUS is a fictitious token, used with
 * the %prec modifier to make unary minus have high precedence.
 */
/* relational operators */
%left TOK_OR
%left TOK_AND
%left TOK_NOT
%left TOK_PLUS TOK_MINUS
%left TOK_TIMES TOK_DIVIDE TOK_MODULUS
%left UMINUS

/* Disjunctions */
%token TOK_START_DISJUNCTION
%token TOK_END_DISJUNCTION

/* Predicates */
%token TOK_EQUAL
%token TOK_EQUAL_EQUAL
%token TOK_APPROX_EQUAL
%token TOK_NOT_APPROX_EQUAL
%token TOK_SAME_TYPE
%token TOK_DIFF_TYPE
%token TOK_NOT_EQ
%token TOK_NOT_EQUAL
%token TOK_LESS
%token TOK_LESS_EQUAL
%token TOK_GREATER
%token TOK_GREATER_EQUAL
%token TOK_CONTAINS
%token TOK_DOES_NOT_CONTAIN
%token TOK_LENGTH_LESS_EQUAL
%token TOK_LENGTH_NOT_EQUAL
%token TOK_LENGTH_LESS
%token TOK_LENGTH_EQUAL
%token TOK_LENGTH_GREATER_EQUAL
%token TOK_LENGTH_GREATER

/* Keywords (in alphabetical order): */
%token TOK_ACCEPT
%token TOK_ACCEPT_ATOM
%token TOK_ACCEPTLINE_COMPOUND
%token TOK_ACCEPTS
%token TOK_ACTIVATES
%token TOK_ADDSTATE
%token TOK_AFTER
%token TOK_ALIAS_FOR
%token TOK_ANY
%token TOK_APPEND
%token TOK_ARROW
%token TOK_ASCID
%token TOK_ASCIZ
%token TOK_AT
%token TOK_ATOM
%token TOK_BIND
%token TOK_BY
%token TOK_BYTE
%token TOK_CALL_INHERITED
%token TOK_CATCH
%token TOK_CLOSEFILE
%token TOK_COMPOUND
%token TOK_CONCAT
%token TOK_COPY
%token TOK_CRLF
%token TOK_CS
%token TOK_DEBUG
%token TOK_DECLARATION_BLOCK
%token TOK_DEFAULT
%token TOK_DO
%token TOK_DOLLAR_LAST
%token TOK_DOUBLE_FLOAT
%token TOK_EB
%token TOK_ELSE
%token TOK_END_BLOCK
%token TOK_END_GROUP
%token TOK_ENTRY_BLOCK
%token TOK_EVERY
%token TOK_EXTERNAL_ROUTINE
%token TOK_FAILURE
%token TOK_FILENAME
%token TOK_FILL
%token TOK_FLOAT
%token TOK_FOR_EACH
%token TOK_GENATOM
%token TOK_GENERIC_METHOD
%token TOK_GENINT
%token TOK_GET
%token TOK_IF
%token TOK_IN
%token TOK_INHERITS_FROM
%token TOK_INSTANCE
%token TOK_INTEGER
%token TOK_IS_OPEN
%token TOK_LENGTH
%token TOK_LEX
%token TOK_LONG
%token TOK_MAKE
%token TOK_MAX
%token TOK_MEA
%token TOK_METHOD
%token TOK_MIN
%token TOK_MODIFY
%token TOK_NTH
%token TOK_OBJECT_CLASS
%token TOK_OF
%token TOK_OFF
%token TOK_ON
%token TOK_ON_EMPTY
%token TOK_ON_ENTRY
%token TOK_ON_EVERY
%token TOK_ON_EXIT
%token TOK_OPAQUE
%token TOK_OPENFILE
%token TOK_OUT
%token TOK_P
%token TOK_PATHNAME
%token TOK_POINTER
%token TOK_POSITION
%token TOK_QUIT
%token TOK_READ_ONLY
%token TOK_READ_WRITE
%token TOK_REFERENCE
%token TOK_REMOVE_EVERY
%token TOK_REMOVE
%token TOK_RESTORESTATE
%token TOK_RETURN
%token TOK_RETURNS
%token TOK_RG
%token TOK_RJUST
%token TOK_RULE
%token TOK_RULE_BLOCK
%token TOK_RULE_GROUP
%token TOK_SAVESTATE
%token TOK_SHORT
%token TOK_SINGLE_FLOAT
%token TOK_SCALAR
%token TOK_SPECIALIZE
%token TOK_SQL_ATTACH
%token TOK_SQL_COMMIT
%token TOK_SQL_DELETE
%token TOK_SQL_DETACH
%token TOK_SQL_FETCH_AS_OBJECT
%token TOK_SQL_FETCH_EACH
%token TOK_SQL_INSERT
%token TOK_SQL_INSERT_FROM_OBJECT
%token TOK_SQL_ROLLBACK
%token TOK_SQL_START
%token TOK_SQL_UPDATE
%token TOK_SQL_UPDATE_FROM_OBJECT
%token TOK_STARTUP
%token TOK_STRATEGY
%token TOK_SUBCOMPOUND
%token TOK_SUBSYMBOL
%token TOK_SUCCESS
%token TOK_SYMBOL
%token TOK_SYMBOL_LENGTH
%token TOK_TABTO
%token TOK_THEN
%token TOK_TRACE
%token TOK_UNSIGNED_BYTE
%token TOK_UNSIGNED_LONG
%token TOK_UNSIGNED_SHORT
%token TOK_USES
%token TOK_VALUE
%token TOK_WHEN
%token TOK_WHILE
%token TOK_WM
%token TOK_WRITE

/* Nonterminals are of type Ast_Node */
%type <node> accept_atom_bti
%type <node> acceptline_compound_bti
%type <node> accepts_clause
%type <node> action
%type <node> action_line
%type <node> action_token
%type <node> actions
%type <node> activates_clause
%type <node> addstate_action
%type <node> after_action
%type <node> alias_clause
%type <node> allowable_func_keyword
%type <node> allowable_func_name
%type <node> arith_expr
%type <node> arith_value
%type <node> at_command
%type <node> attribute
%type <node> attribute_declaration
%type <node> attribute_modifier
%type <node> attribute_name
%type <node> bind_action
%type <node> built_in_keyword
%type <node> built_ins
%type <node> ca_index
%type <node> call_inherited
%type <node> catcher
%type <node> ce
/*
%type <node> ce_disj_list
%type <node> ce_disjunction
*/
%type <node> ce_form
%type <node> rclass
%type <node> classes
%type <node> closefile_action
%type <node> cmplx_expre
%type <node> compound_bti
%type <node> compound_const
%type <node> concat_bti
%type <node> content_predicate
%type <node> constant
%type <node> construct_keyword
%type <node> copy_action
%type <node> crlf_bti
%type <node> db_spec
%type <node> db_spec_name
%type <node> db_scope
%type <node> db_table
%type <node> db_vars
%type <node> debug_action
%type <node> declaration_block
%type <node> default_action
%type <node> end_block
%type <node> end_rule_group
%type <node> expression
%type <node> expre_value
%type <node> entry_block
%type <node> entry_block_clause
%type <node> external_routine_declaration
%type <node> ext_rt_clause
%type <node> ext_rt_param_decl
%type <node> ext_rt_return_decl
%type <node> every_bti
%type <node> float_bti
%type <node> float_const
%type <node> for_each_action
%type <node> function_call
%type <node> genatom_bti
%type <node> genint_bti
%type <node> generic_method_accepts_clause
%type <node> opt_generic_method_returns
%type <node> generic_method_declaration
%type <node> generic_method_when_decl
%type <node> get_bti
%type <node> if_action
%type <node> index_num
%type <node> instance_const
%type <node> integer_bti
%type <node> integer_const
%type <node> is_open_bti
%type <node> length_bti
%type <node> lhs_term
%type <node> lhs_value
%type <node> make_action
%type <node> max_bti
%type <node> modify_action
%type <node> method_accepts_clause
%type <node> method_declaration
%type <node> method_param_decl
%type <node> opt_method_returns_plus_actions
%type <node> method_return_param
%type <node> method_when_decl
%type <node> min_bti
%type <node> name
%type <node> names
%type <node> neg_ce
%type <node> nth_bti
%type <node> object_class_declaration
%type <node> on_off
%type <node> on_empty
%type <node> on_entry
%type <node> on_every
%type <node> on_exit
%type <node> opaque_const
%type <node> openfile_action
%type <node> opt_actions
%type <node> opt_accpt_compound
%type <node> opt_attribute_declarations
%type <node> opt_attribute_modifiers
%type <node> opt_ca_access
%type <node> opt_ce_list
%type <node> opt_comp_element_const
%type <node> opt_compound_flag
%type <node> opt_db_scope
%type <node> opt_domain_class
%type <node> opt_domain_restriction
%type <node> opt_else
%type <node> opt_entry_block_clauses
%type <node> opt_ext_rt_clauses
%type <node> opt_ext_rt_param_decls
%type <node> opt_lhs_terms
%type <node> opt_method_actions
%type <node> opt_method_param_decls
%type <node> opt_method_param_opts
%type <node> opt_name
%type <node> opt_object_class_modifier
%type <node> opt_quit_value
%type <node> opt_rhs_terms
%type <node> opt_rule_block_clauses
%type <node> opt_sql_rse
%type <node> opt_value
%type <node> opt_values
%type <node> opt_param_cardinality
%type <node> opt_param_name
%type <node> opt_param_mech
%type <node> opt_predicate
%type <node> opt_return_mech
%type <node> param_array_length
%type <node> param_type
%type <node> pos_ce
%type <node> position_bti
%type <node> predicate
%type <node> program_construct
%type <node> program_line
%type <node> quit_action
%type <node> quoted_symbol
%type <node> quoted_var
%type <node> remove_action
%type <node> remove_every_action
%type <node> restorestate_action
%type <node> restriction
%type <node> restriction_list
%type <node> return_action
%type <node> returns_clause
%type <node> rhs_term
%type <node> rjust_bti
%type <node> rule
%type <node> rule_block
%type <node> rule_block_clause
%type <node> rule_group_declaration
%type <node> rule_or_p
%type <node> scalar_predicate
%type <node> savestate_action
%type <node> specialize_action
%type <node> sql_actions
%type <node> sql_attach_action
%type <node> sql_commit_action
%type <node> sql_delete_action
%type <node> sql_detach_action
%type <node> sql_fetch_each_action
%type <node> sql_fetch_as_object_action
%type <node> sql_insert_action
%type <node> sql_insert_from_object_action
%type <node> sql_keyword
%type <node> sql_rse
%type <node> sql_rse_element
%type <node> sql_rse_list
%type <node> sql_rollback_action
%type <node> sql_start_action
%type <node> sql_update_action
%type <node> sql_update_from_object_action
%type <node> startup_action
%type <node> startup_actions
%type <node> strategy_clause
%type <node> strategy_name
%type <node> subcompound_bti
%type <node> subsymbol_bti
%type <node> symbol_bti
%type <node> symbol_len_bti
%type <node> symbol_const
%type <node> tabto_bti
%type <node> trace_action
%type <node> trace_name
%type <node> trace_names
%type <node> trace_options
%type <node> uses_clause
%type <node> value
%type <node> value_list
%type <node> value_non_sql
%type <node> variable
%type <node> variables
%type <node> while_action
%type <node> write_action


%%

/****************************************************************************/
/* RULES SECTION							    */

/*
 * Missing: methods and logical constraints,
 * many built-in funcs, lots of actions...
 */

program_construct:	TOK_LPAREN 
			    { /* MRA */ set_in_construct (); }
			  program_line TOK_RPAREN
			    {
			    if (in_error()) {
				clear_error();
				YYABORT;
				}
			    else {
			        rul__main_set_cur_construct ($3);
			        YYACCEPT; /* Return after each construct */
				}
			    }
		|	error
			    {
			    yyclearin;
			    clear_error ();
			    yyclearin;
			    yyerrok;
			    YYABORT;
			    }
;

program_line:		declaration_block
		|	entry_block
		|	rule_block
		|	end_block
		|	rule_group_declaration
		|	end_rule_group
		|	object_class_declaration
		|	method_declaration
		|	generic_method_declaration
		|	external_routine_declaration
		|	on_entry
		|	on_empty
		|	on_exit
		|	on_every
		|	rule
		|	catcher
		|	error
			    {
			    yyclearin;
			    $$ = NULL;
			    }
;

/*
 * DECLARATION BLOCK
 */

declaration_block:	TOK_DECLARATION_BLOCK
			    { /* MRA */ set_cur_construct (
						AST__E_DECL_BLOCK, NULL); }
			name
			    {
			    $$ = rul__ast_node (AST__E_DECL_BLOCK);
			    rul__ast_attach_child ($$, $3);
			    }
;


/*
 * ENTRY BLOCK
 */

entry_block:		TOK_ENTRY_BLOCK name 
			    { /* MRA */ set_cur_construct (
						AST__E_ENTRY_BLOCK, $2); }
			opt_entry_block_clauses
			    {
			    $$ = rul__ast_node (AST__E_ENTRY_BLOCK);
			    rul__ast_attach_children ($$, 2, $2, $4);
			    }
;

opt_entry_block_clauses:
			/* empty */
			    {  $$ = NULL;  }
		|	opt_entry_block_clauses
			TOK_LPAREN entry_block_clause TOK_RPAREN
			    {
			    $$ = rul__ast_append_opt_sibling ($1, $3);
			    }
;

entry_block_clause:	accepts_clause
		|	returns_clause
		|	activates_clause
		|	uses_clause
		|	strategy_clause
;

activates_clause:	TOK_ACTIVATES names
			    {
			    $$ = rul__ast_node (AST__E_ACTIVATES);
			    rul__ast_attach_child ($$, $2);
			    }
;

uses_clause:		TOK_USES names
			    {
			    $$ = rul__ast_node (AST__E_USES);
			    rul__ast_attach_child ($$, $2);
			    }
;

strategy_clause:	TOK_STRATEGY strategy_name
			    {
			    $$ = $2;
			    }
;

on_entry:		TOK_ON_ENTRY
			    { /* MRA */ set_cur_construct (
						AST__E_ON_ENTRY, NULL); }
			startup_actions
			    {
			    $$ = rul__ast_node (AST__E_ON_ENTRY);
			    rul__ast_attach_child ($$, $3);
			    }
		|	TOK_STARTUP startup_actions
			    {
			    $$ = rul__ast_node (AST__E_ON_ENTRY);
			    rul__ast_attach_child ($$, $2);
			    rul__msg_cmp_print (CMP_OBSSTARTUP, $$);
			    }
;

on_empty:		TOK_ON_EMPTY
			    { /* MRA */ set_cur_construct (
						AST__E_ON_EMPTY, NULL); }
			actions
			    {
			    $$ = rul__ast_node (AST__E_ON_EMPTY);
			    rul__ast_attach_child ($$, $3);
			    }
;

on_exit:		TOK_ON_EXIT
			    { /* MRA */ set_cur_construct (
						AST__E_ON_EXIT, NULL); }
			actions
			    {
			    $$ = rul__ast_node (AST__E_ON_EXIT);
			    rul__ast_attach_child ($$, $3);
			    }
;

on_every:		TOK_ON_EVERY
			    { /* MRA */ set_cur_construct (
						AST__E_ON_EVERY, NULL); }
			actions
			    {
			    $$ = rul__ast_node (AST__E_ON_EVERY);
			    rul__ast_attach_child ($$, $3);
			    }
;

startup_actions:	TOK_LPAREN startup_action TOK_RPAREN
			    {
			    $$ = $2;
			    }
		|	startup_actions TOK_LPAREN startup_action TOK_RPAREN
			    {
			    $$ = rul__ast_append_opt_sibling ($1, $3);
			    }
;

startup_action:		action
		|	at_command
/*		|	disable_command
		|	enable_command
		|	trace_command
*/;


at_command:		TOK_AT name
			    { $$ = rul__ast_node (AST__E_AT_ACTION);
			    rul__ast_attach_children ($$, 1, $2);
			    }
;

opt_actions:		/* empty */
			    { $$ = NULL;
			      rul__msg_cmp_print_w_line (
					 CMP_NOACTION,
					 SL_line_number);
			    }
		|	actions
			    { $$ = $1; }
;

opt_method_actions:	/* empty */
			    { $$ = NULL; }
		|	actions
			    { $$ = $1; }
;

actions: 		action_line
		|	actions action_line
			    {
			    $$ = rul__ast_append_opt_sibling ($1, $2);
			    }
		|	error
			    {
			    yyclearin;
			    $$ = NULL;
			    }
;	

action_line:		TOK_LPAREN action TOK_RPAREN
			    {  $$ = $2;  }
		|	TOK_LPAREN construct_keyword
			    {
			    rul__msg_cmp_print_w_line (
			        CMP_UNTERMCON,
				SL_line_number);
			    rul__msg_print (CMP_UNTERMIGN);

			    rul__scan_unget_tokens (2);
			    yyclearin;
			    clear_error ();
			    yyerrok;
			    YYABORT;
			    }
;

action:			addstate_action
		|	after_action
		|	bind_action
		|	call_inherited
		|	closefile_action
		|	copy_action
		|	debug_action
		|	default_action
		|	for_each_action
		|	function_call
		|	if_action
		|	make_action
		|	modify_action
		|	openfile_action
                |       quit_action
		|	remove_action
		|	remove_every_action
		|	restorestate_action
		|	return_action
		|	savestate_action
		|	specialize_action
		|	trace_action
		|	while_action
		|	write_action
		|	sql_attach_action
		|	sql_commit_action
		|	sql_delete_action
		|	sql_detach_action
		|	sql_fetch_each_action
		|	sql_fetch_as_object_action
		|	sql_insert_action
		|	sql_insert_from_object_action
		|	sql_rollback_action
		|	sql_start_action
		|	sql_update_action
		|	sql_update_from_object_action
		|	error
			    {
			    yyclearin;
			    $$ = NULL;
			    }
;

addstate_action:	TOK_ADDSTATE value
			    {
			    $$ = rul__ast_node (AST__E_ADDSTATE_ACTION);
			    rul__ast_attach_children ($$, 1, $2);
			    }
;

after_action:		TOK_AFTER value name
			    {
			    $$ = rul__ast_node (AST__E_AFTER_ACTION);
			    rul__ast_attach_children ($$, 2, $2, $3);
			    }
;

bind_action:		TOK_BIND variable value
			    {
			    $$ = rul__ast_node (AST__E_BIND_ACTION);
			    rul__ast_attach_children ($$, 2, $2, $3);
			    }
;

call_inherited:		TOK_CALL_INHERITED variables
			    {
			    $$ = rul__ast_node (AST__E_CALL_INHERITED);
			    rul__ast_attach_children ($$, 1, $2);
			    }
;

closefile_action:	TOK_CLOSEFILE value_list
			    {
			    $$ = rul__ast_node (AST__E_CLOSEFILE_ACTION);
			    rul__ast_attach_children ($$, 1, $2);
			    }
;

copy_action:		TOK_COPY variable opt_rhs_terms
			    {
			    $$ = rul__ast_node (AST__E_COPY_ACTION);
			    rul__ast_attach_children ($$, 2, $2, $3);
			    }
;

debug_action:		TOK_DEBUG
			    {
			    $$ = rul__ast_node (AST__E_DEBUG_ACTION);
			    }
;

default_action:		TOK_DEFAULT value value
			    {
			    $$ = rul__ast_node (AST__E_DEFAULT_ACTION);
			    rul__ast_attach_children ($$, 2, $2, $3);
			    }
;

for_each_action:	TOK_FOR_EACH variable TOK_IN value actions
			    {
			    $$ = rul__ast_node (AST__E_FOR_EACH_ACTION);
			    rul__ast_attach_children ($$, 3, $2, $4, $5);
			    }
;

if_action:		TOK_IF expression TOK_THEN actions opt_else
			    {
			    Ast_Node if_then;

			    $$ = rul__ast_node (AST__E_IF_ACTION);
			    if_then = rul__ast_node (AST__E_IF_THEN);
			    rul__ast_attach_child (if_then, $4);
			    rul__ast_attach_children ($$, 3, $2, if_then, $5);
			    }
;

make_action:		TOK_MAKE rclass opt_rhs_terms
			    {
			    $$ = rul__ast_node (AST__E_MAKE_ACTION);
			    rul__ast_attach_children ($$, 2, $2, $3);
			    }
;

modify_action:		TOK_MODIFY variable opt_rhs_terms
			    {
			    $$ = rul__ast_node (AST__E_MODIFY_ACTION);
			    rul__ast_attach_children ($$, 2, $2, $3);
			    }
;

openfile_action:	TOK_OPENFILE value value value
			    {
			    $$ = rul__ast_node (AST__E_OPENFILE_ACTION);
			    rul__ast_attach_children ($$, 3, $2, $3, $4);
			    }
;

quit_action:		TOK_QUIT opt_quit_value
			    {
			    $$ = rul__ast_node (AST__E_QUIT_ACTION);
			    rul__ast_attach_child ($$, $2);
			    }
;

opt_quit_value:		/* empty */
			    { $$ = NULL; }
		|	arith_value
		|	TOK_FAILURE
			    { $$ = rul__ast_node (AST__E_FAILURE); }
		|	TOK_SUCCESS
			    { $$ = rul__ast_node (AST__E_SUCCESS); }
;
		
remove_action:		TOK_REMOVE variables
			    {
			    $$ = rul__ast_node (AST__E_REMOVE_ACTION);
			    rul__ast_attach_child ($$, $2);
			    }
;

remove_every_action:	TOK_REMOVE_EVERY classes
			    {
			    $$ = rul__ast_node (AST__E_REMOVE_EVERY_ACTION);
			    rul__ast_attach_child ($$, $2);
			    }
;

restorestate_action:	TOK_RESTORESTATE value
			    {
			    $$ = rul__ast_node (AST__E_RESTORESTATE_ACTION);
			    rul__ast_attach_children ($$, 1, $2);
			    }
;

return_action:		TOK_RETURN opt_value
			    {
			    $$ = rul__ast_node (AST__E_RETURN_ACTION);
			    rul__ast_attach_children ($$, 1, $2);
			    }
;

savestate_action:	TOK_SAVESTATE value
			    {
			    $$ = rul__ast_node (AST__E_SAVESTATE_ACTION);
			    rul__ast_attach_children ($$, 1, $2);
			    }
;

specialize_action:	TOK_SPECIALIZE variable rclass opt_rhs_terms
			    {
			    $$ = rul__ast_node (AST__E_SPECIALIZE_ACTION);
			    rul__ast_attach_children ($$, 3, $2, $3, $4);
			    }
;

trace_action:		TOK_TRACE on_off trace_options
			    {
			    $$ = rul__ast_node (AST__E_TRACE_ACTION);
			    rul__ast_attach_children ($$, 2, $2, $3);
			    }
;

while_action:		TOK_WHILE expression TOK_DO actions
			    {
			    Ast_Node while_do;

			    $$ = rul__ast_node (AST__E_WHILE_ACTION);
			    while_do = rul__ast_node (AST__E_WHILE_DO);
			    rul__ast_attach_child (while_do, $4);
			    rul__ast_attach_children ($$, 2, $2, while_do);
			    }
;

write_action:		TOK_WRITE opt_values
			    {
			    $$ = rul__ast_node (AST__E_WRITE_ACTION);
			    rul__ast_attach_child ($$, $2);
			    }
;

sql_attach_action:   	TOK_SQL_ATTACH db_spec opt_db_scope
			    {
			    $$ = rul__ast_node (AST__E_SQL_ATTACH_ACTION);
			    rul__ast_attach_children ($$, 2, $2, $3);
			    }
;

sql_commit_action:	TOK_SQL_COMMIT
			    {
			    $$ = rul__ast_node (AST__E_SQL_COMMIT_ACTION);
			    }
;

sql_delete_action:	TOK_SQL_DELETE db_table opt_sql_rse
			    {
			    $$ = rul__ast_node (AST__E_SQL_DELETE_ACTION);
			    rul__ast_attach_children ($$, 2, $2, $3);
			    }
;

sql_detach_action:	TOK_SQL_DETACH
			    {
			    $$ = rul__ast_node (AST__E_SQL_DETACH_ACTION);
			    }
;

sql_fetch_each_action:	TOK_SQL_FETCH_EACH db_vars
				TOK_LPAREN sql_rse TOK_RPAREN sql_actions
			    {
			    $$ = rul__ast_node (AST__E_SQL_FETCH_EACH_ACTION);
			    rul__ast_attach_children ($$, 3, $2, $4, $6);
			    }
;

sql_fetch_as_object_action: TOK_SQL_FETCH_AS_OBJECT sql_rse
			{
		        $$ = rul__ast_node (AST__E_SQL_FETCH_AS_OBJECT_ACTION);
			rul__ast_attach_child ($$, $2);
			}
;

sql_insert_action:	TOK_SQL_INSERT db_table sql_rse
			    {
			    $$ = rul__ast_node (AST__E_SQL_INSERT_ACTION);
			    rul__ast_attach_children ($$, 2, $2, $3);
			    }
;

sql_insert_from_object_action:	TOK_SQL_INSERT_FROM_OBJECT variable
		     {
		     $$ = rul__ast_node (AST__E_SQL_INSERT_FROM_OBJECT_ACTION);
		     rul__ast_attach_child ($$, $2);
		     }
;

sql_rollback_action:	TOK_SQL_ROLLBACK
			    {
			    $$ = rul__ast_node (AST__E_SQL_ROLLBACK_ACTION);
			    }
;

sql_start_action:	TOK_SQL_START opt_sql_rse
			    {
			    $$ = rul__ast_node (AST__E_SQL_START_ACTION);
			    rul__ast_attach_child ($$, $2);
			    }
;

sql_update_action:	TOK_SQL_UPDATE db_table sql_rse
			    {
			    $$ = rul__ast_node (AST__E_SQL_UPDATE_ACTION);
			    rul__ast_attach_children ($$, 2, $2, $3);
			    }
;

sql_update_from_object_action:	TOK_SQL_UPDATE_FROM_OBJECT variable opt_sql_rse
		     {
		     $$ = rul__ast_node (AST__E_SQL_UPDATE_FROM_OBJECT_ACTION);
		     rul__ast_attach_children ($$, 2, $2, $3);
		     }
;

db_spec:		TOK_FILENAME name
  			    { $$ = $2; }
		|	TOK_PATHNAME name
		            {
		            $$ = rul__ast_node (AST__E_SQL_PATHNAME);
		            rul__ast_attach_child ($$, $2);
		            }
		|	db_spec_name
;

opt_db_scope:		/* empty */
			    { $$ = NULL; }
		|	db_scope
;

db_scope:		name
;

db_table:		name
;

db_vars:		variables
;

opt_sql_rse:		/* empty */
			    {  $$ = NULL;  }
		|	sql_rse
;

sql_actions:		actions
		            {
		            $$ = rul__ast_node (AST__E_SQL_ACTIONS);
		            rul__ast_attach_child ($$, $1);
		            }
;

sql_rse:		sql_rse_list
		            {
		            $$ = rul__ast_node (AST__E_SQL_RSE);
		            rul__ast_attach_child ($$, $1);
		            }
;

sql_rse_list:		sql_rse_list sql_rse_element
			    {
			    $$ = rul__ast_append_opt_sibling ($1, $2);
			    }
		|	sql_rse_element
;

sql_rse_element:	constant
		|	variable
		|	quoted_var
		|	TOK_EQUAL
			    { $$ = rul__ast_mol_node (AST__E_CONSTANT,
				      rul__mol_make_symbol (yylval.tok.sval));
			    }
		|	TOK_EQUAL_EQUAL
			    { $$ = rul__ast_mol_node (AST__E_CONSTANT,
				      rul__mol_make_symbol (yylval.tok.sval));
			    }
		|	TOK_APPROX_EQUAL
			    { $$ = rul__ast_mol_node (AST__E_CONSTANT,
				      rul__mol_make_symbol (yylval.tok.sval));
			    }
		|	TOK_NOT_APPROX_EQUAL
			    { $$ = rul__ast_mol_node (AST__E_CONSTANT,
				      rul__mol_make_symbol (yylval.tok.sval));
			    }
		|	TOK_NOT_EQ
			    { $$ = rul__ast_mol_node (AST__E_CONSTANT,
				      rul__mol_make_symbol (yylval.tok.sval));
			    }
		|	TOK_NOT_EQUAL
			    { $$ = rul__ast_mol_node (AST__E_CONSTANT,
				      rul__mol_make_symbol (yylval.tok.sval));
			    }
		|	TOK_GREATER
			    { $$ = rul__ast_mol_node (AST__E_CONSTANT,
				      rul__mol_make_symbol (yylval.tok.sval));
			    }
		|	TOK_GREATER_EQUAL
			    { $$ = rul__ast_mol_node (AST__E_CONSTANT,
				      rul__mol_make_symbol (yylval.tok.sval));
			    }
		|	TOK_LESS
			    { $$ = rul__ast_mol_node (AST__E_CONSTANT,
				      rul__mol_make_symbol (yylval.tok.sval));
			    }
		|	TOK_LESS_EQUAL
			    { $$ = rul__ast_mol_node (AST__E_CONSTANT,
				      rul__mol_make_symbol (yylval.tok.sval));
			    }
		|	TOK_PLUS
			    { $$ = rul__ast_mol_node (AST__E_CONSTANT,
				      rul__mol_make_symbol (yylval.tok.sval));
			    }
		|	TOK_MINUS
			    { $$ = rul__ast_mol_node (AST__E_CONSTANT,
				      rul__mol_make_symbol (yylval.tok.sval));
			    }
		|	TOK_TIMES
			    { $$ = rul__ast_mol_node (AST__E_CONSTANT,
				      rul__mol_make_symbol (yylval.tok.sval));
			    }
		|	TOK_DIVIDE
			    { $$ = rul__ast_mol_node (AST__E_CONSTANT,
				      rul__mol_make_symbol (yylval.tok.sval));
			    }
		|	TOK_MODULUS
			    { $$ = rul__ast_mol_node (AST__E_CONSTANT,
				      rul__mol_make_symbol (yylval.tok.sval));
			    }
		|	TOK_ARROW
			    { $$ = rul__ast_mol_node (AST__E_CONSTANT,
				      rul__mol_make_symbol (yylval.tok.sval));
			    }
		|	TOK_START_DISJUNCTION
			    { $$ = rul__ast_mol_node (AST__E_CONSTANT,
				      rul__mol_make_symbol (yylval.tok.sval));
			    }
		|	TOK_END_DISJUNCTION
			    { $$ = rul__ast_mol_node (AST__E_CONSTANT,
				      rul__mol_make_symbol (yylval.tok.sval));
			    }
;

opt_rhs_terms:		/* empty */
			    {  $$ = NULL;  }
		|	opt_rhs_terms rhs_term
			    {
			    $$ = rul__ast_append_opt_sibling ($1, $2);
			    }
;

rhs_term:		TOK_HAT attribute opt_ca_access value
			    {
			    $$ = rul__ast_node (AST__E_RHS_ACTION_TERM);
			    rul__ast_attach_children ($$, 2, $2, $4);
			    rul__ast_attach_child ($2, $3);
			    }
		|	error
			    { yyclearin;  $$ = NULL; }
;

rclass:			name
  		|	variable
		|	TOK_LPAREN accept_atom_bti TOK_RPAREN
			    {  $$ = $2;  }
		|	TOK_LPAREN concat_bti TOK_RPAREN
			    {  $$ = $2;  }
		|	TOK_LPAREN get_bti TOK_RPAREN
			    {  $$ = $2;  }
		|	TOK_LPAREN nth_bti TOK_RPAREN
			    {  $$ = $2;  }
		|	TOK_LPAREN symbol_bti TOK_RPAREN
			    {  $$ = $2;  }
		|	TOK_LPAREN function_call TOK_RPAREN
			    {  $$ = $2;  }
;

classes:		classes rclass
			    {
			    $$ = rul__ast_append_opt_sibling ($1, $2);
			    }
		|	rclass
;

attribute:		name
		|	variable
;

catcher:		TOK_CATCH name 
			    { /* MRA */ set_cur_construct (AST__E_CATCH, $2);}
			  opt_actions
			    {
			    $$ = rul__ast_node (AST__E_CATCH);
			    rul__ast_attach_children ($$, 2, $2, $4);
			    }
;

expression:		TOK_LPAREN cmplx_expre TOK_RPAREN
			    { $$ = $2; }
;

cmplx_expre:		expre_value
		|       cmplx_expre TOK_AND cmplx_expre
			    {
			    $$ = rul__ast_node (AST__E_OPR_AND);
			    rul__ast_attach_child ($$, $1);
			    rul__ast_append_sibling ($1, $3);
			    }
		|       cmplx_expre TOK_OR cmplx_expre
			    {
			    $$ = rul__ast_node (AST__E_OPR_OR);
			    rul__ast_attach_child ($$, $1);
			    rul__ast_append_sibling ($1, $3);
			    }
		|       TOK_NOT expre_value
			    {
			    $$ = rul__ast_node (AST__E_OPR_NOT);
			    rul__ast_attach_child ($$, $2);
			    }
;

expre_value:	        value_non_sql predicate value_non_sql
			    {
			    $$ = rul__ast_node (AST__E_COND_EXPR);
			    rul__ast_attach_children ($$, 3, $1, $2, $3);
			    }
		|       TOK_LPAREN cmplx_expre TOK_RPAREN
			    {
			    $$ = $2;
			    }
;

opt_else:		/* empty */
			    {  $$ = NULL;  }
		|	TOK_ELSE actions
			    {
			    $$ = rul__ast_node (AST__E_IF_ELSE);
			    rul__ast_attach_child ($$, $2);
			    }
;

/*
 * BUILT-IN FUNCTIONS
 */

built_ins:		accept_atom_bti
		|	acceptline_compound_bti
		|	compound_bti
		|	concat_bti
		|	crlf_bti
		|	every_bti
		|	float_bti
		|	genatom_bti
		|	genint_bti
		|	get_bti
		|	integer_bti
		|	is_open_bti
		|	length_bti
		|	max_bti
		|	min_bti
		|	nth_bti
		|	position_bti
		|	rjust_bti
		|	subcompound_bti
		|	subsymbol_bti
		|	symbol_bti
		|	symbol_len_bti
		|	tabto_bti
;

accept_atom_bti:	TOK_ACCEPT_ATOM opt_value
			    {
			    $$ = rul__ast_node (AST__E_BTI_ACCEPT_ATOM);
			    rul__ast_attach_child ($$, $2);
			    }
;

acceptline_compound_bti: TOK_ACCEPTLINE_COMPOUND opt_accpt_compound
			    {
			    $$ = rul__ast_node(AST__E_BTI_ACCEPTLINE_COMPOUND);
			    rul__ast_attach_child ($$, $2);
			    }
;

opt_accpt_compound:	/* empty */
			    { $$ = NULL; }  
  		|	value opt_value
			    { $$ = rul__ast_append_sibling ($1, $2); }
;

compound_bti:		TOK_COMPOUND opt_values
			    {
			    $$ = compound_is_a_constant ($2);
			    if ($$ == NULL) {
			        $$ = rul__ast_node (AST__E_BTI_COMPOUND);
			        rul__ast_attach_child ($$, $2);
			        }
			    }
;

concat_bti:		TOK_CONCAT opt_values
			    {
			    $$ = rul__ast_node (AST__E_BTI_CONCAT);
			    rul__ast_attach_child ($$, $2);
			    }
;

crlf_bti:		TOK_CRLF 
			    {
			    $$ = rul__ast_node(AST__E_BTI_CRLF);
			    }
;

every_bti:		TOK_EVERY rclass
			    {
			    $$ = rul__ast_node(AST__E_BTI_EVERY);
			    rul__ast_attach_child ($$, $2);
			    }
;

float_bti:		TOK_FLOAT value
			    {
			    $$ = rul__ast_node(AST__E_BTI_FLOAT);
			    rul__ast_attach_child ($$, $2);
			    }
;

genatom_bti:		TOK_GENATOM opt_value
			    {
			    $$ = rul__ast_node(AST__E_BTI_GENATOM);
			    rul__ast_attach_child ($$, $2);
			    }
;

genint_bti:		TOK_GENINT
			    {
			    $$ = rul__ast_node(AST__E_BTI_GENINT);
			    }
;

get_bti:		TOK_GET value TOK_HAT attribute
			    {
			    $$ = rul__ast_node(AST__E_BTI_GET);
			    rul__ast_attach_children ($$, 2, $2, $4);
			    }
;

integer_bti:		TOK_INTEGER value
			    {
			    $$ = rul__ast_node(AST__E_BTI_INTEGER);
			    rul__ast_attach_child ($$, $2);
			    }
;

is_open_bti:		TOK_IS_OPEN value
			    {
			    $$ = rul__ast_node(AST__E_BTI_IS_OPEN);
			    rul__ast_attach_child ($$, $2);
			    }
;

length_bti:		TOK_LENGTH value
			    {
			    $$ = rul__ast_node(AST__E_BTI_LENGTH);
			    rul__ast_attach_child ($$, $2);
			    }
;

max_bti:		TOK_MAX opt_values
			    {
			    $$ = rul__ast_node (AST__E_BTI_MAX);
			    rul__ast_attach_child ($$, $2);
			    }
;

min_bti:		TOK_MIN opt_values
			    {
			    $$ = rul__ast_node (AST__E_BTI_MIN);
			    rul__ast_attach_child ($$, $2);
			    }
;

nth_bti:		TOK_NTH value index_num
			    {
			    $$ = rul__ast_node(AST__E_BTI_NTH);
			    rul__ast_attach_children ($$, 2, $2, $3);
			    }
;

position_bti:		TOK_POSITION value opt_predicate value
			    {
			    $$ = rul__ast_node(AST__E_BTI_POSITION);
			    rul__ast_attach_children ($$, 3, $2, $3, $4);
			    }
;

rjust_bti:		TOK_RJUST value
			    {
			    $$ = rul__ast_node(AST__E_BTI_RJUST);
			    rul__ast_attach_child ($$, $2);
			    }
;

subcompound_bti:	TOK_SUBCOMPOUND value index_num index_num
			    {
			    $$ = rul__ast_node(AST__E_BTI_SUBCOMPOUND);
			    rul__ast_attach_children ($$, 3, $2, $3, $4);
			    }
;

subsymbol_bti:		TOK_SUBSYMBOL value index_num index_num
			    {
			    $$ = rul__ast_node(AST__E_BTI_SUBSYMBOL);
			    rul__ast_attach_children ($$, 3, $2, $3, $4);
			    }
;

symbol_bti:		TOK_SYMBOL value
			    {
			    $$ = rul__ast_node(AST__E_BTI_SYMBOL);
			    rul__ast_attach_child ($$, $2);
			    }
;

symbol_len_bti:		TOK_SYMBOL_LENGTH value
			    {
			    $$ = rul__ast_node(AST__E_BTI_SYMBOL_LENGTH);
			    rul__ast_attach_child ($$, $2);
			    }
;

tabto_bti:		TOK_TABTO value
			    {
			    $$ = rul__ast_node(AST__E_BTI_TABTO);
			    rul__ast_attach_child ($$, $2);
			    }
;


/*
 * RULE BLOCK
 */

rule_block:		TOK_RULE_BLOCK name 
			    { /* MRA */ set_cur_construct (
						AST__E_RULE_BLOCK, $2); }
			opt_rule_block_clauses
			    {
			    $$ = rul__ast_node (AST__E_RULE_BLOCK);
			    rul__ast_attach_children ($$, 2, $2, $4);
			    }
;

opt_rule_block_clauses:	/* empty */
			    {  $$ = NULL;  }
		|	opt_rule_block_clauses
			 TOK_LPAREN rule_block_clause TOK_RPAREN
			{
			$$ = rul__ast_append_opt_sibling ($1, $3);
			}
;

rule_block_clause:	uses_clause
		|	strategy_clause
;

strategy_name:		TOK_LEX
			{
			$$ = rul__ast_node (AST__E_STRATEGY);
			rul__ast_attach_long_value ($$, AST__E_TYPE_STRATEGY,
						    CS__C_LEX);
			}
		|	TOK_MEA
			{ 
			$$ = rul__ast_node (AST__E_STRATEGY);
			rul__ast_attach_long_value ($$, AST__E_TYPE_STRATEGY,
						    CS__C_MEA);
			}
;

/*
 * END BLOCK
 */

end_block:		TOK_END_BLOCK 
			    { /* MRA */ set_cur_construct (
						AST__E_END_BLOCK, NULL); }
			opt_name
			    {
			    $$ = rul__ast_node (AST__E_END_BLOCK);
			    rul__ast_attach_child ($$, $3);
			    }
;


/*
 * RULE GROUP
 */

rule_group_declaration:	TOK_RULE_GROUP name
			    {
			    $$ = rul__ast_node (AST__E_RULE_GROUP);
			    rul__ast_attach_child ($$, $2);
			    set_cur_construct (AST__E_RULE_GROUP, $2);
			    }
;

end_rule_group:		TOK_END_GROUP opt_name
			    {
			    $$ = rul__ast_node (AST__E_END_GROUP);
			    rul__ast_attach_child ($$, $2);
			    set_cur_construct (AST__E_END_GROUP, NULL);
			    }
;


/*
 * OBJECT CLASS
 */

object_class_declaration:
			TOK_OBJECT_CLASS name
			    {/*MRA*/ set_cur_construct (AST__E_OBJ_CLASS, $2);}
			 opt_object_class_modifier
			 opt_attribute_declarations
			    {
			    Ast_Node attrs;

			    $$ = rul__ast_node (AST__E_OBJ_CLASS);
			    attrs = rul__ast_node (AST__E_OBJ_ATTRS);
			    rul__ast_attach_child (attrs, $5);
			    rul__ast_attach_children ($$, 3, $2, $4, attrs);
			    }
;

opt_object_class_modifier:
			/* empty */
			    { $$ = NULL; }
		|	TOK_LPAREN TOK_INHERITS_FROM name TOK_RPAREN
			    {
			    $$ = rul__ast_node (AST__E_OBJ_CLASS_INH);
			    rul__ast_attach_child ($$, $3);
			    }
;

opt_attribute_declarations:
			/* empty */
			    {  $$ = NULL;  }
		|	opt_attribute_declarations attribute_declaration
			    {
			    $$ = rul__ast_append_opt_sibling ($1, $2);
			    }
;

attribute_declaration:	TOK_HAT attribute_name opt_compound_flag
				opt_domain_restriction opt_attribute_modifiers
			    {
			    $$ = rul__ast_node (AST__E_ATTRIBUTE);
			    rul__ast_attach_children ($$, 4, $2, $3, $4, $5);
			    }
;

attribute_name:		name
;

opt_compound_flag:
			/* empty */
			    {  $$ = NULL;  }
		|	TOK_COMPOUND
			    {
			    $$ = rul__ast_node (AST__E_ATTR_COMPOUND);
			    }
;

opt_domain_restriction:
			/* empty */
			    { $$ = NULL;  }
                |       TOK_ANY
			    { $$ = NULL;  }
		|	TOK_INSTANCE opt_domain_class
			    {
			    $$ = rul__ast_mol_node (
					AST__E_ATTR_DOMAIN,
				    	rul__mol_make_symbol ("INSTANCE"));
			    rul__ast_attach_children ($$, 1, $2);
			    }
		|	TOK_INTEGER
			    {
			    $$ = rul__ast_mol_node (
					AST__E_ATTR_DOMAIN,
				    	rul__mol_make_symbol ("INTEGER"));
			    }
		|	TOK_FLOAT
			    {
			    $$ = rul__ast_mol_node (
					AST__E_ATTR_DOMAIN,
				    	rul__mol_make_symbol ("FLOAT"));
			    }
		|	TOK_OPAQUE
			    {
			    $$ = rul__ast_mol_node (
					AST__E_ATTR_DOMAIN,
				    	rul__mol_make_symbol ("OPAQUE"));
			    }
		|	TOK_SYMBOL
			    {
			    $$ = rul__ast_mol_node (
					AST__E_ATTR_DOMAIN,
				    	rul__mol_make_symbol ("SYMBOL"));
			    }
;

opt_domain_class:
			/* empty */
			    {  $$ = NULL;  }
		|	TOK_OF name
			    {
			    rul__ast_reset_node_type ($2, AST__E_ATTR_CLASS);
			    $$ = $2;
			    }
;

opt_attribute_modifiers:
			/* empty */
			    {  $$ = NULL;  }
		|	opt_attribute_modifiers attribute_modifier
			    {
			    $$ = rul__ast_append_opt_sibling ($1, $2);
			    }
;

attribute_modifier:	TOK_LPAREN TOK_DEFAULT constant TOK_RPAREN
			    {
			    $$ = rul__ast_node (AST__E_ATTR_DEFAULT);
			    rul__ast_attach_child ($$, $3);
			    }
		|	TOK_LPAREN TOK_DEFAULT compound_const TOK_RPAREN
			    {
			    $$ = rul__ast_node (AST__E_ATTR_COMPOUND_DEFAULT);
			    rul__ast_attach_child ($$, $3);
			    }
		|	TOK_LPAREN TOK_FILL constant TOK_RPAREN
			    {
			    $$ = rul__ast_node (AST__E_ATTR_COMPOUND_FILL);
			    rul__ast_attach_child ($$, $3);
			    }
;


/*
 * GENERIC-METHOD-DECLARATION
 */

generic_method_declaration:
  			TOK_GENERIC_METHOD allowable_func_name
			    { /* MRA */ set_cur_construct (
						AST__E_GENERIC_METHOD, $2); }
                        generic_method_accepts_clause
                        opt_generic_method_returns
			    {
			    $$ = rul__ast_node (AST__E_GENERIC_METHOD);
			    rul__ast_attach_children ($$, 3, $2, $4, $5);
			    }
;


generic_method_accepts_clause:
			TOK_LPAREN TOK_ACCEPTS generic_method_when_decl
                        opt_method_param_decls TOK_RPAREN
			    {
			    $$ = rul__ast_node (AST__E_ACCEPTS_DECL);
			    rul__ast_attach_children ($$, 2, $3, $4);
			    }
;

opt_generic_method_returns:
			/* empty */
			    {  $$ = NULL; }
		|	TOK_LPAREN TOK_RETURNS method_return_param TOK_RPAREN
			    {
			    $$ = rul__ast_node (AST__E_RETURNS_DECL);
			    rul__ast_attach_child ($$, $3);
			    }
;

generic_method_when_decl:
			variable TOK_WHEN TOK_INSTANCE opt_domain_class
			    {
			    $$ = rul__ast_node (AST__E_METHOD_WHEN);
			    rul__ast_attach_children ($$, 2, $1, $4);
			    }
;

/*
 * METHOD-DECLARATION
 */

method_declaration:
  			TOK_METHOD allowable_func_name 
			    { /* MRA */ set_cur_construct (
						AST__E_METHOD, $2); }
			  method_accepts_clause
                          opt_method_returns_plus_actions
			    {
			    $$ = rul__ast_node (AST__E_METHOD);
			    rul__ast_attach_children ($$, 3, $2, $4, $5);
			    }
;

method_accepts_clause:
			TOK_LPAREN
                          TOK_ACCEPTS method_when_decl opt_method_param_decls
			TOK_RPAREN
			    {
			    $$ = rul__ast_node (AST__E_ACCEPTS_DECL);
			    rul__ast_attach_child ($$, $3);
			    rul__ast_append_sibling ($3, $4);
			    }
;

method_when_decl:
			variable TOK_WHEN TOK_INSTANCE TOK_OF name
			    {
			    $$ = rul__ast_node (AST__E_METHOD_WHEN);
			    rul__ast_attach_child ($$, $1);
			    rul__ast_append_sibling ($1, $5);
			    }
;

method_param_decl:
			variable opt_method_param_opts
			    {
			      $$ = rul__ast_node (AST__E_METHOD_PARAM);
			      rul__ast_attach_child ($$, $1);
			      rul__ast_append_sibling ($1, $2);
			    }
;

opt_method_param_decls:
			/* empty */
			    {  $$ = NULL; }
		|	opt_method_param_decls method_param_decl
			    {
			    $$ = rul__ast_append_opt_sibling ($1, $2);
			    }
;

opt_method_returns_plus_actions:
			TOK_LPAREN action TOK_RPAREN opt_method_actions
			    {
			    if ($2 != NULL) {
			        $$ = rul__ast_node (AST__E_METHOD_RHS);
			        rul__ast_attach_children ($$, 2, $2, $4);
			        }
			    }
		|	TOK_LPAREN TOK_RETURNS method_return_param TOK_RPAREN
			opt_actions
			    {
			    Ast_Node rhs;
			    $$ = rul__ast_node (AST__E_RETURNS_DECL);
			    rul__ast_attach_child ($$, $3);

			    rhs = rul__ast_node (AST__E_METHOD_RHS);
			    rul__ast_append_sibling ($$, rhs);
			    rul__ast_attach_child (rhs, $5);
			    }
;

method_return_param:
			variable opt_method_param_opts
			    {
			      $$ = rul__ast_node (AST__E_METHOD_PARAM);
			      rul__ast_attach_child ($$, $1);
			      rul__ast_append_sibling ($1, $2);
			    }
;

opt_method_param_opts:
			opt_compound_flag opt_domain_restriction
			    {
			      if ($1 == NULL)
				$$ = $2;
			      else
				$$ = rul__ast_append_sibling ($1, $2);
			    }
;


/*
 * EXTERNAL-ROUTINE
 */

external_routine_declaration:
			TOK_EXTERNAL_ROUTINE allowable_func_name
			    { /* MRA */ set_cur_construct (
						AST__E_EXT_RT_DECL, $2); }
			opt_ext_rt_clauses
			    {
			    $$ = rul__ast_node (AST__E_EXT_RT_DECL);
			    rul__ast_attach_child ($$, $2);
			    rul__ast_append_sibling ($2, $4);
			    }
;

opt_ext_rt_clauses:
			/* empty */
			    {  $$ = NULL; }
		|	TOK_LPAREN ext_rt_clause TOK_RPAREN opt_ext_rt_clauses 
			    {
			    $$ = rul__ast_append_sibling ($2, $4);
			    }
;

ext_rt_clause:
			alias_clause
		|	accepts_clause
		|	returns_clause
;

alias_clause:
			TOK_ALIAS_FOR name
			    {
			    $$ = rul__ast_node (AST__E_ALIAS_DECL);
			    rul__ast_attach_child ($$, $2);
			    }
;

accepts_clause:
			 TOK_ACCEPTS ext_rt_param_decl opt_ext_rt_param_decls
			    {
			    $$ = rul__ast_node (AST__E_ACCEPTS_DECL);
			    rul__ast_attach_child ($$, $2);
			    rul__ast_append_sibling ($2, $3);
			    }
;

returns_clause:
			TOK_RETURNS ext_rt_return_decl 
			    {
			    $$ = rul__ast_node (AST__E_RETURNS_DECL);
			    rul__ast_attach_child ($$, $2);
			    }
;

ext_rt_param_decl:
			opt_param_name
			opt_param_cardinality
			param_type
			opt_param_mech
			    {
			    $$ = rul__ast_node (AST__E_ACCEPTS_PARAM);
			    rul__ast_attach_child ($$, $3);
			    rul__ast_append_sibling ($3, $2);
			    rul__ast_append_sibling ($3, $4);
			    rul__ast_append_sibling ($3, $1);
			    }
;

opt_ext_rt_param_decls:
			/* empty */
			    {  $$ = NULL; }
		|	ext_rt_param_decl opt_ext_rt_param_decls
			    {
			    $$ = rul__ast_append_sibling ($1, $2);
			    }
;
			
ext_rt_return_decl:
			opt_param_name
			opt_param_cardinality
			param_type
			opt_return_mech
			    {
			    $$ = rul__ast_append_sibling ($3, $2);
			    rul__ast_append_sibling ($3, $4);
			    rul__ast_append_sibling ($3, $1);
			    }
;

opt_param_name:
			/* empty */
			    { $$ = NULL; }
		|	variable
;

opt_param_cardinality:
			 /* empty */
			    {  $$ = NULL; }
		|	TOK_LBRACKET param_array_length TOK_RBRACKET
			    {
                            $$ = $2;
			    }
;

param_type:
			TOK_LONG
			    {
			    $$ = rul__ast_ext_type_node (ext_type_long);
			    }
		|	TOK_SHORT
			    {
			    $$ = rul__ast_ext_type_node (ext_type_short);
			    }
		|	TOK_BYTE
			    {
			    $$ = rul__ast_ext_type_node (ext_type_byte);
			    }
		|	TOK_UNSIGNED_LONG
			    {
			    $$ = rul__ast_ext_type_node (ext_type_uns_long);
			    }
		|	TOK_UNSIGNED_SHORT
			    {
			    $$ = rul__ast_ext_type_node (ext_type_uns_short);
			    }
		|	TOK_UNSIGNED_BYTE
			    {
			    $$ = rul__ast_ext_type_node (ext_type_uns_byte);
			    }
		|	TOK_SINGLE_FLOAT
			    {
			    $$ = rul__ast_ext_type_node (ext_type_float);
			    }
		|	TOK_DOUBLE_FLOAT
			    {
			    $$ = rul__ast_ext_type_node (ext_type_double);
			    }
		|	TOK_ASCIZ
			    {
			    $$ = rul__ast_ext_type_node (ext_type_asciz);
			    }
		|	TOK_ASCID
			    {
			    $$ = rul__ast_ext_type_node (ext_type_ascid);
			    }
		|	TOK_POINTER
			    {
			    $$ = rul__ast_ext_type_node (ext_type_void_ptr);
			    }
		|	TOK_ATOM
			    {
			    $$ = rul__ast_ext_type_node (ext_type_atom);
			    }
;

opt_param_mech:
			/* empty */
			    { $$ = NULL; }
		|	TOK_BY TOK_REFERENCE TOK_READ_ONLY
			    {
			    $$ = rul__ast_ext_mech_node (ext_mech_ref_ro);
			    }
		|	TOK_BY TOK_REFERENCE TOK_READ_WRITE
			    {
			    $$ = rul__ast_ext_mech_node (ext_mech_ref_rw);
			    }
		|	TOK_BY TOK_VALUE
			    {
			    $$ = rul__ast_ext_mech_node (ext_mech_value);
			    }
;

opt_return_mech:
			/* empty */
			    { $$ = NULL; }
		|	TOK_BY TOK_REFERENCE
			    {
			    $$ = rul__ast_ext_mech_node (ext_mech_ref_ro);
			    }
		|	TOK_BY TOK_VALUE
			    {
			    $$ = rul__ast_ext_mech_node (ext_mech_value);
			    }
;

param_array_length:
			/* empty */
			    {
			    $$ = rul__ast_cardinality_node (-1);
			    }
		|	TOK_INTEGER_CONST
			    {
			    $$ = rul__ast_cardinality_node (yylval.tok.ival);
			    }
                |       variable
			    {
			    $$ = rul__ast_node (AST__E_EXT_LEN);
			    rul__ast_attach_value ($$, AST__E_TYPE_MOL,
						   rul__ast_get_value ($1));
			    rul__mol_incr_uses (rul__ast_get_value ($1));
			    rul__ast_free ($1);
			    }
;




/*
 * RULE
 */

rule:			rule_or_p name 
			    { /* MRA */ set_cur_construct (AST__E_RULE, $2);}
			  ce_form opt_ce_list TOK_ARROW opt_actions
			    {
			    Ast_Node lhs, ce1, rhs;

			    $$ = rul__ast_node (AST__E_RULE);
			    lhs = rul__ast_node (AST__E_RULE_LHS);
			    ce1 = rul__ast_node (AST__E_POS_CE);
			    rhs = rul__ast_node (AST__E_RULE_RHS);
			    rul__ast_attach_children ($$, 3, $2, lhs, rhs);
			    rul__ast_attach_children (lhs, 2, ce1, $5);
			    rul__ast_attach_child (ce1, $4);
			    rul__ast_attach_child (rhs, $7);
			    }
;

rule_or_p:		TOK_RULE
			    { $$ = NULL; }
		|	TOK_P
			    { $$ = NULL; }
;

/*
 * LHS
 */

opt_ce_list:		/* empty */
			    {  $$ = NULL;  }
		|	opt_ce_list ce
			    {
			    $$ = rul__ast_append_opt_sibling ($1, $2);
			    }
;

ce:			pos_ce
		|	neg_ce
;

pos_ce:			ce_form
			    {
			    $$ = rul__ast_node (AST__E_POS_CE);
			    rul__ast_attach_child ($$, $1);
			    }
/* until we fully support them...
		|	ce_disjunction
			    {
			    $$ = rul__ast_node (AST__E_CE_DISJ);
			    rul__ast_attach_child ($$, $1);
			    }
*/
;

neg_ce:			TOK_MINUS ce_form
			    {
			    $$ = rul__ast_node (AST__E_NEG_CE);
                            rul__ast_attach_child ($$, $2);
			    }
;

/*
ce_disjunction:		TOK_START_DISJUNCTION ce_disj_list TOK_END_DISJUNCTION
			    {  $$ = $2;  }
;

ce_disj_list:		ce_form
			    {
			    $$ = rul__ast_node (AST__E_POS_CE);
			    rul__ast_attach_child ($$, $1);
			    }
		|	ce_disj_list ce_form
			    {
			    Ast_Node last;
			    last = rul__ast_node (AST__E_POS_CE);
			    rul__ast_attach_child (last, $2);
			    $$ = rul__ast_append_opt_sibling ($1, last);
			    }
;
*/

ce_form:		TOK_LPAREN name opt_lhs_terms TOK_RPAREN
			    {
			    $$ = rul__ast_node (AST__E_CE);
			    rul__ast_attach_children ($$, 2, $2, $3);
			    }
		|	TOK_LPAREN name opt_lhs_terms TOK_ARROW
			    {
			    $$ = rul__ast_node (AST__E_CE);
			    rul__ast_attach_children ($$, 2, $2, $3);

			    rul__msg_cmp_print_w_line (
				CMP_UNTERMCE,
				SL_line_number);

			    rul__scan_unget_tokens (1);
			    }
		|	error
			    {
			    yyclearin;
			    $$ = NULL;
			    }
;

opt_lhs_terms:		/* empty */
			    { $$ = NULL; }
		|	opt_lhs_terms lhs_term
			    { $$ = rul__ast_append_opt_sibling ($1, $2); }
;

lhs_term:		TOK_HAT name opt_ca_access lhs_value
			    {
			    Ast_Node attr;

			    attr = rul__ast_node (AST__E_ATTR_EXPR);
			    rul__ast_attach_children (attr, 2, $2, $3);
			    $$ = rul__ast_node (AST__E_ATTR_TEST);
			    rul__ast_attach_children ($$, 2, attr, $4);
			    }
		|	error
			    { yyclearin;  $$ = NULL; }
;

opt_ca_access:		/* empty */
			    { $$ = NULL; }
		|	TOK_LBRACKET ca_index TOK_RBRACKET
			    {
			    $$ = rul__ast_node (AST__E_ATTR_INDEX);
			    rul__ast_attach_child ($$, $2);
			    }
;

ca_index:		arith_expr
		|	TOK_DOLLAR_LAST
			    {
			    $$ = rul__ast_node (AST__E_DOLLAR_LAST);
			    }
;

index_num:		integer_const
		|	variable
		|	TOK_LPAREN built_ins TOK_RPAREN
			    {  $$ = $2;  }
		|	TOK_LPAREN function_call TOK_RPAREN
			    {  $$ = $2;  }
		|	TOK_LPAREN arith_expr TOK_RPAREN
			    { $$ = $2; }
		|	TOK_DOLLAR_LAST
			    {
			    $$ = rul__ast_node (AST__E_DOLLAR_LAST);
			    }
;

lhs_value:		restriction
		|	TOK_LBRACE restriction_list TOK_RBRACE
			    {  $$ = $2;  }
;

restriction_list:	restriction_list restriction
			    {
			    $$ = rul__ast_append_sibling ($1, $2);
			    }
		|	restriction
;

restriction:		TOK_START_DISJUNCTION value_list TOK_END_DISJUNCTION
			    {
			    Ast_Node list_node;

			    $$ = rul__ast_pred_type_node (PRED__E_EQ);

			    list_node = rul__ast_node (AST__E_VALUE_DISJ);
			    rul__ast_attach_child ($$, list_node);
			    rul__ast_attach_child (list_node, $2);
			    }
		|	predicate value
			    {
			    Ast_Node opt_pred;

			    opt_pred = rul__ast_get_child ($1);
			    if (opt_pred)
			        rul__ast_append_sibling (opt_pred, $2);
			    else
			        rul__ast_attach_child ($1, $2);
			    }
		|	value
			    {
			    $$ = rul__ast_pred_type_node (PRED__E_EQ);
			    rul__ast_attach_child ($$, $1);
			    }
;

arith_expr:		arith_value
		|	arith_expr TOK_PLUS arith_expr
			    {
			    $$ = rul__ast_node (AST__E_OPR_PLUS);
			    rul__ast_attach_children ($$, 2, $1, $3);
			    }
		|	arith_expr TOK_MINUS arith_expr
			    {
			    $$ = rul__ast_node (AST__E_OPR_MINUS);
			    rul__ast_attach_children ($$, 2, $1, $3);
			    }
		|	arith_expr TOK_TIMES arith_expr
			    {
			    $$ = rul__ast_node (AST__E_OPR_TIMES);
			    rul__ast_attach_children ($$, 2, $1, $3);
			    }
		|	arith_expr TOK_DIVIDE arith_expr
			    {
			    $$ = rul__ast_node (AST__E_OPR_DIVIDE);
			    rul__ast_attach_children ($$, 2, $1, $3);
			    }
		|	arith_expr TOK_MODULUS arith_expr
			    {
			    $$ = rul__ast_node (AST__E_OPR_MODULUS);
			    rul__ast_attach_children ($$, 2, $1, $3);
			    }
		|	TOK_MINUS arith_value %prec UMINUS
			    {
			    $$ = rul__ast_node (AST__E_OPR_UMINUS);
			    rul__ast_attach_child ($$, $2);
			    }
;

arith_value:		integer_const
		|	float_const
		|	variable
		|	TOK_LPAREN built_ins TOK_RPAREN
			    {  $$ = $2;  }
		|	TOK_LPAREN function_call TOK_RPAREN
			    {  $$ = $2;  }
		|	TOK_LPAREN arith_expr TOK_RPAREN
			    {
			    $$ = rul__ast_node (AST__E_ARITH_EXPR);
			    rul__ast_attach_child ($$, $2);
			    }
		|	error
			    {
			    yyclearin;
			    $$ = NULL;
			    }
;


on_off:			TOK_OFF
			    {
			    $$ = rul__ast_str_node (yylval.tok.sval);
			    }
		|	TOK_ON
			    {
			    $$ = rul__ast_str_node (yylval.tok.sval);
			    }
;
 	
trace_options:		TOK_TIMES
			    { $$ = rul__ast_str_node (yylval.tok.sval); }
		|	trace_names
;

trace_names:		trace_name
		|	trace_names trace_name
			    {
                            $$ = rul__ast_append_opt_sibling ($1, $2);
			    }
;

trace_name:		TOK_ENTRY_BLOCK
			    { $$ = rul__ast_str_node (yylval.tok.sval); }
		|	TOK_EB
			    { $$ = rul__ast_str_node (yylval.tok.sval); }
		|	TOK_RULE_GROUP
			    { $$ = rul__ast_str_node (yylval.tok.sval); }
		|	TOK_RG
			    { $$ = rul__ast_str_node (yylval.tok.sval); }
		|	TOK_RULE
			    { $$ = rul__ast_str_node (yylval.tok.sval); }
		|	TOK_WM
			    { $$ = rul__ast_str_node (yylval.tok.sval); }
		|	TOK_CS
			    { $$ = rul__ast_str_node (yylval.tok.sval); }
;

/*
 * Variables, values, names, constants.
 */

variables:		variables variable
			    {
			    $$ = rul__ast_append_opt_sibling ($1, $2);
			    }
		|	variable
;

function_call:		allowable_func_name opt_values
			    {
			    $$ = rul__ast_node(AST__E_EXT_RT_CALL);
			    rul__ast_attach_child ($$, $1);
			    rul__ast_append_sibling ($1, $2);
			    }
;

value_list:		value
		|	value_list value
			    {
			    $$ = rul__ast_append_sibling ($1, $2);
			    }
;

opt_value:		/* empty */
			    {  $$ = NULL;  }
		|	value
			    {  $$ = $1;  }
;

opt_values:		/* empty */
			    {  $$ = NULL;  }
		|	opt_values value
			    { $$ = rul__ast_append_opt_sibling ($1, $2); }
;

/* Should grammar disallow functions like MAKE on LHS, or semantics? */
value:			value_non_sql
  		|	TOK_LPAREN sql_fetch_as_object_action TOK_RPAREN
			    {  $$ = $2;  }
;

value_non_sql:		constant
		|	variable
		|	TOK_LPAREN built_ins TOK_RPAREN
			    {  $$ = $2;  }
		|	TOK_LPAREN function_call TOK_RPAREN
			    {  $$ = $2;  }
		|	TOK_LPAREN arith_expr TOK_RPAREN
			    {
			    $$ = rul__ast_node (AST__E_ARITH_EXPR);
			    rul__ast_attach_child ($$, $2);
			    }
		|	TOK_LPAREN copy_action TOK_RPAREN
			    {  $$ = $2;  }
		|	TOK_LPAREN make_action TOK_RPAREN
			    {  $$ = $2;  }
		|	TOK_LPAREN modify_action TOK_RPAREN
			    {  $$ = $2;  }
		|	TOK_LPAREN specialize_action TOK_RPAREN
			    {  $$ = $2;  }
		|	TOK_LPAREN call_inherited TOK_RPAREN
			    {  $$ = $2;  }
;

constant:		integer_const
		|	float_const
		|	opaque_const
		|	instance_const
		|	name
;

names:			names name
			    {
			    $$ = rul__ast_append_opt_sibling ($1, $2);
			    }
		|	name
;

opt_name:		/* empty */
			    {  $$ = NULL;  }
		|	name
;

name:			allowable_func_name
		|	built_in_keyword
			    {
			    $$ = rul__ast_str_node (yylval.tok.sval);
			    }
		|	construct_keyword
			    {
			    $$ = rul__ast_str_node (yylval.tok.sval);
			    }
		|	action_token
			    {
			    $$ = rul__ast_str_node (yylval.tok.sval);
			    }
		|	sql_keyword
			    {
			    $$ = rul__ast_str_node (yylval.tok.sval);
			    }
;

db_spec_name:		allowable_func_name
		|	built_in_keyword
			    {
			    $$ = rul__ast_str_node (yylval.tok.sval);
			    }
		|	construct_keyword
			    {
			    $$ = rul__ast_str_node (yylval.tok.sval);
			    }
		|	action_token
			    {
			    $$ = rul__ast_str_node (yylval.tok.sval);
			    }
;

/* Need to accept names other than actions to disambiguate function call
 * from other actions.
 *
 * Not allowing construct keywords as external routine names simplifies
 * error recovery.
 */
allowable_func_name:	symbol_const
			    /* Complain about names starting with "$" */
		|	quoted_symbol
		|	allowable_func_keyword
			    {
			    $$ = rul__ast_str_node (yylval.tok.sval);
			    }
;

/* The construct type keywords of the language */
construct_keyword:	TOK_DECLARATION_BLOCK
			{  $$ = NULL;  }
		|	TOK_END_BLOCK
			{  $$ = NULL;  }
		|	TOK_END_GROUP
			{  $$ = NULL;  }
		|	TOK_ENTRY_BLOCK
			{  $$ = NULL;  }
		|	TOK_EXTERNAL_ROUTINE
			{  $$ = NULL;  }
		|	TOK_GENERIC_METHOD
			{  $$ = NULL;  }
		|	TOK_METHOD
			{  $$ = NULL;  }
		|	TOK_OBJECT_CLASS
			{  $$ = NULL;  }
		|	TOK_ON_EMPTY
			{  $$ = NULL;  }
		|	TOK_ON_ENTRY
			{  $$ = NULL;  }
		|	TOK_STARTUP
			{  $$ = NULL;  }
		|	TOK_ON_EVERY
			{  $$ = NULL;  }
		|	TOK_ON_EXIT
			{  $$ = NULL;  }
		|	TOK_P
			{  $$ = NULL;  }
		|	TOK_RULE
			{  $$ = NULL;  }
		|	TOK_RULE_BLOCK
			{  $$ = NULL;  }
		|	TOK_RULE_GROUP
			{  $$ = NULL;  }
;

/* The built-in function names */
built_in_keyword:	TOK_ACCEPT_ATOM
			{  $$ = NULL;  }
		|	TOK_ACCEPTLINE_COMPOUND
			{  $$ = NULL;  }
		|	TOK_ACCEPTS
			{  $$ = NULL;  }
		|	TOK_COMPOUND
			{  $$ = NULL;  }
		|	TOK_CONCAT
			{  $$ = NULL;  }
		|	TOK_CRLF
			{  $$ = NULL;  }
		|	TOK_EVERY
			{  $$ = NULL;  }
		|	TOK_FLOAT
			{  $$ = NULL;  }
		|	TOK_GENATOM
			{  $$ = NULL;  }
		|	TOK_GENINT
			{  $$ = NULL;  }
  		|	TOK_GET
			{  $$ = NULL;  }
		|	TOK_INTEGER
			{  $$ = NULL;  }
		|	TOK_IS_OPEN
			{  $$ = NULL;  }
  		|	TOK_LENGTH
			{  $$ = NULL;  }
		|	TOK_MAX
			{  $$ = NULL;  }
		|	TOK_MIN
			{  $$ = NULL;  }
		|	TOK_NTH
			{  $$ = NULL;  }
		|	TOK_POSITION
			{  $$ = NULL;  }
		|	TOK_RETURNS
			{  $$ = NULL;  }
		|	TOK_RJUST
			{  $$ = NULL;  }
		|	TOK_SUBCOMPOUND
			{  $$ = NULL;  }
		|	TOK_SUBSYMBOL
			{  $$ = NULL;  }
		|	TOK_SYMBOL
			{  $$ = NULL;  }
		|	TOK_SYMBOL_LENGTH
			{  $$ = NULL;  }
		|	TOK_TABTO
			{  $$ = NULL;  }
		|	TOK_AND
			{  $$ = NULL;  }
		|	TOK_NOT
			{  $$ = NULL;  }
		|	TOK_OR
			{  $$ = NULL;  }
;


/*
**  Most of the  keywords of the language are usable as function names, except:
**	- Construct type keywords  (e.g. RULE)
** 	- Built-in action names (e.g. MAKE)
**	- Built-in function names (e.g INTEGER)
**	RETURNS and ACCEPTS, AND OR and NOT
*/

allowable_func_keyword:	TOK_ACCEPT
			{  $$ = NULL;  }
		|	TOK_ACTIVATES
			{  $$ = NULL;  }
		|	TOK_ANY
			{  $$ = NULL;  }
		|	TOK_APPEND
			{  $$ = NULL;  }
		|	TOK_ASCID
			{  $$ = NULL;  }
		|	TOK_ASCIZ
			{  $$ = NULL;  }
		|	TOK_ATOM
			{  $$ = NULL;  }
		|	TOK_BY
			{  $$ = NULL;  }
		|	TOK_BYTE
			{  $$ = NULL;  }
		|	TOK_CATCH
			{  $$ = NULL;  }
		|	TOK_CS
			{  $$ = NULL;  }
		|	TOK_DO
			{  $$ = NULL;  }
		|	TOK_DOLLAR_LAST
			{  $$ = NULL;  }
		|	TOK_DOUBLE_FLOAT
			{  $$ = NULL;  }
		|	TOK_EB
			{  $$ = NULL;  }
		|	TOK_ELSE
			{  $$ = NULL;  }
		|	TOK_FAILURE
			{  $$ = NULL;  }
		|	TOK_FILL
			{  $$ = NULL;  }
		|	TOK_IN
			{  $$ = NULL;  }
		|	TOK_INHERITS_FROM
			{  $$ = NULL;  }
		|	TOK_INSTANCE
			{  $$ = NULL;  }
		|	TOK_LEX
			{  $$ = NULL;  }
		|	TOK_LONG
			{  $$ = NULL;  }
		|	TOK_MEA
			{  $$ = NULL;  }
		|	TOK_OF
			{  $$ = NULL;  }
		|	TOK_OUT
			{  $$ = NULL;  }
		|	TOK_OPAQUE
			{  $$ = NULL;  }
		|	TOK_POINTER
			{  $$ = NULL;  }
		|	TOK_READ_ONLY
			{  $$ = NULL;  }
		|	TOK_READ_WRITE
			{  $$ = NULL;  }
		|	TOK_REFERENCE
			{  $$ = NULL;  }
		|	TOK_RG
			{  $$ = NULL;  }
		|	TOK_SHORT
			{  $$ = NULL;  }
		|	TOK_SINGLE_FLOAT
			{  $$ = NULL;  }
		|	TOK_SCALAR
			{  $$ = NULL;  }
		|	TOK_STRATEGY
			{  $$ = NULL;  }
		|	TOK_SUCCESS
			{  $$ = NULL;  }
		|	TOK_THEN
			{  $$ = NULL;  }
		|	TOK_UNSIGNED_BYTE
			{  $$ = NULL;  }
		|	TOK_UNSIGNED_LONG
			{  $$ = NULL;  }
		|	TOK_UNSIGNED_SHORT
			{  $$ = NULL;  }
		|	TOK_USES
			{  $$ = NULL;  }
		|	TOK_VALUE
			{  $$ = NULL;  }
		|	TOK_WM
			{  $$ = NULL;  }
;

action_token:		TOK_ADDSTATE
			{  $$ = NULL;  }
		|	TOK_AFTER
			{  $$ = NULL;  }
		|	TOK_BIND
			{  $$ = NULL;  }
		|	TOK_CALL_INHERITED
			{  $$ = NULL;  }
		|	TOK_CLOSEFILE
			{  $$ = NULL;  }
		|	TOK_COPY
			{  $$ = NULL;  }
		|	TOK_DEBUG
			{  $$ = NULL;  }
		|	TOK_DEFAULT
			{  $$ = NULL;  }
		|	TOK_FOR_EACH
			{  $$ = NULL;  }
		|	TOK_IF
			{  $$ = NULL;  }
		|	TOK_MAKE
			{  $$ = NULL;  }
		|	TOK_MODIFY
			{  $$ = NULL;  }
		|	TOK_OPENFILE
			{  $$ = NULL;  }
		|	TOK_QUIT
			{  $$ = NULL;  }
		|	TOK_REMOVE
			{  $$ = NULL;  }
		|	TOK_REMOVE_EVERY
			{  $$ = NULL;  }
		|	TOK_RESTORESTATE
			{  $$ = NULL;  }
		|	TOK_RETURN
			{  $$ = NULL;  }
		|	TOK_SAVESTATE
			{  $$ = NULL;  }
		|	TOK_SPECIALIZE
			{  $$ = NULL;  }
		|	TOK_TRACE
			{  $$ = NULL;  }
		|	TOK_WHILE
			{  $$ = NULL;  }
		|	TOK_WRITE
			{  $$ = NULL;  }
		|	TOK_SQL_ATTACH
			{  $$ = NULL;  }
		|	TOK_SQL_COMMIT
			{  $$ = NULL;  }
		|	TOK_SQL_DELETE
			{  $$ = NULL;  }
		|	TOK_SQL_DETACH
			{  $$ = NULL;  }
		|	TOK_SQL_FETCH_EACH
			{  $$ = NULL;  }
		|	TOK_SQL_FETCH_AS_OBJECT
			{  $$ = NULL;  }
		|	TOK_SQL_INSERT
			{  $$ = NULL;  }
		|	TOK_SQL_INSERT_FROM_OBJECT
			{  $$ = NULL;  }
		|	TOK_SQL_ROLLBACK
			{  $$ = NULL;  }
		|	TOK_SQL_START
			{  $$ = NULL;  }
		|	TOK_SQL_UPDATE
			{  $$ = NULL;  }
		|	TOK_SQL_UPDATE_FROM_OBJECT
			{  $$ = NULL;  }
;

sql_keyword:		TOK_FILENAME
			{  $$ = NULL; }
		|	TOK_PATHNAME
			{  $$ = NULL; }
;

opt_predicate:		/* empty */
			{ $$ = rul__ast_pred_type_node (PRED__E_EQ); }
		|	scalar_predicate
;

predicate:		scalar_predicate
		|	content_predicate
;

scalar_predicate:	TOK_SAME_TYPE
		    { $$ = rul__ast_pred_type_node (PRED__E_SAME_TYPE); }
		|	TOK_DIFF_TYPE
		    { $$ = rul__ast_pred_type_node (PRED__E_DIFF_TYPE); }
		|	TOK_EQUAL
		    { $$ = rul__ast_pred_type_node (PRED__E_EQUAL); }
		|	TOK_EQUAL_EQUAL
		    { $$ = rul__ast_pred_type_node (PRED__E_EQ); }
		|	TOK_APPROX_EQUAL
		    { $$ = rul__ast_pred_type_node (PRED__E_APPROX_EQUAL); }
		|	TOK_NOT_APPROX_EQUAL
		    { $$ = rul__ast_pred_type_node (PRED__E_NOT_APPROX_EQUAL);}
		|	TOK_NOT_EQ
		    { $$ = rul__ast_pred_type_node (PRED__E_NOT_EQ); }
		|	TOK_NOT_EQUAL
		    { $$ = rul__ast_pred_type_node (PRED__E_NOT_EQUAL); }
		|	TOK_GREATER
		    { $$ = rul__ast_pred_type_node (PRED__E_GREATER); }
		|	TOK_GREATER_EQUAL
		    { $$ = rul__ast_pred_type_node (PRED__E_GREATER_EQUAL); }
		|	TOK_LESS
		    { $$ = rul__ast_pred_type_node (PRED__E_LESS); }
		|	TOK_LESS_EQUAL
		    { $$ = rul__ast_pred_type_node (PRED__E_LESS_EQUAL); }
;

content_predicate:	TOK_LENGTH_EQUAL
	    { $$ = rul__ast_pred_type_node (PRED__E_LENGTH_EQUAL); }
	|	TOK_LENGTH_NOT_EQUAL
	    { $$ = rul__ast_pred_type_node (PRED__E_LENGTH_NOT_EQUAL); }
	|	TOK_LENGTH_GREATER
	    { $$ = rul__ast_pred_type_node (PRED__E_LENGTH_GREATER); }
	|	TOK_LENGTH_GREATER_EQUAL
	    { $$ = rul__ast_pred_type_node (PRED__E_LENGTH_GREATER_EQUAL); }
	|	TOK_LENGTH_LESS
	    { $$ = rul__ast_pred_type_node (PRED__E_LENGTH_LESS); }
	|	TOK_LENGTH_LESS_EQUAL
	    { $$ = rul__ast_pred_type_node (PRED__E_LENGTH_LESS_EQUAL); }
	|	TOK_CONTAINS
	    { $$ = rul__ast_pred_type_node (PRED__E_CONTAINS); }
	|	TOK_DOES_NOT_CONTAIN
	    { $$ = rul__ast_pred_type_node (PRED__E_DOES_NOT_CONTAIN); }
	|	TOK_CONTAINS scalar_predicate
	    { $$ = rul__ast_pred_type_node (PRED__E_CONTAINS);
	      rul__ast_attach_child ($$, $2); }
	|	TOK_DOES_NOT_CONTAIN scalar_predicate
	    { $$ = rul__ast_pred_type_node (PRED__E_DOES_NOT_CONTAIN);
	      rul__ast_attach_child ($$, $2); }
	|	scalar_predicate TOK_CONTAINS
            { $$ = $1;
	      rul__ast_attach_child ($$,
			     rul__ast_pred_type_node (PRED__E_CONTAINS)); }
	|	scalar_predicate TOK_DOES_NOT_CONTAIN
            { $$ = $1;
	      rul__ast_attach_child ($$,
		     rul__ast_pred_type_node (PRED__E_DOES_NOT_CONTAIN)); }
;

/*
 * All non-magic "constant tokens" of the language are in one of
 * predicate, action_token, construct_keyword, non_action_keyword, 
 * or are in the following list:
 *
 *			TOK_PLUS
 *		|	TOK_MINUS
 *		|	TOK_TIMES
 *		|	TOK_DIVIDE
 *		|	TOK_MODULUS
 *		|	TOK_ARROW
 *		|	TOK_START_DISJUNCTION
 *		|	TOK_END_DISJUNCTION
 */


/*
 * Low-level nonterminals which convert tokens to Ast_Nodes.  The tokens
 * with varying values should be used in only these productions.
 */
compound_const:		TOK_LPAREN TOK_COMPOUND opt_comp_element_const
			 TOK_RPAREN
			    {
			    $$ = rul__ast_mol_node (AST__E_CONSTANT,
						    comp_const_val);
			    }
;

opt_comp_element_const:	/* empty */
			    {
			    comp_const_index = 0;
			    comp_const_val = rul__mol_make_comp(0);
			    $$ = NULL;
			    }
		|	opt_comp_element_const constant
			    {
			    comp_const_val =
				rul__mol_set_comp_nth(comp_const_val,
						      ++comp_const_index,
						      rul__ast_get_value($2),
						      NULL);
			    rul__ast_free($2);
			    $$ = NULL;
			    }
;

integer_const:		TOK_INTEGER_CONST
			    {
			    $$ = rul__ast_mol_node (
				    AST__E_CONSTANT,
				    rul__mol_make_int_atom (yylval.tok.ival));
			    }
;

float_const:		TOK_FLOAT_CONST
			    {
			    $$ = rul__ast_mol_node (
				    AST__E_CONSTANT,
				    rul__mol_make_dbl_atom (yylval.tok.fval));
			    }
;

variable:		TOK_VARIABLE
			    {
			    $$ = rul__ast_mol_node (
				    AST__E_VARIABLE,
				    rul__mol_make_symbol (
					upcase_in_place (yylval.tok.sval)));
			    }
;

quoted_var:		TOK_QUOTED_VAR
			    {
			    $$ = rul__ast_mol_node (
				    AST__E_QUOTED_VAR,
				    rul__mol_make_symbol (yylval.tok.sval));
			    }
;

symbol_const:		TOK_SYMBOL_CONST
			    {
			    $$ = rul__ast_str_node (yylval.tok.sval);
			    }
;

quoted_symbol:		TOK_QUOTED_SYMBOL
			    {
			    $$ = rul__ast_mol_node (
				    AST__E_CONSTANT,
				    rul__mol_make_symbol (yylval.tok.sval));
			    }
;

opaque_const:		TOK_OPAQUE_CONST
			    {
			    $$ = rul__ast_mol_node (
				    AST__E_CONSTANT, rul__mol_opaque_null());
/* SB: Don't know reason for this !
**			    if (yylval.tok.ival != 0)
**				{
**				rul__msg_cmp_print_w_line (
**					CMP_INVOPAQUE, SL_line_number);
**			    	}
*/
			    }
;

instance_const:		TOK_INSTANCE_CONST
			    {
			    $$ = rul__ast_mol_node (
				    AST__E_CONSTANT,
				    rul__mol_instance_id_zero ());
			    if (yylval.tok.ival != 0)
				{
				rul__msg_cmp_print_w_line (
					CMP_INVINSTID, SL_line_number);
			    	}
			    }
;

%%


/****************************************************************************/
/* C CODE SECTION							    */


static Ast_Node compound_is_a_constant (Ast_Node ast)
{
  Ast_Node node;
  long     i;
  Molecule tmp_comp;

  rul__mol_start_tmp_comp (10);

  for (node = ast, i = 0;
       node != NULL;
       node = rul__ast_get_sibling (node), i++) {

    if (rul__ast_get_type (node) != AST__E_CONSTANT) {
      tmp_comp = rul__mol_end_tmp_comp ();
      rul__mol_decr_uses (tmp_comp);
      return NULL;
    }
    rul__mol_set_tmp_comp_nth (i, rul__ast_get_value (node));
  }

  node = rul__ast_mol_node (AST__E_CONSTANT, rul__mol_end_tmp_comp ());
  return node;
}



static Boolean		SB_in_construct = FALSE;
static Ast_Node_Type 	SA_construct_type = AST__E_INVALID;
static char	 	SC_construct_name[RUL_C_MAX_SYMBOL_SIZE+1];


/*******************
**                **
**  IN_CONSTRUCT  **
**                **
*******************/

static Boolean in_construct (void)
{
	return (SB_in_construct);
}


/***********************
**                    **
**  SET_IN_CONSTRUCT  **
**                    **
***********************/

static void set_in_construct (void)
{
	SB_in_construct = TRUE;
	SA_construct_type = AST__E_INVALID;
	SC_construct_name[0] = '\0';
}


/**************************************
**                                   **
**  RUL__PARSER_DONE_WITH_CONSTRUCT  **
**                                   **
**************************************/

void rul__parser_done_with_construct (void)
{
	SB_in_construct = FALSE;
	SA_construct_type = AST__E_INVALID;
	SC_construct_name[0] = '\0';
}


/*************************
**                      **
**  GET_CONSTRUCT_TYPE  **
**                      **
*************************/

static Ast_Node_Type get_construct_type (void)
{
	return (SA_construct_type);
}



/*****************************
**                          **
**  CONSTRUCT_TYPE_TO_NAME  **
**                          **
*****************************/

static char *construct_type_to_name (Ast_Node_Type con_type)
{
	switch (con_type) {

	    case AST__E_ENTRY_BLOCK :
			return ("ENTRY-BLOCK");
			break;
	    case AST__E_DECL_BLOCK :
			return ("DECLARATION-BLOCK");
			break;
	    case AST__E_RULE_BLOCK :
			return ("RULE-BLOCK");
			break;
	    case AST__E_END_BLOCK :
			return ("END-BLOCK");
			break;

	    case AST__E_OBJ_CLASS :
			return ("OBJECT-CLASS");
			break;
	    case AST__E_EXT_RT_DECL :
			return ("EXTERNAL-ROUTINE");
			break;

	    case AST__E_CATCH :		
			return ("CATCH");
			break;
	    case AST__E_RULE :
			return ("RULE");
			break;

	    case AST__E_METHOD :
			return ("METHOD");
			break;
	    case AST__E_GENERIC_METHOD :
			return ("GENERIC-METHOD");
			break;

	    case AST__E_RULE_GROUP :
			return ("RULE-GROUP");
			break;
	    case AST__E_END_GROUP :
			return ("END-GROUP statement");
			break;

	    case AST__E_ON_ENTRY :
			return ("ON-ENTRY statement");
			break;
	    case AST__E_ON_EMPTY :
			return ("ON-EMPTY statement");
			break;
	    case AST__E_ON_EXIT :
			return ("ON-EXIT statement");
			break;
	    case AST__E_ON_EVERY :
			return ("ON-EVERY statement");
			break;

	    default:
			return (NULL);
			break;
	}
}



/************************
**                     **
**  SET_CUR_CONSTRUCT  **
**                     **
************************/

static void set_cur_construct (Ast_Node_Type typ, Ast_Node ast)
{
	Mol_Symbol sym;

	SA_construct_type = typ;

	if (ast != NULL  &&  rul__ast_get_value_type (ast) == AST__E_TYPE_MOL) {
	    sym = (Mol_Symbol) rul__ast_get_value (ast);
	    rul__mol_use_printform (sym, SC_construct_name, 
				RUL_C_MAX_SYMBOL_SIZE+1);
	}
}


/*************************
**                      **
**  GET_CONSTRUCT_NAME  **
**                      **
*************************/

static char *get_construct_name (void)
{
	return (SC_construct_name);
}



/**********************
**                   **
**  UPCASE_IN_PLACE  **
**                   **
**********************/

static char *upcase_in_place (char *str)
{
	long i, len;

	len = strlen(str);
	for (i=0; i<len; i++) str[i] = toupper(str[i]);
	return (str);
}

static Boolean SB_syntax_error = FALSE;



/******************
**               **
**  CLEAR_ERROR  **
**               **
******************/

static void clear_error (void)
{
	SB_syntax_error = FALSE;
}


/****************
**             **
**  SET_ERROR  **
**             **
****************/

static void set_error (void)
{
	SB_syntax_error = TRUE;
}


/***************
**            **
**  IN_ERROR  **
**            **
***************/

static Boolean in_error (void)
{
	return (SB_syntax_error);
}




/**************
**           **
**  YYERROR  **
**           **
**************/

static void yyerror (char *msg)
	/*
	**	Construct and post an appropriate error message
	*/
{
	char *str, buffer[100], err_msg[100];
	char near_msg[RUL_C_MAX_SYMBOL_SIZE+25];
	Token_Type typ;
	Token_Value val;

	if (SB_syntax_error) {
            /*
            **   If we are in the middle of a statement which
            **   we already know contains an error, clear the
            **   input buffer and continue.
	    */
	    return;
	}
	set_error ();

	typ = rul__scan_get_last_token (&val);

	switch (typ /* current token type */) {

	    case TOK_FLOAT_CONST :
			sprintf (buffer, "%f", val.fval);
			str = buffer;
			break;

	    case TOK_INTEGER_CONST :
			sprintf (buffer, "%ld", val.ival);
			str = buffer;
			break;

	    case TOK_OPAQUE_CONST :
			sprintf (buffer, "%%x%p", val.ival);
			str = buffer;
			break;

	    case TOK_INSTANCE_CONST :
			sprintf (buffer, "#%ld", val.ival);
			str = buffer;
			break;

	    case 0 :	/* End-Of-File */
			if (in_construct()) {
			    rul__msg_cmp_print_w_line (
				    CMP_UNEXPEOF, SL_line_number, "");
			}
			return;

	    default :
			str = val.sval;
	}

	if ((strcmp (msg, "Syntax error") == 0) ||
	    (strcmp (msg, "parse error") == 0)  ||
	    (strcmp (msg, "syntax error") == 0)) {
	    if (in_construct()) {
		strcpy (err_msg, "");
	    } else {
		strcpy (err_msg, "was expecting '('");
	    }
	} else {
	    sprintf (err_msg, "%s", msg);
	}

	if (str[0] != '\0') {
	    sprintf (near_msg, " at '%s'", str);
	} else {
	    near_msg[0] = '\0';
	}

	rul__msg_cmp_print_w_line (
		CMP_SYNTAXERR, SL_line_number, err_msg, near_msg, "");
}




/************************************
**                                 **
**  RUL__PARSER_GET_CONSTRUCT_STR  **
**                                 **
************************************/

char *rul__parser_get_construct_str (void)
{
	static char in_msg[RUL_C_MAX_SYMBOL_SIZE*2+25];
	char *con_name, *con_type_str;
	Ast_Node_Type con_type;

	con_type = get_construct_type ();
	con_name = get_construct_name ();
	in_msg[0] = '\0';

	/*  Generate the current construct message fragment */
	if (con_type != AST__E_INVALID) {
	    con_type_str = construct_type_to_name (con_type);
	    if (con_type_str != NULL) {
		if (con_name[0] != '\0') {
		    /*  a named construct  */
		    sprintf (in_msg, " in %s %s", con_type_str, con_name);
		} else {
		    /*  an unnamed construct (e.g. on-empty)  */
		    sprintf (in_msg, " in %s", con_type_str);
		}
	    }
	}

	return (in_msg);
}
