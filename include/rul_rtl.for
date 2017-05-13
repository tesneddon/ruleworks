!
! 
! ****************************************************************************
! *                                                                          *
! *  Copyright (c) 1993                                                      *
! *  by DIGITAL Equipment Corporation, Maynard, Mass.                        *
! *                                                                          *
! *  This software is furnished under a license and may be used and  copied  *
! *  only  in  accordance  with  the  terms  of  such  license and with the  *
! *  inclusion of the above copyright notice.  This software or  any  other  *
! *  copies  thereof may not be provided or otherwise made available to any  *
! *  other person.  No title to and ownership of  the  software  is  hereby  *
! *  transferred.                                                            *
! *                                                                          *
! *  The information in this software is subject to change  without  notice  *
! *  and  should  not  be  construed  as  a commitment by DIGITAL Equipment  *
! *  Corporation.                                                            *
! *                                                                          *
! *  DIGITAL assumes no responsibility for the use or  reliability  of  its  *
! *  software on equipment which is not supplied by DIGITAL.                 *
! *                                                                          *
! ****************************************************************************
 
!*** MODULE RUL_RTL IDENT Version V4.0 ***
	PARAMETER RUL_C_MAX_SYMBOL_SIZE = '00000100'X !  Max chars in symbolic atom
	PARAMETER RUL_C_INVALID_LENGTH = -1
	PARAMETER RUL_C_RESET_ATOM = 0
	PARAMETER RUL_C_INVALID_ATOM = 0
	! ** INTEGER*4 RUL_ATOM
! 		Atom Type Predicate Functions
	BYTE      RUL_ATOM_IS_FATOM
	EXTERNAL  RUL_ATOM_IS_FATOM
	BYTE      RUL_ATOM_IS_IATOM
	EXTERNAL  RUL_ATOM_IS_IATOM
	BYTE      RUL_ATOM_IS_SYMBOL
	EXTERNAL  RUL_ATOM_IS_SYMBOL
	BYTE      RUL_ATOM_IS_INSTANCE_ID
	EXTERNAL  RUL_ATOM_IS_INSTANCE_ID
	BYTE      RUL_ATOM_IS_OATOM
	EXTERNAL  RUL_ATOM_IS_OATOM
! 		Atom Conversion Functions
	REAL*4    RUL_FATOM_TO_FLOAT
	EXTERNAL  RUL_FATOM_TO_FLOAT
	INTEGER*4 RUL_FLOAT_TO_FATOM  ! type is "RUL_ATOM"
	EXTERNAL  RUL_FLOAT_TO_FATOM
	INTEGER*4 RUL_GENSYM  ! type is "RUL_ATOM"
	EXTERNAL  RUL_GENSYM
	INTEGER*4 RUL_GENSYMP  ! type is "RUL_ATOM"
	EXTERNAL  RUL_GENSYMP
	INTEGER*4 RUL_GENINT  ! type is "RUL_ATOM"
	EXTERNAL  RUL_GENINT
	INTEGER*4 RUL_IATOM_TO_INTEGER
	EXTERNAL  RUL_IATOM_TO_INTEGER
	INTEGER*4 RUL_OATOM_TO_PTR
	EXTERNAL  RUL_OATOM_TO_PTR
	INTEGER*4 RUL_INTEGER_TO_IATOM  ! type is "RUL_ATOM"
	EXTERNAL  RUL_INTEGER_TO_IATOM
	INTEGER*4 RUL_PTR_TO_OATOM  ! type is "RUL_ATOM"
	EXTERNAL  RUL_PTR_TO_OATOM
	INTEGER*4 RUL_STRING_TO_SYMBOL  ! type is "RUL_ATOM"
	EXTERNAL  RUL_STRING_TO_SYMBOL
	INTEGER*4 RUL_SYMBOL_TO_STRING
	EXTERNAL  RUL_SYMBOL_TO_STRING
	INTEGER*4 RUL_STRING_TO_ATOM  ! type is "RUL_ATOM"
	EXTERNAL  RUL_STRING_TO_ATOM
	INTEGER*4 RUL_ATOM_TO_STRING
	EXTERNAL  RUL_ATOM_TO_STRING
	INTEGER*4 RUL_STRING_TO_COMPOUND  ! type is "RUL_ATOM"
	EXTERNAL  RUL_STRING_TO_COMPOUND
!   	WME Predicate Functions
	BYTE      RUL_ATTR_IS_COMPOUND
	EXTERNAL  RUL_ATTR_IS_COMPOUND
	BYTE      RUL_IS_ATTRIBUTE
	EXTERNAL  RUL_IS_ATTRIBUTE
	BYTE      RUL_IS_CLASS
	EXTERNAL  RUL_IS_CLASS
	BYTE      RUL_IS_SUBCLASS
	EXTERNAL  RUL_IS_SUBCLASS
	BYTE      RUL_IS_INSTANCE
	EXTERNAL  RUL_IS_INSTANCE
!  	WME Retrival Functions 
	INTEGER*4 RUL_GET_ATTR_ATOM  ! type is "RUL_ATOM"
	EXTERNAL  RUL_GET_ATTR_ATOM
	INTEGER*4 RUL_GET_CLASS_STRING
	EXTERNAL  RUL_GET_CLASS_STRING
	INTEGER*4 RUL_GET_COMP_ATTR_LENGTH
	EXTERNAL  RUL_GET_COMP_ATTR_LENGTH
	INTEGER*4 RUL_GET_COMP_ATTR_STRING
	EXTERNAL  RUL_GET_COMP_ATTR_STRING
	INTEGER*4 RUL_GET_COMP_ELEM_ATOM  ! type is "RUL_ATOM"
	EXTERNAL  RUL_GET_COMP_ELEM_ATOM
	INTEGER*4 RUL_GET_INSTANCE
	EXTERNAL  RUL_GET_INSTANCE
	INTEGER*4 RUL_GET_NEXT_INSTANCE  ! type is "RUL_ATOM"
	EXTERNAL  RUL_GET_NEXT_INSTANCE
!  	WME Modification Functions 
	BYTE      RUL_SET_ATTR_ATOM
	EXTERNAL  RUL_SET_ATTR_ATOM
	BYTE      RUL_SET_ATTR_FLOAT
	EXTERNAL  RUL_SET_ATTR_FLOAT
	BYTE      RUL_SET_ATTR_INTEGER
	EXTERNAL  RUL_SET_ATTR_INTEGER
	BYTE      RUL_SET_ATTR_STRING
	EXTERNAL  RUL_SET_ATTR_STRING
	BYTE      RUL_SET_COMP_ATTR_STRING
	EXTERNAL  RUL_SET_COMP_ATTR_STRING
	BYTE      RUL_SET_COMP_ELEM_ATOM
	EXTERNAL  RUL_SET_COMP_ELEM_ATOM
	BYTE      RUL_SET_COMP_ELEM_FLOAT
	EXTERNAL  RUL_SET_COMP_ELEM_FLOAT
	BYTE      RUL_SET_COMP_ELEM_INTEGER
	EXTERNAL  RUL_SET_COMP_ELEM_INTEGER
	BYTE      RUL_SET_COMP_ELEM_OPAQUE
	EXTERNAL  RUL_SET_COMP_ELEM_OPAQUE
	BYTE      RUL_SET_COMP_ELEM_STRING
	EXTERNAL  RUL_SET_COMP_ELEM_STRING
	INTEGER*4 RUL_COPY_INSTANCE  ! type is "RUL_ATOM"
	EXTERNAL  RUL_COPY_INSTANCE
	INTEGER*4 RUL_MAKE_INSTANCE  ! type is "RUL_ATOM"
	EXTERNAL  RUL_MAKE_INSTANCE
	BYTE      RUL_REMOVE_INSTANCE
	EXTERNAL  RUL_REMOVE_INSTANCE
!  	Miscellaneous Functions 
	EXTERNAL  RUL_CLEAR
	EXTERNAL  RUL_COMPLETION
	EXTERNAL  RUL_DEBUG
	INTEGER*4 RUL_GET_FIRING_RULE  ! type is "RUL_ATOM"
	EXTERNAL  RUL_GET_FIRING_RULE
	EXTERNAL  RUL_INITIALIZE
	EXTERNAL  RUL_RUN
	EXTERNAL  RUL_STARTUP
	EXTERNAL  RUL_START_ID_TRANSLATION
	EXTERNAL  RUL_END_ID_TRANSLATION

