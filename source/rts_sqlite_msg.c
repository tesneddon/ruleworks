/*
 * rts_sqlite_msg.c - RULEWORKS SQLite interface message utility routine
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
 *	RULEWORKS SQLite interface ( run-time system)
 *
 * ABSTRACT:
 *      Besides displaying the error messages we rorms an SQL
 *	message WME.  This message WME will contain attributes for
 *	the severity, ident, text of the condition, and the name
 *	of the current rule.
 *	
 * MODIFIED BY:
 *	ESS	Endless Sofwtare Solutions
 *
 * MODIFICATION HISTORY:
 *
 *	15-May-2017	ESS	Created.
 */

#include <common.h>
#include <mol.h>
#include <msg.h>
#include <sql_p.h>
#include <sql.h>

EXTERNAL Mol_Symbol rul__rac_active_rule_name;
EXTERNAL long       rul__rac_rule_firing_cycle;



/*
 * Returns the individual parts of a messge
 */
static char msg_ident_buffer    [MSG__C_LONGEST_MSGID+1];
static char msg_severity_buffer [2] = {0,0};
static char msg_text_buffer     [MSG__C_MAX_MESSAGE_LEN+1];

static Boolean
rul___sql_get_message_parts (char *message_str, char **i, char **s, char **t)
{
  char *severity;		/* Pointer to severity character */
  char *text;			/* pointer to text */
  unsigned id_length;		/* Number of chars in id */

  msg_ident_buffer[0] = '\0';
  msg_severity_buffer[0] = '\0';
  msg_text_buffer[0] = '\0';

  id_length = strchr (message_str, ' ') - message_str;
  assert (id_length <= MSG__C_LONGEST_MSGID);
  strncpy (msg_ident_buffer, message_str, id_length);
  msg_ident_buffer[id_length] = '\0';
  *i = msg_ident_buffer;

  severity = strchr (message_str, ' ') + 1; /* First char after space */
  msg_severity_buffer[0] = severity[0];
  *s = msg_severity_buffer;

  text = severity + 2;	/* One char for severity, one for space */
  strcpy (msg_text_buffer, text);
  *t = msg_text_buffer;

  return TRUE;
}



/************************
 **		       **
 **  rul___sql_message **
 **		       **
 ***********************/

long  rul___sql_message (char *error, void *opt_sql_msg)
{
  char            rule[RUL_C_MAX_SYMBOL_SIZE+1];/* current rule name */
  char            msg_wme[MAX_MSG_WME_SIZE+1];/* max size for make msg WME */
  char		 *msg_ident;
  char		 *msg_severity;
  char		 *msg_text;

  /* report the optional rdb error */

  //if (opt_rdb_msg)
  //  sys$putmsg(opt_rdb_msg, 0, 0, 0);

  rul__msg_print (error);

  /*----------------------------------------------------------------------*/
  /* Also create a message WME, which will include attributes to contain the 
   * message text obtained above, plus attributes for condition severity,
   * condition identifier, and for the current (firing) rule name.
   */
  
  rul___sql_get_message_parts (error, &msg_ident, &msg_severity, &msg_text);

  strcpy (msg_wme, "(SQL$MSG ");
  
  /* Then append the message severity to the message WME */
  strcat (msg_wme, " ^SEV ");
  strcat (msg_wme, msg_severity);
#ifdef TEST
  printf ("\n\n       (severity = '%s') ", msg_severity);
#endif
  
  /* Then append the message condition (ident) to the message WME */
  strcat (msg_wme, " ^COND ");
  strcat (msg_wme, msg_ident);
#ifdef TEST
  printf ("\n\n       (cond ID = '%s') ", msg_ident);
#endif
  
  
  /* Then append the message text to the message WME */
  strcat (msg_wme, " ^TEXT |");
  strcat (msg_wme, msg_text);
  strcat (msg_wme, "|");
#ifdef TEST
  printf ("\n\n       (msg text = '%s') ", msg_text);
#endif
  
  /* And finally, obtain the current (firing) rule's name, and append it to
   * the message WME; end message WME with close parenthesis:
   */
  rul__mol_use_readform (rul__rac_active_rule_name, rule,
			 RUL_C_MAX_SYMBOL_SIZE);
  rule[ rul__mol_get_readform_length (rul__rac_active_rule_name) ] = '\0';
  
  strcat (msg_wme, " ^RULE ");
  strcat (msg_wme, rule);
  strcat (msg_wme, ")");
  
#ifdef TEST
  printf ("\n\n   msg WME = '%s' \n\n", msg_wme);
#endif
  
  rul__make_wme (msg_wme, "");
  
  return RUL_SQL_SUCCESS;
}			    /* end of rul___sql_message routine */

