/*
 * sql_p.h  -  RULEWORKS SQL private header; interface globals, constants, etc:
 */
/****************************************************************************
RuleWorks - Rules based application development tool.

Copyright (C) 1999  Compaq Computer Corporation
Copyright (C) 2017  Endless Software Solutions

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
 *	This C header file contains definitions for RULEWORKS SQL constants,
 *	function prototypes, conditions, etc.
 *
 *	Also see the following RULEWORKS SQL interface documents:
 *	    - functional spec (OPS_SQL_SPEC.TXT)
 *	    - design document (OPS_SQL_DESIGN.TXT).
 *
 * MODIFIED BY:
 *	DEC	Digital Equipment Corporation
 *	CPQ	Compaq Computer Corporation
 *	ESS	Endless Software Solutions
 *
 * MODIFICATION HISTORY:
 *
 *	13-Sep-1991	DEC	Initial version
 *
 *	30-Sep-1991	DEC	Remove unused fetch conditions: SQLRECFET for
 *					success and SQLZERFET for zero records fetched.
 *
 *	25-Nov-1991	DEC	Conditionally compile the message declarations
 *					with globalvalue for VAX C only; otherwise, use
 *					#include <rts_gbl_message.h>. Defined _node.
 *					Added a declaration for _RDB$MESSAGE_VECTOR.
 *
 *	06-Apr-1993	DEC	changes for tin (v5)
 *
 *	01-Dec-1999	CPQ	Release with GPL
 *
 *	15-May-2017	ESS	Split into common private and DB specific.
 */


/*----------------------------------------------------------------------------*/
/* Constants on RUL side: */

/* Define size constants: */

#define MAX_MSG_WME_SIZE      1024

/* Define fetched WME space allocation constants: */

#define WME_SPACE_MIN		  40
#define WME_SPACE_PER_ATTRNAME	  40
#define WME_SPACE_PER_NIL	   3
#define WME_SPACE_PER_INTEGER	  15
#define WME_SPACE_PER_SMALLINT	  15
#define WME_SPACE_PER_QUADWORD	  30
#define WME_SPACE_PER_FLOAT	  15
#define WME_SPACE_PER_DOUBLE	  30
#define WME_SPACE_PER_DATE	  30

#define WME_ID_INIT_ARRAY_SIZE  1000
#define WME_ID_ARRAY_EXPANSION	 500

/*----------------------------------------------------------------------------*/
/* Constants on SQL side: */

#define MAX_FNAME_SIZE		31	/* Rdb/VMS limit; SQL std uses 30 */
#define MAX_NUM_FLD	      2048	/* more attrs than this, out of luck */
#define MAX_SCHEMA_DECL_SIZE   512      /* vms filename of 256 + a few */
#define MAX_SQL_STMT_SIZE     8192	/* very large statement for */
#define MAX_LIST_SIZE	      4096      /* lots of long attr/field names */
#define MAX_RSE_SIZE	      8192      /* */
#define SQL_MISSING_VALUE	-1	/* missing field value for Rdb/VMS */

/* Cached SQL table+field name pairs data will be keyed on concatenation of *
 * both of these strings, so define key size as twice size of each string:  */

#define CACHE_KEY_SIZE	     (2*MAX_FNAME_SIZE)

/* Define transaction status values: */

#define RUL_SQL_NO_TXN	    0
#define RUL_SQL_TXN_ACTIVE   1

/************************ end of sql_p.h **************************/
