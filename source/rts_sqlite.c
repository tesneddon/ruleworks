/*
 * rts_sqlite.c - RULEWORKS RTS SQLite interface routines
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
 *	RULEWORKS SQLite interface (run-time system)
 *
 * ABSTRACT:
 *	This module contains the rul__sql_* routines called by the
 *	generated code.
 *
 *
 * MODIFIED BY:
 *	ESS	Endless Software Solutions
 *
 * MODIFICATION HISTORY:
 *
 *	15-May-2017	ESS	Initial version
 */

#include <common.h>
#include <atom.h>
#include <mol.h>
#include <sql_msg.h>
#include <sql_p.h>
#include <sql.h>
#include <callback.h>
#include <sqlite3.h>

#define TEST 1

sqlite3 *handle;




/**********************
 **		     **
 **  rul__sql_attach **
 **		     **
 **********************/
/*
 *	Declares the database schema which is to be attached to.
 */

long rul__sql_attach (long  flag,	   /* bool, pathname vs file 2nd arg */
		      char *filename,	   /* schema filename or path */
		      char *scope)	   /* scope of database keys */
{
  int     sql_status;                     /* return status from SQLite */
  long    status;                         /* generic return status */

/*===========================================================================*/

#ifdef TEST
  printf ("\n Now in routine rul__sql_attach ... \n");
#endif

  sql_status = sqlite3_open_v2(filename, &handle,
			       SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE, 0);
  if (sql_status != SQLITE_OK) {
    rul___sql_message(SQL_SQLATTFAI, sqlite3_errstr(sql_status)); 
    status = RUL_SQL_ERROR;
  } else {
    status = RUL_SQL_SUCCESS;
  }
  return status;
}					/* End of rul__sql_attach */



/**********************
 **		     **
 **  rul__sql_commit **
 **		     **
 *********************/
/*
 *	Commits the current SQL transaction (if there is one active), 
 *	thus making permanent any changes made to the database during
 *	the transaction.
 */

long rul__sql_commit (void)
{
  long	    sql_status;		    /* return status from SQL procedures */

  /*=======================================================================*/
#ifdef TEST
  printf ("\n Now in routine rul__sql_commit ... \n");
#endif
#if 0
  /* Commit only if there is an active transaction, else signal user */

  if (RUL_SQL__GL_TXN_STATUS) {
    rul__sqlmod_commit (&sql_status);
    if (sql_status != SQL_SUCCESS) {
      rul___sql_message (SQL_SQLCOMFAI, &RDB$MESSAGE_VECTOR);
      rul__sqlmod_rollback (&sql_status);
    }
  }
  else {	/* no currently active transaction, signal warning to user */
    rul___sql_message (SQL_SQLCOMIGN, NULL);
  }

  /* Whether or not the commit was successful (ie it may have failed and txn 
   * would then have been rolled back), or if txn was not active to begin 
   * with, set txn status to "inactive" in any case, and then return 
   */

  RUL_SQL__GL_TXN_STATUS = RUL_SQL_NO_TXN;
#endif
  return RUL_SQL_SUCCESS;
}					/* End of rul__sql_commit */



/***********************
 **		      **
 **  rul__sql_delete  **
 **	 	      **
 **********************/
/*
 *	Removes the specified record(s) from the database.  The first
 *	input argument selects the database table from which records
 *	are to be deleted, and the second (optional) argument identifies
 *	those records to be deleted.
 */

long  rul__sql_delete (char *table, /* table from which rec to be deleted */
		       char *where, /* where clause specs recs to delete  */
		       Entry_Data eb_data)
{
  long    sql_status;                     /* return status from SQL mod rtn */
  char    sql_stmt[MAX_SQL_STMT_SIZE+1];  /* SQL statement, eg to be exec */

  /*=======================================================================*/
#ifdef TEST
  printf ("\n Now in routine rul__sql_delete ... \n");
#endif

  /* Take a look at passed in (RULEWORKS) arguments
   *	arg 1 => name of DB table from which deletes are to be made
   *	arg 2 => optional WHERE clause, which specifies records to be deleted
   * First arg specifies table name
   */
#if 0
  if ((table == 0) ||				    /* if null pointer, or */
      (strspn (table, " ") >= strlen (table))) {    /*  empty or null string */
    rul___sql_message (SQL_SQLINVTAB, NULL);
    return RUL_SQL_ERROR;
  }

  /* Form delete statement so far, ie with table name */

  strcpy (sql_stmt, "DELETE FROM ");
  strcat (sql_stmt, table);
  strcat (sql_stmt, " ");

  /*-----------------------------------------------------------------------*/
  /* Arg2 is the optional WHERE clause which restricts the records to be 
   * deleted; if there, append to delete statement:
   */

  if ((where != 0) &&				   /* if not null ptr, and */
      (strspn (where, " ") < strlen (where))) {    /*  not empty/null string */
    if ((strlen (sql_stmt) + strlen (where)) >= MAX_SQL_STMT_SIZE) {
      rul___sql_message (SQL_SQLSTMLEX, NULL);
      return RUL_SQL_ERROR;
    }
    strcat (sql_stmt, where);
  }

#ifdef TEST
  printf ("\n\n Delete stmt = \"%s\" \n\n", sql_stmt);
#endif
#endif

  return RUL_SQL_SUCCESS;
}					    /* End of rul__sql_delete */



/**********************
 **		     **
 **  rul__sql_detach **
 **		     **
 **********************/
/*
 *	Terminates the current database attachment or session.
 */

long  rul__sql_detach (void)
{
  long     sql_status;                     /* return status from SQLite */
  int      status;                         /* generic return status */

  /*=======================================================================*/
#ifdef TEST
  printf ("\n Now in routine rul__sql_detach ... \n");
#endif

  if (handle == 0)
    return RUL_SQL_SUCCESS;

  sql_status = sqlite3_close(handle);
  if (sql_status != SQLITE_OK) {
    rul___sql_message(SQL_SQLDETFAI, sqlite3_errstr(sql_status)); 
    status = RUL_SQL_ERROR;
  } else {
    status = RUL_SQL_SUCCESS;
  }

  return status;
}					/* End of rul__sql_detach */



/*
 *	Fetch functions: routines to setup for a fetch of data from the 
 *	database, then perform one of two types of fetch, and finally to clean
 *	up after the fetch operation(s) complete.
 *	
 *	    - rul__sql_fetch_setup
 *	    - rul__sql_fetch_fetch_to_wme
 *	    - rul__sql_fetch_fetch_each
 *	    - rul__sql_fetch_cleanup
 *
 *	Declare static variables to be used by the fetch routines.
 *	   Set by fetch_setup,
 *	   used by fetch_each, fetch_to_wme, and
 *	   freed by fetch_cleanup
 */
static	long	fetch_num_fld;			/* number of fields to fetch */
static	char	fetch_fld_name[MAX_NUM_FLD][MAX_FNAME_SIZE+1];  
						/* array of all field names*/
static	long	fetch_fld_type[MAX_NUM_FLD];	/* array of Rdb data types */
static	short	fetch_missing_ind[MAX_NUM_FLD];	/* missing value indicator */
static	void   *fetch_fld_val[MAX_NUM_FLD];	/* field value (or ptr) array*/
static  char	fetch_table[MAX_FNAME_SIZE+1];  /* DB table (+WME class) name*/


/***************************
 **			  **
 **  rul__sql_fetch_setup **
 **			  **
 ***************************/

long  rul__sql_fetch_setup (char *rse)
{
  /*=======================================================================*/
#ifdef TEST
  printf ("\n Now in routine rul__sql_fetch_setup ... \n");
#endif

  return RUL_SQL_SUCCESS;
}					/* End of rul__sql_fetch_setup */



/****************************
 **			   **
 **  rul__sql_fetch_to_wme **
 **			   **
 ***************************/
/*
 *	The rul__sql_fetch_to_wme routine fetches one or more database records
 *	and creates a new WME for each record fetched.  These new WMEs have
 *	a WME-class name that is the same as the database table name from 
 *	which the data was fetched.  The new WME attributes names also map
 *	1-to-1 to the fields in this database table (although the WME-class
 *	may have more or fewer attributes than the table).
 */

long  rul__sql_fetch_to_wme (Molecule   **array, 
			     long        *num_wme,
			     Entry_Data   eb_data)
     /* address of ptr to returned array of WME-IDs (Molecules) */
     /* ptr to number of array elements returned */
{
#ifdef TEST
  printf ("\n Now in routine rul__sql_fetch_to_wme ... \n");
#endif

  return RUL_SQL_SUCCESS;		/* End of rul___sql_fetch_to_wme */
}



/**************************
 **			 **
 **  rul__sql_fetch_each **
 **			 **
 *************************/
/*	
 *	The rul__sql_fetch_each routine fetches database field values and 
 *	binds them to specified RULEWORKS variables, which can then be used
 *      in one or more RHS actions within the fetch-each loop.  This is
 *      repeated for all database records selected by the fetch-each action.
 */

long  rul__sql_fetch_each (Molecule **array, 
			   long       num_var,
			   Entry_Data eb_data)
     /* address of ptr to returned array of values (Molecules) */
     /* # of variables to which field values are to be bound */
{
#ifdef TEST
  printf ("\n Now in routine rul__sql_fetch_each ... \n");
#endif

  return RUL_SQL_SUCCESS;		/* End of rul__sql_fetch_each */
}



/******************************
 **			     **
 **  rul__sql_fetch_cleanup  **
 **			     **
 *****************************/

long  rul__sql_fetch_cleanup (Molecule *array)
     /* ptr to array of WME-IDs or Molecules to bind to 
      * variables.  Call argument should be:
      *	    rul__sql_fetch_cleanup (array)
      * since array is declared as a pointer in caller.
      */
{
  long     i;                              /* scratch counter, eg for index */
  long     sql_status;                     /* return status from SQL mod rtn */

  /*=======================================================================*/

#ifdef TEST
  printf ("\n Now in routine rul__sql_fetch_cleanup ... \n");
#endif
  return RUL_SQL_SUCCESS;
}				    /* End of rul__sql_fetch_cleanup */



/**********************
 **		     **
 **  rul__sql_insert **
 **		     **
 *********************/
/*
 *	This module contains the routine rul__sql_insert, which performs a
 *	database insert operation to create a new DB record, using the 
 *	supplied data as input.
 */

long  rul__sql_insert (char *table,/* table into which insert will be done */
		       char *set,  /* includes lists of fields and of values */
		       Entry_Data eb_data)
{
  long    sql_status;                     /* return status from SQL mod rtn */
  char    sql_stmt[MAX_SQL_STMT_SIZE+1];  /* SQL statement, eg to be exec */

  /*=======================================================================*/
#ifdef TEST
  printf ("\n Now in routine rul__sql_insert ... \n");
#endif
  return RUL_SQL_SUCCESS;
}				    /* End of rul__sql_insert */



/************************
 **		       **
 **  rul__sql_rollback **
 **		       **
 ***********************/
/*
 *	This module contains the routine rul__sql_rollback, which terminates
 *	the current database transaction, if one is active, thus undoing any
 *	changes that may have been made to the DB contents during the 
 *	transaction.
 */

long  rul__sql_rollback (void)
{
  long     sql_status;                     /* return status from SQL mod rtn */

  /*======================================================================*/
#ifdef TEST
  printf ("\n Now in routine rul__sql_rollback ... \n");
#endif

  return RUL_SQL_SUCCESS;
}			    /* End of routine rul__sql_rollback */

/**********************
 **		     **
 **  rul__sql_start  **
 **		     **
 *********************/
/*
 *	This module contains the routine rul__sql_start, which starts the 
 *	requested type of database transaction (default is READ ONLY).
 */

long  rul__sql_start (char *txn_type)
{
  long     len;                            /* length of a string */
  long     sql_status;                     /* return status from SQL mod rtn */
  char     sql_stmt[MAX_SQL_STMT_SIZE+1];  /* SQL statement, eg to be exec */

  /*=======================================================================*/
#ifdef TEST
  printf ("\n Now in routine rul__sql_start ... \n");
#endif

  /* Take a look at input arguments
   *    acceptable number of args are 0 or 1:
   *	0 => start read only transaction, by default
   *	1 => start requested txn type, with any specified options
   *
   * If a null argument only was passed, then the default action is to start 
   * a READ ONLY transaction: 
   */

  return RUL_SQL_SUCCESS;
}				/* End of routine rul__sql_start */



/***********************
 **		      **
 **  rul__sql_update  **
 **		      **
 **********************/
/*
 *	This module contains the routine rul__sql_update, which modifies the 
 *	contents of selected database records according to the field names
 *	and new data values supplied.
 */

long  rul__sql_update (char *table,   /* table whose records will be updated */
		       char *set,     /* field+value pairs, and which recs */
		       Entry_Data eb_data)
{
  long     sql_status;                     /* return status from SQL mod rtn */
  char     sql_stmt[MAX_SQL_STMT_SIZE+1];  /* SQL statement, eg to be exec */

  /*=======================================================================*/
#ifdef TEST
  printf ("\n Now in routine rul__sql_update ... \n");
#endif

  /*-----------------------------------------------------------------------*/
  /* Take a look at input arguments
   *   acceptable number of args is 2:
   *	arg 1 => name of DB table in which records are to be updated
   *	arg 2 => SET expression, with optional WHERE clause
   *
   * First arg specifies table name; if it is not provided, then abort update:
   */
#if 0
  if ((table == 0) ||                             /* if null pointer, or */
      (strspn (table, " ") >= strlen (table))) {  /*  empty or null string */
    rul___sql_message (SQL_SQLINVTAB, NULL);
    return RUL_SQL_ERROR;
  }
#endif

  /*-----------------------------------------------------------------------*/
  /* Second arg contains the fields to be updated, and their new values, and
   * optionally a WHERE clause to define which records are to be updated; 
   * if it is not provided, then abort the update: 
   */
  return RUL_SQL_SUCCESS;
}		/* End of routine rul__sql_update */



/*
 *	This module contains routines to perform writes to the database
 *	based on the contents of a specified WME.  These DB writes may be 
 *	either inserts of new DB records or updates of existing records.
 *	
 *	    - rul__sql_insert_from_wme
 *	    - rul__sql_update_from_wme
 */
/********************************
 **			       **
 **  rul__sql_insert_from_wme  **
 **	 		       **
 *******************************/

long  rul__sql_insert_from_wme (Molecule wme_id,
				Entry_Data eb_data)
{
#ifdef TEST
  printf ("\n Now in routine rul__sql_insert_from_wme ... \n");
#endif
  return RUL_SQL_SUCCESS;
}			/* end of routine rul__sql_insert_from_wme */



/********************************
 **			       **
 **  rul__sql_update_from_wme  **
 **			       **
 ********************************/

long  rul__sql_update_from_wme (Molecule  wme_id,
				char     *where,
				Entry_Data eb_data)
{
#ifdef TEST
  printf ("\n Now in routine rul__sql_update_from_wme ... \n");
#endif
  return RUL_SQL_SUCCESS;
}			/* end of routine rul__sql_update_from_wme */
