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
*/

typedef struct bnode *Bnode;

#define _NORMAL     TRUE
#define _KEYALRINS  -102
#define _KEYNOTFOU  -104
#define _INSVIRMEM  -106

struct bnode {
  Bnode   node_left;
  Bnode   node_right;
  short   node_bal;
  void   *node_ptr;
};

long rul__btree_insert_tree (Bnode *treehead,
			     void  *key,
			     long   control_flags,
			     long   (*cmprtn)(void *, Bnode, void *),
			     long   (*alcrtn)(void *, Bnode *, void *),
			     Bnode *node,
			     void  *user_data);

long rul__btree_lookup_tree (Bnode *treehead,
			     void  *key,
			     long   (*cmprtn)(void *, Bnode, void *),
			     Bnode *node);

long rul__btree_remove_tree (Bnode   *treehead,
			     void    *key,
			     long     (*cmprtn)(void *, Bnode, void *),
			     long     (*dalcrtn)(Bnode, void *),
			     void    *user_data);

long rul__btree_traverse_tree (Bnode *treehead,
			       long   (*actrtn)(Bnode, void *),
			       void  *user_data);

long rul__btree_purge_tree (Bnode *treehead,
			    long   (*dalcrtn)(Bnode, void *),
			    void  *user_data);


