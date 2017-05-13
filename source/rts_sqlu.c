/*
 * rts_sql_utils.c - RULEWORKS SQL interface utility routines
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
 *	This module contains the following routines:
 *
 *	    - rul___sql_get_attr_list
 *	    - rul___sql_get_field_type
 *	    - rul___sql_quadstr
 *	    - rul___sql_write_from_wme
 *
 *	rul___sql_get_attr_list returns a array (list) of all attribute 
 *	names for a specified WME-class, including those attributes inherited 
 *	from any WME super-classes.  When this list is obtained for a new 
 *	WME-class, it is also cached so that subsequent calls to this routine 
 *	can return the list without again accessing the database.
 *
 *	rul___sql_get_field_type either retrieves a database field type value 
 *	(for a specified DB table and field name pair) from cache, or if the 
 *	field type has not yet been cached, it retrieves it from the database 
 *	itself and caches this value for future use.  The cache data is also 
 *	used to indicate whether or not the specified field exists in the 
 *	named database table.
 *	
 *	rul___sql_quadstr converts an input binary quadword (eg fetched from 
 *	the database) into a decimal string.
 *
 *	rul___sql_write_from_wme routine is a generic DB write routine, and
 *	is called by either of the more specific insert/update from WME 
 *	routines.
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
 *	22-Oct-1991	DEC	Don't blank pad cached attr names, since this 
 *					causes problems for rul__get_attr_atom, etc;
 *					however, do then need to pad field name before
 *					DB system tables are read for field type.
 *
 *      23-Oct-1991     DEC     Don't #include sql_var;h; use local variables.
 *
 *	25-Nov-1991	DEC	Used struct _RDB$MESSAGE_VECTOR in sql_gbl.h to
 *					declare RDB$MESSAGE_VECTOR. Added declarations
 *					for rul$$find_wme_by_$id and rul$get_value.
 *					Don't use initializers for automatic variables.
 *
 *	 7-Apr-1993	DEC	TINpan (RULEWORKS) changes
 *
 *	16-Feb-1998	DEC	class type changed to rclass
 *
 *	01-Dec-1999	CPQ	Release with GPL
 */


#include <common.h>
#include <mol.h>
#include <decl.h>
#include <wm.h>
#include <sql_msg.h>
#include <sql_p.h>
#include <sql.h>
#include <callback.h>	/* RULEWORKS RTL helper prototypes */
#include <ssdef.h>

#ifdef TEST
#include <lib$routines.h>
long     lib$init_timer ();          /*  library timer routines, etc */
long     lib$show_timer ();
long     timer_cb_ptr[2] = {0, 0};
#endif

/* Declare external vars: SQLDAs for fetching field types from sys table */
/* Declare Rdb message vector structure globally accessible */
/* Declare other SQL globals */

EXTERNAL  struct  SQLDA  *SQLDA_SYSTBL_OUT;
EXTERNAL  struct  SQLDA  *SQLDA_SYSTBL_IN;
EXTERNAL  long            RUL_SQL__GL_TXN_STATUS;
EXTERNAL  long            RUL_SQL__GL_SYSTBL_STATUS;


/* declare some static molecules */
static Mol_Symbol SM_id;	/* rul__mol_make_symbol ("$ID"); */
static Mol_Symbol SM_inst_of;	/* rul__mol_make_symbol ("$INSTANCE_OF"); */
static Mol_Symbol SM_dbkey;	/* rul__mol_make_symbol ("DBKEY"); */



/******************************
 **			     **
 **  rul___sql_get_attr_list **
 **			     **
 *****************************/

long  rul___sql_get_attr_list (Molecule wme_id,
			       char     attrs[MAX_NUM_FLD][MAX_FNAME_SIZE+1],
			       long    *ptr_num_attr)
     /* Args:
      *
      *   wme_id	 = WME-ID of WME whose attrs want list of
      *   attrs		 = array of attr names
      *   ptr_num_attr = ptr to number of attr for this WME-class
      */
{
  char         key[CACHE_KEY_SIZE+1];	/* concatenated table + field names;
					   forms key for cache lookup */
  long         i;			/* scratch counter, eg for index */
  long	       j;			/* scratch counter */
  long         len;                     /* length of a string */
  long	       attr_count;		/* count of class' attributes  */
  char         wme_class[MAX_FNAME_SIZE+1]; /* WME-class name string */
  Object       wme_addr;		/* WME address */
  Class        rclass;                   /* new style class address */
  Molecule     attr;                    /* new style attribute address */
  Molecule     class_name;	        /* old/new class name Molecule */

  /* initialization */
  if (SM_id == NULL) {
    SM_id = rul__mol_symbol_id ();
    SM_inst_of = rul__mol_make_symbol ("$INSTANCE_OF");
    rul__mol_mark_perm (SM_inst_of);
    SM_dbkey = rul__mol_make_symbol ("DBKEY");
    rul__mol_mark_perm (SM_dbkey);
  }

  /*=======================================================================*/
#ifdef TEST
  printf ("\n\n Now in subroutine rul___sql_get_attr_list ... \n");
#endif
  
  /* Obtain the WME-class of the input WME; will use this later if/when list 
   * of attributes for a given WME-class is cached:
   */
  
  if (!rul__validate_wme_id (wme_id, &wme_addr)) {
    rul___sql_message (SQL_SQLNOTXN, NULL);
    return RUL_SQL_ERROR;
  }

  rclass = rul__wm_get_class (wme_addr);
  class_name = rul__decl_get_class_name (rclass);
  if (!rul__mol_use_printform (class_name, wme_class, MAX_FNAME_SIZE)) {
    rul___sql_message (SQL_SQLINVCLA, NULL);
    return RUL_SQL_ERROR;
  }
  len = rul__mol_get_printform_length (class_name);
  wme_class [len] = '\0';
  $UPCASE (wme_class, i, len);
    
  /*-----------------------------------------------------------------------*/
  /* Has the list of attr names already been cached for this WME-class?
   * If so, used cached list, otherwise build new list. Attempt to extract
   * attr list array of names by traversing the cache (tree) looking for
   * matches on table name (WME-class). Pad the WME-class string up to 
   * its max size before looking for matches. Force uppercase on WME-class
   * before searching cache for it.
   */
  
  /* pad with blanks */
  for (i = len; i < MAX_FNAME_SIZE; i++) { wme_class[i] = ' '; }
  wme_class[MAX_FNAME_SIZE] = '\0';
  
  rul___sql_attr_from_cache (wme_class, attrs, ptr_num_attr);
  
  /* If attr names were cached already for this WME-class, we're done here: */
  
  if (*ptr_num_attr > 0) {
#ifdef TEST
    printf ("\n\n  %ld attr names were retrieved from cache\n", *ptr_num_attr);
#endif
    return RUL_SQL_SUCCESS;
  }
#ifdef TEST
  else {
    printf ("\n\n  Returned from attr_from_cache with #attr = %ld", 
	    *ptr_num_attr);
  }
#endif
  
  /*-----------------------------------------------------------------------*/
  /* If the attr names have not already been cached, we need to obtain the 
   * attr names here, and cache them for future use.
   *
   * Start by obtaining the address of the WME structure from the input WME-ID,
   * and then extract the class name from the WME structure, loop through
   * all its attributes (and those it inherites from its super-classes):
   */
  
  
  attr_count = rul__decl_get_class_num_attrs (rclass);

  if (attr_count > MAX_NUM_FLD) {
      rul___sql_message (SQL_SQLNUMAEX, NULL);
      return RUL_SQL_ERROR;
    }

  /* loop on each attribute for this class
   * (note: start at offset 1 to skip ^$id
   */
  for (i = 0; i < attr_count; i++) {
	      
    /* get attribute name ( as Molecule) */
    attr = rul__decl_get_attr_name (rclass, i) ;
    attrs[i][0] = '\0';
	      
    /* If this attribute name is "special", ie DBKEY, which should
     * never be written to the DB) then ignore it
     */
    if (attr == SM_id || attr == SM_inst_of || attr == SM_dbkey)
      continue;		/* try next token, ie continue to */
	      
    /* For a valid attr name, null terminate it */
    rul__symbol_to_string (attrs[i], RUL_C_MAX_SYMBOL_SIZE, attr);
    len = strlen (attrs[i]);
    attrs[i][len] = '\0';
#ifdef TEST
    printf ("\n    attr[%ld] = '%s'", i, attrs[i]);
#endif
	      
    /* Then cache this WME-class+attr name pair; note that the    */
    /* field type will be initially set to zero.  The attr names  */
    /* should be uppercased (but not blank padded before cached). */
    
    $UPCASE (attrs[i], j, len);	
	      
    strcpy (key, wme_class);	 /* 1st half of key is class */
    key[MAX_FNAME_SIZE] = '\0';
    strcat (key, attrs[i]);	 /* 2nd half key is attr name */
	      
    rul___sql_to_cache (key, 0); 	 /* cache class + attr pair */
	      
  }		/* end attr loop */
      
  /* After all attr names in class */
  *ptr_num_attr = attr_count;
      
  return RUL_SQL_SUCCESS;
}				/* End of rul___sql_get_attr_list routine */



/*******************************
 **			      **
 **  rul___sql_get_field_type **
 **			      **
 ******************************/

long  rul___sql_get_field_type (char  *table,/* table with field of interest */
				char  *field,/* name of field of interest */
				short *Rdb_field_type)/* ret Rdb field type */
{
  char    rse[512];	   /* SQL record selection expression; note small, 
			      fixed size, since this RSE is fixed */
  long	  version;	   /* version fetched along with field type (dummy) */
  long	  num_params;	   /* number of input param markers in RSE (now = 2) */
  long	  num_fields;	   /* number of fetched fields, from RSE (now = 2) */
  char    pname[MAX_FNAME_SIZE+1]; /* fetched field or input param name */
  long	  len_pname;	    /* length of pname */
  long	  ret_stat;	    /* return status */
  char    key[CACHE_KEY_SIZE+1];	/* concatenated table + field names;
					   forms key for cache lookup */
  char    sql_fname[MAX_FNAME_SIZE+1];  /* scratch, blank padded field name */
  long    i;                            /* scratch counter, eg for index */
  long    len;                          /* length of a string */
  long    sql_status;                   /* return status from SQL mod rtn */
  long    status;                       /* generic return status */
  
  /*=======================================================================*/
  /* Init return field type to zero (for no field type, ie field not found): */
  
  *Rdb_field_type = 0;
  
  /* Force uppercase for input table and field names.  Then if necessary pad
   * the table name on right with blanks (as is required for match on SQL 
   * system table fields, for table and field names).  We will eventually
   * need to pad the field name string also, before the SQL system tables are
   * accessed, but since cached attr/field names are not padded, only pad a 
   * scratch copy of the field name (ie sql_fname).
   * SQL doesn't care about the [MAX_FNAME_SIZE] element of field or table 
   * name strings, so can null terminate padded strings, in case either
   * (really just table name) is used back in calling routine.
   */
  
  len = strlen (table);
  $UPCASE (table, i, len);
  for (i=len; i < MAX_FNAME_SIZE; i++) { table[i] = ' '; }
  table[MAX_FNAME_SIZE] = '\0';
  
  len = strlen (field);
  $UPCASE (field, i, len);
  
  strcpy (sql_fname, field);
  
  for (i=len; i < MAX_FNAME_SIZE; i++) { sql_fname[i] = ' '; }
  sql_fname[MAX_FNAME_SIZE] = '\0';
  
  /*-----------------------------------------------------------------------*/
  /* If info on this field is already in cache of field types, then use it.  
   * If data on a given table + field combination is already in the cache, 
   * the field type value may have one of several values:
   *
   *    =  0  -> table+field pair cached, but field existence and type not yet 
   *		  checked and cached via fetch from database;
   *    = -1  -> this table+field pair does not exist in the database;
   *    = +n  -> table+field does exist in database, and field type is cached
   *
   * Given the current use of this cache by rul___sql_get_attr_list and by this
   * routine (both called from rul___sql_write_from_wme, in that order), this 
   * call to rul___sql_from_cache should never fail to at least find the 
   * table+field pair; however, if that occurs, just cache the data and 
   * proceed.  Key for this search will be the table name concatenated with 
   * field name.
   */
  
  strcpy (key, table);
  key[MAX_FNAME_SIZE] = '\0';
  strcat (key, field);
  
  status = rul___sql_from_cache (key, Rdb_field_type);
  
  /* For case where the field type value is already cached, return success 
   * status and the value for the Rdb field type: 
   */
  
  if (status && (*Rdb_field_type > 0)) {
#ifdef TEST
    printf ("\n\n  In get_field_type, for field %s", field);
    printf ("\n\n    1st case: field type > 0 (ie field type cached)");
#endif
    return RUL_SQL_SUCCESS;
  }
  
  /* Next, for the case when the cache field type value has already been set 
   * to indicate that this table+field combination does not exist in the 
   * database, return non-success status (and negative field type, eg -1):
   */
  
  else if (status && (*Rdb_field_type < 0)) {
#ifdef TEST
    printf ("\n\n  In get_field_type, for field %s", field);
    printf ("\n\n    2nd case: field type < 0 (ie field not in database)");
#endif
    return RUL_SQL_WARNING;
  }
  
  /*-----------------------------------------------------------------------*/
  /* And finally, fall thru to here for the 3rd case: info on the specified 
   * field was not already in cache (ie status from the call to 
   * rul___sql_from_cache was non-success) or the table+field pair was in the 
   * cache but its field type field was still zero (ie actual field type had 
   * not yet been fetched from the database), then access this info in the DB.
   */
  
#ifdef TEST
  printf ("\n\n  In get_field_type, for field %s", field);
  if (status) {
    printf ("\n\n    3rd case: field type = 0 (ie DB not yet accessed)");
  }
  else {
    printf ("\n\n    3rd case (alt): table+field pair not yet cached");
  }
#endif
  
  /* Current use of this routine is during insert or update operations, so
   * a txn should be active at this point; test for this, and abort if not (ie 
   * don't want the interface to start an unexpected implicit READ ONLY txn):
   */
  
  if (!RUL_SQL__GL_TXN_STATUS) {
    rul___sql_message (SQL_SQLNOTXN, NULL);
    return RUL_SQL_ERROR;
  }
  
  /*-----------------------------------------------------------------------*/
  /* Prepare select of system table data for field type, if necessary, and 
   * describe the parameters in these statements.
   *
   * Allocate space for the SQLDA areas, for data to be fetched from database 
   * system tables (ie SQLDA_SYSTBL_OUT), and for input parameters 
   * (SQLDA_SYSTBL_IN) unless they have already been allocated.
   *
   * First allocate space for fetched output SQLDA, if necessary.  Note that 
   * here only space for 2 fetched fields is allocated (and similarly for the 
   * input parameter SQLDA):
   */
  
  if (!SQLDA_SYSTBL_OUT) {     /* if space not already allocated, do it now */
#ifdef TEST
    printf ("\n   Allocating space for SQLDA_SYSTBL_OUT ... \n");
#endif
    SQLDA_SYSTBL_OUT = (struct SQLDA *) rul__mem_malloc (
				 sizeof(*SQLDA_SYSTBL_OUT) +
				 (2 * sizeof(SQLDA_SYSTBL_OUT->SQLVARARY[0])));
    if (SQLDA_SYSTBL_OUT == 0) {
      rul___sql_message (SQL_SQLDESALL, NULL);
      return RUL_SQL_ERROR;
    }
    SQLDA_SYSTBL_OUT->SQLN = 2;
  }
  
  /* Now allocate space for input parameter SQLDA, if necessary: */
  
  if (!SQLDA_SYSTBL_IN) {     /* if space not already allocated, do it now */
#ifdef TEST
    printf ("\n   Allocating space for SQLDA_SYSTBL_IN ... \n");
#endif
    SQLDA_SYSTBL_IN = (struct SQLDA *) rul__mem_malloc (
				sizeof(*SQLDA_SYSTBL_IN) +
				(2 * sizeof(SQLDA_SYSTBL_IN->SQLVARARY[0])));
    if (SQLDA_SYSTBL_IN == 0) {
      rul___sql_message (SQL_SQLDESALL, NULL);
      return RUL_SQL_ERROR;
    }
    SQLDA_SYSTBL_IN->SQLN = 2;
  }
  
  /*-----------------------------------------------------------------------*/
  /* Prepare and describe the select statement for accessing field type from 
   * the system tables, only if it has not already been prepared, etc.
   *
   * Note that this RUL_SQL__GL_SYSTBL_STATUS flag must be cleared in any routine
   * which causes an SQL EXECUTE IMMEDIATE to be performed, as this will 
   * implicitly release any prepared statements.  For example, 
   * RUL_SQL__GL_SYSTBL_STATUS is set to zero in attach, delete, insert (both 
   * flavors), start, and update (both) routines.
   */
  
  if (!RUL_SQL__GL_SYSTBL_STATUS) {
      
    /* Form the RSE which will be used to fetch field type data from the 
     * system tables:
     */
      
    strcpy (rse, 
	 "SELECT RDB$VERSION, RDB$FIELD_TYPE FROM RDB$FIELD_VERSIONS F, ");
    strcat (rse,
	"RDB$RELATIONS R WHERE RDB$RELATION_NAME = ? AND RDB$FIELD_NAME = ? ");
    strcat (rse,
	"AND F.RDB$RELATION_ID = R.RDB$RELATION_ID ORDER BY RDB$VERSION DESC");
      
    /* Prepare then describe the RSE, ie setup SQLDA output fields: */
      
    rul__sqlmod_systbl_prep (&sql_status, rse);
    if (sql_status != SQL_SUCCESS) {
      rul___sql_message (SQL_SQLFETPRE, &RDB$MESSAGE_VECTOR);
      return RUL_SQL_ERROR;
    }
      
    rul__sqlmod_systbl_desc_out (&sql_status, SQLDA_SYSTBL_OUT);
    if (sql_status != SQL_SUCCESS) {
      rul___sql_message (SQL_SQLFETDSC, &RDB$MESSAGE_VECTOR);
      return RUL_SQL_ERROR;
    }
      
    /* After the RSE has been prepared, set SQLDATA fields to point to host 
     * variables for fetched fields (ie output):
     */
    
    num_fields = SQLDA_SYSTBL_OUT->SQLD;
      
#ifdef TEST
    printf ("\n\n After sys table access prep, # fields = %ld \n", num_fields);
#endif
      
    for (i =0; i < num_fields; i++) {
      len_pname = strcspn (SQLDA_SYSTBL_OUT->SQLVARARY[i].SQLNAME, " ");
      strncpy (pname, SQLDA_SYSTBL_OUT->SQLVARARY[i].SQLNAME, len_pname);
      pname[len_pname] = '\0';
      if (strcmp (pname, "RDB$FIELD_TYPE") == 0) {
	SQLDA_SYSTBL_OUT->SQLVARARY[i].SQLDATA=(unsigned char *)Rdb_field_type;
      }
      else {
	if (strcmp (pname, "RDB$VERSION") == 0) {
	  SQLDA_SYSTBL_OUT->SQLVARARY[i].SQLDATA = (unsigned char *)&version;
	}
	else {
	  rul___sql_message (SQL_SQLINVFIE, NULL);
	  return RUL_SQL_ERROR;
	}
      }	    /* end for all fields in prepared RSE */
	  
#ifdef TEST
      printf ("\n  Field name = '%s' ", pname);
#endif
    }

    /* Next describe the RSE, ie populate SQLDA for input params: */
      
    rul__sqlmod_systbl_desc (&sql_status, SQLDA_SYSTBL_IN);
    if (sql_status != SQL_SUCCESS) {
      rul___sql_message (SQL_SQLDESFAI, &RDB$MESSAGE_VECTOR);
      return RUL_SQL_ERROR;
    }
      
    /* After the RSE has been described, set SQLDATA fields to point to 
     * host variables for input params:
     */
    
    num_params = SQLDA_SYSTBL_IN->SQLD;
      
#ifdef TEST
    printf ("\n\n After sys table access described, # params = %ld \n\n",
	    num_params);
#endif
      
    for (i =0; i < num_params; i++) {
      len_pname = strcspn (SQLDA_SYSTBL_IN->SQLVARARY[i].SQLNAME, " ");
      strncpy (pname, SQLDA_SYSTBL_IN->SQLVARARY[i].SQLNAME, len_pname);
      pname[len_pname] = '\0';
      if (strcmp (pname, "RDB$RELATION_NAME") == 0) {
	SQLDA_SYSTBL_IN->SQLVARARY[i].SQLDATA = (unsigned char *)table;
      }
      else {
	if (strcmp (pname, "RDB$FIELD_NAME") == 0) {
	  SQLDA_SYSTBL_IN->SQLVARARY[i].SQLDATA = (unsigned char *)sql_fname;
	}
	else {
	  rul___sql_message (SQL_SQLINVFIE, NULL);
	  return RUL_SQL_ERROR;
	}
      }
	  
#ifdef TEST
      printf ("\n  Param name = '%s' ", pname);
#endif
    }		/* end for all param markers in described RSE */
      
    /* Done preparing (etc) the RSE for accessing field type data from the 
     * system tables, so set the "statement prepared" global status variable:
     */
      
    RUL_SQL__GL_SYSTBL_STATUS = TRUE;
      
  }	/* end if sys table access RSE not already prepared */
  
  /*-----------------------------------------------------------------------*/
  /* Open cursor, fetch first record (with max field version), and then close 
   * the cursor.  Start by opening the cursor to be used for the fetch: 
   */
  
  rul__sqlmod_systbl_open (&sql_status, SQLDA_SYSTBL_IN);
  if (sql_status != SQL_SUCCESS) {
    rul___sql_message (SQL_SQLCUROPE, &RDB$MESSAGE_VECTOR);
    return RUL_SQL_ERROR;
  }
  
  /* Now perform the actual fetch of data from the system tables: */
  
#ifdef TEST
  printf ("\n\n Fetching field type ... \n");
#endif
  
  rul__sqlmod_systbl_fetch (&sql_status, SQLDA_SYSTBL_OUT);
  
  /* End-of-stream is a valid return value for this fetch, ie if the field is 
   * not found in the database; in this case just return false (zero) and set
   * the returned (and the cached) field type to -1.
   *
   * Also check for actual fetch errors; "signal" and return error if detected;
   * this will be the default case.  Force the returned field type to -1 if
   * there was any problem.
   *
   * For a normal, successful fetch of field type, cache this value for future
   * use.
   */
  
  switch (sql_status) {

  case SQL_EOS:		/* EOS, ie field not found, no type returned */
    ret_stat = RUL_SQL_WARNING;	/*** note: must be FALSE, ie zero ***/
    *Rdb_field_type = -1;
    rul___sql_to_cache (key, -1);
    break;
    
  case SQL_SUCCESS:			/* normal fetch */
    ret_stat = RUL_SQL_SUCCESS;
    rul___sql_to_cache (key, *Rdb_field_type);
    break;
    
  default:		/* unexpected error, no type returned */
    ret_stat = RUL_SQL_ERROR;
    *Rdb_field_type = -1;
    rul___sql_to_cache (key, -1);
    rul___sql_message (SQL_SQLFETCH, &RDB$MESSAGE_VECTOR);
  }		/* end case on fetch sql_status */
  
  /* And after the fetch, close the cursor: */
  
  rul__sqlmod_systbl_close (&sql_status);
  if (sql_status != SQL_SUCCESS) {
    rul___sql_message (SQL_SQLCURCLO, &RDB$MESSAGE_VECTOR);
    return RUL_SQL_ERROR;
  }
  
  return ret_stat;
}		/* End of rul___sql_get_field_type */


/************************
 **		       **
 **  rul___sql_quadstr **
 **		       **
 ***********************/

long  rul___sql_quadstr (long qw[2],/* input binary quadword, ie 2 longwords */
			 char *qstr)/* output decimal string representation */

#define NUM_DEC  20	    /* quadword = 20 decimal digits, plus sign */
#define NUM_HEX  16	    /* quadword in hex representation is 16 digits */
     
{
  unsigned  char  ba[8];	  /* quadword as 8 byte array (unsigned) */
  unsigned  char  nibble[2];	  /* half-byte, low- and high-order 4 bits */
  long            neg = FALSE;    /* flag (and output offset) set if qw < 0 */
  long	          nb;		  /* byte number, loop counter */
  long            nc;		  /* character position in output qstr */
  long	          nn;		  /* nibble number, loop counter */
  long            hp;		  /* hex place/position, eg 1st digit, etc */
  long            dp;		  /* decimal place, index longo digits[] */
  long            nonzero_digit_found; /* flag TRUE when nonzero digit found */
  long            high_bit_was_set = FALSE; /* set if highorder bit = 1 */
  char            most_neg_value[] = "-9223372036854775808"; /*  -(2**63)  */
  
  /* Define array "digit" which will receive the contributions from each hex
   * digit (nibble), ie digit[i] will eventually contain 10**i place value.
   */
  
  long  digit[NUM_DEC] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  
  /* Define 2-dimensional array "contrib" which contains the contribution 
   * of each hex digit (nibble) to each decimal place.  Note that only 
   * NUM_DEC-1 columns are needed, since hex-digit[15] (16th hex digit) does 
   * not contribute anything to the decimal digit[19] (20th digit).
   */
  
  long  contrib[NUM_HEX][NUM_DEC-1] = {
    
    /* decimal position: */
    /*  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 */ /* hex */
    /* digit*/
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* 0 */
    6, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* 1 */
    6, 5, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* 2 */
    6, 9, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* 3 */
    6, 3, 5, 5, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* 4 */
    6, 7, 5, 8, 4, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* 5 */
    6, 1, 2, 7, 7, 7, 6, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* 6 */
    6, 5, 4, 5, 3, 4, 8, 6, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* 7 */
    6, 9, 2, 7, 6, 9, 4, 9, 2, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* 8 */
    6, 3, 7, 6, 7, 4, 9, 1, 7, 8, 6, 0, 0, 0, 0, 0, 0, 0, 0,	/* 9 */
    6, 7, 7, 7, 2, 6, 1, 1, 5, 9, 9, 0, 1, 0, 0, 0, 0, 0, 0,	/* 10 */
    6, 1, 4, 4, 4, 0, 6, 8, 1, 2, 9, 5, 7, 1, 0, 0, 0, 0, 0,	/* 11 */
    6, 5, 6, 0, 1, 7, 6, 7, 9, 4, 7, 4, 1, 8, 2, 0, 0, 0, 0,	/* 12 */
    6, 9, 4, 0, 7, 3, 7, 2, 6, 9, 9, 5, 3, 0, 5, 4, 0, 0, 0,	/* 13 */
    6, 3, 9, 7, 2, 9, 7, 3, 0, 4, 9, 5, 7, 5, 0, 2, 7, 0, 0,	/* 14 */
    6, 7, 9, 6, 4, 8, 6, 0, 6, 4, 0, 5, 1, 2, 9, 2, 5, 1, 1	/* 15 */
    };
  
  /*=======================================================================*/
  /* This routine converts the input binary quadword into the output 
   * decimal string representation, and also returns the length of the string.
   *
   * Credit for the algorithm (and table of contrib values) goes to Paul S. 
   * Winalski (NAD), from "Quadword Arithmetic Package", 3 February 1990.
   * Note bug in original re 'most negative' value = -(2**63), not -(2**64).
   */
  
  /*-----------------------------------------------------------------------*/
  /* Test for zero quadword, and return "0" immediately if found: */
  
  if ((qw[0] == 0) && (qw[1] == 0)) {
    qstr[0] = '0';
    qstr[1] = '\0';
    return 1;
  }
  
  /*-----------------------------------------------------------------------*/
  /* Test for negative binary, and if so convert to positive value and start 
   * returned string with minus sign prefix.  Neqative to positive quadword 
   * conversion involves taking compliment of both longwords and adding one,
   * picking up low-order to high-order longword carry if necesary.
   *
   * After the negative to positive conversion, one special case to catch
   * is for both longwords = zero; this is case for most negative value.  In
   * this case just return '0'.
   */
  
  if (qw[1] < 0) {	    /* look at high-order longword for "sign" */
    qstr[0] = '-';
    neg = TRUE;
    qw[0] = ~qw[0];
    qw[1] = ~qw[1];
      
    if ((qw[0] & 0x80000000) != 0) {	/* before adding 1, test and clear */
					/* low lword high-order bit (if set) */
      high_bit_was_set = TRUE;
      qw[0] = qw[0] & 0x7FFFFFFF;		/* clear the high-order bit */
    }
    qw[0] = qw[0] + 1;
      
    if (((qw[0] & 0x80000000) != 0) &&	/* high-order bit set again? */
	(high_bit_was_set)) {			/* and was before? Then ...  */
      qw[0] = qw[0] & 0x7FFFFFFF;		/* clear it again, and carry */
      qw[1] = qw[1] + 1;			/* +1 up to high longword */
      qw[1] = qw[1] & 0x7FFFFFFF;		/* clear high longword sign */
    }
    else if (((qw[0] & 0x80000000) == 0) &&	/* if high bit is clear */
	     (high_bit_was_set)) {		/* after the +1, but was 1 */
      qw[0] = qw[0] | 0x80000000;		/* then set back to 1 */
    }
    
    if ((qw[0] == 0) && (qw[1] == 0)) {  	/* "most negative" case */
      strcpy (qstr, most_neg_value);
      qstr[20] = '\0';
      return 20;
    }
  }	    /* end if negative quadword */
  
  /*-----------------------------------------------------------------------*/
  /* Treat quadword as an array of 16 nibbles, ie the quadword is equivalent 
   * to 8 bytes, each of which is divided into 2 nibbles (4 bits each); the
   * value can be thought of as 16 hex digits.
   * 
   * For each nibble (or hex digit), multiply the nibble value by each of the
   * decimal place contributions that hex digit position should make, thus
   * incrementing the value in each decimal place (the digit array).
   */
  
  memcpy (ba, qw, 8);
  for (nb=0; nb<8; nb++) {
    nibble[0] =  ba[nb] & 0x0F;	      /* low-order 4 bits */
    nibble[1] = (ba[nb] & 0xF0) >> 4;     /* 4 high bits, shifted right 4 */
    for (nn=0; nn<2; nn++) {
      if (nibble[nn] != 0) {   /* if nibble not 0, it contributes to qstr */
	hp = (nb*2)+nn;		/* which nibble out of 16 is this? */
	for (dp=0; dp<NUM_DEC-1; dp++) {
	  if (contrib[hp][dp] != 0) {
	    digit[dp] = digit[dp] + (nibble[nn] * contrib[hp][dp]);
	  }
	}
      }
    }
  }
  
  /*-----------------------------------------------------------------------*/
  /* Now we have contributions from all hex digits (nibbles) to all the 
   * decimal digits (in digit array), but since each decimal digit may have
   * received a contribution greater than the 0-9 that it can hold, need 
   * to move those excess contributions to the left (higher digit[] elem):
   */
  
  for (dp=0; dp<NUM_DEC-1; dp++) {
    digit[dp+1] = digit[dp+1] + (digit[dp]/10);	/* carry overflow up */
    digit[dp] = digit[dp] % 10;			/* limit digit to 0-9 */
  }
  
  /*-----------------------------------------------------------------------*/
  /* Now use the digit array values to populate the return qstr.  Init the 
   * character string position index "nc" to 0 or 1, depending on the sign
   * of the quadword (to allow for negative sign).
   */
  
  nc = neg;
  nonzero_digit_found = FALSE;   /* flag set TRUE when non-zero digit found */
  
  for (dp=NUM_DEC-1; dp>=0; dp--) {
    if (nonzero_digit_found || (digit[dp] != 0)) {
      nonzero_digit_found = TRUE;
      qstr[nc] = digit[dp] + 48;		/* ASCII '0' is 48 */
      nc++;
    }
  }
  
  qstr[nc] = '\0';		    /* null terminate the returned string */
  
  return nc;
}			    /* end of routine rul___sql_quadstr */



/*******************************
 **			      **
 **  rul___sql_write_from_wme **
 **			      **
 ******************************/

long  rul___sql_write_from_wme (Molecule  wme_id,    /* WME to be inserted */
				Boolean   insert_mode,/* mode */
				char     *where,     /* selects recs */
				Entry_Data eb_data)
{
  long	   num_attr;		/* number of attr for this WME */
  long	   num_attr_used = 0;	/*  and # of them used to write to DB */
  char     attr_name[MAX_NUM_FLD][MAX_FNAME_SIZE+1];  /* array of attr names;
							 each name's length
							 is limited by SQL */
  char     field_list[MAX_LIST_SIZE+4];    /* string list of field names */
  char     set_clause[MAX_LIST_SIZE+4];    /* SET list of field-value pairs */
  char     value_list[MAX_LIST_SIZE+4];    /* list of field values */
  char     value_str[RUL_C_MAX_SYMBOL_SIZE+1]; /* printable form of attr val */
  Class    rclass;
  Object   wme_addr;
  Molecule attr;                           /* attribute as Molecule */
  Molecule att_val;                        /* attribute value as Molecule */
  Molecule class_name;                     /* class name as Molecule */
  long     i;                              /* scratch counter, eg for index */
  long     field_exists;                   /* set TRUE when DB field exists */
  short    field_type;                     /* eg char vs long */
  long     len;                            /* length of a string */
  long     sql_status;                     /* return status from SQL mod rtn */
  long     status;                         /* generic return status */
  char     sql_stmt[MAX_SQL_STMT_SIZE+1];  /* SQL statement, eg to be exec */
  char     wme_class[MAX_FNAME_SIZE+1];    /* WME-class name string */

  /*=======================================================================*/
#ifdef TEST
  printf ("\n Now in subroutine rul___sql_write_from_wme ... \n");
#endif

  /* Verify that user has explicitly started a (READ WRITE) transaction; if 
   * not then abort the insert before SQL does.
   */

  if (!RUL_SQL__GL_TXN_STATUS) {
    rul___sql_message (SQL_SQLNOTXN, NULL);
    return RUL_SQL_ERROR;
  }

  /* Take a look at passed in WME-ID argument; verify that the WME-ID arg does
   * correspond to an existing WME 
   */

  if (!rul__validate_wme_id (wme_id, &wme_addr)) {
    rul___sql_message (SQL_SQLINVWID, NULL);
    return RUL_SQL_ERROR;
  }

  /*-----------------------------------------------------------------------*/
  /* Obtain the WME-class of the input WME */

  rclass = rul__wm_get_class (wme_addr);
  class_name = rul__decl_get_class_name (rclass);
  if (!rul__mol_use_printform (class_name, wme_class, MAX_FNAME_SIZE)) {
    rul___sql_message (SQL_SQLINVCLA, NULL);
    return RUL_SQL_ERROR;
  }
  len = rul__mol_get_printform_length (class_name);
  wme_class [len] = '\0';
  $UPCASE (wme_class, i, len);
  
  /* Start forming the SQL statement, ie at this point know only table;
   * note that here we assume table and WME-class names are the same.  
   * Also start forming field and values lists, or the set statement:
   */

  if (insert_mode) {
    strcpy (sql_stmt, "INSERT INTO ");
    strcat (sql_stmt, wme_class);
    strcpy (field_list, " (");
    strcpy (value_list, " VALUES (");
  }
  else {		    /* for update mode */
    strcpy (sql_stmt, "UPDATE ");
    strcat (sql_stmt, wme_class);
    strcpy (set_clause, " SET ");
  }

  /*-----------------------------------------------------------------------*/
  /* For each attribute in this WME:
   *    - is it also a field in the DB record? (if not, try next attr)
   *    - check data type of field, since char fields will require quotes
   *    - add attr name to field_list 
   *	    (note that attr and field names are mapped 1-to-1 here)
   *    - get data value, and add to value_list (w/ or w/o quotes)
   *
   *
   * Start by getting a list of attributes for the source WME's WME-class; 
   * input is the WME-ID ofthe source WME, and output is the array of attribute
   * names for that WME-class (and any super-classes) and the number of 
   * attributes:
   *
   * Note that attribute names may only map 1-to-1 to field names in this 
   * interface routine, and that WME-class and database table names are also 
   * 1-to-1 mapped; thus attribute names returned in this list are already 
   * truncated if necessary to the max size aloowed by SQL for field names.
   */
  
  status = rul___sql_get_attr_list (wme_id, attr_name, &num_attr);
  if ((!status) || (num_attr == 0)) {
    rul___sql_message (SQL_SQLATTLIS, NULL);
    return RUL_SQL_ERROR;
  }

  /* For each attribute, is it also a field in the database table?  If not, 
   * then won't store; if so, then also get data type (need to treat any 
   * character string values differently, ie with quotes).  
   */
  
  for (i = 0; i < num_attr; i++) {
    field_exists = rul___sql_get_field_type (wme_class,	/* table name */
					     attr_name[i],   /* + field */
					     &field_type);
    if (field_exists) {
      num_attr_used++;	/* increment count of used (ie written) attr */

      /* If this field is in the table, add the attribute name to the 
       * field list (for insert mode) or to set clause (for update mode): 
       */

      if (insert_mode) {
	if ((strlen (field_list) + strlen (attr_name[i])) >= MAX_LIST_SIZE) {
	  rul___sql_message (SQL_SQLFLDLEX, NULL);
	  return RUL_SQL_ERROR;
	}
	strcat (field_list, attr_name[i]);
	strcat (field_list, ",");
      }
      else {
	if ((strlen (set_clause) + strlen (attr_name[i])) >= MAX_LIST_SIZE) {
	  rul___sql_message (SQL_SQLSETLEX, NULL);
	  return RUL_SQL_ERROR;
	}
	strcat (set_clause, attr_name[i]);
	strcat (set_clause, " = ");
      }

      /* Use TIN supplied routines to obtain value associated with this 
       * attribute, in printable format, rather than continuing to parse 
       * the WME. 
       */

      att_val = rul__get_attr_atom (wme_id, attr_name[i]);
      rul__mol_use_printform (att_val, value_str, RUL_C_MAX_SYMBOL_SIZE);
      
      /* Add the value to the value list, enclosing value string for any 
       * character field (including DATE fields) in single quotes; 
       * whatever the field data type is, if the attribute value is NIL,
       * then just put NULL value on the value list:
       */

      /* verify maximums haven't been exceeded */

      if (insert_mode) {
	if ((strlen (value_list) + strlen (value_str)) >= MAX_LIST_SIZE) {
	  rul___sql_message (SQL_SQLVALLEX, NULL);
	  return RUL_SQL_ERROR;
	}
      }
      else {
	if ((strlen (set_clause) + strlen (value_str)) >= MAX_LIST_SIZE) {
	  rul___sql_message (SQL_SQLSETLEX, NULL);
	  return RUL_SQL_ERROR;
	}
      }

      if (strcmp(value_str, "NIL") == 0) {	    /* missing data value */
	if (insert_mode) {
	  strcat (value_list, "NULL");
	}
	else {
	  strcat (set_clause, "NULL");        
	}
      }
      else {		/* non-NIL attribute value case */
	if ((field_type == Rdb$CHAR)    || 
	    (field_type == Rdb$VARCHAR) ||
	    (field_type == Rdb$DATE))   {
	  if (insert_mode) {
	    strcat (value_list, "'");
	    strcat (value_list, value_str);
	    strcat (value_list, "'");
	  }
	  else {
	    strcat (set_clause, "'");
	    strcat (set_clause, value_str);
	    strcat (set_clause, "'");
	  }
	}
	else {	    /* field is not char data, so no quotes */
	  if (insert_mode) {
	    strcat (value_list, value_str);
	  }
	  else {
	    strcat (set_clause, value_str);
	  }
	}
      }
      if (insert_mode) {
	strcat (value_list, ",");
      }
      else {
	strcat (set_clause, ",");
      }
    }				/* End of the if-field-exists test */
  }			/* end for all attributes for this WME */	

  /* Check number of WME attributes actually used, ie check for empty list! */

  if (num_attr_used <= 0) {
    rul___sql_message (SQL_SQLZERFIE, NULL);
    return RUL_SQL_ERROR;
  }
  
  /* Remove last "," item separator from both lists: */
  
  if (insert_mode) {
    for (i = strlen(field_list) - 1; i >= 0; i--) {
      if (field_list[i] == ',')	{
	field_list[i] = '\0';	    /* replace last comma with null */
	break;                      /*   and done with for loop */
      }
    }

    for (i = strlen(value_list) - 1; i >= 0; i--) {
      if (value_list[i] == ',')	{
	value_list[i] = '\0';       /* replace last comma with null */
	break;                      /*   and done with for loop */
      }
    }

    strcat (field_list, ") ");	/* End field and value lists w/ ") " */
    strcat (value_list, ")");	/*   and ")", respectively */

    if ((strlen (sql_stmt) + strlen (field_list) +
	 strlen (value_list)) >= MAX_SQL_STMT_SIZE) {
      rul___sql_message (SQL_SQLSTMLEX, NULL);
      return RUL_SQL_ERROR;
    }

    strcat (sql_stmt, field_list);	/* Then complete the SQL statement */
    strcat (sql_stmt, value_list);
#ifdef TEST
    printf ("\n\n   Field list = '%s'", field_list);
    printf ("\n\n   Value list = '%s'", value_list);
    printf ("\n\n   SQL insert stmt = '%s' \n", sql_stmt);
#endif
  }

  else {			/* for update mode */
    for (i = strlen(set_clause) - 1; i >= 0; i--) {
      if (set_clause[i] == ',') {
	set_clause[i] = '\0';	    /* replace last comma with null */
	break;                      /*   and done with for loop */
      }
    }

    if ((strlen (sql_stmt) + strlen (set_clause)) >= MAX_SQL_STMT_SIZE) {
      rul___sql_message (SQL_SQLSTMLEX, NULL);
      return RUL_SQL_ERROR;
    }

    strcat (sql_stmt, set_clause);

    if ((where != 0) &&                           /* if not null ptr, and */
	(strspn (where, " ") < strlen (where))) { /*  not empty/null string */
      if ((strlen (sql_stmt) + strlen (where)) >= MAX_SQL_STMT_SIZE) {
	rul___sql_message (SQL_SQLSTMLEX, NULL);
	return RUL_SQL_ERROR;
      }
      strcat (sql_stmt, " ");
      strcat (sql_stmt, where);
    }
#ifdef TEST
    printf ("\n\n   SQL update stmt = '%s' \n", sql_stmt);
#endif
  }

  /*-----------------------------------------------------------------------*/
  /* Perform the write; but first ...
   *
   * Since SQL "execute immediate" is used in rul__sqlmod_exec, then any 
   * prepared statements are released implicitly.  Access to the system tables 
   * (ie to get a field type) uses such prepared statements, so set this 
   * "statement prepared" status to zero, before this call:
   */
  
  RUL_SQL__GL_SYSTBL_STATUS = FALSE;
  
  /* Now execute the DB write: */
  
#ifdef TEST
  printf ("\nPerforming rul___sql_write_from_wme ...\n");
  lib$init_timer (&timer_cb_ptr[1]);
#endif

  rul__sqlmod_exec (&sql_status, sql_stmt);

  if (sql_status != SQL_SUCCESS) {
    rul___sql_message (SQL_SQLWRIFAI, &RDB$MESSAGE_VECTOR);
    return RUL_SQL_ERROR;
  }
  
#ifdef TEST
  lib$show_timer (&timer_cb_ptr[1]);
#endif
  
  /*-----------------------------------------------------------------------*/
  
  return RUL_SQL_SUCCESS;
}				/* End of routine rul___sql_write_from_wme */
