%{
/* rts_cmd_parser.y - yacc input for RULEWORKS command interpreter parser */
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
**  FACILITY:
**	RULEWORKS Command Interpreter
**
**  ABSTRACT:
**	Parse command input.
**      Yacc processes this file and generates rts_cmd_parser_tab.[ch].
**
**  MODIFIED BY:
**	DEC	Digital Equipment Corporation
 *	CPQ	Compaq Computer Corporation
**
**  MODIFICATION HISTORY:
**
**	 1-Oct-1992	DEC	Initial version
**	 1-Nov-1992	DEC	Major enhancements
**	30-Mar-1994	DEC	Make yacc compatible: Don't define bogus
**					YYMAXDEPTH, don't use %pure_parser bisonism,
**					define yylex() with 0 parameters
**	 1-Jun-1994	DEc	Don't ACCVIO on "ppwm ^bogus nil"; "ppwm ^good
**					nil"
**	01-Dec-1999	CPQ	Release with GPL
*/

#include <ctype.h>	/* define toupper 				*/
#include <common.h>	/* declares Molecule, Mol_Compound, Mol_Symbol  */
#include <cli_msg.h>	/* message defs				        */
#include <msg.h>	/* message routines                             */
#include <scan.h>	/* defines Token_Type and Token_Value & scanner	*/
#include <rbs.h>	/* declares Ruling blocks 			*/
#include <cs.h>		/* declares Conflict Set			*/
#include <wm.h>		/* declares Work Memory				*/
#include <mol.h>	/* declares molecules				*/
#include <decl.h>	/* declares declarations			*/
#include <decl_att.h>	/* declares declarations for attributes		*/
#include <list.h>	/* declares lists				*/
#include <ios.h>	/* io streams					*/
#include <dbg.h>	/* declares debugger (interp) info		*/
#include <dbg_p.h>	/* declares debugger private info		*/
#include <callback.h>	/* declares functions i.e. translation...	*/
#ifdef __vms
#include <descrip.h>
#include <rmsdef.h>
#include <iodef.h>
#include <starlet.h>
#include <lib$routines.h>
#include <libclidef.h>
#endif



typedef enum {
   DBG__E_CMD_IS_BOGUS = 0,
   DBG__E_CMD_AT = 1,
   DBG__E_CMD_ADDSTATE,
   DBG__E_CMD_AFTER,
   DBG__E_CMD_CLOSEFILE,
   DBG__E_CMD_COPY,
   DBG__E_CMD_CS,
   DBG__E_CMD_DEFAULT,
   DBG__E_CMD_DISABLE,
   DBG__E_CMD_EBREAK,
   DBG__E_CMD_ENABLE,
   DBG__E_CMD_EXIT,
   DBG__E_CMD_MAKE,
   DBG__E_CMD_MATCHES,
   DBG__E_CMD_MODIFY,
   DBG__E_CMD_NEXT,
   DBG__E_CMD_OPENFILE,
   DBG__E_CMD_RBREAK,
   DBG__E_CMD_PPCLASS,
   DBG__E_CMD_PPWM,
   DBG__E_CMD_PROMPT,
   DBG__E_CMD_QUIT,
   DBG__E_CMD_REMOVE,
   DBG__E_CMD_REMOVE_EVERY,
   DBG__E_CMD_RESTORESTATE,
   DBG__E_CMD_RETURN,
   DBG__E_CMD_RUN,
   DBG__E_CMD_SAVESTATE,
   DBG__E_CMD_SHOW,
   DBG__E_CMD_SPECIALIZE,
   DBG__E_CMD_TRACE,
   DBG__E_CMD_WBREAK,
   DBG__E_CMD_WM,
   DBG__E_CMD_WMHISTORY
} RulDbg_Command;


static RulDbg_Command  dbg_command = DBG__E_CMD_IS_BOGUS;
static long            num_attrs;  /* number of attrs in current class       */
static Molecule	      *result;     /* result and array ???		     */
static long            result_len; /* lenght of result array   	             */
static Pat_Pattern    *pat_pat;	  /* current condition/object pattern        */
static Pat_Classes    *pat_cls;    /* current class pointer                  */
static Pat_Attrs      *pat_att;    /* current attr pointer                   */
static Pat_Tests      *pat_tst;    /* current test pointer                   */
static Pat_Classes   **pcn;        /* pointer to next class pointer          */
static Pat_Attrs     **pan;        /* pointer to next attr pointer           */
static Pat_Tests     **ptn;        /* pointer to next test pointer           */
static Pat_Tests     **pdtn;       /* pointer to next disj test pointer      */
static Decl_Shape      att_shape;  /* current attr shape		     */
static Boolean         pred1;      /* current predidate is a compound pred   */
static Boolean         pred2;      /* opt predidate is a compound pred       */
static Boolean         val_scalar; /* value is scalar                        */
static Boolean         ok; 	   /* continue if ok multiple if's           */
static Decl_Block      cur_blk;    /* current decling block                  */
static Object          cur_wmo;    /* a WMO being made/modified/copied/rem'd */
static Class           cur_cls;    /* class of current WMO                   */
static Class           tmp_cls;    /* temp class holder	                     */
static long            cur_off;
static int             line_number;/* scanner position in the source file    */
static Mol_Instance_Id cur_wid;	   /* current wmo id 			     */
static Mol_Symbol      tmp_atm;    /* temp atom holder	         	     */
static Mol_Symbol      cls_nam;    /* current class name symbol              */
static Mol_Symbol      cur_rb;     /* current ruling block name              */
static Mol_Symbol      cur_rul;    /* current rule name                      */
static Mol_Compound    comp_value; /* used to build a compound value         */
static int             comp_elmts; /* number of elements in a compound value */
static IO_Stream       tmpios;
static int             j;
static long            cur_int;
static double          cur_flt;
static Wm_Opr_Code     wmo_cmd;
static Entry_Data      ebdata;
static RuleName        rul_nam;
static Boolean         on_flag;
static Param_List      pl;
static void           *p1;
static void           *p2;
static void           *p3;
static void           *p4;

/* functions... */
static    void        set_error (void);
static    void        clear_error (void);
static    Boolean     in_error (void);
static    Boolean     in_command (void);
static    long        get_command_type (void);
static    char       *command_type_to_name (RulDbg_Command cmd_type);
static    char       *get_usage_string (RulDbg_Command cmd);
static    char       *get_examples_string (RulDbg_Command cmd);
static    char       *get_hints_string (RulDbg_Command cmd);
static    void        process_command (void);
extern    void        rul__scan_reset_parens(void);
EXTERNAL  Mol_Symbol  rul__rac_active_rule_name;
extern    Boolean     rul__scan_switch_file (IO_Stream);


/*
 * Declare alloca, since Bison code calls it without declaring it.
 */
/*
 * void *alloca(size_t);
 */

/* if not defined it complains, when defined it
   yy defines alloca to be __builtin_alloca
*/
/*
 * #ifndef __GNUC__
 * #define __GNUC__ 0
 * #endif
 */

#define yyparse rul__cmd_yyparse

#ifdef YYPURE
#define yylex(lval)  rul__scan_get_token(lval.tok, &line_number)
#else
#define yylex() rul__scan_get_token(&yylval.tok, &line_number)
#endif

#ifdef YYDEBUG
/* function to print out values of tokens	*/
#define YYERROR_VERBOSE	/* be verbose about diagnosing parsing errors	*/
#define xmalloc malloc	/* where is xmalloc defined/declared? use malloc*/
#else
#define YYDEBUG 0
#endif

static void yyerror(char *msg);

#define INTERP_ERROR	rul__scan_reset_parens(); \
			clear_error (); \
			yyclearin; \
			yyerrok

#define INTERP_ABORT	rul__scan_reset_parens(); \
			clear_error (); \
			rul__dbg_free_pattern (&pat_pat); \
			yyclearin; \
			yyerrok; \
			YYABORT

#define INTERP_ACCEPT	YYACCEPT


%}

/****************************************************************************/
/* DECLARATIONS SECTION							    */

/* %pure_parser not supported by yacc, only bison. */
/*%pure_parser*//* this avoids a clash with an external yylval and typing */

%union {	/* This union is where the parser values are stored:        */
  Token_Value    tok;	/* keeps track of constant token values - terminals */
  Molecule       mol;	/* this is where a molecularized constant is stored */
  Class          cls;	/* symbols are transformed into object-classes	    */
  Object         obj;
  void          *ptr;
  IO_Catagory    ioc;
  IO_Access_Mode ioa;
/*
  Pat_Pattern   *pat;
  Pat_Classes   *pc;
  Pat_Attrs     *pa;
  Pat_Tests     *pt;
*/
};

	
/* each %token declaration below is for a terminal
   symbol in the grammar recognized by the scanner */

/* First the value tokens are listed: */

%token TOK_EOL			/* end-of-line(command) */
%token TOK_EOF			/* end-of-file (exit)   */

%token TOK_INTEGER_CONST	/* integer number	*/
%token TOK_INSTANCE_CONST	/* #1			*/
%token TOK_OPAQUE_CONST		/* %x0			*/
%token TOK_FLOAT_CONST		/* real number		*/

/* Magic characters */
%token TOK_LPAREN
%token TOK_RPAREN
%token TOK_LBRACE
%token TOK_RBRACE
%token TOK_LBRACKET
%token TOK_RBRACKET
%token TOK_HAT
%token TOK_AMPERSAND
%token TOK_TILDE

%token TOK_START_DISJUNCTION
%token TOK_END_DISJUNCTION

/* Arithmetic operators, including precedence.  The first line is lowest
 * precedence, the last highest.  UMINUS is a fictitious token, used with
 * the %prec modifier to make unary minus have high precedence.
 */
%left TOK_PLUS TOK_MINUS
%left TOK_TIMES TOK_DIVIDE TOK_MODULUS
%left UMINUS

/* These declarations associate the terminal symbols with token values */

%token	<tok.sval> TOK_ACCEPT
%token	<tok.sval> TOK_ACCEPT_ATOM
%token	<tok.sval> TOK_ACCEPTLINE_COMPOUND
%token	<tok.sval> TOK_ACCEPTS
%token	<tok.sval> TOK_ACTIVATES
%token	<tok.sval> TOK_ADDSTATE
%token	<tok.sval> TOK_AFTER
%token  <tok.sval> TOK_ALIAS_FOR
%token	<tok.sval> TOK_AND
%token	<tok.sval> TOK_ANY
%token	<tok.sval> TOK_APPEND
%token	<tok.sval> TOK_APPROX_EQUAL
%token	<tok.sval> TOK_ARROW
%token	<tok.sval> TOK_ASCID
%token	<tok.sval> TOK_ASCIZ
%token	<tok.sval> TOK_AT
%token	<tok.sval> TOK_ATOM
%token	<tok.sval> TOK_BIND
%token	<tok.sval> TOK_BLOCK_NAME
%token	<tok.sval> TOK_BY
%token	<tok.sval> TOK_BYTE
%token	<tok.sval> TOK_CATCH
%token	<tok.sval> TOK_CALL_INHERITED
%token	<tok.sval> TOK_CLOSEFILE
%token	<tok.sval> TOK_COMPOUND
%token	<tok.sval> TOK_CONTAINS
%token	<tok.sval> TOK_CONCAT
%token	<tok.sval> TOK_COPY
%token	<tok.sval> TOK_CRLF
%token	<tok.sval> TOK_CS
%token	<tok.sval> TOK_CTRLC
%token	<tok.sval> TOK_DECLARATION_BLOCK
%token  <tok.sval> TOK_DEBUG
%token	<tok.sval> TOK_DEFAULT
%token	<tok.sval> TOK_DIFF_TYPE
%token	<tok.sval> TOK_DISABLE
%token	<tok.sval> TOK_DIVIDE
%token	<tok.sval> TOK_DO
%token	<tok.sval> TOK_DOES_NOT_CONTAIN
%token	<tok.sval> TOK_DOLLAR_LAST
%token	<tok.sval> TOK_DOUBLE_FLOAT
%token	<tok.sval> TOK_EBREAK
%token	<tok.sval> TOK_EB
%token	<tok.sval> TOK_ELSE
%token	<tok.sval> TOK_END_DISJUNCTION
%token	<tok.sval> TOK_ENABLE
%token	<tok.sval> TOK_END_BLOCK
%token	<tok.sval> TOK_END_GROUP
%token	<tok.sval> TOK_ENTRY_BLOCK
%token	<tok.sval> TOK_EQUAL
%token	<tok.sval> TOK_EQUAL_EQUAL
%token	<tok.sval> TOK_EVERY
%token  <tok.sval> TOK_EXCISE
%token	<tok.sval> TOK_EXIT
%token	<tok.sval> TOK_EXTERNAL_ROUTINE
%token	<tok.sval> TOK_FAILURE
%token	<tok.sval> TOK_FILENAME
%token	<tok.sval> TOK_FILL
%token	<tok.sval> TOK_FLOAT
%token	<tok.sval> TOK_FOR_EACH
%token	<tok.sval> TOK_GENATOM
%token	<tok.sval> TOK_GENERIC_METHOD
%token	<tok.sval> TOK_GENINT
%token	<tok.sval> TOK_GET
%token	<tok.sval> TOK_GREATER
%token	<tok.sval> TOK_GREATER_EQUAL
%token	<tok.sval> TOK_IF
%token	<tok.sval> TOK_IN
%token	<tok.sval> TOK_INHERITS_FROM
%token	<tok.sval> TOK_INSTANCE
%token	<tok.sval> TOK_INTEGER
%token	<tok.sval> TOK_IS_OPEN
%token	<tok.sval> TOK_LENGTH
%token	<tok.sval> TOK_LENGTH_EQUAL
%token	<tok.sval> TOK_LENGTH_GREATER 
%token	<tok.sval> TOK_LENGTH_GREATER_EQUAL
%token	<tok.sval> TOK_LENGTH_LESS
%token	<tok.sval> TOK_LENGTH_LESS_EQUAL
%token	<tok.sval> TOK_LENGTH_NOT_EQUAL
%token	<tok.sval> TOK_LESS
%token	<tok.sval> TOK_LESS_EQUAL
%token	<tok.sval> TOK_LEX
%token	<tok.sval> TOK_LONG
%token	<tok.sval> TOK_MAKE
%token	<tok.sval> TOK_MAX
%token	<tok.sval> TOK_MATCHES
%token	<tok.sval> TOK_MEA
%token	<tok.sval> TOK_METHOD
%token	<tok.sval> TOK_MIN
%token	<tok.sval> TOK_MINUS
%token	<tok.sval> TOK_MODIFY
%token	<tok.sval> TOK_MODULUS
%token	<tok.sval> TOK_NEXT
%token	<tok.sval> TOK_NTH
%token	<tok.sval> TOK_NOT
%token	<tok.sval> TOK_NOT_APPROX_EQUAL
%token	<tok.sval> TOK_NOT_EQ
%token	<tok.sval> TOK_NOT_EQUAL
%token	<tok.sval> TOK_ON_EMPTY
%token	<tok.sval> TOK_ON_ENTRY
%token	<tok.sval> TOK_ON_EVERY
%token	<tok.sval> TOK_ON_EXIT
%token	<tok.sval> TOK_OBJECT_CLASS
%token	<tok.sval> TOK_OF
%token	<tok.sval> TOK_OFF
%token	<tok.sval> TOK_ON
%token	<tok.sval> TOK_OPAQUE
%token	<tok.sval> TOK_OPENFILE
%token	<tok.sval> TOK_OR
%token	<tok.sval> TOK_OUT
%token	<tok.sval> TOK_P
%token	<tok.sval> TOK_PATHNAME
%token	<tok.sval> TOK_RBREAK
%token	<tok.sval> TOK_PLUS
%token  <tok.sval> TOK_POINTER
%token	<tok.sval> TOK_POSITION
%token	<tok.sval> TOK_PPCLASS
%token	<tok.sval> TOK_PPWM
%token	<tok.sval> TOK_PROMPT
%token	<tok.sval> TOK_READ_ONLY
%token	<tok.sval> TOK_READ_WRITE
%token	<tok.sval> TOK_QUIT
%token	<tok.sval> TOK_QUOTED_SYMBOL
%token	<tok.sval> TOK_QUOTED_VAR
%token	<tok.sval> TOK_RESTART
%token	<tok.sval> TOK_REFERENCE
%token	<tok.sval> TOK_REMOVE
%token	<tok.sval> TOK_REMOVE_EVERY
%token	<tok.sval> TOK_REPORT
%token	<tok.sval> TOK_RG
%token	<tok.sval> TOK_RUN
%token	<tok.sval> TOK_RESTORESTATE
%token	<tok.sval> TOK_RETURN
%token	<tok.sval> TOK_RETURNS
%token	<tok.sval> TOK_RJUST
%token	<tok.sval> TOK_RULE
%token	<tok.sval> TOK_RULE_BLOCK
%token	<tok.sval> TOK_RULE_GROUP
%token	<tok.sval> TOK_SAVESTATE
%token	<tok.sval> TOK_SAME_TYPE
%token	<tok.sval> TOK_SCALAR
%token	<tok.sval> TOK_SHORT
%token	<tok.sval> TOK_SHOW
%token	<tok.sval> TOK_SINGLE_FLOAT
%token	<tok.sval> TOK_SPACE
%token	<tok.sval> TOK_SPECIALIZE
%token	<tok.sval> TOK_SQL_ATTACH
%token	<tok.sval> TOK_SQL_COMMIT
%token	<tok.sval> TOK_SQL_DELETE
%token	<tok.sval> TOK_SQL_DETACH
%token	<tok.sval> TOK_SQL_FETCH_AS_OBJECT
%token	<tok.sval> TOK_SQL_FETCH_EACH
%token	<tok.sval> TOK_SQL_INSERT
%token	<tok.sval> TOK_SQL_INSERT_FROM_OBJECT
%token	<tok.sval> TOK_SQL_ROLLBACK
%token	<tok.sval> TOK_SQL_START
%token	<tok.sval> TOK_SQL_UPDATE
%token	<tok.sval> TOK_SQL_UPDATE_FROM_OBJECT
%token	<tok.sval> TOK_START_DISJUNCTION
%token	<tok.sval> TOK_STARTUP
%token	<tok.sval> TOK_STRATEGY
%token	<tok.sval> TOK_SUBCOMPOUND
%token	<tok.sval> TOK_SUBSYMBOL
%token	<tok.sval> TOK_SUCCESS
%token	<tok.sval> TOK_SYMBOL
%token	<tok.sval> TOK_SYMBOL_CONST
%token	<tok.sval> TOK_SYMBOL_LENGTH
%token	<tok.sval> TOK_TABTO
%token	<tok.sval> TOK_THEN
%token	<tok.sval> TOK_TIMES
%token	<tok.sval> TOK_TRACE
%token	<tok.sval> TOK_UNSIGNED_BYTE
%token	<tok.sval> TOK_UNSIGNED_LONG
%token	<tok.sval> TOK_UNSIGNED_SHORT
%token	<tok.sval> TOK_USES
%token	<tok.sval> TOK_VALUE
%token	<tok.sval> TOK_VARIABLE
%token	<tok.sval> TOK_VERSION
%token	<tok.sval> TOK_WARNING
%token	<tok.sval> TOK_WBREAK
%token	<tok.sval> TOK_WHEN
%token	<tok.sval> TOK_WHILE
%token	<tok.sval> TOK_WM
%token	<tok.sval> TOK_WMHISTORY
%token	<tok.sval> TOK_WRITE

%token	<tok.ival> TOK_INTEGER_CONST
%token	<tok.ival> TOK_INSTANCE_CONST
%token	<tok.ival> TOK_OPAQUE_CONST

%token	<tok.fval> TOK_FLOAT_CONST

%type	<tok.fval> float_const
%type   <tok.ival> predicate
%type   <tok.ival> single_predicate
%type   <tok.ival> content_predicate
%type   <tok.ival> attr_index
%type   <tok.ival> run_cnt
%type   <tok.ival> ca_index
%type   <tok.ival> integer_const
%type   <tok.ival> opt_ret_value
%type	<tok.sval> quoted_string
%type	<tok.sval> symbol_string
%type	<tok.sval> most_toks

%type	<mol> attr_name
%type	<mol> attr_value
%type	<mol> compound_constant
%type   <mol> constant
%type	<mol> constants
%type	<mol> eb_name
%type	<mol> flt_atom
%type	<mol> int_atom
%type	<mol> molecule
%type	<mol> name
%type	<mol> opaque_atom
%type	<mol> return_value
%type	<mol> rule_name
%type	<mol> scalar_constant
%type	<mol> symbol
%type   <mol> value
%type	<mol> wmh_attr
%type	<mol> wmo_id

%type	<cls> object_class
%type	<cls> ppclass_class

%type	<obj> wmh_wmo

%type	<ptr> disable_keyword 
%type	<ptr> enable_keyword
%type   <ptr> ppwm_show
%type   <ptr> form
%type   <ptr> opt_lhs_terms
%type   <ptr> lhs_term
%type   <ptr> lhs_value
%type   <ptr> restriction
%type   <ptr> restriction_list

%type   <ioc> default_keyword

%type   <ioa> openfile_keyword

/*
%type   <mol> opt_values
%type   <ptr> ce ce_list pos_ce neg_ce forms ce_disjunction
*/


%start	command_line	/* these is where to start in the grammar */

%%

/****************************************************************************/
/* RULES SECTION							    */

command_line:
	command TOK_EOL
		{
		process_command (); 
		dbg_command = DBG__E_CMD_IS_BOGUS;
		INTERP_ABORT;
		}
|	error TOK_EOL
		{ dbg_command = DBG__E_CMD_IS_BOGUS;  INTERP_ABORT;}
|	TOK_LPAREN command TOK_RPAREN
		{ 
		process_command (); 
		dbg_command = DBG__E_CMD_IS_BOGUS;
		INTERP_ABORT;
		}
|	TOK_LPAREN error TOK_RPAREN
		{ dbg_command = DBG__E_CMD_IS_BOGUS;  INTERP_ABORT;}
|	TOK_EOL
		{ dbg_command = DBG__E_CMD_IS_BOGUS;  INTERP_ABORT;}
;

command:
	at_cmd
|	addstate_cmd
|	after_cmd
|	closefile_cmd
|	copy_cmd
|	cs_cmd
|	default_cmd
|	disable_cmd
|	ebreak_cmd
|	enable_cmd
|	exit_cmd
|	make_cmd
|	matches_cmd
|	modify_cmd
|	next_cmd
|	openfile_cmd
|	rbreak_cmd
|	ppclass_cmd
|	ppwm_cmd
|	prompt_cmd
|	quit_cmd
|	remove_cmd
|	remove_every_cmd
|	restorestate_cmd
|	return_cmd
|	run_cmd
|	savestate_cmd
|	show_cmd
|	specialize_cmd
|	trace_cmd
|	wbreak_cmd
|	wm_cmd
|	wmhistory_cmd
;

at_cmd:
	TOK_AT 	{/*MRA*/ dbg_command = DBG__E_CMD_AT;}  symbol
		{
		   p1 = (void *) $3;
		}
;

addstate_cmd:
        TOK_ADDSTATE {/*MRA*/ dbg_command = DBG__E_CMD_ADDSTATE;}  symbol
		{
		   p1 = (void *) $3;
		}
;

after_cmd:
        TOK_AFTER {/*MRA*/ dbg_command = DBG__E_CMD_AFTER;}  after_params
		{ }
;

after_params:
	int_atom symbol
		{
		   p1 = (void *) $1;
		   p2 = (void *) $2;
		}
|	error
		{ INTERP_ABORT; }
;

closefile_cmd:
	TOK_CLOSEFILE
                {/*MRA*/ dbg_command = DBG__E_CMD_CLOSEFILE;
		 rul__dbg_free_param_list (pl); pl = NULL; }
	  closefile_ids
		{ }
;

closefile_ids:
	symbol
		{
		   rul__dbg_add_data_to_list (&pl, DBG__E_PL_MOL, (void*) $1);
		}
|	closefile_ids symbol
		{
		   rul__dbg_add_data_to_list (&pl, DBG__E_PL_MOL, (void*) $2);
		}
;

copy_cmd:
	copy_start attr_name_value_list
		{ }
;

copy_start:
        TOK_COPY {/*MRA*/ dbg_command = DBG__E_CMD_COPY;}  wmo_id
		{
		  cur_wmo = rul__mol_instance_id_value ($3);
		  if (cur_wmo == NULL) {
		    rul__msg_print (CLI_INVWMO, cur_int, "COPY");
		    INTERP_ABORT;
		  }
		  cur_cls = rul__wm_get_class (cur_wmo);
		  num_attrs = rul__decl_get_class_num_attrs (cur_cls) + 1;
		  if (result_len < num_attrs) {
		    result = rul__mem_realloc(result, (num_attrs * 4));
		    result_len = num_attrs;
		  }
		  wmo_cmd = WM__OPR_COPY;
		  for (j = 0; j < result_len; j++) { result[j] = NULL; }
		}
;

cs_cmd:
	TOK_CS
		{ dbg_command = DBG__E_CMD_CS; }
;

default_cmd:
        TOK_DEFAULT {/*MRA*/ dbg_command = DBG__E_CMD_DEFAULT;} 
	symbol default_keyword
		{
		   p1 = (void *) $3;
		   p2 = (void *) $4;
		}
;

default_keyword:
	TOK_ACCEPT
		{ $$ = IOS__E_ACCEPT; }
|	TOK_TRACE
		{ $$ = IOS__E_TRACE; }
|	TOK_WRITE
		{ $$ = IOS__E_WRITE; }
|	error
		{ INTERP_ABORT; }
;

disable_cmd:
        TOK_DISABLE {/*MRA*/ dbg_command = DBG__E_CMD_DISABLE;}  disable_keyword
		{ p1 = (void *) $3; }
;

disable_keyword:
	TOK_WARNING
		{ $$ = (void *) TOK_WARNING; }
|	TOK_WMHISTORY
		{ $$ = (void *) TOK_WMHISTORY; }
|	TOK_BLOCK_NAME
		{ $$ = (void *) TOK_BLOCK_NAME; }
|	TOK_CTRLC
		{ $$ = (void *)TOK_CTRLC; }
|	error
		{ INTERP_ABORT; }
;

ebreak_cmd:
	TOK_EBREAK
                {/*MRA*/ dbg_command = DBG__E_CMD_EBREAK;
		rul__dbg_free_param_list (pl); pl = NULL; }
	    ebreak_what
		{ }
|	TOK_EB
                {/*MRA*/ dbg_command = DBG__E_CMD_EBREAK;
		rul__dbg_free_param_list (pl); pl = NULL; }
	    ebreak_what
		{ }
;

ebreak_what:
	/* empty - ebreak_show */
		{ pl = (void *) -1; }
|	ebreak_change
		{ }
;


ebreak_change:
 	TOK_ON eb_on_par
		{ on_flag = TRUE; }
|  	TOK_OFF eb_off_par
		{ on_flag = FALSE; }
;

eb_on_par:
  	TOK_TIMES
		{ }
|  	ebreak_on_params
		{ }
;

ebreak_on_params:
	eb_name
		{
		   rul__dbg_add_data_to_list (&pl, DBG__E_PL_MOL, (void*) $1);
		}
|	ebreak_on_params eb_name
		{
		   rul__dbg_add_data_to_list (&pl, DBG__E_PL_MOL, (void*) $2);
		}
;

eb_off_par:
  	TOK_TIMES
		{ }
|  	ebreak_off_params
		{ }
;

ebreak_off_params:
	eb_name
		{
		   rul__dbg_add_data_to_list (&pl, DBG__E_PL_MOL, (void*) $1);
		}
|	ebreak_off_params eb_name
		{
		   rul__dbg_add_data_to_list (&pl, DBG__E_PL_MOL, (void*) $2);
		}
|	integer_const
		{
		   rul__dbg_add_data_to_list (&pl, DBG__E_PL_INT, (void*) $1);
		}
|	ebreak_off_params integer_const
		{
		   rul__dbg_add_data_to_list (&pl, DBG__E_PL_INT, (void*) $2);
		}
;

eb_name:
	name
		{ }
;

enable_cmd:
        TOK_ENABLE {/*MRA*/ dbg_command = DBG__E_CMD_ENABLE;}  enable_keyword
		{ p1 = (void *) $3; }
;

enable_keyword:
	TOK_WARNING
		{ $$ = (void *) TOK_WARNING; }
|	TOK_WMHISTORY
		{ $$ = (void *) TOK_WMHISTORY; }
|	TOK_BLOCK_NAME
		{ $$ = (void *) TOK_BLOCK_NAME; }
|	TOK_CTRLC
		{ $$ = (void *) TOK_CTRLC; }
|	error
		{ INTERP_ABORT; }
;


exit_cmd:
	TOK_EXIT
		{
		   dbg_command = DBG__E_CMD_EXIT;
		   p1 = (void *) NULL;
		   cmd_exit ();
		   INTERP_ABORT;
		}
|	TOK_EOF 
		{
		   dbg_command = DBG__E_CMD_EXIT;
		   p1 = (void *) -1;
		   cmd_exit ();
		   INTERP_ABORT;
		}
;

make_cmd:
	make_start attr_name_value_list
		{ }
;

make_start:
        TOK_MAKE {/*MRA*/ dbg_command = DBG__E_CMD_MAKE;}  object_class
		{
		  num_attrs = rul__decl_get_class_num_attrs ($3) + 1;
		  if (result_len < num_attrs) {
		    result = rul__mem_realloc (result, (num_attrs*4));
		    result_len = num_attrs;
		  }
		  wmo_cmd = WM__OPR_MAKE;
		  for (j = 0; j < result_len; j++) {
		    result[j] = NULL;
		  }
		}
;

matches_cmd:
	TOK_MATCHES 
            {/*MRA*/ dbg_command = DBG__E_CMD_MATCHES;
	    rul__dbg_free_param_list (pl); pl = NULL; }
	  matches_rules
		{ }
;

matches_rules:
  	rule_name
		{ }
|	matches_rules rule_name
		{ }
;

modify_cmd:
	modify_start attr_name_value_list
		{ }
;

modify_start:
        TOK_MODIFY {/*MRA*/ dbg_command = DBG__E_CMD_MODIFY;}  wmo_id
		{
		  cur_wmo = rul__mol_instance_id_value ($3);
		  if (cur_wmo == NULL) {
		    rul__msg_print (CLI_INVWMO, cur_int, "MODIFY");
		    INTERP_ABORT;
		  }
		  cur_cls = rul__wm_get_class (cur_wmo);
		  num_attrs = rul__decl_get_class_num_attrs (cur_cls) + 1;
		  if (result_len < num_attrs) {
		    result = rul__mem_realloc (result, (num_attrs * 4));
		    result_len = num_attrs;
		  }
		  wmo_cmd = WM__OPR_MODIFY;
		  for (j = 0; j < result_len; j++) {
		    result[j] = NULL;
		  }
		}
;

next_cmd:
	TOK_NEXT
		{ dbg_command = DBG__E_CMD_NEXT; }
;

openfile_cmd:
        TOK_OPENFILE  {/*MRA*/ dbg_command = DBG__E_CMD_OPENFILE; }
	symbol symbol openfile_keyword
		{
		   p1 = (void *) $3;
		   p2 = (void *) $4;
		   p3 = (void *) $5;
		}
;

openfile_keyword:
	/* empty */
		{ $$ = IOS__E_IN; }
|	TOK_IN
		{ $$ = IOS__E_IN; }
|	TOK_OUT
		{ $$ = IOS__E_OUT; }
|	TOK_APPEND
		{ $$ = IOS__E_APPEND; }
|	error
		{ INTERP_ABORT; }
;

rbreak_cmd:
	TOK_RBREAK
            {/*MRA*/  dbg_command = DBG__E_CMD_RBREAK;
	    rul__dbg_free_param_list (pl); pl = NULL; }
	  rbreak_what
		{ }
;

rbreak_what:
	/* empty */
		{ pl = (void *) -1; }
|	rbreak_change
		{ }
;

rbreak_change:
 	TOK_ON rbreak_on_params
		{ on_flag = TRUE; }
|  	TOK_OFF rb_off_par
		{ on_flag = FALSE; }
;

rbreak_on_params:
	rule_name
		{ }
|	rbreak_on_params rule_name
		{ }
;

rb_off_par:
  	TOK_TIMES
		{ }
|  	rbreak_off_params
		{ }
;

rbreak_off_params:
	rule_name
		{ }
|	rbreak_off_params rule_name
		{ }
|	integer_const
		{
		   rul__dbg_add_data_to_list (&pl, DBG__E_PL_INT, (void*) $1);
		}
|	rbreak_off_params integer_const
		{
		   rul__dbg_add_data_to_list (&pl, DBG__E_PL_INT, (void*) $2);
		}
;

ppclass_cmd:
        TOK_PPCLASS {/*MRA*/ dbg_command = DBG__E_CMD_PPCLASS;}  ppclass_class
		{ }
;

ppclass_class:
  	/* empty - list all classes */
		{ cur_cls = NULL; }
|	object_class
		{ }
;

ppwm_cmd:
	TOK_PPWM
            {/*MRA*/  dbg_command = DBG__E_CMD_PPWM;
	    rul__dbg_free_param_list (pl); pl = NULL; }
	  ppwm_show
		{ }
;

ppwm_show:
	/* empty */
		{ }
|	form /* ce_list */
		{
		   rul__dbg_add_data_to_list (&pl, DBG__E_PL_PAT, (void*) $1);
		   pat_pat = NULL;
		}
;

prompt_cmd:
        TOK_PROMPT {/*MRA*/ dbg_command = DBG__E_CMD_PROMPT;}  symbol
		{ p1 = (void *) $3; }
;

quit_cmd:
        TOK_QUIT {/*MRA*/ dbg_command = DBG__E_CMD_QUIT;}  opt_ret_value
		{ p1 = (void *) $3; }
;

opt_ret_value:
	/* empty */
		{ $$ = EXIT_SUCCESS; }
|	TOK_SUCCESS
		{ $$ = EXIT_SUCCESS; }
|	TOK_FAILURE
		{ $$ = EXIT_FAILURE; }
|	integer_const
		{ $$ = $1; }
;

remove_cmd:
	TOK_REMOVE
            {/*MRA*/  dbg_command = DBG__E_CMD_REMOVE;
	    rul__dbg_free_param_list (pl); pl = NULL; }
	  rem_wmos
		{ }
;

rem_wmos:
	/* empty */
|	TOK_TIMES
		{ }
|	rem_wmos wmo_id
		{
		   rul__dbg_add_data_to_list (&pl, DBG__E_PL_INT, (void*) $2);
		}
|	error
		{ INTERP_ABORT; }
;

remove_every_cmd:
        TOK_REMOVE_EVERY 
	    {/*MRA*/ dbg_command = DBG__E_CMD_REMOVE_EVERY;}
	  object_class
		{ }
;

restorestate_cmd:
        TOK_RESTORESTATE 
	    {/*MRA*/ dbg_command = DBG__E_CMD_RESTORESTATE;}
	  symbol
		{ p1 = (void *) $3; }
;

return_cmd:
        TOK_RETURN {/*MRA*/ dbg_command = DBG__E_CMD_RETURN;}  return_value
		{ p1 = (void *) $3; }
;

return_value:
	/* empty */
		{ $$ = NULL; }
|	molecule
		{ }
;

run_cmd:
	TOK_RUN
            {/*MRA*/ dbg_command = DBG__E_CMD_RUN;}
	  run_cnt
		{ p1 = (void *) $3; }
;

run_cnt:
	/* empty */
		{ $$ = -1; }
|	integer_const
		{ $$ = cur_int;	}
|	symbol_string
		{
		   rul__msg_print (CLI_INVRUNCNT, $1);
		   INTERP_ABORT;
		}
|	quoted_string
		{
		   rul__msg_print (CLI_INVRUNCNT, $1);
		   INTERP_ABORT;
		}
|	flt_atom
		{
		   rul__msg_print_w_atoms(CLI_INVRUNCNF, 1, $1);
		   INTERP_ABORT;
		}
;

savestate_cmd:
        TOK_SAVESTATE {/*MRA*/ dbg_command = DBG__E_CMD_SAVESTATE;}  symbol
		{ p1 = (void *) $3; }
;

show_cmd:
        TOK_SHOW {/*MRA*/ dbg_command = DBG__E_CMD_SHOW;}  show_param
		{ }
;

show_param:
	TOK_VERSION
		{ }
;

specialize_cmd:
	specialize_start attr_name_value_list
		{ }
;

specialize_start:
	TOK_SPECIALIZE
            {/*MRA*/ dbg_command = DBG__E_CMD_SPECIALIZE;}
	  wmo_id object_class
		{
		  cur_wmo = rul__mol_instance_id_value ($3);
		  if (cur_wmo == NULL) {
		    rul__msg_print (CLI_INVWMO, cur_int, "SPECIALIZE");
		    INTERP_ABORT;
		  }
		  tmp_cls = rul__wm_get_class (cur_wmo);
		  tmp_atm = rul__decl_get_class_name (tmp_cls);
		  if (!(rul__decl_is_subclass_of ($4, tmp_cls))) {
		    rul__msg_print_w_atoms(CLI_INVSPECLS, 2, cls_nam, tmp_atm);
		    INTERP_ABORT;
		  }
		  num_attrs = rul__decl_get_class_num_attrs ($4) + 1;
		  if (result_len < num_attrs) {
		    result = rul__mem_realloc (result, (num_attrs * 4));
		    result_len = num_attrs;
		  }
		  wmo_cmd = WM__OPR_SPECIALIZE;
		  for (j = 0; j < result_len; j++) {
		    result[j] = NULL;
		  }
		}
;

trace_cmd:
	TOK_TRACE
            {/*MRA*/ dbg_command = DBG__E_CMD_TRACE;  p1 = NULL;}
	  trace_what
		{ }
;

trace_what:
	/* empty trace_show */
		{ p1 = (void *) -1; }
|	trace_change
		{ }
;

trace_change:
        TOK_ON trace_params
                { on_flag = TRUE; }
|       TOK_OFF trace_params
                { on_flag = FALSE; }
;

trace_params:
	TOK_TIMES
		{ p1 = (void *) ((long) DBG_M_TRACE_ALL); }
|	trace_param_list
		{ }
;

trace_param_list:
  	trace_param
		{ }
|	trace_param_list trace_param
		{ }
;

trace_param:
	TOK_ENTRY_BLOCK
		{ p1 = (void *) ((long) p1 | DBG_M_TRACE_EB); }
|	TOK_EB
		{ p1 = (void *) ((long) p1 | DBG_M_TRACE_EB); }
|	TOK_RULE_GROUP
		{ p1 = (void *) ((long) p1 | DBG_M_TRACE_RG); }
|	TOK_RG
		{ p1 = (void *) ((long) p1 | DBG_M_TRACE_RG); }
|	TOK_RULE
		{ p1 = (void *) ((long) p1 | DBG_M_TRACE_R); }
|	TOK_WM
		{ p1 = (void *) ((long) p1 | DBG_M_TRACE_WM); }
|	TOK_CS
		{ p1 = (void *) ((long) p1 | DBG_M_TRACE_CS); }
;

wbreak_cmd:
	TOK_WBREAK
            {/*MRA*/  dbg_command = DBG__E_CMD_WBREAK;
	    rul__dbg_free_param_list (pl);  pl = NULL;}
	  wbreak_what
		{ }

;

wbreak_what:
	/* empty */
		{ pl = (void *) -1; }
|	wbreak_change
		{ }
;

wbreak_change:
 	TOK_ON wbreak_on_params
		{ on_flag = TRUE; }
|  	TOK_OFF wb_off_par
		{ on_flag = FALSE; }
;


wbreak_on_params:
  	form /* ce_list */
		{
		   rul__dbg_add_data_to_list (&pl, DBG__E_PL_PAT, (void*) $1);
		   pat_pat = NULL;
		}
;

wb_off_par:
  	TOK_TIMES
		{ }
|  	wbreak_off_params
		{ }
;

wbreak_off_params:
	form
		{ 
		   rul__dbg_add_data_to_list (&pl, DBG__E_PL_PAT, (void*) $1);
		   pat_pat = NULL;
		}
|	integer_const
		{
		   rul__dbg_add_data_to_list (&pl, DBG__E_PL_INT, (void*) $1);
		}
|	wbreak_off_params integer_const
		{
		   rul__dbg_add_data_to_list (&pl, DBG__E_PL_INT, (void*) $2);
		}
;

attr_name:
	name
		{ }
;

wm_cmd:
	TOK_WM 
            {/*MRA*/  dbg_command = DBG__E_CMD_WM;
	    rul__dbg_free_param_list (pl);  pl = NULL;}
	  wm_show_what
		{ }
;

wm_show_what:
	/* empty */
		{ }
|	wm_show_wmos
		{ }
;

wm_show_wmos:
	wmo_id
		{
		   rul__dbg_add_data_to_list (&pl, DBG__E_PL_MOL, (void*) $1);
		}
|	wm_show_wmos wmo_id
		{
		   rul__dbg_add_data_to_list (&pl, DBG__E_PL_MOL, (void*) $2);
		}
;

wmhistory_cmd:
	TOK_WMHISTORY
            {/*MRA*/ dbg_command = DBG__E_CMD_WMHISTORY;}
	  wmh_wmo wmh_attr
		{
		  if ($4 != NULL) {
		    if (!rul__decl_is_attr_in_class (cur_cls, $4)) {
		      rul__msg_print_w_atoms (CLI_INVCLSATT, 2, $4, cls_nam);
		      INTERP_ABORT;
		    }
		  }
		  rul__wm_print_histform ($3, $4, RUL__C_STD_ERR);
		  rul__ios_printf(RUL__C_STD_ERR, "\n");
		}
;

wmh_wmo:
	wmo_id
		{
		  $$ = cur_wmo = rul__mol_instance_id_value ($1);
		  if (cur_wmo) {
		    cur_cls = rul__wm_get_class (cur_wmo);
		    cls_nam = rul__decl_get_class_name (cur_cls);
		  }
		  else {
		    rul__msg_print_w_atoms (CLI_NOSUCHWMO, 1, $1);
		    INTERP_ABORT;
		  }
		}
;

wmh_attr:
	/* empty */
		{ $$ = NULL; }
|	TOK_HAT attr_name
		{ $$ = $2; }
;

attr_name_value_list:
	 /* empty */
|	attr_name_value_list attr_name_value
;

attr_name_value:
	TOK_HAT attr_name attr_index attr_value
		{
		  if (!rul__decl_is_attr_in_class (cur_cls, $2)) {
		    rul__msg_print_w_atoms (CLI_INVCLSATT, 2, $2, cls_nam);
		    INTERP_ABORT;
		  }
		  cur_off = rul__decl_get_attr_offset (cur_cls, $2);
		  att_shape = rul__decl_get_attr_shape (cur_cls, $2);
		  if ($3 == 0) {
		    if (att_shape == shape_compound &&
			!rul__mol_is_compound ($4)) {
		      rul__ios_printf (RUL__C_STD_ERR,
			       "   Invalid compound attribute value\n");
		      INTERP_ABORT;
		    }
		    else if (att_shape != shape_compound &&
			     rul__mol_is_compound ($4)) {
		      rul__ios_printf (RUL__C_STD_ERR,
			       "   Invalid scalar attribute value\n");
		      INTERP_ABORT;
		    }
		    result[cur_off] = $4;
		  }
		  else {
		    if (att_shape != shape_compound) {
		      rul__ios_printf (RUL__C_STD_ERR,
			       "   Invalid indexing of scalar attribute\n");
		      INTERP_ABORT;
		    }
		    if (rul__mol_is_compound ($4)) {
		      rul__ios_printf (RUL__C_STD_ERR,
				       "   Invalid indexed attribute value\n");
		      INTERP_ABORT;
		    }
		    if (result[cur_off] == NULL) {
		      if (wmo_cmd != WM__OPR_MAKE)
			result[cur_off] = rul__wm_get_offset_val (cur_wmo,
								  cur_off);
		      else
			result[cur_off] = rul__mol_make_comp (0);
		    }
		    if ($3 == -1) {
		      $3 = rul__mol_get_poly_count (result[cur_off]);
		      if ($3 == 0)
			$3 = 1;
		    }
		    result[cur_off] = rul__mol_set_comp_nth (result[cur_off],
							     $3, $4, 
				     rul__decl_get_attr_fill (cur_cls, $2));

		  }
		}
|	error
		{ INTERP_ABORT; }
;

object_class:
	name TOK_TILDE name
		{
		  if (rul__dbg_is_valid_db ($1)) {
		    cur_blk = rul__decl_get_block ($1);
		    cur_rb = $1;
		    cur_cls = $$ = rul__decl_get_class (cur_blk, $3);
		    if ($$ == NULL) {
		      if (rul__dbg_is_visible_class ($3))
			rul__msg_print_w_atoms (CLI_INVBLKCLS, 2, $3, $1);
		      else
			rul__msg_print_w_atoms (CLI_INVCLSNAM, 1, $3);
		      INTERP_ABORT;
		    }
		    cls_nam = $3;
		  }
		  else {
		    rul__msg_print_w_atoms (CLI_INVBLKNAM, 1, $1);
		    INTERP_ABORT;
		  }
		}
|	name
		{
		  cur_blk = NULL;
		  cur_cls = $$ = rul__decl_get_class_if_unique ($1);
		  if ($$ == NULL) {
		    if (rul__dbg_is_visible_class ($1)) {
		      rul__msg_print_w_atoms (CLI_AMBCLSNAM, 1, $1);
		      rul__dbg_show_amb_class ($1);
		    }
		    else
		      rul__msg_print_w_atoms (CLI_INVCLSNAM, 1, $1);
		    INTERP_ABORT;
		  }
		  cur_blk = rul__decl_get_class_block (cur_cls);
		  if (cur_blk)
		    cur_rb = rul__decl_get_decl_name (cur_blk);
		  cls_nam = $1;
		}
|	error
		{ INTERP_ABORT; }
;

attr_value:
	molecule
;

/*
ce_list:
	ce_list ce
|	ce
;

ce:
	pos_ce
		{ pat_cls->ctype = PAT_CE_POS; }
|	neg_ce
		{ pat_cls->ctype = PAT_CE_NEG; }
;

pos_ce:
	form
		{ pat_cls->ctype = PAT_CE_POS; }
|	ce_disjunction
		{ pat_pat->ptype = PAT_DISJ; }
;

neg_ce:
	TOK_MINUS form
		{ pat_cls->ctype = PAT_CE_NEG; }
;

ce_disjunction:
	TOK_START_DISJUNCTION forms TOK_END_DISJUNCTION
		{ pat_pat->ptype = PAT_DISJ; }
;

forms:
	forms form
|	form
		{ }
;
*/

form:
	TOK_LPAREN object_class 
		{
		  if (pat_pat == NULL) {
		    pat_pat = RUL_MEM_ALLOCATE (Pat_Pattern, 1);
		    pcn = &(pat_pat->pc);
		  }
		  *pcn = pat_cls = RUL_MEM_ALLOCATE (Pat_Classes, 1);
		  pat_cls->cls = $2;
		  pan = &(pat_cls->pa);
		  pcn = &(pat_cls->next);
		}
	    opt_lhs_terms TOK_RPAREN
		{ $$ = pat_pat; }
|	object_class 
		{
		  if (pat_pat == NULL) {
		    pat_pat = RUL_MEM_ALLOCATE (Pat_Pattern, 1);
		    pcn = &(pat_pat->pc);
		  }
		  *pcn = pat_cls = RUL_MEM_ALLOCATE (Pat_Classes, 1);
		  pat_cls->cls = $1;
		  pan = &(pat_cls->pa);
		  pcn = &(pat_cls->next);
		}
	    opt_lhs_terms
		{ $$ = pat_pat; }
;

opt_lhs_terms:
	/* empty */
		{ $$ = NULL; }
|	opt_lhs_terms lhs_term
		{ $$ = $2; }
;

lhs_term:
	ce_attr lhs_value
		{
		  ok = TRUE;
		  if (att_shape != shape_compound ||
		      att_shape == shape_compound && pat_att->idx) {
		     if (pat_att->idx && att_shape != shape_compound) {
			rul__msg_print_w_atoms (CLI_INVATTSYN, 1,
						pat_att->attr);
			ok = FALSE;
		     }
		     else if ((pred1 && val_scalar) ||
			      (pred2 && val_scalar)) {
			rul__msg_print (CLI_INVPRDUSG);
			ok = FALSE;
		     }
		  }
		  else { /* compound attribute */
		     if (!pred1 && val_scalar) {
			rul__msg_print (CLI_INVPRDUSG);
			ok = FALSE;
		     }
		     else if ((pred1 || pred2) && !val_scalar) {
			rul__msg_print (CLI_INVPRDUSG);
			ok = FALSE;
		     }
		  }
		  if (ok == FALSE) {
		     INTERP_ABORT;
		  }
		}
;

ce_attr:
	TOK_HAT attr_name attr_index
		{
		  *pan = pat_att = RUL_MEM_ALLOCATE (Pat_Attrs, 1);
		  if (!rul__decl_is_attr_in_class (pat_cls->cls, $2)) {
		    rul__msg_print_w_atoms (CLI_INVCLSATT, 2, $2, cls_nam);
		    INTERP_ABORT;
		  }
		  pat_att->attr = $2;
		  pat_att->idx = $3;
		  att_shape = rul__decl_get_attr_shape (pat_cls->cls, $2);
		  val_scalar = TRUE;
		  pred1 = FALSE;
		  pred2 = FALSE;
		  pan = &(pat_att->next);
		  ptn = &(pat_att->pt);
		}
|	error
		{ INTERP_ABORT; }
;

attr_index:
	/* empty */
		{ $$ = 0; }
|	TOK_LBRACKET ca_index TOK_RBRACKET
		{ $$ = $2; }
|	error
		{ INTERP_ABORT; }
;

ca_index:
	integer_const
		{
		  $$ = $1;
		  if ($1 == 0) {
		    rul__msg_print (CLI_IDXZERO);
		    INTERP_ABORT;
		  }
		}
|	TOK_DOLLAR_LAST
		{ $$ = -1; }
/*
|	variable
|	TOK_LPAREN function_call TOK_RPAREN
		{ $$ = $2; }
|	TOK_LPAREN arith_expr TOK_RPAREN
		{ $$ = $2; }
*/
;

lhs_value:
	restriction
|	TOK_LBRACE
		{
		  *ptn = pat_tst = RUL_MEM_ALLOCATE (Pat_Tests, 1);
		  pat_tst->ttype = PAT_CONJ;
		  ptn = &(pat_tst->next);
		}
	    restriction_list TOK_RBRACE
		{ }
;

restriction_list:
	restriction_list restriction
|	restriction
;

restriction:
	TOK_START_DISJUNCTION 
		{
		  *ptn = pat_tst = RUL_MEM_ALLOCATE (Pat_Tests, 1);
		  pat_tst->ttype = PAT_DISJ;
		  ptn = &(pat_tst->next);
		  pdtn = &(pat_tst->u.pt);
		}
	    pat_val_list TOK_END_DISJUNCTION
		{ }
|	predicate value
		{
		  *ptn = pat_tst = RUL_MEM_ALLOCATE (Pat_Tests, 1);
		  ptn = &(pat_tst->next);
		  pat_tst->ttype = PAT_SCALAR;
		  pat_tst->pred = $1;
		  pat_tst->u.value = $2;
		} 
|	value
		{
		  *ptn = pat_tst = RUL_MEM_ALLOCATE (Pat_Tests, 1);
		  ptn = &(pat_tst->next);
		  pat_tst->ttype = PAT_SCALAR;
		  pat_tst->pred = TOK_EQUAL_EQUAL;
		  pat_tst->u.value = $1;
		} 
;

pat_val_list:
	value
		{
		  *pdtn = pat_tst = RUL_MEM_ALLOCATE (Pat_Tests, 1);
		  pat_tst->ttype = PAT_SCALAR;
		  pat_tst->pred = TOK_EQUAL_EQUAL;
		  pat_tst->u.value = $1;
		  pdtn = &(pat_tst->next);
		}
|	pat_val_list value
		{
		  *pdtn = pat_tst = RUL_MEM_ALLOCATE (Pat_Tests, 1);
		  pat_tst->ttype = PAT_SCALAR;
		  pat_tst->pred = TOK_EQUAL_EQUAL;
		  pat_tst->u.value = $2;
		  pdtn = &(pat_tst->next);
		}
;

predicate:
  	single_predicate
|	content_predicate
;

single_predicate:
	TOK_APPROX_EQUAL
		{ $$ = TOK_APPROX_EQUAL; }
|	TOK_NOT_APPROX_EQUAL
		{ $$ = TOK_NOT_APPROX_EQUAL; }
|	TOK_EQUAL
		{ $$ = TOK_EQUAL; }
|	TOK_EQUAL_EQUAL
		{ $$ = TOK_EQUAL_EQUAL; }
|	TOK_GREATER
		{ $$ = TOK_GREATER; }
|	TOK_GREATER_EQUAL
		{ $$ = TOK_GREATER_EQUAL; }
|	TOK_LESS
		{ $$ = TOK_LESS; }
|	TOK_LESS_EQUAL
		{ $$ = TOK_LESS_EQUAL; }
|	TOK_NOT_EQ
		{ $$ = TOK_NOT_EQ; }
|	TOK_NOT_EQUAL
		{ $$ = TOK_NOT_EQUAL; }
|	TOK_SAME_TYPE
		{ $$ = TOK_SAME_TYPE; }
|	TOK_DIFF_TYPE
		{ $$ = TOK_DIFF_TYPE; }
;

content_predicate:
	TOK_LENGTH_LESS_EQUAL
		{ $$ = TOK_LENGTH_LESS_EQUAL; pred1 = TRUE; }
|	TOK_LENGTH_EQUAL
		{ $$ = TOK_LENGTH_EQUAL; pred1 = TRUE; }
|	TOK_LENGTH_NOT_EQUAL
		{ $$ = TOK_LENGTH_NOT_EQUAL; pred1 = TRUE; }
|	TOK_LENGTH_LESS
		{ $$ = TOK_LENGTH_LESS; pred1 = TRUE; }
|	TOK_LENGTH_GREATER_EQUAL
		{ $$ = TOK_LENGTH_GREATER_EQUAL; pred1 = TRUE; }
|	TOK_LENGTH_GREATER
		{ $$ = TOK_LENGTH_GREATER; pred1 = TRUE; }
|	TOK_CONTAINS
		{ $$ = TOK_CONTAINS; pred1 = TRUE; }
|	TOK_DOES_NOT_CONTAIN
		{ $$ = TOK_DOES_NOT_CONTAIN; pred1 = TRUE; }
|	TOK_CONTAINS single_predicate
		{ $$ = TOK_CONTAINS + ($2 << 16); pred1 = TRUE; }
|	TOK_DOES_NOT_CONTAIN single_predicate
		{ $$ = TOK_DOES_NOT_CONTAIN + ($2 << 16); pred1 = TRUE; }
|	single_predicate TOK_CONTAINS
		{ $$ = $1 + (TOK_CONTAINS << 16); pred1 = FALSE;
		  pred2 = TRUE; }
|	single_predicate TOK_DOES_NOT_CONTAIN
		{ $$ = $1 + (TOK_DOES_NOT_CONTAIN << 16); pred1 = FALSE;
		  pred2 = TRUE; }
;

/*
arith_expr:
	value TOK_PLUS value
|	value TOK_MINUS value
|	value TOK_TIMES value
|	value TOK_DIVIDE value
|	value TOK_MODULUS value
|	TOK_MINUS value %prec UMINUS
;
*/

value:
	constant
/*
|	variable
|	TOK_LPAREN function_call TOK_RPAREN
|	TOK_LPAREN arith_expr TOK_RPAREN
|	TOK_LPAREN copy_action TOK_RPAREN
|	TOK_LPAREN make_action TOK_RPAREN
|	TOK_LPAREN modify_action TOK_RPAREN
|	TOK_LPAREN specialize_action TOK_RPAREN
*/
;

constant:
	int_atom
|	flt_atom
|	wmo_id
|	opaque_atom
|	symbol
|	compound_constant
;

symbol:
	symbol_string
		{
		  $$ = rul__mol_make_symbol(rul__dbg_toupper($1));
		}
|	quoted_string
		{
		  $$ = rul__mol_make_symbol($1);
		}
;

rule_name:
	name
		{ 
		   rul_nam = rul__mem_malloc (sizeof (struct rulename));
		   rul_nam->rb_name = NULL;
		   rul_nam->rule_name = $1;
		   rul__dbg_add_data_to_list (&pl, DBG__E_PL_RUL,
					      (void*) rul_nam);
		}
|	name TOK_TILDE name
		{
		   rul_nam = rul__mem_malloc (sizeof (struct rulename));
		   rul_nam->rb_name = $1;
		   rul_nam->rule_name = $3;
		   rul__dbg_add_data_to_list (&pl, DBG__E_PL_RUL,
					      (void*) rul_nam);
		}
;

name:
  	most_toks
		{ $$ = rul__mol_make_symbol (rul__dbg_toupper ($1)); }
|	quoted_string
		{ $$ = rul__mol_make_symbol ($1); }
;

molecule:
  	scalar_constant
|	compound_constant
;

quoted_string:
	TOK_QUOTED_SYMBOL
;

symbol_string:
	TOK_TIMES
|	most_toks
;

most_toks:
	TOK_AT
|	TOK_ACCEPT
|	TOK_ACCEPT_ATOM
|	TOK_ACCEPTLINE_COMPOUND
|	TOK_ACCEPTS
|	TOK_ACTIVATES
|	TOK_ADDSTATE
|	TOK_AFTER
|	TOK_ALIAS_FOR
|	TOK_AND
|	TOK_ANY
|	TOK_APPEND
|	TOK_ARROW
|	TOK_ASCID
|	TOK_ASCIZ
|	TOK_ATOM
|	TOK_BIND
|	TOK_BLOCK_NAME
|	TOK_BY
|	TOK_BYTE
|	TOK_CATCH
|	TOK_CALL_INHERITED
|	TOK_CLOSEFILE
|	TOK_COMPOUND
|	TOK_CONCAT
|	TOK_COPY
|	TOK_CRLF
|	TOK_CS
|	TOK_CTRLC
|	TOK_DECLARATION_BLOCK
|	TOK_DEBUG
|	TOK_DEFAULT
|	TOK_DISABLE
|	TOK_DIVIDE
|	TOK_DO
|	TOK_DOLLAR_LAST
|	TOK_DOUBLE_FLOAT
|	TOK_EB
|	TOK_ELSE
|	TOK_ENABLE
|	TOK_END_BLOCK
|	TOK_END_GROUP
|	TOK_ENTRY_BLOCK
|	TOK_EVERY
|	TOK_EXCISE
|	TOK_EXTERNAL_ROUTINE
|	TOK_FAILURE
|	TOK_FILENAME
|	TOK_FILL
|	TOK_FLOAT
|	TOK_FOR_EACH
|	TOK_GENATOM
|	TOK_GENERIC_METHOD
|	TOK_GENINT
|	TOK_GET
|	TOK_IF
|	TOK_IN
|	TOK_INHERITS_FROM
|	TOK_INSTANCE
|	TOK_INTEGER
|	TOK_IS_OPEN
|	TOK_LENGTH
|	TOK_LEX
|	TOK_LONG
|	TOK_MAKE
|	TOK_MATCHES
|	TOK_MAX
|	TOK_MEA
|	TOK_METHOD
|	TOK_MIN
|	TOK_MINUS
|	TOK_MODIFY
|	TOK_MODULUS
|	TOK_NEXT
|	TOK_NOT
|	TOK_NTH
|	TOK_OBJECT_CLASS
|	TOK_OF
|	TOK_OFF
|	TOK_ON
|	TOK_ON_EMPTY
|	TOK_ON_ENTRY
|	TOK_ON_EVERY
|	TOK_ON_EXIT
|	TOK_OPAQUE
|	TOK_OPENFILE
|	TOK_OR
|	TOK_OUT
|	TOK_P
|	TOK_PATHNAME
|	TOK_RBREAK
|	TOK_PLUS
|	TOK_POINTER
|	TOK_POSITION
|	TOK_PPCLASS
|	TOK_PPWM
|	TOK_PROMPT
|	TOK_QUIT
|	TOK_QUOTED_VAR
|	TOK_READ_ONLY
|	TOK_READ_WRITE
|	TOK_REFERENCE
|	TOK_REMOVE
|	TOK_REMOVE_EVERY
|	TOK_REPORT
|	TOK_RESTART
|	TOK_RESTORESTATE
|	TOK_RETURN
|	TOK_RETURNS
|	TOK_RG
|	TOK_RJUST
|	TOK_RULE
|	TOK_RULE_BLOCK
|	TOK_RULE_GROUP
|	TOK_RUN
|	TOK_SAVESTATE
|	TOK_SCALAR
|	TOK_SHORT
|	TOK_SHOW
|	TOK_SINGLE_FLOAT
|	TOK_SPACE
|	TOK_SPECIALIZE
|	TOK_SQL_ATTACH
|	TOK_SQL_COMMIT
|	TOK_SQL_DELETE
|	TOK_SQL_DETACH
|	TOK_SQL_FETCH_AS_OBJECT
|	TOK_SQL_FETCH_EACH
|	TOK_SQL_INSERT
|	TOK_SQL_INSERT_FROM_OBJECT
|	TOK_SQL_ROLLBACK
|	TOK_SQL_START
|	TOK_SQL_UPDATE
|	TOK_SQL_UPDATE_FROM_OBJECT
|	TOK_STARTUP
|	TOK_STRATEGY
|	TOK_SUBSYMBOL
|	TOK_SUBCOMPOUND
|	TOK_SUCCESS
|	TOK_SYMBOL
|	TOK_SYMBOL_CONST
|	TOK_SYMBOL_LENGTH
|	TOK_TABTO
|	TOK_THEN
|	TOK_TRACE
|	TOK_UNSIGNED_BYTE
|	TOK_UNSIGNED_LONG
|	TOK_UNSIGNED_SHORT
|	TOK_USES
|	TOK_VALUE
|	TOK_VARIABLE
|	TOK_VERSION
|	TOK_WARNING
|	TOK_WBREAK
|	TOK_WHILE
|	TOK_WHEN
|	TOK_WM
|	TOK_WMHISTORY
|	TOK_WRITE
;

int_atom:
	integer_const
		{
		  $$ = rul__mol_make_int_atom (cur_int);
		}
;

integer_const:
	TOK_INTEGER_CONST
		{
		  $$ = $1;
		  cur_int = $1;
		}	
;

flt_atom:
	float_const
		{
		  $$ = rul__mol_make_dbl_atom (cur_flt);
		}
;

float_const:
	TOK_FLOAT_CONST
		{
		  $$ = $1;
		  cur_flt = $1;
		}
;

opaque_atom:
	TOK_OPAQUE_CONST
		{
		  $$ = rul__mol_make_opaque ((void *) $1);
		}
;

compound_constant:
	TOK_LPAREN TOK_COMPOUND constants TOK_RPAREN
		{
		  val_scalar = FALSE;
		  $$ = $3;
		}
;

constants:
	/* empty */
		{
		  comp_elmts = 0;
		  comp_value = rul__mol_make_comp (comp_elmts);
		  $$ = comp_value;
		}
|	constants scalar_constant
		{
		  comp_value = rul__mol_set_comp_nth (comp_value,
						      ++comp_elmts,
						      $2,
						      (Molecule) NULL);
		  $$ = comp_value;
		}
;

scalar_constant:
	symbol_string
		{
		  $$ = rul__mol_make_symbol (rul__dbg_toupper ($1));
		}
|	quoted_string
		{
		  $$ = rul__mol_make_symbol ($1);
		}
|	wmo_id
|	int_atom
|	flt_atom
|	opaque_atom
;

wmo_id:
       TOK_INSTANCE_CONST
		{
		  cur_int = $1;
		  if (rul__is_wid_translation_on ()) {
		    $$ = rul__translate_wme_id (rul__mol_make_int_atom ($1));
		  }
		  else {
		    $$ = rul__mol_make_instance_atom ($1);
		    if ($$ == NULL) {
		      rul__msg_print (CLI_INVWMOID, $1);
		      INTERP_ABORT;
		    }
		  }
		}
;

%%


/****************************************************************************/
/* C CODE SECTION							    */

static void       cmd_at (void)
{
   tmpios = rul__ios_open_file (p1, p1, IOS__E_IN);
   if (tmpios != NULL) {
      rul__scan_switch_file (tmpios);
   }
}

static void       cmd_addstate (void)
{
   rul__dbg_addstate ((Molecule) p1);
}

static void       cmd_after (void)
{
   rul__dbg_set_catcher (p1, p2);
}

static void       cmd_closefile (void)
{
   Param_List p = pl;

   while (p) {
      rul__ios_close_file ((Molecule) pl->data);
      p = p->next;
   }
   rul__dbg_free_param_list (pl);
   pl = NULL;
}

static void       cmd_copy (void)
{
   ebdata = rul__dbg_get_ebdata ();
   cur_wmo = rul__wm_copy (cur_wmo, ebdata);
   if (cur_wmo) {
      for (j = DECL_FIRST_USER_ATTR_OFFSET; j < result_len; j++){
	 if (result[j] != NULL)
	   rul__wm_set_offset_val(cur_wmo, j, result[j], ebdata);
      }
      rul__wm_update_and_notify (cur_wmo, ebdata);
      rul__dbg_flag_changes ();
   }
}

static void       cmd_cs (void)
{
   rul__dbg_print_cs ();
}

static void       cmd_default (void)
{
   rul__ios_set_default (p1, (IO_Catagory) p2);
}

static void       cmd_disable (void)
{
   if (p1 == (void *) TOK_WARNING) {
      rul__msg_disable_messages ();
   }
   else if (p1 == (void *) TOK_WMHISTORY) {
      rul__wm_for_each_object_in_wm (rul__wm_free_wmh_data);
      rul__dbg_gl_enable &= ~DBG_M_ENABLE_WMH;
   }
   else if (p1 == (void *) TOK_BLOCK_NAME) {
      rul__dbg_gl_enable &= ~DBG_M_ENABLE_BKN;
   }
   else if (p1 == (void *) TOK_CTRLC) {
      rul__dbg_disable_control_c ();
   }
}

static void       cmd_ebreak (void)
{
   if (pl == (void *) -1)
     rul__dbg_show_ebreaks ();
   else {
      rul__dbg_ebreak (on_flag, pl);
      rul__dbg_free_param_list (pl);
      pl = NULL;
   }
}

static void       cmd_enable (void)
{
   if (p1 == (void *) TOK_WARNING) {
      rul__msg_enable_messages ();
   }
   else if (p1 == (void *) TOK_WMHISTORY) {
      rul__dbg_gl_enable |= DBG_M_ENABLE_WMH;
   }
   else if (p1 == (void *) TOK_BLOCK_NAME) {
      rul__dbg_gl_enable |= DBG_M_ENABLE_BKN;
   }
   else if (p1 == (void *) TOK_CTRLC) {
      rul__dbg_enable_control_y ();
   }
}

static void       cmd_exit (void)
{
   if (p1 == (void *) -1) {
      if (rul__scan_switch_file (0)) {
	 rul__scan_reset_parens ();
	 return;
      }
   }
   rul__dbg_set_exit (EXIT_SUCCESS); 
}

static void       cmd_make (void)
{
   cur_wid = result[DECL_ID_ATTR_OFFSET];
   if (cur_wid != NULL) {
      if (!rul__is_wid_translation_on ()) {
	 rul__msg_print (CLI_INVMAKID);
	 cur_wid = NULL;
	 return;
      }
      else {
	 if (rul__mol_instance_id_value (cur_wid)) {
	    rul__msg_print (CLI_INVMAKTID);
	    return;
	 }
      }
   }
   ebdata = rul__dbg_get_ebdata ();
   cur_wmo = rul__wm_create (cur_cls, cur_wid, ebdata);
   if (cur_wmo) {
      for (j = DECL_FIRST_USER_ATTR_OFFSET; j < result_len; j++){
	 if (result[j] != NULL)
	   rul__wm_set_offset_val (cur_wmo, j, result[j], ebdata);
      }
      rul__wm_update_and_notify (cur_wmo, ebdata);
      rul__dbg_flag_changes ();
   }
}

static void       cmd_matches (void)
{
   rul__dbg_show_matches (pl);
}

static void       cmd_modify (void)
{
   if (cur_wmo) {
      ebdata = rul__dbg_get_ebdata ();
      rul__wm_modify (cur_wmo, ebdata);
      for (j = DECL_FIRST_USER_ATTR_OFFSET; j < result_len; j++){
	 if (result[j] != NULL)
	   rul__wm_set_offset_val (cur_wmo, j, result[j], ebdata);
      }
      rul__wm_update_and_notify (cur_wmo, ebdata);
      rul__dbg_flag_changes ();
   }
}

static void       cmd_next (void)
{
   rul__dbg_show_next ();
}

static void       cmd_openfile (void)
{
   rul__ios_open_file (p1, p2, (IO_Access_Mode) p3);
}

static void       cmd_ppclass (void)
{
   if (cur_cls == NULL)
     rul__dbg_print_class_list ();
   else
     rul__decl_print_class_info (cur_cls, RUL__C_STD_ERR);
}

static void       cmd_ppwm (void)
{
   Pat_Pattern *pat = NULL;

   if (pl)
     rul__dbg_set_pp_pat ((Pat_Pattern *) pl->data);
   rul__wm_for_each_object_in_wm (rul__dbg_pp_print);
   rul__dbg_free_param_list (pl);
   pl = NULL;
   rul__dbg_set_pp_pat (NULL);
}

static void       cmd_prompt (void)
{
   rul__dbg_set_prompt ((Molecule) p1);
}

static void       cmd_quit (void)
{
   rul__dbg_set_exit ((long) p1);
}

static void       cmd_rbreak (void)
{
   if (pl == (void *) -1)
     rul__dbg_show_rbreaks ();
   else {
      rul__dbg_rbreak (on_flag, pl);
      rul__dbg_free_param_list (pl);
      pl = NULL;
   }
}

static void       cmd_remove (void)
{
   Param_List p = pl;
   
   if (pl == NULL) {
      rul__wm_destroy_and_notify_all (0, rul__dbg_get_ebdata ());
   }
   else {
      while (p) {
	 cur_wmo = rul__mol_instance_id_value ((Molecule) p->data);
	 if (cur_wmo == NULL)
	   rul__msg_print (CLI_INVWMO, cur_int, "REMOVE");
	 else {
	    rul__wm_destroy_and_notify (cur_wmo, rul__dbg_get_ebdata ());
	 }
	 p = p->next;
      }
   }
   rul__dbg_flag_changes ();
   rul__dbg_free_param_list (pl);
   pl = NULL;
}

static void       cmd_remove_every (void)
{
   rul__dbg_remove_class_wmes (cur_cls);
   rul__dbg_flag_changes ();
}

static void       cmd_restorestate (void)
{
   rul__dbg_restorestate (p1);
}

static void       cmd_return (void)
{
   rul__dbg_set_return ((Molecule) p1);
}

static void       cmd_run (void)
{
   rul__dbg_set_run ((long) p1);
}

static void       cmd_savestate (void)
{
   rul__dbg_savestate ((Molecule) p1);
}

static void       cmd_show (void)
{
   rul__msg_version (RUL__C_STD_ERR);
}

static void       cmd_specialize (void)
{
   ebdata = rul__dbg_get_ebdata ();
   cur_wmo = rul__wm_specialize (cur_wmo, cur_cls, ebdata);
   if (cur_wmo) {
      for (j = DECL_FIRST_USER_ATTR_OFFSET; j < result_len; j++){
	 if (result[j] != NULL)
	   rul__wm_set_offset_val (cur_wmo, j, result[j], ebdata);
      }
      rul__wm_update_and_notify (cur_wmo, ebdata);
      rul__dbg_flag_changes ();
   }
}

static void       cmd_trace (void)
{
   if (p1 == (void *) -1) {
      if (rul__dbg_gl_trace & DBG_M_TRACE_ALL) {
	 rul__ios_printf (RUL__C_STD_ERR, "   TRACEs set on: ");
	 if (rul__dbg_gl_trace & DBG_M_TRACE_EB)
	   rul__ios_printf (RUL__C_STD_ERR, " ENTRY-BLOCK");
	 if (rul__dbg_gl_trace & DBG_M_TRACE_RG)
	   rul__ios_printf (RUL__C_STD_ERR, " RULE-GROUP");
	 if (rul__dbg_gl_trace & DBG_M_TRACE_R)
	   rul__ios_printf (RUL__C_STD_ERR, " RULE");
	 if (rul__dbg_gl_trace & DBG_M_TRACE_WM)
	   rul__ios_printf (RUL__C_STD_ERR, " WM");
	 if (rul__dbg_gl_trace & DBG_M_TRACE_CS)
	   rul__ios_printf (RUL__C_STD_ERR, " CS");
      }
      else
	rul__ios_printf (RUL__C_STD_ERR, "   No TRACEs set");
      rul__ios_printf (RUL__C_STD_ERR, "\n"); 
   }
   else {
      if (on_flag) {
	 rul__dbg_gl_trace |= (long) p1;
	 if (rul__dbg_gl_trace & DBG_M_TRACE_RG)
	   rul__dbg_set_group ();
      }
      else {
	 rul__dbg_gl_trace &= ~((long) p1);
      }
   }
}

static void       cmd_wbreak (void)
{
   if (pl == (void *) -1)
     rul__dbg_show_wbreaks ();
   else {
      rul__dbg_wbreak (on_flag, pl);
      rul__dbg_free_param_list (pl);
      pl = NULL;
   }
   pat_pat = NULL;
}

static void       cmd_wm (void)
{
   Param_List p = pl;

   if (pl == NULL) {
      rul__wm_for_each_object_in_wm (rul__dbg_print); 
   }
   else {
      while (p) {
	 cur_wmo = rul__mol_instance_id_value ((Molecule) p->data);
	 if (cur_wmo == NULL)
	   rul__msg_print_w_atoms (CLI_NOSUCHWMO, 1, (Molecule) p->data);
	 else
	   rul__dbg_print (cur_wmo);
	 p = p->next;
      }
      rul__dbg_free_param_list (pl);
      pl = NULL;
   }
}

static void       cmd_wmhistory (void)
{
}


static void process_command (void)
{
   switch (dbg_command) {
    case DBG__E_CMD_AT:
      cmd_at ();
      break;
    case DBG__E_CMD_ADDSTATE:
      cmd_addstate ();
      break;
    case DBG__E_CMD_AFTER:
      cmd_after ();
      break;
    case DBG__E_CMD_CLOSEFILE:
      cmd_closefile ();
      break;
    case DBG__E_CMD_COPY:
      cmd_copy ();
      break;
    case DBG__E_CMD_CS:
      cmd_cs ();
      break;
    case DBG__E_CMD_DEFAULT:
      cmd_default ();
      break;
    case DBG__E_CMD_DISABLE:
      cmd_disable ();
      break;
    case DBG__E_CMD_EBREAK:
      cmd_ebreak ();
      break;
    case DBG__E_CMD_ENABLE:
      cmd_enable ();
      break;
    case DBG__E_CMD_EXIT:
      cmd_exit ();
      break;
    case DBG__E_CMD_MAKE:
      cmd_make ();
      break;
    case DBG__E_CMD_MATCHES:
      cmd_matches ();
      break;
    case DBG__E_CMD_MODIFY:
      cmd_modify ();
      break;
    case DBG__E_CMD_NEXT:
      cmd_next ();
      break;
    case DBG__E_CMD_OPENFILE:
      cmd_openfile ();
      break;
    case DBG__E_CMD_RBREAK:
      cmd_rbreak ();
      break;
    case DBG__E_CMD_PPCLASS:
      cmd_ppclass ();
      break;
    case DBG__E_CMD_PPWM:
      cmd_ppwm ();
      break;
    case DBG__E_CMD_PROMPT:
      cmd_prompt ();
      break;
    case DBG__E_CMD_QUIT:
      cmd_quit ();
      break;
    case DBG__E_CMD_REMOVE:
      cmd_remove ();
      break;
    case DBG__E_CMD_REMOVE_EVERY:
      cmd_remove_every ();
      break;
    case DBG__E_CMD_RESTORESTATE:
      cmd_restorestate ();
      break;
    case DBG__E_CMD_RETURN:
      cmd_return ();
      break;
    case DBG__E_CMD_RUN:
      cmd_run ();
      break;
    case DBG__E_CMD_SAVESTATE:
      cmd_savestate ();
      break;
    case DBG__E_CMD_SHOW:
      cmd_show ();
      break;
    case DBG__E_CMD_SPECIALIZE:
      cmd_specialize ();
      break;
    case DBG__E_CMD_TRACE:
      cmd_trace ();
      break;
    case DBG__E_CMD_WBREAK:
      cmd_wbreak ();
      break;
    case DBG__E_CMD_WM:
      cmd_wm ();
      break;
    case DBG__E_CMD_WMHISTORY:
      cmd_wmhistory ();
      break;
   }
}


static Boolean	SB_in_command = FALSE;
static long	SL_command_type = 0;
static char	SC_command_name[RUL_C_MAX_SYMBOL_SIZE+1];


/*****************
**              **
**  IN_COMMAND  **
**              **
*****************/

static Boolean in_command (void)
{
  return (SB_in_command);
}

/***********************
**                    **
**  GET_COMMAND_TYPE  **
**                    **
***********************/

static long get_command_type (void)
{
  return (SL_command_type);
}
/***********************
**                    **
**  SET_COMMAND_TYPE  **
**                    **
***********************/

static void set_command_type (long cmd_type)
{
  SL_command_type = cmd_type;
}



/***************************
**                        **
**  COMMAND_TYPE_TO_NAME  **
**                        **
***************************/

static char *command_type_to_name (RulDbg_Command cmd_type)
{
  switch (cmd_type) {
  case DBG__E_CMD_AT:
    return ("@"); break;
  case DBG__E_CMD_ADDSTATE:
    return ("ADDSTATE"); break;
  case DBG__E_CMD_AFTER:
    return ("AFTER"); break;
  case DBG__E_CMD_CLOSEFILE:
    return ("CLOSEFILE"); break;
  case DBG__E_CMD_COPY:
    return ("COPY"); break;
  case DBG__E_CMD_CS:
    return ("CS"); break;
  case DBG__E_CMD_DEFAULT:
    return ("DEFAULT"); break;
  case DBG__E_CMD_DISABLE:
    return ("DISABLE"); break;
  case DBG__E_CMD_EBREAK:
    return ("EBREAK"); break;
  case DBG__E_CMD_ENABLE:
    return ("ENABLE"); break;
  case DBG__E_CMD_EXIT:
    return ("EXIT"); break;
  case DBG__E_CMD_MAKE:
    return ("MAKE"); break;
  case DBG__E_CMD_MATCHES:
    return ("MATCHES"); break;
  case DBG__E_CMD_MODIFY:
    return ("MODIFY"); break;
  case DBG__E_CMD_NEXT:
    return ("NEXT"); break;
  case DBG__E_CMD_OPENFILE:
    return ("OPENFILE"); break;
  case DBG__E_CMD_RBREAK:
    return ("RBREAK"); break;
  case DBG__E_CMD_PPCLASS:
    return ("PPCLASS"); break;
  case DBG__E_CMD_PPWM:
    return ("PPWM"); break;
  case DBG__E_CMD_PROMPT:
    return ("PROMPT"); break;
  case DBG__E_CMD_QUIT:
    return ("QUIT"); break;
  case DBG__E_CMD_REMOVE:
    return ("REMOVE"); break;
  case DBG__E_CMD_REMOVE_EVERY:
    return ("REMOVE-EVERY"); break;
  case DBG__E_CMD_RESTORESTATE:
    return ("RESTORESTATE"); break;
  case DBG__E_CMD_RUN:
    return ("RUN"); break;
  case DBG__E_CMD_SAVESTATE:
    return ("SAVESTATE"); break;
  case DBG__E_CMD_SHOW:
    return ("SHOW"); break;
  case DBG__E_CMD_SPECIALIZE:
    return ("SPECIALIZE"); break;
  case DBG__E_CMD_TRACE:
    return ("TRACE"); break;
  case DBG__E_CMD_WBREAK:
    return ("WBREAK"); break;
  case DBG__E_CMD_WM:
    return ("WM"); break;
  case DBG__E_CMD_WMHISTORY:
    return ("WMHISTORY"); break;
  default:
    return (""); break;
  }
}



/***********************
**                    **
**  GET_USAGE_STRING  **
**                    **
***********************/

static char *get_usage_string (RulDbg_Command cmd)
{
   char *ret_str;

   switch (cmd) {
	case DBG__E_CMD_AT:		ret_str = CLI_USAGE_AT;  break;
	case DBG__E_CMD_ADDSTATE:	ret_str = CLI_USAGE_ADDSTATE;  break;
	case DBG__E_CMD_AFTER:		ret_str = CLI_USAGE_AFTER;  break;
	case DBG__E_CMD_CLOSEFILE:	ret_str = CLI_USAGE_CLOSEFILE;  break;
	case DBG__E_CMD_COPY:		ret_str = CLI_USAGE_COPY;  break;
	case DBG__E_CMD_CS:		ret_str = CLI_USAGE_CS;  break;
	case DBG__E_CMD_DEFAULT:	ret_str = CLI_USAGE_DEFAULT;  break;
	case DBG__E_CMD_DISABLE:	ret_str = CLI_USAGE_DISABLE;  break;
	case DBG__E_CMD_EBREAK:		ret_str = CLI_USAGE_EBREAK;  break;
	case DBG__E_CMD_ENABLE:		ret_str = CLI_USAGE_ENABLE;  break;
	case DBG__E_CMD_EXIT:		ret_str = CLI_USAGE_EXIT;  break;
	case DBG__E_CMD_MAKE:		ret_str = CLI_USAGE_MAKE;  break;
	case DBG__E_CMD_MATCHES:	ret_str = CLI_USAGE_MATCHES;  break;
	case DBG__E_CMD_MODIFY:		ret_str = CLI_USAGE_MODIFY;  break;
	case DBG__E_CMD_NEXT:		ret_str = CLI_USAGE_NEXT;  break;
	case DBG__E_CMD_OPENFILE:	ret_str = CLI_USAGE_OPENFILE;  break;
	case DBG__E_CMD_RBREAK:		ret_str = CLI_USAGE_RBREAK;  break;
	case DBG__E_CMD_PPCLASS:	ret_str = CLI_USAGE_PPCLASS;  break;
	case DBG__E_CMD_PPWM:		ret_str = CLI_USAGE_PPWM;  break;
	case DBG__E_CMD_PROMPT:		ret_str = "" /*?*/;  break;
	case DBG__E_CMD_QUIT:		ret_str = CLI_USAGE_QUIT;  break;
	case DBG__E_CMD_REMOVE:		ret_str = CLI_USAGE_REMOVE;  break;
	case DBG__E_CMD_REMOVE_EVERY:	ret_str = CLI_USAGE_REMOVE_EVERY;
					break;
	case DBG__E_CMD_RESTORESTATE:	ret_str = CLI_USAGE_RESTORESTATE;
					break;
	case DBG__E_CMD_RETURN:		ret_str = CLI_USAGE_RETURN;  break;
	case DBG__E_CMD_RUN:		ret_str = CLI_USAGE_RUN;  break;
	case DBG__E_CMD_SAVESTATE:	ret_str = CLI_USAGE_SAVESTATE;  break;
	case DBG__E_CMD_SHOW:		ret_str = "" /*?*/;  break;
	case DBG__E_CMD_SPECIALIZE:	ret_str = CLI_USAGE_SPECIALIZE;  break;
	case DBG__E_CMD_TRACE:		ret_str = CLI_USAGE_TRACE;  break;
	case DBG__E_CMD_WBREAK:		ret_str = CLI_USAGE_WBREAK;  break;
	case DBG__E_CMD_WM:		ret_str = CLI_USAGE_WM;  break;
	case DBG__E_CMD_WMHISTORY:	ret_str = CLI_USAGE_WMHISTORY;  break;
	case DBG__E_CMD_IS_BOGUS:	
	default:			ret_str = "";  break;
    }
    return ret_str;
}



/**************************
**                       **
**  GET_EXAMPLES_STRING  **
**                       **
**************************/

static char *get_examples_string (RulDbg_Command cmd)
{
   char *ret_str;

   switch (cmd) {
	case DBG__E_CMD_AT:		ret_str = CLI_EXAMP_AT;  break;
	case DBG__E_CMD_ADDSTATE:	ret_str = CLI_EXAMP_ADDSTATE;  break;
	case DBG__E_CMD_AFTER:		ret_str = CLI_EXAMP_AFTER;  break;
	case DBG__E_CMD_CLOSEFILE:	ret_str = CLI_EXAMP_CLOSEFILE;  break;
	case DBG__E_CMD_COPY:		ret_str = CLI_EXAMP_COPY;  break;
	case DBG__E_CMD_CS:		ret_str = CLI_EXAMP_CS;  break;
	case DBG__E_CMD_DEFAULT:	ret_str = CLI_EXAMP_DEFAULT;  break;
	case DBG__E_CMD_DISABLE:	ret_str = CLI_EXAMP_DISABLE;  break;
	case DBG__E_CMD_EBREAK:		ret_str = CLI_EXAMP_EBREAK;  break;
	case DBG__E_CMD_ENABLE:		ret_str = CLI_EXAMP_ENABLE;  break;
	case DBG__E_CMD_EXIT:		ret_str = CLI_EXAMP_EXIT;  break;
	case DBG__E_CMD_MAKE:		ret_str = CLI_EXAMP_MAKE;  break;
	case DBG__E_CMD_MATCHES:	ret_str = CLI_EXAMP_MATCHES;  break;
	case DBG__E_CMD_MODIFY:		ret_str = CLI_EXAMP_MODIFY;  break;
	case DBG__E_CMD_NEXT:		ret_str = CLI_EXAMP_NEXT;  break;
	case DBG__E_CMD_OPENFILE:	ret_str = CLI_EXAMP_OPENFILE;  break;
	case DBG__E_CMD_RBREAK:		ret_str = CLI_EXAMP_RBREAK;  break;
	case DBG__E_CMD_PPCLASS:	ret_str = CLI_EXAMP_PPCLASS;  break;
	case DBG__E_CMD_PPWM:		ret_str = CLI_EXAMP_PPWM;  break;
	case DBG__E_CMD_PROMPT:		ret_str = "" /*?*/;  break;
	case DBG__E_CMD_QUIT:		ret_str = CLI_EXAMP_QUIT;  break;
	case DBG__E_CMD_REMOVE:		ret_str = CLI_EXAMP_REMOVE;  break;
	case DBG__E_CMD_REMOVE_EVERY:	ret_str = CLI_EXAMP_REMOVE_EVERY;
					break;
	case DBG__E_CMD_RESTORESTATE:	ret_str = CLI_EXAMP_RESTORESTATE;
					break;
	case DBG__E_CMD_RETURN:		ret_str = CLI_EXAMP_RETURN;  break;
	case DBG__E_CMD_RUN:		ret_str = CLI_EXAMP_RUN;  break;
	case DBG__E_CMD_SAVESTATE:	ret_str = CLI_EXAMP_SAVESTATE;  break;
	case DBG__E_CMD_SHOW:		ret_str = "" /*?*/;  break;
	case DBG__E_CMD_SPECIALIZE:	ret_str = CLI_EXAMP_SPECIALIZE;  break;
	case DBG__E_CMD_TRACE:		ret_str = CLI_EXAMP_TRACE;  break;
	case DBG__E_CMD_WBREAK:		ret_str = CLI_EXAMP_WBREAK;  break;
	case DBG__E_CMD_WM:		ret_str = CLI_EXAMP_WM;  break;
	case DBG__E_CMD_WMHISTORY:	ret_str = CLI_EXAMP_WMHISTORY;  break;
	case DBG__E_CMD_IS_BOGUS:	
	default:			ret_str = "";  break;
    }
    return ret_str;
}



/***********************
**                    **
**  GET_HINTS_STRING  **
**                    **
***********************/

static char *get_hints_string (RulDbg_Command cmd)
{
   char *ret_str;

   switch (cmd) {
	case DBG__E_CMD_AT:		ret_str = CLI_HINT_AT;  break;
	case DBG__E_CMD_ADDSTATE:	ret_str = CLI_HINT_ADDSTATE;  break;
	case DBG__E_CMD_AFTER:		ret_str = CLI_HINT_AFTER;  break;
	case DBG__E_CMD_CLOSEFILE:	ret_str = CLI_HINT_CLOSEFILE;  break;
	case DBG__E_CMD_COPY:		ret_str = CLI_HINT_COPY;  break;
	case DBG__E_CMD_CS:		ret_str = CLI_HINT_CS;  break;
	case DBG__E_CMD_DEFAULT:	ret_str = CLI_HINT_DEFAULT;  break;
	case DBG__E_CMD_DISABLE:	ret_str = CLI_HINT_DISABLE;  break;
	case DBG__E_CMD_EBREAK:		ret_str = CLI_HINT_EBREAK;  break;
	case DBG__E_CMD_ENABLE:		ret_str = CLI_HINT_ENABLE;  break;
	case DBG__E_CMD_EXIT:		ret_str = CLI_HINT_EXIT;  break;
	case DBG__E_CMD_MAKE:		ret_str = CLI_HINT_MAKE;  break;
	case DBG__E_CMD_MATCHES:	ret_str = CLI_HINT_MATCHES;  break;
	case DBG__E_CMD_MODIFY:		ret_str = CLI_HINT_MODIFY;  break;
	case DBG__E_CMD_NEXT:		ret_str = CLI_HINT_NEXT;  break;
	case DBG__E_CMD_OPENFILE:	ret_str = CLI_HINT_OPENFILE;  break;
	case DBG__E_CMD_RBREAK:		ret_str = CLI_HINT_RBREAK;  break;
	case DBG__E_CMD_PPCLASS:	ret_str = CLI_HINT_PPCLASS;  break;
	case DBG__E_CMD_PPWM:		ret_str = CLI_HINT_PPWM;  break;
	case DBG__E_CMD_PROMPT:		ret_str = "" /*?*/;  break;
	case DBG__E_CMD_QUIT:		ret_str = CLI_HINT_QUIT;  break;
	case DBG__E_CMD_REMOVE:		ret_str = CLI_HINT_REMOVE;  break;
	case DBG__E_CMD_REMOVE_EVERY:	ret_str = CLI_HINT_REMOVE_EVERY;
					break;
	case DBG__E_CMD_RESTORESTATE:	ret_str = CLI_HINT_RESTORESTATE;
					break;
	case DBG__E_CMD_RETURN:		ret_str = CLI_HINT_RETURN;  break;
	case DBG__E_CMD_RUN:		ret_str = CLI_HINT_RUN;  break;
	case DBG__E_CMD_SAVESTATE:	ret_str = CLI_HINT_SAVESTATE;  break;
	case DBG__E_CMD_SHOW:		ret_str = "" /*?*/;  break;
	case DBG__E_CMD_SPECIALIZE:	ret_str = CLI_HINT_SPECIALIZE;  break;
	case DBG__E_CMD_TRACE:		ret_str = CLI_HINT_TRACE;  break;
	case DBG__E_CMD_WBREAK:		ret_str = CLI_HINT_WBREAK;  break;
	case DBG__E_CMD_WM:		ret_str = CLI_HINT_WM;  break;
	case DBG__E_CMD_WMHISTORY:	ret_str = CLI_HINT_WMHISTORY;  break;
	case DBG__E_CMD_IS_BOGUS:	
	default:			ret_str = "";  break;
    }
    return ret_str;
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
  rul__dbg_set_pp_pat (NULL);
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
      **	Command and post an appropriate error message
      */
{
  char buffer[100], err_msg[100];
  char *str, *cmd_str, *examp_str, *use_str;
  char in_msg[RUL_C_MAX_SYMBOL_SIZE*2+25];
  char token_str[RUL_C_MAX_SYMBOL_SIZE+25];
  char near_msg[RUL_C_MAX_SYMBOL_SIZE+25];
  Boolean dont_print_in = FALSE;
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

  case TOK_FLOAT_CONST:
    sprintf (buffer, "%f", val.fval);
    str = buffer;
    break;

  case TOK_INTEGER_CONST:
    sprintf (buffer, "%ld", val.ival);
    str = buffer;
    break;

  case TOK_INSTANCE_CONST:
    sprintf (buffer, "#%ld", val.ival);
    str = buffer;
    break;

  case TOK_OPAQUE_CONST :
    sprintf (buffer, "%%x%p", val.ival);
    str = buffer;
    break;

  default:
    str = val.sval;
  }

  if (str[0] == '\n') {
    strcpy (token_str, " missing required parameters");
    dont_print_in = TRUE;
  } else if (str[0] == '\0') {
    token_str[0] = '\0';
  } else {
    if (dbg_command == DBG__E_CMD_IS_BOGUS) {
      sprintf (token_str, " unknown command '%s'", str);
    } else {
      sprintf (token_str, " at or near '%s'", str);
    }
  }

  err_msg[0] = '\0';
  near_msg[0] = '\0';
  in_msg[0] = '\0';

  if (dbg_command != DBG__E_CMD_IS_BOGUS) {
    cmd_str = command_type_to_name (dbg_command);
    if (cmd_str[0] != '\0') {
      if (dont_print_in) sprintf (in_msg, " %s command", cmd_str);
      else sprintf (in_msg, " in %s command", cmd_str);
    }
  }

  if ((strcmp (msg, "Syntax error") == 0) ||
      (strcmp (msg, "parse error") == 0)  ||
      (strcmp (msg, "syntax error") == 0)) {
    err_msg[0] = '\0';
  } else {
    sprintf (err_msg, "%s", msg);
  }

  rul__msg_print (CLI_SYNTAXERR, err_msg, in_msg, token_str);
  if (dbg_command != DBG__E_CMD_IS_BOGUS) {
    use_str = get_usage_string (dbg_command);
    examp_str = get_examples_string (dbg_command);
    if (use_str[0] != '\0'  ||  examp_str[0] != '\0') {
	rul__ios_printf (RUL__C_STD_ERR, "%s%s", use_str, examp_str);
    }
  }
}
