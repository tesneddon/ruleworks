/*
 * rts_sqlite_utils.c  -  SQLite Utility Routines
 */
/****************************************************************************
RuleWorks - Rules based application development tool.

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
 *	RULEWORKS SQLite interface
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
 *	ESS	Endless Software Solutions
 *
 * MODIFICATION HISTORY:
 *
 *	15-May-2017	ESS	Initial version
 */

#include <common.h>
#include <sql_msg.h>
#include <sql_p.h>
#include <sql.h>
#include <sqlite3.h>

GLOBAL sqlite3 *handle;

/*************************
 **			**
 ** rul___sqlmod_commit **
 **			**
 *************************/

long  rul___sqlmod_commit (void) {
  const char *stmt = "COMMIT;";		/* SQL statement */
  long	      sql_status;		/* generic return status */
  sqlite3_stmt *pstmt;

  sql_status = sqlite3_prepare_v2(handle, stmt, sizeof(stmt) - 1,
				  &pstmt, 0);
  if (sql_status == SQLITE_OK) {
    sql_status = sqlite3_step(pstmt);

    sqlite3_free(pstmt);
  }

  return sql_status;
}					/* End of rul___sqlmod_commit */

/*****************************
 **			    **
 ** rul___sqlmod_rollback   **
 **			    **
 *****************************/

long  rul___sqlmod_rollback (void) {
  const char *stmt = "ROLLBACK;";	/* SQL statement */
  long	      sql_status;		/* generic return status */
  sqlite3_stmt *pstmt;

  sql_status = sqlite3_prepare_v2(handle, stmt, sizeof(stmt) - 1,
				  &pstmt, 0);
  if (sql_status == SQLITE_OK) {
    sql_status = sqlite3_step(pstmt);

    sqlite3_free(pstmt);
  }

  return sql_status;
}					/* End of rul___sqlmod_rollback */

/*************************
 **			**
 ** rul___sqlmod_start  **
 **			**
 *************************/

long  rul___sqlmod_start (void) {
  const char *stmt = "BEGIN TRANSACTION;";  /* SQL statement */
  long	      sql_status;		    /* generic return status */
  sqlite3_stmt *pstmt;

  sql_status = sqlite3_prepare_v2(handle, stmt, sizeof(stmt) - 1,
				  &pstmt, 0);
  if (sql_status == SQLITE_OK) {
    sql_status = sqlite3_step(pstmt);

    sqlite3_free(pstmt);
  }

  return sql_status;
}					/* End of rul___sqlmod_start */
