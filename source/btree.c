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

!
! The code that follows does not duplicate that in the LIB$BINARY_TREE
! RTL routines.  The routines with names similar to those
! in the RTL (RUL__BTREE_ and LIB$- INSERT_TREE, for example) are
! functionally equivalent.
! 
! The new routines supplied are
!
!  rul__btree_remove_tree ( treehead, symbolstring, compare_rtn,
!                                  dealloc_rtn, user_data )
!
!    Calls dealloc_rtn to remove a single entry.
!
! and
!
!  rul__btree_purge_tree ( treehead, dealloc_rtn, user_data )
!
!    Calls dealloc_rtn to remove each entry: leaves, then branches, then root.
!
! Note: rul__btree_remove_tree as written removes only the first encountered
!  occurrance for duplicate entries;  removing all occurrances of a duplicate
!  requires calling rul__btree_remove_tree for each occurrance (i.e. until
!  lib$_keynotfou is returned).
!
*/

#include <common.h>
#include <btree.h>

static Bnode    cur_node;
static Bnode    rep_node;
static Bnode    del_node;
static void    *keyname;
static long     (*action_rtn) (Bnode, void *);
static long     (*alloc_rtn) (void *, Bnode *, void *);
static long     (*compare_rtn) (void *, Bnode, void *);
static long     (*dealloc_rtn) (Bnode, void *);
static Bnode   *blockretadr;
static long     foundintree;
static long     controlflags;
static void    *user_cxt;

static Boolean balance_left (void)
{
  Bnode down_left,down_right;

  /* ! balance a left subtree which has grown higher...*/
  cur_node->node_bal = cur_node->node_bal - 1;
  if (cur_node->node_bal == 0)
    return TRUE;

  if (cur_node->node_bal & 1)
    return FALSE;

  down_left = cur_node->node_left;
  if (down_left->node_bal < 0) {
    /* ! single ll rotation */
    cur_node->node_left = down_left->node_right;
    down_left->node_right = cur_node;
    cur_node->node_bal = 0;
    cur_node = down_left;
    cur_node->node_bal = 0;
    return TRUE;
  }
  else {
    /* ! double lr rotation */
    down_right = down_left->node_right;
    down_left->node_right = down_right->node_left;
    down_right->node_left = down_left;
    cur_node->node_left = down_right->node_right;
    down_right->node_right = cur_node;
    cur_node->node_bal = 0;
    down_left->node_bal = 0;
    if	(down_right->node_bal > 0)
      down_left->node_bal = -1;
    else if (down_right->node_bal < 0)
      cur_node->node_bal = 1;
    cur_node = down_right;
    cur_node->node_bal = 0;
    return TRUE;
  }
}

static Boolean balance_leftd (void)
{
  Bnode  down_left, down_right;
  short  dl_bal,    dr_bal;

  /*! balance a right subtree which has become shorter... */

  /*! balance was 1 or 0 */
  if (cur_node->node_bal == 1) {
    cur_node->node_bal = 0;
    return FALSE;
  }

  if (cur_node->node_bal == 0) {
    cur_node->node_bal = -1;
    return TRUE;
  }

  /*! balance was -1 */
  down_left = cur_node->node_left;
  dl_bal = down_left->node_bal;
  if (dl_bal <= 0) {
    /*! single ll rotation */
    cur_node->node_left = down_left->node_right;
    down_left->node_right = cur_node;
    if (dl_bal == 0)
      cur_node->node_bal = -1;
    else
      cur_node->node_bal = 0;
    down_left->node_bal = abs(cur_node->node_bal);
    cur_node = down_left;
    return cur_node->node_bal;
  }
  else {
    /*! double lr rotation */
    down_right = down_left->node_right;
    dr_bal = down_right->node_bal;
    down_left->node_right = down_right->node_left;
    down_right->node_left = down_left;
    cur_node->node_left = down_right->node_right;
    down_right->node_right = cur_node;
    if (dr_bal == -1)
      cur_node->node_bal = 1;
    else
      cur_node->node_bal = 0;
    if (dr_bal == 1)
      down_left->node_bal = -1;
    else
      down_left->node_bal = 0;
    cur_node = down_right;
    down_right->node_bal = 0;
    return FALSE;
  }
}	/*! balance_leftd */

static Boolean balance_right (void)
{
  Bnode down_left, down_right;

  /*! rebalance right subtree which is now higher */
  cur_node->node_bal = cur_node->node_bal + 1;

  if (cur_node->node_bal == 0)
    return TRUE;

  if (cur_node->node_bal & 1)
    return FALSE;

  down_right = cur_node->node_right;
  if (down_right->node_bal > 0) {
    /*! single rr rotation */
    cur_node->node_right = down_right->node_left;
    down_right->node_left = cur_node;
    cur_node->node_bal = 0;
    cur_node = down_right;
    cur_node->node_bal = 0;
    return TRUE;
  }
  else {
    /*! double rl routation */
    down_left = down_right->node_left;
    down_right->node_left = down_left->node_right;
    down_left->node_right = down_right;
    cur_node->node_right = down_left->node_left;
    down_left->node_left = cur_node;
    cur_node->node_bal = 0;
    down_right->node_bal = 0;
    if (down_left->node_bal < 0)
      down_right->node_bal = 1;
    else if (down_left->node_bal > 0)
      cur_node->node_bal = -1;
    cur_node = down_left;
    cur_node->node_bal = 0;
    return TRUE;
  }
}	/*! balance_right */

static Boolean balance_rightd (void)
{
  Bnode down_left, down_right;
  short dl_bal,    dr_bal;

  /*! balance a left subtree which has become shorter... */

  /*! balance was -1 or 0 */
  if (cur_node->node_bal == -1)  {
    cur_node->node_bal = 0;
    return FALSE;
  }

  if (cur_node->node_bal == 0) {
    cur_node->node_bal = 1;
    return TRUE;
  }

  /*! balance was 1 */
  down_right = cur_node->node_right;
  dr_bal = down_right->node_bal;
  if (dr_bal >= 0) {
    /*! single rr rotation */
    cur_node->node_right = down_right->node_left;
    down_right->node_left = cur_node;
    if (dr_bal == 0) {
      cur_node->node_bal = 1;
      down_right->node_bal = -1;
      cur_node = down_right;
      return TRUE;
    }
    else {
      cur_node->node_bal = 0;
      down_right->node_bal = 0;
      cur_node = down_right;
      return FALSE;
    }		
  }
  else {
    /*! double rl routation */
    down_left = down_right->node_left;
    dl_bal = down_left->node_bal;
    down_right->node_left = down_left->node_right;
    down_left->node_right = down_right;
    cur_node->node_right = down_left->node_left;
    down_left->node_left = cur_node;

    if (dl_bal == 1)
      cur_node->node_bal = -1;
    else
      cur_node->node_bal = 0;

    if (dl_bal == -1)
      down_right->node_bal = 1;
    else
      down_right->node_bal = 0;

    cur_node = down_left;
    down_left->node_bal = 0;
    return FALSE;
  }
}	/*! balance_rightd */

static Boolean insert_node (void)
{
  Bnode down_left, down_right, save_current;
  long  status,	in_balance;

  if (cur_node == 0) {
    if (!(alloc_rtn) (keyname, &save_current, user_cxt))
      return (foundintree = _INSVIRMEM);
    cur_node = save_current;
    cur_node->node_left = 0;
    cur_node->node_right = 0;
    cur_node->node_bal = 0;
    *blockretadr = cur_node;
    return FALSE;
  }

  save_current = cur_node;
  if ((in_balance = (compare_rtn) (keyname, cur_node, user_cxt)) <= 0) {
    if ((in_balance == 0) && (! controlflags & 1)) {
      /*! matched, but noinsert duplicates set. */
      *blockretadr = cur_node;
      return (foundintree = _KEYALRINS);
    }
    cur_node = cur_node->node_left;
    in_balance = insert_node();
    if ((foundintree == _KEYALRINS) || (foundintree == _INSVIRMEM))
      return TRUE;
    down_left = cur_node;
    cur_node = save_current;
    cur_node->node_left = down_left;
    if (in_balance & 1)
      return TRUE;
    else
      return balance_left();
  }
  else {
    cur_node = cur_node->node_right;
    in_balance = insert_node();
    if ((foundintree == _KEYALRINS) || (foundintree == _INSVIRMEM))
      return TRUE;
    down_right = cur_node;
    cur_node = save_current;
    cur_node->node_right = down_right;
    if (in_balance & 1)
      return TRUE;
    else
      return balance_right();
  }
} /*!  of insert_node */

static Boolean find_rightmost (void)
{
  Bnode down_left, down_right, save_current;
  long  in_balance;

  /*! find rightmost node under cur_node. */
  /*! cur_node->node_right is replaced for each cur_node. */
  /*! rep_node points to replacement node address on return. */

  save_current = cur_node;
  if (cur_node->node_right) { /*!= 0 */
    cur_node = cur_node->node_right;
    in_balance = find_rightmost();
    down_right = cur_node;
    cur_node = save_current;
    cur_node->node_right = down_right;
    if (in_balance & 1)
      return TRUE;
    else
      return balance_leftd();
  }
  else {
    rep_node = cur_node;
    cur_node = rep_node->node_left;
    rep_node->node_left = 0;
    rep_node->node_right = del_node->node_right;
    rep_node->node_bal = del_node->node_bal;
    return FALSE;
  }
}
	
static Boolean remove_node (void)
{
  Bnode down_left, down_right, save_current;
  long  status,	in_balance;

  if (cur_node == 0) {
    /*! no more stuff to look at, may as well go home... */
    foundintree = _KEYNOTFOU;
    return TRUE;
    }

  save_current = cur_node;
  if (in_balance = (compare_rtn) (keyname, cur_node, user_cxt)) { /*!= 0 */
    if (in_balance < 0) {
      cur_node = cur_node->node_left;
      in_balance = remove_node();
      if ((foundintree == _KEYNOTFOU) || (foundintree == _INSVIRMEM))
	return TRUE;
      /*! found down left branch */
      down_left = cur_node;
      cur_node = save_current;
      cur_node->node_left = down_left;
      /*! right branch may now be higher */
      if (in_balance & 1)
	return TRUE;
      else
	return balance_rightd();
    }
    else {
      cur_node = cur_node->node_right;
      in_balance = remove_node();
      if ((foundintree == _KEYNOTFOU) || (foundintree == _INSVIRMEM))
	return TRUE;
      down_right = cur_node;
      cur_node = save_current;
      cur_node->node_right = down_right;
      if (in_balance & 1)
	return TRUE;
      else
	return balance_leftd();
    }
  }

  del_node = cur_node;

  if (cur_node->node_right == 0) {
    cur_node = cur_node->node_left;
    in_balance = FALSE;
  }
  else if (cur_node->node_left == 0) {
    cur_node = cur_node->node_right;
    in_balance = FALSE;
  }
  else {
    /*! cur_node[left and right] are non-zero pointers. */
    cur_node = cur_node->node_left;
    in_balance = find_rightmost();
    /*! cur_node = replacement leftpointer for replacement node */
    /*! rep_node = replacement node */
    down_left = cur_node;
    cur_node = rep_node;
    /*! cur_node = replacement node */
    cur_node->node_left = down_left;
    /*! left side has one less element, */
    /*! may be out of balance */
    if (! (in_balance & 1))
      in_balance = balance_rightd();
  }

  if (dealloc_rtn)
    if ((dealloc_rtn) (del_node, user_cxt))
      return in_balance;
    else
      foundintree = _INSVIRMEM;

  return TRUE;
}	/*! remove_node */

long rul__btree_insert_tree (Bnode *treehead,
			     void  *key,
			     long   control_flags,
			     long   (*cmprtn)(void *, Bnode, void *),
			     long   (*alcrtn)(void *, Bnode *, void *),
			     Bnode *node,
			     void  *user_data)
{

  cur_node     = *treehead;
  keyname      = key;
  compare_rtn  = cmprtn;
  alloc_rtn    = alcrtn;
  blockretadr  = node;
  foundintree  = _NORMAL;
  controlflags = control_flags;
  user_cxt     = user_data;

  /*! call recursive routine to insert node into tree */
  insert_node();

  if (foundintree == _NORMAL)
    *treehead = cur_node;

  return foundintree;
}

long rul__btree_lookup_tree (Bnode *treehead,
			     void  *key,
			     long   (*cmprtn)(void *, Bnode, void *),
			     Bnode *node)
{
  long ch_result;

  cur_node    = *treehead;
  keyname     = key;
  compare_rtn = cmprtn;

  while	(cur_node) { /*!= 0 */
    if ((ch_result = (compare_rtn) (keyname, cur_node, NULL)) == 0) {
      *node = cur_node;
      return _NORMAL;
    }
    else {
      if (ch_result < 0)
	cur_node = cur_node->node_left;
      else
	cur_node = cur_node->node_right;
    }
  }
  return _KEYNOTFOU;
}

long rul__btree_remove_tree (Bnode   *treehead,
			     void    *key,
			     long     (*cmprtn)(void *, Bnode, void *),
			     long     (*dalcrtn)(Bnode, void *),
			     void    *user_data)
{
  Bnode rep_node, del_node;
  long foundintree;

  cur_node    = *treehead;
  keyname     = key;
  compare_rtn = cmprtn;
  dealloc_rtn = dalcrtn;
  foundintree = _NORMAL;
  rep_node    = del_node = NULL;
  user_cxt    = user_data;

  /*! call recursive routine to remove node from tree */
  remove_node();
  if (foundintree == _NORMAL) {
    *treehead = cur_node;
    return _NORMAL;
  }

  return _KEYNOTFOU;
}

static Boolean traverse_tree (Bnode nod)
{
  Bnode right_subtree;
  long status;

  if (nod == 0)
    return _NORMAL;

  if (nod->node_left)  /*!= 0 */
    if (! (status = traverse_tree(nod->node_left)))
      return status;

  right_subtree = nod->node_right;

  if (! (status = (action_rtn) (nod, user_cxt)))
    return status;

  if (right_subtree) /*!= 0 */
    if (! (status = traverse_tree(right_subtree)))
      return status;

  return _NORMAL;
}

static Boolean purge_node (Bnode nod)
{
  long status;

  if (nod == 0)
    return _NORMAL;

  if (nod->node_left) /*!= 0 */
    if (! (status = purge_node(nod->node_left)))
      return status;

  if (nod->node_right) /*!= 0 */
    if (! (status = purge_node(nod->node_right)))
      return status;

  if (! (status = (dealloc_rtn) (nod, user_cxt)))
    return status;

  return _NORMAL;
}

long rul__btree_traverse_tree (Bnode *treehead,
			       long   (*actrtn)(Bnode, void *),
			       void  *user_data)
{
  long status;

  action_rtn = actrtn;
  user_cxt   = user_data;

  if (*treehead) /*!= 0 */
    return traverse_tree (*treehead);
  return _NORMAL;
}

long rul__btree_purge_tree (Bnode *treehead,
			    long   (*dalcrtn)(Bnode, void *),
			    void  *user_data)
{
  long status;

  dealloc_rtn = dalcrtn;
  user_cxt   = user_data;

  if (*treehead == NULL)
    return _NORMAL;

  status = purge_node (*treehead);
  if (status == _NORMAL)
    *treehead = NULL;
  return status;
}
