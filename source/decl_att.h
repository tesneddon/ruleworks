/*
 *	decl_att.h - declares the offsets for the builtin attributes
 */
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
**	RULEWORKS compiler and Run-Time-System
**
**  ABSTRACT:
**	
**
**  MODIFIED BY:
**	DEC	Digital Equipment Corporation
**	CPQ	Compaq Computer Corporation
**
**  MODIFICATION HISTORY:
**
**	 4-Sep-1992	DEC	Initial version
**	01-Dec-1999	CPQ	Releasew ith GPL
*/
#ifndef DECL_ATT__LOADED
#define DECL_ATT__LOADED		1

#define DECL_ID_ATTR_OFFSET		0
#define DECL_INSTANCE_OF_ATTR_OFFSET	1
#define DECL_FIRST_USER_ATTR_OFFSET	2

/* This assumes C's model of zero-based indexing. */
#define DECL_NUM_BUILTIN_ATTRS		DECL_FIRST_USER_ATTR_OFFSET


#endif /* ifndef DECL_ATT__LOADED */
