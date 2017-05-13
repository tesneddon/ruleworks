/****************************************************************************
**                                                                         **
**                            M O L _ P . H                                **
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
**	This header file contains all the private declarations for
**	all run-time values supported by the language.  All these
**	values are opaque to the rest of the run-time system, and
**	can only be created, accessed, or modified via the functional
**	interface to this subsystem (in mol.h), with one exception:
**	equivalence tests.
**
**
**  MODIFIED BY:
**	DEC	Digital Equipment Corporation
**	CPQ	Compaq Computer Corporation
**
**  MODIFICATION HISTORY:
**
**	6-Jun-1992	DEC	Initial version
**	01-Dec-1999	CPQ	Release with GPL
*/



/*	Ensure CHAR_BIT is defined, and then define MOL__BITS_PER_LONG  */

#include <limits.h>	/*  CHAR_BIT should be defined here  */
#if !defined(CHAR_BITS)  &&  (defined(__ultrix__) || defined(__osf__))
#include <sys/types.h>	/*  for the definition of constant macro NBBY */
#ifdef NBBY
#define CHAR_BITS NBBY	/*  another possibility...  */
#endif
#endif

#ifndef CHAR_BITS	/*  As a last resort...  */
#define CHAR_BITS 8
#endif

#define BITS_PER_ULONG	(sizeof(unsigned long) * CHAR_BIT)



/*
**
**	The values supported by the language have been organized
**	into an object class hierarchy as shown below.  The typedef
**	declarations in this file fake this hierarchy given only the
**	limited facilities available in C.
**
**
**				        Molecule
**				       /        \
**				      /          \
**				  Atom            Polyatom
**				/ | | \             \     \
**			       /  | |  \             \     \
**			      /  /  |   \             \     \
**			     /  /   |    Number     Table  Compound
**			    /  |    |      |   \
**			   /   |    |      |    \
**			  /    |    |      |     \
**		     Symbol Wmo_Id Opaque Integer Double
**
**
**	The specific constants used within the ENUM statement
**	below are an encoded form of this inheritance hierarchy.
**	Other than in type equivalence tests, these constants
**	should only be used via the test macros immediately
**	following the enum statement which enable testing for
**	each of the non-leaf classes.
**
**	Note that all the type values are in lower case.
**	All the typedef names for these classes are in mixed case
**	with each word capitalized, and the typedefs for all leaf
**	classes have "_Value" added at the end of the class name.
*/

/*  Abstract molecule types  */
#define POLY	00010
#define ATOM	00030
#define NUMBER	(ATOM | 00100)

#define MASK_2  0077
#define MASK_3	0777

/*  Concrete molecule types  */
typedef enum private_mol_type {
	  cpoly = 	(POLY   | 00100),
	  tpoly = 	(POLY   | 00200),
	  iatom = 	(NUMBER | 01000),
	  datom = 	(NUMBER | 02000),
	  oatom = 	(ATOM   | 00200),
	  satom =	(ATOM   | 00300),
	  watom = 	(ATOM   | 00400)
} Private_Mol_Type;

#define is_polyatomic(x)	((((int) x->type) & MASK_2) == (int) POLY)
#define is_not_polyatomic(x)	((((int) x->type) & MASK_2) != (int) POLY)

#define is_atomic(x)		((((int) x->type) & MASK_2) == (int) ATOM)
#define is_not_atomic(x)	((((int) x->type) & MASK_2) != (int) ATOM)

#define is_numeric(x)		((((int) x->type) & MASK_3) == (int) NUMBER)
#define is_not_numeric(x)	((((int) x->type) & MASK_3) != (int) NUMBER)



/*
**	MOLECULE_PIECE defines the set of fields which appear at the
**	top of all the structures for all the subbclass of molecule.
**	This macro MUST appear as the first item in all typedefs and
**	structure declarations for those subclasses.
*/

#define MOLECULE_PIECE						    \
	unsigned long	 hash_number;				    \
	Private_Mol_Type type;				 	    \
	long		 ref_count;			 	    \
			  /*				 	    \
			  **  A negative ref_count implies 	    \
			  **  that this value has been set 	    \
			  **  permanent, or was so frequent	    \
			  **  that it became permanent		    \
			  */					    \
	struct molecule	 *next	/*  For hash table chains  */

/*
**	Some functions operate on all molecules, regardless of their
**	type, and they refer to their argument via this declaration
**	from the generic class, "Molecule" found in common.h.
*/

struct molecule {
	MOLECULE_PIECE;
};

#define MOL_TO_HASH_NUM(mol)		(mol->hash_number)



/*
**	Similar to the MOLECULE_PIECE, the ATOM_PIECE defintion includes
**	all the fields which are shared by all atomic values.  It MUST
**	always appear immediately after the MOLECULE_PIECE in the typedefs
**	or structure declarations for all subclasses of "Atom".
*/

#define ATOM_PIECE						    \
	unsigned short	printform_length;			    \
	unsigned short	readform_length;			    \
	unsigned long	containee_index


typedef struct atom {
	MOLECULE_PIECE;
	ATOM_PIECE;
} Atomic_Value;

typedef Atomic_Value *Atom;




/*
**	The subclasses of "Atom":
**
**		Number
**		    Double_Value
**		    Integer_Value
**		Opaque_Value
**		Symbolic_Value
**		Instance_Id_Value
*/

typedef struct {
	MOLECULE_PIECE;
	ATOM_PIECE;
	union {	
		long	ival;
		double  dval;
	} value;
} Numeric_Value;


typedef struct {
	MOLECULE_PIECE;
	ATOM_PIECE;
	long	ival;
} Integer_Value;


typedef struct {
	MOLECULE_PIECE;
	ATOM_PIECE;
	double	dval;
} Double_Value;


typedef struct {
	MOLECULE_PIECE;
	ATOM_PIECE;
	void		*oval;
} Opaque_Value;


typedef struct a_symbol {
	MOLECULE_PIECE;
	ATOM_PIECE;
	char		*sval;
	Molecule	soundex_symbol;
} Symbol_Value;

typedef Symbol_Value *Symbol;


typedef struct {
	MOLECULE_PIECE;
	ATOM_PIECE;
	Object		instance_ptr;
	long		wval;  /*  Unique number used for print name  */
} Instance_Id_Value;



/*
**	POLYATOM_PIECE defines the set of fields which appear in
**	all the multi-valued subclasses of Molecule.  It MUST
**	always appear immediately after the MOLECULE_PIECE in the
**	typedefs or structure declarations for all subclasses of
**	the class "Polyatom".
*/

#define POLYATOM_PIECE						    \
	Mol_Int_Atom	atomic_element_count;			    \
	long		element_count;				    \
	long		containment_vector_length;		    \
	unsigned long	containment_vector_bits;		    \
	unsigned long  *containment_bit_vector


/*
**	The generic class, "Polyatom".
*/

typedef struct a_polyatom {
	MOLECULE_PIECE;
	POLYATOM_PIECE;
} Polyatomic_Value;

typedef Polyatomic_Value *Polyatom;


/*
**	The subclasses of "Polyatomic_Value":
**
**		Compound_Value
**		Table_Value
*/

typedef struct a_compound {
	MOLECULE_PIECE;
	POLYATOM_PIECE;
	Molecule	*values;
} Compound_Value;

typedef Compound_Value *Compound;



typedef struct a_table {
	MOLECULE_PIECE;
	POLYATOM_PIECE;
	Molecule	*values;
	Molecule	*keys;
} Table_Value;

typedef Table_Value *Table;



#define  MOL__C_NOT_A_VALID_LENGTH	0
#define  MOL__C_NOT_CONTAINED		0


/*
**	The following functions are private to this subsystem,
**	but shared amoung the files in this susbsystem.
*/

Mol_Polyatom	rul___mol_make_cpoly (long atom_count, Molecule atom_array[]);

Boolean		rul___mol_is_valid (Molecule mol);
Boolean		rul___mol_is_valid_atom (Molecule mol);
Boolean		rul___mol_is_valid_number (Molecule mol);
Boolean		rul___mol_is_valid_atom_of_type (Molecule mol, 
						 Private_Mol_Type t);
Boolean		rul___mol_is_valid_poly (Molecule mol);
Boolean		rul___mol_is_valid_cpoly (Molecule mol);

unsigned long	rul___mol_wmo_id_to_hash_num (long a_wmo_id_num);
unsigned long	rul___mol_mols_to_hash_num_vec (long mol_count,
						Molecule mol_vec[]);

Boolean		rul___mol_string_has_specials (char *str);
Boolean  	rul___mol_quote_string (char *outbuff, long maxlen, char *str);
Mol_Symbol 	rul___mol_soundex_symbol (Mol_Symbol mol);

unsigned long   rul___mol_get_containee_index (Mol_Atom mol);
unsigned long   rul___mol_give_containee_index (Mol_Atom mol);
void 		rul___mol_init_contains_masks (void);
Boolean 	rul___mol_identity_containment (Compound_Value *cpoly_ptr, 
			Mol_Atom atom, Boolean return_if_match_succeeds);

void		rul___mol_quiet_print_printform (Molecule mol, IO_Stream ios);
long		rul___mol_printform_length (Molecule mol);
long		rul___mol_readform_length (Molecule mol);
