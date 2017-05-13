/*
 * rts_sql.c - RULEWORKS RTS SQL interface routines
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
 *	RULEWORKS SQL interface (run-time system)
 *
 * ABSTRACT:
 *	This module contains the rul__sql_* routines called by the
 *	generated code.
 *
 *
 * MODIFIED BY:
 *	DEC	Digital Equipment Corporation
 *	CPQ	Compaq Computer Corporation
 *
 * MODIFICATION HISTORY:
 *
 *	13-Sep-1991	DEC	Initial version
 *
 *	23-Oct-1991	DEC	Don't #include sql_var.h; use local variables.
 *
 *	 7-Apr-1993	DEC	Changes for TIN
 *
 *	01-Dec-1999	CPQ	Release with GPL
 */

#include <common.h>
#include <atom.h>
#include <rac.h>
#include <mol.h>
#include <sql_msg.h>
#include <sql_p.h>
#include <sql.h>
#include <callback.h>

#include <starlet.h>	/* sys$asctim */
#include <dsc.h>	/* date field */

#ifdef TEST
#include <lib$routines.h>
long     lib$init_timer ();          /*  library timer routines, etc */
long     lib$show_timer ();
long     timer_cb_ptr[2] = {0, 0};
#endif

/*
 * Declare global SQL variables: SQLDAs -  structs defined sqlsrv.h
 *  for fetching field types from sys table
*/
GLOBAL  struct  SQLDA  *SQLDA_OUT;
GLOBAL  struct  SQLDA  *SQLDA_SYSTBL_IN;
GLOBAL  struct  SQLDA  *SQLDA_SYSTBL_OUT;
GLOBAL  long	RUL_SQL__GL_TXN_STATUS;	  /* transaction status */
GLOBAL  long	RUL_SQL__GL_SYSTBL_STATUS; /* Status of RSE accessing sys tab */





/**********************
 **		     **
 **  rul__sql_attach **
 **		     **
 **********************/
/*
 *	Declares the database schema which is to be attached to.
 *	The schema may be identified by its filename (default) or
 *	its CDD pathname, and the caller may optionally specify
 *	DBKEY scope (ie duration of validity) to be either for
 *	the duration of each transaction (default) or for the
 *	duration of the attachment to the database.
 */

long rul__sql_attach (long  filenamep,	   /* bool, pathname vs file 2nd arg */
		      char *schema_source, /* schema filename or path */
		      char *scope)	   /* scope of database keys */
{
  char    schema_declaration[MAX_SCHEMA_DECL_SIZE+1]; /* SQL schema decl */
  long    first_nonblank;	    /* offset to 1st non-blank char in a str */
  long    len;                    /* scratch length, eg of string */
  long    i;                      /* scratch counter, eg for index */
  long    sql_status;             /* return status from SQL procedures */

/*===========================================================================*/

#ifdef TEST
  printf ("\n Now in routine rul__sql_attach ... \n");
#endif

  /* At each attach, initialize the cache of field type values for field+table
   * combination, for a specific database (attachment); this just ensures that
   * each attachment a new cache of values is built:
   */

  rul___sql_init_cache ();

  /* Take a look at input arguments:
   *	arg 1 => " { FILENAME (default) | PATHNAME }  DB-NAME"
   *	arg 2 => " { ATTACH | TRANSACTION } "  (default = TRANSACTION)
   *
   * First two args must always be provided by caller;
   * they specifiy the schema source.
   * Check that 2st arg at least appears to be valid:
   */

  if ((schema_source == 0) ||		/* if null ptr, or empty/null string */
      (strspn (schema_source, " ") >= (strlen (schema_source)))) {
    rul___sql_message (SQL_SQLINVSCH, NULL);
    return RUL_SQL_ERROR;
  }

  if (filenamep) {
    strcpy (schema_declaration, "DECLARE SCHEMA FILENAME ");
  }
  else {
    strcpy (schema_declaration, "DECLARE SCHEMA PATHNAME ");
  }

  /* Form schema declaration so far, eg "DECLARE SCHEMA FILENAME my-db" */

  strcat (schema_declaration, schema_source);

  /*------------------------------------------------------------------------*/
  /* Arg3 is the DBKEY scope, which may be either the ATTACH or TRANSACTION 
   * keywords only:
   */    

  if ((scope != 0) &&				    /* if not null ptr, and */
      (strspn (scope, " ") < (len = strlen(scope)))) { /* not empty/null str */

    /* If non-null scope provided, then check that either ATTach or 
     * TRANsaction scoping was specified (force uppercase of scope arg, so 
     * can spot these keywords if present), and abort if not, or append to 
     * schema declaration statement if scope keyword was valid: 
     */

    $UPCASE (scope, i, len);

    first_nonblank = strspn (scope, " ");
    if ((strncmp (&scope[first_nonblank], "ATT", 3) != 0) &&
	(strncmp (&scope[first_nonblank], "TRAN", 4) != 0)) {
      rul___sql_message (SQL_SQLINVDBK, NULL);
      return RUL_SQL_ERROR;
    }
    else {					/* valid DBKEY scope keyword */
      strcat (schema_declaration, " DBKEY SCOPE IS ");
      strcat (schema_declaration, scope);
    }
  }

#ifdef TEST
  printf ("\n\n Schema declaration = \"%s\" \n\n", schema_declaration);
#endif

  /*-----------------------------------------------------------------------*/
  /* Since SQL "execute immediate" is used in rul__sqlmod_exec, then any 
   * prepared statements are released implicitly.  Access to the system 
   * tables (ie to get a field type) uses such prepared statements, so set 
   * this "statement prepared" status to zero, before this call: 
   */

  RUL_SQL__GL_SYSTBL_STATUS = FALSE;

  /* Now execute the attach: */

#ifdef TEST
  printf ("\nPerforming the attach ...\n");
  lib$init_timer (&timer_cb_ptr[1]);
#endif

  rul__sqlmod_exec (&sql_status, schema_declaration);

  if (sql_status != SQL_SUCCESS) {
    rul___sql_message (SQL_SQLATTFAI, &RDB$MESSAGE_VECTOR);
    return RUL_SQL_ERROR;
  }

#ifdef TEST
  lib$show_timer (&timer_cb_ptr[1]);
#endif

  /*-------------------------------------------------------------------------*/
  /* Clear the txn status, since at attach there is no active transaction
   * by definition, and then return with success status:
   */
  
  RUL_SQL__GL_TXN_STATUS = RUL_SQL_NO_TXN;
  return RUL_SQL_SUCCESS;
}					/* End of rul__sql_attach */



/**********************
 **		     **
 **  rul__sql_commit **
 **		     **
 *********************/
/*
 *	Ccommits the current SQL transaction (if there is one active), 
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

  /*-----------------------------------------------------------------------*/
  /* Since SQL "execute immediate" is used in rul__sqlmod_exec, then any 
   * prepared statements are released implicitly.  Access to the system 
   * tables (ie to get a field type) uses such prepared statements, so set 
   * this "statement prepared" status to zero, before this call:
   */

  RUL_SQL__GL_SYSTBL_STATUS = FALSE;

  /* Now execute the delete: */

#ifdef TEST
  lib$init_timer (&timer_cb_ptr[1]);
#endif

  rul__sqlmod_exec (&sql_status, sql_stmt);

  if (sql_status != SQL_SUCCESS) {
    rul___sql_message (SQL_SQLDELFAI, &RDB$MESSAGE_VECTOR);
    return RUL_SQL_ERROR;
  }

#ifdef TEST
  lib$show_timer (&timer_cb_ptr[1]);
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
  long     sql_status;                     /* return status from SQL mod rtn */
  long     status;                         /* generic return status */

  /*=======================================================================*/
#ifdef TEST
  printf ("\n Now in routine rul__sql_detach ... \n");
#endif

  /* Perform the "detach", ie call rul__sqlmod_detach */

  rul__sqlmod_detach (&sql_status);

  if (sql_status != SQL_SUCCESS) {
    rul___sql_message (SQL_SQLDETFAI, &RDB$MESSAGE_VECTOR);
    return RUL_SQL_ERROR;
  }

#ifdef TEST
  lib$show_timer (&timer_cb_ptr[1]);
#endif

  /*-----------------------------------------------------------------------*/
  /* If space for a SQLDA was allocated, free it here (we're done with it; 
   * also note that C rtl routine "free" returns 0 if successful and -1 
   * otherwise). 
   */

  if (SQLDA_OUT) {    
#ifdef TEST
    printf ("\n\n  Deallocating space for SQLDA_OUT ... \n\n");
#endif
    rul__mem_free (SQLDA_OUT);
    SQLDA_OUT = 0;	    /* Init SQLDA pointer after freeing the space */
  }
#ifdef TEST
  else {	/* if the SQLDA was not allocated */
    printf ("\n\n  Note: SQLDA_OUT not currently allocated, so not freed\n\n");
  }
#endif

  /* Next free the SQLDAs used for access to system tables, eg for get field 
   * type:
   */

  if (SQLDA_SYSTBL_OUT)	{    
#ifdef TEST
    printf ("\n\n  Deallocating space for SQLDA_SYSTBL_OUT ... \n\n");
#endif
    rul__mem_free (SQLDA_SYSTBL_OUT);
    SQLDA_SYSTBL_OUT = 0;   /* Init SQLDA pointer after freeing the space */
  }
#ifdef TEST
  else {	/* if the SQLDA was not allocated */
    printf ("\n\n  Note: SQLDA_SYSTBL_OUT not currently allocated, so not freed \n\n");
  }
#endif

  if (SQLDA_SYSTBL_IN) {    
#ifdef TEST
    printf ("\n\n  Deallocating space for SQLDA_SYSTBL_IN ... \n\n");
#endif
    rul__mem_free (SQLDA_SYSTBL_IN);
    SQLDA_SYSTBL_IN = 0;    /* Init SQLDA pointer after freeing the space */
  }
#ifdef TEST
  else {	/* if the SQLDA was not allocated */
    printf ("\n\n  Note: SQLDA_SYSTBL_IN not currently allocated, so not freed \n\n");
  }
#endif

  /*-----------------------------------------------------------------------*/
  /* Clear any cache, eg of database field types, built during the DB 
   * attachment being ended here; this will free the space allocated for the 
   * cache:
   */

  status = rul___sql_free_cache ();
  if (!status) {
    rul___sql_message (SQL_SQLCACFRE, NULL);
  }

  /* Clear status values (ie set = 0), ie if we're detaching from the DB,
   * there can be no active transaction, and the system table access statement
   * must be re-prepared if its ever used again (if re-attached later).
   * Also return success status:
   */

  RUL_SQL__GL_TXN_STATUS = RUL_SQL_NO_TXN;/* no active transaction */
  RUL_SQL__GL_SYSTBL_STATUS = FALSE;	/* sys table access RSE not prepared */

  return RUL_SQL_SUCCESS;
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
  char    uc_rse[MAX_RSE_SIZE+1];    /* scratch copy of input arg string,
					forced to uppercase */
  void   *fld_val_ptr;		     /* ptr to any type field value too big
					to fit in the fld_val array itself,
					will allocate space for those val */
  long    i;                         /* scratch counter, eg for index */
  long    len;                       /* length of a string */
  long    sql_status;                /* return status from SQL mod rtn */
  char   *token;                     /* token, eg a substring */
  long	  flen;			     /* length of field's name  */

  /*=======================================================================*/
#ifdef TEST
  printf ("\n Now in routine rul__sql_fetch_setup ... \n");
#endif

  /* Init a couple of items that are checked in the fetch cleanup routine, to 
   * determine what space (allocated here in setup or other fetch routines)
   * needs to be freed as part of the cleanup; do this here in case we abort
   * out of this or other fetch routine and go to cleanup:
   */

  fetch_num_fld = 0;

  /* Make a scratch copy of the RSE arg (since tokenizing done below will
   * damage the string), and then force the copy to uppercase (so that we
   * can easily find its contained FROM clause):
   */

  if (strlen (rse) > MAX_RSE_SIZE) {
    rul___sql_message (SQL_SQLSTMLEX, NULL);
    return RUL_SQL_ERROR;
  }
  strcpy (uc_rse, rse);       /* scratch copy of input RSE string */
  len = strlen (uc_rse);
  $UPCASE (uc_rse, i, len);

#ifdef TEST
  printf ("\n      Input RSE arg (uppercased) was:\n\n    '%s'\n", uc_rse);
#endif

  /* For FETCH_TO_WME we'll need to extract the WME type of the new WMEs to 
   * be made from fetched data from the input RSE string (it will just be 
   * the table name from which records are fetched).  Step thru uppercase 
   * copy of the rse input arg string (ie uc_rse) with the strtok routine 
   * until "FROM" is found, or until we run out of uc_rse.  Then pick up 
   * (1st) table name (ie following FROM).  Another reason for using the 
   * uc_rse copy of input arg is that strtok inserts nulls into its first 
   * argument.
   */

  fetch_table[0] = '\0';             /* init WME type to zero length string */

  token = (char *) strtok (uc_rse, " ,");
  /* until table name is found, or  end of uc_rse reached  */
  while ((fetch_table[0] == '\0') && (token)) {

    if (strcmp (token, "FROM") == 0) {        /* if FROM token was found */
	                                      /*   then what's next token? */
      token = (char *) strtok ('\0', " ,");   /* note null 1st arg */
      strcpy (fetch_table, token);
    }
    else {           /* not at FROM yet, so try next token */
      token = (char *) strtok ('\0', " ,");	/* note null 1st arg */
    }
  }

  /* Verify that a WME type was indeed found */

  if (fetch_table[0] == '\0') {
    rul___sql_message (SQL_SQLINVFRO, NULL);
    return RUL_SQL_ERROR;
  }

  /*-----------------------------------------------------------------------*/
  /* Allocate space for the SQLDA area, for data to be fetched from DB: */

  if (!SQLDA_OUT) {	/* if space not already allocated, do it now */
#ifdef TEST
    printf ("\n   Allocating space for SQLDA_OUT ... \n");
#endif
    SQLDA_OUT = (struct SQLDA *) rul__mem_malloc (sizeof(*SQLDA_OUT) + 
			(MAX_NUM_FLD * sizeof(SQLDA_OUT->SQLVARARY[0])));
    if (SQLDA_OUT == 0) {
      rul___sql_message (SQL_SQLDESALL, NULL);
      return RUL_SQL_ERROR;
    }
    SQLDA_OUT->SQLN = MAX_NUM_FLD;
  }

  /*-----------------------------------------------------------------------*/
  /* If we wanted to start a READ ONLY transaction explicitly, we'd do it 
   * here; however to allow user to perform fetch within (eg) an explicit 
   * READ WRITE transaction, just let SQL start a READ ONLY txn implicitly 
   * only if the user has not already explicitly started one elsewhere.
   * But, we do need to know which is the case, so that we can commit within 
   * this routine (if we're doing implicit READ ONLY here), or leave commit 
   * to the user if he's controlling start and commit of txn outside this 
   * routine.  Check that later after fetch completed. 
   */

  /*-----------------------------------------------------------------------*/
  /* Prepare select statement */
  
#ifdef TEST
  printf ("\nPrepare (dynamic) select statement ...\n");
  lib$init_timer (&timer_cb_ptr[1]);
#endif

  rul__sqlmod_fetch_prepare (&sql_status, rse);

  if (sql_status != SQL_SUCCESS) {
    rul___sql_message (SQL_SQLFETPRE, &RDB$MESSAGE_VECTOR);
    return RUL_SQL_ERROR;
  }

  rul__sqlmod_fetch_describe (&sql_status, SQLDA_OUT);

  if (sql_status != SQL_SUCCESS) {
    rul___sql_message (SQL_SQLFETDSC, &RDB$MESSAGE_VECTOR);
    return RUL_SQL_ERROR;
  }

#ifdef TEST
  lib$show_timer (&timer_cb_ptr[1]);
#endif

  /* Record the number of fields per record to be retrieved, plus name and 
   * Rdb data type of each field.  Note that we record the SQL data type 
   * ANDed with constant 0xFFFE to convert any odd number data type to even 
   * (the even and odd forms of the field data types are due to whether or 
   * not NULL is allowed for the field - this is Rdb/VMS specific):
   */
  
  fetch_num_fld = SQLDA_OUT->SQLD;
  if (fetch_num_fld > MAX_NUM_FLD) {
      rul___sql_message (SQL_SQLNUMFEX, NULL);
      return RUL_SQL_ERROR;
    }

#ifdef TEST
  printf ("\n  Number of fields to be fetched = %ld \n", fetch_num_fld);
#endif

  for (i = 0; i < fetch_num_fld; i++)	{
    flen = strcspn (SQLDA_OUT->SQLVARARY[i].SQLNAME, " ");
    strncpy (fetch_fld_name[i], SQLDA_OUT->SQLVARARY[i].SQLNAME, flen);
    fetch_fld_name[i][flen] = '\0';
    fetch_fld_type[i] = FFFE & SQLDA_OUT->SQLVARARY[i].SQLTYPE;

    /* Test double precision real field case, ie SQL returns same data type 
     * as for float, but we must distinguish here, since 8 bytes of memory 
     * must be allocated for the double vs 4 for the float:
     */
    
    if ((fetch_fld_type[i] == SQL_FLOAT) && 
	((len = SQLDA_OUT->SQLVARARY[i].SQLLEN) != 4)) {

      if (len == 8) {		/* double precision, ie d-float or g-float */
	fetch_fld_type[i] = SQL_DOUBLE;
      }
      else {			/* unsupported h-float, or unknown type*/
	rul___sql_message (SQL_SQLUNSFLO, NULL);
	return RUL_SQL_ERROR;
      }
    }

#ifdef TEST
    printf ("\n    Field_name[%ld] = \"%s\",  type = %ld ",
	    i, fetch_fld_name[i], fetch_fld_type[i]);
#endif

    /* For each fetched field, set the required SQLDA SQLDATA field to 
     * address of host/local variable to receive the fetched data value, 
     * and similarly for "missing" indicator host variable.  For integer, 
     * etc, data types (ie 4 bytes) where field values themselves can fit 
     * into the fld_val array, insert address of fld_val[i] in SQLDA 
     * SQLDATA field.  But for data types too large to fit there data in 
     * fld_val array, allocate space for such data, and put address of 
     * this newly allocated space in the fld_val array.
     */

    SQLDA_OUT->SQLVARARY[i].SQLIND = &fetch_missing_ind[i];
    if ((fetch_fld_type[i] == SQL_SMALLINT) ||
	(fetch_fld_type[i] == SQL_INTEGER)) {
      SQLDA_OUT->SQLVARARY[i].SQLDATA = (unsigned char *) &fetch_fld_val[i];
    }
    else {		/* field value will not fit in fld_val array itself */
#ifdef TEST
      printf ("\n\n  Allocating space for %s field value \n", fetch_fld_name[i]);
#endif
      switch (fetch_fld_type[i]) {

      case SQL_QUADWORD:
	fld_val_ptr = (long *) rul__mem_malloc (2 * sizeof (long));
	break;

      case SQL_FLOAT:
	fld_val_ptr = (float *) rul__mem_malloc (sizeof (float));
	break;

      case SQL_DOUBLE:
	fld_val_ptr = (double *) rul__mem_malloc (sizeof (double));
	break;

      case SQL_CHAR:
	/* Allocate enough space for SQLLEN character, plus null */
	fld_val_ptr = (char *) rul__mem_malloc ((sizeof (char) * 
					  SQLDA_OUT->SQLVARARY[i].SQLLEN) + 1);
	break;

      case SQL_VARCHAR:
	/* Allocate enough space for SQLLEN character, plus byte 
	 * count (first 2 bytes) and terminal null:
	 */
	fld_val_ptr = (char *) rul__mem_malloc ((sizeof (char) * 
					  SQLDA_OUT->SQLVARARY[i].SQLLEN) + 3);
	break;

      case SQL_DATE:
	/* bin sys date */
	fld_val_ptr = (char *) rul__mem_malloc (sizeof(char) * 8);  
	break;

      default:    /* earlier switch should prevent this, but ... */
	rul___sql_message (SQL_SQLUNSDAT, NULL);
	printf ("\n\n  Error in %s: unexpected SQL data type %ld",
		"rul__sql_fetch_setup", fetch_fld_type[i]);
	printf ("\n        (for field %s at set of SQLDATA field)\n\n",
		fetch_fld_name[i]);
	return RUL_SQL_ERROR;
	
      }		/* end case on data type, for quadword, etc */

      if (fld_val_ptr == 0) {
	rul___sql_message (SQL_SQLVALALL, NULL);
	printf ("\n\n  Error in %s: problem allocating space for %s value\n\n",
		"rul__sql_fetch_setup", fetch_fld_name[i]);
	return RUL_SQL_ERROR;
      }
      else {
	fetch_fld_val[i] = fld_val_ptr;
	SQLDA_OUT->SQLVARARY[i].SQLDATA = (unsigned char *) fld_val_ptr;
      }
    }	    /* end else if one of "large" data types */
  }	/* end for all data fields to be fetched */

  /*-----------------------------------------------------------------------*/
  /* Must open the SQL cursor declared above, before records fetched: */

#ifdef TEST
  printf ("\nOpen SQL cursor ...\n");
  lib$init_timer (&timer_cb_ptr[1]);
#endif

  rul__sqlmod_fetch_open (&sql_status);

  if (sql_status != SQL_SUCCESS) {
    rul___sql_message (SQL_SQLCUROPE, &RDB$MESSAGE_VECTOR);
    return RUL_SQL_ERROR;
  }

#ifdef TEST
  lib$show_timer (&timer_cb_ptr[1]);
  printf ("\nFetch all records from cursor ...\n");
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
  Molecule  wme_id;		    /* WME-IDs in array, as Molecules */
  long	    max_elem;		    /* max # of elem in array of WME-IDs */
  long      new_size;		    /* # of bytes to allocate for new WME */
  Molecule *new_array;		    /* temp ptr to new, larger array */
  long	    first_pass;		    /* true (1) during 1st record fetch */
  long	    is_attr;		    /* true if field has corresponding attr */
  long	    non_blank;		    /* index to first non-blank char in str */
  long      loc_blank;		    /* index to first blank char */
  long      i;                      /* scratch counter, eg for index */
  long      len;                    /* length of a string */
  short     len_date;		    /* length of a date */
  long      sql_status;             /* return status from SQL mod rtn */
  long      status;                 /* generic return status */
  char      fld_str[RUL_C_MAX_SYMBOL_SIZE+1];	/* trimmed field value str */
  static char  *wme;		    /* ptr to WME in string form */
  static long   wme_size;

  /* Declare some pointers, for use when interpreting those fld_val[i] 
   * elements that are pointers to data values, rather than the data values 
   * themselves, ie for any data fetched from the database which won't fit 
   * in a longword:
   */

  char   *ptr_c;		    /* for CHAR data */
  char   *ptr_vc;		    /* for VARCHAR data */
  short  *ptr_si;		    /* for SMALLINT (aka short) data */
  double *ptr_dp;		    /* for DOUBLE (precision float) data */
  long	 *ptr_q;		    /* for QUADWORD data */
  char   *ptr_date;		    /* for (binary VMS system) DATE data */

  /* Declare a DBKEY structure (note Rdb/VMS specific): */

  struct  dbkey_struct {	    /* struct for Rdb/VMS DBKEY values */
    unsigned short  line;
    unsigned long   page;
    unsigned short  area;
  } dbkey;

  char    dbkey_line[10];	   /* text string equivalent of dbkey fields */
  char    dbkey_page[10];
  char    dbkey_area[10];

  /* Declare a fix-string descriptor for use with sys$asctim; the descriptor 
   * will be used for 23 byte date-time string (see sys$library:descrip.h):
   */

  struct dsc$descriptor date_descr = {sizeof (fld_str), DSC_K_DTYPE_T,
				      DSC_K_CLASS_S, NULL};

  /*=======================================================================*/

#ifdef TEST
  printf ("\n Now in routine rul__sql_fetch_to_wme ... \n");
#endif

  /* Init number of WMEs produced by fetch (ie number of WME-IDs in returned 
   * array) to zero: 
   */

  *num_wme = 0;

  /* Allocate space for the new WME string: */

#ifdef TEST
  printf ("\n Allocating space for WME string ... \n");
#endif

  /* Define number of bytes to allocate for the new WME, as it's put together 
   * here; this will be determined (as upper limit) from data in SQLDA, ie 
   * number of fields fetched, there type, and size (especially for char 
   * strings).  Overall, need room for WME-class name, open and close 
   * parentheses, and first space after class name, plus terminal null 
   * (WME_SPACE_MIN).  Then for each field, leave room for 
   * " ^attr-name " (WME_SPACE_PER_ATTRNAME) and either: 
   *	      - SQLLEN bytes (+2 for vertical bars) for char string fields
   *	      - WME_SPACE_PER_NIL bytes for NIL (missing) value
   *	      - WME_SPACE_PER_INTEGER bytes for INTEGER fields
   *	      - WME_SPACE_PER_SMALLINT bytes for SMALLINT (short) fields
   *	      - WME_SPACE_PER_FLOAT bytes for REAL (and DOUBLE, same SQL_type)
   *	      - WME_SPACE_PER_QUADWORD bytes for QUADWORD fields
   *	      - WME_SPACE_PER_DATE bytes for DATE fields
   */
  
  new_size = WME_SPACE_MIN;
  new_size = new_size + (fetch_num_fld * WME_SPACE_PER_ATTRNAME);

  for (i = 0; i < fetch_num_fld; i++) {

    switch (fetch_fld_type[i]) {

    case SQL_SMALLINT:
      new_size += WME_SPACE_PER_SMALLINT;
      break;

    case SQL_INTEGER:
      new_size += WME_SPACE_PER_INTEGER;
      break;

    case SQL_QUADWORD:
      new_size += WME_SPACE_PER_QUADWORD;
      break;

    case SQL_FLOAT:
      new_size += WME_SPACE_PER_FLOAT;
      break;

    case SQL_DOUBLE:
      new_size += WME_SPACE_PER_DOUBLE;
      break;

    case SQL_CHAR:		/* add 2 bytes for vertical bars */
      new_size += (2 + SQLDA_OUT->SQLVARARY[i].SQLLEN);
      break;

    case SQL_VARCHAR:	/* add 2 bytes for vertical bars */
      new_size += (2 + SQLDA_OUT->SQLVARARY[i].SQLLEN);
      break;

    case SQL_DATE:
      new_size += WME_SPACE_PER_DATE;
      break;

    default:		    /* for unexpected SQL data type */
      new_size += WME_SPACE_PER_NIL;    /* ie for NIL */
      
    }		    /* end case on SQLDA field type */
  }		    /* end for all fields, so know space to allocate */

  if (new_size > wme_size) {
    wme = (char *) rul__mem_realloc (wme, sizeof (char) * new_size);
    if (wme == 0) {
      rul___sql_message (SQL_SQLWMEALL, NULL);
      return RUL_SQL_ERROR;
    }
    wme_size = new_size;
  }

#ifdef TEST
  printf ("\n    (allocated %ld bytes for WME string) \n", new_size);
#endif

  /* Allocate space for the array of WME-IDs.  Define initial size parameter,
   * so that if necessary later, we can double, etc, that size:
   */

  max_elem = WME_ID_INIT_ARRAY_SIZE;	  /* eg init room for 1,000 WME-IDs */

#ifdef TEST
  printf ("\n\n  Allocating space for WME-ID array in fetch-to-wme ... \n");
#endif

  *array = (Molecule *) rul__mem_malloc (sizeof (Molecule) * max_elem);
  if (*array == 0) {
    rul___sql_message (SQL_SQLWIDALL, NULL);
    return RUL_SQL_ERROR;
  }    

  /*-----------------------------------------------------------------------*/
  /* Perform the fetch, one record at a time, from the open cursor: */

  first_pass = TRUE;

  sql_status = SQL_SUCCESS;

  while (sql_status == SQL_SUCCESS) {

    rul__sqlmod_fetch (&sql_status, SQLDA_OUT);

    /* Return status from fetch should be either normal/success or 
     * end-of-stream; if it is neither of these values, then abort fetch:
     */

    if ((sql_status != SQL_SUCCESS) &&  /* SQL return status not normal, */
	(sql_status != SQL_EOS)) {	    /*   and not end-of-stream */
      rul___sql_message (SQL_SQLFETCH, &RDB$MESSAGE_VECTOR);
      return RUL_SQL_ERROR;
    }

    /* If this was a normal fetch (ie SQL return status = zero), then build 
     * a WME from the fetched record's field values:
     */

    if (sql_status == SQL_EOS) {
    }

    else if (sql_status == SQL_SUCCESS) {

      /* Start forming the new WME string, beginning with WME-class, 
       * which for SQL-FETCH-TO-WME is required to be the same as the 
       * name of the table from which records are fetched (so variable 
       * "table" is used for WME-class name, and is set in fetch setup 
       * routine):
       */

      strcpy (wme, "(");	/* ie rul__make_wme wants "( ... */
      strcat (wme, fetch_table);	/* so far "(WME-class ... */

      /* Then for each field fetched, we must determine if it has a 
       * corresponding attribute in this WME-class, and if so include the
       * fetched data value (or NIL) in the new WME string, along with 
       * the attribute name (which is mapped 1-to-1 with field name in 
       * this routine, be definition).  If a fetched field has no 
       * corresponding WME attribute, set the type value for that field 
       * to zero; use this as a check during subsequent record fetches
       * (if any for this fetch execution) to avoid overhead of 
       * re-checking all the fields to look for corresponding attribute.
       * For a valid fetched field, if value was "missing", then force 
       * NIL value for the corresponding WME attribute:
       */

      for (i = 0; i < fetch_num_fld; i++) {

	/* first pass AND has matching attr OR not first pass AND has attr */
	if ((first_pass &&
	     (is_attr = rul__is_attribute (fetch_table,
					   fetch_fld_name[i], ""))) ||
	    (!first_pass && (fetch_fld_type[i] != 0))) {

	  /* If field passes either of these two tests, add attr
	   * name (ie field name) to the new WME string:
	   */
	  
	  strcat (wme, " ^");
	  strcat (wme, fetch_fld_name[i]);

	  /* If field value was "missing", then force NIL attribute 
	   * value, else use value fetched from field:
	   */

	  if (fetch_missing_ind[i] == SQL_MISSING_VALUE) {
	    strcat (wme, " NIL");
	  }
	  else {			    /* not "missing" */
	    strcat (wme, " ");
	    switch (fetch_fld_type[i]) {

	    case SQL_SMALLINT:
	      sprintf (fld_str, "%hd", (short) fetch_fld_val[i]);
	      strcat (wme, fld_str);
	      break;

	    case SQL_INTEGER:
	      sprintf (fld_str, "%d", (int) fetch_fld_val[i]);
	      strcat (wme, fld_str);
	      break;
	      
	    case SQL_QUADWORD:
	      /* convert binary quadword to string form */
	      rul___sql_quadstr ((long *)fetch_fld_val[i], fld_str);
	      strcat (wme, "|");
	      strcat (wme, fld_str);
	      strcat (wme, "|");
	      break;

	    case SQL_FLOAT:
	      sprintf (fld_str, "%.8E", *((float *) fetch_fld_val[i]));
	      strcat (wme, fld_str);	    /*  value in     */
	      break;

	    case SQL_DOUBLE:
	      sprintf (fld_str, "%.15E", *((double *) fetch_fld_val[i]));
	      strcat (wme, fld_str);	    /*  value in     */
	      break;

	    case SQL_CHAR:
	      /* SQL will consider both DBKEY and normal char
	       * string fields as "character" data, so must
	       * handle both here; start with non-DBKEY:
	       */
	      if (strcmp (fetch_fld_name[i], "DBKEY")) { /*not DBKEY*/
		ptr_c = (char *) fetch_fld_val[i];
		/* enclose field val in vert bars */
		strcat (wme, "|");	  
		strncat (wme, ptr_c, SQLDA_OUT->SQLVARARY[i].SQLLEN);
		strcat (wme, "|");
	      }
	      else {		/* this is DBKEY data */
		ptr_c = (char *) fetch_fld_val[i];
		memcpy (&dbkey, ptr_c, 8);
		sprintf (dbkey_line, "%hu", dbkey.line);
		sprintf (dbkey_page, "%u", dbkey.page);
		sprintf (dbkey_area, "%hu", dbkey.area);
		strcat (wme, "|");
		strcat (wme, dbkey_area);
		strcat (wme, ":");
		strcat (wme, dbkey_page);
		strcat (wme, ":");
		strcat (wme, dbkey_line);
		strcat (wme, "|");
	      }
	      break;

	    case SQL_VARCHAR:
	      /* note that first 2 bytes returned by SQL 
	       * for a VARCHAR field will be byte count, so
	       * skip over that when including string in WME:
	       */
	      ptr_vc = (char *) fetch_fld_val[i];
	      ptr_si = (short *) ptr_vc;
	      len = (long) *ptr_si;
	      strncpy (fld_str, ptr_vc + 2, len);
	      fld_str[len] = '\0';
	      strcat (wme, "|");      /* enclose field */
	      strcat (wme, fld_str); /*  value in     */
	      strcat (wme, "|");      /*   vert bars   */
	      break;

	    case SQL_DATE:	    /*** Note VMS specific ***/
	      ptr_date = (char *) fetch_fld_val[i];
	      date_descr.dsc_a_pointer = fld_str;
	      status = sys$asctim (&len_date, &date_descr, ptr_date, 0);
	      if (status != SS$_NORMAL) {
		rul___sql_message (SQL_SQLTIMFET, NULL);
		return RUL_SQL_ERROR;
	      }
	      fld_str[len_date] = '\0';
	      strcat (wme, "|");	    /* enclose field */
	      strcat (wme, fld_str);	    /*  value in     */
	      strcat (wme, "|");	    /*   vert bars   */
	      break;

	    default:
	      strcat (wme, " NIL");
	      if (first_pass) {
		rul___sql_message (SQL_SQLUNSDAT, NULL);
		printf ("\n\n  Error in %s: unexpected SQL data type %ld for field %s \n",
			"rul__sql_fetch_to_wme",
			fetch_fld_type[i],
			fetch_fld_name[i]);
	      }
	    }	    /* end case on SQL field data type */
	  }	  /* end else if normal field fetch */
	}       /* end if field has corresponding attr */
	
	/* If this is still first pass through all fields, and one of 
	 * the fetched fields does NOT have a corresponding attribute 
	 * then set its fetch_fld_type array element to zero:
	 */

	if (first_pass && !is_attr) {
	  fetch_fld_type[i] = 0;
	}
      }		/* end for each field fetched */

      /* Complete the WME string, ie close parenthesis, then issue 
       * the make WME command: 
       */

      strcat (wme, ")");
      rul__atom_string_setup (wme);
      wme_id = rul___make_wmos (eb_data->db_name_count, eb_data->db_names);

      /* Then check that the array can hold the next WME-ID (for next 
       * fetch after this one); if array is full now, expand it by 
       * allocating a new, larger space and copying the  current array 
       * contents to it.  (Use memcpy to avoid possible problem with  
       * embedded nulls in array contents.)
       */

      if (*num_wme == max_elem) {	    /* array is now full, so expand */
#ifdef TEST
	printf ("\n\n Expanding space allocated for WME-ID array ...\n\n");
#endif
	max_elem += WME_ID_ARRAY_EXPANSION;
	*array = (Molecule *) rul__mem_realloc (*array,
					((sizeof (Molecule)) * max_elem));
	if (*array == 0) {
	  rul___sql_message (SQL_SQLWIDALL, NULL);
	  printf ("\n\n  Error in %s: problem allocating space for expanded WME-ID array (of %ld elements) \n\n",
		  "rul__sql_fetch_to_wme", max_elem);
	  return RUL_SQL_ERROR;
	}
      }	    /* end space check on array */
      
      /* Add this latest WME-ID to the array of them to be returned, then 
       * increment the array index (and count of WME-IDs returned).  
       * Note the order of these operations: result is first WME-ID 
       * created uses array[0] and count is then set to 1.
       */

      (*array)[*num_wme] = wme_id;    /* ie array[0] on first fetch */
      *num_wme = *num_wme + 1;	    /*   and count=1 for first fetch */	

      first_pass = FALSE;	/* done with 1st (or subsequent) pass */
    }		/* end if sql_status, ie for each fetched record */
  }	    /* end while, ie end-of-stream */

  /*-----------------------------------------------------------------------*/
#ifdef TEST
  if (*num_wme != 0)
    printf ("\n\n            (total of %ld records fetched) \n\n", *num_wme);
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
  long	    mismatch;		    /* flag for SQLDA num_fld NE num_var arg */
  Molecule  nil_atom;		    /* attr value NIL, as Molecule */
  long	    first_pass;		    /* flag set during 1st record fetch */
  long	    non_blank;		    /* index to first non-blank char in str */
  long	    loc_blank;		    /* index to first blank char */
  long      i;                      /* scratch counter, eg for index */
  long      len;                    /* length of a string */
  short     len_date;               /* length of a date string */
  long      sql_status;             /* return status from SQL mod rtn */
  long      status;                 /* generic return status */
  char      fld_str[RUL_C_MAX_SYMBOL_SIZE+1];	/* trimmed field value str */
  
  /* Declare some pointers, for use when interpreting those fld_val[i] 
   * elements that are pointers to data values, rather than the data values 
   * themselves, ie for any data fetched from the database which won't fit 
   * in a longword:
   */

  char   *ptr_c;		    /* for CHAR data */
  char   *ptr_vc;		    /* for VARCHAR data */
  short  *ptr_si;		    /* for SMALLINT (aka short) data */
  double *ptr_dp;		    /* for DOUBLE (precision float) data */
  long	 *ptr_q;		    /* for QUADWORD data */
  char   *ptr_date;		    /* for (binary VMS system) DATE data */

  /* Declare a DBKEY structure (note Rdb/VMS specific): */

  struct  dbkey_struct {    /* struct for Rdb/VMS DBKEY values */
    unsigned short  line;
    unsigned long    page;
    unsigned short  area;
  } dbkey;

  char    dbkey_line[10];	 /* text string equivalent of dbkey fields */
  char    dbkey_page[10];
  char    dbkey_area[10];

  /* Declare a fix-string descriptor for use with sys$asctim; the descriptor 
   * will be used for 23 byte date-time string (see sys$library:descrip.h):
   */
  struct dsc$descriptor date_descr = {sizeof (fld_str), DSC_K_DTYPE_T,
				      DSC_K_CLASS_S, NULL};

  /*=======================================================================*/

#ifdef TEST
  printf ("\n Now in routine rul__sql_fetch_each ... \n");
#endif

  /* Allocate space for the array of value Molecules, which will be
   * returned to caller for each execution of this routine (until
   * EOS reached); this should only be necessary on the first pass,
   * ie for first record fetched for SQL-FETCH-EACH, and we'll use
   * non-existence of array space to determine first pass (during
   * subsequent record fetches, don't want to repeat "mismatch" message).
   * Note that we only need to allocate enough space for the number
   * of variables user wants to bind values to.
   */
  
  if (*array == 0) {
#ifdef TEST
    printf ("\n\n  Allocating space for <var> array in fetch-each ... \n");
#endif
    first_pass = TRUE;
    *array = (Molecule *) rul__mem_malloc (sizeof (Molecule) * num_var);
    if (*array == 0) {
      rul___sql_message (SQL_SQLVALARR, NULL);
      return RUL_SQL_ERROR;
    }
  }
  else {	/* array was already allocated, ie not first pass */
    first_pass = FALSE;
  }

  /* Compare the number of values to be returned to caller vs the number of 
   * fields defined in the fetch SQLDA (based on RSE used during fetch setup).
   * If more fields are included in the SQLDA (ie num_fld) than field values 
   * requested (num_var), just reset num_fld to smaller num_var.  Also set 
   * a "mismatch" flag, to indicate this inequality.  If the reverse is true, 
   * leave num_fld unchanged and set the mismatch flag.  After the num_fld 
   * field values are used, the remaining values requested will have to be 
   * filled with NILs.
   */

  if (fetch_num_fld == num_var) {
    mismatch = FALSE;
  }
  else {			/* mismatched num_fld vs num_var */
    mismatch = TRUE;
    if (fetch_num_fld > num_var) {
      fetch_num_fld = num_var;
    }
  }

  /* Define a NIL Molecule, for use if a fetched field contained "missing", and
   * if more values are requested than fields fetched (and so value Molecule 
   * array would have to be padded will NIL Molecules):
   */

  nil_atom = rul__mol_symbol_nil ();

  /*-----------------------------------------------------------------------*/
  /* Perform the fetch, for the next record, from the open cursor (opened 
   * already in the setup routine: 
   */

  rul__sqlmod_fetch (&sql_status, SQLDA_OUT);

  /* Return status from fetch should be either normal/success or 
   * end-of-stream; if it is neither of these values, then abort the fetch:
   */

  if ((sql_status != SQL_SUCCESS) &&	    /* SQL return status not normal, */
      (sql_status != SQL_EOS)) {	    /*   and not end-of-stream */
    rul___sql_message (SQL_SQLFETCH, &RDB$MESSAGE_VECTOR);
    return RUL_SQL_ERROR;
  }

  /* If this was a normal fetch (ie SQL return status = zero), then convert 
   * fetched field values into RUL Molecules, place them in array, and return 
   * them to caller:
   */

  if (sql_status == SQL_SUCCESS) {

    for (i = 0; i < fetch_num_fld; i++) {

      /* If field value was "missing", then force NIL attribute value, else 
       * use value fetched from field:
       */
      
      if (fetch_missing_ind[i] == SQL_MISSING_VALUE) {
	(*array)[i] = nil_atom;
      }
      else {				    /* not "missing" */
	
	switch (fetch_fld_type[i]) {

	case SQL_SMALLINT:
	  (*array)[i] = rul__integer_to_iatom ((long) fetch_fld_val[i]);
	  break;

	case SQL_INTEGER:
	  (*array)[i] = rul__integer_to_iatom ((long) fetch_fld_val[i]);
	  break;

	case SQL_QUADWORD:
	  /* convert binary quadword to string form */
	  rul___sql_quadstr ((long *) fetch_fld_val[i], fld_str);
	  (*array)[i] = rul__string_to_symbol (fld_str);
	  break;

	case SQL_FLOAT:
	  /***??? !  Using rul__float_to_fatom would be more
	   ***???    efficient for fetch-each floats, but it introduces
	   ***???    errors due to int fld_val[i] arg where a float is
	   ***???    expected by rul__float_to_fatom.  
	   ***/
	  /*  sprintf (fld_str, "%.8E", *((float *) fetch_fld_val[i]);
	   * (*array)[i] = rul__string_to_atom (fld_str);
	   */
	  (*array)[i] = rul__float_to_fatom (*((float *) fetch_fld_val[i]));
	  break;

	case SQL_DOUBLE:
	  /* sprintf (fld_str, "%.15E", *((double *) fetch_fld_val[i]));
	   * (*array)[i] = rul__string_to_atom (fld_str);
	   */
	  (*array)[i] = rul__double_to_fatom (*((double *) fetch_fld_val[i]));
	  break;

	case SQL_CHAR:
	  /* SQL will consider both DBKEY and normal char string 
	   * fields as "character" data, so must handle both here; 
	   * start with non-DBKEY:
	   */
	  if (strcmp (fetch_fld_name[i], "DBKEY")) {		/*not DBKEY*/
	    ptr_c = (char *) fetch_fld_val[i];
	    (*array)[i] = rul__string_to_symbol (ptr_c /*,
				       SQLDA_OUT->SQLVARARY[i].SQLLEN */);
	  }
	  else {			    /* this is DBKEY data */
	    ptr_c = (char *) fetch_fld_val[i];
	    memcpy (&dbkey, ptr_c, 8);
	    sprintf (dbkey_line, "%hu", dbkey.line);
	    sprintf (dbkey_page, "%u", dbkey.page);
	    sprintf (fld_str, "%hu", dbkey.area);
	    strcat (fld_str, ":");
	    strcat (fld_str, dbkey_page);
	    strcat (fld_str, ":");
	    strcat (fld_str, dbkey_line);
	    (*array)[i] = rul__string_to_symbol (fld_str /*,strlen(fld_str)*/);
	  }
	  break;

	case SQL_VARCHAR:
	  /* note that first 2 bytes returned by SQL for a VARCHAR 
	   * field will be byte count, so skip over that when 
	   * including string in WME:
	   */
	  ptr_vc = (char *) fetch_fld_val[i];
	  ptr_si = (short *) ptr_vc;	
	  len = (long) *ptr_si;
	  strncpy (fld_str, ptr_vc + 2, len);
	  fld_str[len] = '\0';
	  (*array)[i] = rul__string_to_symbol (fld_str /*, strlen(fld_str)*/ );
	  break;

	case SQL_DATE:
	  ptr_date = (char *) fetch_fld_val[i];
	  date_descr.dsc_a_pointer = fld_str;
	  status = sys$asctim (&len_date, &date_descr, ptr_date, 0);
	  if (status != SS$_NORMAL) {
	    rul___sql_message (SQL_SQLTIMFET, NULL);
	    return RUL_SQL_ERROR;
	  }
	  fld_str[len_date] = '\0';
	  (*array)[i] = rul__string_to_symbol (fld_str /*, strlen(fld_str)*/);
	  break;

	default:
	  (*array)[i] = nil_atom;
	  if (first_pass) {
	    rul___sql_message (SQL_SQLUNSDAT, NULL);
	    printf ("\n\n  Error in %s: unexpected SQL data type %ld for field %s \n",
		    "rul__sql_fetch_each", fetch_fld_type[i], 
		    fetch_fld_name[i]);
	  }
	}	    /* end case on SQL field data type */
      }		  /* end else if data not missing */
    }	        /* end for all fields fetched */
  }	      /* end if normal fetch, sql_status == SQL_SUCCESS */


  /*-----------------------------------------------------------------------*/
  /* If instead fetch return status was end-of-stream, then there are no more 
   * records, so no values can be returned.  Return EOS status to caller.
   */
  
  else {	    /* end-of-stream was reached */
    return SQL_EOS;	    /* SQL_EOS = 100 for Rdb/VMS SQL */
  }

  /*-----------------------------------------------------------------------*/
  /* If num_var is greater than num_fld (ie number of fetched field values), 
   * then must now force NIL value for those excess values:
   */
  
  if (num_var > fetch_num_fld) {
    for (i=fetch_num_fld; i<num_var; i++) {
      (*array)[i] = nil_atom;
    }
  }

  /* And let user know if either type of mismatch occurred, between num_var 
   * and fetch_num_fld:
   */

  if (mismatch) {
    rul___sql_message (SQL_SQLMISNUM, NULL);
  }

  /*-----------------------------------------------------------------------*/
  /* All done; return success status: */

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

  rul__sqlmod_fetch_close (&sql_status);
  if (sql_status != SQL_SUCCESS) {
    rul___sql_message (SQL_SQLCURCLO, &RDB$MESSAGE_VECTOR);
  }

  /*-----------------------------------------------------------------------*/
  /* Commit the transaction now ONLY if SQL started one for us in this 
   * routine, but not if the user had explicitly started one elsewhere, 
   * ie via start_txn.  Use RUL_SQL__GL_TXN_STATUS to test which is the case: 
   * implicit or explicit txn. If the txn_status is 0, then implict txn 
   * must have been started here, so commit:
   */

  if (!RUL_SQL__GL_TXN_STATUS) {
#ifdef TEST
    printf ("\n\n Commit implicit read only transaction ... \n");
#endif
    rul__sqlmod_commit (&sql_status);
    if (sql_status != SQL_SUCCESS) {
      rul___sql_message (SQL_SQLCOMFAI, &RDB$MESSAGE_VECTOR);
      rul__sqlmod_rollback (&sql_status);
    }
  }

  /*-----------------------------------------------------------------------*/
  /* Free space allocated earlier (in fetch_each or fetch_to_wme) for array 
   * of WME-IDs or values as Molecules:
   */

  if (array != 0) {
    rul__mem_free (array);
    array = 0;
  }

  /*-----------------------------------------------------------------------*/
  /* Then free space allocated for larger than 4 byte field values: */

  for (i = 0; i < fetch_num_fld; i++) {
    if ((fetch_fld_type[i] != SQL_SMALLINT) &&
	(fetch_fld_type[i] != SQL_INTEGER)  &&
	(fetch_fld_val[i] != 0)) {	/* and ptr to actual value not null */
      rul__mem_free ((void *) fetch_fld_val[i]);
      fetch_fld_val[i] = 0;
    }		    /* end if one of larger than 4 byte field values */
  }		/* end for all fields fetched */

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

  /* Verify that user has explicitly started a (READ WRITE) transaction; if 
   * not then abort the insert before SQL does.
   */

  if (!RUL_SQL__GL_TXN_STATUS) {
    rul___sql_message (SQL_SQLNOTXN, NULL);
    return RUL_SQL_ERROR;
  }

  /*-----------------------------------------------------------------------*/
  /* Take a look at input arguments
   *	arg 1 => name of DB table in which records are to be inserted
   *	arg 2 => field list + value list expression
   *
   * First arg specifies table name; if it is not provided, then abort insert: 
   */

  if ((table == 0) ||                             /* if null pointer, or */
      (strspn (table, " ") >= strlen (table))) {  /*  empty or null string */
    rul___sql_message (SQL_SQLINVTAB, NULL);
    return RUL_SQL_ERROR;
  }

  /* If table name arg appears okay, form SQL statement as far as can now: */

  strcpy (sql_stmt, "INSERT INTO ");
  strcat (sql_stmt, table);
  strcat (sql_stmt, " ");

  /*-----------------------------------------------------------------------*/
  /* Second arg contains the insert "set" clause, ie the fields to be 
   * inserted, and their new values; if it is not provided, then abort the 
   * insert:
   */

  if ((set == 0) ||                             /* if null pointer, or */
      (strspn (set, " ") >= strlen (set))) {    /*  empty or null string */
    rul___sql_message (SQL_SQLINVSET, NULL);
    return RUL_SQL_ERROR;
  }

  /* If the field+value lists expression appears okay, append it to the SQL 
   * statement: 
   */

  if ((strlen (sql_stmt) + strlen (set)) >= MAX_SQL_STMT_SIZE) {
    rul___sql_message (SQL_SQLSTMLEX, NULL);
    return RUL_SQL_ERROR;
  }
  strcat (sql_stmt, set);

#ifdef TEST
  printf ("\n\n Insert stmt = \"%s\" \n\n", sql_stmt);
#endif

  /*-----------------------------------------------------------------------*/
  /* Almost ready to perform the insert, but firt ...
   *
   * Since SQL "execute immediate" is used in rul__sqlmod_exec, then any 
   * prepared statements are released implicitly.  Access to the system tables 
   * (ie to get a field type) uses such prepared statements, so set this 
   * "statement prepared" status to zero, before this call:
   */

  RUL_SQL__GL_SYSTBL_STATUS = FALSE;

  /* Now execute the insert: */

#ifdef TEST
  lib$init_timer (&timer_cb_ptr[1]);
#endif

  rul__sqlmod_exec (&sql_status, sql_stmt);
  
  if (sql_status != SQL_SUCCESS) {
    rul___sql_message (SQL_SQLINSFAI, &RDB$MESSAGE_VECTOR);
    return RUL_SQL_ERROR;
  }

#ifdef TEST
  lib$show_timer (&timer_cb_ptr[1]);
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

  /* Rollback only if there is an active transaction, else signal user */

  if (RUL_SQL__GL_TXN_STATUS) {
    rul__sqlmod_rollback (&sql_status);
    if (sql_status != SQL_SUCCESS) {
      rul___sql_message (SQL_SQLROLFAI, &RDB$MESSAGE_VECTOR);
      return RUL_SQL_ERROR;
    }
  }

  else {	/* no currently active transaction, signal error to user */
    rul___sql_message (SQL_SQLROLIGN, NULL);
  }

  RUL_SQL__GL_TXN_STATUS = RUL_SQL_NO_TXN;
  return RUL_SQL_SUCCESS;
}			    /* End of routine rul__sql_rollback */



/*********************
*  SQL RSE ROUTINES  *
*********************/
/*
  The following special SQL RSE conversion routines are defined in this file:

  RUL__SQL_RSE_INIT ();
  RUL__SQL_RSE_SYMBOL (Molecule, quote-flag);
  RUL__SQL_RSE ();

  The SQL RSE functions do not deallocate the buffer used to build
  the RSE string, but instead use static variables to address it:

  rse_buffer		- pointer to the buffer
  rse_buffer_index;	- next unused char in the buffer
  rse_buffer_size;	- number of characters in buffer
  RSE_BUFFER_INCR	- size buffer is incremented by when overflowed
*/

#define RSE_BUFFER_INCR 1024
static	char *rse_buffer;
static	long rse_buffer_index /* = 0 */;  /* Initialized by linker to 0 */
static	long rse_buffer_size /* = 0 */;   /* Initialized by linker to 0 */

static void check_rse_buffer_size (long space_needed)
{
  if ((rse_buffer_size - rse_buffer_index) < space_needed) {
    rse_buffer_size = rse_buffer_size + MAX (RSE_BUFFER_INCR, space_needed);
    rse_buffer = (char *) rul__mem_realloc (rse_buffer, rse_buffer_size);
  }
}

/*********************
 *                   *
 * RUL__SQL_RSE_INIT *
 *                   *
 ********************/
/*
 *    Initialize the External SQL RSE buffer to begin new RSE build
 */

void rul__sql_rse_init ()
{
  rse_buffer_index = 0;
}

/***********************
 *                     *
 * RUL__SQL_RSE_SYMBOL *
 *                     *
 **********************/
/*
 * Adds a symbol's printform representation to the SQL RSE buffer
 */

void rul__sql_rse_symbol (Molecule mol, Boolean quoted)
{
  long  prt_len, tot_len;

  tot_len = prt_len = rul__mol_get_printform_length (mol);

  if (quoted)
    check_rse_buffer_size (prt_len + 3);
  else
    check_rse_buffer_size (prt_len + 1);

  if (quoted) {
    rse_buffer[rse_buffer_index] = '\'';
    rse_buffer_index = rse_buffer_index + 1;
  }

  rul__mol_use_printform (mol, &rse_buffer[rse_buffer_index],
			  rse_buffer_size - rse_buffer_index);
  rse_buffer_index += prt_len;

  if (quoted) {
    rse_buffer[rse_buffer_index] = '\'';
    rse_buffer_index = rse_buffer_index + 1;
  }

  rse_buffer[rse_buffer_index] = ' ';
  rse_buffer_index = rse_buffer_index + 1;
}

/****************
 *              *
 * RUL__SQL_RSE *
 *              *
 ***************/
/*
 * Returns the SQL RSE string
 */

char * rul__sql_rse ()
{
  check_rse_buffer_size (1);
  rse_buffer[rse_buffer_index] = '\0';
  return rse_buffer;
}



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

  if ((txn_type == 0) ||         /* if null ptr, or empty/null string */
      (strspn (txn_type, " ") >= (len = strlen (txn_type)))) {
    strcpy (sql_stmt, "SET TRANSACTION READ ONLY");
#ifdef TEST
    printf ("\n Starting (default) RO transaction ... \n");
#endif
  }

  else {   /* an input arg was provided, so use to define txn type + opt */
    strcpy (sql_stmt, "SET TRANSACTION ");
    strcat (sql_stmt, txn_type);
  }

  /*-----------------------------------------------------------------------*/
  /* Start the requested transaction; but first ...
   * 
   * Since SQL "execute immediate" is used in rul__sqlmod_exec, then any 
   * prepared statements are released implicitly.  Access to the system tables 
   * (ie to get a field type) uses such prepared statements, so set this 
   * "statement prepared" status to zero, before this call:
   */

  RUL_SQL__GL_SYSTBL_STATUS = FALSE;

  /* Now execute the start txn: */

#ifdef TEST
  printf ("\n Starting requested transaction ... \n");
#endif

  rul__sqlmod_exec (&sql_status, sql_stmt);
  if (sql_status != SQL_SUCCESS) {
    rul___sql_message (SQL_SQLSTAFAI, &RDB$MESSAGE_VECTOR);
    RUL_SQL__GL_TXN_STATUS = RUL_SQL_NO_TXN;		/* txn not started */
    return RUL_SQL_ERROR;
  }

  RUL_SQL__GL_TXN_STATUS = RUL_SQL_TXN_ACTIVE;		/* txn is active */

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

  /* Verify that user has explicitly started a (READ WRITE) transaction; if 
   * not then abort the insert before SQL does.
   */

  if (!RUL_SQL__GL_TXN_STATUS) {
    rul___sql_message (SQL_SQLNOTXN, NULL);
    return RUL_SQL_ERROR;
  }

  /*-----------------------------------------------------------------------*/
  /* Take a look at input arguments
   *   acceptable number of args is 2:
   *	arg 1 => name of DB table in which records are to be updated
   *	arg 2 => SET expression, with optional WHERE clause
   *
   * First arg specifies table name; if it is not provided, then abort update:
   */

  if ((table == 0) ||                             /* if null pointer, or */
      (strspn (table, " ") >= strlen (table))) {  /*  empty or null string */
    rul___sql_message (SQL_SQLINVTAB, NULL);
    return RUL_SQL_ERROR;
  }

  /* If table name arg appears okay, form SQL statement as far as can now: */

  strcpy (sql_stmt, "UPDATE ");
  strcat (sql_stmt, table);
  strcat (sql_stmt, " ");

  /*-----------------------------------------------------------------------*/
  /* Second arg contains the fields to be updated, and their new values, and
   * optionally a WHERE clause to define which records are to be updated; 
   * if it is not provided, then abort the update: 
   */

  if ((set == 0) ||                             /* if null pointer, or */
      (strspn (set, " ") >= strlen (set))) {    /*  empty or null string */
    rul___sql_message (SQL_SQLINVSET, NULL);
    return RUL_SQL_ERROR;
  }

  /* If the SET expression appears okay, append it to the SQL statement: */

  if ((strlen (sql_stmt) + strlen (set)) >= MAX_SQL_STMT_SIZE) {
    rul___sql_message (SQL_SQLSTMLEX, NULL);
    return RUL_SQL_ERROR;
  }
  strcat (sql_stmt, set);

#ifdef TEST
  printf ("\n\n Update stmt = \"%s\" \n\n", sql_stmt);
#endif

  /*-----------------------------------------------------------------------*/
  /* Perform the update; but first ...
   *
   * Since SQL "execute immediate" is used in rul__sqlmod_exec, then any 
   * prepared statements are released implicitly.  Access to the system tables 
   * (ie to get a field type) uses such prepared statements, so set this 
   * "statement prepared" status to zero, before this call:
   */

  RUL_SQL__GL_SYSTBL_STATUS = FALSE;

  /* Now execute the update: */

#ifdef TEST
  lib$init_timer (&timer_cb_ptr[1]);
#endif

  rul__sqlmod_exec (&sql_status, sql_stmt);

  if (sql_status != SQL_SUCCESS) {
    rul___sql_message (SQL_SQLUPDFAI, &RDB$MESSAGE_VECTOR);
    return RUL_SQL_ERROR;
  }

#ifdef TEST
  lib$show_timer (&timer_cb_ptr[1]);
#endif

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
  return rul___sql_write_from_wme (wme_id, TRUE, " ", eb_data);
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
  return rul___sql_write_from_wme (wme_id, FALSE, where, eb_data);
}			/* end of routine rul__sql_update_from_wme */


