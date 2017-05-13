/* cmp_scan.lex - lex input for RULEWORKS scanner */
/****************************************************************************
RuleWorks - Rules based application development tool.

Copyright (C) 1999  Compaq Computer Corporation
Copyright (C) 2017  Endless Software Solutions

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
                                                                         **
****************************************************************************/


/*
**  FACILITY:
**	RULEWORKS compiler and runtime system
**
**  ABSTRACT:
**	Scanner (tokenizer) implementation
**	This source file generates rul_scan.c, which is compiled into either
**	cmp_scan or rts_scan_cmd_interp, depending on whether the preprocessor
**	macro SCAN_COMMAND_INTERPRETER is defined.
**
**  MODIFIED BY:
**	DEC	Digital Equipment Corporation
**	CPQ	Compaq Computer Corporation
**	ESS	Endless Software Solutions
**
**  MODIFICATION HISTORY:
**
**	19-Jun-1992	DEC	Initial version
**	21-Oct-1992	DEC	add interp tokens
**	20-Nov-1992	DEC	Conditionally compile language vs. command
**					interpreter scanner; clean up
**	30-Mar-1994	DEC	Rename yylval to tokval to avoid yacc conflict
**	15-Sep-1999	DEC	Add %option noyywrap
**	01-Dec-1999	CPQ	Release with GPL
**	14-May-2017	ESS	Remove YY_PROTO.
*/

/*
 * This is the scanner for RULEWORKS.
 *
 * Lexical errors are corrected (adding quote at end of line, ignoring
 * reserved characters and nulls) by the scanner, and a lexically correct
 * program is presented to the parser.
 *
 * Tokens larger than RUL_C_MAX_SYMBOL_SIZE can be returned.  Perhaps the
 * scanner should complain and truncate them?
 * 
 * Symbols aren't upcased.
 *
 * If we support %include, it should be handled entirely within this module,
 * but it isn't yet.
 *
 * This module doesn't yet depend on flex-specific features, but we probably
 * shouldn't spend much more effort avoiding them.
 *
 * Differences from V4:
 *
 *   The V4 scanner removes the "'"s from quoted-vars, and only calls them
 *   quoted-vars in RSE contexts.  This scanner always calls a quoted-var a
 *   quoted-var, and doesn't remove the quotes.
 *
 *   We don't allow bars in unquoted symbols, even though V4 with
 *   /obsolescent does allow it.  The line "now is|the time for" will
 *   scan as the three tokens "now", "is", and "|the time for|".
 *
 *   We don't interpret // and \\ operators.  (They're just symbols now.)
 *
 * Conditional Compilation:
 *
 *  To allow the command interpreter (debugger) and the language parsers
 *  to define different keyword tokens, we use start conditions and
 *  conditional compilation.  Rules starting with <dbg> are activated
 *  only in the scanner compiled for the command interpreter.  In addition
 *  those rules are conditionaly compiled so the tokens referenced in return
 *  statements don't need to be defined in the language parser.
 */

%{
/*
 * C code here gets copied to the top of the generated scanner.
 */

#include <common.h>
#include <math.h>		/* For atol(), atof() */
#include <string.h>		/* For strncpy(), strcat() */
#include <scan.h>		/* declares Token_Type and Token_Value etc. */
#include <mol.h>		/* declares rul__mol_get_printform etc.	    */
#include <ios.h>
#include <msg.h>
#include <msg_mess.h>

/* SCAN_COMMAND_INTERPRETER might not be defined.  We need a macro which
   can be used by non-preprocessor code, which will always have a value. */

#ifdef SCAN_COMMAND_INTERPRETER
#ifndef INTERP
#define INTERP 1
#endif
#endif
#ifdef INTERP
#define command_interpreter TRUE
#else
#define command_interpreter FALSE
#endif

#ifdef INTERP
#include <dbg.h>		/* debugger structs needed in dbg_tab.h     */
#include <dbg_tab.h>		/* YACC output defining cmd interp tokens   */
#include <cli_msg.h>

#define MAX_FPTR_STACK 10
static IO_Stream     fptr_stack[MAX_FPTR_STACK];
static int           fptr_stack_ptr = 0;
static unsigned int  interp_paren;	/* no-zero if left */
static char         *interp_prompt;	/* prompt string */
static Boolean       interp_cont;	/* command continuation */

#else

#include <cmp_comm.h>		/* Needed by ast.h */
#include <ast.h>		/* Needed by cmp_tab.h */
#include <cmp_tab.h>		/* YACC output defining language tokens	    */

#endif /* INTERP */


/*  Forward declarations  */
static void stash_token (Token_Type typ, unsigned char *str, long line);
static void unstash_token (Token_Type *typ, unsigned char **str, long *line);
static Boolean are_ungot_tokens (void);

static Token_Value tokval;	/* Token values for ints and floats are
				   stored here - traditionally would use
				   external yylval defined by yacc */
/*
 * Our code, which gets inserted into the routine yylex, returns
 * values of type Token_Type.  But Lex defines yylex to return "int"
 * by default.  To keep compilers quiet, we want yylex to return
 * Token_Type.  We can't fix it portably, but we can fix it for Flex.
 */
#ifdef FLEX_SCANNER
#undef YY_DECL
#define YY_DECL Token_Type yylex ( void ) 
#endif

#ifdef YY_INPUT
#undef YY_INPUT
#endif

#ifdef INTERP 
#ifdef __vms
#define YY_INPUT(buf,result,max_size)					\
  interp_cont = (interp_paren != 0);					\
  rul__ios_set_bizzare_args (RUL__C_SOURCE, 0, 0,                       \
			     (void **) &interp_prompt,                  \
			     (void **) &interp_cont);  		        \
  if (rul__ios_fgets (RUL__C_SOURCE,                                    \
		      (char *) buf,                                     \
		      (long) max_size) == NULL) {                       \
    result = YY_NULL;							\
  }									\
  else {								\
    result = strlen((char *) buf);					\
  }									\
  if (!interp_paren && 							\
      (result > 0) && buf[result-1] == '\n') { 				\
    buf[result++] = ' '; 						\
    buf[result] = 0; 							\
  }
#else
#define YY_INPUT(buf,result,max_size)					\
  if (RUL__C_SOURCE == RUL__C_STD_IN) {                                 \
    if (interp_paren != 0)						\
      rul__ios_printf (RUL__C_STD_OUT, "_");		                \
    rul__ios_printf (RUL__C_STD_OUT, interp_prompt);                    \
    rul__ios_flush (RUL__C_STD_OUT);                                    \
  }                                                                     \
  if (rul__ios_fgets (RUL__C_SOURCE,                                    \
		      (char *) buf,                                     \
		      (long) max_size) == NULL) {                       \
    result = YY_NULL;							\
  }									\
  else {								\
    result = strlen((char *) buf);					\
  }									\
  if (!interp_paren && 							\
      (result > 0) && buf[result-1] == '\n') { 				\
    buf[result++] = ' '; 						\
    buf[result] = 0; 							\
  }
#endif /* __vms */
#endif /* ifdef INTERP */


#ifndef INTERP
	/*
	**  The compiler uses the IOS substream to get the input characters
	*/
#define YY_INPUT(buff,result,max_ch)					\
	if ( rul__ios_fgets (RUL__C_SOURCE,				\
			     (char *) buff,				\
			     (long) max_ch ) 				\
		== NULL) {						\
	    result = YY_NULL;						\
	} else {							\
	    result = strlen ((char *) buff);				\
	}
#endif /* ifndef INTERP */

%}

 /* Lex "definitions" go here, shorthand used in "rules" section. */
digit		[0-9]
sign		[-+]
exponent	([eE]{sign}?{digit}+)
 /* "|" is really nonmagic, but we don't treat it that way here. */
nonmagic	[a-zA-Z0-9_$\-:'.,<>`!@*+=\\/?\200-\377]
hex_digit	[0-9a-fA-F]
bar		"|"

/* Bogus table sizes needed only by dumb versions of lex. */
%e 20000
%p 70000
%n 20000

%s dbg

 /* Characters reserved for future use are &, ", %, #, and ~. */
reserved	"&"|\"|"%"|"#"|"~"

/* Cause yywrap to always return 1*/
%option noyywrap


%%
 /****************************************************************************/
 /* RULES SECTION */

 /* Any actions preceding the first rule will be used for initialization
    of the scanner.  Here we turn on the "dbg" start condition if we're
    being compiled for the command interpreter. */
				if (command_interpreter)
				    BEGIN(dbg);

 /*
  * The "constant" tokens are sorted within groups, except for special
  * characters.
  */

 /***********************************************************************/
 /* Tokens used in both language and command interpreter		*/
 /***********************************************************************/
"("				{
#ifdef INTERP
				interp_paren += 1;
#endif
				return TOK_LPAREN;
				}

")"				{
#ifdef INTERP
				if (interp_paren > 0)
				  {
				    interp_paren -= 1;
				    return TOK_RPAREN;
				  } else {
				    rul__msg_print (CLI_UNMRPAREN);
				  }
#else
				return TOK_RPAREN;
#endif /* INTERP */
				}
"{"				return TOK_LBRACE;
"}"				return TOK_RBRACE;
"["				return TOK_LBRACKET;
"]"				return TOK_RBRACKET;
"^"				return TOK_HAT;

"<<"				return TOK_START_DISJUNCTION;
">>"				return TOK_END_DISJUNCTION;

"*"				return TOK_TIMES;
"/"				return TOK_DIVIDE;
"\\"				return TOK_MODULUS; /* quoted single \ */
"+"				return TOK_PLUS;
"-"				return TOK_MINUS;

"="				return TOK_EQUAL;
"=="				return TOK_EQUAL_EQUAL; /* identity */
"~="				return TOK_APPROX_EQUAL;
"-~="				return TOK_NOT_APPROX_EQUAL;
"<=>"				return TOK_SAME_TYPE;
"<->"				return TOK_DIFF_TYPE;
"<>"				return TOK_NOT_EQ;	/* non-identity */
"-="				return TOK_NOT_EQUAL;
"<"				return TOK_LESS;
"<="				return TOK_LESS_EQUAL;
">"				return TOK_GREATER;
">="				return TOK_GREATER_EQUAL;
"[+]"				return TOK_CONTAINS;
"[-]"				return TOK_DOES_NOT_CONTAIN;
"[<=]"				return TOK_LENGTH_LESS_EQUAL;
"[<>]"				return TOK_LENGTH_NOT_EQUAL;
"[<]"				return TOK_LENGTH_LESS;
"[=]"				return TOK_LENGTH_EQUAL;
"[>=]"				return TOK_LENGTH_GREATER_EQUAL;
"[>]"				return TOK_LENGTH_GREATER;

"-->"				return TOK_ARROW;
"@"				return TOK_AT;

"$LAST"				return TOK_DOLLAR_LAST;
"$FAILURE"			return TOK_FAILURE;
"$SUCCESS"			return TOK_SUCCESS;

 /* Following tokens are sorted alphabetically */
ACCEPT				return TOK_ACCEPT;
ACCEPT-ATOM			return TOK_ACCEPT_ATOM;
ACCEPTLINE-COMPOUND		return TOK_ACCEPTLINE_COMPOUND;
ACCEPTS				return TOK_ACCEPTS;
ACTIVATES			return TOK_ACTIVATES;
ADDSTATE			return TOK_ADDSTATE;
AFTER				return TOK_AFTER;
ALIAS-FOR			return TOK_ALIAS_FOR;
ANY				return TOK_ANY;
AND				return TOK_AND;
APPEND				return TOK_APPEND;
ASCID				return TOK_ASCID;
ASCIZ				return TOK_ASCIZ;
ATOM				return TOK_ATOM;
BIND				return TOK_BIND;
BY				return TOK_BY;
BYTE				return TOK_BYTE;
CALL-INHERITED			return TOK_CALL_INHERITED;
CATCH				return TOK_CATCH;
CLOSEFILE			return TOK_CLOSEFILE;
COMPOUND			return TOK_COMPOUND;
CONCAT				return TOK_CONCAT;
COPY				return TOK_COPY;
CRLF				return TOK_CRLF;
CS				return TOK_CS;
DEBUG				return TOK_DEBUG;
DECLARATION-BLOCK		return TOK_DECLARATION_BLOCK;
DEFAULT				return TOK_DEFAULT;
DOUBLE-FLOAT			return TOK_DOUBLE_FLOAT;
DO				return TOK_DO;
EB				return TOK_EB;
ELSE				return TOK_ELSE;
END-BLOCK			return TOK_END_BLOCK;
END-GROUP			return TOK_END_GROUP;
ENTRY-BLOCK			return TOK_ENTRY_BLOCK;
EVERY				return TOK_EVERY;
EXTERNAL-ROUTINE		return TOK_EXTERNAL_ROUTINE;
FILENAME			return TOK_FILENAME;
FILL				return TOK_FILL;
FLOAT				return TOK_FLOAT;
FOR-EACH			return TOK_FOR_EACH;
GENATOM				return TOK_GENATOM;
GENERIC-METHOD			return TOK_GENERIC_METHOD;
GENINT				return TOK_GENINT;
GET				return TOK_GET;
IF				return TOK_IF;
IN				return TOK_IN;
INHERITS-FROM			return TOK_INHERITS_FROM;
INSTANCE-ID			return TOK_INSTANCE;
INTEGER				return TOK_INTEGER;
IS-OPEN				return TOK_IS_OPEN;
LENGTH				return TOK_LENGTH;
LEX				return TOK_LEX;
LONG				return TOK_LONG;
MAKE				return TOK_MAKE;
MAX				return TOK_MAX;
MEA				return TOK_MEA;
METHOD				return TOK_METHOD;
MIN				return TOK_MIN;
MODIFY				return TOK_MODIFY;
NOT				return TOK_NOT;
NTH				return TOK_NTH;
OBJECT-CLASS			return TOK_OBJECT_CLASS;
OF				return TOK_OF;
OFF				return TOK_OFF;
ON				return TOK_ON;
ON-EMPTY			return TOK_ON_EMPTY;
ON-ENTRY			return TOK_ON_ENTRY;
ON-EVERY			return TOK_ON_EVERY;
ON-EXIT				return TOK_ON_EXIT;
OPAQUE				return TOK_OPAQUE;
OPENFILE			return TOK_OPENFILE;
OR				return TOK_OR;
OUT				return TOK_OUT;
P				return TOK_P;
PATHNAME			return TOK_PATHNAME;
POINTER				return TOK_POINTER;
POSITION			return TOK_POSITION;
QUIT				return TOK_QUIT;
READ-ONLY			return TOK_READ_ONLY;
READ-WRITE			return TOK_READ_WRITE;
REFERENCE			return TOK_REFERENCE;
REMOVE-EVERY			return TOK_REMOVE_EVERY;
REMOVE				return TOK_REMOVE;
RESTORESTATE			return TOK_RESTORESTATE;
RETURN				return TOK_RETURN;
RETURNS				return TOK_RETURNS;
RG				return TOK_RG;
RJUST				return TOK_RJUST;
RULE				return TOK_RULE;
RULE-BLOCK			return TOK_RULE_BLOCK;
RULE-GROUP			return TOK_RULE_GROUP;
SAVESTATE			return TOK_SAVESTATE;
SHORT				return TOK_SHORT;
SINGLE-FLOAT			return TOK_SINGLE_FLOAT;
SCALAR				return TOK_SCALAR;
SPECIALIZE			return TOK_SPECIALIZE;
SQL-ATTACH			return TOK_SQL_ATTACH;
SQL-COMMIT			return TOK_SQL_COMMIT;
SQL-DELETE			return TOK_SQL_DELETE;
SQL-DETACH			return TOK_SQL_DETACH;
SQL-FETCH-AS-OBJECT		return TOK_SQL_FETCH_AS_OBJECT;
SQL-FETCH-EACH			return TOK_SQL_FETCH_EACH;
SQL-INSERT			return TOK_SQL_INSERT;
SQL-INSERT-FROM-OBJECT		return TOK_SQL_INSERT_FROM_OBJECT;
SQL-ROLLBACK			return TOK_SQL_ROLLBACK;
SQL-START			return TOK_SQL_START;
SQL-UPDATE			return TOK_SQL_UPDATE;
SQL-UPDATE-FROM-OBJECT		return TOK_SQL_UPDATE_FROM_OBJECT;
STARTUP				return TOK_STARTUP;
STRATEGY			return TOK_STRATEGY;
SUBCOMPOUND			return TOK_SUBCOMPOUND;
SUBSYMBOL			return TOK_SUBSYMBOL;
SYMBOL				return TOK_SYMBOL;
SYMBOL-LENGTH			return TOK_SYMBOL_LENGTH;
TABTO				return TOK_TABTO;
THEN				return TOK_THEN;
TRACE				return TOK_TRACE;
UNSIGNED-BYTE			return TOK_UNSIGNED_BYTE;
UNSIGNED-LONG			return TOK_UNSIGNED_LONG;
UNSIGNED-SHORT			return TOK_UNSIGNED_SHORT;
USES				return TOK_USES;
VALUE				return TOK_VALUE;
WHEN				return TOK_WHEN;
WHILE				return TOK_WHILE;
WM				return TOK_WM;
WRITE				return TOK_WRITE;

%{
#ifdef INTERP
%}
 /***********************************************************************/
 /* Command interpreter specific tokens					*/
 /***********************************************************************/
<dbg>"&"			return TOK_AMPERSAND;
<dbg>"~"			return TOK_TILDE;
<dbg>ADDSTAT			return TOK_ADDSTATE;
<dbg>ADDSTA			return TOK_ADDSTATE;
<dbg>ADDST			return TOK_ADDSTATE;
<dbg>ADDS			return TOK_ADDSTATE;
<dbg>ADD			return TOK_ADDSTATE;
<dbg>AFTE			return TOK_AFTER;
<dbg>AFT			return TOK_AFTER;
<dbg>AF				return TOK_AFTER;
<dbg>BLOCK-NAME                 return TOK_BLOCK_NAME;
<dbg>BLOCK-NAM                  return TOK_BLOCK_NAME;
<dbg>BLOCK-NA                   return TOK_BLOCK_NAME;
<dbg>BLOCK-N                    return TOK_BLOCK_NAME;
<dbg>BLOCK                      return TOK_BLOCK_NAME;
<dbg>BLOC                       return TOK_BLOCK_NAME;
<dbg>BLO                        return TOK_BLOCK_NAME;
<dbg>BL                         return TOK_BLOCK_NAME;
<dbg>CLOSEFIL			return TOK_CLOSEFILE;
<dbg>CLOSEFI			return TOK_CLOSEFILE;
<dbg>CLOSEF			return TOK_CLOSEFILE;
<dbg>CLOSE			return TOK_CLOSEFILE;
<dbg>CLOS			return TOK_CLOSEFILE;
<dbg>CLO			return TOK_CLOSEFILE;
<dbg>CL 			return TOK_CLOSEFILE;
<dbg>CONTROL-C			return TOK_CTRLC;
<dbg>COP			return TOK_COPY;
<dbg>DEFAUL			return TOK_DEFAULT;
<dbg>DEFAU			return TOK_DEFAULT;
<dbg>DEFA			return TOK_DEFAULT;
<dbg>DEF			return TOK_DEFAULT;
<dbg>DISABLE			return TOK_DISABLE;
<dbg>DISABL			return TOK_DISABLE;
<dbg>DISAB			return TOK_DISABLE;
<dbg>DISA			return TOK_DISABLE;
<dbg>DIS			return TOK_DISABLE;
<dbg>EBREAK			return TOK_EBREAK;
<dbg>EBREA			return TOK_EBREAK;
<dbg>EBRE			return TOK_EBREAK;
<dbg>EBR			return TOK_EBREAK;
<dbg>ENABLE			return TOK_ENABLE;
<dbg>ENABL			return TOK_ENABLE;
<dbg>ENAB			return TOK_ENABLE;
<dbg>ENA			return TOK_ENABLE;
<dbg>EXCISE			return TOK_EXCISE;
<dbg>EXCIS			return TOK_EXCISE;
<dbg>EXCI			return TOK_EXCISE;
<dbg>EXC			return TOK_EXCISE;
<dbg>EXIT			return TOK_EXIT;
<dbg>EXI			return TOK_EXIT;
<dbg>EX				return TOK_EXIT;
<dbg>MAK			return TOK_MAKE;
<dbg>MATCHES			return TOK_MATCHES;
<dbg>MATCHE			return TOK_MATCHES;
<dbg>MATCH			return TOK_MATCHES;
<dbg>MATC			return TOK_MATCHES;
<dbg>MAT			return TOK_MATCHES;
<dbg>MODIF			return TOK_MODIFY;
<dbg>MODI			return TOK_MODIFY;
<dbg>MOD			return TOK_MODIFY;
<dbg>NEXT			return TOK_NEXT;
<dbg>NEX			return TOK_NEXT;
<dbg>NE				return TOK_NEXT;
<dbg>OPENFIL			return TOK_OPENFILE;
<dbg>OPENFI			return TOK_OPENFILE;
<dbg>OPENF			return TOK_OPENFILE;
<dbg>OPEN			return TOK_OPENFILE;
<dbg>OPE			return TOK_OPENFILE;
<dbg>OP 			return TOK_OPENFILE;
<dbg>PBREAK			return TOK_RBREAK;  /* obsolescent synonym */
<dbg>PBREA			return TOK_RBREAK;  /* obsolescent synonym */
<dbg>PBRE			return TOK_RBREAK;  /* obsolescent synonym */
<dbg>PBR			return TOK_RBREAK;  /* obsolescent synonym */
<dbg>PB				return TOK_RBREAK;  /* obsolescent synonym */
<dbg>PPCLASS			return TOK_PPCLASS;
<dbg>PPCLAS			return TOK_PPCLASS;
<dbg>PPCLA			return TOK_PPCLASS;
<dbg>PPCL			return TOK_PPCLASS;
<dbg>PPC			return TOK_PPCLASS;
<dbg>PPWM			return TOK_PPWM;
<dbg>PPW			return TOK_PPWM;
<dbg>PP 			return TOK_PPWM;
<dbg>PROMPT			return TOK_PROMPT;
<dbg>PROMP			return TOK_PROMPT;
<dbg>PROM			return TOK_PROMPT;
<dbg>PRO			return TOK_PROMPT;
<dbg>QUI			return TOK_QUIT;
<dbg>QU				return TOK_QUIT;
<dbg>RBREAK			return TOK_RBREAK;
<dbg>RBREA			return TOK_RBREAK;
<dbg>RBRE			return TOK_RBREAK;
<dbg>RBR			return TOK_RBREAK;
<dbg>RB				return TOK_RBREAK;
<dbg>REMOVE-EVER		return TOK_REMOVE_EVERY;
<dbg>REMOVE-EVE			return TOK_REMOVE_EVERY;
<dbg>REMOVE-EV			return TOK_REMOVE_EVERY;
<dbg>REMOVE-E			return TOK_REMOVE_EVERY;
<dbg>REMOVE-			return TOK_REMOVE_EVERY;
<dbg>REMOV			return TOK_REMOVE;
<dbg>REMO			return TOK_REMOVE;
<dbg>REM			return TOK_REMOVE;
<dbg>RESTART			return TOK_RESTART;
<dbg>RESTAR			return TOK_RESTART;
<dbg>RESTA			return TOK_RESTART;
<dbg>RESTORESTAT		return TOK_RESTORESTATE;
<dbg>RESTORESTA			return TOK_RESTORESTATE;
<dbg>RESTOREST			return TOK_RESTORESTATE;
<dbg>RESTORES			return TOK_RESTORESTATE;
<dbg>RESTORE			return TOK_RESTORESTATE;
<dbg>RESTOR			return TOK_RESTORESTATE;
<dbg>RESTO			return TOK_RESTORESTATE;
<dbg>RETUR			return TOK_RETURN;
<dbg>RETU			return TOK_RETURN;
<dbg>RET			return TOK_RETURN;
<dbg>RUN			return TOK_RUN;
<dbg>RU				return TOK_RUN;
<dbg>R				return TOK_RUN;
<dbg>RULES			return TOK_RULE;
<dbg>RUL			return TOK_RULE;
<dbg>SAVESTAT			return TOK_SAVESTATE;
<dbg>SAVESTA			return TOK_SAVESTATE;
<dbg>SAVEST			return TOK_SAVESTATE;
<dbg>SAVES			return TOK_SAVESTATE;
<dbg>SAVE			return TOK_SAVESTATE;
<dbg>SAV			return TOK_SAVESTATE;
<dbg>SA 			return TOK_SAVESTATE;
<dbg>SPECIALIZ			return TOK_SPECIALIZE;
<dbg>SPECIALI			return TOK_SPECIALIZE;
<dbg>SPECIAL			return TOK_SPECIALIZE;
<dbg>SPECIA			return TOK_SPECIALIZE;
<dbg>SPECI			return TOK_SPECIALIZE;
<dbg>SPEC			return TOK_SPECIALIZE;
<dbg>SPE			return TOK_SPECIALIZE;
<dbg>TRAC			return TOK_TRACE;
<dbg>TRA			return TOK_TRACE;
<dbg>TR				return TOK_TRACE;
<dbg>VERSION			return TOK_VERSION;
<dbg>VERSIO			return TOK_VERSION;
<dbg>VERSI			return TOK_VERSION;
<dbg>VERS			return TOK_VERSION;
<dbg>VER			return TOK_VERSION;
<dbg>WARNING			return TOK_WARNING;
<dbg>WARNIN			return TOK_WARNING;
<dbg>WARNI			return TOK_WARNING;
<dbg>WARN			return TOK_WARNING;
<dbg>WAR			return TOK_WARNING;
<dbg>WA				return TOK_WARNING;
<dbg>WATCH			return TOK_TRACE;  /* obsolescent synonym */
<dbg>WATC			return TOK_TRACE;  /* obsolescent synonym */
<dbg>WAT			return TOK_TRACE;  /* obsolescent synonym */
<dbg>WBREAK			return TOK_WBREAK;
<dbg>WBREA			return TOK_WBREAK;
<dbg>WBRE			return TOK_WBREAK;
<dbg>WBR			return TOK_WBREAK;
<dbg>WB				return TOK_WBREAK;
<dbg>WMHISTORY			return TOK_WMHISTORY;
<dbg>WMHISTOR			return TOK_WMHISTORY;
<dbg>WMHISTO			return TOK_WMHISTORY;
<dbg>WMHIST			return TOK_WMHISTORY;
<dbg>WMHIS			return TOK_WMHISTORY;
<dbg>WMHI			return TOK_WMHISTORY;
<dbg>WMH			return TOK_WMHISTORY;
<dbg>WRIT			return TOK_WRITE;
<dbg>WRI			return TOK_WRITE;
%{
#endif /* INTERP */
%}

\n				{
#ifdef INTERP
				if (interp_paren == 0)
				  return TOK_EOL;
#endif
				}

[ \t\013\014\015]		; /* Throw out whitespace */
";".*				; /* Throw out comments */

	/* Now come the non-constant tokens whose values vary */

{sign}?{digit}*"."{digit}+{exponent}?	|
{sign}?{digit}+"."{exponent}	{
				return TOK_FLOAT_CONST;
				}

{sign}?{digit}+"."?		{
				return TOK_INTEGER_CONST;
				}

"#"{digit}+			{
				return TOK_INSTANCE_CONST;
				}

"%"X{hex_digit}+		{
				return TOK_OPAQUE_CONST;
				}

"<"{nonmagic}+">"		{
				return TOK_VARIABLE;
				}

"'<"{nonmagic}+">'"		{
				tokval.sval = yytext;
				return TOK_QUOTED_VAR;
				}

{reserved}			{
				char buff[2];

				buff[0] = yytext[0];
				buff[1] = '\0';
#ifdef INTERP
				rul__msg_print (MSG_RESCHAR, buff);
#else
				rul__msg_cmp_print_w_line (
					MSG_RESCHAR,
					rul__ios_curr_line_num (RUL__C_SOURCE),
					buff);
#endif
				/* Don't return; ignore and get next token */
				}

{bar}([^\|\n]|{bar}{bar})*{bar}	{
				return TOK_QUOTED_SYMBOL;
				}

{bar}([^\|\n]|{bar}{bar})*	{
				/* Unmatched vertical bars... */
#ifdef INTERP
				rul__msg_print (MSG_QUOTEMISSING);
#else
				rul__msg_cmp_print_w_line (
				     MSG_QUOTEMISSING, 
				     rul__ios_curr_line_num (RUL__C_SOURCE));
#endif
				strcat ( (char *) yytext, "|");
				return TOK_QUOTED_SYMBOL;
				}

{nonmagic}*			{
				/* for (i = 0; i < yyleng; i++) {   */
				/*   yytext[i] = toupper(yytext[i]); } */
				tokval.sval = yytext;
				return TOK_SYMBOL_CONST;
				}

[\000-\377]			{
				/*  Rule of last resort  */
				char buff[5];
				sprintf (buff, "\\%03d",
					 (unsigned char) yytext[0]);
#ifdef INTERP
				if (yytext[0] != 0)
				    rul__msg_print (MSG_CNTLCHAR, buff);
#else
				rul__msg_cmp_print_w_line (
					MSG_CNTLCHAR,
					rul__ios_curr_line_num (RUL__C_SOURCE),
					buff);
#endif
				}

%%

#ifdef INTERP


/****************************
**                         **
**  RUL__SCAN_SWITCH_FILE  **
**                         **
****************************/

Boolean
rul__scan_switch_file (IO_Stream new_ios)
{
  if (new_ios) {
    if (fptr_stack_ptr >= MAX_FPTR_STACK) {
      rul__ios_printf(RUL__C_STD_ERR,
		"%%Maximum file nesting of %d exceeded\n", MAX_FPTR_STACK);
    }
    else {
      fptr_stack[fptr_stack_ptr] = RUL__C_SOURCE;
      fptr_stack[++fptr_stack_ptr] = new_ios;
      rul__ios_set_source_stream(new_ios);
      YY_NEW_FILE;
    }
  }
  else {
    if (fptr_stack_ptr > 0) {
      rul__ios_close_file(
		rul__ios_stream_name_symbol(
			fptr_stack[fptr_stack_ptr]));
      rul__ios_set_source_stream(fptr_stack[--fptr_stack_ptr]);
      YY_NEW_FILE;
    }
    else {
      rul__ios_set_source_stream(RUL__C_STD_IN);
      return FALSE;
    }
  }
  return TRUE;
}

void
rul__scan_reset_parens (void)
{
    interp_paren = 0;
}

void
rul__scan_set_prompt (Molecule prompt)
{
  if (interp_prompt)
    rul__mem_free(interp_prompt);
  interp_prompt = rul__mol_get_printform(prompt);
}
#endif


Token_Type	SL_last_token = 0;
Token_Value	SA_last_token_value;


/*******************************
**                            **
**  RUL__SCAN_GET_LAST_TOKEN  **
**                            **
*******************************/

Token_Type
rul__scan_get_last_token (Token_Value *value)
{
	*value = SA_last_token_value;
	return (SL_last_token);
}




/**************************
**                       **
**  RUL__SCAN_GET_TOKEN  **
**                       **
**************************/

Token_Type
rul__scan_get_token (Token_Value *value, int *lineno)
     /*
      **  This is the main entry point into this module 
      */
{
  Token_Type tok_type;
  long line_number = -1;
  long tok_line, tok_len, i, j, len;
  unsigned char *tok_text;
  long count, hex_val;
  static unsigned char unquoted_symbol [RUL_C_MAX_SYMBOL_SIZE + 1];

  
  /*
   **  Get the next token, either from the 'ungot' list,
   **  or from the lexical analyzer itself.
   */
  if (are_ungot_tokens()) {
    unstash_token (&tok_type, &tok_text, &tok_line);
    if (tok_line != line_number) line_number = tok_line;
    
  }
  else {
    tok_type = yylex ();
    tok_text = (unsigned char *) yytext;
    tok_len  = strlen((char *) tok_text);
    line_number = rul__ios_curr_line_num (RUL__C_SOURCE);
    tok_line = line_number;
  }

  /*
   **  Create the the token's value assignment
   */
  switch (tok_type) {
  case TOK_FLOAT_CONST:
    tokval.fval = atof ( (char *) tok_text);
    break;
    
  case TOK_INTEGER_CONST:
    tokval.ival = atol ( (char *) tok_text);
    break;
    
  case TOK_INSTANCE_CONST:
    tokval.ival = atol ( (char *) &tok_text[1]);
    break;
    
  case TOK_OPAQUE_CONST:
    count = sscanf ((char *) &tok_text[2], "%x", &hex_val);
    assert (count == 1);
    tokval.ival = hex_val;
    break;
    
  case TOK_QUOTED_SYMBOL:
    /*
     **  Remove the enclosing vertical bars here...
     **  and reduce embedded double-bars to singles
     */
    j = 0;
    len = strlen((char *)tok_text)-1;
    if (len > RUL_C_MAX_SYMBOL_SIZE - 1)
      len = RUL_C_MAX_SYMBOL_SIZE - 2;
    for (i = 1; i < len; i++) {
      if (tok_text[i] == '|' &&  tok_text[i+1] == '|' &&  i + 1 < len) {
	unquoted_symbol[j++] = '|';
	i++;
      }
      else {
	unquoted_symbol[j++] = tok_text[i];
      }
    }
    unquoted_symbol[j] = '\0';
    if (tok_len > RUL_C_MAX_SYMBOL_SIZE - 1) {
      rul__msg_print (MSG_STRTRUNCD, "Quoted symbol", unquoted_symbol);
    }
    tokval.uval = unquoted_symbol;
    tok_len = j;
    break;
    
  case TOK_VARIABLE:
    tokval.uval = tok_text;
    if (tok_len > RUL_C_MAX_SYMBOL_SIZE) {
      tok_len = RUL_C_MAX_SYMBOL_SIZE;
      strncpy ((char *)unquoted_symbol, (char *) tok_text,  tok_len);
      unquoted_symbol[tok_len-1] = '>';
      unquoted_symbol[tok_len] = '\0';
      tokval.uval = unquoted_symbol;
      rul__msg_print (MSG_STRTRUNCD, "Variable", unquoted_symbol);
    }
    break;

  case TOK_QUOTED_VAR:
    tokval.uval = tok_text;
    if (tok_len > RUL_C_MAX_SYMBOL_SIZE) {
      tok_len = RUL_C_MAX_SYMBOL_SIZE;
      strncpy ((char *)unquoted_symbol, (char *) tok_text,  tok_len);
      unquoted_symbol[tok_len-2] = '>';
      unquoted_symbol[tok_len-1] = '\'';
      unquoted_symbol[tok_len] = '\0';
      tokval.uval = unquoted_symbol;
      rul__msg_print (MSG_STRTRUNCD, "Quoted variable", unquoted_symbol);
    }
    break;

  case TOK_SYMBOL_CONST:
    tokval.uval = tok_text;
    if (tok_len > RUL_C_MAX_SYMBOL_SIZE) {
      tok_len = RUL_C_MAX_SYMBOL_SIZE;
      strncpy ((char *)unquoted_symbol, (char *) tok_text,  tok_len);
      unquoted_symbol[tok_len] = '\0';
      tokval.uval = unquoted_symbol;
      rul__msg_print (MSG_STRTRUNCD, "Symbol", unquoted_symbol);
    }
    break;
    
  default:
    if (tok_type == 0) {
      
#ifdef INTERP
      /*
       **  YACC won't define TOK_EOF to be 0
       **  even when we ask it to, so here we
       **  arrange to return TOK_EOF on eof.
       */
      tok_type = TOK_EOF;
#endif
    } else {
      tokval.uval = tok_text;
    }
  } /* end switch */
  
  SA_last_token_value = tokval;
  SL_last_token = tok_type;
  stash_token (tok_type, tok_text, tok_line);
  
  *value = tokval;
  *lineno = tok_line;
  return (tok_type);
}



#ifdef SCANTEST
/*
 * Code for interactively testing the scanner.  This module can be compiled
 * by itself by #defining SCANTEST, and then linked and run for testing.
 */
main()
{
  Token_Value val;
  int line;
  Token_Type token_type;

  rul__mol_init ();
  rul__ios_init ();

  rul__ios_set_source_stream (RUL__C_STD_IN);

  do {
    token_type = rul__scan_get_token (&val, &line);
    rul__ios_printf (RUL__C_STD_ERR, "\t%d\tLine %d\n", token_type, line);

    switch (token_type) {
    case TOK_FLOAT_CONST:
      rul__ios_printf (RUL__C_STD_ERR, "\t\t%f\n", val.fval);
      break;
    case TOK_INTEGER_CONST:
      rul__ios_printf (RUL__C_STD_ERR, "\t\t%ld\n", val.ival);
      break;
    case TOK_INSTANCE_CONST:
      rul__ios_printf (RUL__C_STD_ERR, "\t\t#%ld\n", val.ival);
      break;
    case TOK_OPAQUE_CONST:
      rul__ios_printf (RUL__C_STD_ERR, "\t\t%%x%p\n", val.ival);
      break;
    case TOK_EOF:
    case 0:
      return;
      break;
    case TOK_VARIABLE:
    case TOK_QUOTED_VAR:
    case TOK_QUOTED_SYMBOL:
    case TOK_SYMBOL_CONST:
      rul__ios_printf (RUL__C_STD_ERR, "\t\t%s\n", val.uval);
    }
  } while (token_type != TOK_EOF  &&  token_type != 0);
}
#endif


/*
**  The following are all private for the token stack
*/

#define TOKEN_UNGET_LIMIT 4

#define UNGET_WRAPUP -3

static Token_Type  	SL_prev_token_types[TOKEN_UNGET_LIMIT];
static unsigned char   *SA_prev_token_strs[TOKEN_UNGET_LIMIT];
static long	 	SL_prev_token_lines[TOKEN_UNGET_LIMIT];

static unsigned char	ST_prev_buffer[TOKEN_UNGET_LIMIT]
					[RUL_C_MAX_SYMBOL_SIZE+1];
static long	  	SL_num_stashed = 0;
static long	   	SL_next_index = 0;

static long	   	SL_num_ungot = 0;




/*****************************
**                          **
**  RUL__SCAN_UNGET_TOKENS  **
**                          **
*****************************/

void
rul__scan_unget_tokens (long n)
{
	assert (n < SL_num_stashed);

	SL_num_ungot = MIN (SL_num_ungot + n, SL_num_stashed);
}



/***********************
**                    **
**  ARE_UNGOT_TOKENS  **
**                    **
***********************/

static Boolean
are_ungot_tokens (void)
{
	return (SL_num_ungot > 0);
}



/******************
**               **
**  STASH_TOKEN  **
**               **
******************/

static void
stash_token (Token_Type typ, unsigned char *str, long line)
{
	long i;

	if (SL_num_ungot == UNGET_WRAPUP) {
	    SL_num_ungot = 0;
	    return;
	}
	if (SL_num_ungot > 0) return;

	for (i=1; i<TOKEN_UNGET_LIMIT; i++) {
	    SL_prev_token_types[i-1] 	= SL_prev_token_types[i];
	    SA_prev_token_strs[i-1] 	= SA_prev_token_strs[i];
	    SL_prev_token_lines[i-1] 	= SL_prev_token_lines[i];
	}
	SL_prev_token_types[TOKEN_UNGET_LIMIT-1] = typ;
	SL_prev_token_lines[TOKEN_UNGET_LIMIT-1] = line;
	SA_prev_token_strs[TOKEN_UNGET_LIMIT-1] = 
				&(ST_prev_buffer[SL_next_index][0]);
	strncpy ((char *) &(ST_prev_buffer[SL_next_index][0]),
		(char *) str, RUL_C_MAX_SYMBOL_SIZE);
	ST_prev_buffer[SL_next_index][RUL_C_MAX_SYMBOL_SIZE] = '\0';
	SL_next_index = (SL_next_index + 1) % TOKEN_UNGET_LIMIT;
	SL_num_stashed = MIN (SL_num_stashed + 1, TOKEN_UNGET_LIMIT);
}

#ifndef NDEBUG

static void
show_stash ()
{
	long i;

	rul__ios_printf (RUL__C_STD_ERR, "\n  Stash --");
	for (i=0; i<SL_num_stashed; i++) {
	    rul__ios_printf (RUL__C_STD_ERR, 
			"\n    SA_prev_token_strs[%ld] = '%s'",
			i, SA_prev_token_strs[i]) ;
	}
	rul__ios_printf (RUL__C_STD_ERR, "\n");
}
#endif



/********************
**                 **
**  UNSTASH_TOKEN  **
**                 **
********************/

static void
unstash_token (Token_Type *typ, unsigned char **str, long *line)
{
	long i;

	assert (SL_num_stashed > 0  &&  SL_num_ungot > 0);

	i = TOKEN_UNGET_LIMIT - SL_num_ungot;	
	*typ  = SL_prev_token_types[i];
	*str  = SA_prev_token_strs[i];
	*line = SL_prev_token_lines[i];

	SL_num_ungot --;
	if (SL_num_ungot == 0) {
	    /* trigger wrap-up */
	    SL_num_ungot = UNGET_WRAPUP; 
	}
}
