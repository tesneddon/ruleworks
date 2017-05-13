(**)
(**)
(******************************************************************************)
(**                                                                          **)
(**  Copyright (c) 1993                                                      **)
(**  by DIGITAL Equipment Corporation, Maynard, Mass.                        **)
(**                                                                          **)
(**  This software is furnished under a license and may be used and  copied  **)
(**  only  in  accordance  with  the  terms  of  such  license and with the  **)
(**  inclusion of the above copyright notice.  This software or  any  other  **)
(**  copies  thereof may not be provided or otherwise made available to any  **)
(**  other person.  No title to and ownership of  the  software  is  hereby  **)
(**  transferred.                                                            **)
(**                                                                          **)
(**  The information in this software is subject to change  without  notice  **)
(**  and  should  not  be  construed  as  a commitment by DIGITAL Equipment  **)
(**  Corporation.                                                            **)
(**                                                                          **)
(**  DIGITAL assumes no responsibility for the use or  reliability  of  its  **)
(**  software on equipment which is not supplied by DIGITAL.                 **)
(**                                                                          **)
(******************************************************************************)
 
(*** MODULE RUL_RTL IDENT Version V4.0 ***)
 
CONST	RUL_C_MAX_SYMBOL_SIZE = 256;    (* Max chars in symbolic atom       *)
	RUL_C_INVALID_LENGTH = -1;
	RUL_C_RESET_ATOM = 0;
	RUL_C_INVALID_ATOM = 0;
 
TYPE	RUL_ATOM =^INTEGER;
 
(*		Atom Type Predicate Functions                               *)
 
[ASYNCHRONOUS] FUNCTION RUL_ATOM_IS_FATOM (
	atom_value : RUL_ATOM) : BOOLEAN; EXTERNAL;
 
[ASYNCHRONOUS] FUNCTION RUL_ATOM_IS_IATOM (
	atom_value : RUL_ATOM) : BOOLEAN; EXTERNAL;
 
[ASYNCHRONOUS] FUNCTION RUL_ATOM_IS_SYMBOL (
	atom_value : RUL_ATOM) : BOOLEAN; EXTERNAL;
 
[ASYNCHRONOUS] FUNCTION RUL_ATOM_IS_INSTANCE_ID (
	atom_value : RUL_ATOM) : BOOLEAN; EXTERNAL;
 
[ASYNCHRONOUS] FUNCTION RUL_ATOM_IS_OATOM (
	atom_value : RUL_ATOM) : BOOLEAN; EXTERNAL;

(*		Atom Conversion Functions                                   *)
 
[ASYNCHRONOUS] FUNCTION RUL_FATOM_TO_FLOAT (
	atom_value : RUL_ATOM) : SINGLE; EXTERNAL;
 
[ASYNCHRONOUS] FUNCTION RUL_FLOAT_TO_FATOM (
	float_value : SINGLE) : RUL_ATOM; EXTERNAL;
 
[ASYNCHRONOUS] FUNCTION RUL_GENSYM : RUL_ATOM; EXTERNAL;
 
[ASYNCHRONOUS] FUNCTION RUL_GENSYMP (
	char_string : [CLASS_S] PACKED ARRAY [$l1..$u1:INTEGER] OF CHAR) : RUL_ATOM; EXTERNAL;
 
[ASYNCHRONOUS] FUNCTION RUL_GENINT : RUL_ATOM; EXTERNAL;
 
[ASYNCHRONOUS] FUNCTION RUL_IATOM_TO_INTEGER (
	atom_value : RUL_ATOM) : INTEGER; EXTERNAL;
 
[ASYNCHRONOUS] FUNCTION RUL_INTEGER_TO_IATOM (
	integer_value : INTEGER) : RUL_ATOM; EXTERNAL;

[ASYNCHRONOUS] FUNCTION RUL_OATOM_TO_PTR (
	atom_value : RUL_ATOM) : INTEGER; EXTERNAL;
 
[ASYNCHRONOUS] FUNCTION RUL_PTR_TO_OATOM (
	integer_value : INTEGER) : RUL_ATOM; EXTERNAL;
 
[ASYNCHRONOUS] FUNCTION RUL_STRING_TO_SYMBOL (
	char_string : [CLASS_S] PACKED ARRAY [$l1..$u1:INTEGER] OF CHAR) : RUL_ATOM; EXTERNAL;
 
[ASYNCHRONOUS] FUNCTION RUL_SYMBOL_TO_STRING (
	VAR char_buffer : [CLASS_S,VOLATILE] PACKED ARRAY [$l1..$u1:INTEGER] OF CHAR;
	buffer_size : INTEGER;
	atom_value : RUL_ATOM) : INTEGER; EXTERNAL;
 
[ASYNCHRONOUS] FUNCTION RUL_STRING_TO_ATOM (
	char_string : [CLASS_S] PACKED ARRAY [$l1..$u1:INTEGER] OF CHAR) : RUL_ATOM; EXTERNAL;
 
[ASYNCHRONOUS] FUNCTION RUL_ATOM_TO_STRING (
	VAR char_buffer : [CLASS_S,VOLATILE] PACKED ARRAY [$l1..$u1:INTEGER] OF CHAR;
	buffer_size : INTEGER;
	atom_value : RUL_ATOM) : INTEGER; EXTERNAL;
 
(*  	WME Predicate Functions                                             *)
 
[ASYNCHRONOUS] FUNCTION RUL_STRING_TO_COMPOUND (
	char_string : [CLASS_S] PACKED ARRAY [$l1..$u1:INTEGER] OF CHAR) : RUL_ATOM; EXTERNAL;
 
[ASYNCHRONOUS] FUNCTION RUL_ATTR_IS_COMPOUND (
	class_name : [CLASS_S] PACKED ARRAY [$l1..$u1:INTEGER] OF CHAR;
	attr_name : [CLASS_S] PACKED ARRAY [$l2..$u2:INTEGER] OF CHAR;
	block_name : [CLASS_S] PACKED ARRAY [$l3..$u3:INTEGER] OF CHAR) : BOOLEAN; EXTERNAL;
 
[ASYNCHRONOUS] FUNCTION RUL_IS_ATTRIBUTE (
	class_name : [CLASS_S] PACKED ARRAY [$l1..$u1:INTEGER] OF CHAR;
	attr_name : [CLASS_S] PACKED ARRAY [$l2..$u2:INTEGER] OF CHAR;
	block_name : [CLASS_S] PACKED ARRAY [$l3..$u3:INTEGER] OF CHAR) : BOOLEAN; EXTERNAL;
 
[ASYNCHRONOUS] FUNCTION RUL_IS_CLASS (
	class_name : [CLASS_S] PACKED ARRAY [$l1..$u1:INTEGER] OF CHAR;
	block_name : [CLASS_S] PACKED ARRAY [$l2..$u2:INTEGER] OF CHAR) : BOOLEAN; EXTERNAL;
 
[ASYNCHRONOUS] FUNCTION RUL_IS_SUBCLASS (
	child_class : [CLASS_S] PACKED ARRAY [$l1..$u1:INTEGER] OF CHAR;
	parent_class : [CLASS_S] PACKED ARRAY [$l2..$u2:INTEGER] OF CHAR;
	block_name : [CLASS_S] PACKED ARRAY [$l3..$u3:INTEGER] OF CHAR) : BOOLEAN; EXTERNAL;
 
[ASYNCHRONOUS] FUNCTION RUL_IS_INSTANCE (
	instance_id : RUL_ATOM) : BOOLEAN; EXTERNAL;
 
(* 	WME Retrival Functions                                              *)
 
[ASYNCHRONOUS] FUNCTION RUL_GET_ATTR_ATOM (
	instance_id : RUL_ATOM;
	attribute : [CLASS_S] PACKED ARRAY [$l2..$u2:INTEGER] OF CHAR) : RUL_ATOM; EXTERNAL;
 
[ASYNCHRONOUS] FUNCTION RUL_GET_CLASS_STRING (
	VAR char_buffer : [CLASS_S,VOLATILE] PACKED ARRAY [$l1..$u1:INTEGER] OF CHAR;
	BUF_LEN : INTEGER;
	instance_id : RUL_ATOM) : INTEGER; EXTERNAL;
 
[ASYNCHRONOUS] FUNCTION RUL_GET_COMP_ATTR_LENGTH (
	instance_id : RUL_ATOM;
	attribute : [CLASS_S] PACKED ARRAY [$l2..$u2:INTEGER] OF CHAR) : INTEGER; EXTERNAL;
 
[ASYNCHRONOUS] FUNCTION RUL_GET_COMP_ATTR_STRING (
	VAR char_buffer : [CLASS_S,VOLATILE] PACKED ARRAY [$l1..$u1:INTEGER] OF CHAR;
	buffer_len : INTEGER;
	instance_id : RUL_ATOM;
	attribute : [CLASS_S] PACKED ARRAY [$l4..$u4:INTEGER] OF CHAR) : INTEGER; EXTERNAL;
 
[ASYNCHRONOUS] FUNCTION RUL_GET_COMP_ELEM_ATOM (
	instance_id : RUL_ATOM;
	attribute : [CLASS_S] PACKED ARRAY [$l2..$u2:INTEGER] OF CHAR;
	element_index : INTEGER) : RUL_ATOM; EXTERNAL;
 
[ASYNCHRONOUS] FUNCTION RUL_GET_INSTANCE (
	VAR char_buffer : [CLASS_S,VOLATILE] PACKED ARRAY [$l1..$u1:INTEGER] OF CHAR;
	buffer_len : INTEGER;
	instance_id : RUL_ATOM) : INTEGER; EXTERNAL;
 
[ASYNCHRONOUS] FUNCTION RUL_GET_NEXT_INSTANCE (
	instance_id : RUL_ATOM) : RUL_ATOM; EXTERNAL;
 
(* 	WME Modification Functions                                          *)
 
[ASYNCHRONOUS] FUNCTION RUL_SET_ATTR_ATOM (
	instance_id : RUL_ATOM;
	attribute : [CLASS_S] PACKED ARRAY [$l2..$u2:INTEGER] OF CHAR;
	VALUE_ : RUL_ATOM) : BOOLEAN; EXTERNAL;
 
[ASYNCHRONOUS] FUNCTION RUL_SET_ATTR_FLOAT (
	instance_id : RUL_ATOM;
	attribute : [CLASS_S] PACKED ARRAY [$l2..$u2:INTEGER] OF CHAR;
	VALUE_ : SINGLE) : BOOLEAN; EXTERNAL;
 
[ASYNCHRONOUS] FUNCTION RUL_SET_ATTR_INTEGER (
	instance_id : RUL_ATOM;
	attribute : [CLASS_S] PACKED ARRAY [$l2..$u2:INTEGER] OF CHAR;
	VALUE_ : INTEGER) : BOOLEAN; EXTERNAL;
 
[ASYNCHRONOUS] FUNCTION RUL_SET_ATTR_STRING (
	instance_id : RUL_ATOM;
	attribute : [CLASS_S] PACKED ARRAY [$l2..$u2:INTEGER] OF CHAR;
	VALUE_ : [CLASS_S] PACKED ARRAY [$l3..$u3:INTEGER] OF CHAR) : BOOLEAN; EXTERNAL;
 
[ASYNCHRONOUS] FUNCTION RUL_SET_COMP_ATTR_STRING (
	instance_id : RUL_ATOM;
	attribute : [CLASS_S] PACKED ARRAY [$l2..$u2:INTEGER] OF CHAR;
	values_list : [CLASS_S] PACKED ARRAY [$l3..$u3:INTEGER] OF CHAR) : BOOLEAN; EXTERNAL;
 
[ASYNCHRONOUS] FUNCTION RUL_SET_COMP_ELEM_ATOM (
	instance_id : RUL_ATOM;
	attribute : [CLASS_S] PACKED ARRAY [$l2..$u2:INTEGER] OF CHAR;
	index : INTEGER;
	VALUE_ : RUL_ATOM) : BOOLEAN; EXTERNAL;
 
[ASYNCHRONOUS] FUNCTION RUL_SET_COMP_ELEM_FLOAT (
	instance_id : RUL_ATOM;
	attribute : [CLASS_S] PACKED ARRAY [$l2..$u2:INTEGER] OF CHAR;
	index : INTEGER;
	VALUE_ : SINGLE) : BOOLEAN; EXTERNAL;
 
[ASYNCHRONOUS] FUNCTION RUL_SET_COMP_ELEM_INTEGER (
	instance_id : RUL_ATOM;
	attribute : [CLASS_S] PACKED ARRAY [$l2..$u2:INTEGER] OF CHAR;
	index : INTEGER;
	VALUE_ : INTEGER) : BOOLEAN; EXTERNAL;

[ASYNCHRONOUS] FUNCTION RUL_SET_COMP_ELEM_OPAQUE (
	instance_id : RUL_ATOM;
	attribute : [CLASS_S] PACKED ARRAY [$l2..$u2:INTEGER] OF CHAR;
	index : INTEGER;
	VALUE_ : INTEGER) : BOOLEAN; EXTERNAL;
 
[ASYNCHRONOUS] FUNCTION RUL_SET_COMP_ELEM_STRING (
	instance_id : RUL_ATOM;
	attribute : [CLASS_S] PACKED ARRAY [$l2..$u2:INTEGER] OF CHAR;
	index : INTEGER;
	VALUE_ : [CLASS_S] PACKED ARRAY [$l4..$u4:INTEGER] OF CHAR) : BOOLEAN; EXTERNAL;
 
[ASYNCHRONOUS] FUNCTION RUL_COPY_INSTANCE (
	instance_id : RUL_ATOM) : RUL_ATOM; EXTERNAL;
 
[ASYNCHRONOUS] FUNCTION RUL_MAKE_INSTANCE (
	VALUE_ : [CLASS_S] PACKED ARRAY [$l1..$u1:INTEGER] OF CHAR;
	block_name : [CLASS_S] PACKED ARRAY [$l2..$u2:INTEGER] OF CHAR) : RUL_ATOM; EXTERNAL;
 
[ASYNCHRONOUS] FUNCTION RUL_REMOVE_INSTANCE (
	instance_id : RUL_ATOM) : BOOLEAN; EXTERNAL;
 
(* 	Miscellaneous Functions                                             *)
 
[ASYNCHRONOUS] PROCEDURE RUL_CLEAR; EXTERNAL;
 
[ASYNCHRONOUS] PROCEDURE RUL_COMPLETION (
	%IMMED [UNBOUND, ASYNCHRONOUS] PROCEDURE ADDRESS); EXTERNAL;
 
[ASYNCHRONOUS] PROCEDURE RUL_DEBUG; EXTERNAL;
 
[ASYNCHRONOUS] FUNCTION RUL_GET_FIRING_RULE : RUL_ATOM; EXTERNAL;
 
[ASYNCHRONOUS] PROCEDURE RUL_INITIALIZE; EXTERNAL;
 
[ASYNCHRONOUS] PROCEDURE RUL_RUN; EXTERNAL;
 
[ASYNCHRONOUS] PROCEDURE RUL_STARTUP; EXTERNAL;
 
[ASYNCHRONOUS] PROCEDURE RUL_START_ID_TRANSLATION; EXTERNAL;
 
[ASYNCHRONOUS] PROCEDURE RUL_END_ID_TRANSLATION; EXTERNAL;

