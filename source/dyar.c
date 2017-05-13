/****************************************************************************
**                                                                         **
**                          O P S _ D Y A R . C                            **
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
**  FACILITY:
**	RULEWORKS compiler and run time system
**
**  ABSTRACT:
**	This file contains the low level dynamic-array package
**
**  MODIFIED BY:
**	DEC	Digital Equipment Corporation
**	CPQ	Compaq Computer Corporation
**
**  MODIFICATION HISTORY:
**
**	 8-Mar-1993	DEC	Initial version
**	01-Dec-1999	CPQ	Release with GPL
*/



#include <common.h>
#include <dyar.h>

#define DYAR_VERIFY 12321


struct dynamic_array {
		void	      **dynamic_array;
		long		array_length;
		long		items_stored;
#ifndef NDEBUG
		long		verification;
#endif
}; /*   *Dynamic_Array   */


static  Boolean dyar_is_valid_array (Dynamic_Array array);
static  void  	dyar_ensure_array_has_room (Dynamic_Array array, long posn);



#ifndef NDEBUG



/**************************
**                       **
**  DYAR_IS_VALID_ARRAY  **
**                       **
**************************/

static Boolean dyar_is_valid_array (Dynamic_Array array)
	/*  Use only in asserts  */
{
	return (array != NULL  &&  array->verification == DYAR_VERIFY);
}
#endif




/*********************************
**                              **
**  DYAR_ENSURE_ARRAY_HAS_ROOM  **
**                              **
*********************************/

static void  dyar_ensure_array_has_room (Dynamic_Array array, long posn)
{
	long i, start;

	assert (dyar_is_valid_array (array));

	if (array->array_length <= posn) {

	    /*  Alocation wasn't big enough */
	    start = array->array_length;
	    /*  Set new size based on current size and position to be set */
	    array->array_length +=
			   MAX (posn - start + 10, 
				MIN (array->array_length, 1000));
	    array->dynamic_array = 
			rul__mem_realloc (
				  array->dynamic_array, 
				  array->array_length * sizeof(void *));
	    assert (array->dynamic_array != NULL);

#ifndef NDEBUG
	    for (i=start; i<array->array_length; i++) {
		array->dynamic_array[i] = NULL;
	    }
#endif
	}
}




/*****************************
**                          **
**  RUL__DYAR_CREATE_ARRAY  **
**                          **
*****************************/

Dynamic_Array rul__dyar_create_array (long initial_size)
{
	Dynamic_Array array;
	long i;

	array = rul__mem_malloc (sizeof (struct dynamic_array));
	assert (array != NULL);

	array->dynamic_array = 
		rul__mem_malloc (initial_size * sizeof(void *));
	assert (array->dynamic_array != NULL);

	array->array_length = initial_size;
	array->items_stored = 0;

#ifndef NDEBUG
	array->verification = DYAR_VERIFY;
	for (i=0; i<array->array_length; i++) {
	    array->dynamic_array[i] = NULL;
	}
#endif

	return (array);
}




/***************************
**                        **
**  RUL__DYAR_FREE_ARRAY  **
**                        **
***************************/

void rul__dyar_free_array (Dynamic_Array array)
{
	assert (dyar_is_valid_array (array));

	if (array->dynamic_array) rul__mem_free (array->dynamic_array);

#ifndef NDEBUG
	array->dynamic_array = NULL;
	array->verification = 0;
	array->array_length = 0;
	array->items_stored = 0;
#endif

	rul__mem_free (array);
}




/********************************
**                             **
**  RUL__DYAR_SET_ARRAY_EMPTY  **
**                             **
********************************/

void rul__dyar_set_array_empty (Dynamic_Array array)
	  /*
	  **  Throws away the contents of the array, and sets
	  **  the element count to be 0.
	  */
{
	assert (dyar_is_valid_array (array));

#ifndef NDEBUG
	{
	    long i;
	    for (i=0; i<array->array_length; i++)
	        array->dynamic_array[i] = NULL;
	}
#endif

	array->items_stored = 0;
}





/***********************
**                    **
**  RUL__DYAR_APPEND  **
**                    **
***********************/

void rul__dyar_append (Dynamic_Array array, void *new_val)
	/*
	**  Inserts a new element after the last existing element.
	*/
{
	assert (dyar_is_valid_array (array));

	dyar_ensure_array_has_room (array, array->items_stored);

	array->dynamic_array[array->items_stored] = new_val;
	array->items_stored += 1;
}



/*********************************
**                              **
**  RUL__DYAR_APPEND_IF_UNIQUE  **
**                              **
*********************************/

void rul__dyar_append_if_unique (Dynamic_Array array, void *new_val)
	/*
	**  If the specified new_val is not already in the
	**  dynamic array, then it inserts it as a new element
	**  after the last existing element.  If it is already
	**  in the dynamic array, do nothing.
	*/
{
	long i;
	assert (dyar_is_valid_array (array));

	for (i=0; i<array->items_stored; i++) {
	    if (array->dynamic_array[i] == new_val) return;
	}

	/*  It's not already in the array, so add it.  */

	dyar_ensure_array_has_room (array, array->items_stored);
	array->dynamic_array[array->items_stored] = new_val;
	array->items_stored += 1;
}





/************************
**                     **
**  RUL__DYAR_PREPEND  **
**                     **
************************/

void rul__dyar_prepend (Dynamic_Array array, void *new_val)
	/*
	**  Inserts a new element in front of the first existing element.
	*/
{
	long i;

	assert (dyar_is_valid_array (array));

	dyar_ensure_array_has_room (array, array->items_stored);

	for (i=array->items_stored-1; i>=0; i--) {
	    array->dynamic_array[i+1] = array->dynamic_array[i];
	}
	array->dynamic_array[0] = new_val;
	array->items_stored += 1;
}




/**************************
**                       **
**  RUL__DYAR_POP_FIRST  **
**                       **
**************************/

void *rul__dyar_pop_first (Dynamic_Array array)
	/*
	**  Removes the element at the front of the existing
	**  array and returns that element.
	*/
{
	long i;
	void *elem;

	assert (dyar_is_valid_array (array));

	if (array->items_stored == 0) return (NULL);

	elem = array->dynamic_array[0];

	for (i=1; i<array->items_stored; i++) {
	    array->dynamic_array[i-1] = array->dynamic_array[i];
	}
	array->items_stored -= 1;

#ifndef NDEBUG
	array->dynamic_array[array->items_stored] = NULL;
#endif
	return (elem);
}




/*************************
**                      **
**  RUL__DYAR_POP_LAST  **
**                      **
*************************/

void *rul__dyar_pop_last (Dynamic_Array array)
	/*
	**  Removes the element at the back of the existing
	**  array and returns that element.
	*/
{
	void *elem;

	assert (dyar_is_valid_array (array));

	if (array->items_stored == 0) return (NULL);

	elem = array->dynamic_array[array->items_stored-1];
	array->items_stored -= 1;

#ifndef NDEBUG
	array->dynamic_array[array->items_stored] = NULL;
#endif
	return (elem);
}




/***************************
**                        **
**  RUL__DYAR_REMOVE_NTH  **
**                        **
***************************/

void *rul__dyar_remove_nth (Dynamic_Array array, long n)
	/*
	**  Removes the specified element from the existing
	**  array and returns that element.
	**  	Note:  N is a 0-based index.
	*/
{
	long i;
	void *elem;

	assert (dyar_is_valid_array (array));
	assert (n >= 0);

	if (n >= array->items_stored) return (NULL);

	elem = array->dynamic_array[n];

	for (i=n+1; i<array->items_stored; i++) {
	    array->dynamic_array[i-1] = array->dynamic_array[i];
	}
	array->items_stored -= 1;

#ifndef NDEBUG
	array->dynamic_array[array->items_stored] = NULL;
#endif
	return (elem);
}



/*****************************
**                          **
**  RUL__DYAR_REMOVE_VALUE  **
**                          **
*****************************/

void rul__dyar_remove_value (Dynamic_Array array, void *val_to_remove)
	  /*
	  **  Removes all occurances of the specified 
	  **  opaque value from the array.
	  */
{
	long i, j, item_count;

	assert (dyar_is_valid_array (array));

	item_count = array->items_stored;
	for (i=0; i<item_count; i++) {
	    while (array->dynamic_array[i] == val_to_remove) {
	        for (j=i+1; j<item_count; j++) {
		    array->dynamic_array[j-1] = array->dynamic_array[j];
		}
		item_count -= 1;
	    }
	}
	array->items_stored = item_count;
}




/************************
**                     **
**  RUL__DYAR_GET_NTH  **
**                     **
************************/

void *rul__dyar_get_nth (Dynamic_Array array, long n)
	/*
	**  Returns the N'th element of the array.
	**  Does not change the array.
	**  	Note:  N is a 0-based index.
	*/
{
	assert (dyar_is_valid_array (array));
	assert (n >= 0);

	if (n >= array->items_stored) return (NULL);

	return (array->dynamic_array[n]);
}




/************************
**                     **
**  RUL__DYAR_SET_NTH  **
**                     **
************************/

void rul__dyar_set_nth (Dynamic_Array array, long n, void *new_val)
	/*
	**  Set the N'th element of the array.  If necessary, any
	**  unused positions between the highest set and N will
	**  be filled in with NULLs.
	**				Note:  N is a 0-based index.
	*/
{
	long i;

	assert (dyar_is_valid_array (array));
	assert (n >= 0);

	dyar_ensure_array_has_room (array, n);

	if (n > array->items_stored) {
	    for (i= array->items_stored; i<n; i++) {
		array->dynamic_array[i] = NULL;
	    }
	}
	array->dynamic_array[n] = new_val;
	array->items_stored = MAX (n + 1, array->items_stored); 
}




/***************************
**                        **
**  RUL__DYAR_GET_LENGTH  **
**                        **
***************************/

long rul__dyar_get_length (Dynamic_Array array)
	/*
	**  Return the number of filled positions.
	*/
{
	if (array == NULL) return (0);

	assert (dyar_is_valid_array (array));
	
	return (array->items_stored);
}

void **rul__dyar_get_array (Dynamic_Array array)
	  /*
	   ** return the pointer to the array
	   ** and free the Dynamic_array struct
	   */
{
    void **array_ptr;

	if (array == NULL) return (0);

	assert (dyar_is_valid_array (array));

        array_ptr = array->dynamic_array;
	rul__mem_free (array);

        return array_ptr;
}
