/* cmp_conrg.c - Construct registration subsystem */
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
 *	RULEWORKS compiler
 *
 *  ABSTRACT:
 * The debugger needs to know what constructs are in what blocks and
 * rule-groups at runtime.
 *
 * The compiler needs to enforce the restriction that no two constructs
 * within a block have the same name (for RuleBase).
 *
 * The compiler can be helpful by enforcing the restriction that no two
 * blocks can have the same name.
 *
 * We satisfy all of these requirements by "registering" each
 * construct as it's encountered.
 *
 * When a block is registered, all constructs in the previous block disappear.
 * Entry_block, decl_block, and rule_block constructs remain (and will
 * continue to be returned by get_construct_*).  get_next_construct returns
 * constructs for the current block only.
 *
 * Molecule reference counts for names of constructs and blocks are maintained
 * by this module.
 *
 *  MODIFIED BY:
 *	DEC	Digital Equipment Corporation
 *	CPQ	Compaq Computer Corporation
 *
 *  MODIFICATION HISTORY:
 *	14-Jan-1993	DEC	Initial version
 *	22-Jan-1993	DEC	Clarify names
 *	01-Dec-1999	CPQ	Release with GPL
 */

#include <ctype.h>
#include <common.h>
#include <cmp_comm.h>
#include <hash.h>
#include <conrg.h>
#include <mol.h>		/* To incr/decr reference counts */


#ifdef __MSDOS
#define CONSTRUCT_HT_SIZE 101
#define BLOCK_HT_SIZE 11
#else
#define CONSTRUCT_HT_SIZE 1009
#define BLOCK_HT_SIZE 101
#endif


typedef struct construct {
  struct construct *next;	/* next list entry			*/
  Mol_Symbol	   conrg_name;	/* name of construct	 		*/
  Construct_Type   conrg_type;	/* type of construct			*/
  long		   line_num;	/* Line number where construct starts   */
  long		   block_index;	/* Number of this block		        */
  long             index;       /* construct index                      */
} *Construct;

/*
 * Variables which keep track of "current" objects
 */

static Hash_Table	construct_ht;	/* Hash table of constructs in block */
static Hash_Table	block_ht;    	/* Hash table of blocks		   */
static Construct	conrg_first;	/* First construct in block 	   */
static Construct	conrg_last;	/* Last construct in block 	   */
static Boolean		conrg_init;	/* TRUE if initialized		   */
static Boolean		is_main;	/* TRUE if main eb compiling	   */

static Mol_Symbol	current_block_name;
static Mol_Symbol	current_group_name;
static long		current_block_index;	/* A number for each block */
static long             conrg_count;
static long		conrg_catcher_count;

static void
conrg_free (void *cons)
{
  rul__mol_decr_uses(((Construct) cons)->conrg_name);
  rul__mem_free(cons);
}

/*****************************************************************************/
/* Forget any constructs in the current block. */
static void
conrg_clear_cur_block (void)
{
  Construct cur_construct = conrg_first;
  Construct next_construct;

  /* The first construct on the list is the block, which we don't free. */
  if (cur_construct)
      cur_construct = cur_construct->next;

  while (cur_construct) {
    next_construct = cur_construct->next;
    conrg_free (cur_construct);
    cur_construct = next_construct;
  }

  conrg_first = conrg_last = NULL;

  if (construct_ht)
    rul__hash_delete_table (construct_ht);
  construct_ht = NULL;

  conrg_count  = 0;
  conrg_catcher_count = 0;

  if (current_block_name != rul__mol_symbol_nil ()) {
    rul__mol_decr_uses (current_block_name);
    current_block_name = rul__mol_symbol_nil ();
  }
  if (current_group_name != rul__mol_symbol_nil ()) {
    rul__mol_decr_uses (current_group_name);
    current_group_name = rul__mol_symbol_nil ();
  }
}

void
rul__conrg_clear_all_blocks (void)
{
  if (construct_ht)
    conrg_clear_cur_block();

  if (block_ht) {
    rul__hash_for_each_entry (block_ht, conrg_free);
    rul__hash_delete_table (block_ht);
    block_ht = NULL;
  }
}

/*****************************************************************************/
void
rul__conrg_register_block(Mol_Symbol block_name,
			  Construct_Type block_type,
			  long line_num)
{
    Construct new_block;
    char *block_name_string;

    if (!conrg_init) {	/* if no other blocks have been registered */
      conrg_init = TRUE;
      is_main = FALSE;
      block_ht = rul__hash_create_table (BLOCK_HT_SIZE);
      current_block_name = rul__mol_symbol_nil ();
      current_group_name = rul__mol_symbol_nil ();
    }
    else
      /* Forget info for constructs in previous block. */
      conrg_clear_cur_block ();

    /* create a new hash table for constructs in this block */
    construct_ht = rul__hash_create_table (CONSTRUCT_HT_SIZE);

    /* Allocate a new block structure. */
    new_block = RUL_MEM_ALLOCATE(struct construct, 1);
    new_block->next = NULL;	/* Not used */
    new_block->conrg_name = block_name;
    rul__mol_incr_uses(block_name);
    new_block->conrg_type = block_type;
    new_block->line_num = line_num;
    new_block->block_index = ++current_block_index;
    new_block->index =  ++conrg_count;

    /* Add this block to the block hash table. */
    rul__hash_add_entry(block_ht, block_name, new_block);

    /* Make the block be the first construct in the construct list. */
    conrg_first = conrg_last = new_block;

    current_block_name = block_name;
    rul__mol_incr_uses (block_name);

    /* Is this the main entry-block ? */
    is_main = FALSE;
    block_name_string = rul__mol_get_printform (block_name);

    if (strlen (block_name_string) == 4)
      if ((toupper (block_name_string[0]) == 'M') &&
          (toupper (block_name_string[1]) == 'A') &&
          (toupper (block_name_string[2]) == 'I') &&
	  (toupper (block_name_string[3]) == 'N'))
	is_main = TRUE;

}

/*****************************************************************************/
Boolean
rul__conrg_is_main_block(void)
{
  return is_main;
}

/*****************************************************************************/
void
rul__conrg_register_construct(Mol_Symbol name,
			      Construct_Type ctype,
			      long line_num)
{
  Construct new_construct;

  assert (conrg_last != NULL);	/* The block should be on the construct list */

  /* The construct must not already exist */
  if (ctype != RUL__C_END_GROUP &&
      ctype != RUL__C_END_BLOCK)
    assert(rul__conrg_get_construct_type(name) == RUL__C_NOISE);

  if (ctype == RUL__C_CATCH)
    conrg_catcher_count++;
  
  new_construct = RUL_MEM_ALLOCATE(struct construct, 1);
  new_construct->next = NULL;
  new_construct->conrg_name = name;
  rul__mol_incr_uses(name);
  new_construct->conrg_type = ctype;
  new_construct->line_num = line_num;
  new_construct->block_index = current_block_index;
  new_construct->index = ++conrg_count;

  conrg_last->next = new_construct;
  conrg_last = new_construct;

  /* If this is a named construct, add it to the construct hash table. */
  if (ctype == RUL__C_RULE ||
      ctype == RUL__C_CATCH ||
      ctype == RUL__C_OBJ_CLASS ||
      ctype == RUL__C_EXT_ROUTINE ||
      ctype == RUL__C_RULE_GROUP ||
      ctype == RUL__C_METHOD)
    rul__hash_add_entry(construct_ht, name, new_construct);

  if (ctype == RUL__C_RULE_GROUP) {
    current_group_name = name;
    rul__mol_incr_uses (name);
  }

  if (ctype == RUL__C_END_GROUP) {
    rul__mol_decr_uses (current_group_name);
    current_group_name = rul__mol_symbol_nil ();
  }

}

    
/*****************************************************************************/
Construct_Type
rul__conrg_get_construct_type(Mol_Symbol name)
{
  Construct conrge = rul__hash_get_entry(construct_ht, name);

  if (conrge)
      return conrge->conrg_type;
  else
      return RUL__C_NOISE;
}

/*****************************************************************************/
long
rul__conrg_get_construct_index(Mol_Symbol name)
{
  Construct conrge = rul__hash_get_entry(construct_ht, name);

  if (conrge)
    return conrge->index;
  else if (name == current_block_name) /* why isn't this done for type/line? */
    return 1;
  else
    return 0;
}

/*****************************************************************************/
Construct_Type
rul__conrg_get_block_type(Mol_Symbol name)
{
  Construct block;

  if (!block_ht)
      return RUL__C_NOISE;	/* No blocks exist */
  else {
      block = rul__hash_get_entry(block_ht, name);

      if (block)
	  return block->conrg_type;
      else
	  return RUL__C_NOISE;
  }
}

/*****************************************************************************/
long
rul__conrg_get_construct_line(Mol_Symbol construct_name)
{
    Construct con = rul__hash_get_entry(construct_ht, construct_name);

    assert (con != NULL);		/* It must exist */

    return con->line_num;
}
	
/*****************************************************************************/
long
rul__conrg_get_cur_block_index(void)
{
    return current_block_index;
}

/*****************************************************************************/
Mol_Symbol
rul__conrg_get_cur_block_name(void)
{
    return current_block_name;
}

/*****************************************************************************/
Mol_Symbol
rul__conrg_get_cur_group_name(void)
{
    return current_group_name;
}

/*****************************************************************************/
Boolean
rul__conrg_get_next_construct(void **context,
			      Mol_Symbol *name,
			      Construct_Type *ctype)
{
  Construct con = (Construct) *context;

  if (!con) {
    *context = (void *) conrg_first;
    con = conrg_first;
  }
  else {
    *context = (void *) con->next;
    con = con->next;
  }

  if (con) {
    *name = con->conrg_name;
    *ctype = con->conrg_type;
    return TRUE;
  }
  else
    return FALSE;		/* No more constructs */
}


long
rul__conrg_get_count(void)
{
  return conrg_count;
}

long
rul__conrg_get_catcher_count(void)
{
  return conrg_catcher_count;
}

