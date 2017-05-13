/* RULEWORKS class declarations */
/****************************************************************************
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
 *  FACILITY:
 *	RULEWORKS run time system and compiler
 *
 *  ABSTRACT:
 *	Declaration routines.  Includes object classes and declaring blocks.
 *	Error checking must be done by the callers of this subsystem.
 *
 *  MODIFIED BY:
 *	DEC	Digital Equipment Corporation
**	CPQ	Compaq Computer Corporation
 *
 *  MODIFICATION HISTORY:
 *
 *	19-Aug-1992	DEC	Initial version, copied from ops_wme_class.c.
 *
 *	 1-Dec-1992	DEC	Split off declaration block code:
 *					Hid declaration block structure from
 *			        class code
 *
 *	11-Jan-1993	DEC	Changed from rul__decl_set_cur_attr_shape to
 *					rul__decl_set_cur_attr_compound, since we don't
 *					generate constants of type Decl_Shape.
 *
 *	24-Feb-1993	DEC	Set the domain and class-restriction for
 *					the $ID built-in attribute, and the domain
 *					for ^$INSTANCE-OF
 *
 *      12-Nov-1993	DEC	Add support for methods
 *
 *	18-May-1994	DEC	Change "1 << i" to "1l << i" where long needed.
 *
 *	16-Feb-1998	DEC	class type changed to rclass
 *
 *	01-Dec-1999	CPQ	Release with GPL
 */

#include <common.h>
#include <decl.h>
#include <hash.h>		/* Needed before decl_c_p.h */
#include <decl_c_p.h> 		/* For Class data structure */
#include <decl_att.h>
#include <mol.h>
#include <ios.h>
#include <list.h>
#include <dyar.h>

#define ATTRIBUTE_HT_SIZE 53	/* Expected number of attributes per class */

/*
 * Private (static) variables
 *
 * Variables which keep track of "current" objects
 */
static Class		current_class;
static Attribute	current_attribute;
static Method		current_method;

/*
 * info for methods
 */
#define is_a_generic_method(meth)   (meth->method_id == 0)

static Molecule SM__system_method_name [SYSTEM_METHODS];
static Boolean  SB__gen_sys_methods_defined = FALSE;
static Boolean  SB__system_method_names = FALSE;

/*
 * Private function forward declarations
 */

static Attribute
get_attribute (Class class_obj, Mol_Symbol attribute_name);

static void
set_cur_attr_to_inh_clone (Attribute parent_attr);

static Attribute 
copy_attribute (Attribute old_attr);

static void
append_to_local_attr_list (Class rclass, Attribute attribute);

Attribute rul__decl_remove_attr_cur_class (Attribute attr);

static void add_class_method (Class rclass);

static void decl_init_sys_meth_mols (void);


/*****************************************************************************/
Decl_Block
rul__decl_get_class_block (Class class_desc)
{
    if (class_desc->name == rul__mol_symbol_root()) return NULL;
    return class_desc->block;
}


/*****************************************************************************/
Mol_Symbol
rul__decl_get_class_block_name (Class class_desc)
{
  if (class_desc->name == rul__mol_symbol_root ())
    return NULL;
  return rul__decl_get_decl_name (rul__decl_get_class_block (class_desc));
}

/*****************************************************************************/
Mol_Symbol
rul__decl_get_class_name (Class class_desc)
{
    return (class_desc->name);
}

/*****************************************************************************/
Class
rul__decl_get_class_next (Class class_desc)
{
    return (class_desc->next);
}


/*****************************************************************************/
/*
 * Incremental class constructors
 */
/*****************************************************************************/
void
rul__decl_create_class (Mol_Symbol block_name, Mol_Symbol class_name)
/* Creates a class.  Class must get a parent with
 * rul__decl_set_cur_class_parent() before adding attributes.
 */
{
    Decl_Block block = rul__decl_get_block (block_name);

    assert (rul__hash_get_entry(rul__decl_get_block_class_ht(block),
				class_name)
	    == NULL); /* assert class doesn't already exist */

    /* Do initialization here for lack of a better place, since
     * this routine is called before these symbols are needed.
     */

    current_class = RUL_MEM_ALLOCATE (struct rclass, 1);
    current_class->block = block;

    current_class->name = class_name;
    rul__mol_incr_uses (class_name);

    /*
     * Create the hash table of attributes in this class.
     */
    current_class->attribute_ht = rul__hash_create_table (ATTRIBUTE_HT_SIZE);

    /*
     * Create the hash table of methods in this class.
     */
    current_class->method_ht = rul__hash_create_table (METHOD_HT_SIZE);

    /*
     * Put the class in the hash table of classes in this block.
     */
    rul__hash_add_entry (rul__decl_get_block_class_ht (block),
			 class_name, current_class);

    /*
     * Add new class to end of ordered list of classes in this block.
     */
    if (rul__decl_get_block_classes(block) == NULL)	/* Empty list */
	rul__decl_set_block_classes(block, current_class);
    else			/* At least one class already there */
	rul__decl_get_block_classes_end(block)->next = current_class;
    rul__decl_set_block_classes_end(block, current_class);
}


/*****************************************************************************/
Class
rul__decl_destroy_class (Class rclass)
{
  Attribute attr;
  Class next_class;

  rul__mol_decr_uses (rclass->name);

  next_class = rclass->next;
  attr = rclass->local_attributes;
  while (attr != NULL)
    attr = rul__decl_remove_attr_cur_class(attr);
    
  rul__hash_delete_table (rclass->attribute_ht);

  if (rclass->method_ht)
    rul__hash_delete_table (rclass->method_ht);

  if (rclass->init_vector != NULL)
    rul__mem_free(rclass->init_vector);

  rul__mem_free(rclass);
  return next_class;
}

/*****************************************************************************/
void
rul__decl_set_cur_class_parent (Mol_Symbol parent_name)
/* Must be called before adding attributes to current class. */
/* Implicit inputs: current_class */
{
    Class parent_class = rul__decl_get_class (current_class->block,
					      parent_name);

    assert (current_class != NULL);
    assert (parent_class != NULL);

    /* Add current_class to the head of the list of parent's children. */
    current_class->sibling = parent_class->child;
    parent_class->child = current_class;
    current_class->parent = parent_class;

    parent_class->number_of_children++;

    /* Can't use offsets already used by parent. */
    /* Set next_offset to max(current, parent) */
    if (parent_class->next_offset > current_class->next_offset)
	current_class->next_offset = parent_class->next_offset;

    /* Clone the parent class's attribute table ... */
    rul__hash_copy_each_entry (
		       current_class->attribute_ht, /*  Target hash table  */
		       parent_class->attribute_ht); /*  Source hash table  */

    /* Clone the parent class's method table ... */
    rul__hash_copy_each_entry (
		       current_class->method_ht, /*  Target hash table  */
		       parent_class->method_ht); /*  Source hash table  */

    rul__decl_add_attr_cur_class (rul__mol_symbol_id ());
    rul__decl_set_cur_attr_domain (dom_instance_id);
    rul__decl_set_cur_attr_class (current_class);

    rul__decl_add_attr_cur_class (rul__mol_symbol_instance_of ());
    rul__decl_set_cur_attr_domain (dom_symbol);
    rul__decl_set_cur_attr_default (current_class->name);

}



/*****************************************************************************/
void
rul__decl_set_cur_class_patpart (long index, long pattern_part)
{
    rul__decl_set_inh_pattern_part (current_class, index, 
				    (unsigned long) pattern_part);
}



/*****************************************************************************/
void
rul__decl_set_cur_class_masklen (long mask_length)
{
    rul__decl_set_inh_mask_length (current_class, mask_length);
}



/*****************************************************************************/
void
rul__decl_add_attr_cur_class (Mol_Symbol attr_name)
/* Implicit outputs: current_attribute, current_class */
{
    Attribute parent_attr;

    assert (current_class != NULL);

    /* See if this attribute is in the class's attribute hash table. */

    parent_attr = get_attribute (current_class, attr_name);

    if (parent_attr) {
	/* If already declared copy attribute struct from parent */
	set_cur_attr_to_inh_clone (parent_attr);
    }
    else {			/* New attribute */
	current_attribute = RUL_MEM_ALLOCATE (struct attribute, 1);
	rul__hash_add_entry (current_class->attribute_ht,
			     attr_name,
			     current_attribute);

	current_attribute->name = attr_name;
	rul__mol_incr_uses (attr_name);

	current_attribute->offset = current_class->next_offset;
	current_attribute->is_builtin = FALSE; /* This attr is user-defined */
	current_attribute->domain = dom_any; /* Default to untyped */
	current_attribute->shape = shape_atom;
	current_attribute->class_restrict = NULL;
	current_attribute->default_value = rul__mol_symbol_nil();
	current_attribute->fill_value = rul__mol_symbol_nil();
	current_attribute->is_original_decl = TRUE;

	current_class->next_offset++;

	append_to_local_attr_list (current_class, current_attribute);
    }
}

/*****************************************************************************/
Attribute rul__decl_remove_attr_cur_class (Attribute attr)
{
  Attribute next_attr;

  next_attr = attr->next;
  rul__mol_decr_uses(attr->name);
  rul__mol_decr_uses(attr->default_value);
  rul__mol_decr_uses(attr->fill_value);

  rul__mem_free(attr);
  return next_attr;
}

/*****************************************************************************/
void
rul__decl_set_cur_attr_domain (Decl_Domain data_type)
{
    Molecule new_val;

    current_attribute->domain = data_type;

    switch (data_type) {
	case dom_symbol:
		return;	  /* Already have default/fill of NIL  */
		break;

	/* note: all of these molecule values below are permanent */
	case dom_number :
	case dom_int_atom :
		new_val = rul__mol_integer_zero ();
		break;
	case dom_dbl_atom :
		new_val = rul__mol_double_zero ();
		break;
	case dom_instance_id :
		new_val = rul__mol_instance_id_zero();
		break;
	case dom_opaque :
		new_val = rul__mol_opaque_null ();
		break;
	default :
		new_val = rul__mol_symbol_nil ();
		break;
    }
    if (current_attribute->shape == shape_compound) {
	if (current_attribute->fill_value == rul__mol_symbol_nil()) {
	    current_attribute->fill_value = new_val;
	}
    } else {
	if (current_attribute->default_value == rul__mol_symbol_nil()) {
	    current_attribute->default_value = new_val;
	}
    }
}


/*****************************************************************************/
void
rul__decl_set_cur_attr_compound (void)
{
    current_attribute->shape = shape_compound;
    current_attribute->default_value = rul__mol_compound_empty();
}

/*****************************************************************************/
void
rul__decl_set_cur_attr_class (Class instance_class)
{
    current_attribute->class_restrict = instance_class;
}

/*****************************************************************************/
void
rul__decl_set_cur_attr_default (Molecule default_value)
{
  if (current_attribute->default_value != rul__mol_symbol_nil()) {
    /*may already be inherited*/
    rul__mol_decr_uses (current_attribute->default_value);
  }
  current_attribute->default_value = default_value;
  rul__mol_incr_uses (default_value);
}

/*****************************************************************************/
void
rul__decl_set_cur_attr_fill (Molecule fill_value)
{
    rul__mol_incr_uses (fill_value);
    current_attribute->fill_value = fill_value;
}

/*****************************************************************************/
void
rul__decl_set_cur_attr_offset (int offset)
{
    current_attribute->offset = offset;
}


/*****************************************************************************/
static void
set_init_vector_value (Attribute attr)
/* This routine is used as a parameter to rul__hash_for_each_entry from
   rul__decl_finish_cur_class.  Set the entry in the init_vector to the
   default value of this attribute. */
{
    /* Don't store attributes before $INSTANCE-OF (i.e., $ID). */
    if (attr->offset >= DECL_INSTANCE_OF_ATTR_OFFSET)
	current_class->init_vector
	    [attr->offset - DECL_INSTANCE_OF_ATTR_OFFSET] =
		attr->default_value;
}

/*****************************************************************************/
Class
rul__decl_finish_cur_class (void)
{
    /* Create and fill in the class's initialization vector */

    int num_attrs = rul__decl_get_class_num_attrs (current_class);

    /* Init vector holds attrs starting with $INSTANCE-OF. */
    current_class->init_vector =
	rul__mem_calloc (num_attrs - 1,	/* don't include $ID */
			 sizeof (Molecule));
    rul__hash_for_each_entry (current_class->attribute_ht,
			      (void (*) (void *)) set_init_vector_value);
    return current_class;
}


/*****************************************************************************/
/*
 * Finding class identifiers
 */
/*****************************************************************************/
Class
rul__decl_get_class_if_unique (Mol_Symbol class_name)
{
    Decl_Block db;
    Class rclass = NULL;
    int number_found = 0;

    for (db = rul__decl_get_first_decl_block();
	 db != NULL && number_found <= 1;
	 db = rul__decl_get_next_decl_block(db)) {
      if (rclass == NULL) {
	if ((rclass = rul__decl_get_class (db, class_name)) != NULL)
	  number_found++;
      }
      else {
	if (rul__decl_get_class (db, class_name) != NULL)
	  number_found++;
      }
    }

    if (number_found == 1)
	return rclass;		/* It is unique */
    else
	return NULL;		/* Not found, or not unique */
}


/*****************************************************************************/
Class
rul__decl_get_class (Decl_Block block, Mol_Symbol class_name)
/* Returns NULL if class_name isn't an OBJECT-CLASS name in the block. */
{
    return (Class)
	rul__hash_get_entry (rul__decl_get_block_class_ht(block), class_name);
}


/*****************************************************************************/
Class
rul__decl_get_visible_class (Mol_Symbol class_name)
/* Returns class if visible in current entry block.  Else returns NULL. */
{
    List	db_list_iterator;
    Mol_Symbol	db_name;
    Class	rclass;

    /*
     * Loop through all declaring blocks visible to the current entry block
     * looking for one which defines the class.
     */
    for (db_list_iterator = rul__decl_get_visible_decl_blks ();
	 !rul__list_is_empty (db_list_iterator);
	 db_list_iterator = rul__list_rest (db_list_iterator)) {

	db_name = rul__list_first(db_list_iterator);
	    
	rclass = rul__decl_get_class (rul__decl_get_block(db_name), class_name);
	if (rclass != NULL)
	    return rclass;
	}

    return NULL;		/* Not found */
}    
	
  

/*****************************************************************************/
Class
rul__decl_get_visible_class_rt (Mol_Symbol class_name,
				Mol_Symbol blocks[],
				long blk_count)
/* Returns class if visible in current entry block at runtime. */
{
  Class	rclass = NULL;
  long        i;

  /*
   * Loop through the array of currently visible declaring blocks of
   * the current entry block looking for one which defines the class.
   */
  for (i = 0; i < blk_count; i++) {
    if ((rclass = rul__decl_get_class (rul__decl_get_block (blocks[i]),
				      class_name)) != NULL)
      break;
  }
  
  return rclass;
}    


Boolean
rul__decl_is_class_visible_rt  (Class rclass,
				Mol_Symbol blocks[],
				long blk_count)
/* Returns class if visible in current entry block at runtime. */
{
  Mol_Symbol class_block_name = rul__decl_get_class_block_name (rclass);
  long        i;

  /*
   * Loop through the array of currently visible declaring blocks of
   * the current entry block looking for one which defines the class.
   */
  for (i = 0; i < blk_count; i++) {
    if (class_block_name == blocks[i]) return TRUE;
  }
  
  return FALSE;
}    
	
  

static void decl_for_each_subclass (Class rclass, void (*func)(Class subclass))
{
  if (rclass) {
    (*func) (rclass);
    decl_for_each_subclass (rclass->child, func);

    while (rclass->sibling) {
      decl_for_each_subclass (rclass->sibling, func);
      rclass = rclass->sibling;
    }
  }
}


/*****************************************************************************/
void rul__decl_for_each_subclass (Class rclass, void (*func)(Class subclass))
{
  (*func) (rclass);
  decl_for_each_subclass (rclass->child, func);
}


static Boolean decl_is_a_subclass (Class rclass, Class parent)
{
  if (parent) {
    if (rclass == parent)
      return TRUE;
    if (decl_is_a_subclass (rclass, parent->child))
      return TRUE;
    while (parent->sibling) {
      if (decl_is_a_subclass (rclass, parent->sibling))
	return TRUE;
      parent = parent->sibling;
    }
  }
  return FALSE;
}


Boolean rul__decl_is_a_subclass (Class rclass, Class parent)
{
  if (rclass == parent)
    return TRUE;
  return (decl_is_a_subclass (rclass, parent->child));
}  

/*****************************************************************************/
/*
 * Querying class and attribute information
 */
/*****************************************************************************/
Boolean
rul__decl_is_attr_in_class (Class class_desc, Mol_Symbol attribute_name)
{
    assert (class_desc != NULL);
    return get_attribute (class_desc, attribute_name) != NULL;
}


/*****************************************************************************/
Boolean
rul__decl_is_locl_attr_in_class (Class class_desc, Mol_Symbol attribute_name)
/* This returns true if the attribute has been explicitly declared or
 * redeclared in this specific class.  This is intended only for semantic
 * checking to catch defining an attribute twice in one class.
 */
{
    Attribute a;

    assert (class_desc != NULL);
    /* Checking the hash-table would only tell us if the attribute is declared
     * for this class.  It would return true for all inherited attributes.  So
     * we need to search through the local_attributes list of just this class.
     */
    for (a = class_desc->local_attributes;
	 a != NULL && a->name != attribute_name;
	 a = a->next)
	;
    if (a == NULL)
	return FALSE;
    else
	return TRUE;
}


/*****************************************************************************/
Boolean
rul__decl_is_leaf_class (Class class_desc)
{
    assert (class_desc != NULL);
    return (class_desc->child == NULL); /* If no children, it's a leaf */
}

/*
 * rul__decl_is_subclass_of is defined in rul_decl_inh.c
 */


/*****************************************************************************/
Boolean
rul__decl_is_root_class (Class class_desc)
{
    assert (class_desc != NULL);
    return (class_desc->parent == class_desc); /* If parent is self, root */
}

/*
 * rul__decl_is_subclass_of is defined in rul_decl_inh.c
 */


/*****************************************************************************/
long
rul__decl_get_attr_offset (Class class_desc, Mol_Symbol attribute_name)
{
    return get_attribute (class_desc, attribute_name)->offset;
}


/*****************************************************************************/
Mol_Symbol
rul__decl_get_attr_name (Class class_desc, long offset)
/* Given the offset into an object and the class of the object, return
 * the name of the attribute at that offset.
 * This isn't incredibly fast, and is intended only for uses where speed
 * isn't critical such as debugger commands.  If this needs to be sped up,
 * data structures should change.
 */
{
    Class	c;
    Attribute	a;

    assert (class_desc != NULL);

    for (c = class_desc; c != NULL; c = c->parent) {
	for (a = c->local_attributes; a != NULL; a = a->next)
	    if (a->offset == offset)
		return a->name;
	if (rul__decl_is_root_class (c))
	    break;
    }
    return NULL;		/* No attribute at that offset */
}


/*****************************************************************************/
Decl_Domain
rul__decl_get_attr_domain (Class class_desc, Mol_Symbol attribute_name)
{
    return get_attribute (class_desc, attribute_name)->domain;
}

/*****************************************************************************/
Decl_Shape
rul__decl_get_attr_shape (Class class_desc, Mol_Symbol attribute_name)
{
    return get_attribute (class_desc, attribute_name)->shape;
}

/*****************************************************************************/
Class
rul__decl_get_attr_class (Class class_desc, Mol_Symbol attribute_name)
{
    return get_attribute (class_desc, attribute_name)->class_restrict;
}

/*****************************************************************************/
Molecule
rul__decl_get_attr_default (Class class_desc, Mol_Symbol attribute_name)
{
    return get_attribute (class_desc, attribute_name)->default_value;
}

/*****************************************************************************/
Molecule
rul__decl_get_attr_fill (Class class_desc, Mol_Symbol attribute_name)
{
    return get_attribute (class_desc, attribute_name)->fill_value;
}

/*****************************************************************************/
Boolean
rul__decl_is_attr_builtin (Class class_desc, Mol_Symbol attribute_name)
{
    return get_attribute (class_desc, attribute_name)->is_builtin;
}

/*****************************************************************************/
Molecule *
rul__decl_get_class_default (Class class_desc)
{
    return class_desc->init_vector;
}

/*****************************************************************************/
long
rul__decl_get_class_num_attrs (Class class_desc)
/* Return number of attrs, including builtin attrs. */
{
    /* The first attr is at offset 0. */
    return class_desc->next_offset;
}


/*****************************************************************************/
int
rul__decl_get_class_depth (Class class_desc)
{
    int depth;
    Class c;

    /* Walk up the hierarchy from the given class to $ROOT, counting the
     * number of steps.  Depth of $ROOT is 0.
     */
    for (depth = 0, c = class_desc;
	 c->name != rul__mol_symbol_root();
	 depth++, c = c->parent)
	;
    return depth;
}

/*****************************************************************************/
Class
rul__decl_get_class_parent (Class class_desc)
{
    assert (class_desc != NULL);

    if (class_desc->name == rul__mol_symbol_root()) return NULL;

    return class_desc->parent;
}


/*****************************************************************************/
Class
rul__decl_get_class_ancestor (Class class_desc)
{
    Class c;

    /* Walk up the hierarchy from the given class to $ROOT, counting the
     * number of steps.  Depth of $ROOT is 0.
     */
    c = class_desc;
    while (c->parent->name != rul__mol_symbol_root()) {
	c = c->parent;
    }
    return (c);
}

/*****************************************************************************/

Class 
rul__decl_lowest_common_class (Class class_1, Class class_2)
	/*  Find the lowest common parent class that is lower than $root  */
{
	Class tmp, other, top_class;
	long depth_1, depth_2;

	if (class_1 == NULL) return NULL;
	if (class_2 == NULL) return NULL;

	top_class = rul__decl_get_class_ancestor (class_1);

	/*  if they don't fall into the same family, ... */
	if (top_class != rul__decl_get_class_ancestor (class_2)) return NULL;

	depth_1 = rul__decl_get_class_depth (class_1);
	depth_2 = rul__decl_get_class_depth (class_2);
	if (depth_1 > depth_2) {
	    tmp = class_2;
	    other = class_1;
	} else {
	    tmp = class_1;
	    other = class_2;
	}

	while (tmp != top_class) {
	    if (rul__decl_is_subclass_of (other, tmp)) return tmp;
	    tmp = rul__decl_get_class_parent (tmp);
	}
	return (top_class);
}



#ifndef NDEBUG
/*****************************************************************************/
void
rul__decl_dump_class (Class c)
/* For debugging.  Print out the class pointed to by c. */
{
    Attribute attr;

    if (c->name == rul__mol_symbol_root())
	rul__ios_printf ( RUL__C_STD_ERR, "\n\nDecl_Block: %s",
		rul__mol_get_readform (rul__decl_get_class_block_name(c)));
    rul__ios_printf( RUL__C_STD_ERR, "\nClass: %-15s #child: %2d  Sibling: %s  1st-Child: %s",
	rul__mol_get_readform (c->name),
	c->number_of_children,
        c->sibling ? 	rul__mol_get_readform(c->sibling->name) : " ",
        c->child ?	rul__mol_get_readform(c->child->name)   : " ");
    for (attr = c->local_attributes; attr != NULL; attr = attr->next) {
        rul__ios_printf( RUL__C_STD_ERR, "\n  Attr:    %-20s %s %s %d ", 
	    rul__mol_get_readform (attr->name),
	    attr->is_original_decl ? " o " : "*r*",
	    attr->shape == shape_compound ? "(compound)" : "          ",
	    attr->offset);
	if (attr->fill_value != rul__mol_symbol_nil())
	    rul__ios_printf ( RUL__C_STD_ERR, "\n          Fill:     %-20s",
		    rul__mol_get_readform (attr->fill_value));
	if (attr->default_value != rul__mol_symbol_nil() &&
	    attr->default_value != rul__mol_compound_empty())
	    rul__ios_printf ( RUL__C_STD_ERR, "\n          Default:  %-20s",
		    rul__mol_get_readform (attr->default_value));
    }
}
#endif


#ifndef NDEBUG
/*****************************************************************************/
void
rul__decl_dump_classes ()
/* For debugging.  Print out all classes. */
{
    Decl_Block	block;
    Class	rclass;

    for (block = rul__decl_get_first_decl_block();
	 block != NULL;
	 block = rul__decl_get_next_decl_block(block))
	for (rclass = rul__decl_get_block_classes(block);
	     rclass != NULL;
	     rclass = rclass->next)
	    rul__decl_dump_class (rclass);
}
#endif



/*****************************************************************************/
void
rul__decl_create_root_class (Mol_Symbol block_name)
{

    rul__decl_create_class (block_name, rul__mol_symbol_root());

    /* Set parent to self to mark as root. */

    current_class->parent = current_class;

    /* Add the builtin attributes. */

    current_class->next_offset = DECL_ID_ATTR_OFFSET;
    rul__decl_add_attr_cur_class (rul__mol_symbol_id ());
    rul__decl_set_cur_attr_domain (dom_instance_id);
    rul__decl_set_cur_attr_class (current_class);

    current_class->next_offset = DECL_INSTANCE_OF_ATTR_OFFSET;
    rul__decl_add_attr_cur_class (rul__mol_symbol_instance_of ());
    rul__decl_set_cur_attr_domain (dom_symbol);
    rul__decl_set_cur_attr_default (current_class->name);

    current_class->next_offset = DECL_FIRST_USER_ATTR_OFFSET;

    rul__decl_finish_cur_class (); /* Create init vector */
}


/*****************************************************************************/
/*
 * Private functions
 */
/*****************************************************************************/
static Attribute
get_attribute (Class class_obj, Mol_Symbol attribute_name)
{
    assert (class_obj != NULL);
    return (Attribute) rul__hash_get_entry(class_obj->attribute_ht,
					   attribute_name);
}


/*********************
**                  **
**  COPY_ATTRIBUTE  **
**                  **
*********************/

static Attribute
copy_attribute (Attribute old_attr)
{
    Attribute new_attr;

    new_attr = RUL_MEM_ALLOCATE (struct attribute, 1);
    *new_attr = *old_attr;	/* struct assignment */

    new_attr->next = NULL;
    rul__mol_incr_uses(new_attr->name);
    rul__mol_incr_uses(new_attr->default_value);
    rul__mol_incr_uses(new_attr->fill_value);

    return new_attr;
}


/********************************
**                             **
**  APPEND_TO_LOCAL_ATTR_LIST  **
**                             **
********************************/

static void
append_to_local_attr_list (Class rclass, Attribute attribute)
{
    /* Add new attribute to end of the class's attribute list. */

    if (rclass->local_attributes == NULL) /*  This is the first attribute  */
        rclass->local_attributes = attribute;
    else
	/* At least one attribute already there, so add at end of list */
        rclass->local_attributes_end->next = attribute;
    rclass->local_attributes_end = attribute; /* Now last attr on list */
}



/********************************
**                             **
**  SET_CUR_ATTR_TO_INH_CLONE  **
**                             **
********************************/

static void
set_cur_attr_to_inh_clone (Attribute parent_attr)
{
    /*
    **  Copy the inherited attribute structure
    */
    current_attribute = copy_attribute (parent_attr);

    current_attribute->is_original_decl = FALSE;

    /*
    **  Add the cloned structure to the current class's list of 'local' attrs
    */
    append_to_local_attr_list (current_class, current_attribute);

    /*
    **  Replace the inherited attribute in the
    **  current class's attribute hash table.
    */
    rul__hash_replace_entry (	current_class->attribute_ht,
				current_attribute->name,
			  	current_attribute);
}

/*****************************************************************************/

static void print_class_attr (Attribute attr, IO_Stream ios)
{
   Molecule def_val = rul__mol_symbol_nil ();


   rul__ios_printf(ios, "     ^");
   rul__mol_print_readform(attr->name, ios);

   switch (attr->shape) {
    case shape_atom:
      break;
    case shape_compound:
      rul__ios_printf(ios, " compound");
      break;
    case shape_table:
      rul__ios_printf(ios, " table");
      break;
   }

   switch (attr->domain) {
    case dom_any:
      break;
    case dom_symbol:
      rul__ios_printf(ios, " symbol");
      break;
    case dom_instance_id:
      rul__ios_printf(ios, " instance-id");
      if (attr->class_restrict &&
	  rul__decl_get_class_name (attr->class_restrict) !=
						  rul__mol_symbol_root ()) {
	 rul__ios_printf(ios, " of ");
	 rul__mol_print_readform (
			  rul__decl_get_class_name(attr->class_restrict), ios);
      }
      def_val = rul__mol_instance_id_zero ();
      break;
    case dom_opaque:
      rul__ios_printf(ios, " opaque");
      def_val = rul__mol_opaque_null ();
      break;
    case dom_number:
      rul__ios_printf(ios, " number");
      def_val = rul__mol_integer_zero ();
      break;
    case dom_int_atom:
      rul__ios_printf(ios, " integer");
      def_val = rul__mol_integer_zero ();
      break;
    case dom_dbl_atom:
      rul__ios_printf(ios, " float");
      def_val = rul__mol_double_zero ();
      break;
   }

   if (attr->shape == shape_compound) {
      if (attr->default_value != rul__mol_compound_empty()) {
	 rul__ios_printf(ios, " (default ");
	 rul__mol_print_readform(attr->default_value, ios);
	 rul__ios_printf(ios, ")");
      }
      if (attr->fill_value != def_val) {
	 rul__ios_printf(ios, " (fill ");
	 rul__mol_print_readform(attr->fill_value, ios);
	 rul__ios_printf(ios, ")");
      }
   }
   else {
      if (attr->default_value != def_val &&
	  attr->default_value != rul__mol_symbol_root ()) {
	 rul__ios_printf(ios, " (default ");
	 rul__mol_print_readform(attr->default_value, ios);
	 rul__ios_printf(ios, ")");
      }
   }
   rul__ios_printf(ios, "\n");
}

static void rul___decl_print_parent_classes (Class c, Dynamic_Array atts,
					     IO_Stream ios)
{
   Attribute attr, tmp_attr;
   long      i, len;

   if (c->parent && c->parent->name != rul__mol_symbol_root())
     rul___decl_print_parent_classes (c->parent, atts, ios);

   for (attr = c->local_attributes; attr != NULL; attr = attr->next) {

      len = rul__dyar_get_length (atts);
      for (i = 0; i < len; i++) {
	 tmp_attr = (Attribute) rul__dyar_get_nth (atts, i);
	 if (attr->name == tmp_attr->name) {
	    rul__dyar_set_nth (atts, i, attr);
	    break;
	 }
      }
      if (i == len)
	rul__dyar_append (atts, attr);
   }
   
   rul__ios_printf(ios, "   ");
   rul__mol_print_readform(c->name, ios);
   rul__ios_printf(ios, "\n");
}


void rul__decl_print_class_info (Class c, IO_Stream ios)
{
   Attribute attr;
   Dynamic_Array atts = rul__dyar_create_array (10);

   rul___decl_print_parent_classes (c, atts, ios);

   attr = (Attribute) rul__dyar_pop_first (atts);
   while (attr) {
      print_class_attr (attr, ios);
      attr = (Attribute) rul__dyar_pop_first (atts);
   }
   rul__dyar_free_array (atts);

}




/******************************
**                           **
**  RUL__DECL_INT_TO_DOMAIN  **
**                           **
******************************/

Decl_Domain rul__decl_int_to_domain (long domain_as_int)
{
  Decl_Domain ret_dom = dom_any;

  switch ((Decl_Domain) domain_as_int) {

  case dom_any:
    ret_dom = dom_any;
    break;

  case dom_symbol:
    ret_dom = dom_symbol;
    break;

  case dom_instance_id:	
    ret_dom = dom_instance_id;
    break;

  case dom_opaque:
    ret_dom = dom_opaque;
    break;

  case dom_number:
    ret_dom = dom_number;
    break;

  case dom_int_atom:
    ret_dom = dom_int_atom;
    break;

  case dom_dbl_atom:
    ret_dom = dom_dbl_atom;
    break;

  default:
    assert (FALSE);  /* invalid domain  */
  }
  
  return ret_dom;
}




/**************************************************************************
 *                                                                        *
 *                   Methods.....                                         *
 *                                                                        *
 *************************************************************************/


Method rul__decl_create_method (Decl_Block block,
				Mol_Symbol meth_name,
				Boolean is_method, /* false if generic */
				long param_cnt)
{
  Method     prev_meth;
  Class      rclass = NULL;
  long       len;

  if (is_method)
    len = (sizeof (struct method));
  else
    len = (sizeof(struct method) + 
	   (param_cnt * (sizeof(struct method_param))));

  current_method = rul__mem_calloc (1, len);
  current_method->next = NULL;
  current_method->name = meth_name;
  rul__mol_incr_uses (meth_name);
  current_method->param_cnt = param_cnt;
  if (is_method)
    current_method->method_id = rul__decl_get_block_method_cnt (block);

  prev_meth = rul__hash_get_entry (rul__decl_get_block_method_ht (block),
				   meth_name);
  if (prev_meth) {
    while (prev_meth->next)
      prev_meth = prev_meth->next;
    prev_meth->next = current_method;
  }
  else {
    rul__hash_add_entry (rul__decl_get_block_method_ht (block),
			 meth_name, current_method);
  }

  return current_method;
}

void rul__decl_destroy_methods (Method meth)
{
  Method next;
  long   i;

  while (meth) {
    rul__mol_decr_uses (meth->name);
    next = meth->next;
    rul__mem_free (meth);
    meth = next;
  }
}

void rul__decl_method_remove (Method meth)
{
  Method     curr_meth, last_meth = NULL;
  Decl_Block block = rul__decl_get_curr_decl_block ();

  curr_meth = rul__hash_get_entry (rul__decl_get_block_method_ht (block),
				   meth->name);
  while (curr_meth) {
    if (curr_meth == meth) {
      if (last_meth)
	last_meth->next = curr_meth->next;
      else
	rul__hash_remove_entry (rul__decl_get_block_method_ht (block),
				meth->name);
      rul__mem_free (meth);
      break;
    }
    last_meth = curr_meth;
    curr_meth = curr_meth->next;
  }
}

Method rul__decl_get_visible_method (Mol_Symbol meth_name)
{
  List        db_list_iterator;
  Mol_Symbol  db_name;
  Method      meth = NULL;
  
  /* Loop through all declaring blocks visible to the current block
   * looking for one which defines the method.
   * Returns 1st method found in any visible block with same name.
   */

  for (db_list_iterator = rul__decl_get_visible_decl_blks ();
       !rul__list_is_empty (db_list_iterator);
       db_list_iterator = rul__list_rest (db_list_iterator)) {

    db_name = rul__list_first (db_list_iterator);

    meth = rul__hash_get_entry (rul__decl_get_block_method_ht (
					       rul__decl_get_block(db_name)),
				meth_name);
    if (meth != NULL)
      return meth;
  }

  return NULL;                /* Not found */
}

void rul__decl_set_method_func (Method_Func func)
{
  current_method->func = func;
}

void rul__decl_set_method_param (long index, Decl_Shape shape,
				 Decl_Domain domain, Class rclass)
{
  current_method->params[index].shape  = shape;
  current_method->params[index].domain = domain;
  current_method->params[index].rclass  = rclass;
}

void rul__decl_finish_method (void)
{
  /* add method to class and all subclasses */

  rul__decl_for_each_subclass (current_method->params[0].rclass,
			       add_class_method);

  current_method = NULL;
}


static void add_class_method (Class subclass)
{
  Method     meth;
  Hash_Table meth_ht;
  long       system_method;

  /* add method to class */
  meth_ht = rul__decl_get_class_method_ht (subclass);

  /* see if method is already defined */
  meth = rul__decl_get_class_method (current_method->params[0].rclass,
				     current_method->name);

  subclass->sys_methods |= rul__decl_is_system_method (current_method->name);

  if (meth) {
    /* if the defined method class is not a subclass of the this class
       then replace it */
    if (!rul__decl_is_a_subclass (meth->params[0].rclass, subclass))
      rul__hash_replace_entry (meth_ht, current_method->name, current_method);
  }
  else
    rul__hash_add_entry (meth_ht, current_method->name, current_method);

}


Hash_Table rul__decl_get_class_method_ht (Class rclass)
{
  return rclass->method_ht;
}

long rul__decl_get_class_sys_methods (Class rclass)
{
  return rclass->sys_methods;
}

Method rul__decl_get_class_method (Class rclass, Mol_Symbol meth_name)
{
  return (rul__hash_get_entry (rul__decl_get_class_method_ht (rclass),
			       meth_name));
}

Boolean rul__decl_is_generic_method (Method meth)
{
  return (is_a_generic_method (meth));
}

static Method find_generic_method (Mol_Symbol meth_name,
				   Class rclass, Decl_Block block)
{
  Method meth;

  meth = rul__hash_get_entry (rul__decl_get_block_method_ht (block),
			      meth_name);

  while (meth) {
    if (is_a_generic_method(meth)) {
      if (!meth->params[0].rclass ||
	  rul__decl_is_a_subclass (rclass, meth->params[0].rclass))
	return meth;
    }
    meth = meth->next;
  }
  return NULL;
}

Method rul__decl_get_generic_method (Class rclass, Mol_Symbol meth_name)
{
  Method      meth = NULL;
  List        db_list_iterator;
  Mol_Symbol  db_name;
  Decl_Block  block;

  if (rclass) {
    meth = find_generic_method (meth_name, rclass, rclass->block);
  }

  else {
    for (db_list_iterator = rul__decl_get_visible_decl_blks ();
	 !rul__list_is_empty (db_list_iterator);
	 db_list_iterator = rul__list_rest (db_list_iterator)) {
      
      db_name = rul__list_first (db_list_iterator);
      block = rul__decl_get_block (db_name);
      meth = find_generic_method (meth_name, rclass, block);
      if (meth)
	break;
    }
  }
  return meth;
}

Mol_Symbol rul__decl_get_method_name (Method meth)
{
  return meth->name;
}

long rul__decl_get_method_num (Method meth)
{
  return meth->method_id;
}

Method rul__decl_get_method_next (Method meth)
{
  return meth->next;
}

Method_Func rul__decl_get_method_func (Method meth)
{
  return meth->func;
}

long rul__decl_get_method_num_params (Method meth)
{
  return meth->param_cnt;
}

Decl_Shape rul__decl_get_method_par_shape (Method meth, long index)
{
  return meth->params[index].shape;
}

Decl_Domain rul__decl_get_method_par_domain (Method meth, long index)
{
  return meth->params[index].domain;
}

Class rul__decl_get_method_par_class (Method meth, long index)
{
  return meth->params[index].rclass;
}

static void decl_init_sys_meth_mols (void)
{
  if (!SB__system_method_names) {
    SB__system_method_names = TRUE;
    SM__system_method_name [BEFORE_MAKE] =
      rul__mol_make_symbol ("$BEFORE-MAKE");
    rul__mol_mark_perm (SM__system_method_name [BEFORE_MAKE]);
    SM__system_method_name [BEFORE_MODIFY] =
      rul__mol_make_symbol ("$BEFORE-MODIFY");
    rul__mol_mark_perm (SM__system_method_name [BEFORE_MODIFY]);
    SM__system_method_name [BEFORE_REMOVE] =
      rul__mol_make_symbol ("$BEFORE-REMOVE");
    rul__mol_mark_perm (SM__system_method_name [BEFORE_REMOVE]);
    SM__system_method_name [AFTER_MAKE] =
      rul__mol_make_symbol ("$AFTER-MAKE");
    rul__mol_mark_perm (SM__system_method_name [AFTER_MAKE]);
    SM__system_method_name [AFTER_MODIFY] =
      rul__mol_make_symbol ("$AFTER-MODIFY");
    rul__mol_mark_perm (SM__system_method_name [AFTER_MODIFY]);
    SM__system_method_name [ON_ATTR_MODIFY] =
      rul__mol_make_symbol ("$ON-ATTR-MODIFY");
    rul__mol_mark_perm (SM__system_method_name [ON_ATTR_MODIFY]);
    SM__system_method_name [ON_COMP_ATTR_MODIFY] =
      rul__mol_make_symbol ("$ON-COMP-ATTR-MODIFY");
    rul__mol_mark_perm (SM__system_method_name [ON_COMP_ATTR_MODIFY]);
    SM__system_method_name [NO_METHOD_FOUND] =
      rul__mol_make_symbol ("$NO-METHOD-FOUND");
    rul__mol_mark_perm (SM__system_method_name [NO_METHOD_FOUND]);
  }
}

long rul__decl_is_system_method (Mol_Symbol meth_name)
{
  long i;

  decl_init_sys_meth_mols ();

  for (i = 0; i < SYSTEM_METHODS; i++) {
    if (meth_name == SM__system_method_name [i])
      return (1l << i);
  }
  return 0;
}

Mol_Symbol rul__decl_get_sys_meth_name (System_Method_Type met_type)
{
  return SM__system_method_name [met_type];
}


void rul__decl_define_sys_methods (Decl_Block block)
{
  long       i, par_cnt;
  Method     meth;

  decl_init_sys_meth_mols ();

  if (!SB__gen_sys_methods_defined) {
    SB__gen_sys_methods_defined = TRUE;

    for (i = 0; i < SYSTEM_METHODS; i++) {
      par_cnt = 2;
      if (i == 5 || i == 6)
	par_cnt = 4;
      meth = rul__decl_create_method (block, SM__system_method_name [i],
				      FALSE, par_cnt);
      rul__decl_set_method_param (0, shape_atom, dom_instance_id, NULL);
      rul__decl_set_method_param (1, shape_atom, dom_symbol, NULL);
      rul__decl_set_method_param (2, shape_atom, dom_any, NULL);
      if (i == 5 || i == 6) {
	rul__decl_set_method_param (2, shape_atom, dom_symbol, NULL);
	if (i == 5)
	  rul__decl_set_method_param (3, shape_atom, dom_any, NULL);
	else
	  rul__decl_set_method_param (3, shape_compound, dom_any, NULL);
	rul__decl_set_method_param (4, shape_atom, dom_any, NULL);
      }
    }
  }
}

