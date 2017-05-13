/* cmp_msg.c - Diagnostic message printing functions for the compiler */
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
**                                                                         **
****************************************************************************/


/*
 *  FACILITY:
 *	RULEWORKS compiler
 *
 *  ABSTRACT:
 *	Routines for printing compiler diagnostic messages, formatted
 *	according to operating system conventions.  All messages must 
 *	be defined in msg_mess.h.  Includes compiler messages, printed 
 *	to listing and with linenumber and filename to stderr, and
 *	non-compiler messages, simply printed to stderr.
 *
 *  Restrictions:
 *	Should respect users VMS MESSAGE settings /nofac/nosev/noid/notext,
 *	not yet implemented.
 *
 *	This method of storing messages (#define-ing the strings) is intended
 *	as a temporary solution.  The message text is stored in each module
 *	that uses the message, which is wasteful.  Each module must include
 *	msg_mess.h, which changes often requiring excessive
 *	recompilations.
 *	But when the implementation is redesigned, source code which calls
 *	these routines shouldn't need to change.
 *
 *  MODIFIED BY:
 *	DEC	Digital Equipment Corporation
 *	CPQ	Compaq Computer Corporation
 *
 *  MODIFICATION HISTORY:
 *
 *	 9-Sep-1992	DEC	Initial version
 *	 8-Dec-1992	DEC	Seperated compiler and rts messsage funcs
 *	01-Dec-1999	CPQ	Release with GPL
 */


#include <common.h>
#include <cmp_comm.h>
#include <msg.h>		/* Interface for this module */
#include <stdarg.h>
#include <stdio.h>		/* For sprintf(), vfprintf() */
#include <string.h>		/* For strchr() */
#include <ast.h>
#include <ios.h>
#include <mol.h>


static char cmp_msg_format_buffer [MSG__C_MAX_MESSAGE_LEN];




/*****************************
**                          **
**  GET_CMP_MESSAGE_FORMAT  **
**                          **
*****************************/

/*
 * Return the format string for a compiler message, including the filename
 * and linenumber.  String returned is statically allocated.
 */
static char *
get_cmp_message_format (char *message_str, long line_num)
{
    char *short_msg;
    Mol_Symbol file;
    char file_name[RUL_C_MAX_SYMBOL_SIZE+1];
    char *construct_string;
    extern char *rul__parser_get_construct_str (void);

    short_msg = rul___msg_get_message_format (message_str);
			/* Message w/o file & line */
    construct_string = rul__parser_get_construct_str();
			/*  '' or ' in RULE FOO-BAR'  */

    file = rul__ios_file_name_symbol (RUL__C_SOURCE);
    if (file != NULL) {
	rul__mol_use_printform (file, file_name, RUL_C_MAX_SYMBOL_SIZE);

	if (line_num != IOS__C_NOT_A_LINE_NUM) {

#ifdef __UNIX
            sprintf (cmp_msg_format_buffer, "%s:%ld: %s%s",
			file_name, line_num, short_msg, construct_string);
#else
            sprintf (cmp_msg_format_buffer, "%s, at line %ld%s in %s",
			short_msg, line_num, construct_string, file_name);
#endif /* __UNIX */

	} else  /* no line number was available */ {

#ifdef __UNIX
            sprintf (cmp_msg_format_buffer, "%s: %s%s",
			file_name, short_msg, construct_string);
#else
            strcpy (cmp_msg_format_buffer, short_msg);
	    strcat (cmp_msg_format_buffer, construct_string);

#endif /* __UNIX */

	}

    } else /* No current source file */  {

	strcpy (cmp_msg_format_buffer, short_msg);
    }
    return cmp_msg_format_buffer;
}




/*************************
**                      **
**  RUL__MSG_CMP_PRINT  **
**                      **
*************************/
/*
 * Print message with filename and linenumber to stderr and listing.
 */
void
rul__msg_cmp_print (char *message_str, Ast_Node ast, ...)
{
    va_list ap;
    char *big_message;		/* Message with linenumber & filename */
    char *small_message;	/* Message without linenumber & filename */
    char message_buffer[MSG__C_MAX_MESSAGE_LEN];
    long line_num;

    /* Print the message with linenumber and filename to stderr. */
    /* (If the listing is being printed to stderr, perhaps we shouldn't print
       this.) */

    line_num = rul__ast_nearest_line_number (ast);

    big_message = get_cmp_message_format (message_str, line_num);
    va_start (ap, ast);
    vsprintf (message_buffer, big_message, ap);
    rul__ios_printf (RUL__C_STD_ERR, "%s\n", message_buffer);
    va_end (ap);

    /* Print the message without linenumber and filename to the listing. */
    small_message = rul___msg_get_message_format (message_str);
    va_start (ap, ast);
    vsprintf (message_buffer, small_message, ap);
    rul__ios_printf (RUL__C_LISTING, "%s\n", message_buffer);
    va_end (ap);
}




/********************************
**                             **
**  RUL__MSG_CMP_PRINT_W_LINE  **
**                             **
********************************/

void
rul__msg_cmp_print_w_line (char *message_str, long line_num, ...)
{
    va_list ap;
    char *big_message;		/* Message with linenumber & filename */
    char *small_message;	/* Message without linenumber & filename */
    char message_buffer[MSG__C_MAX_MESSAGE_LEN];

    /* Print the message with linenumber and filename to stderr. */
    /* (If the listing is being printed to stderr, perhaps we shouldn't print
       this.) */
    big_message = get_cmp_message_format (message_str, line_num);
    va_start (ap, line_num);
    vsprintf (message_buffer, big_message, ap);
    rul__ios_printf (RUL__C_STD_ERR, "%s\n", message_buffer);
    va_end (ap);

    /* Print the message without linenumber and filename to the listing. */
    small_message = rul___msg_get_message_format (message_str);
    va_start (ap, line_num);
    vsprintf (message_buffer, small_message, ap);
    rul__ios_printf (RUL__C_LISTING, "%s\n", message_buffer);
    va_end (ap);
}




/*********************************
**                              **
**  RUL__MSG_CMP_PRINT_W_ATOMS  **
**                              **
*********************************/

void
rul__msg_cmp_print_w_atoms (Msg_Id msg_id, Ast_Node ast, long mol_count, ...)
{
    va_list ap;
    Mol_Atom mol_args[MSG__C_MAX_ATOMS];
    char mol_printforms[MSG__C_MAX_ATOMS][(RUL_C_MAX_SYMBOL_SIZE*2)+1];
    char comp_buf[(RUL_C_MAX_SYMBOL_SIZE*2)+1];
    long i, ccnt;

    assert (mol_count <= MSG__C_MAX_ATOMS);
    
    /*  First get all the molecules, and convert them into strings  */
    va_start (ap, mol_count);
    for (i=0; i<mol_count && i<MSG__C_MAX_ATOMS; i++) {
	mol_args[i] = va_arg (ap, Molecule);
	assert (rul__mol_is_valid(mol_args[i]));
	if (rul__mol_is_atom (mol_args[i]))
	  rul__mol_use_printform (mol_args[i], 
				  &(mol_printforms[i][0]),
				  RUL_C_MAX_SYMBOL_SIZE);
	else {
	  strcpy (&(mol_printforms[i][0]), "(COMPOUND");
	  ccnt = rul__mol_get_poly_count (mol_args[i]);
	  if (ccnt) {
	    rul__mol_use_printform (rul__mol_get_comp_nth (mol_args[i], 1), 
				    comp_buf,  RUL_C_MAX_SYMBOL_SIZE);
	    if (ccnt > 1)
	      strcat (comp_buf, " ...");
	  }
	  if (ccnt) {
	    strcat (&(mol_printforms[i][0]), " ");
	    strcat (&(mol_printforms[i][0]), comp_buf);
	  }
	  strcat (&(mol_printforms[i][0]), ")");
	}
    }
    va_end (ap);

    /*  Then call rul__msg_cmp_print with the appropriate args  */
    if (mol_count == 0) {
	rul__msg_cmp_print (msg_id, ast);
    } else if (mol_count == 1) {
	rul__msg_cmp_print (msg_id, ast, &(mol_printforms[0][0]));
    } else if (mol_count == 2) {
	rul__msg_cmp_print (msg_id, ast,
		&(mol_printforms[0][0]),
		&(mol_printforms[1][0]));
    } else if (mol_count == 3) {
	rul__msg_cmp_print (msg_id, ast,
		&(mol_printforms[0][0]),
		&(mol_printforms[1][0]),
		&(mol_printforms[2][0]));
    } else if (mol_count > 3) {
	rul__msg_cmp_print (msg_id, ast,
		&(mol_printforms[0][0]),
		&(mol_printforms[1][0]),
		&(mol_printforms[2][0]),
		&(mol_printforms[3][0]));
    }
}

