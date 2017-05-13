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
**	RULEWORKS run time system and compiler
**
**  ABSTRACT:
**	This module contains the functions which allow other
**	subsystems access to the print forms of molecular values.
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
**	15-Jun-1992	DEC	Initial version
**	01-Dec-1999	CPQ	Release with GPL
*/


#include <string.h>
#include <common.h>
#include <stdarg.h>
#include <ios.h>
#include <mol_p.h>
#include <mol.h>
#include <msg.h>
#include <rts_msg.h>

static Boolean token_is_a_number (char *str);
static void mol_compute_form_lengths (Molecule mol);




/*****************************
**                          **
**  RUL__MOL_GET_PRINTFORM  **
**                          **
*****************************/

char *
rul__mol_get_printform (Molecule mol)
{
	char	*str;
	long	prtlen;

	assert (rul___mol_is_valid (mol));

	prtlen = rul__mol_get_printform_length (mol);
	str = (char *) rul__mem_malloc (1 + prtlen);

        if (str == NULL) return (NULL);

	rul__mol_use_printform (mol, str, prtlen + 1);
        return (str);
}



/****************************
**                         **
**  RUL__MOL_GET_READFORM  **
**                         **
****************************/

char *
rul__mol_get_readform (Molecule mol)
{
	char	*str;
	long	prtlen;

	assert (rul___mol_is_valid (mol));

	prtlen = rul__mol_get_readform_length (mol);
	str = (char *) rul__mem_malloc (1 + prtlen);

	if (str == NULL) return (NULL);

	rul__mol_use_readform (mol, str, prtlen + 1);
        return (str);
}



/*****************************
**                          **
**  RUL__MOL_USE_PRINTFORM  **
**                          **
*****************************/

Boolean
rul__mol_use_printform (Molecule mol, char *out_buffer, long max_chars)
{
	char	*sval, *tmp;
	long	ival, wval;
	long	prtlen, used_len, i;
	double	dval;
	void	*oval;
	Compound_Value	*cpoly_ptr;

	out_buffer[0] = '\0';

        assert (rul___mol_is_valid (mol));

        prtlen = rul__mol_get_printform_length (mol);
	if (prtlen >= max_chars) {
	    /*
	    **    if the output buffer isn't big enough, return failure.
	    */
	    return (FALSE);
	}

	switch (mol->type) {
            case satom :    sval = ((Symbol_Value *) mol)->sval;
                            strcpy (out_buffer, sval);
                            break;
            case iatom :    ival = ((Integer_Value *) mol)->ival;
                            sprintf (out_buffer, "%ld", ival);
                            break;
            case datom :    dval = ((Double_Value *) mol)->dval;
                            strcpy (out_buffer, rul__mol_format_double(dval));
                            break;
            case oatom :    oval = ((Opaque_Value *) mol)->oval;
                            sprintf (out_buffer, "%%x%p", oval);
                            break;
            case watom :    wval = ((Instance_Id_Value *) mol)->wval;
                            sprintf (out_buffer, "#%ld", wval);
                            break;
            case cpoly :
			    cpoly_ptr = (Compound_Value *) mol;
			    tmp = out_buffer;
			    used_len = 0;
			    for (i=0; i<cpoly_ptr->element_count; i++) {
				if (i > 0) {
				    strcat (tmp, " ");
				    tmp ++;
				}
				rul__mol_use_printform (
					cpoly_ptr->values[i],
					tmp, prtlen + 1 - used_len);
				used_len += strlen (tmp);
				tmp += strlen (tmp);
			    }
			    break;
	    case tpoly :
			    /*?*  NYI  *?*/
			    break;
        }
	return (TRUE);
}



/****************************
**                         **
**  RUL__MOL_USE_READFORM  **
**                         **
****************************/

Boolean
rul__mol_use_readform (Molecule mol, char *out_buffer, long max_chars)
{
	char		*tmp;
	long		i, readlen, used_len;
	Symbol_Value	*smol;
	Compound_Value	*cpoly_ptr;

	out_buffer[0] = '\0';

	assert (rul___mol_is_valid (mol));

        readlen = rul__mol_get_readform_length (mol);
	if (readlen >= max_chars) {
	    /*    if the output buffer isn't big enough, return failure.    */
	    return (FALSE);
	}

	if (mol->type == cpoly) {
	    /*
	    **    Compounds are printed with their normal
	    **    constructor function wrapped around them.
	    */
	    cpoly_ptr = (Compound_Value *) mol;
	    tmp = out_buffer;
	    strcat (tmp, "(COMPOUND");
	    tmp += strlen (tmp);
	    used_len = strlen (tmp);
	    for (i=0; i<cpoly_ptr->element_count; i++) {
		strcat (tmp, " ");
		tmp += 1;
		used_len = strlen (tmp);
		rul__mol_use_readform (
			cpoly_ptr->values[i],
			tmp, readlen + 1 - used_len);
		tmp += strlen (tmp);
	    }
	    strcat (tmp, ")");
            return (TRUE);
	}

	if (mol->type == satom  &&
	    (rul___mol_printform_length(mol) != 
			rul___mol_readform_length(mol))) {
	    /*
	    **    Quoted symbols are printed with quotes
	    */
	    smol = ((Symbol_Value *) mol);
	    rul___mol_quote_string (out_buffer, max_chars, smol->sval);
	    return (TRUE);
	}

	/*
	**    If none of the special conditions apply,
	**    then return the normal printform.
	*/
	return ( rul__mol_use_printform (mol, out_buffer, max_chars));
}




/***********************************
**                                **
**  RUL__MOL_GET_COMP_RAW_LENGTH  **
**                                **
***********************************/

long
rul__mol_get_comp_raw_length (Molecule mol)
{
	long readlen;

	assert (rul___mol_is_valid (mol)  &&  mol->type == cpoly);

	readlen = rul__mol_get_readform_length (mol);

	if (mol->type == cpoly) {
	    return (readlen - strlen ("(COMPOUND )"));
	}
	return readlen;
}



/*************************************
**                                  **
**  RUL__MOL_USE_COMP_RAW_READFORM  **
**                                  **
*************************************/

Boolean
rul__mol_use_comp_raw_readform (Molecule mol,
				char *out_buffer, long max_chars)
{
	char		*tmp;
	long		i, readlen, used_len;
	Compound_Value	*cpoly_ptr;

	out_buffer[0] = '\0';

	assert (rul___mol_is_valid (mol)  &&  mol->type == cpoly);

        readlen = rul__mol_get_comp_raw_length (mol);
	if (readlen >= max_chars) {
	    /*    if the output buffer isn't big enough, return failure.    */
	    return (FALSE);
	}

	/*
	**  The raw readform for a compound is just the readforms
	**  of the elements of the compound seperated by spaces.
	*/
	cpoly_ptr = (Compound_Value *) mol;
	tmp = out_buffer;
	tmp += strlen (tmp);
	used_len = strlen (tmp);
	for (i=0; i<cpoly_ptr->element_count; i++) {
	    if (i > 0) {
		strcat (tmp, " ");
		tmp += 1;
	    }
	    used_len = strlen (tmp);
	    rul__mol_use_readform (
			cpoly_ptr->values[i],
			tmp, readlen + 1 - used_len);
	    tmp += strlen (tmp);
	}
        return (TRUE);
}



/*******************************
**                            **
**  RUL__MOL_PRINT_PRINTFORM  **
**                            **
*******************************/

void
rul__mol_print_printform (Molecule mol, IO_Stream ios)
{
	assert (rul___mol_is_valid (mol));
	rul___mol_quiet_print_printform (mol, ios);
}



/**************************************
**                                   **
**  RUL___MOL_QUIET_PRINT_PRINTFORM  **
**                                   **
**************************************/

void
rul___mol_quiet_print_printform (Molecule mol, IO_Stream ios)
{
	char	*sval;
	long	ival, wval, i;
	double	dval;
	void	*oval;
	Compound_Value *cpoly_ptr;

	switch (mol->type) {
            case satom :    sval = ((Symbol_Value *) mol)->sval;
                            rul__ios_printf (ios, "%s", sval);
                            break;
            case iatom :    ival = ((Integer_Value *) mol)->ival;
                            rul__ios_printf (ios, "%ld", ival);
                            break;
            case datom :    dval = ((Double_Value *) mol)->dval;
                            rul__ios_printf (ios, "%s", 
				rul__mol_format_double(dval));
                            break;
            case oatom :    oval = ((Opaque_Value *) mol)->oval;
                            rul__ios_printf (ios, "%%x%p", oval);
                            break;
            case watom :    wval = ((Instance_Id_Value *) mol)->wval;
                            rul__ios_printf (ios, "#%ld", wval);
                            break;
	    case cpoly :
			    cpoly_ptr = (Compound_Value *) mol;
			    for (i=0; i<cpoly_ptr->element_count; i++) {
				if (i > 0) rul__ios_printf (ios, " ");
				rul__mol_print_printform (
					cpoly_ptr->values[i], ios);
			    }
			    break;
	    case tpoly :
			    /*?*  NYI  *?*/
			    break;
        }
}



/******************************
**                           **
**  RUL__MOL_PRINT_READFORM  **
**                           **
******************************/

void
rul__mol_print_readform (Molecule mol, IO_Stream ios)
{
	long		i;
	Compound_Value	*cpoly_ptr;

	assert (rul___mol_is_valid (mol));

	if (mol->type == satom  &&
	       (rul___mol_printform_length(mol) != 
			rul___mol_readform_length(mol))) {

	    char buff[RUL_C_MAX_READFORM_SIZE+1];

	    /*
	    **    Quoted symbols are printed with quotes
	    */
	    rul___mol_quote_string (buff,
			RUL_C_MAX_READFORM_SIZE+1, ((Symbol_Value *)mol)->sval);
	    rul__ios_printf (ios, "%s", buff);
	    return;
	}

	if (mol->type == cpoly) {
	    /*
	    **    Compounds are printed with their normal
	    **    constructor function wrapped around them.
	    */
	    rul__ios_printf (ios, "(COMPOUND");
	    cpoly_ptr = (Compound_Value *) mol;
	    for (i=0; i<cpoly_ptr->element_count; i++) {
		rul__ios_printf (ios, " ");
		rul__mol_print_readform (cpoly_ptr->values[i], ios);
	    }
	    rul__ios_printf (ios, ")");
	    return;
	}

        /*
	**    If none of the special conditions apply,
	**    then print the normal printform.
	*/
	rul__mol_print_printform ( mol, ios);
}




/************************************
**                                 **
**  RUL__MOL_GET_PRINTFORM_LENGTH  **
**                                 **
************************************/

long
rul__mol_get_printform_length (Molecule mol)
{
	long	i, prtlen;
	Compound_Value *cpoly_ptr;

	assert (rul___mol_is_valid (mol));

	if (is_atomic(mol)) {
	    return (rul___mol_printform_length (mol));

	} else if (mol->type == cpoly) {

	    cpoly_ptr = (Compound_Value *) mol;
	    prtlen = 0;
	    for (i=0; i<cpoly_ptr->element_count; i++) {
	        if (i > 0) prtlen += 1;	/*  space delimiter  */
	        prtlen += rul___mol_printform_length (cpoly_ptr->values[i]);
	    }
	    return (prtlen);
	} else {
	    return (0);
	}
}



/***********************************
**                                **
**  RUL__MOL_GET_READFORM_LENGTH  **
**                                **
***********************************/

long
rul__mol_get_readform_length (Molecule mol)
{
	long	i, readlen;
	Compound_Value *cpoly_ptr;

	assert (rul___mol_is_valid (mol));

	if (is_atomic(mol)) {
	    return (rul___mol_readform_length (mol));

	} else if (mol->type == cpoly) {

	    cpoly_ptr = (Compound_Value *) mol;
	    readlen = strlen ("(COMPOUND)");
	    for (i=0; i<cpoly_ptr->element_count; i++) {
	        readlen += 1;	/*  space delimiter  */
	        readlen += rul___mol_readform_length (cpoly_ptr->values[i]);
	    }
	    return (readlen);
	} else {
	    return (0);
	}
}



/*******************************
**                            **
**  MOL_COMPUTE_FORM_LENGTHS  **
**                            **
*******************************/

static void
mol_compute_form_lengths (Molecule mol)
{
  char buffer[RUL_C_MAX_READFORM_SIZE+1];
  Atom mol_atom;
  Symbol_Value	*satom_ptr;
  Integer_Value   *iatom_ptr;
  Double_Value    *datom_ptr;
  Opaque_Value    *oatom_ptr;
  Instance_Id_Value *watom_ptr;
  Compound_Value	*cpoly_ptr;
  long i;

  assert (rul___mol_is_valid (mol));

  if (is_atomic(mol)) {
    mol_atom = (Atom) mol;

    if (mol_atom->readform_length  != MOL__C_NOT_A_VALID_LENGTH) {
      return;
    }

    switch (mol->type) {

    case satom:
      /*  For symbols, printform_len should be set
       **  already, so just set the readform_length
       */
      
      satom_ptr = (Symbol_Value *) mol;
      if (rul___mol_string_has_specials (satom_ptr->sval)) {
	/*
	 **  This string contains odd characters, or 
	 **  the tokenizer thinks this is a number.
	 */
	rul___mol_quote_string (buffer, RUL_C_MAX_READFORM_SIZE+1, 
				satom_ptr->sval);
	satom_ptr->readform_length = strlen(buffer);
	
      } else {
	satom_ptr->readform_length = satom_ptr->printform_length;
      }
      break;
      
    case iatom:
      iatom_ptr = (Integer_Value *) mol;
      sprintf (buffer, "%ld", iatom_ptr->ival);
      iatom_ptr->printform_length = strlen (buffer);
      iatom_ptr->readform_length = iatom_ptr->printform_length;
      break;
      
    case datom:
      datom_ptr = (Double_Value *) mol;
      strcpy (buffer, rul__mol_format_double(datom_ptr->dval));
      datom_ptr->printform_length = strlen (buffer);
      datom_ptr->readform_length = datom_ptr->printform_length;
      break;
      
    case oatom:
      oatom_ptr = (Opaque_Value *) mol;
      sprintf (buffer, "%%x%p", oatom_ptr->oval);
      oatom_ptr->printform_length = strlen (buffer);
      oatom_ptr->readform_length = oatom_ptr->printform_length;
      break;
      
    case watom:
      watom_ptr = (Instance_Id_Value *) mol;
      sprintf (buffer, "#%ld", watom_ptr->wval);
      watom_ptr->printform_length = strlen (buffer);
      watom_ptr->readform_length = watom_ptr->printform_length;
      break;
    }
    
  } else {
    
    /*
     **  For compounds, verify that all the atoms contained in 
     **  it have has their lengths computed.
     */
    cpoly_ptr = (Compound_Value *) mol;
    for (i=0; i<cpoly_ptr->element_count; i++) {
      mol_compute_form_lengths (cpoly_ptr->values[i]);
    }
  }
}



/*********************************
**                              **
**  RUL___MOL_PRINTFORM_LENGTH  **
**                              **
*********************************/

long
rul___mol_printform_length (Molecule mol)
{
	if (mol->type == satom) {
	    /*  symbols have their printform_length's set upon creation  */
	    return (((Atom)mol)->printform_length);
	}
	mol_compute_form_lengths (mol);
	return (((Atom)mol)->printform_length);
}



/********************************
**                             **
**  RUL___MOL_READFORM_LENGTH  **
**                             **
********************************/

long
rul___mol_readform_length (Molecule mol)
{
	mol_compute_form_lengths (mol);
	return (((Atom)mol)->readform_length);
}



/************************
**                     **
**  TOKEN_IS_A_NUMBER  **
**                     **
************************/

static Boolean
token_is_a_number (char *str)
{
	/*
	**  Transition table for recognizing numbers.
	**
	**		State	Description
	**		=====	===========
	**		  0	  Starting, no input yet received
	**		  1	  Received starting sign
	**		  2	  Received starting sign and a period
	**		  3	  Received 'E' starting exponent
	**		  4	  Received 'E' and a sign, starting exponent
	**		  5	* Valid terminal -- Integer
	**		  6	* Valid terminal -- Float wo/ exponent
	**		  7	* Valid terminal -- Float w/ exponent
	**		  8	  Invalid number
	**
	*/
        static const long trans_table[9][5] = 
				       {  { 1, 2, 8, 5, 8 },
                                          { 8, 2, 8, 5, 8 },
                                          { 8, 8, 8, 6, 8 },
                                          { 4, 8, 8, 7, 8 },
                                          { 8, 8, 8, 7, 8 },
                                          { 8, 6, 8, 5, 8 },
                                          { 8, 8, 3, 6, 8 },
                                          { 8, 8, 8, 7, 8 },
                                          { 8, 8, 8, 8, 8 } };
	long curr_state = 0;
	long i = 0;
	long in_class ;

	while (str[i] != '\0') {

	    if (str[i] == '+'  ||  str[i] == '-')	in_class = 0;
	    else if (str[i] == '.') 			in_class = 1;
	    else if (str[i] == 'E') 			in_class = 2;
	    else if (str[i] >= '0'  &&  str[i] <= '9')  in_class = 3;
	    else 					in_class = 4;

	    curr_state = trans_table[curr_state][in_class] ;
	    i++;
	}

	if (curr_state == 5  ||  curr_state == 6  ||  curr_state == 7) {
	    return (TRUE);
	}
	return (FALSE);
}





/************************************
**                                 **
**  RUL___MOL_STRING_HAS_SPECIALS  **
**                                 **
************************************/

Boolean
rul___mol_string_has_specials (char *str)
{
	long	i = 0;
	long	digit_count = 0;
	long	alpha_count = 0;
	long	sign_count = 0;
	long	period_count = 0;
	long	e_count = 0;
	long	length;

	if (str == NULL) return (FALSE);

	if (str[0] == '\0') return (TRUE);

	while (str[i] != '\0') {
	    /*
	    **    Uppercase letters are not special
	    */
	    if (str[i] < 'A'  ||  str[i] > 'Z') {
	    	/*
	    	**    Digits are not special
	    	*/
		if (str[i] < '0'  ||  str[i] > '9') {
		    /*
		    **	  Lower case letters ARE special
		    */
		    if (str[i] >= 'a'  &&  str[i] <= 'z') return (TRUE);
		    /*
		    **    Control characters and ' ' ARE special
	 	    */
		    if (str[i] < '!') return (TRUE);
		    if (str[i] == '\177') return (TRUE);
		    /*
		    **    Reserved characters ARE special
		    */		    
		    if (str[i] == '"'  ||  str[i] == '#'  ||
			str[i] == '%'  ||  str[i] == '&'  ||
			str[i] == '('  ||  str[i] == ')'  ||
			str[i] == '['  ||  str[i] == ']'  ||
			str[i] == '^'  ||  str[i] == '~'  ||
			str[i] == '{'  ||  str[i] == '}'  ||
			str[i] == ';'  ||  str[i] == '|')
			    return (TRUE);
		    /*
		    **    Anything else (e.g. '*', '@', compose characters)
		    **    is not special.
		    */
		    if (str[i] == '+'  ||  str[i] == '-') sign_count++;
		    if (str[i] == '.') period_count++;
		} else {
		    digit_count++;
		}
	    } else {
		alpha_count++;
		if (str[i] == 'E') e_count++; 
	    }
	    i++;
	}
	/*
	**  If we got to this point, then the only potential problem
	**  with this symbol is hat it might look like a number.
	*/
	if (digit_count == 0  ||  period_count > 1  ||  
	    sign_count > 2    ||  e_count > 1       ||
	    alpha_count > e_count)  return (FALSE);

	length = strlen (str);
	/*  simple integer */
	if (digit_count == length) return (TRUE);
	/*  simple float */
	if (period_count == 1  &&  length == (digit_count + 1)) return (TRUE);
	
	return (token_is_a_number (str));
}



/*****************************
**                          **
**  RUL___MOL_QUOTE_STRING  **
**                          **
*****************************/

Boolean
rul___mol_quote_string (char *out_buffer, long max_len, char *str)
{
	long in = 0;
	long out = 0;

	if (out == max_len) return (FALSE);
	out_buffer[out++] = '|';	/*  opening quote  */

	while (out < max_len  &&  str[in] != '\0') {
	    if 	(str[in] == '|') {
		/*  double embedded quotes  */
		out_buffer[out++] = '|';
		if (out == max_len) return (FALSE);
		out_buffer[out++] = '|';
	    } else {
		out_buffer[out++] = str[in];
	    }
	    in++;
	}

	if (out == max_len) return (FALSE);
	out_buffer[out++] = '|';	/*  closing quote  */

	if (out == max_len) return (FALSE);
	out_buffer[out++] = '\0';	/*  string terminator  */

	return (TRUE);
}




/*********************
**                  **
**  RUL__MOL_PRINT  **
**                  **
*********************/

long
rul__mol_print (
#if __VMS
		Molecule *mol)
#else
		Molecule mol)
#endif
	/*
	**  Molecule printer designed to be called from the debuggers
	*/
{
	Molecule in_arg;

#if __VMS
	in_arg = *mol;
#else
	in_arg = mol;
#endif

	rul__ios_printf ( RUL__C_STD_ERR, "\n  Molecule: ");
	rul__mol_print_readform (in_arg, RUL__C_STD_ERR);
	rul__ios_printf ( RUL__C_STD_ERR, "\n");

	return (in_arg->ref_count);
}





/*****************************
**                          **
**  RUL__MOL_FORMAT_DOUBLE  **
**                          **
*****************************/

char *
rul__mol_format_double (double d)
{
	static char buffer[50];
	long i;

	sprintf (buffer, "%.15g", d);
	if (strchr (buffer, '.') == NULL) {

	    /*  Can't let C turn this into an integer...  */
	    strcat (buffer, ".0");

	} else {

	    if (strchr (buffer, 'e') == NULL) {
		/*
		**  Has a decimal and no exponent,
		**  so strip trailing zeros
		*/
		i = strlen (buffer) - 1;
		while (buffer[i-1] != '.'  &&  buffer[i] == '0') {
		    buffer[i] = '\0';
		    i--;
		}
	    }
	}
	return (buffer);
}
