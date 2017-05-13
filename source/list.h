/* list.h - List package */
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
 * FACILITY:
 *	RULEWORKS compiler/run time system
 *
 * ABSTRACT:
 *	This file is the external specification for rul_list.c, which
 *	provides a high-level list abstraction.
 *
 *	Lists are ordered collections of arbitrary objects (void *).
 *	Major functions include list creation, adding and removing
 *	objects to a list, and first and rest for accessing elements
 *	in a list.
 *
 * AUTHOR:
 *	DEC	Digital Equipment Corporation
 *	CPQ	Compaq Computer Corporation
 *
 * MODIFICATION HISTORY:
 *
 *	19-Aug-1992	DEC	Initial version
 *	01-Dec-1999	CPQ	Release with GPL
 */

/*
 * Table of contents:
 *
 *	rul__list_create_list
 *	rul__list_delete_list
 *	rul__list_add_first
 *	rul__list_add_last
 *	rul__list_remove_first
 *	rul__list_first
 *	rul__list_next
 *	rul__list_rest
 *	rul__list_is_empty
 *
 * Coming soon?
 *	iteration and searching
 */


/* Allocate and initialize an empty list, and return it. */
List
rul__list_create_list (void);

/* Free the list, but don't free the objects within the list. */
void
rul__list_delete_list (List list);


/* Add object to front of list.  The list returned can later be passed to
   remove_first. */
List
rul__list_add_first (List list, void *object_ptr);

/* Add object to end of list.  The list returned can later be passed to
   remove_first. */
List
rul__list_add_last (List list, void *object_ptr);

/* Destructively remove first object from list, and return the object.
   An object can be removed from the middle of another existing list. */
void *
rul__list_remove_first (List list);


/* Return the first object in the list. */
void *
rul__list_first (List list);

/* Return the next list pointer.
   NULL is returned when list->next->obj is NULL (next points to header). */
List
rul__list_next (List list);

/* Return the list containing all but the first object (non-destructive).
   NULL is returned when passed a list with 0 or 1 objects. */
List
rul__list_rest (List list);

/* Return TRUE if the list contains no objects.  Comparing a list to NULL
   will not tell if a list is empty, this function must be used. */
Boolean
rul__list_is_empty (List list);
