/* Hash table package */
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
 * FACILITY:
 *	RULEWORKS compiler/run time system
 *
 * ABSTRACT:
 *	This module provides functions for creating and accessing arbitrarily
 *	sized hash tables keyed by Molecules.
 *
 * AUTHOR:
 *	DEC	Digital Equipment Corporation
 *	CPQ	Compaq Computer Corporation
 *
 * MODIFICATION HISTORY:
 *
 *	20-Sep-1990	DEC	Initial version
 *
 *	 6-Nov-1990	DEC	Prefix global names with rul__
 *
 *	14-Nov-1990	DEC	Renamed rul__dump_hash_table to
 *					rul__for_each_hash_entry (generic iterator
 *					function).
 *
 *	 7-Aug-1991	DEC	Change hash function to strip atom's tag bits
 *					and to always return positive result.
 *
 *	 3-Oct-1991	DEC	Added rul__delete_hash_table.
 *
 *	 5-Aug-1992	DEC	V5 changes.
 *
 *	 5-Mar-1993	DEC	Fix freeing of bucket chains.  Switch to the
 *					struct w/inline array style used elsewhere.
 *
 *	01-Dec-1999	CPQ	Release with GPL
 */

#include <common.h>
#include <hash.h>
#include <mol.h>		/* For rul__mol_to_hash_num */

typedef struct hash_chain {
  Molecule	symbol;		/* The key being hashed */
  void		*object;	/* The structure being stored */
  struct hash_chain
		*next;		/* Next entry in this bucket */
} Hash_Chain;

typedef Hash_Chain *Bucket;	/* A bucket is a pointer to a hash chain */

struct hash_table {		/* "typedef"ed in common.h */
  int		size;		/* Number of buckets in this table */
  Bucket	buckets[1];	/* Array of buckets (allocated larger than 1) */
};


/* Forward private declarations */
static unsigned long
hash (Molecule key, int number_of_buckets);

static void
delete_chain (Hash_Chain *chain);


Hash_Table
rul__hash_create_table (int number_of_buckets)
{
    Hash_Table ht;

    assert (number_of_buckets > 1);

    /* Allocate the Hash_Table and the buckets it points to
    ** as one big chunk.
    */
    ht = (Hash_Table) 
	 rul__mem_calloc(1, sizeof (struct hash_table) +
			      ((number_of_buckets - 1) * sizeof(Bucket)));
    ht->size = number_of_buckets;

    return ht;
}


/* Note that this routine doesn't deallocate objects stored in
   the hash table.  rul_for_each_hash_entry can be used for that. */
void
rul__hash_delete_table (Hash_Table table)
{
    int i;

    for (i = 0; i < table->size; i++)
	delete_chain (table->buckets[i]);
    rul__mem_free (table);
}


/*
 * rul__hash_get_entry
 *
 * Return a pointer the object stored in the given table with the given key.
 * If the key isn't found in the table, return NULL.
 */
void *
rul__hash_get_entry (const Hash_Table table, const Molecule key)
{
    Hash_Chain *hc;

    for (hc = table->buckets[hash (key, table->size)];
	 /* hc starts as first entry in the bucket */
	 hc != NULL;		/* loop until end of chain */
	 hc = hc->next) {
	if (hc->symbol == key)
	    return hc->object;	/* Got it, return pointer to the object */
    }
    return NULL;		/* hc is NULL; key isn't in table */
}


/*
 * rul__hash_add_entry
 *
 * Stores object with given key in table.  If key is already in table,
 * just returns NULL.
 */
void *
rul__hash_add_entry (Hash_Table table, Molecule key, void *object)
{
    Hash_Chain *hc, *new_chain_link;
    unsigned long hash_index;

    hash_index = hash (key, table->size);
    for (hc = table->buckets[hash_index];
	 /* hc starts as first entry in the bucket */
	 hc != NULL;		/* loop until end of chain */
	 hc = hc->next) {
	if (hc->symbol == key)
	    return NULL;	/* Oops, it's already there; return NULL */
    }
    new_chain_link = RUL_MEM_ALLOCATE (Hash_Chain, 1);
    new_chain_link->symbol = key;
    rul__mol_incr_uses (key);
    new_chain_link->object = object;
    new_chain_link->next = table->buckets[hash_index];
    table->buckets[hash_index] = new_chain_link;
    return object;
}


/*
 * rul__hash_replace_entry
 *
 * Stores object with given key in table.  If key is already in table,
 * replaces the value associated with that key.
 */
void
rul__hash_replace_entry (Hash_Table table, Molecule key, void *object)
{
    Hash_Chain *hc, *new_chain_link;
    unsigned long hash_index;

    hash_index = hash (key, table->size);
    for (hc = table->buckets[hash_index];
	 /* hc starts as first entry in the bucket */
	 hc != NULL;		/* loop until end of chain */
	 hc = hc->next) {

	if (hc->symbol == key) {
	    /*
	     * There was an entry for this key, so replace it.
	     */
	    hc->object = object;
	    return;
	}
    }
    /*
     * There was no entry for this key, so add it.
     */
    new_chain_link = RUL_MEM_ALLOCATE (Hash_Chain, 1);
    new_chain_link->symbol = key;
    rul__mol_incr_uses (key);
    new_chain_link->object = object;
    new_chain_link->next = table->buckets[hash_index];
    table->buckets[hash_index] = new_chain_link;
}

/*
 * rul__hash_remove_entry
 *
 * Removes entry with given key in table.
 */
extern void
rul__hash_remove_entry (Hash_Table table, Mol_Symbol key)
{
  Hash_Chain *hc, *phc = NULL;
  unsigned long hash_index;

  hash_index = hash (key, table->size);
  for (hc = table->buckets[hash_index];  hc != NULL;
       phc = hc, hc = hc->next) {

    if (hc->symbol == key) {
      /*
       * There was an entry for this key, so remove it.
       */
      if (phc)
	phc->next = hc->next;
      else
	table->buckets[hash_index] = NULL;
      rul__mol_decr_uses (hc->symbol);
      rul__mem_free (hc);
      return;
    }
  }
}


/*
 * rul__hash_copy_each_entry
 *
 *  Iterates over the given source hash table, 
 *  copying each entry into the target hash table.
 */
void
rul__hash_copy_each_entry (Hash_Table target_table, Hash_Table source_table)
{
    int i;
    Hash_Chain *hc;

    for (i = 0; i < source_table->size; i++) {
        for (hc = source_table->buckets[i]; hc != NULL; hc = hc->next) {
	    rul__hash_add_entry (target_table, hc->symbol, hc->object);
        }
    }
}


/*
 * rul__hash_for_each_entry
 *
 * Iterates over the given hash table, calling the routine for each entry,
 * passing the entry as a parameter to the routine.
 */
void
rul__hash_for_each_entry (Hash_Table table, void (*routine)(void *))
{
    int i;
    Hash_Chain *hc;

    for (i = 0; i < table->size; i++) {
	for (hc = table->buckets[i]; hc != NULL; hc = hc->next) {
	    (*routine)(hc->object);
	}
    }
}



/*
 * Private functions
 */

/*
 * hash
 *
 * Returns a relatively evenly distributed "random" number between
 * 0 and number_of_buckets-1.  For a given set of input, it always
 * returns the same output.
 */
static unsigned long
hash (Molecule key, int number_of_buckets)
{
    return rul__mol_to_hash_num (key) % number_of_buckets;
}


/* Iteratively delete links in a hash_chain. */
static void
delete_chain (Hash_Chain *chain)
{
    Hash_Chain *ch, *tmp;

    ch = chain;
    while (ch) {
	tmp = ch->next;
        rul__mol_decr_uses (ch->symbol);
	rul__mem_free (ch);
	ch = tmp;
    }
}
