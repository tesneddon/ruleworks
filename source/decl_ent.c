/* RULEWORKS entry block declarations */
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
 *	Declaration routines.  Includes Entry blocks.
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
#include <hash.h>
#include <list.h>
#include <mol.h>

#define VISIBLE_ROUTINES_HT_SIZE 101

struct entry_block {
  Mol_Symbol	name;
  Hash_Table	visible_routines;    /* Hash table keying external routines
					names to declaration blocks */
  List		visible_decl_blocks; /* List of names of decl blocks currently
					visible in this entry block */
  Boolean	on_clause_declared[DECL_ON_EXIT+1];
  List		activated_rule_blocks;/*List of names of rule blocks activated*/
  Strategy	implicit_rule_block_strategy;
};

/*
 * Lists of objects
 */

static List		entry_block_list = NULL;

/*
 * Variables which keep track of "current" objects
 */

static Entry_Block	current_entry_block = NULL;


Mol_Symbol
rul__decl_get_entry_block_name (void)
{
  if (current_entry_block)
    return current_entry_block->name;
  else
    return NULL;            /* No entry block */
}

List
rul__decl_get_visible_decl_blks(void)
{
  if (current_entry_block)
    return current_entry_block->visible_decl_blocks;
  else
    return NULL;		/* No entry block */
}

Hash_Table
rul__decl_get_entry_block_rt_ht (void)
{
  if (current_entry_block)
    return current_entry_block->visible_routines;
  else
    return NULL;		/* No entry block */
}

List
rul__decl_get_rule_block_list (void)
{
  if (current_entry_block)
    return current_entry_block->activated_rule_blocks;
  else
    return NULL;		/* No entry block */
}


/*
 * Block visibility
 */

void
rul__decl_set_entry_block (Mol_Symbol entry_block_name)
{
    List	eb_list_iterator;
    Entry_Block	eb;

    /* First time initialization */
    if (entry_block_list == NULL)
	entry_block_list = rul__list_create_list ();

    /* If this entry block already exists, just set current_entry_block */
    for (eb_list_iterator = entry_block_list;
	 !rul__list_is_empty (eb_list_iterator);
	 eb_list_iterator = rul__list_rest (eb_list_iterator)) {
	eb = rul__list_first (eb_list_iterator);
	if (eb->name == entry_block_name)
	    current_entry_block = eb;
	}

    /* If entry block didn't already exist, create new one */
    if (rul__list_is_empty (eb_list_iterator)) {
	current_entry_block = RUL_MEM_ALLOCATE (struct entry_block, 1);
	current_entry_block->name = entry_block_name;
	rul__mol_incr_uses (entry_block_name);
	current_entry_block->visible_routines = 
		rul__hash_create_table (VISIBLE_ROUTINES_HT_SIZE);
	current_entry_block->visible_decl_blocks = rul__list_create_list ();
	current_entry_block->activated_rule_blocks = rul__list_create_list ();
	}
}

void
rul__decl_init_block_visibility (void)
{
    current_entry_block->name = NULL;
    rul__list_delete_list (current_entry_block->visible_decl_blocks);
    current_entry_block->visible_decl_blocks = rul__list_create_list ();
    rul__hash_delete_table (current_entry_block->visible_routines);
    current_entry_block->visible_routines =
                rul__hash_create_table (VISIBLE_ROUTINES_HT_SIZE);
}

void
rul__decl_make_block_visible (Mol_Symbol decl_block_name)
{
  rul__list_add_last (current_entry_block->visible_decl_blocks,
		      decl_block_name);
  rul__mol_incr_uses (decl_block_name);
}

void
rul__decl_eb_on_clause_was_seen (On_Clause_Type on_clause_seen)
{
  current_entry_block->on_clause_declared[on_clause_seen] = TRUE;
}

Boolean
rul__decl_eb_was_on_clause_seen (On_Clause_Type on_clause_seen)
{
  return current_entry_block->on_clause_declared[on_clause_seen];
}

void
rul__decl_add_rule_block_to_eb (Mol_Symbol rule_block_name)
{
  rul__list_add_last (current_entry_block->activated_rule_blocks,
		      rule_block_name);
  rul__mol_incr_uses (rule_block_name);
}

void
rul__decl_free_entry_block (Mol_Symbol entry_block_name)
{
  List	eb_list_iterator;
  Entry_Block	eb;
  List	block_list;
  Mol_Symbol	block_name;

  if (entry_block_list == NULL)
    return;

  for (eb_list_iterator = entry_block_list;
       !rul__list_is_empty (eb_list_iterator);
       eb_list_iterator = rul__list_rest (eb_list_iterator)) {
    eb = rul__list_first (eb_list_iterator);
    if (eb->name == entry_block_name) {
      rul__mol_decr_uses (entry_block_name);

      for (block_list = eb->visible_decl_blocks;
	   !rul__list_is_empty (block_list);
	   block_list = rul__list_rest (block_list)) {
	block_name = rul__list_first (block_list);
	rul__mol_decr_uses (block_name);
      }
      rul__list_delete_list (block_list);

      for (block_list = eb->activated_rule_blocks;
	   !rul__list_is_empty (block_list);
	   block_list = rul__list_rest (block_list)) {
	block_name = rul__list_first (block_list);
	rul__mol_decr_uses (block_name);
      }
      rul__list_delete_list (block_list);

      rul__mem_free (eb);

      rul__list_remove_first (eb_list_iterator);
    }
  }
}

void rul__decl_set_eb_strategy (Strategy eb_strategy)
{
  current_entry_block->implicit_rule_block_strategy = eb_strategy;
}

Strategy rul__decl_get_eb_strategy (void)
{
  return current_entry_block->implicit_rule_block_strategy;
}

