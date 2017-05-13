/****************************************************************************
**                                                                         **
**                        G E N N E T _ P . H                              **
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
 *	RULEWORKS compiler system
 *
 * ABSTRACT:
 *	This module defines the names and functions needed for the 
 *	generation of the match network generated functions.
 *
 * MODIFIED BY:
 *	DEC	Digital Equipment Corporation
 *	CPQ	Compaq Computer Corporation
 *
 * MODIFICATION HISTORY:
 *
 *	 15-Mar-1993	DEC	Initial version
 *	01-Dec-1999		CPQ	Release with GPL
 */




/*
**	Define symbolic names for the match function template variables
*/
#ifdef LONG_NAMES

#define  DEL_TOK_ARG		"delta_tok"
#define  DEL_SIGN_ARG		"sign"

#define  IN_INST_ARRAY_ARG	"in_inst_array"
#define  OUT_INST_ARRAY_ARG	"out_inst_array"

#define  IN_HSH_ARRAY_ARG	"in_hsh_array"
#define  OUT_HSH_ARRAY_ARG	"out_hsh_array"

#define  IN_AUX_ARRAY_ARG	"in_aux_array"
#define  OUT_AUX_ARRAY_ARG	"out_aux_array"

	/*  	Locals for 2-input nodes 	*/
#define  BETA_TOK_ARG		"beta_tok"
#define  X_CNT_ARG		"cross_matches"
#define  ACT_ON_ARG		"act_on"
#define  OUT_SIGN_ARG		"out_sign"

	/*	Other local variables		*/
#define  INST_PTR_ARG		"inst_ptr"
#define  LOCAL_DEL_TOK		"local_delta"
#define  BLK_NAME_ARG		"block_name"
#define  RULE_NAME_ARG		"rule_name"

#else

#define  DEL_TOK_ARG		"dt"
#define  DEL_SIGN_ARG		"s"

#define  IN_INST_ARRAY_ARG	"iia"
#define  OUT_INST_ARRAY_ARG	"oia"

#define  IN_HSH_ARRAY_ARG	"iha"
#define  OUT_HSH_ARRAY_ARG	"oha"

#define  IN_AUX_ARRAY_ARG	"iaa"
#define  OUT_AUX_ARRAY_ARG	"oaa"

	/*  	Locals for 2-input nodes 	*/
#define  BETA_TOK_ARG		"bt"
#define  X_CNT_ARG		"cm"
#define  ACT_ON_ARG		"a"
#define  OUT_SIGN_ARG		"os"

	/*	Other local variables		*/
#define  INST_PTR_ARG		"ip"
#define  LOCAL_DEL_TOK		"ld"
#define  BLK_NAME_ARG		"bn"
#define  RULE_NAME_ARG		"rn"

#endif



/*
**	Declare subsystem private functions
*/

void rul___gen_lhs_var_set_cur_node (Net_Node node);

void rul___gen_lhs_var_set_cur_side (Boolean is_left_side);

void rul___gen_lhs_var_assigns (Value lhs_var, Net_Node next_node);
