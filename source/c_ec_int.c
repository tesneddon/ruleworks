/****************************************************************************
**                                                                         **
**              C M P _ E M I T _ C _ I N T E R N A L . C                  **
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

Email: info@ruleworks@co.uk

****************************************************************************/


/*
 * FACILITY:
 *	RULEWORKS compiler
 *
 * ABSTRACT:
 *	This module contains the C code generator implementation
 *	of the low-level generic code emitter.
 *
 *	Specifically, this file contains the internals of that subsystem
 *
 * MODIFIED BY:
 *	DEC	Digital Equipment Corporation
 *	CPQ	Compaq Computer Corporation
 *
 * MODIFICATION HISTORY:
 *
 *	28-Jan-1993	DEC	Initial version
 *	01-Dec-1999	CPQ	Release with GPL
 */

#include <common.h>
#include <cmp_comm.h>
#include <ios.h>
#include <emit.h>
#include <emit_c.h>

#define INDENT_INCR 	4
#define MAX_INDENT  	60
#define MARGIN 		79

static long SL_chars_in_buff = 0;
static long SL_current_indent = 0;
static char ST_buffer [4*RUL_C_MAX_SYMBOL_SIZE+1];
static void insert_indent (void);




/********************
**                 **
**  INSERT_INDENT  **
**                 **
********************/

static void insert_indent (void)
{
	long i, j, indent;

	assert (SL_chars_in_buff == 0);

	indent = MIN (SL_current_indent, MAX_INDENT);

	i = 0;
/* use spaces - either this or need to keep track of current colunm
 *              can't use strlen
 *
 *	for (j=0; j<indent/8; j++) {
 *	    ST_buffer[i] = '\t';
 *	    i++;
 *	}
 *	for (j=0; j<indent%8; j++) {
 *	    ST_buffer[i] = ' ';
 *	    i++;
 *	}
 */
	for (j=0; j<indent; j++) {
	    ST_buffer[i] = ' ';
	    i++;
	}
	ST_buffer[i] = '\0';
	SL_chars_in_buff = i;
}




/************************
**                     **
**  RUL___EMIT_STRING  **
**                     **
************************/

void rul___emit_string (char *str)
{
	long new_len;

	new_len = strlen (str);

	if (SL_chars_in_buff == 0) {
	    /*  Starting a new line, first indent  */
	    insert_indent ();
	} else {
	    if ((new_len + SL_chars_in_buff) > MARGIN) {
		/*  Not enough room on his line, so kick it out  */
	        if (strchr (ST_buffer, (int) '%') == NULL) {
		    ST_buffer[SL_chars_in_buff++] = '\n';
		    ST_buffer[SL_chars_in_buff] = '\0';
		    rul__ios_printf (RUL__C_OBJECT, ST_buffer);
		}
		else
		    rul__ios_printf (RUL__C_OBJECT, "%s\n", ST_buffer);
		SL_chars_in_buff = 0;
		insert_indent ();
	    }
	}
	strcpy (&(ST_buffer[SL_chars_in_buff]), str);
	SL_chars_in_buff += new_len;

	if (SL_chars_in_buff > MARGIN) {
	    if (strchr (ST_buffer, (int) '%') == NULL) {
	        ST_buffer[SL_chars_in_buff++] = '\n';
		ST_buffer[SL_chars_in_buff] = '\0';
		rul__ios_printf (RUL__C_OBJECT, ST_buffer);
	    }
	    else
	        rul__ios_printf (RUL__C_OBJECT, "%s\n", ST_buffer);
	    SL_chars_in_buff = 0;
	}
}




/*************************
**                      **
**  RUL___EMIT_NEWLINE  **
**                      **
*************************/

void rul___emit_newline (void)
{
	if (SL_chars_in_buff > 0)
	    rul__ios_printf (RUL__C_OBJECT, ST_buffer);

	rul__ios_printf (RUL__C_OBJECT, "\n");
	SL_chars_in_buff = 0;
}



/********************************
**                             **
**  RUL___EMIT_VERIFY_NEWLINE  **
**                             **
********************************/

void rul___emit_verify_newline (void)
{
	if (SL_chars_in_buff > 0) {
	    rul__ios_printf (RUL__C_OBJECT, ST_buffer);
	    rul__ios_printf (RUL__C_OBJECT, "\n");
	    SL_chars_in_buff = 0;
	}
}



/*********************************
**                              **
**  RUL___EMIT_EOS_AND_NEWLINE  **
**                              **
*********************************/

void rul___emit_eos_and_newline (void)
{
	assert (SL_chars_in_buff > 0);

	rul__ios_printf (RUL__C_OBJECT, "%s;\n", ST_buffer);
	SL_chars_in_buff = 0;
}




/*****************************
**                          **
**  RUL___EMIT_INCR_INDENT  **
**                          **
*****************************/

void rul___emit_incr_indent (void)
{
#ifndef NDEBUG
	SL_current_indent += INDENT_INCR;
#endif
}



/*****************************
**                          **
**  RUL___EMIT_DECR_INDENT  **
**                          **
*****************************/

void rul___emit_decr_indent (void)
{
#ifndef NDEBUG
	SL_current_indent -= INDENT_INCR;
	assert (SL_current_indent >= 0);
#endif
}
