/****************************************************************************
**                                                                         **
**                        C M P _ C O M M . H       


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
**                                                                         **
****************************************************************************/

/*
 * FACILITY:
 *	RuleWorks compiler
 *
 * ABSTRACT:
 *	This module provides the set of common type 
 *	declarations used exclusively by the compiler.
 *	
 *	Note:	It does not replace or include common.h,
 *		it merely augments the information there.
 *
 * MODIFIED BY:
 *	DEC	Digital Equipment Corporation
 *	CPQ	Compaq Computer Corporation
 *
 * MODIFICATION HISTORY:
 *
 *	6-Dec-1992	DEC	Initial version
 *	01-Dec-1999	CPQ	Releasew ith GPL
 */


/*  Define the unique reserved function name prefix  */
#define GEN_NAME_PREFIX	"rul_"


typedef struct net_node *Net_Node;
typedef struct net_test *Net_Test;
typedef struct relation	*Relation;
typedef struct value	*Value;

typedef enum {
	CMP__E_INT_OBJECT = 673,
	CMP__E_INT_VOID_PTR,
	CMP__E_INT_LONG,
	CMP__E_INT_FUNC_PTR,
	CMP__E_INT_CLASS,
	CMP__E_INT_CS,
	CMP__E_INT_CS_ENTRY,
	CMP__E_INT_REFRACTION_SET,
	CMP__E_INT_DELTA_TOK,
	CMP__E_INT_DELTA_STRUCT,
	CMP__E_INT_DEL_TOK_SIGN,
	CMP__E_INT_MOLECULE,
	CMP__E_INT_CONSTRUCT_TYPE,
	CMP__E_INT_BETA_SET,
	CMP__E_INT_BETA_TOK,
	CMP__E_INT_CATCHER_FUNC, /* Catcher_Function */
	CMP__E_INT_MATCHES_FUNC, /* Matches_Function */
	CMP__E_INT_PROP_FUNC,	 /* Propagate_Function */
	CMP__E_INT_ENTRY_DATA	 /* Entry block stack data */
} Rul_Internal_Type;


typedef enum {
	CMP__E_FIELD__NONE = 0,
	/*
	**  Delta_Token fields
	*/ 
	CMP__E_FIELD_BETA_INST_COUNT = 1091,
	CMP__E_FIELD_BETA_INST_VEC,
	CMP__E_FIELD_BETA_HSH_VEC,
	CMP__E_FIELD_BETA_AUX_VEC,
	/*
	**  Beta_Token fields
	*/
	CMP__E_FIELD_DELTA_SIGN,
	CMP__E_FIELD_DELTA_OBJ,
	CMP__E_FIELD_DELTA_INST_ID,
	CMP__E_FIELD_DELTA_CLASS
} Rul_Internal_Field;


typedef enum {
	CMP__E_ARITH_PLUS = 603,
	CMP__E_ARITH_MINUS,
	CMP__E_ARITH_TIMES,
	CMP__E_ARITH_DIVIDE,
	CMP__E_ARITH_MODULUS,
	CMP__E_ARITH_UNARY_MINUS
} Arith_Oper_Type;


typedef enum {
        PRED__E_NOT_A_PRED = 0,
	PRED__E_EQ,
	PRED__E_EQUAL,
	PRED__E_APPROX_EQUAL,
	PRED__E_NOT_APPROX_EQUAL,
	PRED__E_SAME_TYPE,
	PRED__E_DIFF_TYPE,
	PRED__E_NOT_EQ,
	PRED__E_NOT_EQUAL,
	PRED__E_LESS,
	PRED__E_LESS_EQUAL,
	PRED__E_GREATER,
	PRED__E_GREATER_EQUAL,
	PRED__E_CONTAINS,
	PRED__E_DOES_NOT_CONTAIN,
	PRED__E_LENGTH_LESS_EQUAL,
	PRED__E_LENGTH_NOT_EQUAL,
	PRED__E_LENGTH_LESS,
	PRED__E_LENGTH_EQUAL,
	PRED__E_LENGTH_GREATER_EQUAL,
	PRED__E_LENGTH_GREATER,
	PRED__E_COMP_INDEX_VALID
} Pred_Type;

typedef enum {
	CMP__E_COND_AND = 205,
	CMP__E_COND_OR,
	CMP__E_COND_NOT
} Cond_Oper_Type;

char *rul__parser_get_construct_str (void);

