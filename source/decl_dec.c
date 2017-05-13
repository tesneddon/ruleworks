/* RULEWORKS declaration block declarations */
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
 *	RULEWORKS run time system and compiler
 *
 *  ABSTRACT:
 *	Declaration routines.  Includes Declaration blocks.
 *	Error checking must be done by the callers of this subsystem.
 *
 *  MODIFIED BY:
 *	DEC	Digital Equipment Corporation
**	CPQ	Compaq Computer Corporation
 *
 *  MODIFICATION HISTORY:
 *
 *	25-Nov-1992	DEC	Initial version
 *	01-Dec-1999	CPQ	Release with GPL
 */

#include <common.h>
#include <decl.h>
#include <mol.h>
#include <hash.h>
#include <list.h>
#include <ctype.h>


#define CLASS_HT_SIZE 101	/* Expected number of classes per block  */
#define ROUTINE_HT_SIZE 101	/* Expected number of routines per block */

struct decl_block {
  Decl_Block	next;		/* Global List of ALL declaration blocks*/
  Mol_Symbol	name;		/* Name of the declaration block	*/
  char		*short_name;	/* 8 char name for index file name      */
  Ext_Rt_Decl	routines;	/* List of external routine declarations*/
  Hash_Table    routine_ht;	/* Hash Table of ext. routine declarations*/
  Hash_Table	method_ht;	/* Hash table of this block's methods	*/
  long     	method_count;	/* count of methods this block     	*/
  Class		classes;	/* List of classes in order declared	*/
  Class		classes_end;	/* Pointer to last class in list	*/
  Hash_Table	class_ht;	/* Hash table of this block's classes	*/
  List		wmo_list;	/* Working Memory Object list		*/
  long		level_count;	/* depth of the inheritance tree	*/
  long		*sib_count_array;/*count of siblings at each level	*/
  long		sib_array_len;	/* current size of sib_count_array	*/
  long		*bits_per_level;/* number of bits needed to represent
				   the inheritance codes at each level	*/
  long		total_bits;	/* the total number of bits needed	*/
  long		code_size;	/* number of longwords needed to store
				   inheritance patterns and masks	*/
};

/* List of all declaration blocks in order created. */

static Decl_Block	decl_block_list = NULL;
static Decl_Block	decl_block_list_end = NULL; /* Last block in list */

/*
 * Variable which keeps track of "current" declaration block object
 */

static Decl_Block	current_decl_block;

/*
 * Declaring blocks
 */

Decl_Block
rul__decl_get_curr_decl_block (void)
{
    return current_decl_block;
}

void
rul__decl_set_curr_decl_block (Decl_Block curr_decl_blk)
{
  current_decl_block = curr_decl_blk;
}

void
rul__decl_create_decl_block (Mol_Symbol block_name)
{
    current_decl_block = RUL_MEM_ALLOCATE (struct decl_block, 1);

    assert (rul__decl_get_block (block_name) == NULL);

    current_decl_block->next = NULL; 
    current_decl_block->name = block_name;
    rul__mol_incr_uses (block_name);
    current_decl_block->short_name =
      rul__decl_make_decl_short_name (block_name);
    current_decl_block->routines = NULL;
    current_decl_block->routine_ht = rul__hash_create_table (ROUTINE_HT_SIZE);
    current_decl_block->method_ht = rul__hash_create_table (METHOD_HT_SIZE);
    current_decl_block->method_count = 0;
    current_decl_block->classes = NULL;
    current_decl_block->classes_end = NULL;
    current_decl_block->class_ht = rul__hash_create_table (CLASS_HT_SIZE);
    current_decl_block->wmo_list = rul__list_create_list ();

    /* Add this block to end of list of all blocks. */
    if (decl_block_list == NULL) /* Empty list */
	decl_block_list = current_decl_block;
    else			/* At least one block already there */
	decl_block_list_end->next = current_decl_block;
    decl_block_list_end = current_decl_block;

    rul__decl_create_root_class (block_name);
}

Decl_Block
rul__decl_destroy_decl_block (Decl_Block decl_block)
{
  Decl_Block next_decl_block;
  extern Class rul__decl_destroy_class (Class class_ptr);
  extern void rul__decl_destroy_ext_rtns (Ext_Rt_Decl ext_rt);
  Class class_ptr;

  next_decl_block = decl_block->next;
  rul__mol_decr_uses (decl_block->name);
  rul__mem_free(decl_block->short_name);
  rul__decl_destroy_ext_rtns (decl_block->routines);
  rul__hash_delete_table(decl_block->routine_ht);
  rul__hash_for_each_entry (decl_block->method_ht,
			    (void (*) (void *)) rul__decl_destroy_methods);
  rul__hash_delete_table(decl_block->method_ht);
  class_ptr = decl_block->classes;
  while (class_ptr != NULL)
    class_ptr = rul__decl_destroy_class(class_ptr);
  rul__hash_delete_table(decl_block->class_ht);
  rul__list_delete_list (decl_block->wmo_list);
  rul__mem_free(decl_block->sib_count_array);
  rul__mem_free(decl_block->bits_per_level);

  rul__mem_free (decl_block);
  return next_decl_block;
}

void
rul__decl_destroy_decl_blocks (void)
{
  Decl_Block db_ptr;

  db_ptr = decl_block_list;
  while (db_ptr != NULL)
    db_ptr = rul__decl_destroy_decl_block (db_ptr);

  decl_block_list = NULL;
  decl_block_list_end = NULL;
}


void
rul__decl_finish_decl_block (void)
{
    if (current_decl_block != NULL)
	rul__decl_encode_inheritance (current_decl_block->name);
    current_decl_block = NULL;	/* There's no longer a current block */
}


Boolean
rul__decl_is_decl_block (Mol_Symbol block_name)
{
    return rul__decl_get_block (block_name) != NULL;
}

Decl_Block
rul__decl_get_block (Mol_Symbol block_name)
/* This routine gets the named declaration block from the global 
 * list of "known" declaration blocks.
 */
{
    Decl_Block b;

    for (b = decl_block_list;
	 b != NULL && b->name != block_name;
	 b = b->next)
	;
    return b;
}

Decl_Block
rul__decl_get_first_decl_block (void)
/* This routine gets the first declaration block from the global 
 * list of "known" declaration blocks.
 */
{
    return decl_block_list;
}

Decl_Block
rul__decl_get_next_decl_block (Decl_Block block)
/* This routine gets the declaration block after the given one
 * from the global list of "known" declaration blocks.
 */
{
    return block->next;
}



Mol_Symbol
rul__decl_get_decl_name (Decl_Block block)
{
    return block->name;
}

char 
*rul__decl_get_decl_short_name (Decl_Block block)
{
    return block->short_name;
}

Hash_Table
rul__decl_get_block_routine_ht (Decl_Block block)
{
    return block->routine_ht;
}

void
rul__decl_set_block_routines (Decl_Block block, Ext_Rt_Decl rt)
{
    block->routines = rt;
}

Ext_Rt_Decl
rul__decl_get_block_routines (Decl_Block block)
{
    return block->routines;
}

Hash_Table
rul__decl_get_block_method_ht (Decl_Block block)
{
    return block->method_ht;
}

long
rul__decl_get_block_method_cnt (Decl_Block block)
{
    return ++block->method_count;
}

Boolean 
rul__decl_block_has_classes (Mol_Symbol block_name)
{
    Decl_Block block = rul__decl_get_block (block_name);

    if (rul__decl_get_block_classes (block) ==
	rul__decl_get_block_classes_end (block))
	/* only the root class defined */
	return FALSE;
    else
	return TRUE;
}

void
rul__decl_set_block_classes (Decl_Block block, Class cl)
{
    block->classes = cl;
}

Class
rul__decl_get_block_classes (Decl_Block block)
{
    return block->classes;
}

void
rul__decl_set_block_classes_end (Decl_Block block, Class cl)
{
    block->classes_end = cl;
}

Class
rul__decl_get_block_classes_end (Decl_Block block)
{
    return block->classes_end;
}

Hash_Table
rul__decl_get_block_class_ht (Decl_Block block)
{
    return block->class_ht;
}

List
rul__decl_get_block_wmo_list (Decl_Block block)
{
    return block->wmo_list;
}

void
rul__decl_set_block_level_count (Decl_Block block, long value)
{
    block->level_count = value;
}

long
rul__decl_get_block_level_count (Decl_Block block)
{
    return block->level_count;
}

void
rul__decl_set_block_sca (Decl_Block block, long *value)
{
    block->sib_count_array = value;
}

void
rul__decl_set_block_sca_elt (Decl_Block block, long index, long value)
{
    block->sib_count_array[index] = value;
}

long *
rul__decl_get_block_sca (Decl_Block block)
{
    return block->sib_count_array;
}

long
rul__decl_get_block_sca_elt (Decl_Block block, long index)
{
    return block->sib_count_array[index];
}

void
rul__decl_set_block_sib_a_len (Decl_Block block, long value)
{
    block->sib_array_len = value;
}

long
rul__decl_get_block_sib_a_len (Decl_Block block)
{
    return block->sib_array_len;
}

void
rul__decl_set_block_bpl (Decl_Block block, long *value)
{
    block->bits_per_level = value;
}

void
rul__decl_set_block_bpl_elt (Decl_Block block, long index, long value)
{
    block->bits_per_level[index] = value;
}

long *
rul__decl_get_block_bpl (Decl_Block block)
{
    return block->bits_per_level;
}

long
rul__decl_get_block_bpl_elt (Decl_Block block, long index)
{
    return block->bits_per_level[index];
}

void
rul__decl_set_block_total_bits (Decl_Block block, long value)
{
    block->total_bits = value;
}

long
rul__decl_get_block_total_bits (Decl_Block block)
{
    return block->total_bits;
}

void
rul__decl_set_block_code_size (Decl_Block block, long value)
{
    block->code_size = value;
}

long
rul__decl_get_block_code_size (Decl_Block block)
{
    return block->code_size;
}




char *rul__decl_make_decl_short_name (Mol_Symbol block_name)
{
    char *name = (char *) rul__mem_malloc (9);
    char blk_nam[RUL_C_MAX_READFORM_SIZE + 1];
    int  i, name_len;

    rul__mol_use_printform (block_name, blk_nam, sizeof(blk_nam));

    /*
     **  select 8 chars for the filename and id string from the block name
     */
    for (i = 0, name_len = 0; name_len < 8; i++)
        {
        if (blk_nam[i] == '\0')
            break;

	if ((blk_nam[i] >= 'A' && blk_nam[i] <= 'Z') ||
	    (blk_nam[i] >= '0' && blk_nam[i] <= '9') ||
	    (blk_nam[i] >= 'a' && blk_nam[i] <= 'z') ||
	    (blk_nam[i] == '_' && name_len > 0))
	    {
	    name[name_len++] = tolower (blk_nam[i]);
	    }
        else if (name_len > 0)
	    {			        /* except for the first character  */
            name[name_len++] = '_';	/* replace funny chars with an '_' */
	    }
	else
	    {
	    assert (FALSE); 		/* 1st char should be alphabetic */
	    }
	}
    name[name_len] = '\0';
    return name;
}
