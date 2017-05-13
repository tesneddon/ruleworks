/****************************************************************************
**                                                                         **
**             R T S _ M O L _ S C A L A R _ P R E D S . C                 **
**                                                                         **
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
**	This module contains the functions for performing scalar
**	predicate comparisons of molecular values.
**
**	These routines are used at run-time.
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
**	8-Jul-1992	DEC	Initial version
**	01-Dec-1999	CPQ	Release with GPL
*/


#include <common.h>
#include <stdarg.h>
#include <mol.h>
#include <mol_p.h>
#include <i18n.h>
#include <ctype.h>		/* For toupper */
#include <msg.h>
#include <rts_msg.h>

#define M_INT(mol)  ((Integer_Value *) mol)
#define M_DBL(mol)  ((Double_Value *) mol)
#define M_SYM(mol)  ((Symbol_Value *) mol)

static Boolean  off_by_insert_1 (char *longer_str, char *shorter_str);
static Boolean  off_by_one_transpose (char *str1, char *str2);
static Boolean  rul___mol_is_greater (Molecule mol1, Molecule mol2);
static Boolean  rul___mol_is_less (Molecule mol1, Molecule mol2);




/***********************
**                    **
**  RUL__MOL_EQ_PRED  **
**                    **
***********************/

Boolean rul__mol_eq_pred (Molecule mol1, Molecule mol2)
{
  if (mol1 == NULL || mol2 == NULL)
    return (FALSE);

  assert (rul__mol_is_valid (mol1));
  assert (rul__mol_is_valid (mol2));

  if (mol1 == mol2)
    return (TRUE);

  return (FALSE);
}

/***************************
**                        **
**  RUL__MOL_NOT_EQ_PRED  **
**                        **
***************************/

Boolean rul__mol_not_eq_pred (Molecule mol1, Molecule mol2)
{
  assert (rul__mol_is_valid (mol1));
  assert (rul__mol_is_valid (mol2));

  return (! rul__mol_eq_pred (mol1, mol2));
}




/**************************
**                       **
**  RUL__MOL_EQUAL_PRED  **
**                       **
**************************/

Boolean rul__mol_equal_pred (Molecule mol1, Molecule mol2)
{
    if (mol1 == NULL  ||  mol2 == NULL) return (FALSE);

    assert (rul__mol_is_valid (mol1));
    assert (rul__mol_is_valid (mol2));

    if (mol1 == mol2) return (TRUE);

    if (is_numeric(mol1)  &&  is_numeric(mol2)) {
        if (mol1->type == iatom) {
	    if (mol2->type == iatom) {
	        if (M_INT(mol1)->ival == M_INT(mol2)->ival) return (TRUE);
	    } else {
	        if (M_INT(mol1)->ival == M_DBL(mol2)->dval) return (TRUE);
	    }
	} else {
	    if (mol2->type == iatom) {
	        if (M_DBL(mol1)->dval == M_INT(mol2)->ival) return (TRUE);
	    } else {
	        if (M_DBL(mol1)->dval == M_DBL(mol2)->dval) return (TRUE);
	    }
	}
    }
    else if (mol1->type == satom  &&  mol2->type == satom) {
        if (rul__i18n_string_compare (M_SYM(mol1)->sval,
				      M_SYM(mol2)->sval) == 0) return (TRUE);
    }
    return (FALSE);
}


/******************************
**                           **
**  RUL__MOL_NOT_EQUAL_PRED  **
**                           **
******************************/

Boolean rul__mol_not_equal_pred (Molecule mol1, Molecule mol2)
{
    assert (rul__mol_is_valid (mol1));
    assert (rul__mol_is_valid (mol2));

    return (! rul__mol_equal_pred (mol1, mol2));
}



/***********************
**                    **
**  RUL__MOL_LT_PRED  **
**                    **
***********************/

Boolean rul__mol_lt_pred (Molecule mol1, Molecule mol2)
{
    if (mol1 == NULL  ||  mol2 == NULL) return (FALSE);

    assert (rul__mol_is_valid (mol1));
    assert (rul__mol_is_valid (mol2));

    if (mol1 == mol2) return (FALSE);

    if (is_numeric(mol1)  &&  is_numeric(mol2)) {
        if (mol1->type == iatom) {
	    if (mol2->type == iatom) {
	        if (M_INT(mol1)->ival  <  M_INT(mol2)->ival) return (TRUE);
	    } else {
	        if (M_INT(mol1)->ival  <  M_DBL(mol2)->dval) return (TRUE);
	    }
	} else {
	    if (mol2->type == iatom) {
	        if (M_DBL(mol1)->dval  <  M_INT(mol2)->ival) return (TRUE);
	    } else {
	        if (M_DBL(mol1)->dval  <  M_DBL(mol2)->dval) return (TRUE);
	    }
	}
    }
    else if (mol1->type == satom  &&  mol2->type == satom) {
        if (rul__i18n_string_compare (M_SYM(mol1)->sval,
				      M_SYM(mol2)->sval) < 0) return (TRUE);
    }
    return (FALSE);
}




/************************
**                     **
**  RUL__MOL_LTE_PRED  **
**                     **
************************/

Boolean rul__mol_lte_pred (Molecule mol1, Molecule mol2)
{
    if (mol1 == NULL  ||  mol2 == NULL) return (FALSE);

    assert (rul__mol_is_valid (mol1));
    assert (rul__mol_is_valid (mol2));

    if (mol1 == mol2) return (TRUE);

    if (is_numeric(mol1)  &&  is_numeric(mol2)) {
        if (mol1->type == iatom) {
	    if (mol2->type == iatom) {
	        if (M_INT(mol1)->ival <= M_INT(mol2)->ival) return (TRUE);
	    } else {
	        if (M_INT(mol1)->ival <= M_DBL(mol2)->dval) return (TRUE);
	    }
	} else {
	    if (mol2->type == iatom) {
	        if (M_DBL(mol1)->dval <= M_INT(mol2)->ival) return (TRUE);
	    } else {
	       if (M_DBL(mol1)->dval <= M_DBL(mol2)->dval) return (TRUE);
	    }
	}
    }
    else if (mol1->type == satom  &&  mol2->type == satom) {
        if (rul__i18n_string_compare (M_SYM(mol1)->sval,
				      M_SYM(mol2)->sval) <= 0) return (TRUE);
    }
    return (FALSE);
}




/***********************
**                    **
**  RUL__MOL_GT_PRED  **
**                    **
***********************/

Boolean rul__mol_gt_pred (Molecule mol1, Molecule mol2)
{
    if (mol1 == NULL  ||  mol2 == NULL) return (FALSE);

    assert (rul__mol_is_valid (mol1));
    assert (rul__mol_is_valid (mol2));

    if (mol1 == mol2) return (FALSE);

    if (is_numeric(mol1)  &&  is_numeric(mol2)) {
        if (mol1->type == iatom) {
	    if (mol2->type == iatom) {
	        if (M_INT(mol1)->ival  >  M_INT(mol2)->ival) return (TRUE);
	    } else {
	        if (M_INT(mol1)->ival  >  M_DBL(mol2)->dval) return (TRUE);
	    }
	} else {
	    if (mol2->type == iatom) {
	        if (M_DBL(mol1)->dval  >  M_INT(mol2)->ival) return (TRUE);
	    } else {
	        if (M_DBL(mol1)->dval  >  M_DBL(mol2)->dval) return (TRUE);
	    }
	}
    }
    else if (mol1->type == satom  &&  mol2->type == satom) {
        if (rul__i18n_string_compare (M_SYM(mol1)->sval,
				      M_SYM(mol2)->sval) > 0) return (TRUE);
    }
    return (FALSE);
}




/************************
**                     **
**  RUL__MOL_GTE_PRED  **
**                     **
************************/

Boolean rul__mol_gte_pred (Molecule mol1, Molecule mol2)
{
    if (mol1 == NULL  ||  mol2 == NULL) return (FALSE);

    assert (rul__mol_is_valid (mol1));
    assert (rul__mol_is_valid (mol2));

    if (mol1 == mol2) return (TRUE);

    if (is_numeric(mol1)  &&  is_numeric(mol2)) {
        if (mol1->type == iatom) {
	    if (mol2->type == iatom) {
	        if (M_INT(mol1)->ival  >=  M_INT(mol2)->ival) return (TRUE);
	    } else {
	        if (M_INT(mol1)->ival  >=  M_DBL(mol2)->dval) return (TRUE);
	    }
	} else {
	    if (mol2->type == iatom) {
	        if (M_DBL(mol1)->dval  >=  M_INT(mol2)->ival) return (TRUE);
	    } else {
	        if (M_DBL(mol1)->dval  >=  M_DBL(mol2)->dval) return (TRUE);
	    }
	}
    }
    else if (mol1->type == satom  &&  mol2->type == satom) {
        if (rul__i18n_string_compare (M_SYM(mol1)->sval,
				      M_SYM(mol2)->sval) >= 0) return (TRUE);
    }
    return (FALSE);
}




/******************************
**                           **
**  RUL__MOL_SAME_TYPE_PRED  **
**                           **
******************************/

Boolean rul__mol_same_type_pred (Molecule mol1, Molecule mol2)
{
	if (mol1 == NULL  ||  mol2 == NULL)  return FALSE;

	assert (rul__mol_is_valid (mol1));
	assert (rul__mol_is_valid (mol2));

	if (is_polyatomic(mol1) || is_polyatomic(mol2))  return FALSE;
	return (mol1->type == mol2->type);
}



/******************************
**                           **
**  RUL__MOL_DIFF_TYPE_PRED  **
**                           **
******************************/

Boolean rul__mol_diff_type_pred (Molecule mol1, Molecule mol2)
{
	if (mol1 == NULL  ||  mol2 == NULL)  return FALSE;

	assert (rul__mol_is_valid (mol1));
	assert (rul__mol_is_valid (mol2));

	if (is_polyatomic(mol1) || is_polyatomic(mol2))  return FALSE;
	return (mol1->type != mol2->type);
}




/*******************************
**                            **
**  RUL___MOL_SOUNDEX_SYMBOL  **
**                            **
*******************************/

Mol_Symbol rul___mol_soundex_symbol (Mol_Symbol mol)
{
	char *soundex_str;
	Symbol_Value *smol = (Symbol_Value *) mol;

	assert (rul___mol_is_valid (mol)  &&  mol->type == satom);

	if (smol->soundex_symbol == NULL) {

	    soundex_str = rul__i18n_string_soundex (smol->sval);
	    smol->soundex_symbol = rul__mol_make_symbol (soundex_str);
	    rul__mem_free (soundex_str);
	}
	return ((Mol_Symbol) smol->soundex_symbol) ;
}


#define MAX2(x,y)	((x)>(y)?(x):(y))
#define ABS1(x)		((x)<0.0?((x)*-1.0):(x))



/**********************************
**                               **
**  RUL__MOL_NOT_APPROX_EQ_PRED  **
**                               **
**********************************/

Boolean rul__mol_not_approx_eq_pred (Molecule mol1, Molecule mol2)
{
	return (! rul__mol_approx_eq_pred (mol1, mol2));
}


/******************************
**                           **
**  RUL__MOL_APPROX_EQ_PRED  **
**                           **
******************************/

Boolean rul__mol_approx_eq_pred (Molecule mol1, Molecule mol2)
{
	double d_1, d_2, dmax, ddif;
	Symbol_Value *smol1, *smol2;
	long len1, len2;

	if (mol1 == NULL  ||  mol2 == NULL) return (FALSE);

	assert (rul__mol_is_valid (mol1));
	assert (rul__mol_is_valid (mol2));

	if (mol1 == mol2) return (TRUE);

	if (is_numeric(mol1)  &&  is_numeric(mol2)) {
	    /*
	    **  For numbers, first convert them both into doubles
	    */
	    if (mol1->type == iatom) {
		d_1 = M_INT(mol1)->ival;
	    } else {
		d_1 = M_DBL(mol1)->dval;
	    }
	    if (mol2->type == iatom) {
		d_2 = M_INT(mol2)->ival;
	    } else {
		d_2 = M_DBL(mol2)->dval;
	    }
	    /*
	    **  Now compute the absolute value of their difference
	    */
	    ddif = ABS1(d_1 - d_2);
	    dmax = MAX2(ABS1(d_1), ABS1(d_2));
	    if (ddif <= (0.01 * dmax)) {
		return TRUE;
	    } else {
		return FALSE;
	    }
	}

	if (mol1->type == satom  &&  mol2->type == satom) {
	    smol1 = (Symbol_Value *) mol1;
	    smol2 = (Symbol_Value *) mol2;

	    if (rul___mol_soundex_symbol (mol1) == 
		rul___mol_soundex_symbol (mol2)) return (TRUE);

	    len1 = rul___mol_printform_length(mol1);
	    if (len1 < 3) return (FALSE);
	    len2 = rul___mol_printform_length(mol2);
	    if (len2 < 3) return (FALSE);

	    if ((len1 - len2) == 1) {
	    	/*  Check for:		off by insert 1*/
		return (off_by_insert_1 (smol1->sval, smol2->sval));
	    }
	    if ((len2 - len1) == 1) {
	    	/*  Check for:		the other off by insert 1 */
		return (off_by_insert_1 (smol2->sval, smol1->sval));
	    }
	    if (len1 == len2) {
  		/*  Check for:		off by transpose 2  */
		return (off_by_one_transpose (smol1->sval, smol2->sval));
	    }
	}
	return (FALSE);
}



/**********************
**                   **
**  OFF_BY_INSERT_1  **
**                   **
**********************/

static Boolean off_by_insert_1 (char *long_str, char *short_str)
{
	long i;
	long shorter_len = strlen (short_str);
	char sbuff[RUL_C_MAX_SYMBOL_SIZE+1], lbuff[RUL_C_MAX_SYMBOL_SIZE+1];

	for (i=0; i<=shorter_len; i++) {
	    sbuff[i] = toupper(short_str[i]);
	    lbuff[i] = toupper(long_str[i]);
	}
	lbuff[shorter_len+1] = '\0';

	/*  walk over the common prefix (if any)  */
	i = 0;
	while (sbuff[i] == lbuff[i]) i++;
	if (lbuff[i+1] == sbuff[i]) {
	    return (0 == strcmp (&(lbuff[i+1]), &(sbuff[i])));
	}
	return (FALSE);
}



/***************************
**                        **
**  OFF_BY_ONE_TRANSPOSE  **
**                        **
***************************/

static Boolean off_by_one_transpose (char *str1, char *str2)
{
	long i;
	long len = strlen (str1);
	char buff_1[RUL_C_MAX_SYMBOL_SIZE+1], buff_2[RUL_C_MAX_SYMBOL_SIZE+1];

	for (i=0; i<=len; i++) {
	    buff_1[i] = toupper(str1[i]);
	    buff_2[i] = toupper(str2[i]);
	}

	/*  walk over the common prefix (if any)  */
	i = 0;
	while (buff_1[i] == buff_2[i]) i++;
	if (buff_1[i+1] == buff_2[i]  &&  buff_2[i+1] == buff_1[i]) {
	    return (0 == strcmp (&(buff_2[i+2]), &(buff_1[i+2])));
	}
	return (FALSE);
}




/***********************
**                    **
**  RUL__MOL_MAX_MIN  **
**                    **
***********************/

Mol_Symbol  rul__mol_max_min (Boolean is_max, long mol_count, ...)
{
  va_list ap;
  Molecule mol, cmol, cur_mol = NULL;
  long i, j, k;
  Private_Mol_Type cur_type;
  Boolean comp_flag = FALSE;

  assert (mol_count > 0);
  
  va_start (ap, mol_count);
  
  for (i = 1; i <= mol_count; i++) {
    mol = va_arg (ap, Molecule);
    assert (rul___mol_is_valid (mol));
    
    if (rul__mol_is_atom (mol)) {
      i += 1;
      cur_mol = mol;
    }
    else if (rul__mol_get_poly_count (mol)) {
      cur_mol = rul__mol_get_comp_nth (mol, 1);
      comp_flag = TRUE;
    }

    if (cur_mol) {
      cur_type = cur_mol->type;
      break;
    }
  }

  for (; cur_mol != NULL && i <= mol_count; i++) {

    if (!comp_flag)
      mol = va_arg (ap, Molecule);
      
    assert (rul___mol_is_valid (mol));
    
    if (rul__mol_is_atom (mol)) {
      if (is_max) {
	if (rul___mol_is_greater (mol, cur_mol))
	  cur_mol = mol;
      }
      else {
	if (rul___mol_is_less (mol, cur_mol))
	  cur_mol = mol;
      }
    }
    else {
      k = rul__mol_get_poly_count (mol);
      j = 1;
      if (comp_flag) {
	j = 2;
	comp_flag = FALSE;
      }
      for (; j <= k; j++) {
	cmol = rul__mol_get_comp_nth (mol, j);
	if (is_max) {
	  if (rul___mol_is_greater (cmol, cur_mol))
	    cur_mol = cmol;
	}
	else {
	  if (rul___mol_is_less (cmol, cur_mol))
	    cur_mol = cmol;
	}
      }
    }
  }
  
  if (cur_mol) {
    rul__mol_incr_uses (cur_mol);
    return (cur_mol);
  }

  return rul__mol_symbol_nil ();
}


static Boolean rul___mol_is_greater (Molecule mol1, Molecule mol2)
{

  switch (mol2->type) {

  case iatom :
  case datom :
    if (rul__mol_is_number (mol1))
      return (rul__mol_gt_pred (mol1, mol2));
    break;

  case satom :
    if (rul__mol_is_symbol (mol1))
      return (rul__mol_gt_pred (mol1, mol2));
    break;

  case oatom :
    if (rul__mol_is_opaque (mol1))
      return (((Opaque_Value *) mol1)->oval > ((Opaque_Value *) mol2)->oval);
    break;

  case watom :
    if (rul__mol_is_instance_id (mol1))
      return (((Instance_Id_Value *) mol1)->wval >
	      ((Instance_Id_Value *) mol2)->wval);
    break;
  }

  rul__msg_print_w_atoms (RTS_INVMAXMIN, 1, mol1);
  return (FALSE);
}

static Boolean rul___mol_is_less (Molecule mol1, Molecule mol2)
{

  switch (mol2->type) {

  case iatom :
  case datom :
    if (rul__mol_is_number (mol1))
      return (rul__mol_lt_pred (mol1, mol2));
    break;

  case satom :
    if (rul__mol_is_symbol (mol1))
      return (rul__mol_lt_pred (mol1, mol2));
    break;

  case oatom :
    if (rul__mol_is_opaque (mol1))
      return (((Opaque_Value *) mol1)->oval < ((Opaque_Value *) mol2)->oval);
    break;

  case watom :
    if (rul__mol_is_instance_id (mol1))
      return (((Instance_Id_Value *) mol1)->wval <
	      ((Instance_Id_Value *) mol2)->wval);
    break;
  }

  rul__msg_print_w_atoms (RTS_INVMAXMIN, 1, mol1);
  return (FALSE);
}

Mol_Symbol rul__mol_subsymbol (Mol_Symbol mol, long start_chr, long end_chr)
{
  char buf[RUL_C_MAX_SYMBOL_SIZE+1];
  long len = 0;

  if (rul__mol_is_atom (mol)) {

    if (rul__mol_use_printform (mol, buf, RUL_C_MAX_SYMBOL_SIZE+1))
      len = strlen (buf);

    if (start_chr == -1)
      start_chr = len;

    if (end_chr == -1 || end_chr > len)
      end_chr = len;

    if (start_chr < 1 || start_chr > len || end_chr < start_chr)
      return rul__mol_make_symbol ("");
  
    buf[end_chr] = 0;
    return (rul__mol_make_symbol (&(buf[start_chr - 1])));
  }

  rul__msg_print_w_atoms (RTS_INVSUBSYM, 1, mol);
  return rul__mol_make_symbol ("");
}
