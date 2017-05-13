/*
 * sql_cache.c  -  cache SQL table+field name/type; based on VMS LIB$TREE_xxx
 */
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
 * FACILITY:
 *	RULEWORKS SQL interface
 *
 * ABSTRACT:
 *	This module contains routines to build and use a cache of SQL table
 *	name + field name pairs (equivalent to WME-class name + attribute name 
 *	pairs, since these are by definition mapped 1-to-1 for cases where this 
 *	cached data is used).  Also caches DB field types for each table+field
 *	pair.
 *
 *	This module contains the following routines:
 *	
 *	    - rul___sql_init_cache
 *	    - rul___sql_to_cache
 *	    - rul___sql_from_cache
 *	    - rul___sql_trav_cache
 *	    - rul___sql_free_cache
 *	    - rul___sql_attr_from_cache
 *	    - rul___sql_node_allocate
 *	    - rul___sql_node_compare
 *	
 * MODIFIED BY:
 *	DEC	Digital Equipment Corporation
 *	CPQ	Compaq Computer Corporation
 *
 * MODIFICATION HISTORY:
 *
 *	13-Sep-1991	DEC	Initial version
 *
 *	23-Oct-1991	DEC	Don't #include sql_var.h; use local variables.
 *
 *	25-Nov-1991	DEC	Moved definition of struct _node into sql_gbl.h
 *					and copied function declarations to definitions
 *
 *	01-Dec-1999	CPQ	Releae with GPL
 *
 */


#include <common.h>
#include <sql_msg.h>
#include <sql_p.h>
#include <sql.h>
#include <libdef.h>
#include <lib$routines.h>

static   Node  tree;	    /* ptr to tree head */


/***************************
**		 	  **
**  rul___sql_from_cache  **
**			  **
***************************/

long rul___sql_from_cache (char *key, /* ptr to key string for cache search */
			   short *type)	/* ptr to returned field type */
{
  Node    node; 	            /* a node in the tree, eg node found */
                                    /*	by LIB$LOOKUP_TREE call		 */
  long    user_data = 0;	    /* for LIB$ tree routines */
  long    status;		    /* generic return status */

  /*======================================================================*/
  /* For input key value, ie SQL table+field name pair (or WME-class+attribute
   * pair), retrieve the cached field type value associated with this pair.
   *
   * If the tree is still empty, don't even try a lookup:
   */

  if (tree == 0) {
    *type = 0;
    return  RUL_SQL_ERROR;	    /* not successful; nothing found */
  }


  /* Attempt to find a tree node containing the input key string: */

  status = lib$lookup_tree (&tree, key, rul___sql_node_compare, &node);

  switch (status) {
	    
  case LIB$_NORMAL:		    /* lookup successful */
    *type = node->val;
    break;

  case LIB$_KEYNOTFOU:	    /* lookup key not found */
    *type = 0;
    return RUL_SQL_ERROR;
    break;

  default:			    /* lookup problem */
    rul___sql_message (SQL_SQLCACLOO, NULL);
    return RUL_SQL_ERROR;
  }		/* end case on lookup status */

  return RUL_SQL_SUCCESS;
}		/* end rul___sql_from_cache */



/************************
**		       **
**  rul___sql_to_cache **
**		       **
************************/

long rul___sql_to_cache (char  *key,   /* ptr to key string for cache search */
			 short  type)  /* short int field type to be cached */
{
  Node  node;	              /* a node in the tree, eg node found */
                              /*	by LIB$LOOKUP_TREE call		 */
  long  ins_flg = 0;	      /* control flag for LIB$INSERT_TREE */
  long  user_data = 0;	      /* for LIB$ tree routines */
  long  status;		      /* generic return status */
  long  len;		      /* length of a string */

  /*========================================================================*/
  /* Insert the new data (key string) into the cache: */

  status = lib$insert_tree (&tree, key, &ins_flg, rul___sql_node_compare, 
			    rul___sql_node_allocate, &node, user_data);

  switch (status) {

  case LIB$_NORMAL:	    /* insert normal */
#ifdef TEST
    printf ("\n\n  New tree node inserted for key = '%s' \n", key);
#endif
    len = strlen (key);
    strcpy (node->str, key);
    node->str[len] = '\0';
    node->val = type;
    break;

  case LIB$_KEYALRINS:	    /* insert unnecessary */
#ifdef TEST
    printf ("\n\n   Note: key '%s' already (previously) inserted \n", key);
#endif
    node->val = type;	    /* update existing node's val field */
    break;

  default:		    /* insert problem */
    rul___sql_message (SQL_SQLCACINS, NULL);
    return RUL_SQL_ERROR;

  }	    /* end case on insert status */

  return RUL_SQL_SUCCESS;
}		/* end rul___sql_to_cache */



/**************************
**			 **
**  rul___sql_init_cache **
**			 **
**************************/


void rul___sql_init_cache (void)
{
  /* If the cache is being used for the first time, or if it was properly 
   * freed after previous use, then the tree head should already be zero.  
   * Whether or not it is, clear it now.  This does mean that if a user had 
   * cached some data, and failed to detach from the database (at which time 
   * the clean up of cache would have occurred), then the space allocated 
   * for the cache will not be freed.
   */
  
  tree = 0;
}		    /* end rul___sql_init_cache routine */



/**************************
**			 **
**  rul___sql_free_cache **
**			 **
**************************/


long rul___sql_free_cache (void)
{
  long	    status;		    /* generic return status */

  /*=======================================================================*/
  /* Free whatever space had been previously allocated for this cache: */

  status = RUL_SQL_SUCCESS;
  if (tree != 0) {
    status = rul___sql_free_node (tree);
    tree = 0;
  }

  return status;
}		    /* end of rul___sql_free_cache routine */



/**************************
**			 **
**  rul___sql_free_node  **
**			 **
**************************/

long rul___sql_free_node (Node node)
     /* a tree node, to be freed if no descendents */
{
  long	    status;		    /* generic return status */

  /*======================================================================*/
  /* Don't apply if node address is zero: */

  if (node == 0) {
    return RUL_SQL_SUCCESS;
  }

  /* If left or right descendent nodes are not zero, apply this routine 
   * recursively to it (and its descendents):
   */

  if (node->l_ptr != 0) {
#ifdef TEST
    printf ("\n  Follow l_ptr of node '%s' ", node->str);
#endif
    status = rul___sql_free_node (node->l_ptr);
    if (!status) {
      return RUL_SQL_ERROR;
    }
  }

  if (node->r_ptr != 0) {
#ifdef TEST
    printf ("\n  Follow r_ptr of node '%s' ", node->str);
#endif
    status = rul___sql_free_node (node->r_ptr);
    if (!status) {
      return RUL_SQL_ERROR;
    }
  }

  /* Once we get here everything below should be free, so free this node: */

#ifdef TEST
  printf ("\n  Freeing node '%s' ", node->str);
#endif

  rul__mem_free (node);

  return RUL_SQL_SUCCESS;
}		    /* end of rul___sql_free_node routine */



/****************************
**			   **
**  rul___sql_node_compare **
**			   **
****************************/

long rul___sql_node_compare (char *key,
			     Node  node, 
			     long  dummy)
{
  long	    compare_result;

  /*=======================================================================*/
  /* Compare input key string to node's string field; return 0 if they're 
   * equal, otherwise return -1 if the key string is "less than" the node's 
   * string value, or return +1 if it's "greater than" the node's string.
   */

  compare_result = strcmp (key, node->str);

  if (compare_result < 0)
    return -1;

  if (compare_result > 0)
    return 1;

  return 0;
}		    /* end rul___sql_node_compare routine */



/*****************************
**			    **
**  rul___sql_node_allocate **
**			    **
*****************************/

long rul___sql_node_allocate (char *key, 
			      Node *node, 
			      long dummy)
{

  /* Allocate space for a new cache node: */

  *node = (struct _node *) rul__mem_malloc (sizeof(struct _node));
  if (*node == 0) {
    rul___sql_message (SQL_SQLCACALL, NULL);
    return RUL_SQL_ERROR;
  }

  return RUL_SQL_SUCCESS;
}		    /* end rul___sql_node_allocate routine */



/*******************************
**			      **
**  rul___sql_attr_from_cache **
**			      **
*******************************/

long rul___sql_attr_from_cache (char halfkey[MAX_FNAME_SIZE+1],
				char attr_name[MAX_NUM_FLD][MAX_FNAME_SIZE+1],
				long *ptr_num_attr)
     /* Args:
      *
      *   halfkey	 = 1st half of key, ie table/class name
      *   attr_name	 = array of attr names returned
      *   ptr_num_attr = ptr to number of attr names (in array)
      */
{

  /* Use call to cache traversal routine to build a list of atribute names 
   * (as an array) for the input WME-class name (ie input halfkey argument).
   * Also the return the number of attribute names found in the cache for 
   * this WME-class.
   */

  *ptr_num_attr = 0;		    /* init attr count to zero */

  rul___sql_trav_cache (tree, halfkey, attr_name, ptr_num_attr);

  return RUL_SQL_SUCCESS;

}	    /* end of rul___sql_attr_from_cache routine */



/**************************
**			 **
**  rul___sql_trav_cache **
**			 **
**************************/

long rul___sql_trav_cache (Node  node,
			   char  halfkey[MAX_FNAME_SIZE+1],
			   char  attr_name[MAX_NUM_FLD][MAX_FNAME_SIZE+1],
			   long *ptr_num_attr)
     /* Args:
      *
      *   node	       = ptr to cache (tree head, ie root node)
      *   halfkey      = 1st half of key, ie table/class name
      *   attr_name    = array of attr names returned
      *   ptr_num_attr = ptr to number of attr names (in array)
      */
{
  long	    halfkey_compare;	    /* result of str compare on 1st half key */
  long	    len;		    /* length of a string */

  /*=======================================================================*/
  /* Traverse the input tree (or sub-tree) adding any attribute names found
   * in the cache for the input WME_class name (ie the halfkey input arg) 
   * to the array/list of attributes.  Also increment the count of these 
   * attribute names.
   */

  if (node != NULL) {		    /* don't do anything if node is null */
    halfkey_compare = strncmp (halfkey, node->str, MAX_FNAME_SIZE);

    if (halfkey_compare < 0) {       /* try left branch looking for matches */
      rul___sql_trav_cache (node->l_ptr, halfkey, attr_name, ptr_num_attr);
    }

    else if (halfkey_compare > 0) {  /* try right branch for matches */
      rul___sql_trav_cache (node->r_ptr, halfkey, attr_name, ptr_num_attr);
    }

    /* Fall thru to here means the input halfkey (ie class/table name) 
     * matches for this node, so add the node's key 2nd half (ie attr name)
     * to the list of attr names.  Of course, must also try left and right 
     * branches for further matches.
     */

    else {			    /* matching node */

      /* Go left: */
      rul___sql_trav_cache (node->l_ptr, halfkey, attr_name, ptr_num_attr);

      /* Get current node attr name data: */
      strncpy (attr_name[*ptr_num_attr], &node->str[MAX_FNAME_SIZE],
	       MAX_FNAME_SIZE);
      len = strlen (attr_name[*ptr_num_attr]);
      attr_name[*ptr_num_attr][len] = '\0';
      *ptr_num_attr = *ptr_num_attr + 1;    /* incr cached attr count */

      /* Go right: */
      rul___sql_trav_cache (node->r_ptr, halfkey, attr_name, ptr_num_attr);
    }
  }

  return RUL_SQL_SUCCESS;

}	    /* end of rul___sql_trav_cache routine */
