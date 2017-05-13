/* ver_msg.h - compiler run-time version message definitions */
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
 *	Informational messages are defined in this file.
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
 *	23-Oct-1995	DEC	Version 2.0 & new copyright date
 *	19-NOV-1996	DEC	Version 2.0-1 & new copyright date
 *	01-Sep-1997	DEC	VER_EXPIRE_YEAR now expressed as yyyy and not yy
 *	01-Dec-1999	CPQ	Release with GPL V2.2
 */


/************************** Version description lines **********************/

#define VER_EXPIRES	  0		/* enables expiration check */
#define VER_EXPIRE_MONTH  12    	/* 1 - 12 */
#define VER_EXPIRE_DAY    30		/* 1 - 31 */
#define VER_EXPIRE_YEAR   1999		/* year in yyyy format */


#define CMP_VERSION \
 "  RuleWorks (tm) -- Version 2.2\n"
#define CMP_COPYRIGHT \
 "  Copyright (c) 1993-1999 Compaq Computer Corporation.  All Rights Reserved.\n"


#define CMP_INTERNAL	  1	/* enables the following legalities message */
#define CMP_INTERNAL_ONLY \
 " RuleWorks comes with ABSOLUTELY NO WARRANTY. This is free software, and you are welcome to redistribute it under certain conditions; see GNU General Public License for details.\n\n"


#define RUL__C_PRODUCT_PROMPT	"RuleWorks> "
#define RUL__C_MSG_PREFIX	"RUL"	/* e.g. "RUL" in RUL-W-INV... */
#define RUL__C_DEF_IN_FILE_EXT	".rul"
#define RUL__C_DECL_FILE_EXT	".use"


/*************************  actual error messages  *************************/

#define VER_LICEXPIRED \
    "LICEXPIRED E This version of compiler has expired."
