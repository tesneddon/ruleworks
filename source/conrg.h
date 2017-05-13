/* conrg.h - Construct registration subsystem */
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
 *	RULEWORKS compiler
 *
 * ABSTRACT:
 *	This file is the external specification for cmp_conrg.c, which
 *	provides registration of constructs.
 *
 * AUTHOR:
 *	DEC	Digital Equipment Corporation
 *	CPQ	Compaq Computer Corporation
 *
 * MODIFICATION HISTORY:
 *	14-Jan-1993	DEC	Initial version
 *	01-Dec-1999	CPQ	Release with GPL
 */



void
rul__conrg_register_construct(Mol_Symbol construct_name,
			      Construct_Type construct_type,
			      long line_num);
void
rul__conrg_register_block(Mol_Symbol block_name,
			  Construct_Type block_type,
			  long line_num);

/* Returns TRUE if we are compiling the MAIN entry block */
Boolean
rul__conrg_is_main_block (void);

/* Returns RUL__C_NOISE if the construct doesn't exist. */
Construct_Type
rul__conrg_get_construct_type(Mol_Symbol construct_name);

long
rul__conrg_get_construct_index(Mol_Symbol construct_name);

Construct_Type
rul__conrg_get_block_type(Mol_Symbol block_name);

long
rul__conrg_get_cur_block_index(void);

long
rul__conrg_get_construct_line(Mol_Symbol construct_name);

Mol_Symbol
rul__conrg_get_cur_block_name(void);


Mol_Symbol
rul__conrg_get_cur_group_name(void);


/* Get constructs of this block in the order in which they were registered.
   Pass NULL for context to get first construct (the block).
   Returns FALSE if no more constructs. */
Boolean
rul__conrg_get_next_construct(void **context,
			      Mol_Symbol *name,
			      Construct_Type *ctype);


long
rul__conrg_get_count(void);

long
rul__conrg_get_catcher_count(void);

void
rul__conrg_clear_all_blocks (void);

