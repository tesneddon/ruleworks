/****************************************************************************
**                                                                         **
**                    R T S _ M O L _ A R I T H . C                        **
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
**	RULEWORKS run time system
**
**  ABSTRACT:
**	This module contains the functions for performing arithmetic
**	operations over numeric of molecular values.
**
**  MODIFIED BY:
**	DEC	Digital Equipment Corporation
**	CPQ	Compaq Computer Corporation
**
**  MODIFICATION HISTORY:
**
**	12-Oct-1992	DEC	Initial version
**	01-Dec-1999	CPQ	Release with GPL
*/


#include <common.h>
#include <mol_p.h>
#include <mol.h>
#include <ios.h>
#include <msg.h>
#include <rts_msg.h>


#define M_INT(mol)  ((Integer_Value *) mol)
#define M_DBL(mol)  ((Double_Value *) mol)

static void print_nonnumval_msg (Molecule mol, char *oper);
static Molecule int_atom_zero (void);



/*************************
**                      **
**  RUL__MOL_ARITH_ADD  **
**                      **
*************************/

Mol_Number rul__mol_arith_add (Mol_Number mol1, Mol_Number mol2)
{
	Molecule sum ;

	assert (rul___mol_is_valid (mol1));
	assert (rul___mol_is_valid (mol2));

	if (!rul___mol_is_valid_number (mol1)) {
	  print_nonnumval_msg (mol1, "add");
	  return (int_atom_zero ());
	}
	if (!rul___mol_is_valid_number (mol2)) {
	  print_nonnumval_msg (mol2, "add");
	  return (int_atom_zero ());
	}

	if (mol1->type == iatom) {
	    if (mol2->type == iatom) {
		sum = rul__mol_make_int_atom (
			M_INT(mol1)->ival  +  M_INT(mol2)->ival);
	    } else {
	        sum = rul__mol_make_dbl_atom (
			M_INT(mol1)->ival  +  M_DBL(mol2)->dval);
	    }
	} else {
	    if (mol2->type == iatom) {
	        sum = rul__mol_make_dbl_atom (
			M_DBL(mol1)->dval  +  M_INT(mol2)->ival);
	    } else {
	        sum = rul__mol_make_dbl_atom (
			M_DBL(mol1)->dval  +  M_DBL(mol2)->dval);
	    }
	}
	return (sum) ;
}


/*************************
**                      **
**  RUL__MOL_ARITH_NEG  **
**                      **
*************************/

Mol_Number rul__mol_arith_neg (Mol_Number mol1)
{
	assert (rul___mol_is_valid (mol1));

	if (!rul___mol_is_valid_number (mol1)) {
	  print_nonnumval_msg (mol1, "neg");
	  return (int_atom_zero ());
	}

	if (mol1->type == iatom)
	    return (rul__mol_make_int_atom (- M_INT(mol1)->ival));
	else
	    return (rul__mol_make_dbl_atom (- M_DBL(mol1)->dval));

}



/******************************
**                           **
**  RUL__MOL_ARITH_SUBTRACT  **
**                           **
******************************/

Mol_Number rul__mol_arith_subtract (Mol_Number mol1, Mol_Number mol2)
{
	Molecule diff ;

	assert (rul___mol_is_valid (mol1));
	assert (rul___mol_is_valid (mol2));

	if (!rul___mol_is_valid_number (mol1)) {
	  print_nonnumval_msg (mol1, "subtract");
	  return (int_atom_zero ());
	}
	if (!rul___mol_is_valid_number (mol2)) {
	  print_nonnumval_msg (mol2, "subtract");
	  return (int_atom_zero ());
	}

	if (mol1->type == iatom) {
	    if (mol2->type == iatom) {
		diff = rul__mol_make_int_atom (
			M_INT(mol1)->ival  -  M_INT(mol2)->ival);
	    } else {
	        diff = rul__mol_make_dbl_atom (
			M_INT(mol1)->ival  -  M_DBL(mol2)->dval);
	    }
	} else {
	    if (mol2->type == iatom) {
	        diff = rul__mol_make_dbl_atom (
			M_DBL(mol1)->dval  -  M_INT(mol2)->ival);
	    } else {
	        diff = rul__mol_make_dbl_atom (
			M_DBL(mol1)->dval  -  M_DBL(mol2)->dval);
	    }
	}
	return (diff) ;
}



/******************************
**                           **
**  RUL__MOL_ARITH_MULTIPLY  **
**                           **
******************************/

Mol_Number rul__mol_arith_multiply (Mol_Number mol1, Mol_Number mol2)
{
	Molecule prod ;

	assert (rul___mol_is_valid (mol1));
	assert (rul___mol_is_valid (mol2));

	if (!rul___mol_is_valid_number (mol1)) {
	  print_nonnumval_msg (mol1, "multiply");
	  return (int_atom_zero ());
	}
	if (!rul___mol_is_valid_number (mol2)) {
	  print_nonnumval_msg (mol2, "multiply");
	  return (int_atom_zero ());
	}

	if (mol1->type == iatom) {
	    if (mol2->type == iatom) {
		prod = rul__mol_make_int_atom (
			M_INT(mol1)->ival  *  M_INT(mol2)->ival);
	    } else {
	        prod = rul__mol_make_dbl_atom (
			M_INT(mol1)->ival  *  M_DBL(mol2)->dval);
	    }
	} else {
	    if (mol2->type == iatom) {
	        prod = rul__mol_make_dbl_atom (
			M_DBL(mol1)->dval  *  M_INT(mol2)->ival);
	    } else {
	        prod = rul__mol_make_dbl_atom (
			M_DBL(mol1)->dval  *  M_DBL(mol2)->dval);
	    }
	}
	return (prod) ;
}



/****************************
**                         **
**  RUL__MOL_ARITH_DIVIDE  **
**                         **
****************************/

Mol_Number rul__mol_arith_divide (Mol_Number mol1, Mol_Number mol2)
{
	Molecule quotient = NULL;

	assert (rul___mol_is_valid (mol1));
	assert (rul___mol_is_valid (mol2));

	if (!rul___mol_is_valid_number (mol1)) {
	  print_nonnumval_msg (mol1, "divide");
	  return (int_atom_zero ());
	}
	if (!rul___mol_is_valid_number (mol2)) {
	  print_nonnumval_msg (mol2, "divide");
	  return (int_atom_zero ());
	}

	if (mol2->type == iatom) {
	    if (M_INT(mol2)->ival == 0) {
	        rul__msg_print_w_atoms (RTS_DIVZERO, 1, mol1);
		if (mol1->type == iatom)
		    return (rul__mol_make_int_atom(0));
		else
		    return (rul__mol_make_dbl_atom(0.0));
	    }
	} else {
	    if (M_DBL(mol2)->dval == 0.0) {
		rul__msg_print_w_atoms (RTS_DIVZERO, 1, mol1);
		return (rul__mol_make_dbl_atom(0.0));
	    }
	}

	if (mol1->type == iatom) {
	    if (mol2->type == iatom) {
		quotient = rul__mol_make_int_atom (
			M_INT(mol1)->ival  /  M_INT(mol2)->ival);
	    } else {
	        quotient = rul__mol_make_dbl_atom (
			M_INT(mol1)->ival  /  M_DBL(mol2)->dval);
	    }
	} else {
	    if (mol2->type == iatom) {
	        quotient = rul__mol_make_dbl_atom (
			M_DBL(mol1)->dval  /  M_INT(mol2)->ival);
	    } else {
	        quotient = rul__mol_make_dbl_atom (
			M_DBL(mol1)->dval  /  M_DBL(mol2)->dval);
	    }
	}
	return (quotient) ;
}



/****************************
**                         **
**  RUL__MOL_ARITH_MODULO  **
**                         **
****************************/

Mol_Number rul__mol_arith_modulo (Mol_Number mol1, Mol_Number mol2)
{
  long i1, i2;

	assert (rul___mol_is_valid (mol1));
	assert (rul___mol_is_valid (mol2));

	if (!rul___mol_is_valid_number (mol1)) {
	  print_nonnumval_msg (mol1, "modulo");
	  return (int_atom_zero ());
	}
	if (!rul___mol_is_valid_number (mol2)) {
	  print_nonnumval_msg (mol2, "modulo");
	  return (int_atom_zero ());
	}

	if (mol1->type == iatom  &&  mol2->type == iatom) {
	    return (rul__mol_make_int_atom (
				    M_INT(mol1)->ival  %  M_INT(mol2)->ival));
	} else {

	    i1 = M_INT(mol1)->ival;
	    i2 = M_INT(mol2)->ival;

	    if (mol1->type != iatom) {
		rul__msg_print_w_atoms (RTS_INVMOD, 1, mol1);
		i1 = M_DBL(mol1)->dval;
	    }
	    if (mol2->type != iatom) {
		rul__msg_print_w_atoms (RTS_INVMODULO, 1, mol2);
		i2 = M_DBL(mol2)->dval;
	    }
	    return (rul__mol_make_int_atom (i1 % i2));
	}
}




/**************************
**                       **
**  PRINT_NONNUMVAL_MSG  **
**                       **
**************************/

void print_nonnumval_msg (Molecule mol, char *oper)
{
  char buf1[RUL_C_MAX_SYMBOL_SIZE + 1];
  char buf2[RUL_C_MAX_SYMBOL_SIZE + 1] = ", in arithmetic ";

  rul__mol_use_readform (mol, buf1, RUL_C_MAX_SYMBOL_SIZE);
  strcat (buf2, oper);
  strcat (buf2, " operation");
  rul__msg_print (RTS_NONNUMVAL, buf1, buf2);
}



static Molecule int_atom_zero (void)
{
	static Molecule zero = NULL;

	if (zero == NULL) {
	    zero = rul__mol_make_int_atom (0);
	    rul__mol_mark_perm (zero);
	}
	return zero;
}
