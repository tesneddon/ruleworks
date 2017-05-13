/*
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

*/


/*
 * Values of non-constant tokens can be stored in variables of type
 * Token_Value.
 */
typedef union {
  unsigned char	*uval;	/* For [quoted-]symbols, [quoted-]variables */
  char		*sval;  /* same as above; this is easier than type-casting */
  long		ival;	/* For int-consts, instance-ids, and opaques */
  double	fval;	/* For float-consts */
} Token_Value;

typedef Token_Value	YYSTYPE;
typedef int             Token_Type;


/************************************************************************
** Functions ************************************************************
************************************************************************/

/*
 * Read the next token.  Return its type, and set parameter to value.
 * value.sval is a pointer to a string owned by the scan module.
 */
Token_Type rul__atom_get_atom         (Molecule *atom);
void       rul__atom_restart          (FILE *file_ptr);
void      *rul__atom_create_lex_buf   (void);
void       rul__atom_delete_lex_buf   (void *buf);
void       rul__atom_switch_lex_buf   (void *buf);

/*
 * setup the atomizer to "get" the next atom from a string buffer 
 */
void  rul__atom_string_setup (char *char_string);

/*
 * setup the atomizer to "get" the atom from a file 
 */
void  rul__atom_file_setup (FILE *fileptr);
void  rul__atom_file_close (void);
long  rul__atom_get_line_count (void);

#define	TOK_INTEGER_CONST	258
#define	TOK_FLOAT_CONST		259
#define	TOK_SYMBOL_CONST	260
#define	TOK_QUOTED_SYMBOL	261
#define	TOK_MIS_QUOTED_SYMBOL	262
#define	TOK_LPAREN		263
#define	TOK_RPAREN		264
#define	TOK_LBRACE		265
#define	TOK_RBRACE		266
#define	TOK_LBRACKET		267
#define	TOK_RBRACKET		268
#define	TOK_HAT			269
#define	TOK_COMPOUND		270
#define	TOK_INSTANCE_ID		271
#define	TOK_EOL			272
#define	TOK_EOF			273
#define TOK_OPAQUE_CONST	274
#define TOK_AMPERSAND           275
#define TOK_TILDE               276

