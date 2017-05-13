/****************************************************************************
**                                                                         **
**                  O P S _ M O L _ H A S H _ N U M . C                    **
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
**	This module contains the functions which return an
**	unsigned long value given some specific type of input
**	data.
**
**	Note these routines are carefully written to NOT be dependent
**	on specific machines or machine specific data type sizes, except:
**        - the constants CHAR_BITS and LONG_MAX found in the standard C
**		language header file, limits.h
**	  - the standard C macro, sizeof
**
**  MODIFIED BY:
**	DEC	Digital Equipment Corporation
**	CPQ	Compaq Computer Corporation
**
**  MODIFICATION HISTORY:
**
**	5-Mar-1992	DEC	Initial version
**	01-Dec-1999	CPQ	Release with GPL
*/



#include <common.h>
#include <limits.h>
#include <stdarg.h>

#include <mol_p.h>
#include <mol.h>


	/*
	**	Hash-numbers are generated for scalar values
	**	using a routing table algorithm, and for non-scalar
	**	values by simple combinations of the constituent
	**	scalar value's hash-numbers, as defined below.
	*/

#define MOLS_ORDERED_HASH_INCR(hsh,mol)		((hsh + 1) ^ (mol)->hash_number)
		/* add 1, then bitwise XOR (left associative) */

#define MOLS_UNORDERED_HASH_INCR(hsh,mol)	(hsh ^ (mol)->hash_number)
		/* bitwise XOR (bi-directional associative) */


	/*
	**	When doing the routing, initial insertion into
	**	the route table is determined based on the data
	**	type being hashed, by means of the following
	**	constants:
	*/

#define STR_OFFSET   0		/*  The offsets for Atom types should be    */
#define INT_OFFSET  31		/*  less than the route table size of 256.  */
#define DBL_OFFSET  63
#define OPQ_OFFSET  95
#define WMO_OFFSET  127

#define CMP_OFFSET  (LONG_MAX / 7)
#define TAB_OFFSET  (LONG_MAX / 5)






	/*
	**	The route table below contains a pseudo random
	**	permutation of the integers from 0 to 256,
	**	and is used by the various hash-number functions
	**	to ensure good table distributions and 
	**	low hash costs (even for machines without
	**	hardware division operations).
	**
	**	The basic hash algorithm is a variant of that
	**	from "Fast Hashing of Variable Length Text
	**	Strings", Peter Pearson, CACM, v33, n6, p677
	**
	*/

static	const  unsigned char SB__route_table[256] = {
		134,   69,  160,   77,   51,  124,  187,  253,
		 14,   23,  105,  142,   91,   43,  231,  238,
		 47,  154,  190,   84,  219,  242,   36,  123,
		 17,  239,    2,   67,  229,  162,  143,   73,
		200,  120,  117,  104,  108,   60,  177,  224,
		133,  110,  225,  100,   92,  216,  171,  149,
		 10,   98,   55,  227,  165,   76,  156,  128,
		204,  251,   48,  118,  255,  163,  193,   89,
		203,  155,  130,  210,  244,  146,   86,   95,
		127,  208,   21,  213,   97,  221,  240,   63,
		 94,  226,  248,   88,    7,  207,  246,  218,
		148,  114,  222,   59,  175,  150,  173,  122,
		212,   35,    8,   30,  166,   33,  206,   16,
		152,   41,  237,   52,   66,  186,  198,  235,
		 65,  232,  112,  140,   71,  116,  132,   99,
		  0,  215,   28,  158,  249,  135,   87,  171,
		192,  164,   85,   27,   90,  179,  233,   49,
		136,   57,  172,   53,  168,  250,   12,   20,
		234,   13,  121,   38,   70,   58,  129,  185,
		151,   82,  113,  230,  125,  201,   50,  223,
		196,    6,   78,  106,  184,  202,  145,   62,
		  5,  174,   79,   83,   26,   44,   25,  245,
		 81,   71,  147,  211,  195,   31,  153,   19,
		 72,  119,  188,  126,  205,   46,  194,   74,
		241,  252,   29,   64,  138,   40,  167,   18,
		247,  199,   22,  181,  115,  243,   37,  103,
		183,  180,  137,    4,  169,  209,   80,   96,
		228,   54,    3,   56,  144,  178,   34,  191,
		 93,  159,   24,  131,  107,  139,  197,    1,
		109,  217,   15,   32,  170,  236,  101,  141,
		 68,   39,  157,  176,   75,    9,  254,   45,
		 42,  182,  189,  102,  220,  214,   11,  111
	};






/**********************************
**                               **
**  RUL__MOL_STRING_TO_HASH_NUM  **
**                               **
**********************************/

unsigned long	rul__mol_string_to_hash_num (char *str)
{
	/*
	**	Given a character string, return
	**	the corresponding hash number
	*/
	long		i, j;
	unsigned char	index_frags[sizeof(long)];
	unsigned long	index;

	for (j=0; j<sizeof(long); j++) index_frags[j] = j + STR_OFFSET;

	i = 0;
	while (str[i] != '\0') {
	    for (j=0; j<sizeof(long); j++) {
		index_frags[j] =
		  SB__route_table[((unsigned char *)str)[i] ^ index_frags[j]];
	    }
	    i++;
	}
	index = 0;
	for (j=0; j<sizeof(long); j++) {
	    index = (index << CHAR_BIT) | index_frags[j];
	}

	return (index);
}



/***********************************
**                                **
**  RUL__MOL_INTEGER_TO_HASH_NUM  **
**                                **
***********************************/

unsigned long	rul__mol_integer_to_hash_num (long a_long)
{
	/*
	**	Given an integer, return
	**	the corresponding hash number
	*/
	long		i, j;
	unsigned char	index_frags[sizeof(long)];
	unsigned char	*a_long_as_bytes = (unsigned char *) &a_long; 
	unsigned long	index;

	for (j=0; j<sizeof(long); j++) index_frags[j] = j + INT_OFFSET;

	for (i=0; i<sizeof(long); i++) {
	    for (j=0; j<sizeof(long); j++) {
		index_frags[j] = 
		    SB__route_table [ a_long_as_bytes[i] ^ index_frags[j] ];
	    }
	}
	index = 0;
	for (j=0; j<sizeof(long); j++) {
	    index = (index << CHAR_BIT) | index_frags[j];
	}
	return (index);
}



/**********************************
**                               **
**  RUL__MOL_DOUBLE_TO_HASH_NUM  **
**                               **
**********************************/

unsigned long	rul__mol_double_to_hash_num (double a_double)
{
	/*
	**	Given a double float, return
	**	the corresponding hash index
	*/
	long		i, j;
	unsigned char	index_frags[sizeof(long)];
	unsigned char	*a_double_as_bytes = (unsigned char *) &a_double; 
	unsigned long	index;

	for (j=0; j<sizeof(long); j++)
		index_frags[sizeof(long) - (j + 1)] = j + DBL_OFFSET;

	for (i=0; i<sizeof(double); i++) {
	    for (j=0; j<sizeof(long); j++) {
		index_frags[j] = 
		    SB__route_table [ a_double_as_bytes[i] ^ index_frags[j] ];
	    }
	}
	index = 0;
	for (j=0; j<sizeof(long); j++) {
	    index = (index << CHAR_BIT) | index_frags[j];
	}
	return (index);
}



/***********************************
**                                **
**  RUL__MOL_ADDRESS_TO_HASH_NUM  **
**                                **
***********************************/

unsigned long	rul__mol_address_to_hash_num (void *an_address)
{
	/*
	**	Given an arbitrary_address,
	**	return the corresponding hash number
	*/
	long		i, j;
	unsigned char	index_frags[sizeof(long)];
	unsigned char	*an_address_as_bytes = (unsigned char *) &an_address; 
	unsigned long	index;

	for (j=0; j<sizeof(long); j++) index_frags[j] = j + OPQ_OFFSET;

	for (i=0; i<sizeof(long); i++) {
	    for (j=0; j<sizeof(long); j++) {
		index_frags[j] = 
		    SB__route_table [ an_address_as_bytes[i] ^ index_frags[j] ];
	    }
	}
	index = 0;
	for (j=0; j<sizeof(long); j++) {
	    index = (index << CHAR_BIT) | index_frags[j];
	}
	return (index);
}



/***********************************
**                                **
**  RUL___MOL_WMO_ID_TO_HASH_NUM  **
**                                **
***********************************/

unsigned long	rul___mol_wmo_id_to_hash_num (long a_wmo_id_num)
{
	/*
	**	Given an integer wmo id number, then
	**	return the corresponding hash number
	*/
	long		i, j;
	unsigned char	index_frags[sizeof(long)];
	unsigned char	*a_wmo_id_num_as_bytes =
				(unsigned char *) &a_wmo_id_num; 
	unsigned long		index;

	for (j=0; j<sizeof(long); j++) index_frags[j] = j + WMO_OFFSET;

	for (i=0; i<sizeof(long); i++) {
	    for (j=0; j<sizeof(long); j++) {
		index_frags[j] = 
		    SB__route_table[a_wmo_id_num_as_bytes[i] ^ index_frags[j]];
	    }
	}
	index = 0;
	for (j=0; j<sizeof(long); j++) {
	    index = (index << CHAR_BIT) | index_frags[j];
	}
	return (index);
}




/***************************
**                        **
**  RUL__MOL_TO_HASH_NUM  **
**                        **
***************************/

unsigned long	rul__mol_to_hash_num (Molecule mol)
{
	return (MOL_TO_HASH_NUM(mol));
}




/*************************************
**                                  **
**  RUL___MOL_MOLS_TO_HASH_NUM_VEC  **
**                                  **
*************************************/

unsigned long	rul___mol_mols_to_hash_num_vec (long mol_count, 
					   Molecule mol_vector[])
{
	unsigned long num;
	long i;

	num = CMP_OFFSET;
	for (i=0; i<mol_count; i++) {
	    num = MOLS_ORDERED_HASH_INCR(num,mol_vector[i]);
	}
	return (num);
}




/********************************
**                             **
**  RUL__MOL_MOLS_TO_HASH_NUM  **
**                             **
********************************/

unsigned long	rul__mol_mols_to_hash_num (long mol_count, ...)
{
	va_list ap;
	long num, i;
	Molecule m;

	va_start (ap, mol_count);
	num = CMP_OFFSET;
	for (i=0; i<mol_count; i++) {
	    m = va_arg (ap, Molecule);
	    num = MOLS_ORDERED_HASH_INCR(num,m);
	}
	va_end (ap);
	return (num);
}



/**********************************
**                               **
**  RUL__MOL_INIT_HASH_NUM_INCR  **
**                               **
**********************************/

unsigned long	 rul__mol_init_hash_num_incr (void)
{
	return (CMP_OFFSET);
}



/*************************************
**                                  **
**  RUL__MOL_MOLS_TO_HASH_NUM_INCR  **
**                                  **
*************************************/

unsigned long	 rul__mol_mols_to_hash_num_incr (unsigned long hsh,
						 Molecule mol)
{
	    return (MOLS_ORDERED_HASH_INCR(hsh, mol));
}
