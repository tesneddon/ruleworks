/* cons.h - cons package */
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
 * FACILITY:
 *	RULEWORKS compiler/run time system
 *
 * ABSTRACT:
 *	This file is the external specification for rul_cons.c, which
 *	provides a high-level cons abstraction.
 *
 * AUTHOR:
 *	DEC	Digital Equipment Corporation
 *	CPQ	Compaq Computer Corporation
 *
 * MODIFICATION HISTORY:
 *	14-Jan-1993	DEC	Initial version
 *	01-Dec-1999	CPQ	Release with GPL
 */


/* add an entry in the cons list and hash table */
void rul__cons_create_cons (Mol_Symbol name);

/* get an entries index,  returns zero if not found */
long rul__cons_get_index (Mol_Symbol name);

/* get next entry in list,  set context = NULL for first
   returns index, zero if end */
long rul__cons_get_next (void **context, Mol_Symbol *name);

/* get the count of constants */
long rul__cons_get_count (void);

void rul__cons_init (void);

void rul__cons_clear (void);

/* add an entry in the class list and hash table */
void rul__class_create_class (Class class_id);

/* get an entries index,  returns zero if not found */
long rul__class_get_index (Class class_id);

/* get next entry in list,  set context = NULL for first
   returns index, zero if end */
long rul__class_get_next (void **context, Class *class_id);

/* get the count of classes */
long rul__class_get_count (void);

void rul__class_init (void);

void rul__class_clear (void);

