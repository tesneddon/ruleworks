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
 *  FACILITY:
 *	RULEWORKS run time system
 *
 *  ABSTRACT:
 *	Indexfile sub-subsystem declarations
 *
 *  MODIFIED BY:
 *      DEC	Digital Equipment Corporation
 *	CPQ	Compaq Computer Corporation
 *
 *  MODIFICATION HISTORY:
 *
 *	29-Mar-1993	DEC	Initial Version
 *	01-Dec-1999	CPQ	Release with GPL
 */


/* the following isn't exactly correct, but is probably big enough... */
#define LONGEST_LINE 16*RUL_C_MAX_READFORM_SIZE

/*  I/O macros  */

#define RUL_C_MAX_PATTERN	(sizeof("999"))

#define USE_VERSION             "(USE-VERSION"
#define VERSION_NUMBER		"V5.0-0"
#define DECLARATION_STR		"(DECLARATION-BLOCK"
#define PATT_SIZ		"(PATTERN-SIZE"
#define COMPILATION_ID		"(COMPILATION-ID"

#define OBJECT_CLASS		"(OBJECT-CLASS"
#define INHERIT_FROM		"(INHERITS-FROM"
#define IMPORT_DEFS		"(IMPORT-DEFS"
#define SHAPE_COMPOUND		"COMPOUND "
#define SHAPE_TABLE		"TABLE    "
#define SHAPE_SCALAR		"SCALAR   "
#define DOMAIN_ANY		"ANY      "
#define DOMAIN_SYMBOL		"SYMBOL   "
#define DOMAIN_INSTANCE		"INSTANCE "
#define DOMAIN_OPAQUE		"OPAQUE   "
#define DOMAIN_NUMBER		"NUMBER   "
#define DOMAIN_INTEGER		"INTEGER  "
#define DOMAIN_FLOAT		"FLOAT    "
#define INSTANCE_OF		"(OF "
#define OPEN_COMPOUND		"(COMPOUND"
#define OPEN_TABLE		"(TABLE"
#define DEFAULT			"(DEFAULT "
#define FILL			"(FILL "


#define GENERIC_METHOD		"(GENERIC-METHOD"
#define ROOT                    "$ROOT"

#define EXTERNAL_ROUTINE	"(EXTERNAL-ROUTINE"
#define ALIAS			"(ALIAS-FOR"
#define ACCEPTS			"(ACCEPTS"
#define RETURNS			"(RETURNS"

#define END_BLOCK		"(END-BLOCK"
#define END_USE			"(END-USE"
#define USE_SPACE		" "
#define USE_TAB_1		"  "
#define USE_TAB_2		"    "
#define USE_ATTR_TAB		"      "
#define EXT_RT_TAB		"             "
#define USE_NEWLINE		"\n"
#define CLAUSE_BEGIN		"("
#define CLAUSE_END		")"
#define CONSTRUCT_END		")\n"


