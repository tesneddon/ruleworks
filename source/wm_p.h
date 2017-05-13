/*
 *	rts_wm.c - implements the Working Memory subsystem
 */
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
**	RULEWORKS
**
**  ABSTRACT:
**	
**
**  MODIFIED BY:
**	DEC	Digital Equipment Corporation
**	CPQ	Compaq Computer Corporation
**
**  MODIFICATION HISTORY:
**
**	11-Aug-1992	DEC	Initial version
**	01-Dec-1999	CPQ	Release with GPL
*/

#ifndef DECL_ATT__LOADED
#include <decl_att.h>	/*defines DECL_NUM_BUILTIN_ATTRS etc.*/
#endif


struct object {			/* NOTE: WMO = Working Memory Object	*/
  List         obj_list;	/* pointer back to the List (Elmt) which
				   references this WMO (vs. prev/next)	*/
  Class        obj_class;	/* pointer to the WMO's class descriptor*/
  long         obj_num_attrs;	/* num. of attrs in this WMO (+builtins)*/
  long         obj_cycle;	/* cycle on which this WMO was created	*/
  Wm_Opr_Code  obj_oper;        /* object change code, make,copy,mod... */
  long         obj_changes;     /* track of nested change attempts      */
  Mol_Symbol   obj_mod_rule;	/* rule that last modified this object  */
  Mol_Symbol   obj_mod_rb;	/* rb that last modified this object  */
  Mol_Symbol  *obj_attr_rule;	/* pointer to an array of rule names which
				   modified corresponding attr posns	*/
  Mol_Symbol  *obj_attr_rb;	/* pointer to an array of rule blocks which
				   modified corresponding attr posns	*/
  long         obj_time_tag;	/* the time tag when this WMO was made	*/
  Molecule     obj_attr_val[DECL_NUM_BUILTIN_ATTRS];
				/* the array of attribute values	*/
};

#ifndef MOL_NIL
#define MOL_NIL	rul__mol_symbol_nil()
#endif

#define WM_GET_OBJ_LIST(wmo)			((wmo)->obj_list)
#define WM_GET_OBJ_CLASS(wmo)			((wmo)->obj_class)
#define WM_GET_OBJ_NUM_ATTRS(wmo)		((wmo)->obj_num_attrs)
#define WM_GET_OBJ_CYCLE(wmo)			((wmo)->obj_cycle)
#define WM_GET_OBJ_OPER(wmo)			((wmo)->obj_oper)
#define WM_GET_OBJ_CHANGES(wmo)			((wmo)->obj_changes)
#define WM_GET_OBJ_ATTR_RULES(wmo)		((wmo)->obj_attr_rule)
#define WM_GET_OBJ_ATTR_RBS(wmo)		((wmo)->obj_attr_rb)
#define WM_GET_OBJ_ATTR_RULE(wmo,attr_num)		\
	(WM_GET_OBJ_ATTR_RULES(wmo) == NULL ? MOL_NIL :	\
	 ((wmo)->obj_attr_rule[(attr_num)]))
#define WM_GET_OBJ_ATTR_RB(wmo,attr_num)		\
	(WM_GET_OBJ_ATTR_RBS(wmo) == NULL ? MOL_NIL :	\
	 ((wmo)->obj_attr_rb[(attr_num)]))
#define	WM_GET_OBJ_CRE_RULE(wmo)			\
	WM_GET_OBJ_ATTR_RULE(wmo,DECL_ID_ATTR_OFFSET)
#define	WM_GET_OBJ_CRE_RB(wmo)			\
	WM_GET_OBJ_ATTR_RB(wmo,DECL_ID_ATTR_OFFSET)
#define	WM_GET_OBJ_MOD_RULE(wmo)		((wmo)->obj_mod_rule)
#define	WM_GET_OBJ_MOD_RB(wmo)  		((wmo)->obj_mod_rb)
#define WM_GET_OBJ_TIME_TAG(wmo)		((wmo)->obj_time_tag)
#define WM_GET_OBJ_ATTR_VALS(wmo)		((wmo)->obj_attr_val)
#define WM_GET_OBJ_ATTR_VAL(wmo,attr)		((wmo)->obj_attr_val[(attr)])
#define WM_GET_OBJ_IDENTIFIER(wmo)			\
	WM_GET_OBJ_ATTR_VAL(wmo,DECL_ID_ATTR_OFFSET)
#define WM_GET_OBJ_CLASS_NAME(wmo)			\
	WM_GET_OBJ_ATTR_VAL(wmo,DECL_INSTANCE_OF_ATTR_OFFSET)

#define WM_IS_OBJ_SNAPSHOT(wmo)			(WM_GET_OBJ_LIST(wmo) == NULL)

#define WM_SET_OBJ_LIST(wmo,list)			\
	WM_GET_OBJ_LIST(wmo)		= (list)
#define WM_SET_OBJ_CLASS(wmo,rclass)			\
	WM_GET_OBJ_CLASS(wmo)		= (rclass)
#define WM_SET_OBJ_NUM_ATTRS(wmo,num_attrs)		\
	WM_GET_OBJ_NUM_ATTRS(wmo)	= (num_attrs)
#define WM_SET_OBJ_CYCLE(wmo,cycle)			\
	WM_GET_OBJ_CYCLE(wmo)		= (cycle)
#define WM_SET_OBJ_CHANGES(wmo,count)			\
	WM_GET_OBJ_CHANGES(wmo)		= (count)
#define WM_SET_OBJ_OPER(wmo,oper)			\
        WM_GET_OBJ_OPER(wmo)		= (oper)
#define WM_SET_OBJ_ATTR_RULES(wmo,attr_rules)		\
	WM_GET_OBJ_ATTR_RULES(wmo) 	= (attr_rules)
#define WM_SET_OBJ_ATTR_RBS(wmo,attr_rbs)		\
	WM_GET_OBJ_ATTR_RBS(wmo) 	= (attr_rbs)
#define WM_SET_OBJ_ATTR_RULE(wmo,attr_num,rule_name)	\
	((wmo)->obj_attr_rule[(attr_num)])=(rule_name)
#define WM_SET_OBJ_ATTR_RB(wmo,attr_num,rb_name)	\
	((wmo)->obj_attr_rb[(attr_num)])=(rb_name)
#define	WM_SET_OBJ_CRE_RULE(wmo,rule_name)		\
	WM_SET_OBJ_ATTR_RULE(wmo,DECL_ID_ATTR_OFFSET,rule_name)
#define	WM_SET_OBJ_CRE_RB(wmo,rb_name)		\
	WM_SET_OBJ_ATTR_RB(wmo,DECL_ID_ATTR_OFFSET,rb_name)
#define	WM_SET_OBJ_MOD_RULE(wmo,obj_mod_rule)			\
	WM_GET_OBJ_MOD_RULE(wmo)	= (obj_mod_rule)
#define	WM_SET_OBJ_MOD_RB(wmo,obj_mod_rb)			\
	WM_GET_OBJ_MOD_RB(wmo)		= (obj_mod_rb)
#define WM_SET_OBJ_TIME_TAG(wmo,time_tag)		\
	WM_GET_OBJ_TIME_TAG(wmo)	= (time_tag)


#define WM_SET_OBJ_ATTR_VAL(wmo,attr,val)		\
	WM_GET_OBJ_ATTR_VAL(wmo,attr)	= (val)
#define WM_SET_OBJ_IDENTIFIER(wmo,instance_id)		\
	WM_GET_OBJ_IDENTIFIER(wmo)	= (instance_id)
#define WM_SET_OBJ_CLASS_NAME(wmo,class_name)		\
	WM_GET_OBJ_CLASS_NAME(wmo)	= (class_name)


