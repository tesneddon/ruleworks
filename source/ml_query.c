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
**  FACILITY:
**	RULEWORKS compiler and run-time system
**
**  ABSTRACT:
**	This module contains the functions which allow other
**	subsystems access to the contents of molecular values.
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
**	15-Mar-1992	DEC	Initial version
**
**	17-Jun-1994	DEC	rul__mol_use_printform() needs length+1 for
**					max_chars parameter
**
**	01-Dec-1999	CPQ	Release with GPL
*/



#include <common.h>
#include <stdarg.h>
#include <mol.h>
#include <mol_p.h>
#include <ios.h>
#include <dyar.h>
#include <msg.h>
#include <rts_msg.h>

/***********************
**                    **
**  RUL__MOL_IS_ATOM  **
**                    **
***********************/

Boolean	rul__mol_is_atom (Molecule mol)
{
	assert (rul___mol_is_valid (mol));

	return (is_atomic(mol));
}



/***************************
**                        **
**  RUL__MOL_IS_POLYATOM  **
**                        **
***************************/

Boolean	rul__mol_is_polyatom (Molecule mol)
{
	assert (rul___mol_is_valid (mol));

	return (is_polyatomic(mol));
}



/*************************
**                      **
**  RUL__MOL_IS_NUMBER  **
**                      **
*************************/

Boolean	rul__mol_is_number (Molecule mol)
{
	assert (rul___mol_is_valid (mol));

	return (is_numeric(mol));
}



/*************************
**                      **
**  RUL__MOL_IS_SYMBOL  **
**                      **
*************************/

Boolean	rul__mol_is_symbol (Molecule mol)
{
	assert (rul___mol_is_valid (mol));

	return (mol->type == satom);
}



/***************************
**                        **
**  RUL__MOL_IS_INT_ATOM  **
**                        **
***************************/

Boolean	rul__mol_is_int_atom (Molecule mol)
{
	assert (rul___mol_is_valid (mol));

	return (mol->type == iatom);
}



/***************************
**                        **
**  RUL__MOL_IS_DBL_ATOM  **
**                        **
***************************/

Boolean	rul__mol_is_dbl_atom (Molecule mol)
{
	assert (rul___mol_is_valid (mol));

	return (mol->type == datom);
}



/*************************
**                      **
**  RUL__MOL_IS_OPAQUE  **
**                      **
*************************/

Boolean	rul__mol_is_opaque (Molecule mol)
{
	assert (rul___mol_is_valid (mol));

	return (mol->type == oatom);
}



/******************************
**                           **
**  RUL__MOL_IS_INSTANCE_ID  **
**                           **
******************************/

Boolean	rul__mol_is_instance_id (Molecule mol)
{
	assert (rul___mol_is_valid (mol));

	return (mol->type == watom);
}



/***************************
**                        **
**  RUL__MOL_IS_COMPOUND  **
**                        **
***************************/

Boolean	rul__mol_is_compound (Molecule mol)
{
	assert (rul___mol_is_valid (mol));

	return (mol->type == cpoly);
}



/************************
**                     **
**  RUL__MOL_IS_TABLE  **
**                     **
************************/

Boolean	rul__mol_is_table (Molecule mol)
{
	assert (rul___mol_is_valid (mol));

	return (mol->type == tpoly);
}



/******************************
**                           **
**  RUL__MOL_GET_VALUE_TYPE  **
**                           **
******************************/

Molecule_Type	 rul__mol_get_value_type (Molecule mol)
{
    	register Molecule_Type mol_type;

    	assert (rul___mol_is_valid (mol));

        switch (mol->type) {
            case satom :	mol_type = symbol;			break;
            case iatom :	mol_type = int_atom;			break;
            case datom :	mol_type = dbl_atom;			break;
            case oatom :	mol_type = opaque;			break;
            case watom :	mol_type = instance_id;			break;
	    case cpoly :	mol_type = compound;			break;
	    case tpoly :	mol_type = table;			break;
	    default :		mol_type = invalid_molecule_type;	break;
	}
	return (mol_type);
}



/*************************
**                      **
**  RUL__MOL_GET_SHAPE  **
**                      **
*************************/

Decl_Shape rul__mol_get_shape (Molecule mol)
{
	assert (rul___mol_is_valid (mol));
	if (mol->type == cpoly) return shape_compound;
	if (mol->type == tpoly) return shape_table;
	return shape_atom;
}


/**************************
**                       **
**  RUL__MOL_GET_DOMAIN  **
**                       **
**************************/

Decl_Domain rul__mol_get_domain (Molecule mol)
{
	Decl_Domain dom;
	Molecule elem;
	long i, length;

    	assert (rul___mol_is_valid (mol));

        switch (mol->type) {
            case satom :
			return dom_symbol;	 
			break;
            case iatom :	
			return dom_int_atom;	 
			break;
            case datom :	
			return dom_dbl_atom;	 
			break;
            case oatom :	
			return dom_opaque;
			break;
            case watom :
			return dom_instance_id;
			break;

	    case cpoly :
			length = rul__mol_get_poly_count (mol);
			if (length == 0) return dom_any;
			for (i=1; i<=length; i++) {
			    elem = rul__mol_get_comp_nth (mol, i);
			    if (i == 1) {
				dom = rul__mol_get_domain (elem);
			    } else {
				dom = rul__mol_domain_union (dom,
						rul__mol_get_domain (elem));
			    }
			}
			return dom;
			break;
	}
	assert (FALSE);
	return dom_invalid;
}


/****************************
**                         **
**  RUL__MOL_DOMAIN_UNION  **
**                         **
****************************/

Decl_Domain rul__mol_domain_union (Decl_Domain dom1, Decl_Domain dom2)
{
	Decl_Domain dom;
	if (rul__mol_is_subdomain (dom1, dom2)) return dom1;
	if (rul__mol_is_subdomain (dom2, dom1)) return dom2;

	if (dom1 == dom_int_atom  &&  dom2 == dom_dbl_atom) return dom_number;
	if (dom2 == dom_int_atom  &&  dom1 == dom_dbl_atom) return dom_number;

	return dom_any;
}



/****************************
**                         **
**  RUL__MOL_IS_SUBDOMAIN  **
**                         **
****************************/

Boolean rul__mol_is_subdomain (
		Decl_Domain required_domain, Decl_Domain actual_domain)
{
	if (required_domain == dom_any)  return TRUE;
        if (actual_domain == required_domain)  return TRUE;
        if (required_domain == dom_number &&
                (actual_domain == dom_int_atom || 
		 actual_domain == dom_dbl_atom)) {
            return TRUE;
	}
	return FALSE;
}



/******************************
**                           **
**  RUL__MOL_INT_ATOM_VALUE  **
**                           **
******************************/

long	rul__mol_int_atom_value (Mol_Int_Atom mol)
{
	assert (rul___mol_is_valid_atom_of_type (mol, iatom));

	return ( ((Integer_Value *)mol)->ival );
}



/******************************
**                           **
**  RUL__MOL_DBL_ATOM_VALUE  **
**                           **
******************************/

double	rul__mol_dbl_atom_value (Mol_Dbl_Atom mol)
{
	assert (rul___mol_is_valid_atom_of_type (mol, datom));

        return ( ((Double_Value *)mol)->dval );
}



/*********************************
**                              **
**  RUL__MOL_NUMBER_AS_INTEGER  **
**                              **
*********************************/

long	rul__mol_number_as_integer (Mol_Number mol)
{
	long	ival;

	assert (rul___mol_is_valid_number (mol));

	if (mol->type == iatom) {
	    return ( ((Integer_Value *)mol)->ival );
	} else if (mol->type == datom) {
	    ival = ((Double_Value *)mol)->dval;
	    return (ival);
	}
	return (0);
}



/********************************
**                             **
**  RUL__MOL_NUMBER_AS_DOUBLE  **
**                             **
********************************/

double	rul__mol_number_as_double (Mol_Number mol)
{
	double	dval;

	assert (rul___mol_is_valid_number (mol));

	if (mol->type == datom) {
	    return ( ((Double_Value *)mol)->dval );
	} else if (mol->type == iatom) {
	    dval = ((Integer_Value *)mol)->ival;
	    return (dval);
	}
	return (0.0);
}



/****************************
**                         **
**  RUL__MOL_SYMBOL_VALUE  **
**                         **
****************************/

char	*rul__mol_symbol_value (Mol_Symbol mol)
{
	char	*str;

	assert (rul___mol_is_valid_atom_of_type (mol, satom));

        str = (char *) rul__mem_malloc (1 + rul___mol_printform_length (mol));

        if (str == NULL) {
	    /*?*/ rul__ios_printf ( RUL__C_STD_ERR, "\n Error:  malloc failure in rul__mol_symbol_value\n");
            return (NULL);
        }
        strcpy (str, ((Symbol_Value *)mol)->sval);
        return (str);
}



/****************************
**                         **
**  RUL__MOL_OPAQUE_VALUE  **
**                         **
****************************/

void	*rul__mol_opaque_value (Mol_Opaque mol)
{
	assert (rul___mol_is_valid_atom_of_type (mol, oatom));

	return ( ((Opaque_Value *)mol)->oval );
}




/*********************************
**                              **
**  RUL__MOL_INSTANCE_ID_VALUE  **
**                              **
*********************************/

Object	rul__mol_instance_id_value (Mol_Instance_Id mol)
{
	if (mol == NULL) return NULL;
	assert (rul___mol_is_valid_atom_of_type (mol, watom));

	return ( ((Instance_Id_Value *)mol)->instance_ptr );
}



/*************************************
**                                  **
**  RUL__MOL_SET_INSTANCE_ID_VALUE  **
**                                  **
*************************************/

void	rul__mol_set_instance_id_value (Mol_Instance_Id mol,
					Object instance_ptr)
{
	Instance_Id_Value *wmol ;

	assert (rul___mol_is_valid_atom_of_type (mol, watom));

	wmol = (Instance_Id_Value *) mol;
	wmol->instance_ptr = instance_ptr;
}



/************************
**                     **
**  MOL_ALREADY_FREED  **
**                     **
************************/

static void mol_already_freed (Molecule mol)
{
	if (is_atomic(mol)) {
	    rul__ios_printf (RUL__C_STD_ERR, 
			"\n Internal Error:  already freed atom, ");
	} else {
	    rul__ios_printf (RUL__C_STD_ERR, 
			"\n Internal Error:  already freed compound, ");
	}
	rul___mol_quiet_print_printform (mol, RUL__C_STD_ERR);
	rul__ios_printf (RUL__C_STD_ERR, ", received.\n");
}


/*************************
**                      **
**  RUL___MOL_IS_VALID  **
**                      **
*************************/

Boolean	rul___mol_is_valid (Molecule mol)
	/*
	**	This private version should only be used from within
	**	assert statements within this sub-system.
	*/
{
	if (mol == NULL) {
	    rul__ios_printf (RUL__C_STD_ERR,
	    		"\n Internal Error:  null molecule received.\n");
	    return (FALSE);
	}

	if (mol->type != iatom  &&  mol->type != datom  &&
	    mol->type != satom  &&  mol->type != oatom  &&
	    mol->type != watom  &&  mol->type != cpoly  &&
	    mol->type != tpoly) {
		rul__ios_printf (RUL__C_STD_ERR,
			"\n Internal Error:  non-molecule received.\n");
		return (FALSE);
	}

	if (mol->ref_count == 0) {
/* SB: 	    mol_already_freed (mol);*/
	    return (TRUE); /* we'll be nice */
	}

	return (TRUE);
}




/************************
**                     **
**  RUL__MOL_IS_VALID  **
**                     **
************************/

Boolean	rul__mol_is_valid (Molecule mol)
	/*
	**	The exported version of the function above.
	*/
{
	if (mol == NULL) {
	    return (FALSE);
	}

	if (mol->type != iatom  &&  mol->type != datom  &&
	    mol->type != satom  &&  mol->type != oatom  &&
	    mol->type != watom  &&  mol->type != cpoly  &&
	    mol->type != tpoly) {
		return (FALSE);
	}

	if (mol->ref_count == 0) {
#ifndef NDEBUG
/* SB:	    mol_already_freed (mol);*/
	    return (TRUE);
#else
	    return (FALSE);
#endif
	}

	return (TRUE);
}



/******************************
**                           **
**  RUL___MOL_IS_VALID_ATOM  **
**                           **
******************************/

Boolean	rul___mol_is_valid_atom (Molecule mol)
{
  if (rul___mol_is_valid (mol)) {
    if (is_atomic(mol)) {
      return (TRUE);
    }
  }
  return (FALSE);
}


/********************************
**                             **
**  RUL___MOL_IS_VALID_NUMBER  **
**                             **
********************************/

Boolean	rul___mol_is_valid_number (Molecule mol)
{
  if (rul___mol_is_valid_atom (mol)) {
    if (is_numeric(mol)) {
      return (TRUE);
    }
  }
  return (FALSE);
}





/**************************************
**                                   **
**  RUL___MOL_IS_VALID_ATOM_OF_TYPE  **
**                                   **
**************************************/

Boolean	rul___mol_is_valid_atom_of_type (Molecule mol, Private_Mol_Type typ)
{
  if (rul___mol_is_valid_atom (mol)) {
    if (mol->type == typ) {
      return (TRUE);
    }
    else {
      rul__ios_printf ( RUL__C_STD_ERR, "\n Internal Error:  ");
      switch (mol->type) {
      case satom :
	rul__ios_printf ( RUL__C_STD_ERR, "symbol");
	break;
      case iatom :
	rul__ios_printf ( RUL__C_STD_ERR, "integer");
	break;
      case datom :
	rul__ios_printf ( RUL__C_STD_ERR, "float");
	break;
      case oatom :
	rul__ios_printf ( RUL__C_STD_ERR, "opaque");
	break;
      case watom :
	rul__ios_printf ( RUL__C_STD_ERR, "instance-id");
	break;
      }
      rul__ios_printf ( RUL__C_STD_ERR, " argument received; expected ");
      switch (typ) {
      case satom :
	rul__ios_printf ( RUL__C_STD_ERR, "symbol");
	break;
      case iatom :
	rul__ios_printf ( RUL__C_STD_ERR, "integer");
	break;
      case datom :
	rul__ios_printf ( RUL__C_STD_ERR, "float");
	break;
      case oatom :
	rul__ios_printf ( RUL__C_STD_ERR, "opaque");
	break;
      case watom :
	rul__ios_printf ( RUL__C_STD_ERR, "instance-id");
	break;
      }
      rul__ios_printf ( RUL__C_STD_ERR, ".\n");
    }
  }
  return (FALSE);
}




/******************************
**                           **
**  RUL___MOL_IS_VALID_POLY  **
**                           **
******************************/

Boolean	rul___mol_is_valid_poly (Molecule mol)
{
  if (rul___mol_is_valid (mol)) {
    if (is_polyatomic(mol)) {
      return (TRUE);
    }
    else {
      rul__ios_printf ( RUL__C_STD_ERR,
		       "\n Internal Error:  atomic argument received;");
      rul__ios_printf ( RUL__C_STD_ERR, " expected a polyatom.\n");
    }
  }
  return (FALSE);
}



/*******************************
**                            **
**  RUL___MOL_IS_VALID_CPOLY  **
**                            **
*******************************/

Boolean	rul___mol_is_valid_cpoly (Molecule mol)
{
  if (rul___mol_is_valid (mol)) {
    if (mol->type == cpoly) {
      return (TRUE);
    } else {
      rul__ios_printf ( RUL__C_STD_ERR,
		       "\n Internal Error:  atomic argument received;");
      rul__ios_printf ( RUL__C_STD_ERR, " expected a compound\n");
    }
  }
  return (FALSE);
}




/****************************
**                         **
**  RUL__MOL_CONCAT_ATOMS  **
**                         **
****************************/

Mol_Symbol  rul__mol_concat_atoms (long mol_count, ...)
{
  va_list ap;
  Molecule mol, out_mol, cmol;
  Dynamic_Array vector = NULL;
  long i, j, k, len;
  char *buffer;
  
  assert (mol_count > 0);

  vector = rul__dyar_create_array (mol_count + 10);
  va_start (ap, mol_count);

  len = 0;
  for (i = 0; i < mol_count; i++) {
    mol = va_arg (ap, Molecule);
    assert (rul___mol_is_valid (mol));
    
    if (len > RUL_C_MAX_SYMBOL_SIZE)
      break;

    if (rul__mol_is_atom (mol)) {
      rul__dyar_append (vector, mol);
      len += rul__mol_get_printform_length (mol);
    }

    else {
      k = rul__mol_get_poly_count (mol);
      for (j = 1; j <= k; j++) {
	cmol = rul__mol_get_comp_nth (mol, j);
	rul__dyar_append (vector, cmol);
	len += rul__mol_get_printform_length (cmol);
      }
    }
  }
  va_end (ap);
  
  if (len == 0) {
    out_mol = rul__mol_make_symbol ("");
  }
  
  else {
    buffer = (char *) rul__mem_malloc ((len + 1) * sizeof(char));
    buffer[0] = '\0';
    while (mol = (Molecule) rul__dyar_pop_first (vector)) {
      /* Note that len+1 is correct for only the first call, but that's OK
       * since we know we allocated the buffer to be big enough. */
      rul__mol_use_printform (mol, &(buffer[strlen(buffer)]), len + 1);
      /* len -= rul__mol_get_printform_length (mol); */
    }

    if (len > RUL_C_MAX_SYMBOL_SIZE) {
      len = RUL_C_MAX_SYMBOL_SIZE;
      buffer[len] = '\0';
      rul__msg_print (RTS_FATCAT, buffer);
    }

    out_mol = rul__mol_make_symbol (buffer);
    rul__mem_free (buffer);
  }
  if (vector)
    rul__dyar_free_array (vector);
  return (out_mol);
}

