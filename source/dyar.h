/*  Dynamic array package */
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



Dynamic_Array 	rul__dyar_create_array (long initial_size);

void 		rul__dyar_free_array (Dynamic_Array array);

void		rul__dyar_set_array_empty (Dynamic_Array array);
		  /*
		  **  Throws away the contents of the array, and sets
		  **  the element count to be 0.
		  */

void 		rul__dyar_append (Dynamic_Array array, void *new_val);
		  /*
		  **  Inserts a new element after the last existing element.
		  */

void 		rul__dyar_append_if_unique (Dynamic_Array array, void *new_val);
		  /*
		  **  If the specified new_val is not already in the
		  **  dynamic array, then it inserts it as a new element
		  **  after the last existing element.  If it is already
		  **  in the dynamic array, do nothing.
		  */

void 		rul__dyar_prepend (Dynamic_Array array, void *new_val);
		  /*
		  **  Inserts a new element in front of the first element.
		  */

void	       *rul__dyar_pop_first (Dynamic_Array array);
		  /*
		  **  Removes the element at the front of the existing
		  **  array and returns that element.
		  */

void	       *rul__dyar_pop_last (Dynamic_Array array);
		  /*
		  **  Removes the element at the back of the existing
		  **  array and returns that element.
		  */

void	       *rul__dyar_get_nth (Dynamic_Array array, long n);
		  /*
		  **  Returns the N'th element of the array.
		  **  Does not change the array.
		  **			Note:  N is a 0-based index.
		  */

void	       *rul__dyar_remove_nth (Dynamic_Array array, long n);
		  /*
		  **  Removes the specified element from the existing
		  **  array and returns that element.
		  **			Note:  N is a 0-based index.
		  */

void	       rul__dyar_remove_value (Dynamic_Array array, void *old_val);
		  /*
		  **  Removes all occurances of the specified 
		  **  opaque value from the array.
		  */

void 		rul__dyar_set_nth (Dynamic_Array array, long n, void *new_val);
		  /*
		  **  Set the N'th element of the array.  If necessary,
		  **  any unused positions between the highest set and 
		  **  position N will be filled in with NULLs.
		  **			Note:  N is a 0-based index.
		  */

long 		rul__dyar_get_length (Dynamic_Array array);
		  /*
		  **  Return the number of filled positions.
		  */

void          **rul__dyar_get_array (Dynamic_Array array);
		  /*
		   ** return the pointer to the array
		   ** and free the Dynamic_array struct
		   */
