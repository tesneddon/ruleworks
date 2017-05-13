/****************************************************************************
**                                                                         **
**            O P S _ M O L _ M O L E C U L E _ T A B L E . C              **
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
**	This module contains the functions (in most cases,
**	internal to the molecule subsystem) which operate
**	directly on the molecule table.
**
**	These routines are used at both compile time and run time.
**
**  MODIFIED BY:
**	DEC	Digital Equipment Corporation
**	CPQ	Compaq Computer Corporation
**
**  MODIFICATION HISTORY:
**
**	6-Mar-1992	DEC	Initial version
**  20-Jul-1998	DEC	Integers are no longer permanent
**  10-Aug-1998	DEC	ref_count_trace functions added
**					rul__mol_print_molecule function added
**					rul__mol_print_mem_cons function added
**	01-Dec-1999	CPQ	Release with GPL
*/


#include <common.h>
#include <i18n.h>
#include <mol_p.h>
#include <mol.h>
#include <ios.h>
#include <rts_msg.h>
#include <msg.h>


        /* include special system header files */
#include <math.h>
#include <limits.h>

#if !defined(CHAR_BITS)  &&  (defined(__ultrix__) || defined(__osf__))
#include <sys/types.h>  /* for the definition of constant macro NBBY */
#ifdef NBBY
#define CHAR_BITS NBBY
#endif
#endif




/*	Special Permanent Molecule Constants	  */

static Mol_Symbol SA_nil;			/*  NIL		  */
static Mol_Symbol SA_root;			/*  $ROOT	  */
static Mol_Symbol SA_crlf;			/*  (CRLF)	  */
static Mol_Symbol SA_id;			/*  $ID		  */
static Mol_Symbol SA_instance_of;  		/*  $INSTANCE-OF  */

static Mol_Int_Atom SA_integer_zero;		/*  0		  */
static Mol_Dbl_Atom SA_double_zero;		/*  0.0		  */
static Mol_Instance_Id SA_instance_zero;	/*  #0		  */
static Mol_Opaque SA_opaque_null;		/*  %x0		  */
static Mol_Compound SA_compound_empty;		/*  (compound)	  */



	/*
	**	We may want to modify the size of the molecule
	**	table depending on the specific architecture.
	**	For instance, this table could be considerably
	**	smaller for limited memory architectures.
	**
	**	This entire subsystem is implemented implicitly
	**	assuming that the table size is some power
	**	2 in size, and if the size is changed, then
	**	so too must the INDEX_SHIFT constants specified below.
	**	It is also implemented assuming that the entire index
	**	will fit into a longword.
	**
	**
	**	INDEX_SHIFT is the number of bits which
	**	are not needed out of the total number of
	**	bits in a longword worth of information.
	**
	**	Example of alternate table sizes:
	**
	**		TABLE_SIZE   8192
	**		INDEX_SHIFT  19
	**
	**		TABLE_SIZE   32768
	**		INDEX_SHIFT  17
	*/
#if defined(__VMS) || defined(__UNIX) || defined(__osf__)
#define MOLECULE_TABLE_SIZE   16384
#define MOLECULE_TABLE_SIZE_BITS 14 /* log2(MOLECULE_TABLE_SIZE) */
#else
#define MOLECULE_TABLE_SIZE   4096
#define MOLECULE_TABLE_SIZE_BITS 12 /* log2(MOLECULE_TABLE_SIZE) */
#endif

#ifndef CHAR_BITS
#define CHAR_BITS 8			    /* Number of Bits Per Byte */
#endif

#define MOLECULE_INDEX_SHIFT (sizeof(Molecule)*CHAR_BITS - MOLECULE_TABLE_SIZE_BITS)
#define HASH_NUM_TO_INDEX(hsh)	(((unsigned long) hsh)>>MOLECULE_INDEX_SHIFT)


	/*
	**	The permanence threshold is the number of
	**	references which, if exceeded, cause a
	**	transient molecule to be converted to permanent.
	**	This number must be smaller than 1/2 the capacity of
	**	the ref_count field in the Molecular_Value structure.
	*/
#define	PERMANENCE_THRESHOLD	8000
#define PERMANENT		-1



	/*
	**	The molecule table itself is an array
	**	of pointers to molecule structures.
	**	Overflows, when needed are chained off
	**	the structures resident in each bucket.
	*/
static  Molecule  *SA__molecule_table;


	/*
	**	This variable stores the next wmo number
	**	to be used.  It monotonically increases
	**	except when the back command is used.
	*/
static	long	SL__next_wmo_number;


	/*
	**	Similarly, this variable is the monotonically 
	**	increasing counter used for the eq-containment test.
	*/
static  unsigned long   SL__next_containee_index;


static	Boolean	arrays_equal (long len, void *array_1[], void *array_2[]);
static  void	free_molecule (Molecule mol);
static  Mol_Instance_Id mol_make_null_instance_id (void);
static	void	rul__mol_print_molecule (Molecule mol);
int ref_count_trace = 0;




/********************
**                 **
**  RUL__MOL_INIT  **
**                 **
********************/

void
rul__mol_init (void)
{
	long	i;
	static	Boolean mol_inited = FALSE;

	if (!mol_inited) {
	    mol_inited = TRUE;

	    SA__molecule_table = (Molecule *)
		    rul__mem_calloc (MOLECULE_TABLE_SIZE, sizeof(Molecule));

	    SL__next_wmo_number = 1;
	    SL__next_containee_index = 1;
	    rul___mol_init_contains_masks();

	    /*
	    **  Create the predefined permanent constants
	    */
	    SA_nil = rul__mol_make_symbol ("NIL");
	    rul__mol_mark_perm (SA_nil);

	    SA_root = rul__mol_make_symbol ("$ROOT");
	    rul__mol_mark_perm (SA_root);

	    SA_crlf = rul__mol_make_symbol ("\n");
	    rul__mol_mark_perm (SA_crlf);

	    SA_id = rul__mol_make_symbol ("$ID");
	    rul__mol_mark_perm (SA_id);

	    SA_instance_of = rul__mol_make_symbol ("$INSTANCE-OF");
	    rul__mol_mark_perm (SA_instance_of);

	    SA_integer_zero = rul__mol_make_int_atom (0);
	    rul__mol_mark_perm (SA_integer_zero);

	    SA_double_zero = rul__mol_make_dbl_atom (0.0);
	    rul__mol_mark_perm (SA_double_zero);

	    SA_instance_zero = mol_make_null_instance_id ();
	    rul__mol_mark_perm (SA_instance_zero);

	    SA_opaque_null = rul__mol_make_opaque (NULL);
	    rul__mol_mark_perm (SA_opaque_null);

	    SA_compound_empty = rul__mol_make_comp (0);
	    rul__mol_mark_perm (SA_compound_empty);
	}
}




/********************
**                 **
**  FREE_MOLECULE  **
**                 **
********************/

static void
free_molecule (Molecule mol)
{
	long i;
	Compound_Value *cpoly_ptr;
	Symbol_Value	*satom_ptr;

	if (mol->type == satom) {
	    /*
	    **    For all symbols with an associated
	    **    soundex_symbol need to decrement the
	    **    the references to the soundex_symbol
	    */
	    satom_ptr = (Symbol_Value *) mol;
	    if (satom_ptr->soundex_symbol) {
	        rul__mol_decr_uses (satom_ptr->soundex_symbol);
	    }

	} else if (mol->type == cpoly) {
	    /*
	    **    For compounds, we need to decrement
	    **    the references to all the contained atoms
	    */
	    cpoly_ptr = (Compound_Value *) mol;
	    rul__mol_decr_uses (cpoly_ptr->atomic_element_count);
	    for (i=0; i<cpoly_ptr->element_count; i++) {
		rul__mol_decr_uses ((Molecule) cpoly_ptr->values[i]);
	    }
	}
#ifdef NDEBUG
	rul__mem_free (mol);
#endif
}



/***************************
**                        **
**  RUL__MOL_MAKE_SYMBOL  **
**                        **
***************************/

Mol_Symbol
rul__mol_make_symbol (char *str)
{
	long		num, index, len;
	Symbol_Value   *satom_ptr;
	char            buf[RUL_C_MAX_SYMBOL_SIZE+1];
	char           *buf_ptr = buf;

	num = rul__mol_string_to_hash_num (str);
        index = HASH_NUM_TO_INDEX(num);
	len = strlen (str);

	satom_ptr = (Symbol_Value *) SA__molecule_table[index];
	while (satom_ptr != NULL) {
	    if (satom_ptr->type == satom  &&
		len == rul___mol_printform_length ((Molecule) satom_ptr)  &&
		strcmp (satom_ptr->sval, str) == 0)  break;
	    satom_ptr = (Symbol_Value *) satom_ptr->next;
	}

	if (satom_ptr != NULL) {
	    /*
	    **    If there already is a symbol which corresponds
	    **    to this particular string, just increment its use.
	    */
#ifndef NDEBUG
	    if (satom_ptr->ref_count == 0) {
		/*
		**  This is a "born-again" symbol which wasn't really
		**  freed.  To "bring it back to life", we must undo what
		**  free_molecule did by incrementing the reference count
		**  on its soundex_symbol, if it has one (in addition to
		**  incrementing the reference count on the symbol).
		*/
		if (satom_ptr->soundex_symbol != NULL) {        
		    rul__mol_incr_uses (satom_ptr->soundex_symbol);
		}
	    }
#endif
	    rul__mol_incr_uses ((Molecule) satom_ptr);

	} else {
	    /*
	    **    If there is not already a symbol which corresponds
	    **    to this string, then allocate the structure (plus
	    **    room for the string itself), fill it, and add it to
	    **    the molecule table.
	    */
            if (len > RUL_C_MAX_SYMBOL_SIZE) {
	      len = RUL_C_MAX_SYMBOL_SIZE;
	      strncpy (buf, str, len);
	      buf[len] = '\0';
	      rul__msg_print (RTS_SYMTRUNC, buf);
	    }
	    else
	      buf_ptr = str;

	    satom_ptr = (Symbol_Value *) 
		rul__mem_malloc (len + 1 + sizeof (Symbol_Value));

	    if (satom_ptr != NULL) {
		satom_ptr->type = satom;
		satom_ptr->hash_number = num;
		satom_ptr->ref_count = 1;
		satom_ptr->soundex_symbol = NULL;
		satom_ptr->printform_length = len;
		satom_ptr->readform_length = MOL__C_NOT_A_VALID_LENGTH;
		satom_ptr->containee_index = MOL__C_NOT_CONTAINED;

		/*    Save the string, which follows the Symbol_Value    */
		satom_ptr->sval = (char *) (satom_ptr + 1);
		strcpy (satom_ptr->sval, buf_ptr);

		/*    Insert it into the table    */
		satom_ptr->next = SA__molecule_table[index];
		SA__molecule_table[index] = (Molecule) satom_ptr;

	    } else {
		/*  malloc failure in rul__make_symbol  */
	        return (NULL);
	    }
	}
	return ((Mol_Symbol) satom_ptr);
}



/*************************
**                      **
**  RUL__MOL_MARK_PERM  **
**                      **
*************************/

void
rul__mol_mark_perm (Molecule mol)
{
  long i;
  Compound_Value *cpoly_ptr;

        assert (rul___mol_is_valid (mol));

	if (mol->type == cpoly) {
	    /*
	    **    For compounds, we need to make
	    **    each contained atom permanent also.
	    */
	    cpoly_ptr = (Compound_Value *) mol;
	    rul__mol_mark_perm (cpoly_ptr->atomic_element_count);
	    for (i=0; i<cpoly_ptr->element_count; i++) {
		rul__mol_mark_perm ((Molecule) cpoly_ptr->values[i]);
	    }
	}
	mol->ref_count = PERMANENT;
}



/*****************************
**                          **
**  RUL__MOL_MAKE_INT_ATOM  **
**                          **
*****************************/

Mol_Int_Atom
rul__mol_make_int_atom (long an_int)
{
	long		num, index;
	Integer_Value	*iatom_ptr;

	num = rul__mol_integer_to_hash_num (an_int);
	index = HASH_NUM_TO_INDEX(num);


	iatom_ptr = (Integer_Value *) SA__molecule_table[index];
	while (iatom_ptr != NULL) {
	    if (iatom_ptr->type == iatom  &&
		iatom_ptr->ival == an_int)  break;
	    iatom_ptr = (Integer_Value *) iatom_ptr->next;
	}

	if (iatom_ptr != NULL) {
	    /*
	    **    If there already is a integer atom which corresponds
	    **    to this particular value, just increment its use.
	    */
	    rul__mol_incr_uses ((Molecule) iatom_ptr);

	} else {
	    /*
	    **    If there is not already a integer atom which corresponds
	    **    to this value, then allocate the structure, fill it, and
	    **    add it to the molecule table.
	    */
	    iatom_ptr = (Integer_Value *) 
				rul__mem_malloc (sizeof (Integer_Value));

	    if (iatom_ptr != NULL) {
		iatom_ptr->type = iatom;
		iatom_ptr->hash_number = num;
		iatom_ptr->ref_count = 1; /* SB: no longer PERMANENT; */
		iatom_ptr->ival = an_int;
		iatom_ptr->printform_length = MOL__C_NOT_A_VALID_LENGTH;
		iatom_ptr->readform_length  = MOL__C_NOT_A_VALID_LENGTH;
		iatom_ptr->containee_index  = MOL__C_NOT_CONTAINED;

		/*    Insert it into the table    */
		iatom_ptr->next = SA__molecule_table[index];
		SA__molecule_table[index] = (Molecule) iatom_ptr;

	    } else {
		/*  malloc failure in rul__make_int_atom  */
	        return (NULL);
	    }
	}
	return ((Mol_Int_Atom) iatom_ptr);
}



/*****************************
**                          **
**  RUL__MOL_MAKE_DBL_ATOM  **
**                          **
*****************************/

Mol_Dbl_Atom
rul__mol_make_dbl_atom (double a_dbl)
{
	long		num, index;
	Double_Value	*datom_ptr;

	num = rul__mol_double_to_hash_num (a_dbl);
        index = HASH_NUM_TO_INDEX(num);

	datom_ptr = (Double_Value *) SA__molecule_table[index];
	while (datom_ptr != NULL) {
	    if (datom_ptr->type == datom  &&
		datom_ptr->dval == a_dbl)  break;
	    datom_ptr = (Double_Value *) datom_ptr->next;
	}

	if (datom_ptr != NULL) {
	    /*
	    **    If there already is a float atom which corresponds
	    **    to this particular value, just increment its use.
	    */
	    rul__mol_incr_uses ((Molecule) datom_ptr);

	} else {
	    /*
	    **    If there is not already a float atom which corresponds
	    **    to this value, then allocate the structure, fill it,
	    **    and add it to the molecule table.
	    */
	    datom_ptr = (Double_Value *)
				rul__mem_malloc (sizeof (Double_Value));

	    if (datom_ptr != NULL) {
		datom_ptr->type = datom;
		datom_ptr->hash_number = num;
		datom_ptr->ref_count = 1;
		datom_ptr->dval = a_dbl;
		datom_ptr->printform_length = MOL__C_NOT_A_VALID_LENGTH;
		datom_ptr->readform_length  = MOL__C_NOT_A_VALID_LENGTH;
		datom_ptr->containee_index  = MOL__C_NOT_CONTAINED;

		/*    Insert it into the table    */
		datom_ptr->next = SA__molecule_table[index];
		SA__molecule_table[index] = (Molecule) datom_ptr;

	    } else {
		/*  malloc failure in rul__make_dbl_atom  */
	        return (NULL);
	    }
	}
	return ((Mol_Dbl_Atom) datom_ptr);
}



/***************************
**                        **
**  RUL__MOL_MAKE_OPAQUE  **
**                        **
***************************/

Mol_Opaque
rul__mol_make_opaque (void * an_opaque)
{
	long		num, index;
	Opaque_Value	*oatom_ptr;

	num = rul__mol_address_to_hash_num (an_opaque);
        index = HASH_NUM_TO_INDEX(num);

	oatom_ptr = (Opaque_Value *) SA__molecule_table[index];
	while (oatom_ptr != NULL) {
	    if (oatom_ptr->type == oatom  &&
		oatom_ptr->oval == an_opaque)  break;
	    oatom_ptr = (Opaque_Value *) oatom_ptr->next;
	}

	if (oatom_ptr != NULL) {
	    /*
	    **    If there already is a float atom which corresponds
	    **    to this particular value, just increment its use.
	    */
	    rul__mol_incr_uses ((Molecule) oatom_ptr);

	} else {
	    /*
	    **    If there is not already a opaque atom which corresponds
	    **    to this value, then allocate the structure, fill it,
	    **    and add it to the molecule table.
	    */
	    oatom_ptr = (Opaque_Value *) 
				rul__mem_malloc (sizeof (Opaque_Value));

	    if (oatom_ptr != NULL) {
		oatom_ptr->type = oatom;
		oatom_ptr->hash_number = num;
		oatom_ptr->ref_count = 1;
		oatom_ptr->oval = an_opaque;
		oatom_ptr->printform_length = MOL__C_NOT_A_VALID_LENGTH;
		oatom_ptr->readform_length  = MOL__C_NOT_A_VALID_LENGTH;
		oatom_ptr->containee_index  = MOL__C_NOT_CONTAINED;

		/*    Insert it into the table    */
		oatom_ptr->next = SA__molecule_table[index];
		SA__molecule_table[index] = (Molecule) oatom_ptr;

	    } else {
		/*  malloc failure in rul__make_opaque  */
	        return (NULL);
	    }
	}
	return ((Mol_Opaque) oatom_ptr);
}



/********************************
**                             **
**  RUL__MOL_MAKE_INSTANCE_ID  **
**                             **
********************************/

Mol_Instance_Id
rul__mol_make_instance_id (Object instance_ptr)
{
	long		   num, index;
	Instance_Id_Value *watom_ptr;

	/*
	**    Instance_Id_Values, are by their very nature unique,
	**    and therefore we don't need to check if there
	**    already exists an identical Molecule in the table.
	**
	**    Thus all that needs to be done is allocate the
	**    structure, fill it, give it an id number,
	**    and then add it to the molecule table.
	*/

        watom_ptr = (Instance_Id_Value *)
			rul__mem_malloc (sizeof (Instance_Id_Value));

	if (watom_ptr != NULL) {
            watom_ptr->type = watom;
            watom_ptr->ref_count = 1;
            watom_ptr->instance_ptr = instance_ptr;

	    watom_ptr->wval = SL__next_wmo_number;
	    SL__next_wmo_number++;

	    watom_ptr->printform_length = MOL__C_NOT_A_VALID_LENGTH;
	    watom_ptr->readform_length  = MOL__C_NOT_A_VALID_LENGTH;
	    watom_ptr->containee_index  = MOL__C_NOT_CONTAINED;

	    num = rul___mol_wmo_id_to_hash_num (watom_ptr->wval);
            index = HASH_NUM_TO_INDEX(num);
	    watom_ptr->hash_number = num;

            /*    Insert it into the table    */
            watom_ptr->next = SA__molecule_table[index];
            SA__molecule_table[index] = (Molecule) watom_ptr;

        } else {
	    /*  malloc failure in rul__mol_make_instance_id  */
	    return (NULL);
	}
	return ((Mol_Instance_Id) watom_ptr);
}



/**********************************
**                               **
**  RUL__MOL_MAKE_INSTANCE_ATOM  **
**                               **
**********************************/

Mol_Instance_Id
rul__mol_make_instance_atom (long id_value)
{
        Mol_Instance_Id    id_atom;
	long		   num, index;
	Instance_Id_Value *watom_ptr;

	id_atom = rul__mol_id_num_to_instance_id (id_value);
	if (id_atom) {
	  rul__mol_incr_uses (id_atom);
	  return id_atom;
	}

        watom_ptr = (Instance_Id_Value *)
			rul__mem_malloc (sizeof (Instance_Id_Value));

	if (watom_ptr != NULL) {
            watom_ptr->type = watom;
            watom_ptr->ref_count = 1;
            watom_ptr->instance_ptr = NULL;
	    watom_ptr->wval = id_value;
	    watom_ptr->printform_length = MOL__C_NOT_A_VALID_LENGTH;
	    watom_ptr->readform_length  = MOL__C_NOT_A_VALID_LENGTH;
	    watom_ptr->containee_index  = MOL__C_NOT_CONTAINED;

	    num = rul___mol_wmo_id_to_hash_num (watom_ptr->wval);
            index = HASH_NUM_TO_INDEX(num);
	    watom_ptr->hash_number = num;

            /*    Insert it into the table    */
            watom_ptr->next = SA__molecule_table[index];
            SA__molecule_table[index] = (Molecule) watom_ptr;
        }

	return ((Mol_Instance_Id) watom_ptr);
}



/********************************
**                             **
**  MOL_MAKE_NULL_INSTANCE_ID  **
**                             **
********************************/

static Mol_Instance_Id
mol_make_null_instance_id (void)
{
	long		   num, index;
	Instance_Id_Value *watom_ptr;

	/*  Make the special WME-ID,  #0  */

        watom_ptr = (Instance_Id_Value *)
			rul__mem_malloc (sizeof (Instance_Id_Value));

	assert (watom_ptr != NULL);

        watom_ptr->type = watom;
        watom_ptr->ref_count = 1;
        watom_ptr->instance_ptr = NULL;
	watom_ptr->wval = 0;
	watom_ptr->printform_length = MOL__C_NOT_A_VALID_LENGTH;
	watom_ptr->readform_length  = MOL__C_NOT_A_VALID_LENGTH;
	watom_ptr->containee_index  = MOL__C_NOT_CONTAINED;

	num = rul___mol_wmo_id_to_hash_num (0);
        index = HASH_NUM_TO_INDEX(num);
	watom_ptr->hash_number = num;

        /*    Insert it into the table    */
        watom_ptr->next = SA__molecule_table[index];
        SA__molecule_table[index] = (Molecule) watom_ptr;

	return ((Mol_Instance_Id) watom_ptr);
}



/********************************
**                             **
**  RUL__MOL_NEXT_INSTANCE_ID  **
**                             **
********************************/

Mol_Instance_Id
rul__mol_next_instance_id (Mol_Instance_Id inst_id)
{
    Mol_Instance_Id      next;
    long                 num;

    if (inst_id == NULL)
      num = 1;
    else
      num = ((Instance_Id_Value *) inst_id)->wval + 1;

    while (num < SL__next_wmo_number) {
      next = rul__mol_id_num_to_instance_id (num);
      if (next) return (next);
      num += 1;
    }

    return (NULL);
}



/*************************************
**                                  **
**  RUL__MOL_ID_NUM_TO_INSTANCE_ID  **
**                                  **
*************************************/

Mol_Instance_Id
rul__mol_id_num_to_instance_id (long a_wmo_number)
{
	long		   num, index;
	Instance_Id_Value *watom_ptr;

	num = rul___mol_wmo_id_to_hash_num (a_wmo_number);
	index = HASH_NUM_TO_INDEX(num);

	watom_ptr = (Instance_Id_Value *) SA__molecule_table[index];
	while (watom_ptr != NULL) {
	    if (watom_ptr->type == watom  &&
		watom_ptr->wval == a_wmo_number)  break;
	    watom_ptr = (Instance_Id_Value *) watom_ptr->next;
	}

#ifndef NDEBUG
	if (watom_ptr != NULL)
	  if ((Mol_Instance_Id) watom_ptr->ref_count == 0)
	    return (NULL);
#endif
	return ((Mol_Instance_Id) watom_ptr);
}



/**********************************
**                               **
**  RUL__MOL_UNMAKE_INSTANCE_ID  **
**                               **
**********************************/

void
rul__mol_unmake_instance_id (Mol_Instance_Id mol)
{
	/*
	**    This function should be used ONLY from in the
	**    BACK debugger command.  If this was the last
	**    reference to this wmo_id, and the wmo number
	**    is the highest used, then unuse this molecule,
	**    and decrement the global wmo_number counter.
	*/
        Instance_Id_Value *a_watom = (Instance_Id_Value *) mol;

	assert (rul___mol_is_valid_atom_of_type (mol, watom));

	if (mol->ref_count == 1) {
	    if (a_watom->wval == (SL__next_wmo_number - 1)) {
		/*
		**   This is indeed the last reference.
		*/
		SL__next_wmo_number--;
		rul__mol_decr_uses (mol);

#ifndef NDEBUG
	    } else {

		rul__ios_printf ( RUL__C_STD_ERR, "\n Internal Error:  attempt to unmake an instance-id");
		rul__ios_printf ( RUL__C_STD_ERR, " out of proper order.\n");
#endif
	    }
#ifndef NDEBUG
	} else {

	    rul__ios_printf ( RUL__C_STD_ERR, "\n Inernal Error:  attempt to unmake instance-id");
	    rul__ios_printf ( RUL__C_STD_ERR, " on watom with remaining references.\n");
#endif
        }
}



/***************************
**                        **
**  RUL___MOL_MAKE_CPOLY  **
**                        **
***************************/

Mol_Compound
rul___mol_make_cpoly (long atom_count, Molecule atom_array[])
{
	/*
	**    Given a flattened array of atoms, find
	**    the compound value that corresponds to that
	**    set of values, or make a new compound value.
	*/
	Compound_Value	*cpoly_ptr;
	long		i, num, index, co_len;

	/*
	**    First, see if this compound already exists.
	*/
	num = rul___mol_mols_to_hash_num_vec (atom_count, atom_array);
        index = HASH_NUM_TO_INDEX(num);

	cpoly_ptr = (Compound_Value *) SA__molecule_table[index];
	while (cpoly_ptr != NULL) {
	    if (cpoly_ptr->type == cpoly  &&
		atom_count == cpoly_ptr->element_count  &&
		arrays_equal (atom_count,
				(void **)cpoly_ptr->values,
				(void **)atom_array))
		    break;  /*  Found it !  */
	    cpoly_ptr = (Compound_Value *) cpoly_ptr->next;
	}

	if (cpoly_ptr != NULL) {
	    /*
	    **    If a compound containing this set of values in
	    **	  this exact order already exists as a molecule,
	    **    then just increment its references.
	    **
	    **    Note, do not increment the references to the values
	    **    contained in the compound, except when the compound
	    **    is initially created.
	    */
#ifndef NDEBUG
	    if (cpoly_ptr->ref_count == 0) {
		/*
		**  This is a "born-again" compound which wasn't really
		**  freed.  To "bring it back to life", we must undo what
		**  free_molecule did by incrementing the reference count
		**  on all the contained molecules (in addition to 
		**  incrementing the reference count on the compound).
		*/
	        rul__mol_incr_uses (cpoly_ptr->atomic_element_count);
	        for (i=0; i<cpoly_ptr->element_count; i++) {
		    rul__mol_incr_uses ((Molecule) cpoly_ptr->values[i]);
	        }
	     }
#endif
	  rul__mol_incr_uses ((Molecule) cpoly_ptr);

	} else {
	    /*
	    **    If there is not already a compound identical to this
	    **    to this one, then allocate the structure (plus room
	    **    for the actual values, and for the containee bit vector),
	    **    fill the value vector, and add it to the molecule table.
	    */

	    /*  First determine the length for the containee vector  */
	    if (atom_count == 0) {
		co_len = 0;
	    } else {
		co_len = 1 + atom_count / 0.25 / BITS_PER_ULONG;
	    }

	    /*  Then allocate the single composite object  */
	    cpoly_ptr = (Compound_Value *) 
		rul__mem_malloc (
			sizeof (Compound_Value) +
			(atom_count * sizeof(Molecule)) +
			(co_len * sizeof(unsigned long)));

	    if (cpoly_ptr != NULL) {
		cpoly_ptr->type = cpoly;
		cpoly_ptr->hash_number = num;
		cpoly_ptr->ref_count = 1;

		cpoly_ptr->element_count = atom_count;
		cpoly_ptr->atomic_element_count =
				rul__mol_make_int_atom (atom_count);

		/*
		**  Save the set of values into the array (which 
		**  immediately follows the Compound_Value
		**  in the same allocation.
		*/
		cpoly_ptr->values = (Molecule *) (cpoly_ptr + 1);
		for (i=0; i<atom_count; i++) {
		    cpoly_ptr->values[i] = atom_array[i];
		    rul__mol_incr_uses ((Molecule) atom_array[i]);
		}

		/*
		**  The containment vector follows the atom array,
		**  also in the same allocation.
		**
		**  The negative length is the sign that this 
		**  containee vector has never been filled.
		*/
		cpoly_ptr->containment_vector_length = -1 * co_len;
		cpoly_ptr->containment_vector_bits = co_len * BITS_PER_ULONG;
		cpoly_ptr->containment_bit_vector =
                        (unsigned long *) &(cpoly_ptr->values[atom_count]);

		/*
		**    Now, insert this new compound into the molecule table
		*/
		cpoly_ptr->next = SA__molecule_table[index];
		SA__molecule_table[index] = (Mol_Compound) cpoly_ptr;

	    } else {
		/*  malloc failure in rul___make_cpoly  */
	        return (NULL);
	    }
	}
	return ((Mol_Compound) cpoly_ptr);
}



/************************************
**                                 **
**  RUL___MOL_GET_CONTAINEE_INDEX  **
**                                 **
************************************/




/************************************
**                                 **
**  RUL___MOL_GET_CONTAINEE_INDEX  **
**                                 **
************************************/

unsigned long
rul___mol_get_containee_index (Mol_Atom mol)
{
	/*
	**  Return the current containee index for this atom.
	*/
	assert (rul___mol_is_valid (mol));
	assert (is_atomic (mol));
	return ((Atomic_Value *) mol)->containee_index;
}



/*************************************
**                                  **
**  RUL___MOL_GIVE_CONTAINEE_INDEX  **
**                                  **
*************************************/

unsigned long
rul___mol_give_containee_index (Mol_Atom mol)
	/*
	**  Return the containee index for his atom, and 
	**  if it doesn't have one yet, give it one.
	*/
{
        Atomic_Value *atom;

	assert (rul___mol_is_valid (mol));
	assert (is_atomic (mol));

        atom = (Atomic_Value *) mol;
        if (atom->containee_index == MOL__C_NOT_CONTAINED) {

            atom->containee_index = SL__next_containee_index;

            SL__next_containee_index++;
            if (SL__next_containee_index == MOL__C_NOT_CONTAINED) {
                /*  overflowed containee_index  */
                SL__next_containee_index = 1;
            }
        }
        return (atom->containee_index);
}





/*******************
**                **
**  ARRAYS_EQUAL  **
**                **
*******************/

static Boolean
arrays_equal (long len, void *array_1[], void *array_2[])
{
	long	i;
	for (i=0; i<len; i++) {
	    if (array_1[i] != array_2[i]) return (FALSE);
	}
	return (TRUE);
}



void
ref_count_trace_on (void)
{
    ref_count_trace = 1;
}

void
ref_count_trace_off (void)
{
    ref_count_trace = 0;
}

/*************************
**                      **
**  RUL__MOL_INCR_USES  **
**                      **
*************************/

void
rul__mol_incr_uses (Molecule mol)
{
	if (mol != NULL  &&  
#ifdef NDEBUG
    /*a molecule must have at least one reference or it would have been freed*/
		mol->ref_count > 0 )
#else
   /*since molecules may not be actually freed, they may have no references */
		mol->ref_count >= 0 )
#endif
	{
	    mol->ref_count ++;
	    if (ref_count_trace) 
	    {
		rul__ios_printf ( RUL__C_STD_ERR, "\n\n INC TO:");
		rul__mol_print_molecule (mol);
	    }

	    if (mol->ref_count > PERMANENCE_THRESHOLD) {
		mol->ref_count = PERMANENT;
	    }
	}
}




/*************************
**                      **
**  RUL__MOL_DECR_USES  **
**                      **
*************************/

void
rul__mol_decr_uses (Molecule mol)
{
	long index;
	Molecule tmp;

	if (mol == NULL)
	  return;

	assert (rul___mol_is_valid (mol));

        if (mol->ref_count == PERMANENT) {
            /*    Marked as permanent    */
            return;
        }
        
        if (mol->ref_count >= 1) {

            /*    Not permanent, and one or more references    */
            mol->ref_count = mol->ref_count - 1;
	    if (ref_count_trace)
	    {
		rul__ios_printf ( RUL__C_STD_ERR, "\n\n DEC TO:");
		rul__mol_print_molecule (mol);
	    }

	    /*    If after decrementing for this reference, there
	    **    are still remaining references, then we're done
	    */
	    if (mol->ref_count > 0) return;

            /*
            **     If it was not permanent, and there are no remaining
            **     references, then remove this molecule from the
            **     molecule table, and then free it.
            */
            index = HASH_NUM_TO_INDEX(mol->hash_number);
            tmp = SA__molecule_table[index];
            if (tmp == mol) {
#ifdef NDEBUG
	      /*remove the molecule from the table (unlink it from the chain)*/
                SA__molecule_table[index] = mol->next;
#endif
                free_molecule (mol);
                return;
            } else {
                while (tmp != NULL) {
                    if (tmp->next == mol) {
#ifdef NDEBUG
	      /*remove the molecule from the table (unlink it from the chain)*/
                        tmp->next = mol->next;
#endif
                        free_molecule (mol);
                        return;
                    }
                    tmp = tmp->next;
                }
            }
	}
}




/**************************
**                       **
**  RUL__MOL_SYMBOL_NIL  **
**                       **
**      and other        **
**  special constants    **
**                       **
**************************/


Mol_Symbol
rul__mol_symbol_nil (void)
{
	return SA_nil;
}

Mol_Symbol
rul__mol_symbol_root (void)
{
	return SA_root;
}

Mol_Symbol
rul__mol_symbol_crlf (void)
{
	return SA_crlf;
}

Mol_Symbol
rul__mol_symbol_id (void)
{
	return SA_id;
}

Mol_Symbol
rul__mol_symbol_instance_of (void)
{
	return SA_instance_of;
}

Mol_Int_Atom
rul__mol_integer_zero (void)
{
	return SA_integer_zero;
}

Mol_Dbl_Atom
rul__mol_double_zero (void)
{
	return SA_double_zero;
}

Mol_Opaque
rul__mol_opaque_null (void)
{
	return SA_opaque_null;
}

Mol_Instance_Id
rul__mol_instance_id_zero (void)
{
	return SA_instance_zero;
}

Mol_Compound
rul__mol_compound_empty (void)
{
	return SA_compound_empty;
}


static long gensym_count;

/**********************
**                   **
**  RUL__MOL_GENSYM  **
**                   **
**********************/

Mol_Symbol
rul__mol_gensym (Mol_Symbol prefix)
{
  char        buf[RUL_C_MAX_SYMBOL_SIZE+51] = "";
  char        num_buf[50];

  if (prefix) {
    assert (rul___mol_is_valid (prefix));
    rul__mol_use_printform (prefix, buf, RUL_C_MAX_SYMBOL_SIZE+1);
  }

  if (strlen (buf) == 0)
    strcpy (buf, "G:");

  sprintf (num_buf, "%ld", ++gensym_count);
  strcat (buf, num_buf);
  return (rul__mol_make_symbol (buf));
}




/**********************
**                   **
**  RUL__MOL_GENINT  **
**                   **
**********************/

Mol_Int_Atom
rul__mol_genint (void)
{
  return (rul__mol_make_int_atom (++gensym_count));
}





/****************************
**                         **
**  RUL__MOL_SET_GEN_COUNT **
**                         **
****************************/

long
rul__mol_get_gen_count (void)
{
  return gensym_count;
}





/****************************
**                         **
**  RUL__MOL_SET_GEN_COUNT **
**                         **
****************************/

void
rul__mol_set_gen_count (long new_count)
{
  gensym_count = new_count;
}




//#ifndef NDEBUG


/***********************************
**                                **
**  RUL__MOL_PRINT_ALL_MOLECULES  **
**                                **
***********************************/

void
rul__mol_print_all_molecules (void)
{
	long	  i;
	Molecule  mol;

	rul__ios_printf ( RUL__C_STD_ERR, "\n\n  Molecule Table Contents:\n");
	for (i=0; i<MOLECULE_TABLE_SIZE; i++) {
	    mol = SA__molecule_table[i];
	    while (mol != NULL) {
		/*
		**    Traverse the overflow chain at each bucket
		*/
		if (mol->ref_count != 0) {

		    if (is_atomic(mol)) {
		        rul__ios_printf ( RUL__C_STD_ERR, "\n    Atom:     bucket=%5d, refs=%3d, type=",
			    i, mol->ref_count);
		    } else {
		        rul__ios_printf ( RUL__C_STD_ERR, "\n    PolyAtom: bucket=%5d, refs=%3d, type=",
			    i, mol->ref_count);
		    }
	            switch (mol->type) {
                        case satom :    rul__ios_printf ( RUL__C_STD_ERR, "Symbol,   value= ");
                                        break;
                        case iatom :    rul__ios_printf ( RUL__C_STD_ERR, "Integer,  value= ");
                                        break;
                        case datom :    rul__ios_printf ( RUL__C_STD_ERR, "Float,    value= ");
                                        break;
                        case oatom :    rul__ios_printf ( RUL__C_STD_ERR, "Opaque,   value= ");
                                        break;
                        case watom :    rul__ios_printf ( RUL__C_STD_ERR, "Wmo Id,   value= ");
                                        break;
                        case cpoly :    rul__ios_printf ( RUL__C_STD_ERR, "Compound, value= ");
                                        break;
                        default :       rul__ios_printf ( RUL__C_STD_ERR, "  U N K N O W N !");
                                        break;
                    }
                    rul__mol_print_readform (mol, RUL__C_STD_ERR);
		}
		mol = mol->next;
	    }
	}
}

/***********************************
**                                **
**  RUL__MOL_PRINT_MEM_CONS       **
**                                **
***********************************/

void
rul__mol_print_mem_cons (void)
{
	long	  i, memory_consumed=0;
	Molecule  mol;

	for (i=0; i<MOLECULE_TABLE_SIZE; i++) {
	    mol = SA__molecule_table[i];
	    while (mol != NULL) {
		/*
		**    Traverse the overflow chain at each bucket
		*/
		if (mol->ref_count != 0) {
		    memory_consumed += sizeof(*mol);
		}
		mol = mol->next;
	    }
	}

	rul__ios_printf ( RUL__C_STD_ERR, 
			  "\n\n  Molecule Table Memory Consumption: %.2f Kb\n",
			  (float)memory_consumed/1000);
}

/***********************************
**                                **
**  RUL__MOL_PRINT_MOLECULE	  **
**                                **
***********************************/

void
rul__mol_print_molecule (Molecule mol)
{
	long	  i;
	Molecule  this_mol;

	for (i=0; i<MOLECULE_TABLE_SIZE; i++) {
	    this_mol = SA__molecule_table[i];
	    if (this_mol == mol) {
		/*
		**    Print Attribute Values.
		*/
/*		if (mol->ref_count != 0) */{

		    if (is_atomic(mol)) {
		        rul__ios_printf ( RUL__C_STD_ERR, "    Atom:     bucket=%5d, refs=%3d, type=",
			    i, mol->ref_count);
		    } else {
		        rul__ios_printf ( RUL__C_STD_ERR, "    PolyAtom: bucket=%5d, refs=%3d, type=",
			    i, mol->ref_count);
		    }
	            switch (mol->type) {
                        case satom :    rul__ios_printf ( RUL__C_STD_ERR, "Symbol,   value= ");
                                        break;
                        case iatom :    rul__ios_printf ( RUL__C_STD_ERR, "Integer,  value= ");
                                        break;
                        case datom :    rul__ios_printf ( RUL__C_STD_ERR, "Float,    value= ");
                                        break;
                        case oatom :    rul__ios_printf ( RUL__C_STD_ERR, "Opaque,   value= ");
                                        break;
                        case watom :    rul__ios_printf ( RUL__C_STD_ERR, "Wmo Id,   value= ");
                                        break;
                        case cpoly :    rul__ios_printf ( RUL__C_STD_ERR, "Compound, value= ");
                                        break;
                        default :       rul__ios_printf ( RUL__C_STD_ERR, "  U N K N O W N !");
                                        break;
                    }
                    rul__mol_print_readform (mol, RUL__C_STD_ERR);
		    rul__ios_printf ( RUL__C_STD_ERR, "\n");
		}
	    }
	}
}

/***********************************
**                                **
**  RUL__MOL_CHECK_REF_COUNT	  **
**                                **
***********************************/

int
rul__mol_check_ref_count (Molecule mol)
{
	long	  i;
	Molecule  this_mol;

	for (i=0; i<MOLECULE_TABLE_SIZE; i++) {
	    this_mol = SA__molecule_table[i];
	    if (this_mol == mol) {
		return(mol->ref_count);
	    }
	}

	rul__ios_printf ( RUL__C_STD_ERR, "\n\n  rul__mol_check_ref_count : Molecule does not exist:\n");
	return(0);
}

//#endif

//#ifndef NDEBUG


/*********************************
**                              **
**  RUL__MOL_PRINT_TABLE_STATS  **
**                              **
*********************************/

#define MAX_FREQ 100

void
rul__mol_print_table_stats (void)
{
	long	chain_count, i, top;
	double	avg_chain;
	double	bucket_count = 0.0;
	long	frequency[MAX_FREQ];
	long	mol_count   = 0;
	long	satom_count = 0;
	long	iatom_count = 0;
	long	datom_count = 0;
	long	oatom_count = 0;
	long	watom_count = 0;
	long	cpoly_count = 0;
	long	unknown = 0;
	Molecule mol;

	for (i=0; i<MAX_FREQ; i++) {
	    frequency[i] = 0;
	}

	for (i=0; i<MOLECULE_TABLE_SIZE; i++) {
	    chain_count = 0;
	    mol = SA__molecule_table[i];
	    while (mol != NULL) {
		/*
		**    Traverse the overflow chain at each bucket
		*/
		chain_count ++;
	        switch (mol->type) {
		    case satom :    satom_count ++;  break;
		    case iatom :    iatom_count ++;  break;
		    case datom :    datom_count ++;  break;
		    case oatom :    oatom_count ++;  break;
		    case watom :    watom_count ++;  break;
		    case cpoly :    cpoly_count ++;  break;
		    default :	    unknown++;       break;
		}
		mol = mol->next;
	    }
	    mol_count += chain_count;
	    if (chain_count >= MAX_FREQ) {
		rul__ios_printf ( RUL__C_STD_ERR,
				  "\nOOPS:  freq overflow  = %ld for %ld",
				  chain_count, i);
	    } else {
	        frequency[chain_count] += 1;
	    }
	}

	rul__ios_printf ( RUL__C_STD_ERR,
			  "\n\n  Molecule Table Statistics:\n");

	top = MAX_FREQ - 1;
	while (top >= 0 && frequency[top] == 0) top--;
	for (i=0; i<=top; i++) {
	    rul__ios_printf ( RUL__C_STD_ERR,
			      "\n    buckets with chain length %3d :    %5d",
			      i, frequency[i]);
	    if (i>0) bucket_count += frequency[i];
	}

	avg_chain = mol_count / bucket_count;
	rul__ios_printf ( RUL__C_STD_ERR, "\n\n  Total Molecule Count  = %5d",
			  mol_count);
	rul__ios_printf ( RUL__C_STD_ERR, "\n    Symbol Count        = %5d",
			  satom_count);
	rul__ios_printf ( RUL__C_STD_ERR, "\n    Int Atom Count      = %5d",
			  iatom_count);
	rul__ios_printf ( RUL__C_STD_ERR, "\n    Dbl Atom Count      = %5d",
			  datom_count);
	rul__ios_printf ( RUL__C_STD_ERR, "\n    Opaque Count        = %5d",
			  oatom_count);
	rul__ios_printf ( RUL__C_STD_ERR, "\n    Instance Id Count   = %5d",
			  watom_count);
	rul__ios_printf ( RUL__C_STD_ERR, "\n    Compound Count      = %5d",
			  cpoly_count);
	if (unknown > 0) {
	    rul__ios_printf ( RUL__C_STD_ERR,
			      "\n    UNKNOWN Count       = %5d", unknown);
	}
	rul__ios_printf ( RUL__C_STD_ERR,
			  "\n  Average Chain Length  = %5.3f\n\n", avg_chain);
}

//#endif



/*******************************
**                            **
**  RUL__MOL_PRINT_REMAINING  **
**                            **
*******************************/

void
rul__mol_print_remaining (void)
{
	long i;
	long found_one = 0;
	Molecule  mol;

	for (i=0; i<MOLECULE_TABLE_SIZE; i++) {
	    mol = SA__molecule_table[i];
	    while (mol != NULL) {
		/*
		**    Traverse the overflow chain at each bucket
		*/
		if (mol->ref_count != PERMANENT
//#ifndef NDEBUG
/* normally, any molecules with no references remaining would have been freed
   and removed from the table. Since the molecules may not actually be freed,
   it may remain in the table with no references. Therefore, only print those
   molecules which wouldn't have been freed (they have at least one reference).
*/
			&&   mol->ref_count != 0
//#endif
		) {
		    if (! found_one) {
			rul__ios_printf ( RUL__C_STD_ERR,
					  "\n\n  Remaining Molecule Table Contents:\n");
			found_one = 1;
		    }
		    rul__ios_printf ( RUL__C_STD_ERR, "\n\t");

	            switch (mol->type) {
                        case satom :    rul__ios_printf ( RUL__C_STD_ERR,
							  "Symbol, ");
                                        break;
                        case iatom :    rul__ios_printf ( RUL__C_STD_ERR,
							  "Integer, ");
                                        break;
                        case datom :    rul__ios_printf ( RUL__C_STD_ERR,
							  "Float, ");
                                        break;
                        case oatom :    rul__ios_printf ( RUL__C_STD_ERR,
							  "Opaque, ");
                                        break;
                        case watom :    rul__ios_printf ( RUL__C_STD_ERR,
							  "Instance-Id, ");
                                        break;
                        case cpoly :    rul__ios_printf ( RUL__C_STD_ERR,
							  "Compound, ");
                                        break;
                        default :       rul__ios_printf ( RUL__C_STD_ERR,
							  "  U N K N O W N !");
                                        break;
                    }
                    rul__mol_print_readform (mol, RUL__C_STD_ERR);
		    rul__ios_printf ( RUL__C_STD_ERR,
				      ", had %ld reference%s remaining.", 
				      mol->ref_count,
				      mol->ref_count > 1 ? "s" : "");
		}
		mol = mol->next;
	    }
	}
}
