/****************************************************************************
**                                                                         **
**                   C M P _ N E T _ T E S T S . C                         **
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
 * FACILITY:
 *	RULEWORKS compiler
 *
 * ABSTRACT:
 *	This module provides the compiler functions for 
 *	capturing the predicate test information required 
 *	when building the RETE network for the current block.
 *
 * MODIFIED BY:
 *	DEC	Digiatl Equipment Corporation
 *	CPQ	Compaq Computer Corporation
 *
 * MODIFICATION HISTORY:
 *
 *	12-Jan-1992	DEC	Initial version
 *	01-Dec-1999	CPQ	Releasew with GPL
 */


#include <common.h>
#include <cmp_comm.h>
#include <ast.h>
#include <ios.h>
#include <net.h>
#include <val.h>
#include <net_p.h>



/***************************
**                        **
**  RUL__NET_CREATE_TEST  **
**                        **
***************************/

Net_Test rul__net_create_test (Pred_Type pred, Pred_Type opred,
			       Value val_1, Value val_2)
{
	Net_Test new_test;

	/*  Create the new test structure  */
	new_test = (Net_Test) rul__mem_malloc (sizeof (struct net_test));
	assert (new_test != NULL);

	new_test->predicate = pred;
	new_test->opt_pred  = opred;
	new_test->value_1   = val_1;
	new_test->value_2   = val_2;
	new_test->next_test = NULL;

	return (new_test);
}



/**************************
**                       **
**  RUL__NET_FREE_TESTS  **
**                       **
**************************/

void rul__net_free_tests (Net_Test cur_test)
{
	Net_Test next;

	if (cur_test == NULL) return;

	if (cur_test->value_1)  rul__val_free_value (cur_test->value_1);
	if (cur_test->value_2)  rul__val_free_value (cur_test->value_2);

	next = cur_test->next_test;
	rul__mem_free (cur_test);

	rul__net_free_tests (next);
}




/***************************
**                        **
**  RUL__NET_ATTACH_TEST  **
**                        **
***************************/

void rul__net_attach_test (Net_Node node, Net_Test new_test)
{
	Net_Test *list_head, tmp;

	assert (is_1_in_node(node)  ||  is_2_in_node(node));

	if (is_1_in_node(node)) {
	    list_head = &(((Net_1_In_Node)node)->first_test);
	} else {
	    list_head = &(((Net_2_In_Node)node)->first_test);
	}

	/*  Attach it to the specified test list */
	if (*list_head) {
	    tmp = *list_head;
	    while (tmp->next_test) tmp = tmp->next_test;
	    tmp->next_test = new_test;
	} else {
	    *list_head = new_test;
	}

	/* Update test specificity count for current rule */
	rul___net_increment_test_count ();
}




/******************************
**                           **
**  RUL__NET_TEST_IS_SIMPLE  **
**                           **
******************************/

Boolean rul__net_test_is_simple (Net_Test test)
{
	if (rul__val_is_simple_expr(test->value_1)  &&
	    rul__val_is_simple_expr(test->value_2)) {
	    return (TRUE);
	}
	return (FALSE);
}





/*****************************
**                          **
**  RUL__NET_TEST_IS_LOCAL  **
**                          **
*****************************/

Boolean rul__net_test_is_local (Net_Test test, long ce_index)
{
	Boolean non_local_1, non_local_2;

	non_local_1 = rul__val_is_local (test->value_1, ce_index);
	non_local_2 = rul__val_is_local (test->value_2, ce_index);

	return (non_local_1 && non_local_2);
}



#ifndef NDEBUG


/***************************
**                        **
**  RUL__NET_PRINT_TESTS  **
**                        **
***************************/

void rul__net_print_tests (Net_Test first_test, char *indent)
{
	Net_Test tst;

	tst = first_test;
	while (tst) {
	    rul__net_test_print (tst, indent);
	    tst = tst->next_test;
	}
}



/**************************
**                       **
**  RUL__NET_TEST_PRINT  **
**                       **
**************************/

void rul__net_test_print (Net_Test test, char *indent)
{

	rul__ios_printf (RUL__C_STD_ERR, "\n%sTest:  ", indent);
	rul___val_print (test->value_1);
	rul__ios_printf (RUL__C_STD_ERR, " %s ",
			rul__ast_pred_to_string (test->predicate));
	if (test->opt_pred)
	  rul__ios_printf (RUL__C_STD_ERR, " %s ",
			   rul__ast_pred_to_string (test->opt_pred));
	rul___val_print (test->value_2);
}

#endif /* ifndef NDEBUG */
