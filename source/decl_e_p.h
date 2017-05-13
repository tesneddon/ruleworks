/* Typedefs for private objects (not in common.h). */

/************************************************************************
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
******************************************************************/

typedef struct param_decl     *Param_Decl;

struct param_decl {
  Param_Decl	next;
  Mol_Symbol	name;
  Ext_Type	type;
  Cardinality	len;
  Mol_Symbol	arg;
  Ext_Mech	mech;
  long		stack_index;
};

struct ext_rt_decl {
  Ext_Rt_Decl	next;
  Mol_Symbol	name;
  Decl_Block	block;
  Mol_Symbol	alias_for;
  long		num_params;
  long		num_by_ref;
  Boolean	return_value;
  Param_Decl	input_params;
  Param_Decl	return_param;
};

struct ext_alias_decl {
  Ext_Alias_Decl next;
  Ext_Rt_Decl	 func;
};


