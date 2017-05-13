/* msg_mess.h - Message definitions */
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
 *  FACILITY:
 *	RULEWORKS run time system and compiler
 *
 *  ABSTRACT:
 *	Diagnostic messages are defined in this file.  This is intended
 *	as a temporary solution for defining messages.
 *
 *  Format:
 *	Each message is defined with a #define, with a macro name of
 *	MSG_msgid (or fac_msgid), where msgid is the (uppercase)
 *	message ID on VMS.  The replacement string for the macro
 *	starts at the beginning of the next line.  It's a string
 *	literal consisting of the message ID in uppercase, a space,
 *	the severity - one of {I W E F} (uppercase), a space, and the
 *	text of the message.  A comment describing the message starts
 *	on the next line and continues as needed.
 *
 *	The text of the message is a C language printf() format
 *	string.  String inserts are represented as %s, characters as
 *	%c, numeric (decimal) inserts as %d.  % needs to be doubled as
 *	%%.  Backslash and quote need to be quoted by preceding with \.
 *	Newline is \n.
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

#define MSG_NULLCHAR \
    "NULLCHAR W Null character (ASCII 0) ignored; remove it"
/*
 * A null character (ASCII 0) was found in the source file while scanning.
 * The character should be removed.
 */

#define MSG_QUOTEMISSING \
    "QUOTEMISSING W Closing quote missing; added at end of line"
/*
 * The source line has unbalanced vertical bars.
 */

#define MSG_RESCHAR \
    "RESCHAR W Unquoted reserved character '%s' ignored"
/*
 * One of the characters we reserved for future use (&, ", %, ~) was found
 * in the source file.  It should be removed or quoted in vertical bars.
 */

#define MSG_CNTLCHAR \
    "CNTLCHAR W Unquoted control character (%s) ignored"
/*
 * An unquoted control character was found in the input.
 * It should be removed or quoted in vertical bars.
 */

#define MSG_STRTRUNCD \
    "STRTRUNCD W %s too long, truncated to %s"
/*
 * a constant was too long to be valid, truncated
 */


/******************** Miscellaneous errors *************************/


#define MEM_INSUFMEM \
    "INSUFMEM F Insufficient dynamic memory available"
/*
 * An attempt has been made to allocate dynamic memory within either
 * the compiler or the run-time system.  You're out of luck...
 */

