/****************************************************************************
**                                                                         **
**                  R T S _ I 1 8 N _ S T R I N G . C           

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
**	This file contains the string functions that may need to be changed
**	when this product is localized.  These functions include:
**
**	    rul__i18n_string_soundex  -  
**			Uses the soundex algorithm to group similar 
**			sounding English words together.  It is used
**			for the approximatly-equals predicate over symbols.
**
**	    rul__i18n_string_compare -
**			Compares two strings for which one should come
**			first in a collating sequence.  It is used by the
**			greater-than and less-than predicates over symbols.
**
**
**  MODIFIED BY:
**	DEC	Digital Equipment Corporation
 *	CPQ	Compaq Computer Corporation
**
**  MODIFICATION HISTORY:
**
**	10-Sep-1992	DEC	Initial version
*	01-Dec-1999	CPQ	Release with GPL
*/

#include <ctype.h>
#include <common.h>
#include <i18n.h>

#ifdef __VMS
#include <descrip.h>
#include <str$routines.h>	/* For str$compare_multi */
#endif


/*  Local function prototypes  */
static Boolean is_multinational_alpha (char ch);
static char multinational_to_upper (char ch);
static char get_consonant_group (char ch);




/*******************************
**                            **
**  RUL__I18N_STRING_SOUNDEX  **
**                            **
*******************************/

char *rul__i18n_string_soundex (char *str)
{
/*
**    These SOUNDEX values are computed according to an algorithm similar to
**    that described in The Art Of Computer Programming, V. 3, pp. 391-392,
**    by Donald Knuth.
**
**	1.  Find the first alphabetic character in the symbol, uppercase it,
**	    and save it as the first character of the output code.
**	2.  Replace each non-alphabetic character with a code of 0.
**	3.  Ignoring case, replace each consonant in the symbol after the
**	    first alphabetic character (except H's and W's) with the 
**	    corresponding consonant group code:
**		1: {B, F, P, V}		2: {C, G, J, K, Q, S, X, Z}
**		3: {D, T}		4: {L}
**		5: {M, N}		6: {R}
**	4.  If two or more adjacent characters contain the same numeric code,
**	    then remove all but the first occurance.
**	5.  Ignoring case and the first alphabetic character, remove all vowels, 
**	    all Y's, all H's, all W's, and all code 0's.
**
**	The remaining string is our soundex code.
**
**	For example:
**		Tracy           ==>  T62
**		St. Laurent     ==>  S34653
**	        Steven          ==>  S315
**	     	Stephen         ==>  S315
**	     	Stefano         ==>  S315
*/
	char in_buff[RUL_C_MAX_SYMBOL_SIZE+1];
	char out_buff[RUL_C_MAX_SYMBOL_SIZE+1];
	long i, start, out_index;
	char ch, *ret_str;

	assert (str  &&  (str[0] != '\0'));

	/*  Find the first alphabetic character  */
	i = 0;
	while (str[i] != '\0'  &&  ! isalpha(str[i])
			       &&  ! is_multinational_alpha(str[i])) i++;

	if (str[i] == '\0') {
	    /* no alphabetic characters were found; return the empty string  */
	    ret_str = (char *) rul__mem_malloc (1);
	    ret_str[0] = '\0';
	    return (ret_str);
	}

	/*  Save the first alphabetic character  */
	out_index = 1;
	out_buff[0] = multinational_to_upper(str[i]);
	start = i + 1;

	/*  Perform approximate phoneme group substitution  */
	i = start;
	while ((ch = str[i]) != '\0') {
	    in_buff[i] = get_consonant_group (ch);
	    i++;
	}
	in_buff[i] = '\0';

	/*  Remove redundent sounds, vowel sounds,
	**  non-alphabetics, and wimpy consonants
	*/
	i = start;
	while ((ch = in_buff[i]) != '\0') {
	    if (ch != in_buff[i+1]) {
	        if (ch > '0'  &&  ch <= '9') {
		    out_buff[out_index] = ch;
		    out_index++;
		}
	    }
	    i++;
	}
	out_buff[out_index] = '\0';

	/*  Return the resulting string  */
	ret_str = rul__mem_malloc (strlen(out_buff) + 1);
	strcpy (ret_str, out_buff);

	return (ret_str);
}





/*****************************
**                          **
**  IS_MULTINATIONAL_ALPHA  **
**                          **
*****************************/

static Boolean is_multinational_alpha (char ch)
{
	static const char multis[] = 
		"àÀáÁâÂãÃäÄåÅæÆçÇèÈéÉêÊìÌíÍîÎïÏñÑòÒóÓôÔöÖ÷×øØùÙúÚûÛüÜýÝß";

	if (strchr (multis, ch) == NULL) return FALSE;
	return TRUE;
}



/*****************************
**                          **
**  MULTINATIONAL_TO_UPPER  **
**                          **
*****************************/

static char multinational_to_upper (char ch)
{
	static const char multis[] = 
		"àÀáÁâÂãÃäÄåÅæÆçÇèÈéÉêÊìÌíÍîÎïÏñÑòÒóÓôÔöÖ÷×øØùÙúÚûÛüÜýÝß";
	static const char upper_multis[] =
		"AAAAAAAAAAAAAASSEEEEEEIIIIIIIINNOOOOOOOOOOOOUUUUUUUUYYS0";
	long i;

	if (strchr (multis, ch) == NULL) {
	    return toupper(ch);
	}
	i = 0;
	while (multis[i] != ch  &&  multis[i] != '\0') i++;
	return (upper_multis[i]);
}




/**************************
**                       **
**  GET_CONSONANT_GROUP  **
**                       **
**************************/

static char  get_consonant_group (char ch)
{
	/*
	**  This should really be done using an internationalization
	**  toolkit of some kind.  Until then, this will do tolerably well.
	**  Note that it maps multi-national characters to their likeliest
	**  English counterpart.
	*/
	static const char source_str[] =
"abcdefghijklmnopqrstuvwxyzàÀáÁâÂãÃäÄåÅæÆçÇèÈéÉêÊìÌíÍîÎïÏñÑòÒóÓôÔöÖ÷×øØùÙúÚûÛüÜýÝß";
	static const char subst_str[] = 
"a123e12hi22455o12623u1w2y2aaaaaaaaaaaaaa22eeeeeeiiiiiiii55oooooooooooouuuuuuuuyy2";

	long i;

	for (i=0; i<sizeof(source_str)-1; i++) {
	    if (source_str[i] == ch  ||  source_str[i] == tolower(ch)) {
		return (subst_str[i]);
	    }
	}
	/*  If it wasn't an alphabetic character, return a code 0.  */
	return ('0');
}




/*******************************
**                            **
**  RUL__I18N_STRING_COMPARE  **
**                            **
*******************************/

int rul__i18n_string_compare (char *str1, char *str2)
{
#if (defined(__VMS) || defined(VAXC))
	struct dsc$descriptor string1, string2 ;

	string1.dsc$w_length =	strlen(str1) ;
	string1.dsc$b_dtype =	DSC$K_DTYPE_T ;
	string1.dsc$b_class =	DSC$K_CLASS_S ;
	string1.dsc$a_pointer =	str1 ;

	string2.dsc$w_length =	strlen(str2) ;
	string2.dsc$b_dtype =	DSC$K_DTYPE_T ;
	string2.dsc$b_class =	DSC$K_CLASS_S ;
	string2.dsc$a_pointer =	str2 ;

	return (str$compare_multi (&string1, &string2, 1, 1)) ; 
#else
	long  i, l;
	char *tmp1, *tmp2;

	l = strlen (str1);
	tmp1 = rul__mem_malloc (l + 1);
	strcpy (tmp1, str1);
	for (i=0; i<l; i++) tmp1[i] = toupper (tmp1[i]);
	l = strlen (str1);
	tmp2 = rul__mem_malloc (l + 1);
	strcpy (tmp2, str2);
	for (i=0; i<l; i++) tmp2[i] = toupper (tmp2[i]);
	i = (strcoll (tmp1, tmp2));
	rul__mem_free (tmp1);
	rul__mem_free (tmp2);
	return i;
#endif
}
