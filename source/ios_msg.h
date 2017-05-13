/* ios_msg.h - I/O Subsystem Message definitions */
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
 *	CPQ	Compaq Computer Corporation
 *
 *  MODIFICATION HISTORY:
 *
 *	 9-Sep-1992	DEC	Initial version
 **	01-Dec-1999	CPQ	Release with GPL
 */


/**************************  IOS errors  **********************/

#define IOS_FILEOPEN \
    "FILEOPEN W File id %s is already in use"

#define IOS_CANTOPEN \
    "CANTOPEN W Unable to open file %s for %s"

#define IOS_NOSUCHFILE \
    "NOSUCHFILE W File id %s does not exist"

#define IOS_BADOUTFILE \
    "BADOUTFILE W Invalid write; file id %s was opened for reading"

#define IOS_BADINFILE \
    "BADINFILE W Invalid read; file id %s was opened for writing"

#define IOS_INVACCIOS \
    "INVACCIOS W Invalid file id for ACCEPT-ATOM"

#define IOS_INVACLIOS \
    "INVACLIOS W Invalid file id for ACCEPTLINE-COMPOUND"

#define IOS_INVDEFKEY \
    "INVDEFKEY W Invalid DEFAULT keyword, %s; must be ACCEPT, WRITE, or TRACE"

#define IOS_INVOPNMOD \
  "INVOPNMOD E Invalid open mode %s; must be IN, OUT or APPEND"

/*
**  Following 6 are used for attempts to read or write from a default after
**  that file has been closed.  User action: remember to reset the default
**  when you close the file.
*/
#define IOS_DEFOUTCLOS \
  "DEFOUTCLOS W Invalid write; current default, file id %s, is closed"

#define IOS_DEFINCLOS \
  "DEFINCLOS W Invalid accept; current default, file id %s, is closed"

#define IOS_DEFOUTRESD \
  "DEFOUTRESD I Resetting the default for output back to standard output"

#define IOS_DEFINRESD \
  "DEFINRESD I Resetting the default for input back to standard input"

/*
**  Attempts to set default to files that were opened for the
**  wrong direction
*/
#define IOS_BADTRADEF \
    "BADTRADEF W Invalid default for trace; file id %s was opened for reading"

#define IOS_BADWRIDEF \
    "BADWRIDEF W Invalid default for write; file id %s was opened for reading"

#define IOS_BADACCDEF \
    "BADACCDEF W Invalid default for accept; file id %s was opened for writing"

