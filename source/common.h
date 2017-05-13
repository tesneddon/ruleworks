/****************************************************************************
**                                                                         **
**                          C O M M O N . H      

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
**	RuleWorks run time system and RuleWorks compiler
**
**  ABSTRACT:
**	This header file contains the global constants and declarations.
**
**
**  MODIFIED BY:
**	DEC	Digital Equipment Corporation
 *	CPQ	Compaq Computer Corporation
**
**  MODIFICATION HISTORY:
**
**	21-Aug-1991	DEC	Initial version
**	 4-Dec-1992	DEC	add construct types
**	 9-Apr-1994	DEC	Add WINDOWS
**	01-Dec-1999	CPQ	Releasew ith GPL
*/

/*
 * Files
 */

#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <assert.h>
/*
 *  INCLUDE-IN-GEND.H
 */
#include <stdlib.h>		/* For exit(), EXIT_SUCCESS, EXIT_FAILURE */

/*
**  System identification macros:
**
**	Since there is no global conventions, our conventions are
**	that machine or OS identifiers are upper case, and begin
**	with two underscores.  Unfortunately this is an error, since
**	identifiers starting with underscore are reserved.
**
**	We use:
**	    __UNIX
**	    __VMS
**	    __MSDOS
*/

#if !defined(__UNIX) &&  (defined(unix) || defined(__unix) || defined(UNIX))
#define __UNIX 1
#endif
#if !defined(__VMS) &&  (defined(vms) || defined(__vms) || defined(VMS))
#define __VMS 1
#endif
#if !defined(__MSDOS) && (defined(MSDOS) || defined(__MSDOS__))
#define __MSDOS 1
#endif

#if !defined(WINDOWS)
#if defined(__WINDOWS__) || defined(_Windows) || defined(_WINDOWS)
/*	    Watcom		    Borland		 Microsoft */
#define WINDOWS 1		/* We are building for Microsoft Windows */
#endif
#endif

/*
 * Avoid clashes between e.g. user's ENTRY-BLOCK named List and our typedef
 * named List.
 */
#define List                RulList
#define Hash_Table          RulHash_Table
#define Dynamic_Array       RulDynamic_Array
#define IO_Stream           RulIO_Stream
#define Molecule            RulMolecule
#define Molecule_Ptr        RulMolecule_Ptr
#define Object              RulObject

#define Entry_Block         RulEntry_Block
#define Entry_Data          RulEntry_Data
#define Decl_Block          RulDecl_Block
#define Class               RulClass
#define Ext_Rt_Decl         RulExt_Rt_Decl
#define Ext_Alias_Decl      RulExt_Alias_Decl
#define Method              RulMethod
#define Method_Func         RulMethod_Func
#define Molecule_Type       RulMolecule_Type

/*
**  RTS typedefs
*/
#define Delta_Token         RulDelta_Token
#define Delta_Queue         RulDelta_Queue
#define Beta_Token          RulBeta_Token
#define Beta_Collection     RulBeta_Collection
#define Conflict_Set_Entry  RulConflict_Set_Entry
#define Conflict_Subset     RulConflict_Subset
#define Refraction_Set      RulRefraction_Set
#define Cardinality         RulCardinality
#define Token_Sign          RulToken_Sign
#define Construct_Type      RulConstruct_Type
#define Catcher_Function    RulCatcher_Function
#define Matches_Function    RulMatches_Function
#define Propagate_Function  RulPropagate_Function
#define Class_Member        RulClass_Member

#ifdef __VMS
#define String              RulString
#else
#define String              RulString
#endif

#define Pointer             RulPointer

#define Mol_Atom            RulMol_Atom
#define Mol_Symbol          RulMol_Symbol
#define Mol_Instance_Id     RulMol_Instance_Id
#define Mol_Opaque          RulMol_Opaque
#define Mol_Number          RulMol_Number
#define Mol_Int_Atom        RulMol_Int_Atom
#define Mol_Dbl_Atom        RulMol_Dbl_Atom
#define Mol_Polyatom        RulMol_Polyatom
#define Mol_Compound        RulMol_Compound
#define Mol_Table           RulMol_Table
#define Decl_Domain         RulDecl_Domain
#define Decl_Shape          RulDecl_Shape
#define Strategy            RulStrategy
#define Boolean             RulBoolean


/*
 * Exit Status Definitions
 */
#ifdef __VMS
#include <ssdef.h>
#ifdef VAXC
#undef EXIT_FAILURE
#define EXIT_FAILURE SS$_ABORT
#endif
#endif

#ifndef NULL
#define NULL		((void *) 0)
#endif

/*
 * Type definitions
 */
#ifdef FALSE
#undef FALSE
#endif

#ifdef TRUE
#undef TRUE
#endif

#ifdef VAXC
#define FALSE	(1 == 0)
#define TRUE	(0 == 0)

typedef long RulBoolean;
#else
typedef enum Rulboolean {
  FALSE=(1==0),
  TRUE=(0==0)
  } RulBoolean;
#endif

/*
 * storage class macros
 */

#ifdef VAXC_SHARABLE
#define GLOBAL   globaldef
#define EXTERNAL globalref
#else
#define GLOBAL
#define EXTERNAL extern
#endif

#ifndef MIN
#define MIN(a,b) (((a) <= (b)) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a,b) (((a) >  (b)) ? (a) : (b))
#endif

/*
 * Constants
 *
 * The RUL_C_ constants should really be included from RUL_rtl.h
 */

#ifndef OPS_C_MAX_SYMBOL_SIZE
#define OPS_C_MAX_SYMBOL_SIZE RUL_C_MAX_SYMBOL_SIZE
#endif
#define RUL_C_MAX_SYMBOL_SIZE	256	/* Max characters in symbolic atom */


/*
**	Abstract Data Type Declarations
*/

typedef struct list			*RulList;
typedef struct hash_table 		*RulHash_Table;
typedef struct dynamic_array 		*RulDynamic_Array;

typedef struct io_stream		*RulIO_Stream;
typedef struct molecule			*RulMolecule;
typedef RulMolecule			*RulMolecule_Ptr;
typedef struct object			*RulObject;

typedef struct entry_block		*RulEntry_Block;
typedef struct entry_data		*RulEntry_Data;
typedef struct decl_block		*RulDecl_Block;
typedef struct rclass			*RulClass;
typedef struct ext_rt_decl		*RulExt_Rt_Decl;
typedef struct ext_alias_decl		*RulExt_Alias_Decl;
typedef struct method                   *RulMethod;

/*
**  RTS typedefs
*/
typedef struct delta_token		*RulDelta_Token;
typedef struct delta_queue		*RulDelta_Queue;
typedef struct beta_token		*RulBeta_Token;
typedef struct beta_collection		*RulBeta_Collection;
typedef struct conflict_set_entry	*RulConflict_Set_Entry;
typedef struct conflict_subset		*RulConflict_Subset;
typedef struct refraction_set		*RulRefraction_Set;
typedef long                             RulCardinality;
typedef long                             RulToken_Sign;
typedef struct class_member             *RulClass_Member;


#ifdef __VMS
typedef struct dsc$descriptor RulString;
#else
typedef struct dsc_descriptor RulString;
#endif

typedef void *RulPointer;

/*
 * typedef synonyms for Molecule.  These allow more explicit parameter
 * declarations for functions which operate on specific types of Molecules.
 */
typedef RulMolecule
    RulMol_Atom,
    RulMol_Symbol,
    RulMol_Instance_Id,
    RulMol_Opaque,
    RulMol_Number,
    RulMol_Int_Atom,
    RulMol_Dbl_Atom,
    RulMol_Polyatom,
    RulMol_Compound,
    RulMol_Table;


/*
 * definitions are set to be compatible with the previous version of OPS5
 * Note: from ops_parser.h
 *       OPS$K_UNKNOWN_CONST	 0
 *       OPS$K_STARTUP_CONST	 1
 *       OPS$K_RULE_CONST	 2
 *       OPS$K_CATCH_CONST	 3
 *       OPS$K_EXTERNAL_CONST	 4
 *       OPS$K_LITERAL_CONST	 5
 *       OPS$K_LITERALIZE_CONST	 6
 *       OPS$K_VECTATTR_CONST	 7
 *       OPS$K_COMMENT_CONST	 8
 *       OPS$K_WME_CLASS_CONST	 9
 *       OPS$K_EXTROUTINE_CONST	10
 *       OPS$K_DECLBLOCK_CONST	11
 *       OPS$K_ENTRYBLOCK_CONST	12
 *       OPS$K_RULEBLOCK_CONST	13
 *       OPS$K_ENDBLOCK_CONST	14
 *       OPS$K_RULEGROUP_CONST	15
 *       OPS$K_ENDGROUP_CONST	16
 */

typedef enum {
  RUL__C_NOISE		= 0,
  RUL__C_RULE		= 2,
  RUL__C_CATCH		= 3,
  RUL__C_COMMENT	= 8, /* same from here... */
  RUL__C_OBJ_CLASS	= 9,
  RUL__C_EXT_ROUTINE	= 10,
  RUL__C_DECL_BLOCK	= 11,
  RUL__C_ENTRY_BLOCK	= 12,
  RUL__C_RULE_BLOCK	= 13,
  RUL__C_END_BLOCK	= 14,
  RUL__C_RULE_GROUP	= 15,
  RUL__C_END_GROUP	= 16,
  RUL__C_ON_ENTRY	= 17,
  RUL__C_ON_EVERY	= 18,
  RUL__C_ON_EMPTY	= 19,
  RUL__C_ON_EXIT	= 20,
  RUL__C_METHOD		= 21
  } RulConstruct_Type;


/*
 *  The type of an attribute or parameter is 
 *  composed of it's domain and shape.
 */

typedef enum {
  dom_invalid = 0,
  dom_any = 455,
  dom_symbol,
  dom_instance_id,
  dom_opaque,
  dom_number,
  dom_int_atom,
  dom_dbl_atom
} RulDecl_Domain;

typedef enum {
  shape_invalid = 0,
  shape_molecule = 364,
  shape_atom,
  shape_compound,
  shape_table
} RulDecl_Shape;

/* Conflict resolution strategies */
typedef long RulStrategy;

/* END-INCLUDE-IN-GEND.H  *********************************************** */

#ifndef OPS__C_SUCCESS
#define OPS__C_SUCCESS	RUL__C_SUCCESS
#endif

#ifndef OPS__C_RETURN
#define OPS__C_RETURN	RUL__C_RETURN
#endif

#define RUL__C_SUCCESS	0
#define RUL__C_RETURN	1

/*
 * Memory allocation routine function prototypes
 * ??msg.h
 */
#define RUL_MEM_ALLOCATE(type, count)	((type *)rul__mem_calloc((count), \
								 sizeof(type)))
void *rul__mem_calloc (size_t nmemb, size_t size);
void *rul__mem_malloc (size_t size);
void *rul__mem_realloc (void *ptr, size_t size);
void  rul__mem_free (void *ptr);


/*
**  CMP Typedefs
*/
typedef struct ast_node  		*Ast_Node;


/*
 * External (to RULEWORKS) Data Types
 */

typedef enum {
  ext_type_invalid = 0,
  ext_type_long = 273,
  ext_type_short,
  ext_type_byte,
  ext_type_uns_long,
  ext_type_uns_short,
  ext_type_uns_byte,
  ext_type_float,
  ext_type_double,
  ext_type_asciz,
  ext_type_ascid,
  ext_type_void, /* only used for external-routine w/ no RETURNS or no args */
  ext_type_void_ptr,
  ext_type_atom,
  ext_type_atom_ptr,
  ext_type_for_each,
  ext_type_object,
  ext_type_none
} Ext_Type;

#define EXT_TYPE_FIRST ext_type_long
#define EXT_TYPE_LAST ext_type_object

/*
 * External "shape" types 
 */

	/*
	**   Used to indicate array dimensions
	**	n == -1 	means implicitly sized array
	**	n ==  0  	means scalar value; not an array
	**	n >   0		means n is explicit size of array
	*/
#define EXT__C_NOT_ARRAY 	 0
#define EXT__C_IMPLICIT_ARRAY	-1

/*
 * used be methods for indicating an invalid array index
 */
#define METH__C_INVALID_INDEX   -1


/*
 * External Routine Passing mechanisms
 */

typedef enum {
  ext_mech_invalid = 0,
  ext_mech_value = 963,
  ext_mech_ref_rw,
  ext_mech_ref_ro
} Ext_Mech;

#define  CS__C_MEA  171
#define  CS__C_LEX  172

/*  Signs for changes to WM, conflict subsets, and beta memories */
#define  DELTA__C_SIGN_POSITIVE   1
#define  DELTA__C_SIGN_NEGATIVE  -1


