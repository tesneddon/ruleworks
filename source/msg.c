/* Diagnostic message printing functions */
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
 *  FACILITY:
 *	RULEWORKS run time system and compiler
 *
 *  ABSTRACT:
 *	Routines for printing diagnostic messages, formatted according
 *	to operating system conventions.  All messages must be defined
 *	in msg_mess.h.  Includes compiler messages, printed to
 *	listing and with linenumber and filename to stderr, and
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
**	CPQ	Compaq Computer Corporation
 *
 *  MODIFICATION HISTORY:
 *
 *	 9-Sep-1992	DEC	Initial version
 *	 8-Dec-1992	DEC	Converted to IO_Stream's
 *	01-Dec-1999	CPQ	Release with GPL
 */


#include <common.h>
#include <msg.h>		/* Interface for this module */
#include <stdarg.h>
#include <stdio.h>		/* For sprintf(), vfprintf() */
#include <string.h>		/* For strchr() */
#include <ios.h>
#include <mol.h>
#include <ver_msg.h>

static char     msg_format_buffer [MSG__C_MAX_MESSAGE_LEN];
static long     severe_errors = 0;
static long     warning_errors = 0;
static long     info_errors = 0;
static Boolean  messages_enabled = TRUE;
static Boolean  in_debugger = FALSE;



#ifndef __VMS

/*******************
**                **
**  SEVERITY_STR  **
**                **
*******************/

/* Translate cryptic single-character severity into English for non-VMS. */
static char *
severity_str (char c)
{
    switch (c) {
      case 'I': return "informational";
      case 'W': return "warning";
      case 'E': return "error";
      case 'F': return "fatal";
    }
    return "?";
}
#endif /* not __VMS */




/***********************************
**                                **
**  RUL___MSG_GET_MESSAGE_FORMAT  **
**                                **
***********************************/

/*
 * Return a printf format string for the given message, formatted for the
 * proper operating system.  Returned string is statically allocated.
 */
char *
rul___msg_get_message_format (char *message_str)
{
    /* message_str example: "RESCHAR E Reserved character \"%c\" ignored" */
    char *severity;		/* Pointer to severity character */
    char *text;			/* pointer to text */
#ifdef __VMS
    char *id;			/* Pointer to id */
    unsigned id_length;		/* Number of chars in id */
#endif

    severity = strchr (message_str, ' ') + 1; /* First char after space */
    text = severity + 2;	/* One char for severity, one for space */

#ifdef __VMS
    id = message_str;		/* string starts with ID */
    id_length = strchr (id, ' ') - id;
    assert (id_length <= MSG__C_LONGEST_MSGID);

    /*
    **  Since this format string will be passed first to sprintf,
    **	then vfprintf,
    **  the %s must be doubled twice. 
    */
    sprintf (msg_format_buffer,
			"%%%%%s-%c-%.*s, %s",
			RUL__C_MSG_PREFIX,
			*severity,
			id_length,
			id,
			text);
#else
    sprintf (msg_format_buffer,
			"%s: %s",
			severity_str (*severity),
			text);
#endif /* __VMS */

    if (severity[0] == 'E' ||
	severity[0] == 'e' ||
	severity[0] == 'F' ||
	severity[0] == 'f')
      severe_errors += 1;

    else if (severity[0] == 'W' ||
	     severity[0] == 'w')
      warning_errors += 1;

    else if (severity[0] == 'I' ||
	     severity[0] == 'i' ||
	     severity[0] == 'S' ||
	     severity[0] == 's')
      info_errors += 1;
      
    return msg_format_buffer;
}



void rul__msg_set_in_debugger (Boolean flag)
{
  in_debugger = flag;
}

void rul__msg_enable_messages (void)
{
  messages_enabled = TRUE;
}

void rul__msg_disable_messages (void)
{
  messages_enabled = FALSE;
}




/************************
**                     **
**  RUL__MSG_VERSION   **
**                     **
************************/

void rul__msg_version (IO_Stream ios)
{
  rul__ios_printf (ios, "%s", CMP_VERSION);
  rul__ios_printf (ios, "%s", CMP_COPYRIGHT);
}




/*********************
**                  **
**  RUL__MSG_PRINT  **
**                  **
*********************/
/*
 * Print a message to stderr.  This doesn't include filename and linenumber,
 * and doesn't get printed to the listing.  Use rul__msg_cmp_print for all
 * compiler messages.
 */
void
rul__msg_print (char *message_str, ...)
{
  va_list ap;
  static char message_buffer[MSG__C_MAX_MESSAGE_LEN];
  char *small_message;
  long save_errors = severe_errors + warning_errors;

  va_start (ap, message_str);
  small_message = rul___msg_get_message_format (message_str);
  vsprintf (message_buffer, small_message, ap);
  
  if (in_debugger || messages_enabled ||
      save_errors == (severe_errors + warning_errors))
    rul__ios_printf (RUL__C_STD_ERR, "%s\n", message_buffer);

  va_end (ap);
}




/*****************************
**                          **
**  RUL__MSG_PRINT_W_ATOMS  **
**                          **
*****************************/

void
rul__msg_print_w_atoms (Msg_Id msg_id, long mol_count, ...)
{
    va_list ap;
    Mol_Atom mol_args[MSG__C_MAX_ATOMS];
    char mol_printforms[MSG__C_MAX_ATOMS][RUL_C_MAX_SYMBOL_SIZE+1];
    long i;

    assert (mol_count <= MSG__C_MAX_ATOMS);
    
    /*  First get all the molecules, and convert them into strings  */
    va_start (ap, mol_count);
    for (i=0; i<mol_count && i<MSG__C_MAX_ATOMS; i++) {
	mol_args[i] = va_arg (ap, Molecule);
	assert (rul__mol_is_valid(mol_args[i]));
	rul__mol_use_printform (mol_args[i], 
				&(mol_printforms[i][0]),
				RUL_C_MAX_SYMBOL_SIZE);
    }
    va_end (ap);

    /*  Then call rul__msg_print with the appropriate args  */
    if (mol_count == 0) {
	rul__msg_print (msg_id);
    } else if (mol_count == 1) {
	rul__msg_print (msg_id, &(mol_printforms[0][0]));
    } else if (mol_count == 2) {
	rul__msg_print (msg_id,
		&(mol_printforms[0][0]),
		&(mol_printforms[1][0]));
    } else if (mol_count == 3) {
	rul__msg_print (msg_id,
		&(mol_printforms[0][0]),
		&(mol_printforms[1][0]),
		&(mol_printforms[2][0]));
    } else if (mol_count > 3) {
	rul__msg_print (msg_id,
		&(mol_printforms[0][0]),
		&(mol_printforms[1][0]),
		&(mol_printforms[2][0]),
		&(mol_printforms[3][0]));
    }
}



/*********************************
**                              **
**  RUL__MSG_GET_SEVERE_ERRORS  **
**                              **
*********************************/

long rul__msg_get_severe_errors (void)
{
  return severe_errors;
}
