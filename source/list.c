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
 *	20-Aug-1992	DEC	Initial Version
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

#include <common.h>
#include <list.h>
#include <ios.h>



struct list {
  struct list *next;
  struct list *prev;
  void *obj_ptr;
};



/* Allocate and initialize an empty list, and return it. */

List
rul__list_create_list (void)
{
  register List list;

  list = rul__mem_malloc(sizeof(struct list));

  list->next	= list;
  list->prev	= list;
  list->obj_ptr	= NULL;

  return list;
}



/* Free the list, but don't free the objects within the list. */

void
rul__list_delete_list (List list)
{
  List current;
  List next;

  current = list;		/* start with the first list element	*/

  do {
    next = current->next;	/* capture the next list element	*/
    rul__mem_free(current);	/* remove the current list element	*/
    current = next;		/* update the current list element	*/
  } while (current != list);	/* until we return to the list head	*/
}



/* Add object to front of list.  The list returned can later be passed to
   remove_first. */

List
rul__list_add_first (List list, void *object_ptr)
{
  register List new_first;
  register List old_first;

  /*
		---------  old	---------
	list--->|  next	|------>|  next	|
		---------	---------
		|  prev	|<------|  prev	|
		---------	---------
  */

  old_first	= list->next;

  new_first	= rul__mem_malloc(sizeof(struct list));

  list->next	= new_first;

  new_first->next= old_first;
  new_first->prev= old_first->prev;
  new_first->obj_ptr= object_ptr;

  old_first->prev= new_first;

  /*
		---------  new	---------  old	---------
	list--->|  next	|------>|  next	|------>|  next	|
		---------	---------	---------
		|  prev	|<------| prev	|<------| prev	|
		---------	---------	---------
  */

  return new_first;
}



/* Add object to end of list.  The list returned can later be passed to
   remove_first. */

List
rul__list_add_last (List list, void *object_ptr)
{
  register List new_last;
  register List old_last;

  /*
		     -------------------------------------------
		    |						|
		    v						|
		---------  	---------  	---------	|
	list--->|  next	|------>|  next	|------>|  next	|-------
		---------	---------	---------
		|  prev	|<------|  prev	|<------|  prev	|<------
		---------	---------	---------  old	|
		    |						|
		    |						|
		     -------------------------------------------
  */

  old_last	= list->prev;

  new_last	= rul__mem_malloc(sizeof(struct list));

  list->prev	= new_last;

  new_last->next	= old_last->next;
  new_last->prev	= old_last;
  new_last->obj_ptr	= object_ptr;

  old_last->next	= new_last;

  /*
		     -----------------------------------------------
		    |						    |
		    v						    |
		---------  	---------  	---------	---------
	list--->|  next	|------>|  next	|------>|  next	|------>|  next	|
		---------	---------	---------	---------
		|  prev	|<------|  prev	|<------|  prev	|<------|  prev	|
		---------	---------	---------  old	---------
		    |						    ^
		    |						    |new
		     -----------------------------------------------
  */

  return new_last;
}



/* Destructively remove first object from list, and return the object.
   An object can be removed from the middle of another existing list. */

void *
rul__list_remove_first (List list)
{
  List pred;
  List succ;
  List new_first;
  void *object_ptr;

  if (list->obj_ptr == NULL) {	/* If "header" of list */
      new_first = list->next->next; /* Second will become first */
      object_ptr = list->next->obj_ptr;	/* Object to return */
      rul__mem_free (list->next);
      list->next = new_first;
      new_first->prev = list;
  }
  else {			/* Nonheader - Pointer into middle of list */
      pred = list->prev;
      succ = list->next;

      pred->next	= succ;
      succ->prev	= pred;

      object_ptr = list->obj_ptr;

      rul__mem_free(list);
  }

  return object_ptr;
}



/* Return the first object in the list. */

void *
rul__list_first (List list)
{
  if (list->obj_ptr == NULL)	/* given a list header */
    return list->next->obj_ptr;	/* return the first true element's object */
  else
    return list->obj_ptr;
}



/* Return the next 'list' pointer in the list. */

List
rul__list_next (List list)
{
    if (list->next->obj_ptr)
      return list->next;
  else
    return NULL;
}


/* Return the list containing all but the first object (non-destructive).
   NULL is returned when passed a list with 0 or 1 objects. */

List
rul__list_rest (List list)
{
  if (rul__list_is_empty (list)) return NULL;

  if (list->obj_ptr == NULL)	/* If passed list "header" */
      if (list->next->obj_ptr == NULL || list->next->next->obj_ptr == NULL)
	  return NULL;		/* Zero or one objects in list */
      else
	  return list->next->next;
  else				/* Passed pointer into middle of list */
      if (list->next->obj_ptr == NULL)
	  return NULL;
      else
	  return list->next;
}


/* Return TRUE if the list contains no objects.  Comparing a list to NULL
   will not tell if a list is empty, this function must be used. */

Boolean
rul__list_is_empty (List list)
{
  return list == NULL || list->next == list; /* NULL, or list points to self */
}

void
rul__list_print (List list)
{
  while (!rul__list_is_empty(list)) {
    rul__ios_printf (RUL__C_STD_OUT, "%p\n", rul__list_first(list));
    list = rul__list_rest(list);
  }
}

