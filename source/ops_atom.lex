/* rts_scan_atomizer.lex - lex atom input for RuleWorks rts  */
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
**  FACILITY:
**	RULEWORKS runtime system
**
**  ABSTRACT:
**	Scanner (tokenizer) implementation
**	Used for ACCEPT-ATOM, ACCEPTLINE-COMPOUND and the
**	API routine RUL_MAKE_WME
**
**  MODIFIED BY:
**	DEC	Digital Equipment Corporation
**	CPQ	Compaq Computer Corporation
**
**  MODIFICATION HISTORY:
**
**	26-Jan-1993	DEC	Initial version (subset of rul_scan.lex)
**	15-Sep-1999	DEC	Add %option noyywrap
**	01-Dec-1999	CPQ	Release with GPL
*/

/*
 * This is the scanner/atomizer for RULEWORKS rts.
 */

%{
/*
 * C code here gets copied to the top of the generated scanner.
 */

#include <common.h>
#include <math.h>		/* For atol(), atof() */
#include <stdio.h>		/* For stdin */
#include <string.h>		/* For strncpy(), strcat() */
#include <mol.h>		/* declares rul__mol_get_printform etc.	    */
#include <msg.h>
#include <msg_mess.h>
#include <ios.h>		/* declares the ios functions */
#include <atom.h>		/* defines tokens and functions */
#include <ctype.h>		/* For toupper() */

extern Token_Type rul__atomizer (void);   /* yy_input */

static Token_Value yylval;	/* Token values for ints and floats are
				   stored here */

/* static variables for atom from string functions */

static IO_Stream      SA_string_ios = NULL;
				/* the ios for RUL__ATOM_STRING_SETUP  */
static char          *ST_str_buffer = NULL;
				/* pointer to data for string input    */
static char          *ST_str_pointer = NULL;
				/* pointer to current string input     */
static unsigned long  SL_str_buffer_len = 0;
				/* chars remaining for string input    */

static char *rul___string_input (char *buf, unsigned long maxlen,
			         void *opt1, void *opt2);

static IO_Stream      SA_file_ios = NULL;
				/* the ios for RUL__ATOM_FILE_SETUP  */
static FILE          *SA_file_ptr = NULL;
				/* the bizzare file pointer */

static char *rul___file_input (char *buf, unsigned long maxlen,
			       void *opt1, void *opt2);

static long           SL_line_count;

/*
 * Our code, which gets inserted into the routine yylex, returns
 * values of type Token_Type.  But Lex defines yylex to return "int"
 * by default.  To keep compilers quiet, we want yylex to return
 * Token_Type.  We can't fix it portably, but we can fix it for Flex.
 */

#ifdef YY_DECL
#undef YY_DECL
#endif
#define YY_DECL int rul__atomizer YY_PROTO(( void )) 

	/*
	**  The rts atomizer uses the IOS substream to get the input characters
	*/
#ifdef YY_INPUT
#undef YY_INPUT
#endif
#define YY_INPUT(buff,result,max_ch)					\
	if (rul__ios_fgets (RUL__C_ATOM,				\
		            (char *) buff,				\
			    (long) max_ch) 				\
		== NULL) {						\
	    result = YY_NULL;						\
	} else {							\
	    result = strlen ((char *) buff);				\
	}                                                               \
	if ((result != YY_NULL) && buff[result-1] == '\n') {            \
            buff[result++] = ' ';                                       \
	    buff[result] = 0;                                           \
        }

/* make these defines so this module can co-exist
 * (same olb) as the other lex module
 */
/*
#define yytext			 yy_atom_text
#define yyleng			 yy_atom_leng
#define yyin			 yy_atom_in
#define yyout			 yy_atom_out
#define yyrestart		 yy_atom_restart
#define yy_switch_to_buffer	 yy_atom_switch_buf
#define yy_load_buffer_state	 yy_atom_load_buf_state
#define yy_create_buffer	 yy_atom_create_buf
#define yy_delete_buffer	 yy_atom_delete_buf
#define yy_init_buffer		 yy_atom_init_buf
*/

%}

 /* Lex "definitions" go here, shorthand used in "rules" section. */
digit		[0-9]
hex_digit       [0-9a-fA-F]
sign		[-+]
exponent	([eE]{sign}?{digit}+)
 /* "|" is really nonmagic, but we don't treat it that way here. */
nonmagic	[a-zA-Z0-9_$\-:'.,<>`!@*+=\\/?#\200-\377]
bar		"|"
 /* Characters reserved for future use are &, ", %, and ~. */
reserved	"&"|\"|"%"|"~"
/* Bogus table sizes needed only by dumb versions of lex. */
%e 20000
%p 70000
%n 20000

/* Cause yywrap to always return 1 */
%option noyywrap

%%
 /****************************************************************************/
 /* RULES SECTION */

 /*
  * The "constant" tokens are sorted within groups, except for special
  * characters.
  */

"^"				return TOK_HAT;
"("				return TOK_LPAREN;
")"				return TOK_RPAREN;
"{"				return TOK_LBRACE;
"}"				return TOK_RBRACE;
"["				return TOK_LBRACKET;
"]"				return TOK_RBRACKET;
"&"				return TOK_AMPERSAND;
"~"				return TOK_TILDE;
COMPOUND			return TOK_COMPOUND;
\n		 		return TOK_EOL;
[ \t]				; /* Throw out whitespace */
";".*				; /* Throw out comments */

%{
/* Now come the non-constant tokens whose values vary */
%}

{sign}?{digit}*"."{digit}+{exponent}?	|
{sign}?{digit}+"."{exponent}	{
				return TOK_FLOAT_CONST;
				}

{sign}?{digit}+"."?		{
				return TOK_INTEGER_CONST;
				}

"#"{digit}+			{ /*skip the "#" and capture the number ([1])*/
				return TOK_INSTANCE_ID;
				}

"%"X{hex_digit}+		{
				return TOK_OPAQUE_CONST;
				}

{reserved}			{
				char buff[2];

				buff[0] = yytext[0];
				buff[1] = '\0';
				rul__msg_print (MSG_RESCHAR, buff);
				/* Don't return; ignore and get next token */
				}

{bar}([^\|\n]|{bar}{bar})*{bar}	{
				return TOK_QUOTED_SYMBOL;
				}

{bar}([^\|\n]|{bar}{bar})*	{
				/* Unmatched vertical bars... add one */

				rul__msg_print (MSG_QUOTEMISSING);
				strcat ( (char *) yytext, "|");
				return TOK_MIS_QUOTED_SYMBOL;
				}

{nonmagic}*			{
				/* for (i = 0; i < yyleng; i++) {   */
				/*   yytext[i] = toupper(yytext[i]); } */
				yylval.sval = yytext;
				return TOK_SYMBOL_CONST;
				}

[^\000]				{
				char buff[5];
				sprintf (buff, "\\%03d",
						(unsigned char) yytext[0]);
				rul__msg_print (MSG_CNTLCHAR, buff);
				}

\000				{
				return TOK_EOF;
				}

%%


/***************************
**                        **
**  RUL__ATOM_GET_ATOM    **
**                        **
***************************/

/*  This is the main entry point into this module */	

Token_Type
rul__atom_get_atom (Molecule *mol)
{
	Token_Type     tok_type;
	long           tok_len, i, j, hex_val, count;
	unsigned char *tok_text;
	unsigned char  tmp_sym [RUL_C_MAX_SYMBOL_SIZE + 1];

	/*
	**  Get the next token
	*/
        tok_type = rul__atomizer ();
	tok_text = (unsigned char *) yytext;
	tok_len  = strlen((char *) tok_text);

	/*
	**  Create the the token's value assignment
	*/
	switch (tok_type) {
	    case TOK_FLOAT_CONST:
		yylval.fval = atof ( (char *) tok_text);
		*mol = rul__mol_make_dbl_atom(yylval.fval);
		break;

	    case TOK_INTEGER_CONST:
		yylval.ival = atol ( (char *) tok_text);
		*mol = rul__mol_make_int_atom(yylval.ival);
		break;

	    case TOK_INSTANCE_ID:
		yylval.ival = atol ( (char *) &tok_text[1]);
		*mol = rul__mol_make_instance_atom(yylval.ival);
		break;

	    case TOK_OPAQUE_CONST:
		count = sscanf ((char *) &tok_text[2], "%x", &hex_val);
		assert (count == 1);
		yylval.ival = hex_val;
		*mol = rul__mol_make_opaque ((void *) hex_val);
		break;

	    case TOK_QUOTED_SYMBOL:
		/*
		**  Remove the enclosing vertical bars here...
		**  and reduce embedded double-bars to singles
		*/
		j = 0;
		tok_len = strlen((char *)tok_text)-1;
		for (i=1; i<tok_len; i++) {
		    if (tok_text[i] == '|'  
			&&  tok_text[i+1] == '|'
			&&  i + 1 < tok_len) 
		    {
			tmp_sym[j++] = '|';
			i++;
		    } else {
			tmp_sym[j++] = tok_text[i];
		    }
		}
		tmp_sym[j] = '\0';
		yylval.uval = tmp_sym;
		tok_type = TOK_SYMBOL_CONST;
		*mol = rul__mol_make_symbol(yylval.sval);
		break;

	    case TOK_MIS_QUOTED_SYMBOL:
		/* remove the leadng vertical bar here... */
		if (tok_len <= (RUL_C_MAX_SYMBOL_SIZE)) {
		  strncpy ((char *) tmp_sym,
			   (char *) tok_text+1,
			   tok_len-1);
		  tmp_sym[tok_len-1] = 0;
		}
		else {
		  strncpy ((char *) tmp_sym,
			   (char *) tok_text+1,
			   RUL_C_MAX_SYMBOL_SIZE);
		  tmp_sym[RUL_C_MAX_SYMBOL_SIZE] = 0;
		}
		yylval.uval = tmp_sym;
		tok_type = TOK_SYMBOL_CONST;
		*mol = rul__mol_make_symbol(yylval.sval);
		break;

	    case TOK_SYMBOL_CONST:
		if (tok_len < RUL_C_MAX_SYMBOL_SIZE) {
		  strcpy ((char *) tmp_sym, (char *) tok_text);
		}
		else {
		  strncpy ((char *) tmp_sym,
			   (char *) tok_text,
			   RUL_C_MAX_SYMBOL_SIZE);
		  tmp_sym[RUL_C_MAX_SYMBOL_SIZE] = 0;
		}
		for (i=0; i<tok_len; i++) {
		  tmp_sym[i] = toupper(tmp_sym[i]);
		}
		yylval.uval = tmp_sym;
		*mol = rul__mol_make_symbol(yylval.sval);
		break;

	    default:
		if (tok_type == 0) {
		    /*
		    **  YACC won't define TOK_EOF to be 0
		    **  even when we ask it to, so here we
		    **  arrange to return TOK_EOF on eof.
		    */
    		    tok_type = TOK_EOF;
		    *mol = NULL;
		}
		else {
		    yylval.uval = tok_text;
		    *mol = rul__mol_make_symbol(yylval.sval);
		}
	        break;
	
	} /* end switch */

#ifdef DEBUG_LEX_TOKENS
	if (tok_type == TOK_EOF) printf("\n  Token type = TOK_EOF");
	else printf("\n  Token type = %3d", tok_type);
#endif

	return (tok_type);
}

void rul__atom_restart (FILE *file)
{
  if (yy_current_buffer)
    yyrestart ((file == NULL) ? stdin : file);
}

void *rul__atom_create_lex_buf (void)
{
  return (void *) yy_create_buffer (stdin, YY_BUF_SIZE);
}

void rul__atom_delete_lex_buf (void *buf)
{
  if (buf)
    yy_delete_buffer ((YY_BUFFER_STATE) buf);
}

void rul__atom_switch_lex_buf (void *buf)
{
  if (buf)
    yy_switch_to_buffer ((YY_BUFFER_STATE) buf);
}



/*****************************
**			    **
**  RUL__ATOM_STRING_SETUP  **
**			    **
*****************************/

void rul__atom_string_setup (char *char_string)
{
	Mol_Symbol stream_name;

	if (SA_string_ios == NULL) {
	    /*  If this is the first time here, set up the string ios  */
	    stream_name = rul__mol_make_symbol ("$STRING-INPUT");
	    SA_string_ios = rul__ios_open_bizzare (stream_name,
					0, 0, 0,
					(Gets_Function) rul___string_input,
					0, 0);
	} else {
	    /*  If we've been here before, clear up the old trash  */
	    if (ST_str_buffer != NULL) 
		rul__mem_free (ST_str_buffer);
	}

	/*  set up the stuff for RUL___STRING_INPUT  */
	if (char_string != NULL) {
	  SL_str_buffer_len = strlen (char_string);
	  ST_str_buffer = (char *) rul__mem_malloc (SL_str_buffer_len + 1);
	  ST_str_pointer = ST_str_buffer;
	  strcpy (ST_str_buffer, char_string);
	}
	else
	  SL_str_buffer_len = 0;

	rul__ios_set_atom_stream (SA_string_ios);
	rul__atom_restart (NULL);
}


/*************************
**			**
**  RUL___STRING_INPUT	**
**			**
**************************/

static char *
rul___string_input (char *buf, unsigned long maxlen, void *opt1, void *opt2)
{
	long max_chars = MIN (maxlen, SL_str_buffer_len);

	if (SL_str_buffer_len) {
	    strncpy (buf, ST_str_pointer, max_chars);
	    ST_str_pointer += max_chars;
	    SL_str_buffer_len -= max_chars;
	    buf[max_chars] = '\0';
	    if (SL_str_buffer_len == 0) {
		rul__mem_free (ST_str_buffer);
	        ST_str_buffer = NULL;
	    }
	    return (buf);
  	}
  	return (NULL);
}




/*****************************
**			    **
**  RUL__ATOM_FILE_SETUP    **
**			    **
*****************************/

void rul__atom_file_setup (FILE *fileptr)
{
  Mol_Symbol stream_name;

  rul__atom_file_close ();
  SA_file_ptr = fileptr;
  SL_line_count = 0;

  if (SA_file_ios == NULL) {
    /*  If this is the first time here, set up the string ios  */
    stream_name = rul__mol_make_symbol ("$FILE-INPUT");
    SA_file_ios = rul__ios_open_bizzare (stream_name,
					 0, 0, 0,
					 (Gets_Function) rul___file_input,
					 0, 0);
  }

  SL_str_buffer_len = 0;
  ST_str_buffer = NULL;
  ST_str_pointer = ST_str_buffer;
  rul__ios_set_atom_stream (SA_file_ios);
  rul__atom_restart (NULL);
}

void rul__atom_file_close (void)
{
  if (SA_file_ptr) {
    fclose (SA_file_ptr);
    SA_file_ptr = NULL;
  }
}


/*************************
**			**
**  RUL___FILE_INPUT	**
**			**
**************************/

static char *
  rul___file_input (char *buf, unsigned long maxlen, void *opt1, void *opt2)
{
  long max_chars = MIN (maxlen, SL_str_buffer_len);
  char buffer[8192];
  char *cmp_str1 = (char *) opt1;
  char *cmp_str2 = (char *) opt2;
  char *buf_ptr;

  /* if nothing in buffer, get some data */

  if (SL_str_buffer_len == 0 && SA_file_ptr != NULL) {

    buf_ptr = fgets (buffer, 8192, SA_file_ptr);

    if (buf_ptr) {

      SL_str_buffer_len = strlen (buf_ptr);

      if (buf_ptr[SL_str_buffer_len-1] == '\n') {
	buf_ptr[--SL_str_buffer_len] = '\0';
	SL_line_count += 1;
      }

      if ((cmp_str1 && (strcmp (cmp_str1, buf_ptr) == 0)) ||
	  (cmp_str2 && (strcmp (cmp_str2, buf_ptr) == 0))) {
	SL_str_buffer_len = 0;
	return (NULL);
      }	

      ST_str_buffer = (char *) rul__mem_malloc (SL_str_buffer_len + 1);
      ST_str_pointer = ST_str_buffer;
      strcpy (ST_str_buffer, buf_ptr);
    }
  }

  /* buffer has data, pass it along... */

  if (SL_str_buffer_len) {
    max_chars = MIN (maxlen, SL_str_buffer_len);
    strncpy (buf, ST_str_pointer, max_chars);
    ST_str_pointer += max_chars;
    SL_str_buffer_len -= max_chars;
    buf[max_chars] = '\0';
    if (SL_str_buffer_len == 0) {
      rul__mem_free (ST_str_buffer);
      ST_str_buffer = NULL;
    }
    return (buf);
  }
  return (NULL);
}

long rul__atom_get_line_count (void)
{
  return SL_line_count;
}
