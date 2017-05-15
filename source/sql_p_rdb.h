/*
 * sql_p_rdb.h  -  RULEWORKS SQL private header; interface globals, constants, etc:
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
 *	15-May-2017	ESS	Split to Rdb specific.
 */


/*----------------------------------------------------------------------------*/
/* Constants on RUL side: */

/* Define Rdb/VMS field type constants, ie in RDB$FIELD_VERSIONS relation: */

#define Rdb$SMALLINT	7
#define Rdb$INTEGER	8
#define Rdb$QUADWORD	9
#define Rdb$FLOAT      10
#define Rdb$DOUBLE     27
#define Rdb$CHAR       14
#define Rdb$VARCHAR    37	    /* ie VARCHAR(n) and LONG VARCHAR fields */
#define Rdb$DATE       35

/* Define SQL field data types, ie as SQL sets in SQLDA: */

#define SQL_SMALLINT    500
#define SQL_INTEGER     496
#define SQL_QUADWORD    504
#define SQL_FLOAT       480	    /* same SQL_type returned for DOUBLE */
#define SQL_DOUBLE       80	    /* RUL SQL specific; not set by SQL */
#define SQL_CHAR        452
#define SQL_VARCHAR     448
#define SQL_DATE        502

/* Define FFFE constant, which will be ANDed with SQLDA SQLTYPE when local 
 field data type is recorded (since a database field which allows null/missing 
 data will have SQL data type increased by one); result of this AND will be 
 to convert an odd number to next lower even number:
*/

#define FFFE  0xFFFE

/*----------------------------------------------------------------------------*/
/* Rdb/VMS SQL definitions: */

#include <sqlsrvda.h>               /* SQLDA and SQL data type definitions */
#include <sqlsrv.h>		    /* SQL_SUCCESS etc.			   */

/* Declare Rdb message vector structure globally accessible */

struct _RDB$MESSAGE_VECTOR {
  long      RDB$LU_NUM_ARGUMENTS;
  long      RDB$LU_STATUS;
  long      RDB$LU_ARGUMENTS[18];
};

extern struct _RDB$MESSAGE_VECTOR RDB$MESSAGE_VECTOR;

/* Define b-tree node structure: */

struct  _node {
  struct  _node  *l_ptr;
  struct  _node  *r_ptr;
  short	          reserved_word;
  char	          str[CACHE_KEY_SIZE+1];
  int		  val;
};

typedef struct _node *Node;

/* #define SQL_SUCCESS    0 */		    /* from <sqlsrv.h> */
/* #define SQL_EOS	100 */


/* declare the support routines */

long rul___sql_from_cache     (char  *key,
			       short *Rdb_field_type);

long rul___sql_to_cache       (char  *key,
			       short  Rdb_field_type);

void rul___sql_init_cache     (void);

long rul___sql_free_cache     (void);

long rul___sql_free_node      (Node node);

long rul___sql_node_compare   (char *key,
			       Node  node,
			       long  dummy);

long rul___sql_node_allocate  (char *key,
			       Node *node,
			       long  dummy);

long rul___sql_attr_from_cache(char halfkey[MAX_FNAME_SIZE+1], 
			       char attr_name[MAX_NUM_FLD][MAX_FNAME_SIZE+1], 
			       long *ptr_num_attr);

long rul___sql_trav_cache     (Node  node,
			       char  halfkey[], 
			       char  attrs[MAX_NUM_FLD][MAX_FNAME_SIZE+1], 
			       long *ptr_num_attr);

long rul___sql_get_attr_list  (Molecule  wme_id, 
			       char      attrs[MAX_NUM_FLD][MAX_FNAME_SIZE+1], 
			       long     *ptr_num_attr);

long rul___sql_get_field_type (char  *SQL_table_name,
			       char  *SQL_field_name,
			       short *Rdb_field_type);

long rul___sql_message        (char *errtxt, void *opt_rdb_err);

long rul___sql_quadstr        (long  qw[2],
			       char *qstr);

long rul___sql_write_from_wme (Molecule    wme_id,
			       Boolean     insert_mode, 
			       char       *SQL_where_clause,
			       Entry_Data  eb_data);

void rul__sqlmod_exec         (long *sts, char *sch);

void rul__sqlmod_commit       (long *sts);

void rul__sqlmod_rollback     (long *sts);

void rul__sqlmod_detach       (long *sts);

void rul__sqlmod_fetch_prepare (long *sts, char *rse);

void rul__sqlmod_fetch_describe (long *sts, struct SQLDA *sqlda);

void rul__sqlmod_fetch_open   (long *sts);

void rul__sqlmod_fetch        (long *sts, struct SQLDA *sqlda);

void rul__sqlmod_fetch_close  (long *status);

void rul__sqlmod_systbl_prep     (long *sts, char *rse);

void rul__sqlmod_systbl_desc_out (long *sts, struct SQLDA *sqlda);

void rul__sqlmod_systbl_desc     (long *sts, struct SQLDA *sqlda);

void rul__sqlmod_systbl_open     (long *sts, struct SQLDA *sqlda);

void rul__sqlmod_systbl_fetch    (long *sts, struct SQLDA *sqlda);

void rul__sqlmod_systbl_close    (long *sts);

/* Define $UPCASE macro, which uppercases input string */

char toupper (char);

#ifndef $UPCASE
#define $UPCASE(str, uc_i, len) \
    for (uc_i=0; uc_i < len; uc_i++) { str[uc_i] = toupper (str[uc_i]); }
#endif



/************************ end of sql_p.h **************************/
