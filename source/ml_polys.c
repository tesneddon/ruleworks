/****************************************************************************
**                                                                         **
**                O P S _ M O L _ P O L Y A T O M S . C                    **
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
**  FACILITY:
**	RULEWORKS compiler and run-time system
**
**  ABSTRACT:
**	This module contains the functions which allow other
**	subsystems access to the polyatomic molecular values.
**
**	These routines are used at both compile time and run-time.
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
**	12-Mar-1992	DEC	Initial version
**
**	18-May-1994	DEC	Change "1 << i" to "1l << i" where long needed.
**
**	14-Nov-1994	DEC	Fix empty containment coredump on OSF/1 AXP.
**
**	01-Dec-1999	CPQ	Release with GPL
*/



#include <common.h>
#include <stdarg.h>
#include <mol_p.h>
#include <mol.h>

static Boolean mol_fill_containee_vector (Compound_Value *cpoly_ptr,
					  Mol_Atom contained_atom);


/******************************
**                           **
**  RUL__MOL_GET_POLY_COUNT  **
**                           **
******************************/

long	rul__mol_get_poly_count (Mol_Polyatom poly_val)
{
	assert (rul___mol_is_valid_poly (poly_val));

	return (((Polyatomic_Value *)poly_val)->element_count);
}



/***********************************
**                                **
**  RUL__MOL_GET_POLY_COUNT_ATOM  **
**                                **
***********************************/

Mol_Int_Atom	rul__mol_get_poly_count_atom (Mol_Polyatom poly_val)
{
        assert (rul___mol_is_valid_poly (poly_val));

        return (((Polyatomic_Value *)poly_val)->atomic_element_count);
}



/*********************************
**                              **
**  RUL__MOL_GET_POLY_COUNT_RT  **
**                              **
*********************************/

Mol_Int_Atom rul__mol_get_poly_count_rt (Mol_Polyatom poly_val)
{
	Mol_Int_Atom len;

	assert (rul___mol_is_valid_poly (poly_val));

	len = rul__mol_get_poly_count_atom (poly_val);
	rul__mol_incr_uses (len);
	return (len);
}




/***********************************
**                                **
**  RUL__MOL_GET_POLY_COUNT_LAST  **
**                                **
***********************************/
/* NOTE: to be used on RHS, returns a 1 if zero */
/* ex: modify <id> ^compound-attr[$last] new-value */ 
 
long	rul__mol_get_poly_count_last (Mol_Polyatom poly_val)
{
  long poly_count;

  assert (rul___mol_is_valid_poly (poly_val));

  if ((poly_count = (((Polyatomic_Value *)poly_val)->element_count)) == 0)
    poly_count = 1;

  return poly_count;
}




/***************************
**                        **
**  RUL__MOL_LEN_EQ_PRED  **
**                        **
***************************/

Boolean	rul__mol_len_eq_pred (Molecule poly_val, Molecule atom_val)
{
	long	i;
	Polyatomic_Value *poly_ptr;

	if (poly_val == NULL  ||  atom_val == NULL) return (FALSE);

	assert (rul___mol_is_valid (poly_val));
	assert (rul___mol_is_valid (atom_val));

	if (is_polyatomic (poly_val)  &&  atom_val->type == iatom) {

	    i = ((Integer_Value *)atom_val)->ival;
	    poly_ptr = (Polyatomic_Value *) poly_val;
	    if (i == poly_ptr->element_count) return (TRUE);	    
	}
	return (FALSE);
}




/****************************
**                         **
**  RUL__MOL_LEN_NEQ_PRED  **
**                         **
****************************/

Boolean	rul__mol_len_neq_pred (Molecule poly_val, Molecule atom_val)
{
	long	i;
	Polyatomic_Value *poly_ptr;

	if (poly_val == NULL  ||  atom_val == NULL) return (FALSE);

	assert (rul___mol_is_valid (poly_val));
	assert (rul___mol_is_valid (atom_val));

	if (is_polyatomic (poly_val)  &&  atom_val->type == iatom) {

	    i = ((Integer_Value *)atom_val)->ival;
	    poly_ptr = (Polyatomic_Value *) poly_val;
	    if (i == poly_ptr->element_count) return (FALSE);
	}
	return (TRUE);
}




/***************************
**                        **
**  RUL__MOL_LEN_LT_PRED  **
**                        **
***************************/

Boolean	rul__mol_len_lt_pred (Molecule poly_val, Molecule atom_val)
{
	long	i;
	Polyatomic_Value *poly_ptr;

	if (poly_val == NULL  ||  atom_val == NULL) return (FALSE);

	assert (rul___mol_is_valid (poly_val));
	assert (rul___mol_is_valid (atom_val));

	if (is_polyatomic (poly_val)  &&  atom_val->type == iatom) {

	    i = ((Integer_Value *)atom_val)->ival;
	    poly_ptr = (Polyatomic_Value *) poly_val;
	    if (poly_ptr->element_count < i) return (TRUE);	    
	}
	return (FALSE);
}




/****************************
**                         **
**  RUL__MOL_LEN_LTE_PRED  **
**                         **
****************************/

Boolean	rul__mol_len_lte_pred (Molecule poly_val, Molecule atom_val)
{
	long	i;
	Polyatomic_Value *poly_ptr;

	if (poly_val == NULL  ||  atom_val == NULL) return (FALSE);

	assert (rul___mol_is_valid (poly_val));
	assert (rul___mol_is_valid (atom_val));

	if (is_polyatomic (poly_val)  &&  atom_val->type == iatom) {

	    i = ((Integer_Value *)atom_val)->ival;
	    poly_ptr = (Polyatomic_Value *) poly_val;
	    if (poly_ptr->element_count <= i) return (TRUE);	    
	}
	return (FALSE);
}




/***************************
**                        **
**  RUL__MOL_LEN_GT_PRED  **
**                        **
***************************/

Boolean	rul__mol_len_gt_pred (Molecule poly_val, Molecule atom_val)
{
	long	i;
	Polyatomic_Value *poly_ptr;

	if (poly_val == NULL  ||  atom_val == NULL) return (FALSE);

	assert (rul___mol_is_valid (poly_val));
	assert (rul___mol_is_valid (atom_val));

	if (is_polyatomic (poly_val)  &&  atom_val->type == iatom) {

	    i = ((Integer_Value *)atom_val)->ival;
	    poly_ptr = (Polyatomic_Value *) poly_val;
	    if (poly_ptr->element_count > i) return (TRUE);
	}
	return (FALSE);
}




/****************************
**                         **
**  RUL__MOL_LEN_GTE_PRED  **
**                         **
****************************/

Boolean	rul__mol_len_gte_pred (Molecule poly_val, Molecule atom_val)
{
	long	i;
	Polyatomic_Value *poly_ptr;

	if (poly_val == NULL  ||  atom_val == NULL) return (FALSE);

	assert (rul___mol_is_valid (poly_val));
	assert (rul___mol_is_valid (atom_val));

	if (is_polyatomic (poly_val)  &&  atom_val->type == iatom) {

	    i = ((Integer_Value *)atom_val)->ival;
	    poly_ptr = (Polyatomic_Value *) poly_val;
	    if (poly_ptr->element_count >= i) return (TRUE);	    
	}
	return (FALSE);
}





/********************************
**                             **
**  RUL__MOL_INDEX_VALID_PRED  **
**                             **
********************************/

Boolean	 rul__mol_index_valid_pred (Molecule poly_val, Molecule atom_val)
{
	long	i;
	Polyatomic_Value *poly_ptr;

	if (poly_val == NULL  ||  atom_val == NULL) return (FALSE);

	assert (rul___mol_is_valid (poly_val));
	assert (rul___mol_is_valid (atom_val));

	if (is_polyatomic (poly_val)  &&  atom_val->type == iatom) {

	    i = ((Integer_Value *)atom_val)->ival;
	    poly_ptr = (Polyatomic_Value *) poly_val;
	    if (i > 0  &&  i <= poly_ptr->element_count) return (TRUE);	    
	}
	return (FALSE);
}



/*****************************
**                          **
**  RUL__MOL_CONTAINS_PRED  **
**                          **
*****************************/

Boolean	rul__mol_contains_pred (Molecule mol1, Molecule mol2,
				Boolean (*pred_func) (Molecule, Molecule))
{
	long		i;
	Compound_Value *cpoly_ptr;
	Molecule	atom;
	Boolean		contains;

	if (mol1 == NULL || mol2 == NULL) return (FALSE);

	assert (rul___mol_is_valid (mol1));
	assert (rul___mol_is_valid (mol2));

	if (is_polyatomic (mol1)) {
	    if (is_polyatomic (mol2)) return FALSE;
	    contains = TRUE;
	    atom = mol2;
	    cpoly_ptr = (Compound_Value *) mol1;

  	} else {
	    if (is_atomic (mol2)) return FALSE;
	    contains = FALSE;
	    atom = mol1;
	    cpoly_ptr = (Compound_Value *) mol2;
	}

	if (pred_func != NULL) {
	    /*
	    **  Using some scalar predicate other than identity
	    */
	    if (contains) {
	        for (i=0; i<cpoly_ptr->element_count; i++) {
		    /*  All we need is one that passes the test...  */
		    if ((*pred_func) (cpoly_ptr->values[i], atom))
			return (TRUE);
		}
	    } else {
	        for (i=0; i<cpoly_ptr->element_count; i++) {
		    /*  contained within ...  */
		    if ((*pred_func) (atom, cpoly_ptr->values[i]))
			return (TRUE);
		}
	    }

	} else {
	    /*
	    **  Identity predicate
	    */
	    return rul___mol_identity_containment (cpoly_ptr, atom, TRUE);
	}
	return (FALSE);
}



/*********************************
**                              **
**  RUL__MOL_NOT_CONTAINS_PRED  **
**                              **
*********************************/

Boolean	rul__mol_not_contains_pred (Molecule mol1, Molecule mol2,
				Boolean (*pred_func) (Molecule, Molecule))
{
	long		i;
	Compound_Value *cpoly_ptr;
	Mol_Atom	atom;
	Boolean		contains;

	if (mol1 == NULL || mol2 == NULL) return (FALSE);

	assert (rul___mol_is_valid (mol1));
	assert (rul___mol_is_valid (mol2));

	if (is_polyatomic (mol1)) {
	    if (is_polyatomic (mol2)) return FALSE;
	    contains = TRUE;
	    atom = mol2;
	    cpoly_ptr = (Compound_Value *) mol1;

  	} else {
	    if (is_atomic (mol2)) return FALSE;
	    contains = FALSE;
	    atom = mol1;
	    cpoly_ptr = (Compound_Value *) mol2;
	}

	if (pred_func != NULL) {
	    /*
	    **  Using some scalar predicate other than identity
	    */
	    if (contains) {
	        for (i=0; i<cpoly_ptr->element_count; i++) {
		    /*  All we need is one that fails the test...  */
		    if ((*pred_func) (cpoly_ptr->values[i], atom))
			return (FALSE);
		}
	    } else {
	        for (i=0; i<cpoly_ptr->element_count; i++) {
		    /*  not contained within ...  */
		    if ((*pred_func) (atom, cpoly_ptr->values[i]))
			return (FALSE);
		}
	    }

	} else {
	    /*
	    **  Identity predicate
	    */
	    return rul___mol_identity_containment (cpoly_ptr, atom, FALSE);
	}
	return (TRUE);
}




/*
**  The following definitions are used in the bit-wise containment test
*/

#if ULONG_MAX == 0xFFFFFFFF		/* i.e. BITS_PER_ULONG == 32 */
#define BIT_INDEX_TO_WORD_INDEX(bit_index)	((bit_index) >> 5)
#define RAW_INDEX_TO_BIT_INDEX(raw_index,vbits)	((raw_index) & ((vbits) - 1))	
#else
#define BIT_INDEX_TO_WORD_INDEX(bit_index)	((bit_index) / BITS_PER_ULONG)
#define RAW_INDEX_TO_BIT_INDEX(raw_index,vbits)	((raw_index) % (vbits))	
#endif

static unsigned long SL_mask_vector[BITS_PER_ULONG];




/************************************
**                                 **
**  RUL___MOL_INIT_CONTAINS_MASKS  **
**                                 **
************************************/

void rul___mol_init_contains_masks (void)
{
	long i;

	for (i=0; i<BITS_PER_ULONG; i++) {
	    SL_mask_vector[i] = 1l << i;
	}
}




/*************************************
**                                  **
**  RUL___MOL_IDENTITY_CONTAINMENT  **
**                                  **
*************************************/

Boolean rul___mol_identity_containment (Compound_Value *cpoly_ptr, 
			Mol_Atom atom, Boolean return_if_match_succeeds)
{
	long i, bit_index, word_index, bit_in_word, right_bit;

	/* If compound is empty, no atom is contained in it. */
	if (cpoly_ptr->element_count == 0)
	    return !return_if_match_succeeds;

	if (cpoly_ptr->containment_vector_length < 0) {
	    /*  containment vector has never been computed  */
	    if (mol_fill_containee_vector (cpoly_ptr, atom)) {
		return (return_if_match_succeeds);
	    }
	    return (!return_if_match_succeeds);
	}

	bit_index = rul___mol_get_containee_index (atom);
	if (bit_index == MOL__C_NOT_CONTAINED) {
	    return (!return_if_match_succeeds);
	}

	/*  
	**  Do the bit-wise containment test
	*/
	bit_index = RAW_INDEX_TO_BIT_INDEX(bit_index,
			cpoly_ptr->containment_vector_bits);
	if (bit_index < BITS_PER_ULONG) {
	    /*  in the first longword of the vector  */
	    right_bit = cpoly_ptr->containment_bit_vector[0] & 
			SL_mask_vector[bit_index];
	} else {
	    word_index = BIT_INDEX_TO_WORD_INDEX(bit_index);
	    bit_in_word = bit_index - (word_index * BITS_PER_ULONG);
	    right_bit = cpoly_ptr->containment_bit_vector[word_index] & 
			SL_mask_vector[bit_in_word];
	}
	if (right_bit == 0) {
	    return (!return_if_match_succeeds);
	}

	/*
	**  As a last resort, compare every element
	*/
	for (i=0; i<cpoly_ptr->element_count; i++) {
	    if (atom == cpoly_ptr->values[i])
		return (return_if_match_succeeds);
	}
	return (!return_if_match_succeeds);
}



/********************************
**                             **
**  MOL_FILL_CONTAINEE_VECTOR  **
**                             **
********************************/

static Boolean mol_fill_containee_vector (Compound_Value *cpoly_ptr,
					  Mol_Atom contained_atom)
{
	long i;
	unsigned long bit_index, word_index, bit_in_word;
	Boolean found_atom = FALSE;

	/*  A negative length is the indicator that this compound's
	**  containment vector has never been computed.
	*/
	assert (cpoly_ptr->containment_vector_length < 0);

	cpoly_ptr->containment_vector_length *= -1;
	for (i=0; i<cpoly_ptr->element_count; i++) {
	    if (cpoly_ptr->values[i] == contained_atom) found_atom = TRUE;
	    /* Prevent RAW_INDEX_TO_BIT_INDEX() from dividing by 0. */
	    assert (cpoly_ptr->containment_vector_bits != 0);
	    bit_index = RAW_INDEX_TO_BIT_INDEX(
	    	    rul___mol_give_containee_index (cpoly_ptr->values[i]),
		    cpoly_ptr->containment_vector_bits);
	    word_index = BIT_INDEX_TO_WORD_INDEX(bit_index);
	    bit_in_word = bit_index - (word_index * BITS_PER_ULONG);
	    cpoly_ptr->containment_bit_vector[word_index] |= 
		    SL_mask_vector[bit_in_word];
	}
	return (found_atom);
}





/*************************************
**                                  **
**  RUL___MOL_POSSIBLE_CONTAINMENT  **
**                                  **
*************************************/

Boolean rul___mol_possible_containment (Compound_Value *cpoly_ptr,
					Mol_Atom atom)
{
	/*
	**  Returns FALSE when there is no possibility that the
	**  specified atom is contained within the specified compound.
	**
	**  If it returns TRUE, the atom may or may not be present.
	*/
	long bit_index, word_index, bit_in_word, right_bit;

	/* If compound is empty, no atom is contained in it. */
	if (cpoly_ptr->element_count == 0)
	    return FALSE;

	if (cpoly_ptr->containment_vector_length < 0) {
	    /*  containment vector has never been computed  */
	    if (mol_fill_containee_vector (cpoly_ptr, atom)) return (TRUE);
	    return (FALSE);
	}

	if (rul___mol_get_containee_index (atom) == MOL__C_NOT_CONTAINED) {
	    return (FALSE);
	}

	/*  Try the bit containment test  */
	bit_index = RAW_INDEX_TO_BIT_INDEX(bit_index,
				cpoly_ptr->containment_vector_bits);
	word_index = BIT_INDEX_TO_WORD_INDEX(bit_index);
	bit_in_word = bit_index - (word_index * BITS_PER_ULONG);
	right_bit = cpoly_ptr->containment_bit_vector[word_index] & 
				SL_mask_vector[bit_in_word];
	if (right_bit == 0) {
	    return (FALSE);
	}

	return (TRUE);
}




/****************************
**                         **
**  RUL__MOL_HAS_KEY_PRED  **
**                         **
****************************/

#if 0	/*?*  NYI  */
Boolean	rul__mol_has_key_pred (Molecule poly_val, Molecule atom_val)
{
	Table_Value *tpoly_ptr;

	if (poly_val == NULL  ||  atom_val == NULL) return (FALSE);

	assert (rul___mol_is_valid (poly_val));
	assert (rul___mol_is_valid_atom (atom_val));

	if (poly_val->type == tpoly  &&  is_atomic(atom_val)) {

	    tpoly_ptr = (Table_Value *) poly_val;
	}
	return (FALSE);
}
#endif




/*******************************
**                            **
**  RUL__MOL_HAS_NO_KEY_PRED  **
**                            **
*******************************/

#if 0	/*?*  NYI  */
Boolean	rul__mol_has_no_key_pred (Molecule poly_val, Molecule atom_val)
{
	Table_Value *tpoly_ptr;

	if (poly_val == NULL  ||  atom_val == NULL) return (FALSE);

	assert (rul___mol_is_valid (poly_val));
	assert (rul___mol_is_valid (atom_val));

	if (poly_val->type == tpoly  &&  is_atomic(atom_val)) {

	    tpoly_ptr = (Table_Value *) poly_val;

	    /*?* NYI */
	}
	return (TRUE);
}
#endif
