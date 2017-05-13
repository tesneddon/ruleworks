/****************************************************************************
**                                                                         **
**                             I 1 8 N . H                                 **
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
**	This file contains the exported declarations for all the 
**	functions that may need to be changed when this product
**	is localized.
**
**	This file also contains all the non-message string constants
**	such as the prompt string. 
*/



/*
**	Constants:
*/

#define RUL__C_MATCHES_STRING "matches for"




/*
**	Functions:
*/

char *rul__i18n_string_soundex (char *str) ;
	/*
	**	Uses the soundex algorithm to group similar 
	**	sounding English words together.  It is used
	**	for the approximatly-equals predicate over symbols.
	*/


int rul__i18n_string_compare (char *str1, char *str2) ;
	/*
	**	Compares two strings for which one should come
	**	first in a collating sequence.  It is used by the
	**	greater-than and less-than predicates over symbols.
	*/
