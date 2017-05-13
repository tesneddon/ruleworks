/*
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

*/

/* Declarations shared by:
     rul_decl_class.c
     rul_decl_inh.c.
 */

typedef struct    attribute	    *Attribute;
typedef struct    class_encoding    *Class_Encoding;
typedef struct    method_param       Method_Param;

struct rclass {
  Class		 next;		     /* Chain of all classes in block */
  Class		 sibling;	     /* List of siblings */
  Mol_Symbol	 name;		     /* Class name */
  Decl_Block	 block;		     /* Decl block this class is declared */
  Class		 parent;	     /* Parent class */
  Class		 child;		     /* First subclass */
  int		 number_of_children; /* Count of direct descendents */
  Attribute	 local_attributes;   /* List of attributes (re)declared here */
  Attribute	 local_attributes_end; /* Last attribute declared so far */
  Hash_Table	 attribute_ht;	     /* Hash table of this class's (including
				        ancestors') attributes */
  long		 next_offset;	     /* First slot which is not allocated */
  Class_Encoding encoding;	     /* For fast subclass determination */
  long           sys_methods;	     /* bits for which sys method defined */
  Hash_Table	 method_ht;	     /* Methods for this class's (including
				        ancestors') methods */
  Molecule	*init_vector;	     /* Vector for all the default values
				        for attributes, at correct offsets */
};

struct attribute {
  Attribute	next;		/* Chain of attributes in this class */
  Mol_Symbol	name;
  int		offset;		/* Number of longwords from front of WME */
  Boolean	is_builtin;	/* True for predeclared $ID and $INSTANCE-OF */
  Decl_Domain	domain;		/* dom_any, dom_symbol, dom_int_atom, ... */
  Decl_Shape	shape;		/* shape_atom, shape_compound, shape_table */
  Class		class_restrict;	/* for 'INSTANCE-OF foo' declarations */
  Molecule	default_value;
  Molecule	fill_value;
  Boolean	is_original_decl; /* TRUE if this is not a re-declaration of
				     an inherited attribute */
};

struct method_param {
  Decl_Shape    shape;
  Decl_Domain	domain;
  Class 	rclass;
};

struct method {
  Method	 next;
  Mol_Symbol	 name;
  long           method_id; /* == 0 if generic */
  Method_Func    func;
  long		 param_cnt;
  Method_Param	 params[1];
};





