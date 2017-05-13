/* cmp_cons.c - RULEWORKS compiler constants list/table */
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
**                                                                         **
****************************************************************************/


/*
 *  FACILITY:
 *	RULEWORKS run time system and compiler
 *
 *  ABSTRACT:
 *	Maintain a list and table for constants.
 *
 *  MODIFIED BY:
 *	DEC	Digital Equipment Corporation
 *	CPQ	Compaq Computer Corporation
 *
 *  MODIFICATION HISTORY:
 *	14-Jan-1993	DEC	Initial version
 *	01-Dec-1999	CPQ	Release with GPL
 */

#include <common.h>
#include <cmp_comm.h>
#include <hash.h>
#include <cons.h>
#include <mol.h>
#include <decl.h>


#ifdef __MSDOS
#define CONS_HT_SIZE 	 307
#define CLASS_HT_SIZE 	  37

#else
#define CONS_HT_SIZE 	1999
#define CLASS_HT_SIZE 	1009
#endif


struct cons_list {
  struct cons_list *next;	/* next list entry			*/
  Molecule	   cons_name;  	/* molecule of constant 		*/
  long             cons_index;	/* index of constant within block	*/
};

typedef struct cons_list *Cons_List;

/*
 * Variables which keep track of "current" objects
 */

static  Hash_Table	cons_ht;    	/* Hash table keying cons in block */
static  Cons_List       cons_first;	/* First of cons in block 	   */
static  Cons_List       cons_last;	/* Last of cons in block 	   */
static  long		cons_cnt;	/* current index count	           */
static  Boolean		cons_init;	/* init flag			   */


/* check for existing entry - returns TRUE/FALSE */
static Boolean cons_is_valid (Mol_Symbol name);

static void
cons_free (void *cons)
{
  rul__mol_decr_uses (((Cons_List) cons)->cons_name);
  rul__mem_free (cons);
}

void
rul__cons_clear (void)
{
  Cons_List cons = cons_first;
  Cons_List cons_nxt;

  while (cons) {
    cons_nxt = cons->next;
    cons_free (cons);
    cons = cons_nxt;
  }

  cons_first = cons_last = NULL;
  cons_cnt = 0;

  if (cons_ht)
    rul__hash_delete_table (cons_ht);
}

void
rul__cons_init (void)
{
  if (cons_ht != NULL)
    rul__cons_clear ();

  cons_ht = rul__hash_create_table (CONS_HT_SIZE);

  cons_init = TRUE;
}

void
rul__cons_create_cons (Molecule name)
{
  Cons_List consl;
  long      comp_len, i;

  if (!cons_init)
    rul__cons_init ();
  
  if (!(cons_is_valid (name))) {
  
    if (rul__mol_is_compound (name)) {
      comp_len = rul__mol_get_poly_count (name);
      for (i = 1; i <= comp_len; i++) {
	rul__cons_create_cons (rul__mol_get_comp_nth (name, i));
      }
    }

    consl = RUL_MEM_ALLOCATE (struct cons_list, 1);
    consl->next = NULL;
    consl->cons_name = name;
    rul__mol_incr_uses (name);
    consl->cons_index = ++cons_cnt;
    
    if (!cons_first)
      cons_last = cons_first = consl;
    else {
      cons_last->next = consl;
      cons_last = consl;
    }
    
    rul__hash_add_entry (cons_ht, name, consl);
  }

  return;
}

    
static Boolean cons_is_valid (Molecule name)
{
  return (rul__hash_get_entry (cons_ht, name) != NULL);
}

long rul__cons_get_index (Molecule name)
{
  Cons_List consl;

  consl = rul__hash_get_entry (cons_ht, name);
  if (consl)
    return (consl->cons_index);

  return (0);
}

long rul__cons_get_next (void **context, Molecule *name)
{
  Cons_List consl = (Cons_List) *context;

  if (consl) {
    *context = (void *) consl->next;
    consl = consl->next;
  }
  else {
    *context = (void *) cons_first;
    consl = cons_first;
  }

  if (consl) {
    *name = consl->cons_name;
    return (consl->cons_index);
  }

  return (0);
}

long rul__cons_get_count (void)
{
  return (cons_cnt);
}





struct class_list {
  struct class_list *next;	   /* next list entry			*/
  Class	   	     class_id;     /* class of classtant 		*/
  long               class_index;  /* index of classtant within block   */
};

typedef struct class_list *Class_List;

/*
 * Variables which keep track of "current" objects
 */

static  Hash_Table	class_ht;    	/* Hash table keying class in block */
static  Class_List      class_first;	/* First of class in block 	   */
static  Class_List      class_last;	/* Last of class in block 	   */
static  long		class_cnt;	/* current index count	           */
static  Boolean		class_init;	/* init flag			   */


/* check for existing entry - returns TRUE/FALSE */
static Boolean class_is_valid (Class class_id);


void
rul__class_clear (void)
{
  Class_List class_ptr = class_first, class_nxt;

  while (class_ptr) {
    class_nxt = class_ptr->next;
    rul__mem_free(class_ptr);
    class_ptr = class_nxt;
  }

  class_first = class_last = NULL;
  class_cnt = 0;

  if (class_ht)
    rul__hash_delete_table (class_ht);
}

void
rul__class_init (void)
{
  if (class_ht != NULL)
    rul__class_clear ();

  class_ht = rul__hash_create_table (CLASS_HT_SIZE);

  class_init = TRUE;
}

void rul__class_create_class (Class class_id)
{
  Class_List class_ptr;

  if (!class_init)
    rul__class_init ();
  
  if (!(class_is_valid (class_id))) {
  
    class_ptr = RUL_MEM_ALLOCATE (struct class_list, 1);
    class_ptr->next = NULL;
    class_ptr->class_id = class_id;
    class_ptr->class_index = ++class_cnt;
    
    if (!class_first)
      class_last = class_first = class_ptr;
    else {
      class_last->next = class_ptr;
      class_last = class_ptr;
    }
    
    rul__hash_add_entry (class_ht,
			 rul__decl_get_class_name (class_id),
			 class_ptr);
  }

  return;
}

    
static Boolean class_is_valid (Class class_id)
{
  return (rul__hash_get_entry (class_ht,
			       rul__decl_get_class_name (class_id)) != NULL);
}

long rul__class_get_index (Class class_id)
{
  Class_List class_ptr;

  class_ptr = rul__hash_get_entry (class_ht,
				   rul__decl_get_class_name (class_id));
  if (class_ptr)
    return (class_ptr->class_index);

  return (0);
}

long rul__class_get_next (void **context, Class *class_id)
{
  Class_List class_ptr = (Class_List) *context;

  if (class_ptr) {
    *context = (void *) class_ptr->next;
    class_ptr = class_ptr->next;
  }
  else {
    *context = (void *) class_first;
    class_ptr = class_first;
  }

  if (class_ptr) {
    *class_id = class_ptr->class_id;
    return (class_ptr->class_index);
  }

  return (0);
}

long rul__class_get_count (void)
{
  return (class_cnt);
}


