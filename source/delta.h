/****************************************************************************
**                                                                         **
**                       R T S _ D E L T A . H                             **
**                                                                         **
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
**  FACILITY:
**	RULEWORKS run time system
**
**  ABSTRACT:
**	This file contains the exported definitions for the delta queue
**	and delta token subsystem, to be used elsewhere in the system.
**
**
**  MODIFIED BY:
**	DEC	Digital Equipment Corporation
**	CPQ	Compaq Computer Corporation
**
**  MODIFICATION HISTORY:
**
**	28-Jul-1992	DEC	Initial version
**	01-Dec-1999	CPQ	Release with GPL
*/


/* INCLUDE-IN-GEND.H  *********************************************** */
/*
**  The following struct information should really be used only
**  by the places where delta tokens are created.  Everywhere
**  else should treat them as opaques, by using the macros defined
**  below as if they were functions.
*/
struct delta_token {
	Token_Sign	sign;
	Object		instance;
	Class		instance_class;
	Mol_Instance_Id	instance_id;
};

/* END-INCLUDE-IN-GEND.H  *********************************************** */

#define rul__delta_tokens_decling_block(del_tok_ptr)	\
    (rul__decl_get_class_block_name(del_tok_ptr->instance_class))

#define rul__delta_tokens_class(del_tok_ptr)		\
    (del_tok_ptr->instance_class)

#define rul__delta_tokens_timetag(del_tok_ptr)		\
    (rul__wm_get_timetag(del_tok_ptr->instance))

#define rul__delta_tokens_sign(del_tok_ptr)		\
    (del_tok_ptr->sign)

#define rul__delta_tokens_instance(del_tok_ptr)		\
    (del_tok_ptr->instance)

#define rul__delta_tokens_instance_id(del_tok_ptr)	\
    (del_tok_ptr->instance_id)



Delta_Queue	rul__delta_make_queue ( void ) ;

void		rul__delta_free_queue ( Delta_Queue dq ) ;


/*
**  The rul__delta_add_to_queue copies the information it needs
**  onto the Delta_Queue.
*/
void	rul__delta_add_to_queue (
			Delta_Queue dq,
			long last_propagated_tt,
			Token_Sign sign,
			Mol_Instance_Id inst_id,
			long timetag) ;

/*
**  The rul__delta_get_from_queue routine removes the first
**  Delta_Token from the specified Delta_Queue, and returns
**  it to the caller.
**
**  The caller is responsible for calling rul__mem_free on
**  the Delta_Token returned when he is done with it.
*/
Delta_Token	rul__delta_get_from_queue (
			Delta_Queue dq,
			long target_tt ) ;


