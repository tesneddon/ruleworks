/*
 * sql.h  -  RuleWorks SQL interface globals, constants, etc:
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
 *	RuleWorks SQL interface
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
**	CPQ	Compaq Computer Corporation
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
 */

/*----------------------------------------------------------------------------*/
/* Declare SQL interface function prototypes: */
/* INCLUDE-IN-GEND.H  *********************************************** */

void rul__sql_rse_init       (void);

char *rul__sql_rse           (void);

void rul__sql_rse_symbol     (RulMolecule     symbol,
			      RulBoolean      quoted);


long rul__sql_attach          (long         filenamep, 
			       char        *schema_source,
			       char        *dbkey_scope);

long rul__sql_commit          (void);

long rul__sql_delete          (char           *SQL_table_name,
			       char           *SQL_where_clause,
			       RulEntry_Data   eb_data);

long rul__sql_detach          (void);

long rul__sql_fetch_setup     (char        *rse);

long rul__sql_fetch_to_wme    (RulMolecule   **wme_id_array,
			       long           *num_wmes_fetched,
			       RulEntry_Data   eb_data);

long rul__sql_fetch_each      (RulMolecule   **value_array,
			       long            num_values,
			       RulEntry_Data   eb_data);

long rul__sql_fetch_cleanup   (RulMolecule    *array);

long rul__sql_insert          (char           *SQL_table_name,
			       char           *SQL_set_expression,
			       RulEntry_Data   eb_data);

long rul__sql_insert_from_wme (RulMolecule     wme_id,
			       RulEntry_Data   eb_data);

long rul__sql_rollback        (void);

long rul__sql_start           (char          *txn_type);

long rul__sql_update          (char          *SQL_table_name,
			       char          *SQL_set_expression,
			       RulEntry_Data  eb_data);

long rul__sql_update_from_wme (RulMolecule    wme_id,
			       char          *SQL_where_clause,
			       RulEntry_Data  eb_data);


/* END-INCLUDE-IN-GEND.H  *********************************************** */
/* Define return status values for RULEWORKS SQL routines: */

#define	RUL_SQL_SUCCESS	1
#define RUL_SQL_WARNING	0
#define RUL_SQL_ERROR	0


