/* api_msg_mess.h - Application Program Interface Message definitions */
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
**                                                                         **
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
 *	%c, numeric (decimal) inserts as %d or %ld.  % needs to be doubled as
 *	%%.  Backslash and quote need to be quoted by preceding with \.
 *	Newline is \n.
 *
 *  MODIFIED BY:
 *	DEC	Digital Equipment Corporation
 *
 *  MODIFICATION HISTORY:
 *
 *	 9-Sep-1992	DEC	Initial version
 *	 1-Dec-1999 CPQ	Release with GPL
 */

/************************* API (rts_cb_common) *****************/

#define API_ATTNOTCLA \
  "ATTNOTCLA E Attribute %s not in class %s"

#define API_ATTRNOTCOMP \
  "ATTNOTCOM E Attribute %s not compound"

#define API_ATTRNOTSCLR \
  "ATTNOTSCA E Attribute %s not scalar"

#define API_BADCMPVAL \
  "BADCMPVAL E Only keyword COMPOUND allowed after (, encountered %s"

#define API_INVATMPAR \
  "INVATMPAR E Invalid atom parameter%s"

#define API_INVATTSTR \
  "INVATTVAL E Invalid attribute %s, not a symbol"

#define API_INVATTVAL \
  "INVATTVAL E Attribute %s not same shape as value %s"

#define API_INVBLOCK \
  "INVBLOCK E Invalid DECLARATION-BLOCK %s, block not visible"

#define API_INVCLASS \
  "INVCLASS E Invalid class %s, class not visible"

#define API_INVCLSATT \
  "INVCLSATT E Attribute %s not in class %s"

#define API_INVCLSNAM \
  "INVCLASS E Invalid class %s, not a valid symbol"

#define API_INVCOMPIDX \
  "INVCOMPIDX E Invalid compound element index %ld, must be > 0"

#define API_INVSTRDSC \
  "INVSTRDSC E Invalid string descriptor, must be static"

#define API_INVVALTYP \
  "INVVALTYP E Invalid value, %s, for attribute %s"

#define API_INVWMEHAT \
  "INVWMEATT E Invalid WMO string at %s, expecting '^'"

#define API_INVWMEEOF \
  "INVWMEEOF E Invalid WMO, unexpected EOF encountered"

#define API_NONWMEIDARG \
  "NONWMEID E Invalid INSTANCE ID %s"

#define API_NOSUCHWME \
  "NOSUCHWME E No such WMO %s"

#define API_NONSYMARG \
  "NONSYMARG E Nonsymbolic atom %s"

#define API_OBSOLFEAT \
  "OBSOLFEAT W Obsolete feature %s"

#define API_WIDTRANSOFF \
  "WIDTRANSOFF W WM ID translation not enabled"

#define API_WIDTRANSON \
  "WIDTRANSON W WM ID translation already enabled"

#define API_UNMRPAREN \
  "UNMRPAREN W Unexpected right parenthesis ')' ignored"

#define API_CANTOPEN \
  "CANTOPEN W Unable to open STATE file %s for %s"

#define API_CLSNOTBLK \
  "CLSNOTBLK W Class %s not declared in declaration block %s"

#define API_NOSUCHCLS \
  "NOSUCHCLS W No such class %s"

#define API_INVDECLBK \
  "INVDECLBK E Invalid declaration block %s"

#define API_NOCLSNAM \
  "NOCLSNAM E Class name not specified"

#define API_INVSPECLS \
  "INVSPECLS E Invalid WMO class, %s not subclass of %s"

#define API_INVCMPATM \
  "INVCMPATM W Invalid compound atom value %s, must be quoted"

#define API_INVRESFIL \
  "INVRESFIL W Invalid restorestate file %s, bad version: %s"

#define API_REFDATERR \
  "REFDATERR E Refraction set data error, expected '(', was %s at line %s"

#define API_REFERRID \
  "REFERRID E Refraction set data error, expecting instance-id, was %s at line %s"

