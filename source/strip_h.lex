/* ***************************************************************************

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

*************************************************************************** */

%{
#include <stdio.h>
#include <ver_msg.h>
#include <stdlib.h>

#define TRUE 1
#define FALSE 0

#ifndef fileno
extern int fileno(FILE *fp);
#endif
extern int strlen(const unsigned char *str);

static long debug_lex = 0;
static long include_it = FALSE;

%}

%x COMMENT

/* Cause yywrap to always return 1*/
%option noyywrap

%%
"/"\*		{	/* start of comment */
			BEGIN(COMMENT);
			if (debug_lex) printf ("\n  start of comment");
		}

<COMMENT>.	{	/* strip everything within comments */
			if (debug_lex > 1) 
				printf ("\n  toss comments (%s)", yytext);
		}

<COMMENT>"INCLUDE-IN-GEND.H" {
			include_it = TRUE;
		}

<COMMENT>"END-INCLUDE-IN-GEND.H" {
			include_it = FALSE;
		}

<COMMENT>\n	{
			if (debug_lex) printf ("\n  newline in comment");
		}

<COMMENT>\*"/"	{	/* end comment */
			BEGIN(INITIAL);
			if (debug_lex) printf ("\n  end of comment");
		}

\n+		{
			if (debug_lex) printf ("\n  newline(s)");
		    if (include_it) {
			fprintf (yyout, "\n");
		    }
		}

^[ \t]+\n+	{	/* blank line */
			if (debug_lex) printf ("\n  blank line");
		}

[ \t]+$		{	/* strip trailing white space */
			if (debug_lex) printf ("\n  trailing white space");
		}

[ \t]+		{	/* minimize other white space */
		    if (include_it) {
			fprintf (yyout, " ");
			if (debug_lex) printf ("\n  minimize white space");
		    }
		}

\014\n+		{	/* strip form feeds */
			if (debug_lex) printf ("\n  strip form feeds");
		}

[a-z_]+" "(\*)?[a-z_]+")"	|
[a-z_]+" "(\*)?[a-z_]+","	{
			/* strip argument names from protos */
		    if (include_it) {
			long i = 0;
			while (yytext[i] != ' ') i++;
			if (yytext[i+1] == '*') i+=2;
			yytext[i] = '\0';
			fprintf (yyout, "%s", yytext);
			yytext[i] = ' ';
			fprintf (yyout, "%c",
				yytext[strlen((unsigned char *)yytext)-1]);
			if (debug_lex) printf ("\n  strip proto arg names");
		    }
		}

.		{
		    if (include_it) {
			fprintf (yyout, "%s", yytext);
			if (debug_lex > 1) 
				printf ("\n  real stuff (%s)", yytext);
		    }
		}
%%

int main (long argc, char **argv)
{
	if (argc > 1) {
	    yyin = fopen (argv[1], "r");
	    if (yyin == NULL) {
		printf ("\nError:  Unable to open %s for reading", argv[1]);
		return (EXIT_FAILURE);
	    }
	} else {
	    yyin = stdin;
	}
	if (argc > 2) {
	    yyout = fopen (argv[2], "w");
	    if (yyout == NULL) {
		printf ("\nError:  Unable to open %s for writing", argv[2]);
		return (EXIT_FAILURE);
	    }
	} else {
	    yyout = stdout;
	}
	fprintf (yyout, "/*\n");
	fprintf (yyout, "    %s", CMP_VERSION);
	fprintf (yyout, "    %s", CMP_COPYRIGHT);
	fprintf (yyout, "    %s", CMP_INTERNAL_ONLY);
	fprintf (yyout, "*/");

	yylex();

	return (EXIT_SUCCESS);
}
