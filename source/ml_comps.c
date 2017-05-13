/****************************************************************************
**                                                                         **
**                O P S _ M O L _ C O M P O U N D S . C                    **
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
**	RULEWORKS run time system and compiler
**
**  ABSTRACT:
**	This module contains the functions which allow other
**	subsystems access to the compound molecular values.
**
**	These routines are used at both compile time and run time.
**
**	The exported interface descriptions are in the header file,
**	mol.h, and the internal version of these declarations
**	are in mol_p.h.
**
**  MODIFIED BY:
**	DEC	Digital Equipment Corporation
**	CPQ	Compaq Computer Corporation
**
**  MODIFICATION HISTORY:
**
**	3-Jun-1992	DEC	Initial version
**	01-Dec-1999	CPQ	Release with GPL
*/



#include <common.h>
#include <stdarg.h>
#include <dyar.h>
#include <ios.h>
#include <mol.h>
#include <msg.h>
#include <rts_msg.h>
#include <cmp_msg.h>
#include <mol_p.h>

static Mol_Compound end_tmp_comp (Boolean decr_flag);

/*************************
**                      **
**  RUL__MOL_MAKE_COMP  **
**                      **
*************************/

Mol_Compound rul__mol_make_comp (long arg_count, ...)
{
	va_list	 	ap;
	Molecule 	mol;
	Mol_Polyatom	cpoly_mol;
	Molecule	*comp_array = NULL;
	Compound_Value	*tcv;
	long		comp_array_length = 100;
	long		item_count = 0;
	long		arg_index = 0;
	long		i;

	/*
	**    First task is to fill a flattened atom array
	*/
	if (arg_count > 0) {
	    comp_array = (Molecule *) rul__mem_malloc (comp_array_length
							* sizeof(Molecule));
	    va_start (ap, arg_count);
	    while (arg_index < arg_count) {
		mol = va_arg (ap, Molecule);

		assert (rul___mol_is_valid (mol)) ;

		if (is_atomic(mol)) {
		    /*
		    **    For atomic arguments, just copy them
		    */
		    if (item_count == comp_array_length) {
			/*    if there is not enough room, make more    */
			comp_array_length *= 3;
	    		comp_array =
				(Molecule *) rul__mem_realloc (comp_array,
					  		comp_array_length
							* sizeof(Molecule));
		    }
		    /*    now that we know there is enough room    */
		    comp_array[item_count] = mol;
		    item_count ++;

		} else if (mol->type == cpoly) {
		    /*
		    **    For a compound argument, then
		    **    copy all its values.
		    */
		    tcv = (Compound_Value *) mol;
		    for (i=0; i<tcv->element_count; i++) {
		        if (item_count == comp_array_length) {
			    /*    if there is not enough room, make more  */
			    comp_array_length *= 3;
	    		    comp_array =
					(Molecule *) rul__mem_realloc (
							 comp_array,
			    		  		 comp_array_length
							 * sizeof(Molecule));
		        }
			/*    now that we know there is enough room    */
			comp_array[item_count] = tcv->values[i];
			item_count ++;
		    }
		}
		arg_index ++;
	    }
	    va_end (ap);
	}
	cpoly_mol = rul___mol_make_cpoly (item_count, comp_array);
	if (comp_array != NULL) {
	    rul__mem_free (comp_array);
	}
	return (cpoly_mol);
}



/***********************
**                    **
**  RUL__MOL_SUBCOMP  **
**                    **
***********************/

Mol_Compound  rul__mol_subcomp (Mol_Compound mol,
				long start_index, long end_index)
{
	Mol_Compound	cpoly_mol;
	Molecule	*comp_array = NULL;
	Compound_Value	*old_cpoly;
	long		i, item_count, index;

	assert (rul___mol_is_valid_cpoly (mol));

	old_cpoly = (Compound_Value *) mol;
	
	/*
	**    First task is to fill a flattened atom array
	*/
	comp_array = (Molecule *)
		rul__mem_malloc (old_cpoly->element_count * sizeof(Molecule));

	item_count = 0;
	for (i=0; i<old_cpoly->element_count; i++) {
	    index = i + 1;
	    if (index >= start_index  &&  index <= end_index) {
		comp_array[item_count] = old_cpoly->values[i];
		item_count ++;
	    }
	}

	cpoly_mol = rul___mol_make_cpoly (item_count, comp_array);
	if (comp_array != NULL) {
	    rul__mem_free (comp_array);
	}
	return (cpoly_mol);
}



/**********************************
**                               **
**  RUL__MOL_REMOVE_COMP_NTH_RT  **
**                               **
**********************************/

Mol_Compound rul__mol_remove_comp_nth_rt (Mol_Compound mol,
					  Mol_Int_Atom int_mol)
{
  long n;
  
  if (!rul__mol_is_int_atom (int_mol)) {
    rul__msg_print_w_atoms (RTS_INVCMPIDX, 1, int_mol);
    return rul__mol_symbol_nil ();
  }

  n = rul__mol_int_atom_value (int_mol);
  if (n < 1) {
    rul__msg_print_w_atoms (RTS_INVCMPIDX, 1, int_mol);
    return rul__mol_symbol_nil ();
  }

  return rul__mol_remove_comp_nth (mol, n);
}

/*******************************
**                            **
**  RUL__MOL_REMOVE_COMP_NTH  **
**                            **
*******************************/

Mol_Compound rul__mol_remove_comp_nth (Mol_Compound mol, long n)
{
	Molecule	*comp_array = NULL;
	Compound_Value	*old_cpoly;
	Mol_Compound 	cpoly_mol;
	long		i, item_count, index;

	assert (rul___mol_is_valid_cpoly (mol));

	old_cpoly = (Compound_Value *) mol;

	/*
	**    First task is to fill a flattened atom array
	*/
	comp_array = (Molecule *)
		rul__mem_malloc (old_cpoly->element_count * sizeof(Molecule));

	item_count = 0;
	for (i=0; i<old_cpoly->element_count; i++) {
	    index = i + 1;
	    if (index != n) {
		comp_array[item_count] = old_cpoly->values[i];
		item_count ++;
	    }
	}

	cpoly_mol = rul___mol_make_cpoly (item_count, comp_array);
	if (comp_array != NULL) {
	    rul__mem_free (comp_array);
	}
	return (cpoly_mol);
}



/********************************
**                             **
**  RUL__MOL_POSITION_IN_COMP  **
**                             **
********************************/

long	rul__mol_position_in_comp (Mol_Compound mol,
				   Boolean (*pred_func) (Molecule, Molecule),
				   Molecule atom_val)
{
	Compound_Value *poly_ptr;
	long i;

	assert (rul___mol_is_valid_cpoly (mol));

	poly_ptr = (Compound_Value *) mol;

/*? optimize???...
	if (pred_func == NULL)
	    if (!rul___mol_possible_containment (mol, atom_val))
	        return (0);
*/

	for (i = 0; i < poly_ptr->element_count; i++) {
	    if (pred_func == NULL) {
	        if (atom_val == poly_ptr->values[i]) {
	            return (i + 1);
		}
	    }
	    else {
	        if ((*pred_func) (poly_ptr->values[i], atom_val)) {
		    return (i + 1);
		}
	    }
	}
	return (0);
}





/********************************
**                             **
**  RUL__MOL_GET_COMP_NTH_MOL  **
**                             **
********************************/

Mol_Atom rul__mol_get_comp_nth_mol (Mol_Compound mol, Mol_Int_Atom int_mol)
{
  long n;
  Compound_Value *poly_ptr;

  if (!rul__mol_is_int_atom (int_mol)) {
    rul__msg_print_w_atoms (RTS_INVCMPIDX, 1, int_mol);
    return rul__mol_symbol_nil ();
  }

  n = rul__mol_int_atom_value (int_mol);
  if (n < 1) {
    rul__msg_print_w_atoms (RTS_INVCMPIDX, 1, int_mol);
    return rul__mol_symbol_nil ();
  }

  if (! rul___mol_is_valid_cpoly (mol)) {
    rul__msg_print_w_atoms (RTS_INVNTHPAR, 1, mol);
    return rul__mol_symbol_nil ();
  }

  poly_ptr = (Compound_Value *) mol;
  if (n > poly_ptr->element_count) {
    return rul__mol_symbol_nil ();
  }

  return (rul__mol_get_comp_nth (mol, n));
}


/***********************************
**                                **
**  RUL__MOL_GET_COMP_NTH_MOL_RT  **
**                                **
***********************************/

Mol_Atom rul__mol_get_comp_nth_mol_rt (
		Mol_Compound compound_value, Mol_Int_Atom mol_index)
{
  Molecule elem;

  elem = rul__mol_get_comp_nth_mol (compound_value, mol_index);
  rul__mol_incr_uses (elem);
  return (elem);
}



/*******************************
**                            **
**  RUL__MOL_GET_COMP_NTH_RT  **
**                            **
*******************************/

Mol_Atom rul__mol_get_comp_nth_rt (
		Mol_Compound compound_value, long index)
{
  Molecule elem;

  elem = rul__mol_get_comp_nth (compound_value, index);
  rul__mol_incr_uses (elem);
  return (elem);
}


/****************************
**                         **
**  RUL__MOL_GET_COMP_NTH  **
**                         **
****************************/

Mol_Atom rul__mol_get_comp_nth (Mol_Compound mol, long index)
{
	Compound_Value *poly_ptr;

	if (mol == NULL) return (rul__mol_symbol_nil ());
	assert (rul___mol_is_valid_cpoly (mol));

	poly_ptr = (Compound_Value *) mol;

	if (index < 1) {
	    return (rul__mol_symbol_nil ());
	} else if (index > poly_ptr->element_count) {
	    return (rul__mol_symbol_nil ());
	}

	return (poly_ptr->values[index-1]);
}




/*****************************
**                          **
**  RUL__MOL_GET_COMP_LAST  **
**                          **
*****************************/

Mol_Atom rul__mol_get_comp_last (Mol_Compound mol)
{
	Compound_Value *poly_ptr;

	if (mol == NULL) return (rul__mol_symbol_nil ());
	assert (rul___mol_is_valid_cpoly (mol));

	poly_ptr = (Compound_Value *) mol;

	if (poly_ptr->element_count < 1) return (rul__mol_symbol_nil ());

	return (poly_ptr->values[poly_ptr->element_count - 1]);
}



/********************************
**                             **
**  RUL__MOL_GET_COMP_LAST_RT  **
**                             **
********************************/

Mol_Atom rul__mol_get_comp_last_rt (Mol_Compound mol)
{
	Molecule elem;

	elem = rul__mol_get_comp_last (mol);
	rul__mol_incr_uses (elem);

	return (elem);
}



/****************************
**                         **
**  RUL__MOL_SET_COMP_NTH  **
**                         **
****************************/

Mol_Compound rul__mol_set_comp_nth (Mol_Compound mol, long n,
				    Molecule atom_val, Molecule fill_val)
{
	Mol_Compound	cpoly_mol;
	Molecule	*comp_array = NULL;
	Compound_Value	*old_cpoly;
	long		i, len;
	Molecule        int_mol;

	assert (rul___mol_is_valid_cpoly (mol));

	if (n < 1) {
	    int_mol = rul__mol_make_int_atom (n);
	    rul__msg_print_w_atoms (RTS_INVCMPIDX, 1, int_mol);
	    rul__mol_decr_uses (int_mol);
	    return (NULL);
	}
	old_cpoly = (Compound_Value *) mol;

	/*
	**    First task is to fill a flattened atom array
	*/
	len = (old_cpoly->element_count > n ? old_cpoly->element_count : n);
	comp_array = (Molecule *)
		rul__mem_malloc (len * sizeof(Molecule));

	for (i=0; i<len; i++) {
	    if (i+1 == n) {
		comp_array[i] = atom_val;
	    } else if (i < old_cpoly->element_count) {
		comp_array[i] = old_cpoly->values[i];
	    } else {
		if (fill_val != NULL) {

		    assert (rul___mol_is_valid_atom (fill_val));

		    /*  Use the supplied fill value, if there is one  */
		    comp_array[i] = fill_val;
		} else {
		    /*  Otherwise, fill with NILs  */
		    comp_array[i] = rul__mol_symbol_nil ();
		}
	    }
	}

	cpoly_mol = rul___mol_make_cpoly (len, comp_array);
	if (comp_array != NULL) {
	    rul__mem_free (comp_array);
	}
	return (cpoly_mol);
}



static	Dynamic_Array	SA_tmp_mol_array   = NULL;
static  Boolean	   	SB_tmp_mol_in_use  = FALSE;


/******************************
**                           **
**  RUL__MOL_START_TMP_COMP  **
**                           **
******************************/

void rul__mol_start_tmp_comp (long initial_length)
{
	assert (SB_tmp_mol_in_use == FALSE);
	assert (initial_length >= 0);

	SA_tmp_mol_array = rul__dyar_create_array (MAX(initial_length,10));
	SB_tmp_mol_in_use = TRUE;
}



/********************************
**                             **
**  RUL__MOL_SET_TMP_COMP_NTH  **
**                             **
********************************/

void rul__mol_set_tmp_comp_nth (long index, Molecule elem_value)
{
	assert (SB_tmp_mol_in_use == TRUE);
	assert (SA_tmp_mol_array != NULL);
	assert (index >= 0);
	assert (rul__mol_is_valid (elem_value));

	rul__dyar_set_nth (SA_tmp_mol_array, index, elem_value);
}




/****************************
**                         **
**  RUL__MOL_END_TMP_COMP  **
**                         **
****************************/

Mol_Compound rul__mol_end_tmp_comp (void)
{
  return (end_tmp_comp (FALSE));
}

/***********************************
**                                **
**  RUL__MOL_END_TMP_COMP_W_DECR  **
**                                **
***********************************/

Mol_Compound rul__mol_end_tmp_comp_w_decr (void)
{
  return (end_tmp_comp (TRUE));
}

/*******************
**                **
**  END_TMP_COMP  **
**                **
*******************/

static Mol_Compound end_tmp_comp (Boolean decr_flag)
{
  Mol_Compound	cpoly_mol = NULL;
  long		i, j = 0, length;
  Molecule 	*tmp_array = NULL;
  
  assert (SB_tmp_mol_in_use == TRUE);
  assert (SA_tmp_mol_array != NULL);
  
  length = rul__dyar_get_length (SA_tmp_mol_array);
  
  if (length > 0) {
    
    tmp_array = (Molecule *) rul__mem_malloc (length * sizeof(Molecule));
    assert (tmp_array != NULL);
    
    for (i = 0; i < length; i++) {
      tmp_array[j] = (Molecule) rul__dyar_get_nth (SA_tmp_mol_array, i);
      if (tmp_array[j])
	j++;
    }
  }

  cpoly_mol = rul___mol_make_cpoly (j, tmp_array);
  
  /*
   * decrement uses count on valid molecules
   */
  if (decr_flag) {
    for (i = 0; i < j; i++) {
      rul__mol_decr_uses (tmp_array[i]);
    }
  }

  /*  Now cleanup  */
  if (tmp_array)
    rul__mem_free (tmp_array);
  rul__dyar_free_array (SA_tmp_mol_array);
  SA_tmp_mol_array = NULL;
  SB_tmp_mol_in_use = FALSE;

  return (cpoly_mol);
}
