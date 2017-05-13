/* msg.h - Diagnostic message function declarations */
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
 *	Interface for routines which display diagnostic messages.
 *
 *  MODIFIED BY:
 *	DEC	Digital Equipment Corporation
**	CPQ	Compaq Computer Corporation
 *
 *  MODIFICATION HISTORY:
 *
 *	 9-Sep-1992	DEC	Initial version
 *	01-Dec-1999	CPQ	Release with GPL
 */



#define MSG__C_LONGEST_MSGID 	32	/* Longest FOO in %RUL-W-FOO */
#define MSG__C_MAX_MESSAGE_LEN 	(RUL_C_MAX_SYMBOL_SIZE*6)

/* Note:  changing this following definition is NOT sufficient */
#define MSG__C_MAX_ATOMS 	4


typedef char *Msg_Id;		/* For temporary hack, Msg_Ids are strings */


/* MSG internal use only:  */
char *rul___msg_get_message_format (char *str);


/*
 * Print compiler message with inserts to stderr and listing.  The file
 * name, line number, and severity (if turned on) will be included.
 *
 * Compiler message output examples for VMS and other:
 *	VMS:
 *		%RUL-W-RESCHAR, Reserved character "~" ignored.
 * 			At line number 23 in FOO.RUL;3.
 * 
 * 	Non-VMS:
 * 		foo.rul:23: warning: Reserved character "~" ignored.
 */
void rul__msg_cmp_print (Msg_Id msg_id, Ast_Node ast, ...);

void rul__msg_cmp_print_w_line (Msg_Id msg_id, long line_number, ...);

void rul__msg_set_in_debugger (Boolean flag);
void rul__msg_enable_messages (void);
void rul__msg_disable_messages (void);
void rul__msg_version (IO_Stream ios);

/*
 * Print message with inserts to stderr, and only to stderr.
 * Note:  Use rul__msg_cmp_print for any compiler messages.
 */
void rul__msg_print (Msg_Id msg_id, ...);


/*
 * Print a message with inserts to stderr, where all the inserts
 * are atoms.  This is merely a convienence function that converts
 * all its arguments from molecules into strings, and then sends the
 * results off to the rul__msg_print function.
 *
 * Note:  The maximum number of atom inserts is 4.
 */
void rul__msg_print_w_atoms (Msg_Id msg_id, long mol_count, ...);


/*
 * Same as above, except for compiler messages; calls rul__msg_cmp_print.
 */
void rul__msg_cmp_print_w_atoms (Msg_Id msg_id,
				 Ast_Node ast, long mol_count, ...);

/*
 * Routine to get count of any ERROR or FATAL severity errors
 */
long rul__msg_get_severe_errors (void);

