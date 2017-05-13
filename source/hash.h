/* hash.h - Hash table package */
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
 *	This file is the external specification for rul_hash.c, which
 *	provides functions for creating and accessing arbitrarily
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
 *	 5-Aug-1992	DEC	V5 changes
 *
 *	 8-Dec-1992	DEC	changed Hash_Table to a pointer to a struct
 *					and moved it to COMMON.H
 *
 *	01-Dec-1999	CPQ	Release with GPL
 */

/*
 * rul__hash_create_table
 *
 * Allocate and return a new hash table of the given size.
 */
extern Hash_Table
rul__hash_create_table(int number_of_buckets);

/*
 * rul__hash_delete_table
 *
 * Delete a previously created hash table.
 */
extern void
rul__hash_delete_table(Hash_Table table);

/*
 * rul__hash_get_entry
 *
 * Return a pointer the object stored in the given table with the given key.
 * If the key isn't found in the table, return NULL.
 */
extern void *
rul__hash_get_entry(const Hash_Table table, const Mol_Symbol key);

/*
 * rul__hash_add_entry
 *
 * Stores address of object with given key in table.  If key is
 * already in table, immediately returns NULL, else returns object.
 */
extern void *
rul__hash_add_entry(Hash_Table table, Mol_Symbol key, void *object);

/*
 * rul__hash_replace_entry
 *
 * Stores address of object with given key in table.  If key is
 * already in table, replaces the value associated with that key.
 */
extern void
rul__hash_replace_entry(Hash_Table table, Mol_Symbol key, void *object);

/*
 * rul__hash_remove_entry
 *
 * Removes entry with given key in table.
 */
extern void
rul__hash_remove_entry(Hash_Table table, Mol_Symbol key);

/*
 * rul__hash_for_each_entry
 *
 * Iterate over hash table calling  routine  for each object in the table.
 */
extern void
rul__hash_for_each_entry(Hash_Table table, void (*routine)(void *));

/*
 * rul__hash_copy_each_entry
 *
 *  Iterates over the given source hash table, 
 *  copying each entry into the target hash table.
 */
extern void
rul__hash_copy_each_entry (Hash_Table target_table, Hash_Table source_table);
