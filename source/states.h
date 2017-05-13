/*
 *	states.h - implements the add,save,restore state subsystem
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
**	RULEWORKS Runtime system
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
**	29-Sep-1993	DEC	Initial version
**	21-Aug-1998	DEC	Definition of rul__at added
**	01-Dec-1999	CPQ	Release with GPL
*/
/* INCLUDE-IN-GEND.H  *********************************************** */

void rul__at (Mol_Symbol file);

void rul__addstate (Mol_Symbol file, Entry_Data eb_data);

void rul__restorestate (Mol_Symbol file, Entry_Data eb_data);

void rul__savestate (Mol_Symbol file, Entry_Data eb_data);

/* END-INCLUDE-IN-GEND.H  *********************************************** */

