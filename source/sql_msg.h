/* sql_msg.h - Standard Query Language Message definitions */
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
 *  FACILITY:
 *	RULEWORKS run time system and compiler
 *
 *  ABSTRACT:
 *	Diagnostic messages are defined in this file.  This is intended
 *	as a temporary solution for defining messages.
 *
 *  Format:
 *	Each message is defined with a #define, with a macro name of
 *	MSG_msgid (or fac_msgid), where msgid is the (uppercase)
 *	message ID on VMS.  The replacement string for the macro
 *	starts at the beginning of the next line.  It's a string
 *	literal consisting of the message ID in uppercase, a space,
 *	the severity - one of {I W E F} (uppercase), a space, and the
 *	text of the message.  A comment describing the message starts
 *	on the next line and continues as needed.
 *
 *	The text of the message is a C language printf() format
 *	string.  String inserts are represented as %s, characters as
 *	%c, numeric (decimal) inserts as %d.  % needs to be doubled as
 *	%%.  Backslash and quote need to be quoted by preceding with \.
 *	Newline is \n.
 *
 *  MODIFIED BY:
 *	DEC	Digital Equipment Corporation
**	CPQ	Compaq Computer Corporation
 *
 *  MODIFICATION HISTORY:
 *
 *	 9-Sep-1992	DEC	Initial version
 *	01-Dec-1999	CPQ	Release with GPL
 */

/******************** SQL (run-time-system errors) ************/

#define SQL_SQLATTFAI \
  "SQLATTFAI E SQL attach to database failed"

#define SQL_SQLATTLIS \
  "SQLATTLIS E Attribute list lookup failed"

#define SQL_SQLCACALL \
  "SQLCACALL E Cache space allocation failed"

#define SQL_SQLCACFRE \
  "SQLCACFRE E Error freeing space for cached field type data"

#define SQL_SQLCACINS \
  "SQLCACINS E Insert error while caching field type data"

#define SQL_SQLCACLOO \
  "SQLCACLOO E Lookup error while searching cached field type data"

#define SQL_SQLCOMFAI \
  "SQLCOMFAI E SQL commit of transaction failed"

#define SQL_SQLCOMIGN \
  "SQLCOMIGN E Commit ignored, no transaction active"

#define SQL_SQLCURCLO \
  "SQLCURCLO E SQL fetch cursor close failed"

#define SQL_SQLCUROPE \
  "SQLCUROPE E SQL fetch cursor open failed"

#define SQL_SQLDELFAI \
  "SQLDELFAI E SQL delete operation failed"

#define SQL_SQLDESALL \
  "SQLDESALL E SQL descriptor area (SQLDA) space allocation failed"

#define SQL_SQLDESFAI \
  "SQLDESFAI E SQL describe of get field type RSE failed"

#define SQL_SQLDETFAI \
  "SQLDETFAI E SQL detach from database failed"

#define SQL_SQLFETCH \
  "SQLFETCH E SQL fetch failed"

#define SQL_SQLFETDSC \
  "SQLFETDSC E Description of SQL fetch statement failed"

#define SQL_SQLFETPRE \
  "SQLFETPRE E Preparation of SQL fetch statement failed"

#define SQL_SQLINSFAI \
  "SQLINSFAI E SQL insert operation failed"

#define SQL_SQLINVCLA \
  "SQLINVCLA E Invalid OBJECT-CLASS found for specified INSTANCE-ID"

#define SQL_SQLINVDBK \
  "SQLINVDBK E Invalid database key scope specified, attach aborted"

#define SQL_SQLINVFIE \
  "SQLINVFIE E Invalid field name found in RSE for get field type"

#define SQL_SQLINVFRO \
  "SQLINVFRO E Invalid FROM clause in specified RSE, fetch aborted"

#define SQL_SQLINVSCH \
  "SQLINVSCH E Invalid schema source specified, attach aborted"

#define SQL_SQLINVSET \
  "SQLINVSET E Invalid SET clause, insert aborted"

#define SQL_SQLINVTAB \
  "SQLINVTAB E Invalid database table name specified"

#define SQL_SQLINVWID \
  "SQLINVWID E Invalid INSTANCE-ID specified"

#define SQL_SQLMISNUM \
  "SQLMISNUM E Number of fetched fields not equal to values requested"

#define SQL_SQLNOTXN \
  "SQLNOTXN E No transaction active, current operation aborted"

#define SQL_SQLNUMAEX \
  "SQLNUMAEX E SQL number of attributes exceeded maximum of 2048"

#define SQL_SQLNUMFEX \
  "SQLNUMFEX E SQL number of fields exceeded maximum of 2048"

#define SQL_SQLOLDWME \
  "SQLOLDWME E Old-style WMEs are not supported"

#define SQL_SQLROLFAI \
  "SQLROLFAI E SQL rollback of transaction failed"

#define SQL_SQLROLIGN \
  "SQLROLIGN E Rollback ignored, no transaction active"

#define SQL_SQLSTAFAI \
  "SQLSTAFAI E SQL start transaction failed"

#define SQL_SQLSTMLEX \
  "SQLSTMLEX E SQL statement length exceeded maximum of 8192"

#define SQL_SQLFLDLEX \
  "SQLFLDLEX E SQL field list length exceeded maximum of 4096"

#define SQL_SQLSETLEX \
  "SQLSETLEX E SQL set clause length exceeded maximum of 4096"

#define SQL_SQLVALLEX \
  "SQLVALLEX E SQL value list length exceeded maximum of 4096"

#define SQL_SQLTIMFET \
  "SQLTIMFET E Error fetching date-time database field value"

#define SQL_SQLUNSDAT \
  "SQLUNSDAT E Unsupported field data type"

#define SQL_SQLUNSFLO \
  "SQLUNSFLO E Unsupported float data type, or unknown type"

#define SQL_SQLUPDFAI \
  "SQLUPDFAI E SQL update operation failed"

#define SQL_SQLVALALL \
  "SQLVALALL E Space allocation for fetched value failed"

#define SQL_SQLVALARR \
  "SQLVALARR E Space allocation for fetched value array failed"

#define SQL_SQLWIDALL \
  "SQLWIDALL E Space allocation for INSTANCE-ID array failed"

#define SQL_SQLWMEALL \
  "SQLWMEALL E Space allocation for new fetched object failed"

#define SQL_SQLWRIFAI \
  "SQLWRIFAI E SQL database insert or update failed"

#define SQL_SQLZERFIE \
  "SQLZERFIE E No fields specified, aborting write to database"

