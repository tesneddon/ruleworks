/****************************************************************************
**                                                                         **
**                           V A L _ P . H                                 **
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
 * FACILITY:
 *	RULEWORKS compiler system
 *
 * ABSTRACT:
 *	This module defines the internals for the
 *	compiler unified value structures.
 *
 * MODIFIED BY:
 *	DEC	Digital Equipment Corporation
**	CPQ	Compaq Computer Corporation
 *
 * MODIFICATION HISTORY:
 *
 *	 12-Jan-1993	DEC	Initial version
 *	01-Dec-1999	CPQ	Release with GPL
 */




#define VAL__C_VALUE_VERIFY	3312
#define BOGUS_VALUE_TYPE        0	/* for dead end asserts */
#define VAL__C_NOT_AN_INDEX	-1


#ifdef	NDEBUG		/* for use in Value structures when debugging */
#define VAL_DEBUG_COMMON
#else
#define VAL_DEBUG_COMMON	\
		long			 verification;
#endif



/*
*
*	LHS Value's have been organized into the object class
*	hierarchy as shown below.  The typedefs in this file fake
*	that hierarchy given the limited facilities available in C.
*
*
*				              Value
*			                    ___/ \___
*			                ___/         \___  (LHS only)
*			               /                 \
*	                      Nonposition_Value         Position_Value
*	                  _____/   /  |  \   \_____             \
*	            _____/      __/   |   \__      \_____   Position_Elem_Value
*	     ______/        ___/      |      \___        \______      
*	    /              /          |          \              \      
*  Constant_Value  Arith_Value  Variable_Value   Complex_Value  Assignment_Value
*      /      \                      / \             /      \ 
*  Rul_Value  Animal_Value          /   \  Function_Value  Disjunction_Value
*               ___________________/     \___________________________
*              /             /                 \                     \
*    LHS_Var_Value  RHS_Var_Value  Unnamed_Mol_Var_Value  Unnamed_Ext_Var_Value
*/


#define BASE	      023000

/*	First level types 	*/
#define VAL_NONPOS	0200
#define VAL_POS		0300

/*	Second level types	*/
#define VAL_POS_ELEM	0010

#define VAL_CONST	0010
#define VAL_VAR		0020
#define VAL_ARITH_EXPR	0030
#define VAL_COMPLEX	0040
#define VAL_ASSIGNMENT	0050
#define VAL_COND_EXPR	0060

/*	Third level types	*/
#define VAL_FUNCTION	0001
#define VAL_DISJ	0002

#define VAL_RUL			0001
#define VAL_ANIMAL		0002
#define VAL_CLASS		0003
#define VAL_CONSTRUCT	  	0004
#define VAL_CONSTRUCT_TYPE	0005

#define VAL_VAR_LHS	0001
#define VAL_VAR_RHS	0002
#define VAL_VAR_UNM_MOL	0003
#define VAL_VAR_UNM_EXT	0004
#define VAL_VAR_BLK_LOC	0005
#define VAL_VAR_NAM_EXT 0006

/*	Level Masks		*/
#define MASK1 		0700
#define MASK2 		0070
#define MASK3 		0007
#define MASK2_3 	0077
#define MASKALL		0777




typedef enum {
  /*
   **  Includes only the set of possible direct instance types
   **  (for example, does not include a val__e_non_pos)
   */
  VAL__E_INVALID        = 0,
  
  VAL__E_POS	        = (BASE | VAL_POS),
  VAL__E_POS_ELEM       = (BASE | VAL_POS    | VAL_POS_ELEM),
  
  VAL__E_LHS_VARIABLE   = (BASE | VAL_NONPOS | VAL_VAR | VAL_VAR_LHS),
  VAL__E_RHS_VARIABLE   = (BASE | VAL_NONPOS | VAL_VAR | VAL_VAR_RHS),
  VAL__E_UNM_MOL_VAR    = (BASE | VAL_NONPOS | VAL_VAR | VAL_VAR_UNM_MOL),
  VAL__E_UNM_EXT_VAR    = (BASE | VAL_NONPOS | VAL_VAR | VAL_VAR_UNM_EXT),
  VAL__E_BLK_OR_LOCAL   = (BASE | VAL_NONPOS | VAL_VAR | VAL_VAR_BLK_LOC),
  VAL__E_NAM_EXT_VAR	= (BASE | VAL_NONPOS | VAL_VAR | VAL_VAR_NAM_EXT),
  
  VAL__E_ARITH_EXPR     = (BASE | VAL_NONPOS | VAL_ARITH_EXPR),
  VAL__E_ASSIGNMENT     = (BASE | VAL_NONPOS | VAL_ASSIGNMENT),
  VAL__E_COND_EXPR      = (BASE | VAL_NONPOS | VAL_COND_EXPR),
  
  VAL__E_RUL_CONST      = (BASE | VAL_NONPOS | VAL_CONST | VAL_RUL),
  VAL__E_ANIMAL_CONST   = (BASE | VAL_NONPOS | VAL_CONST | VAL_ANIMAL),
  VAL__E_CLASS_CONST    = (BASE | VAL_NONPOS | VAL_CONST | VAL_CLASS),
  VAL__E_CONSTRUCT      = (BASE | VAL_NONPOS | VAL_CONST | VAL_CONSTRUCT),
  VAL__E_CONSTRUCT_TYPE = (BASE | VAL_NONPOS | VAL_CONST | VAL_CONSTRUCT_TYPE),
  
  VAL__E_COMPLEX	= (BASE | VAL_NONPOS | VAL_COMPLEX),
  VAL__E_FUNCTION	= (BASE | VAL_NONPOS | VAL_COMPLEX | VAL_FUNCTION),
  VAL__E_DISJUNCTION    = (BASE | VAL_NONPOS | VAL_COMPLEX | VAL_DISJ)
    
} Value_Type ;




#define is_value(x)		(((int)x->type & ~MASKALL) == BASE)

#define is_position_val(x)	(((int)x->type & ~MASK2_3) == VAL__E_POS)
#define is_position_elem_val(x)	(x->type == VAL__E_POS_ELEM)

#define is_non_position_val(x)	 	\
		(((int)x->type & ~MASK2_3) == (BASE | VAL_NONPOS))
#define is_assignment_val(x)	(x->type == VAL__E_ASSIGNMENT)

#define is_constant_val(x)	 	\
		(((int)x->type & ~MASK3) == (BASE | VAL_NONPOS | VAL_CONST))
#define is_rul_constant_val(x)	(x->type == VAL__E_RUL_CONST)
#define is_animal_constant_val(x) 	\
				(x->type == VAL__E_ANIMAL_CONST)
#define is_class_constant_val(x) 	\
				(x->type == VAL__E_CLASS_CONST)
#define is_construct_val(x)	(x->type == VAL__E_CONSTRUCT)
#define is_construct_type_val(x) 	\
				(x->type == VAL__E_CONSTRUCT_TYPE)

#define is_variable_val(x)		\
	(((int)x->type & ~MASK3) == (BASE | VAL_NONPOS | VAL_VAR))
#define is_lhs_var_val(x)	(x->type == VAL__E_LHS_VARIABLE)
#define is_rhs_var_val(x)	(x->type == VAL__E_RHS_VARIABLE)
#define is_unnamed_mol_var_val(x)	\
				(x->type == VAL__E_UNM_MOL_VAR)
#define is_unnamed_ext_var_val(x)	\
				(x->type == VAL__E_UNM_EXT_VAR)
#define is_named_ext_var_val(x)		\
				(x->type == VAL__E_NAM_EXT_VAR)
#define is_blk_or_local_val(x)	(x->type == VAL__E_BLK_OR_LOCAL)

#define is_arith_expr_val(x)	(x->type == VAL__E_ARITH_EXPR)

#define is_cond_expr_val(x)	(x->type == VAL__E_COND_EXPR)

#define is_complex_val(x)	 	\
		(((int)x->type & ~MASK3) == (BASE | VAL_NONPOS | VAL_COMPLEX))
#define is_function_val(x)	(x->type == VAL__E_FUNCTION)
#define is_disjunction_val(x)	(x->type == VAL__E_DISJUNCTION)




#define VAL_COMMON		\
		Value_Type		 type; 			\
		VAL_DEBUG_COMMON				\
		Decl_Domain		 domain;		\
		Decl_Shape		 shape;			\
		Class			 rclass;

struct value {
	VAL_COMMON
};



#define VAL_POS_COMMON	\
		long			 ce_index;		\
		Mol_Symbol		 attr_name;		\
		long			 attr_offset;

typedef struct position_value {
	VAL_COMMON
	VAL_POS_COMMON
} *Position_Value;



typedef struct position_elem_value {
	VAL_COMMON
	VAL_POS_COMMON
	Value				 index;
} *Position_Elem_Value;



typedef struct rul_constant_value {
	VAL_COMMON
	Molecule		  	 constant_value;
} *Rul_Constant_Value;



typedef struct animal_constant_value {
	VAL_COMMON
	Ext_Type	  	 ext_type;
	union {
		long		 sign_int_val;
		unsigned long	 unsign_int_val;
		double		 dbl_val;
		char		*asciz_val;
		/* String_Desc	*ascid_val; * future opimization opportunity */
		void		*opaque_val;
	} val;
} *Animal_Constant_Value;


typedef struct class_constant_value {
	VAL_COMMON
	Class	  		 class_value;
} *Class_Constant_Value;

typedef struct construct_value {
	VAL_COMMON
	Molecule	  	 construct_value;
} *Construct_Value;


typedef struct construct_type_value {
	VAL_COMMON
	Molecule	  	 construct_value;
	Construct_Type	  	 construct_type;
} *Construct_Type_Value;



#define VAL_VARIABLE_COMMON	\
		Molecule		 variable_name;

typedef struct variable_value {
	VAL_COMMON
	VAL_VARIABLE_COMMON
} *Variable_Value;





typedef struct lhs_variable_value {
	VAL_COMMON
	VAL_VARIABLE_COMMON
	long				 lhs_var_id_number;
} *LHS_Variable_Value;



typedef struct blk_or_local_value {
	VAL_COMMON
	VAL_VARIABLE_COMMON
	Rul_Internal_Type		arg_type;
	Mol_Symbol			arg_name;
	Boolean				arg_indirect;
	long				arg_index;
	Rul_Internal_Field		arg_field;
} *Blk_Or_Local_Value;



typedef struct rhs_variable_value {
	VAL_COMMON
	VAL_VARIABLE_COMMON
	long				 cache_index;
} *RHS_Variable_Value;



typedef struct unnamed_mol_variable_value {
	VAL_COMMON
	VAL_VARIABLE_COMMON
	long				 cache_index;
} *Unnamed_Mol_Variable_Value;



typedef struct unnamed_ext_variable_value {
	VAL_COMMON
	VAL_VARIABLE_COMMON
	long				 cache_index;
	Ext_Type			 cache_type;
	long				 array_length;
	long				 array_index;
	Ext_Mech			 passing_mech;
	Boolean				 memory_allocated;
} *Unnamed_Ext_Variable_Value;



typedef struct named_ext_variable_value {
	VAL_COMMON
	VAL_VARIABLE_COMMON
	Ext_Type			 cache_type;
	long				 array_length;
	long				 array_index;
	Ext_Mech			 passing_mech;
	Boolean				 memory_allocated;
} *Named_Ext_Variable_Value;



typedef struct arith_expr_value {
	VAL_COMMON
	Arith_Oper_Type		 arith_operator;
	Value			 operand_1;
	Value			 operand_2;
} *Arith_Expr_Value;


typedef struct cond_expr_value {
	VAL_COMMON
	Cond_Oper_Type		 cond_operator;
	Value			 operand_1;
	Value			 operand_2;
} *Cond_Expr_Value;


#define VAL_COMPLEX_COMMON	\
		Molecule		 function_name;		\
		Value			 next_value;		\
		Value			 prev_value;		\
                Ext_Mech		 passing_mech;          \
		long			 subvalue_count;	\
		Value		  	 subvalues[1];

typedef struct complex_value {
	VAL_COMMON
	VAL_COMPLEX_COMMON
} *Complex_Value;

typedef struct complex_value *Function_Value;
typedef struct complex_value *Disjunction_Value;



typedef struct assignment_value {
	VAL_COMMON
	Value				 to_variable;
	Value				 from_value;
	Value				 next_value;
	Value				 prev_value;
} *Assignment_Value;
