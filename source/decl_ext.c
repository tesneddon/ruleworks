/* RULEWORKS external routine declarations */
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
 *	Declaration routines.  Includes routines to support compile time
 *	external routine data structures 
 *
 *  MODIFIED BY:
 *	DEC	Digital Equipment Corporation
**	CPQ	Compaq Computer Corporation
 *
 *  MODIFICATION HISTORY:
 *
 *	28-Aug-1992	DEC	Initial version.
 *	01-Dec-1999	CPQ	Release with GPL
 */

#include <common.h>
#include <decl_e_p.h>
#include <decl.h>
#include <hash.h>
#include <mol.h>
#include <list.h>

#define EXT_ALIAS_HT_SIZE 37

/* Static local Variables for the current declaration */

static	Ext_Rt_Decl	current_routine = NULL;
static	Param_Decl	current_param = NULL;
static  Hash_Table      ext_alias_ht = NULL;

/* Incremental External Routine Constructors */

void rul__decl_create_ext_rt (Mol_Symbol func_name)
{
  /*
   * This routine assumes that the current declaration block has been set
   * and that the current declaration block does not contain a
   * declaration for this routine (and so the compile time data structures
   * for it need to be created).
   *
   * It allocates memory for an ext rt declaration struct, which then become
   * the "current external routine".
   * This routine sets the defaults (alias for itself, zero parameters,
   * no return value...)
   *
   * the ext rt declaration struct is not added to the current decl block
   */

  Decl_Block block = rul__decl_get_curr_decl_block ();

  current_routine = RUL_MEM_ALLOCATE (struct ext_rt_decl, 1);

  current_routine->next = rul__decl_get_block_routines (block);
  current_routine->name = func_name;
  current_routine->block = block;
  current_routine->alias_for = func_name;
  current_routine->num_params = 0;
  current_routine->num_by_ref = 0;
  current_routine->return_value = FALSE;
  current_routine->input_params = NULL;
  current_routine->return_param = NULL;
  rul__mol_incr_uses (current_routine->name);
  rul__mol_incr_uses (current_routine->alias_for);
}

void destroy_ext_rt_param (Param_Decl param)
{
  if (param->name != NULL)
    rul__mol_decr_uses (param->name);
  if (param->arg != NULL)
    rul__mol_decr_uses (param->arg);
  rul__mem_free(param);
}

void rul__decl_destroy_ext_rt (Ext_Rt_Decl ext_rt)
{
  Param_Decl param, next_param;

  param = ext_rt->input_params;
  while (param != NULL) {
    next_param = param->next;
    destroy_ext_rt_param (param);
    param = next_param;
  }

  if (ext_rt->return_param != NULL)
    destroy_ext_rt_param (ext_rt->return_param);

  rul__mol_decr_uses (ext_rt->name);
  rul__mol_decr_uses (ext_rt->alias_for);
  rul__mem_free (ext_rt);

  if (ext_rt == current_routine) {
    current_routine = NULL;
    current_param = NULL;
  }
}

void rul__decl_destroy_ext_rtns (Ext_Rt_Decl ext_rt)
{
  Ext_Rt_Decl next_ext_rt;

  while (ext_rt != NULL) {
    next_ext_rt = ext_rt->next;
    rul__decl_destroy_ext_rt (ext_rt);
    ext_rt = next_ext_rt;
  }
}

void rul__decl_add_ext_rt_alias (Mol_Symbol alias_name)
{
/*
 * This routine sets the alias slot of the current external routine
 * declaration.
 */
    rul__mol_decr_uses (current_routine->alias_for);
    current_routine->alias_for = alias_name;
    rul__mol_incr_uses (current_routine->alias_for);
}

void rul__decl_add_ext_rt_param (Ext_Type data_type,
				 Cardinality array_len,
				 Ext_Mech mech,
				 Mol_Symbol param_name,
				 Mol_Symbol opt_arg)
{
  /*
   * This routine creates a parameter declaration and adds it to the list
   * of input params in the current external routine declaration.  It also
   * check that more than the specified number of input args has not been 
   * specified.
   */

  if (current_param == NULL) {
    current_param = RUL_MEM_ALLOCATE (struct param_decl, 1);
    current_routine->input_params = current_param;
  }
  else {
    current_param->next = RUL_MEM_ALLOCATE (struct param_decl, 1);
    current_param = current_param->next;
  }

  current_param->next = NULL;
  current_param->type = data_type;
  current_param->len  = array_len;
  current_param->mech = mech;
  current_param->name = param_name;
  current_param->arg  = opt_arg;

  current_routine->num_params = current_routine->num_params + 1;

  if (mech == ext_mech_ref_rw || mech == ext_mech_ref_ro) {
    current_param->stack_index = current_routine->num_by_ref * 2;
    current_routine->num_by_ref = current_routine->num_by_ref + 1;
  }
  else
    current_param->stack_index = 0;

  if (param_name != NULL)
    rul__mol_incr_uses (param_name);
  if (opt_arg != NULL)
    rul__mol_incr_uses (opt_arg);
}

void rul__decl_add_ext_rt_ret_val (Ext_Type     type,
				   Cardinality  array_len,
				   Ext_Mech     mech,
				   Mol_Symbol   opt_arg)
{
  /*
   * This routine creates a parameter declaration adds it to the current
   * external routine declaration and set thge return value flag to true.
   */

  current_param = RUL_MEM_ALLOCATE (struct param_decl, 1);
  current_routine->return_param = current_param;

  current_param->next = NULL;
  current_param->type = type;
  current_param->len  = array_len;
  current_param->mech = mech;
  current_param->arg  = opt_arg;
  if (opt_arg != NULL)
    rul__mol_incr_uses (opt_arg);
  current_routine->return_value = TRUE;
  current_param = NULL;
}

Ext_Rt_Decl rul__decl_get_current_rt (void)
{
  return current_routine;
}

void rul__decl_finish_ext_rt (void)
{
  /*
   * This routine adds the current external routine to the list of
   * external routines in the current declaration block and
   * the list of external routines visible to the current entry block.
   * it also clears the current external routine decl pointers
   *
   * It also adds the alias name to the alias hash table. This is used
   * only by the compiler for routine declarations.
   */

  Ext_Alias_Decl alias, prev_alias;
  Decl_Block     block = rul__decl_get_curr_decl_block ();

  if (current_routine != NULL) {

    /* register this routine with the current declaring block */
    rul__decl_set_block_routines (block, current_routine);
    rul__hash_add_entry (rul__decl_get_block_routine_ht (block),
			 current_routine->name, current_routine);
    
    /* register this routine with the current entry block if any */
    if (rul__decl_get_entry_block_name () != NULL)
      rul__hash_add_entry (rul__decl_get_entry_block_rt_ht (),
			   current_routine->name, current_routine);
    
    /* create the alias hash table */
    if (ext_alias_ht == NULL)
      ext_alias_ht = rul__hash_create_table (EXT_ALIAS_HT_SIZE);
    
    /* create the alias struct and set pointer to ext_rt_decl */
    alias = RUL_MEM_ALLOCATE (struct ext_alias_decl, 1);
    alias->func = current_routine;
    
    /* find previous and link in current alias in filo order */
    prev_alias = (Ext_Alias_Decl) rul__hash_get_entry (ext_alias_ht,
					       current_routine->alias_for);
    if (prev_alias == NULL) {
      alias->next = 0;
      rul__hash_add_entry (ext_alias_ht, current_routine->alias_for, alias);
    }
    else {
      alias->next = prev_alias;
      rul__hash_replace_entry (ext_alias_ht,
			       current_routine->alias_for, alias);
    }
  }
  current_routine = NULL;
  current_param = NULL;
}

Ext_Rt_Decl rul__decl_get_next_ext_rt (Ext_Rt_Decl ext_rt)
{
  return ext_rt->next;
}

Ext_Alias_Decl rul__decl_get_next_alias (Ext_Alias_Decl alias)
{
  return alias->next;
}


/* External Alias Routine Information */

Hash_Table rul__decl_get_alias_ht (void)
{
  return ext_alias_ht;
}

void rul__decl_delete_alias_ht (void)
{
  rul__hash_delete_table (ext_alias_ht);
  ext_alias_ht = NULL;
}



/* Querying External Routine Information */

Boolean rul__decl_is_an_ext_rt (Mol_Symbol func)
{
  /*
   * This routine returns true and sets the "current external routine"
   * when an external routine declaration exists for a given routine
   * name in the current declaration block.  It returns false when
   * no external routine declaration currently exists...
   */

  current_routine = rul__hash_get_entry (rul__decl_get_block_routine_ht
					 (rul__decl_get_curr_decl_block()),
					 func);
  current_param = NULL;
  
  if (current_routine == NULL)
    return FALSE;
  else
    return TRUE;
}

Ext_Rt_Decl rul__decl_get_ext_rt_decl (Decl_Block block, Mol_Symbol func)
{
  /* 
   * This routine returns the external routine declaration structure 
   * for the specified routine in the specified declaration block.
   */
  return rul__hash_get_entry(rul__decl_get_block_routine_ht(block), func);
}

Ext_Rt_Decl rul__decl_get_visible_ext_rt (Mol_Symbol func)
{
  List	        db_list_iterator;
  Decl_Block	db;
  Ext_Rt_Decl   ext_rt;
  
  /*
   * Search the visible declaration blocks for the specified function.
   * If found, this routine returns the external routine declaration,
   * else it returns NULL.
   */

  for (db_list_iterator = rul__decl_get_visible_decl_blks();
       !rul__list_is_empty (db_list_iterator);
       db_list_iterator = rul__list_rest (db_list_iterator)) {

    db = rul__decl_get_block (rul__list_first (db_list_iterator));
    ext_rt = rul__decl_get_ext_rt_decl (db, func);
    if (ext_rt)
      return ext_rt;
  }

  return NULL;
}

Boolean rul__decl_ext_rts_are_identical (Ext_Rt_Decl rt1, Ext_Rt_Decl rt2)
{
  /* This routine compares the given external routine declarations
   * If they are identical, this routine returns true, else it returns false.
   */
  
  Param_Decl param1, param2;
  long i;
  
  if ((rt1->alias_for    != rt2->alias_for)  ||
      (rt1->num_params   != rt2->num_params) ||
      (rt1->num_by_ref   != rt2->num_by_ref) ||
      (rt1->return_value != rt2->return_value))
    return FALSE;

  if ((rt1->return_value) &&
      ((rt1->return_param->type != rt2->return_param->type) ||
       (rt1->return_param->len  != rt2->return_param->len)  ||
       (rt1->return_param->mech != rt2->return_param->mech)))
    return FALSE;
  
  param1 = rt1->input_params;
  param2 = rt2->input_params;
  for (i = rt1->num_params; i > 0; i--) {
    assert (param1 != NULL);
    assert (param2 != NULL);
    if ((param1->type != param2->type) ||
	(param1->len  != param2->len)  ||
	(param1->arg  != param2->arg)  ||
	(param1->mech != param2->mech))
      return FALSE;
    else {
      param1 = param1->next;
      param2 = param2->next;
    }
  }
  return TRUE;
}


/*
 * The following routines hide the structure of the external routine
 * declaration data structure (Ext_Rt_Decl).
 */

Mol_Symbol rul__decl_ext_rt_name (Ext_Rt_Decl ext_rt)
{
  return ext_rt->name;
}

Mol_Symbol rul__decl_ext_rt_alias (Ext_Rt_Decl ext_rt)
{
  return ext_rt->alias_for;
}

Decl_Block rul__decl_ext_rt_decl_blk (Ext_Rt_Decl ext_rt)
{
  return ext_rt->block;
}

Ext_Rt_Decl rul__decl_ext_rt_next_routine (Ext_Rt_Decl ext_rt)
{
  return ext_rt->next;
}

long rul__decl_ext_rt_num_args (Ext_Rt_Decl ext_rt)
{
  return (ext_rt->num_params); 
}

long rul__decl_ext_rt_by_ref (Ext_Rt_Decl ext_rt)
{
  return (ext_rt->num_by_ref);
}

long rul__decl_ext_rt_param_index (Ext_Rt_Decl ext_rt, Mol_Symbol param_name)
{
  Param_Decl param = ext_rt->input_params;
  long       i;

  for (i = 1; param; param = param->next, i++) {
    if (param->name == param_name)
      return i;
  }
  return 0;
}

Ext_Type rul__decl_ext_rt_param_type (Ext_Rt_Decl ext_rt, long param_index)
{
  Param_Decl param = ext_rt->input_params;
  long i;

  for (i=1; i<param_index; i++) {
    param = param->next;
  }
  return (param->type);
}

Ext_Mech rul__decl_ext_rt_param_mech (Ext_Rt_Decl ext_rt, long param_index)
{
  Param_Decl param = ext_rt->input_params;
  long i;
  
  for (i=1; i<param_index; i++) {
    param = param->next;
  }
  return (param->mech);
}

Cardinality rul__decl_ext_rt_param_a_len (Ext_Rt_Decl ext_rt, long param_index)
{
  Param_Decl param = ext_rt->input_params;
  long i;

  for (i=1; i<param_index; i++) {
    param = param->next;
  }
  return (param->len);
}

long rul__decl_ext_rt_param_s_index (Ext_Rt_Decl ext_rt, long param_index)
{
  Param_Decl param = ext_rt->input_params;
  long i;

  for (i=1; i<param_index; i++) {
    param = param->next;
  }
  return (param->stack_index);
}

Mol_Symbol rul__decl_ext_rt_param_name (Ext_Rt_Decl ext_rt, long param_index)
{
  Param_Decl param = ext_rt->input_params;
  long i;

  for (i=1; i<param_index; i++) {
    param = param->next;
  }
  return (param->name);
}

Mol_Symbol rul__decl_ext_rt_param_a_arg (Ext_Rt_Decl ext_rt, long param_index)
{
  Param_Decl param = ext_rt->input_params;
  long i;

  for (i=1; i<param_index; i++) {
    param = param->next;
  }
  return (param->arg);
}

Boolean rul__decl_ext_rt_ret_val (Ext_Rt_Decl ext_rt)
{
  return (ext_rt->return_value);
}

Ext_Type rul__decl_ext_rt_ret_type (Ext_Rt_Decl ext_rt)
{
  Param_Decl param = ext_rt->return_param;

  return ((param == NULL) ? ext_type_invalid : (param->type));
}

Ext_Mech rul__decl_ext_rt_mech (Ext_Rt_Decl ext_rt)
{
  Param_Decl param = ext_rt->return_param;

  return ((param == NULL) ? ext_mech_invalid : (param->mech));
}

Cardinality rul__decl_ext_rt_ret_a_len (Ext_Rt_Decl ext_rt)
{
  Param_Decl param = ext_rt->return_param;

  return ((param == NULL) ? EXT__C_NOT_ARRAY : (param->len));
}

Mol_Symbol rul__decl_ext_rt_ret_a_arg (Ext_Rt_Decl ext_rt)
{
  Param_Decl param = ext_rt->return_param;

  return ((param == NULL) ? NULL : (param->arg));
}

void rul__decl_register_ext_rt_in_eb (Ext_Rt_Decl rt)
{
  rul__hash_add_entry(rul__decl_get_entry_block_rt_ht (), rt->name, rt);
}

