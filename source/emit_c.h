/****************************************************************************
**                                                                         **
**                          E M I T _ C . H                                **
**                                                                         **
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
 * FACILITY:
 *	RULEWORKS compiler
 *
 * ABSTRACT:
 *	This module contains private declarations for the c code emmitters
 *
 * MODIFIED BY:
 *	DEC	Digital Equipment Corporation
 *	CPQ	Compaq Computer Corporation
 *
 * MODIFICATION HISTORY:
 *
 *	24-Feb-1993	DEC	Initial version
 *	01-Dec-1999	CPQ	Release with GPL
 */

/*
**  The following are private to the emit subsystem,
**  and should not be used anywhere else
*/

void rul___emit_string (char *str);
void rul___emit_newline (void);
void rul___emit_verify_newline (void);
void rul___emit_eos_and_newline (void);
void rul___emit_incr_indent (void);
void rul___emit_decr_indent (void);
char *rul___emit_get_ext_type_abbr (Ext_Type type, Cardinality a_len);
