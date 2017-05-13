/* scan.h - External interface for RULEWORKS scanner */
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
**	RULEWORKS compiler and run-time system
**
**  ABSTRACT:
**	Scanner (tokenizer) interface
**
**  MODIFIED BY:
**	DEC	Digital Equipment Corporation
**	CPQ	Compaq Computer Corporation
**
**  MODIFICATION HISTORY:
**
**	19-Jun-1992	DEC	Initial version
**	21-Oct-1992	DEC	changes for interpreter
**	01-Dec-1999	CPQ	Release with GPL
*/

/*
 * Lex does all the input.  The parser calls rul__scan_get_token() to get the
 * next token.  It returns the token number, and sets arguments with the value
 * of the token (for non-constant tokens), and the line number.
 */

typedef int Token_Type;

/*
 * Values of non-constant tokens can be stored in variables of type
 * Token_Value.
 */
typedef union {
  unsigned char	*uval;	/* For [quoted-]symbols, [quoted-]variables */
  char		*sval;  /* same as above; this is easier than type-casting */
  long		ival;	/* For int-consts */
  double	fval;	/* For float-consts */
} Token_Value;


/************************************************************************
** Functions ************************************************************
************************************************************************/

/*
 * Read the next token.  Return its type, and set parameters to value,
 * and source line number.  value.sval is a pointer to a string owned
 * by the scan module.
 */
extern Token_Type
rul__scan_get_token (Token_Value *value, int *lineno);

extern Token_Type rul__scan_get_last_token (Token_Value *value);
void rul__scan_unget_tokens (long n);


/*
 * Initialize the scanner, call once for each file being scanned (including
 * first file).
 */
extern void
rul__scan_new_file (FILE *source_file);

/*
 * set the new prompt strint
 */
extern void
rul__scan_set_prompt(Molecule prompt);

