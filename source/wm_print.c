/*
 *	rts_wm.c - implements the Working Memory subsystem
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
**  FACILITY:
**	RULEWORKS
**
**  ABSTRACT:
**	
**
**  MODIFIED BY:
**	DEC	Digital Equipment Corporation
**	CPQ	Compaq Computer Corporation
**
**  MODIFICATION HISTORY:
**
**	11-Aug-1992	DEC	Initial version
**	01-Dec-1992	DEC	Add *_histforms
**	16-Feb-1998	DEC	class type changed to rclass
**	01-Dec-1999	CPQ	Release  with GPL
*/

#include <common.h>		/* defn. of Molecule, Class, and Object	*/
#include <mol.h>		/* declarations of molecule functions	*/
#include <ios.h>		/* declarations of I/O stream functions	*/
#include <decl.h>		/* declarations of class/attribute funcs*/
#include <wm.h>			/* declarations of WM public functions	*/
#include <decl_att.h>		/* offsets of builtin attributes	*/
#include <wm_p.h>		/* declarations of WM private types	*/
#include <limits.h>		/* defn. of LONG_MAX			*/
#include <dbg.h>		/* dbg_enable & BKN                     */

/* These constants are potentially misnamed and are also misplaced @@@	*/

#ifdef __STDC__
#define xstr(s) #s
#define str(s) xstr(s)
#else
#define str(s) "s"
#endif

/* These macros are used in calculating the number of characters used to
   represent a WME */

#define MAX_CHARS_IN_LONG sizeof(str(LONG_MAX))
#define MAX_CHARS_IN_TIME_TAG MAX_CHARS_IN_LONG

#define STRINGS_IN_WMO(num_attrs_in_wmo) \
	(6 + 1 + (4 * (num_attrs_in_wmo - DECL_NUM_BUILTIN_ATTRS)) + 1)

#define CONST_STR_LEN(str)		(sizeof(str) - 1)
#define VAR_STR_LEN(var)		strlen(var)
#define MOL_READFORM_STR_LEN(mol)	rul__mol_get_readform_length(mol)
#define MOL_PRINTFORM_STR_LEN(mol)	rul__mol_get_printform_length(mol)
#define PUSH_STR_LEN(len)		str_len[str_index++] = len
#define INCR_BUF_LEN(len)		buf_len += len
#define ATTR_NAME(rclass,attr_num)	rul__decl_get_attr_name(rclass,attr_num)
#define ATTR_VALUE(wmo,attr_num)	WM_GET_OBJ_ATTR_VAL(wmo,attr_num)
#define CRE_RULE(wmo)			WM_GET_OBJ_CRE_RULE(wmo)
#define CRE_RB(wmo)			WM_GET_OBJ_CRE_RB(wmo)
#define ATTR_RULE(wmo,attr_num)		WM_GET_OBJ_ATTR_RULE(wmo,attr_num)
#define ATTR_RB(wmo,attr_num)		WM_GET_OBJ_ATTR_RB(wmo,attr_num)
#define POP_STR_LEN()			str_len[str_index++]
#define DECR_BUF_LEN(len)		buf_len -= len
#define ADVANCE_BUF(len)		buf += len
#define GET_STR_LEN()			str_len[str_index]
#define PUT_MOL_PRINTFORM_IN_BUF(mol)	rul__mol_use_printform(mol,buf,buf_len)
#define PUT_MOL_READFORM_IN_BUF(mol)	rul__mol_use_readform(mol,buf,buf_len)
#define PUT_STRING_IN_BUF(str)		strcpy(buf,str)

static Molecule nil_val = NULL;
static Molecule nil_comp_val = NULL;


char *
rul__wm_get_printform (Object wmo)
{
  long str_index;	/* index of string within WMO buffer		     */
  long *str_len;	/* array of string lengths within WMO buffer	     */
  long buf_len; 	/*cumulative/remaining buffer length to represent WMO*/
  long attr_num;	/*attribute number (used to index through attributes)*/
  long num_attrs;	/* total number of attibutes in this object-class    */
  char *buffer; 	/* pointer to entire character buffer for the WMO    */
  char *buf;		/* pointer to end of WMO buffer			     */
  char time_tag_buf[MAX_CHARS_IN_TIME_TAG];	/* buffer to store time-tag
						   to see how big it is before
						   copying it to the WMO buf */

  if (wmo == NULL) {
    buffer = rul__mem_malloc (sizeof (char));
    buffer[0] = '\0';
    return buffer;
  }

  str_index = 0;
  num_attrs = WM_GET_OBJ_NUM_ATTRS (wmo);
  str_len = rul__mem_malloc (sizeof (long) * STRINGS_IN_WMO (num_attrs));
  buf_len = 0;
  sprintf (time_tag_buf, "%ld", WM_GET_OBJ_TIME_TAG (wmo));
  
  INCR_BUF_LEN (PUSH_STR_LEN (MOL_PRINTFORM_STR_LEN (
					     WM_GET_OBJ_IDENTIFIER (wmo))));
  INCR_BUF_LEN (PUSH_STR_LEN (CONST_STR_LEN (" ")));
  INCR_BUF_LEN (PUSH_STR_LEN (VAR_STR_LEN (time_tag_buf)));
  INCR_BUF_LEN (PUSH_STR_LEN (CONST_STR_LEN (" [")));
  if (rul__dbg_gl_enable & DBG_M_ENABLE_BKN) {
     if (WM_GET_OBJ_MOD_RB (wmo)) {
	INCR_BUF_LEN (PUSH_STR_LEN (MOL_PRINTFORM_STR_LEN (
					   WM_GET_OBJ_MOD_RB (wmo))));
	INCR_BUF_LEN (PUSH_STR_LEN (CONST_STR_LEN ("~")));
     }
  }
  INCR_BUF_LEN (PUSH_STR_LEN (MOL_PRINTFORM_STR_LEN (
					     WM_GET_OBJ_MOD_RULE (wmo))));
  INCR_BUF_LEN (PUSH_STR_LEN (CONST_STR_LEN ("] (")));
  INCR_BUF_LEN (PUSH_STR_LEN (MOL_PRINTFORM_STR_LEN (
					     WM_GET_OBJ_CLASS_NAME(wmo))));
  
  /* for each attribute in the WMO... */
  for (attr_num=DECL_FIRST_USER_ATTR_OFFSET; attr_num<num_attrs; attr_num++) {
    INCR_BUF_LEN (PUSH_STR_LEN (CONST_STR_LEN (" ^")));
    INCR_BUF_LEN (PUSH_STR_LEN (MOL_PRINTFORM_STR_LEN (
			    ATTR_NAME (WM_GET_OBJ_CLASS (wmo), attr_num))));
    INCR_BUF_LEN (PUSH_STR_LEN (CONST_STR_LEN (" ")));
    INCR_BUF_LEN (PUSH_STR_LEN (MOL_PRINTFORM_STR_LEN (
					    ATTR_VALUE (wmo, attr_num))));
  }
  
  INCR_BUF_LEN(PUSH_STR_LEN(CONST_STR_LEN(")")));
  
  buffer = rul__mem_malloc(buf_len * sizeof(char));

  str_index = 0;
  buf = buffer;

  PUT_MOL_PRINTFORM_IN_BUF (WM_GET_OBJ_IDENTIFIER (wmo));
  ADVANCE_BUF (GET_STR_LEN ());
  DECR_BUF_LEN (POP_STR_LEN ());

  PUT_STRING_IN_BUF (" ");
  ADVANCE_BUF (GET_STR_LEN ());
  DECR_BUF_LEN (POP_STR_LEN ());

  PUT_STRING_IN_BUF (time_tag_buf);
  ADVANCE_BUF (GET_STR_LEN ());
  DECR_BUF_LEN (POP_STR_LEN ());

  PUT_STRING_IN_BUF (" [");
  ADVANCE_BUF (GET_STR_LEN ());
  DECR_BUF_LEN (POP_STR_LEN ());

  if (rul__dbg_gl_enable & DBG_M_ENABLE_BKN) {
     if (WM_GET_OBJ_MOD_RB (wmo)) {
	PUT_MOL_PRINTFORM_IN_BUF (WM_GET_OBJ_MOD_RB (wmo));
	ADVANCE_BUF (GET_STR_LEN ());
	DECR_BUF_LEN (POP_STR_LEN ());

	PUT_STRING_IN_BUF ("~");
	ADVANCE_BUF (GET_STR_LEN ());
	DECR_BUF_LEN (POP_STR_LEN ());
     }
  }

  PUT_MOL_PRINTFORM_IN_BUF (WM_GET_OBJ_MOD_RULE (wmo));
  ADVANCE_BUF (GET_STR_LEN ());
  DECR_BUF_LEN (POP_STR_LEN ());

  PUT_STRING_IN_BUF ("] (");
  ADVANCE_BUF (GET_STR_LEN ());
  DECR_BUF_LEN (POP_STR_LEN ());

  PUT_MOL_PRINTFORM_IN_BUF (WM_GET_OBJ_CLASS_NAME (wmo));
  ADVANCE_BUF (GET_STR_LEN ());
  DECR_BUF_LEN (POP_STR_LEN ());

  /* for each attribute in the WMO... */
  for (attr_num = DECL_FIRST_USER_ATTR_OFFSET;
       attr_num < num_attrs;
       attr_num++) {

    PUT_STRING_IN_BUF (" ^");
    ADVANCE_BUF (GET_STR_LEN ());
    DECR_BUF_LEN (POP_STR_LEN ());

    PUT_MOL_PRINTFORM_IN_BUF (ATTR_NAME (WM_GET_OBJ_CLASS (wmo), attr_num));
    ADVANCE_BUF (GET_STR_LEN ());
    DECR_BUF_LEN (POP_STR_LEN ());

    PUT_STRING_IN_BUF (" ");
    ADVANCE_BUF (GET_STR_LEN ());
    DECR_BUF_LEN (POP_STR_LEN ());

    PUT_MOL_PRINTFORM_IN_BUF (ATTR_VALUE (wmo, attr_num));
    ADVANCE_BUF (GET_STR_LEN ());
    DECR_BUF_LEN (POP_STR_LEN ());
  }

  PUT_STRING_IN_BUF (")");
  ADVANCE_BUF (GET_STR_LEN ());
  DECR_BUF_LEN (POP_STR_LEN ());

  assert(buf_len == 0);

  rul__mem_free (str_len);
  
  return buffer;
}

Boolean
rul__wm_use_printform (Object wmo, char *buffer, long max_chars)
{
  long attr_num;
  long num_attrs;
  long str_len;
  Boolean mol_fits_in_buf;
  char time_tag_buf[sizeof(" ")-1+MAX_CHARS_IN_TIME_TAG+sizeof(" ]")];
  char *buf;

#define APPEND_CONST_TO_BUF(str) \
  if (max_chars < sizeof(str)) \
    return FALSE; \
  strcpy(buf, str); \
  buf += (sizeof(str) - 1); \
  max_chars -= (sizeof(str) - 1)

#define APPEND_STRING_TO_BUF(str) \
  str_len = strlen(str); \
  if (max_chars < str_len) \
    return FALSE; \
  strcpy(buf, str); \
  buf += str_len; \
  max_chars -= str_len

#define APPEND_MOL_PRINT_FORM_TO_BUF(mol) \
  mol_fits_in_buf = rul__mol_use_printform(mol, buf, max_chars); \
  if (! mol_fits_in_buf) \
    return FALSE; \
  str_len = strlen(buf); \
  buf += str_len; \
  max_chars -= str_len


  /* set a local variable to be a pointer to the end of the buffer */
  buf = buffer;

  /* make sure the buffer can be terminated */
  if (max_chars < 1)
    return FALSE;

  /* make sure the buffer is terminated */
  buf[0] = '\0';

  if (wmo == NULL)
    return FALSE;
    
  /* attempt to store the object's instance identifier in the buffer */
  APPEND_MOL_PRINT_FORM_TO_BUF (WM_GET_OBJ_IDENTIFIER (wmo));

  /* store the object's time-tag in a local buffer (since we must find out
  how long it is before we can attempt to store it in the destination buffer)*/
  sprintf (time_tag_buf, " %ld [", WM_GET_OBJ_TIME_TAG (wmo));

  /* find out who many characters would be used to store the time-tag */
  APPEND_STRING_TO_BUF (time_tag_buf);

  if (rul__dbg_gl_enable & DBG_M_ENABLE_BKN) {
     /* store the name of the rule block which modified this object */
     if (WM_GET_OBJ_MOD_RB (wmo)) {
	APPEND_MOL_PRINT_FORM_TO_BUF (WM_GET_OBJ_MOD_RB (wmo));
	APPEND_CONST_TO_BUF ("~");
     }
  }

  /* attempt to store the name of the rule which modified this object */
  APPEND_MOL_PRINT_FORM_TO_BUF (WM_GET_OBJ_MOD_RULE (wmo));

  /* if there aren't enough characters left in the buffer; return failure */
  APPEND_CONST_TO_BUF ("] (");

  /* attempt to store the object's class name */
  APPEND_MOL_PRINT_FORM_TO_BUF (WM_GET_OBJ_CLASS_NAME (wmo));

  num_attrs = WM_GET_OBJ_NUM_ATTRS (wmo);

  /* for each attribute in the WMO... */
  for (attr_num = DECL_FIRST_USER_ATTR_OFFSET;
       attr_num < num_attrs;
       attr_num++) {

    /* attempt to store the attribute name */
    APPEND_CONST_TO_BUF (" ^");
    APPEND_MOL_PRINT_FORM_TO_BUF (ATTR_NAME (
				     WM_GET_OBJ_CLASS (wmo), attr_num));

    /* store the attribute value */
    APPEND_CONST_TO_BUF (" ");
    APPEND_MOL_PRINT_FORM_TO_BUF (ATTR_VALUE (wmo, attr_num));
  }

  APPEND_CONST_TO_BUF (")");

  return TRUE;
}

void
rul__wm_print_printform (Object wmo, IO_Stream ios)
{ /* #1 1 [RULE] (OBJECT-CLASS ^ATTRIBUTE-1 VALUE-1 ...) */
  long attr_num, num_attrs;

  if (wmo == NULL)
    return;

  /* print the object's instance identifier */
  rul__mol_print_printform (WM_GET_OBJ_IDENTIFIER (wmo), ios);

  /* print the object's time-tag */
  rul__ios_printf (ios, " %ld [", WM_GET_OBJ_TIME_TAG (wmo));

  if (rul__dbg_gl_enable & DBG_M_ENABLE_BKN) {
     /* print the name of the rule which last modified the object */
     if (WM_GET_OBJ_MOD_RB (wmo)) {
	rul__mol_print_printform (WM_GET_OBJ_MOD_RB (wmo), ios);
	rul__ios_printf (ios, "~");
     }
  }
  rul__mol_print_printform (WM_GET_OBJ_MOD_RULE (wmo), ios);
  rul__ios_printf (ios, "] (");

  /* print the object's class name */
  rul__mol_print_printform (WM_GET_OBJ_CLASS_NAME (wmo), ios);

  num_attrs = WM_GET_OBJ_NUM_ATTRS(wmo);

  /* for each attribute in the WMO... */
  for (attr_num = DECL_FIRST_USER_ATTR_OFFSET;
       attr_num < num_attrs;
       attr_num++) {

    /* print the attribute name */
    rul__ios_printf (ios, " ^");
    rul__mol_print_printform (ATTR_NAME (WM_GET_OBJ_CLASS (wmo), attr_num),
			      ios);

    /* print the attribute value */
    rul__ios_printf (ios, " ");
    rul__mol_print_printform (ATTR_VALUE (wmo, attr_num), ios);
  }

  rul__ios_printf (ios, ")");
}

char *
rul__wm_get_readform (Object wmo)
{
  long str_index;	/* index of string within WMO buffer		     */
  long *str_len;	/* array of string lengths within WMO buffer	     */
  long buf_len; 	/*cumulative/remaining buffer length to represent WMO*/
  long attr_num;	/*attribute number (used to index through attributes)*/
  long num_attrs;	/* total number of attibutes in this object-class    */
  char *buffer; 	/* pointer to entire character buffer for the WMO    */
  char *buf;		/* pointer to end of WMO buffer			     */
  Boolean print_it;	/* flag for printing nil attr/value pairs            */
  char time_tag_buf[MAX_CHARS_IN_TIME_TAG];	/* buffer to store time-tag
						   to see how big it is before
						   copying it to the WMO buf */


  if (wmo == NULL) {
    buffer = rul__mem_malloc (sizeof (char));
    buffer[0] = '\0';
    return buffer;
  }

  if (nil_val == NULL) {
    nil_val = rul__mol_symbol_nil ();
    nil_comp_val = rul__mol_make_comp (0);
    rul__mol_mark_perm (nil_comp_val);
  }

  str_index = 0;
  num_attrs = WM_GET_OBJ_NUM_ATTRS (wmo);
  str_len = rul__mem_malloc (sizeof (long) * STRINGS_IN_WMO (num_attrs));
  buf_len = 0;
  sprintf (time_tag_buf ,"%ld", WM_GET_OBJ_TIME_TAG (wmo));

  INCR_BUF_LEN (PUSH_STR_LEN (MOL_READFORM_STR_LEN (
				    WM_GET_OBJ_IDENTIFIER (wmo))));
  INCR_BUF_LEN (PUSH_STR_LEN (CONST_STR_LEN (" ")));
  INCR_BUF_LEN (PUSH_STR_LEN (VAR_STR_LEN (time_tag_buf)));
  INCR_BUF_LEN (PUSH_STR_LEN (CONST_STR_LEN (" [")));
  if (rul__dbg_gl_enable & DBG_M_ENABLE_BKN) {
     if (WM_GET_OBJ_MOD_RB (wmo)) {
	INCR_BUF_LEN (PUSH_STR_LEN (MOL_READFORM_STR_LEN (
					  WM_GET_OBJ_MOD_RB (wmo))));
	INCR_BUF_LEN (PUSH_STR_LEN (CONST_STR_LEN ("~")));
     }
  }
  INCR_BUF_LEN (PUSH_STR_LEN (MOL_READFORM_STR_LEN (
				    WM_GET_OBJ_MOD_RULE (wmo))));
  INCR_BUF_LEN (PUSH_STR_LEN (CONST_STR_LEN ("] (")));
  if (rul__dbg_gl_enable & DBG_M_ENABLE_BKN) {
     INCR_BUF_LEN (PUSH_STR_LEN (MOL_READFORM_STR_LEN (
				    rul__decl_get_class_block_name (
						    WM_GET_OBJ_CLASS (wmo)))));
     INCR_BUF_LEN (PUSH_STR_LEN (CONST_STR_LEN ("~")));
  }
  INCR_BUF_LEN (PUSH_STR_LEN (MOL_READFORM_STR_LEN (
					    WM_GET_OBJ_CLASS_NAME (wmo))));

  /* for each attribute in the WMO... */
  for (attr_num = DECL_FIRST_USER_ATTR_OFFSET;
       attr_num < num_attrs;
       attr_num++) {

    print_it = TRUE;

    if ((rul__decl_get_attr_shape (WM_GET_OBJ_CLASS (wmo),
	   ATTR_NAME (WM_GET_OBJ_CLASS (wmo), attr_num)) == shape_compound)) {
      if ((rul__decl_get_attr_default (WM_GET_OBJ_CLASS (wmo),
	      ATTR_NAME (WM_GET_OBJ_CLASS (wmo), attr_num)) == nil_comp_val) &&
	  (ATTR_VALUE (wmo,attr_num) == nil_comp_val)) {
	print_it = FALSE;
      }
    }
    else {
      if ((rul__decl_get_attr_default (WM_GET_OBJ_CLASS (wmo),
	        ATTR_NAME (WM_GET_OBJ_CLASS (wmo), attr_num)) == nil_val) &&
	  (ATTR_VALUE (wmo, attr_num) == nil_val)) {
	print_it = FALSE;
      }
    }
    if (print_it == TRUE) {
      INCR_BUF_LEN (PUSH_STR_LEN (CONST_STR_LEN (" ^")));
      INCR_BUF_LEN (PUSH_STR_LEN (MOL_READFORM_STR_LEN (
			     ATTR_NAME (WM_GET_OBJ_CLASS (wmo), attr_num))));
      INCR_BUF_LEN (PUSH_STR_LEN (CONST_STR_LEN (" ")));
      INCR_BUF_LEN (PUSH_STR_LEN (MOL_READFORM_STR_LEN (
			     ATTR_VALUE (wmo, attr_num))));
    }
  }

  INCR_BUF_LEN (PUSH_STR_LEN (CONST_STR_LEN (")")));

  buffer = rul__mem_malloc (buf_len * sizeof (char));

  str_index = 0;
  buf = buffer;

  PUT_MOL_READFORM_IN_BUF (WM_GET_OBJ_IDENTIFIER (wmo));
  ADVANCE_BUF (GET_STR_LEN ());
  DECR_BUF_LEN (POP_STR_LEN ());

  PUT_STRING_IN_BUF (" ");
  ADVANCE_BUF (GET_STR_LEN ());
  DECR_BUF_LEN (POP_STR_LEN ());

  PUT_STRING_IN_BUF (time_tag_buf);
  ADVANCE_BUF (GET_STR_LEN  ());
  DECR_BUF_LEN (POP_STR_LEN ());

  PUT_STRING_IN_BUF (" [");
  ADVANCE_BUF (GET_STR_LEN ());
  DECR_BUF_LEN (POP_STR_LEN ());

  if (rul__dbg_gl_enable & DBG_M_ENABLE_BKN) {
     if (WM_GET_OBJ_MOD_RB (wmo)) {
	PUT_MOL_READFORM_IN_BUF (WM_GET_OBJ_MOD_RB (wmo));
	ADVANCE_BUF (GET_STR_LEN ());
	DECR_BUF_LEN (POP_STR_LEN ());

	PUT_STRING_IN_BUF ("~");
	ADVANCE_BUF (GET_STR_LEN ());
	DECR_BUF_LEN (POP_STR_LEN ());
     }
  }

  PUT_MOL_READFORM_IN_BUF (WM_GET_OBJ_MOD_RULE (wmo));
  ADVANCE_BUF (GET_STR_LEN ());
  DECR_BUF_LEN (POP_STR_LEN ());

  PUT_STRING_IN_BUF ("] (");
  ADVANCE_BUF (GET_STR_LEN ());
  DECR_BUF_LEN (POP_STR_LEN ());

  if (rul__dbg_gl_enable & DBG_M_ENABLE_BKN) {
     PUT_MOL_READFORM_IN_BUF (rul__decl_get_class_block_name (
						    WM_GET_OBJ_CLASS (wmo)));
     ADVANCE_BUF (GET_STR_LEN ());
     DECR_BUF_LEN (POP_STR_LEN ());
     PUT_STRING_IN_BUF ("~");
     ADVANCE_BUF (GET_STR_LEN ());
     DECR_BUF_LEN (POP_STR_LEN ());
  }
  PUT_MOL_READFORM_IN_BUF (WM_GET_OBJ_CLASS_NAME (wmo));
  ADVANCE_BUF (GET_STR_LEN ());
  DECR_BUF_LEN (POP_STR_LEN ());

  /* for each attribute in the WMO... */
  for (attr_num=DECL_FIRST_USER_ATTR_OFFSET; attr_num<num_attrs; attr_num++) {
    print_it = TRUE;

    if ((rul__decl_get_attr_shape (WM_GET_OBJ_CLASS (wmo),
	   ATTR_NAME (WM_GET_OBJ_CLASS (wmo),attr_num)) == shape_compound)) {
      if ((rul__decl_get_attr_default (WM_GET_OBJ_CLASS (wmo),
	   ATTR_NAME (WM_GET_OBJ_CLASS (wmo),attr_num)) == nil_comp_val) &&
	  (ATTR_VALUE (wmo,attr_num) == nil_comp_val)) {
	print_it = FALSE; }
    }
    else {
      if ((rul__decl_get_attr_default (WM_GET_OBJ_CLASS (wmo),
	   ATTR_NAME (WM_GET_OBJ_CLASS (wmo),attr_num)) == nil_val) &&
	  (ATTR_VALUE (wmo,attr_num) == nil_val)) {
	print_it = FALSE; }
    }
    if (print_it == TRUE) {
      PUT_STRING_IN_BUF (" ^");
      ADVANCE_BUF (GET_STR_LEN ());
      DECR_BUF_LEN (POP_STR_LEN ());

      PUT_MOL_READFORM_IN_BUF (ATTR_NAME (WM_GET_OBJ_CLASS (wmo),attr_num));
      ADVANCE_BUF (GET_STR_LEN ());
      DECR_BUF_LEN (POP_STR_LEN ());

      PUT_STRING_IN_BUF (" ");
      ADVANCE_BUF (GET_STR_LEN ());
      DECR_BUF_LEN (POP_STR_LEN ());

      PUT_MOL_READFORM_IN_BUF (ATTR_VALUE (wmo,attr_num));
      ADVANCE_BUF (GET_STR_LEN ());
      DECR_BUF_LEN (POP_STR_LEN ());
    }
  }

  PUT_STRING_IN_BUF (")");
  ADVANCE_BUF (GET_STR_LEN ());
  DECR_BUF_LEN (POP_STR_LEN ());

  assert (buf_len == 0);

  rul__mem_free (str_len);
  
  return buffer;
}

Boolean
rul__wm_use_readform (Object wmo, char *buffer, long max_chars)
{
  long attr_num;
  long num_attrs;
  long str_len;
  Boolean mol_fits_in_buf;
  char time_tag_buf[sizeof (" ")-1+MAX_CHARS_IN_TIME_TAG+sizeof (" ]")];
  char *buf;
  Boolean print_it;	/* flag for printing nil attr/value pairs            */

#define APPEND_CONST_TO_BUF(str) \
  if (max_chars < sizeof(str)) \
    return FALSE; \
  strcpy(buf, str); \
  buf += (sizeof(str) - 1); \
  max_chars -= (sizeof(str) - 1)

#define APPEND_STRING_TO_BUF(str) \
  str_len = strlen(str); \
  if (max_chars < str_len) \
    return FALSE; \
  strcpy(buf, str); \
  buf += str_len; \
  max_chars -= str_len

#define APPEND_MOL_READ_FORM_TO_BUF(mol) \
  mol_fits_in_buf = rul__mol_use_readform(mol, buf, max_chars); \
  if (! mol_fits_in_buf) \
    return FALSE; \
  str_len = strlen(buf); \
  buf += str_len; \
  max_chars -= str_len

#define APPEND_MOL_PRINT_FORM_TO_BUF(mol) \
  mol_fits_in_buf = rul__mol_use_printform(mol, buf, max_chars); \
  if (! mol_fits_in_buf) \
    return FALSE; \
  str_len = strlen(buf); \
  buf += str_len; \
  max_chars -= str_len

  if (nil_val == NULL) {
    nil_val = rul__mol_symbol_nil ();
    nil_comp_val = rul__mol_make_comp (0);
    rul__mol_mark_perm (nil_comp_val);
  }

  /* set a local variable to be a pointer to the end of the buffer */
  buf = buffer;

  /* make sure the buffer can be terminated */
  if (max_chars < 1)
    return FALSE;

  /* make sure the buffer is terminated */
  buf[0] = '\0';

  if (wmo == NULL)
    return FALSE;

  /* attempt to store the object's instance identifier in the buffer */
  APPEND_MOL_READ_FORM_TO_BUF (WM_GET_OBJ_IDENTIFIER (wmo));

  /* store the object's time-tag in a local buffer (since we must find out
  how long it is before we can attempt to store it in the destination buffer)*/
  sprintf (time_tag_buf, " %ld [", WM_GET_OBJ_TIME_TAG (wmo));

  /* find out who many characters would be used to store the time-tag */
  APPEND_STRING_TO_BUF (time_tag_buf);

  if (rul__dbg_gl_enable & DBG_M_ENABLE_BKN) {
     /* attempt to store the rule block which last modified this object */
     if (WM_GET_OBJ_MOD_RB (wmo)) {
	APPEND_MOL_READ_FORM_TO_BUF (WM_GET_OBJ_MOD_RB (wmo));
	APPEND_CONST_TO_BUF ("~");
     }
  }

  APPEND_MOL_READ_FORM_TO_BUF (WM_GET_OBJ_MOD_RULE (wmo));
  APPEND_CONST_TO_BUF ("] (");

  if (rul__dbg_gl_enable & DBG_M_ENABLE_BKN) {
     /* attempt to store the object's block&class name */
     APPEND_MOL_READ_FORM_TO_BUF (rul__decl_get_class_block_name (
					       WM_GET_OBJ_CLASS	(wmo)));
     APPEND_CONST_TO_BUF ("~");
  }
  APPEND_MOL_READ_FORM_TO_BUF (WM_GET_OBJ_CLASS_NAME (wmo));

  num_attrs = WM_GET_OBJ_NUM_ATTRS (wmo);

  /* for each attribute in the WMO... */
  for (attr_num=DECL_FIRST_USER_ATTR_OFFSET; attr_num<num_attrs; attr_num++) {
    print_it = TRUE;

    if ((rul__decl_get_attr_shape (WM_GET_OBJ_CLASS (wmo),
	   ATTR_NAME (WM_GET_OBJ_CLASS (wmo),attr_num)) == shape_compound)) {
      if ((rul__decl_get_attr_default (WM_GET_OBJ_CLASS (wmo),
	   ATTR_NAME (WM_GET_OBJ_CLASS (wmo),attr_num)) == nil_comp_val) &&
	  (ATTR_VALUE (wmo,attr_num) == nil_comp_val)) {
	print_it = FALSE; }
    }
    else {
      if ((rul__decl_get_attr_default (WM_GET_OBJ_CLASS (wmo),
	   ATTR_NAME (WM_GET_OBJ_CLASS (wmo),attr_num)) == nil_val) &&
	  (ATTR_VALUE (wmo,attr_num) == nil_val)) {
	print_it = FALSE; }
    }
    if (print_it == TRUE) {

      /* attempt to store the attribute name */
      APPEND_CONST_TO_BUF (" ^");
      APPEND_MOL_READ_FORM_TO_BUF (ATTR_NAME (WM_GET_OBJ_CLASS (wmo),attr_num));

      /* store the attribute value */
      APPEND_CONST_TO_BUF (" ");
      APPEND_MOL_READ_FORM_TO_BUF (ATTR_VALUE (wmo,attr_num));
    }
  }
  APPEND_CONST_TO_BUF (")");

  return TRUE;
}

long
rul__wm_get_readform_length (Object wmo)
{
  long attr_num;
  long num_attrs;
  long str_len = 0;
  char time_tag_buf[sizeof(" ")-1+MAX_CHARS_IN_TIME_TAG+sizeof(" ]")];
  Boolean print_it;	/* flag for printing nil attr/value pairs            */

  if (nil_val == NULL) {
    nil_val = rul__mol_symbol_nil ();
    nil_comp_val = rul__mol_make_comp (0);
    rul__mol_mark_perm (nil_comp_val);
  }

  if (wmo == NULL)
    return 0;

  /* add the object's instance identifier length */
  str_len += rul__mol_get_readform_length (WM_GET_OBJ_IDENTIFIER  (wmo));

  /* store the object's time-tag in a local buffer to find the length */
  sprintf (time_tag_buf, " %ld [", WM_GET_OBJ_TIME_TAG (wmo));
  str_len += strlen (time_tag_buf);

  if (rul__dbg_gl_enable & DBG_M_ENABLE_BKN) {
     /* add name of the rule block length */
     if (WM_GET_OBJ_MOD_RB (wmo)) {
	str_len += rul__mol_get_readform_length (WM_GET_OBJ_MOD_RB (wmo));
	/* increment for "~" */
	str_len += 1;
     }
  }

  /* add name of the rule length */
  str_len += rul__mol_get_readform_length (WM_GET_OBJ_MOD_RULE (wmo));

  /* increment for "] (" */
  str_len += 3;

  /* add block~ length */
  if (rul__dbg_gl_enable & DBG_M_ENABLE_BKN) {
     str_len += rul__mol_get_readform_length (
			   rul__decl_get_class_block_name (
					    WM_GET_OBJ_CLASS (wmo)));
     str_len += 1;
  }

  /* add class length */
  str_len += rul__mol_get_readform_length (WM_GET_OBJ_CLASS_NAME (wmo));

  num_attrs = WM_GET_OBJ_NUM_ATTRS (wmo);

  /* for each attribute in the WMO... */
  for (attr_num=DECL_FIRST_USER_ATTR_OFFSET; attr_num<num_attrs; attr_num++) {
    print_it = TRUE;

    if ((rul__decl_get_attr_shape (WM_GET_OBJ_CLASS (wmo),
	   ATTR_NAME (WM_GET_OBJ_CLASS (wmo),attr_num)) == shape_compound)) {
      if ((rul__decl_get_attr_default (WM_GET_OBJ_CLASS (wmo),
	   ATTR_NAME (WM_GET_OBJ_CLASS (wmo),attr_num)) == nil_comp_val) &&
	  (ATTR_VALUE (wmo,attr_num) == nil_comp_val)) {
	print_it = FALSE; }
    }
    else {
      if ((rul__decl_get_attr_default (WM_GET_OBJ_CLASS (wmo),
	   ATTR_NAME (WM_GET_OBJ_CLASS (wmo),attr_num)) == nil_val) &&
	  (ATTR_VALUE (wmo,attr_num) == nil_val)) {
	print_it = FALSE; }
    }
    if (print_it == TRUE) {

      /* add length of " ^" */
      str_len += 2;

      /* add length of attribute name */
      str_len += rul__mol_get_readform_length (
		       ATTR_NAME (WM_GET_OBJ_CLASS (wmo), attr_num));
    
      /* add length of " " */
      str_len += 1;

      /* add length of attribute value */
      str_len += rul__mol_get_readform_length (ATTR_VALUE  (wmo, attr_num));
    }
  }

  /* add length of ")" */
  str_len += 1;

  return str_len;
}


void
rul__wm_print_readform (Object wmo, IO_Stream ios)
{ /* #1 1 [RULE] (OBJECT-CLASS ^ATTRIBUTE-1 VALUE-1 ...) */
  long attr_num, num_attrs;
  Boolean print_it;	/* flag for printing nil attr/value pairs            */

  if (wmo == NULL)
    return;

  if (nil_val == NULL) {
    nil_val = rul__mol_symbol_nil ();
    nil_comp_val = rul__mol_make_comp (0);
    rul__mol_mark_perm (nil_comp_val);
  }

  /* print the object's instance identifier */
  rul__mol_print_readform (WM_GET_OBJ_IDENTIFIER (wmo), ios);

  /* print the object's time-tag */
  rul__ios_printf (ios, " %ld [", WM_GET_OBJ_TIME_TAG (wmo));

  if (rul__dbg_gl_enable & DBG_M_ENABLE_BKN) {
     /* print the name of the rule which last modified the object */
     if (WM_GET_OBJ_MOD_RB (wmo)) {
	rul__mol_print_readform (WM_GET_OBJ_MOD_RB(wmo), ios);
	rul__ios_printf (ios, "~");
     }
  }
  rul__mol_print_readform (WM_GET_OBJ_MOD_RULE(wmo), ios);
  rul__ios_printf (ios, "] (");

  if (rul__dbg_gl_enable & DBG_M_ENABLE_BKN) {
     /* print the object's block&class name */
     rul__mol_print_readform (rul__decl_get_class_block_name (
					      WM_GET_OBJ_CLASS (wmo)), ios);
     rul__ios_printf (ios, "~");
  }
  rul__mol_print_readform (WM_GET_OBJ_CLASS_NAME (wmo), ios);

  num_attrs = WM_GET_OBJ_NUM_ATTRS (wmo);

  /* for each attribute in the WMO... */
  for (attr_num=DECL_FIRST_USER_ATTR_OFFSET; attr_num<num_attrs; attr_num++) {
    print_it = TRUE;

    if ((rul__decl_get_attr_shape (WM_GET_OBJ_CLASS (wmo),
	   ATTR_NAME (WM_GET_OBJ_CLASS (wmo),attr_num)) == shape_compound)) {
      if ((rul__decl_get_attr_default  (WM_GET_OBJ_CLASS (wmo),
	   ATTR_NAME (WM_GET_OBJ_CLASS (wmo),attr_num)) == nil_comp_val) &&
	  (ATTR_VALUE (wmo,attr_num) == nil_comp_val)) {
	print_it = FALSE; }
    }
    else {
      if ((rul__decl_get_attr_default (WM_GET_OBJ_CLASS (wmo),
	   ATTR_NAME (WM_GET_OBJ_CLASS (wmo),attr_num)) == nil_val) &&
	  (ATTR_VALUE (wmo,attr_num) == nil_val)) {
	print_it = FALSE; }
    }
    if (print_it == TRUE) {

      /* print the attribute name */
      rul__ios_printf (ios, " ^");
      rul__mol_print_readform (ATTR_NAME (WM_GET_OBJ_CLASS (wmo), attr_num),
			       ios);

      /* print the attribute value */
      rul__ios_printf (ios, " ");
      rul__mol_print_readform (ATTR_VALUE (wmo,attr_num), ios);
    }
  }

  rul__ios_printf (ios, ")");
}


char *
rul__wm_get_histform (Object wmo, Molecule att)
{
  long   str_index;	/* index of string within WMO buffer		     */
  long  *str_len;	/* array of string lengths within WMO buffer	     */
  long   buf_len; 	/*cumulative/remaining buffer length to represent WMO*/
  long   attr_num;	/*attribute number (used to index through attributes)*/
  long   num_attrs;	/* total number of attibutes in this object-class    */
  char  *buffer; 	/* pointer to entire character buffer for the WMO    */
  char  *buf;		/* pointer to end of WMO buffer			     */
  Class  rclass;	/* wmo's class					     */
  char   time_tag_buf[MAX_CHARS_IN_TIME_TAG];	/* buffer to store time-tag
						   to see how big it is before
						   copying it to the WMO buf */


  if (wmo == NULL) {
    buffer = rul__mem_malloc (sizeof (char));
    buffer[0] = '\0';
    return buffer;
  }

  str_index = 0;
  num_attrs = WM_GET_OBJ_NUM_ATTRS (wmo);
  str_len = rul__mem_malloc (sizeof (long) * STRINGS_IN_WMO (num_attrs));
  buf_len = 0;
  sprintf (time_tag_buf,"%ld",WM_GET_OBJ_TIME_TAG (wmo));
  rclass = WM_GET_OBJ_CLASS (wmo);

  INCR_BUF_LEN (PUSH_STR_LEN (MOL_READFORM_STR_LEN (
					    WM_GET_OBJ_IDENTIFIER (wmo))));
  INCR_BUF_LEN (PUSH_STR_LEN (CONST_STR_LEN (" ")));
  INCR_BUF_LEN (PUSH_STR_LEN (VAR_STR_LEN (time_tag_buf)));
  INCR_BUF_LEN (PUSH_STR_LEN (CONST_STR_LEN (" [")));
  if (rul__dbg_gl_enable & DBG_M_ENABLE_BKN) {
     if (WM_GET_OBJ_MOD_RB (wmo)) {
	INCR_BUF_LEN (PUSH_STR_LEN (MOL_READFORM_STR_LEN (
					      WM_GET_OBJ_MOD_RB (wmo))));
	INCR_BUF_LEN (PUSH_STR_LEN (CONST_STR_LEN ("~")));
     }
  }
  INCR_BUF_LEN (PUSH_STR_LEN (MOL_READFORM_STR_LEN (
					    WM_GET_OBJ_MOD_RULE (wmo))));
  INCR_BUF_LEN (PUSH_STR_LEN (CONST_STR_LEN ("] (")));
  if (rul__dbg_gl_enable & DBG_M_ENABLE_BKN) {
     INCR_BUF_LEN (PUSH_STR_LEN (MOL_READFORM_STR_LEN (
				       rul__decl_get_class_block_name (
					    WM_GET_OBJ_CLASS (wmo)))));
     INCR_BUF_LEN (PUSH_STR_LEN (CONST_STR_LEN ("~")));
  }
  INCR_BUF_LEN (PUSH_STR_LEN (MOL_READFORM_STR_LEN (
					    WM_GET_OBJ_CLASS_NAME (wmo))));
  INCR_BUF_LEN (PUSH_STR_LEN (CONST_STR_LEN (" [")));
  if (rul__dbg_gl_enable & DBG_M_ENABLE_BKN) {
     if (CRE_RB (wmo)) {
	INCR_BUF_LEN (PUSH_STR_LEN (MOL_READFORM_STR_LEN (CRE_RB (wmo))));
	INCR_BUF_LEN (PUSH_STR_LEN (CONST_STR_LEN ("~")));
     }
  }
  INCR_BUF_LEN (PUSH_STR_LEN (MOL_READFORM_STR_LEN (CRE_RULE (wmo))));
  INCR_BUF_LEN (PUSH_STR_LEN (CONST_STR_LEN ("]")));

  /* for each attribute in the WMO... */
  for (attr_num=DECL_FIRST_USER_ATTR_OFFSET; attr_num<num_attrs; attr_num++){

    if (!att || att == ATTR_NAME (rclass, attr_num)) {

      INCR_BUF_LEN (PUSH_STR_LEN (CONST_STR_LEN (" ^")));
      INCR_BUF_LEN (PUSH_STR_LEN (MOL_READFORM_STR_LEN (
		  		  ATTR_NAME (rclass,attr_num))));
      INCR_BUF_LEN (PUSH_STR_LEN (CONST_STR_LEN (" ")));
      INCR_BUF_LEN (PUSH_STR_LEN (MOL_READFORM_STR_LEN (
				  ATTR_VALUE (wmo,attr_num))));
      INCR_BUF_LEN (PUSH_STR_LEN (CONST_STR_LEN (" [")));
      if (ATTR_RB (wmo, attr_num)) {
	 if (rul__dbg_gl_enable & DBG_M_ENABLE_BKN) {
	    INCR_BUF_LEN (PUSH_STR_LEN (MOL_READFORM_STR_LEN (
						  ATTR_RB (wmo, attr_num))));
	    INCR_BUF_LEN (PUSH_STR_LEN (CONST_STR_LEN ("~")));
	 }
      }
      INCR_BUF_LEN (PUSH_STR_LEN (MOL_READFORM_STR_LEN (
				  ATTR_RULE (wmo, attr_num))));
      INCR_BUF_LEN (PUSH_STR_LEN (CONST_STR_LEN ("]")));
    }
  }

  INCR_BUF_LEN (PUSH_STR_LEN (CONST_STR_LEN (")")));

  buffer = rul__mem_malloc (buf_len * sizeof (char));

  str_index = 0;
  buf = buffer;

  PUT_MOL_READFORM_IN_BUF (WM_GET_OBJ_IDENTIFIER (wmo));
  ADVANCE_BUF (GET_STR_LEN ());
  DECR_BUF_LEN (POP_STR_LEN ());

  PUT_STRING_IN_BUF (" ");
  ADVANCE_BUF (GET_STR_LEN ());
  DECR_BUF_LEN (POP_STR_LEN ());

  PUT_STRING_IN_BUF (time_tag_buf);
  ADVANCE_BUF (GET_STR_LEN ());
  DECR_BUF_LEN (POP_STR_LEN ());

  PUT_STRING_IN_BUF (" [");
  ADVANCE_BUF (GET_STR_LEN ());
  DECR_BUF_LEN (POP_STR_LEN ());

  if (rul__dbg_gl_enable & DBG_M_ENABLE_BKN) {
     if (WM_GET_OBJ_MOD_RB (wmo)) {
	PUT_MOL_READFORM_IN_BUF (WM_GET_OBJ_MOD_RB (wmo));
	ADVANCE_BUF (GET_STR_LEN ());
	DECR_BUF_LEN (POP_STR_LEN ());

	PUT_STRING_IN_BUF ("~");
	ADVANCE_BUF (GET_STR_LEN ());
	DECR_BUF_LEN (POP_STR_LEN ());
     }
  }

  PUT_MOL_READFORM_IN_BUF (WM_GET_OBJ_MOD_RULE (wmo));
  ADVANCE_BUF (GET_STR_LEN ());
  DECR_BUF_LEN (POP_STR_LEN ());

  PUT_STRING_IN_BUF ("] (");
  ADVANCE_BUF (GET_STR_LEN ());
  DECR_BUF_LEN (POP_STR_LEN ());

  if (rul__dbg_gl_enable & DBG_M_ENABLE_BKN) {
     PUT_MOL_READFORM_IN_BUF (rul__decl_get_class_block_name (
						    WM_GET_OBJ_CLASS (wmo)));
     ADVANCE_BUF (GET_STR_LEN ());
     DECR_BUF_LEN (POP_STR_LEN ());
     PUT_STRING_IN_BUF ("~");
  }
  PUT_MOL_READFORM_IN_BUF (WM_GET_OBJ_CLASS_NAME (wmo));
  ADVANCE_BUF (GET_STR_LEN ());
  DECR_BUF_LEN (POP_STR_LEN ());

  PUT_STRING_IN_BUF (" [");
  ADVANCE_BUF (GET_STR_LEN ());
  DECR_BUF_LEN (POP_STR_LEN ());

  if (rul__dbg_gl_enable & DBG_M_ENABLE_BKN) {
     if (CRE_RB (wmo)) {
	PUT_MOL_READFORM_IN_BUF (CRE_RB (wmo));
	ADVANCE_BUF (GET_STR_LEN ());
	DECR_BUF_LEN (POP_STR_LEN ());

	PUT_STRING_IN_BUF ("~");
	ADVANCE_BUF (GET_STR_LEN ());
	DECR_BUF_LEN (POP_STR_LEN ());
     }
  }

  PUT_MOL_READFORM_IN_BUF (CRE_RULE (wmo));
  ADVANCE_BUF (GET_STR_LEN ());
  DECR_BUF_LEN (POP_STR_LEN ());

  PUT_STRING_IN_BUF ("]");
  ADVANCE_BUF (GET_STR_LEN ());
  DECR_BUF_LEN (POP_STR_LEN ());

  /* for each attribute in the WMO... */
  for (attr_num=DECL_FIRST_USER_ATTR_OFFSET; attr_num<num_attrs; attr_num++) {

    if (!att || att == ATTR_NAME (rclass, attr_num)) {

      PUT_STRING_IN_BUF (" ^");
      ADVANCE_BUF (GET_STR_LEN ());
      DECR_BUF_LEN (POP_STR_LEN ());

      PUT_MOL_READFORM_IN_BUF (ATTR_NAME (rclass,attr_num));
      ADVANCE_BUF (GET_STR_LEN ());
      DECR_BUF_LEN (POP_STR_LEN ());

      PUT_STRING_IN_BUF (" ");
      ADVANCE_BUF (GET_STR_LEN ());
      DECR_BUF_LEN (POP_STR_LEN ());

      PUT_MOL_READFORM_IN_BUF (ATTR_VALUE (wmo,attr_num));
      ADVANCE_BUF (GET_STR_LEN ());
      DECR_BUF_LEN (POP_STR_LEN ());

      PUT_STRING_IN_BUF (" [");
      ADVANCE_BUF (GET_STR_LEN ());
      DECR_BUF_LEN (POP_STR_LEN ());

      if (rul__dbg_gl_enable & DBG_M_ENABLE_BKN) {
	 if (ATTR_RB (wmo, attr_num)) {
	    PUT_MOL_READFORM_IN_BUF (ATTR_RB (wmo, attr_num));
	    ADVANCE_BUF (GET_STR_LEN ());
	    DECR_BUF_LEN (POP_STR_LEN ());

	    PUT_STRING_IN_BUF ("~");
	    ADVANCE_BUF (GET_STR_LEN ());
	    DECR_BUF_LEN (POP_STR_LEN ());
	 }
      }

      PUT_MOL_READFORM_IN_BUF (ATTR_RULE (wmo, attr_num));
      ADVANCE_BUF (GET_STR_LEN ());
      DECR_BUF_LEN (POP_STR_LEN ());

      PUT_STRING_IN_BUF ("]");
      ADVANCE_BUF (GET_STR_LEN ());
      DECR_BUF_LEN (POP_STR_LEN ());
    }
  }

  PUT_STRING_IN_BUF (")");
  ADVANCE_BUF (GET_STR_LEN ());
  DECR_BUF_LEN (POP_STR_LEN ());

  assert (buf_len == 0);

  rul__mem_free (str_len);
  
  return buffer;
}

Boolean
rul__wm_use_histform (Object wmo, Molecule att, char *buffer, long max_chars)
{
  long     attr_num;
  long     num_attrs;
  long     str_len;
  Boolean  mol_fits_in_buf;
  char     time_tag_buf[sizeof(" ")-1+MAX_CHARS_IN_TIME_TAG+sizeof(" ]")];
  char    *buf;
  Class    rclass;

#define APPEND_CONST_TO_BUF(str) \
  if (max_chars < sizeof(str)) \
    return FALSE; \
  strcpy(buf, str); \
  buf += (sizeof(str) - 1); \
  max_chars -= (sizeof(str) - 1)

#define APPEND_STRING_TO_BUF(str) \
  str_len = strlen(str); \
  if (max_chars < str_len) \
    return FALSE; \
  strcpy(buf, str); \
  buf += str_len; \
  max_chars -= str_len

#define APPEND_MOL_READ_FORM_TO_BUF(mol) \
  mol_fits_in_buf = rul__mol_use_readform(mol, buf, max_chars); \
  if (! mol_fits_in_buf) \
    return FALSE; \
  str_len = strlen(buf); \
  buf += str_len; \
  max_chars -= str_len

#define APPEND_MOL_PRINT_FORM_TO_BUF(mol) \
  mol_fits_in_buf = rul__mol_use_printform(mol, buf, max_chars); \
  if (! mol_fits_in_buf) \
    return FALSE; \
  str_len = strlen(buf); \
  buf += str_len; \
  max_chars -= str_len

  /* set a local variable to be a pointer to the end of the buffer */
  buf = buffer;

  /* make sure the buffer can be terminated */
  if (max_chars < 1)
    return FALSE;

  /* make sure the buffer is terminated */
  buf[0] = '\0';

  if (wmo == NULL)
    return FALSE;

  /* get this wmo's class */
  rclass = WM_GET_OBJ_CLASS(wmo);

  /* attempt to store the object's instance identifier in the buffer */
  APPEND_MOL_READ_FORM_TO_BUF(WM_GET_OBJ_IDENTIFIER(wmo));

  /* store the object's time-tag in a local buffer (since we must find out
  how long it is before we can attempt to store it in the destination buffer)*/
  sprintf(time_tag_buf, " %ld [", WM_GET_OBJ_TIME_TAG(wmo));

  /* find out who many characters would be used to store the time-tag */
  APPEND_STRING_TO_BUF(time_tag_buf);

  if (rul__dbg_gl_enable & DBG_M_ENABLE_BKN) {
     /* store the name of the rule which last modified this object */
     if (WM_GET_OBJ_MOD_RB (wmo)) {
	APPEND_MOL_PRINT_FORM_TO_BUF(WM_GET_OBJ_MOD_RB(wmo));
	APPEND_CONST_TO_BUF("~");
     }
  }
  APPEND_MOL_PRINT_FORM_TO_BUF(WM_GET_OBJ_MOD_RULE(wmo));
  APPEND_CONST_TO_BUF("] (");

  if (rul__dbg_gl_enable & DBG_M_ENABLE_BKN) {
     /* attempt to store the object's class name */
     APPEND_MOL_PRINT_FORM_TO_BUF (rul__decl_get_class_block_name (
					       WM_GET_OBJ_CLASS	(wmo)));
     APPEND_CONST_TO_BUF ("~");
  }
  APPEND_MOL_PRINT_FORM_TO_BUF (WM_GET_OBJ_CLASS_NAME(wmo));

  if (rul__dbg_gl_enable & DBG_M_ENABLE_BKN) {
     /* attempt to store the name of the rule which cre'd/spec'd this object */
     if (CRE_RB (wmo)) {
	APPEND_MOL_PRINT_FORM_TO_BUF(CRE_RB(wmo));
	APPEND_CONST_TO_BUF("~");
     }
  }
  APPEND_MOL_PRINT_FORM_TO_BUF(CRE_RULE(wmo));

  num_attrs = WM_GET_OBJ_NUM_ATTRS(wmo);

  /* for each attribute in the WMO... */
  for (attr_num=DECL_FIRST_USER_ATTR_OFFSET; attr_num<num_attrs; attr_num++) {

    if (!att || att == ATTR_NAME(rclass, attr_num)) {

      /* attempt to store the attribute name */
      APPEND_CONST_TO_BUF(" ^");
      APPEND_MOL_READ_FORM_TO_BUF(ATTR_NAME(WM_GET_OBJ_CLASS(wmo),attr_num));

      /* store the attribute value */
      APPEND_CONST_TO_BUF(" ");
      APPEND_MOL_READ_FORM_TO_BUF(ATTR_VALUE(wmo,attr_num));

      APPEND_CONST_TO_BUF(" [");
      if (rul__dbg_gl_enable & DBG_M_ENABLE_BKN) {
	 if (ATTR_RB (wmo, attr_num)) {
	    APPEND_MOL_PRINT_FORM_TO_BUF(ATTR_RB(wmo, attr_num));
	    APPEND_CONST_TO_BUF("~");
	 }
      }
      APPEND_MOL_PRINT_FORM_TO_BUF(ATTR_RULE(wmo, attr_num));
      APPEND_CONST_TO_BUF("]");
    }
  }

  APPEND_CONST_TO_BUF(")");

  return TRUE;
}


void
rul__wm_print_histform (Object wmo, Molecule att, IO_Stream ios)
{ /* #1 1 [RULE] (OBJECT-CLASS [RULE] ^ATTRIBUTE-1 VALUE-1 ...) */
  long attr_num, num_attrs;
  Class  rclass;

  if (wmo == NULL)
    return;

  /* get this wmo's class */
  rclass = WM_GET_OBJ_CLASS(wmo);

  /* print the object's instance identifier */
  rul__mol_print_readform (WM_GET_OBJ_IDENTIFIER(wmo), ios);

  /* print the object's time-tag */
  rul__ios_printf (ios, " %ld [", WM_GET_OBJ_TIME_TAG(wmo));

  if (rul__dbg_gl_enable & DBG_M_ENABLE_BKN) {
     /* print the name of the rule which last modified the object */
     if (WM_GET_OBJ_MOD_RB (wmo)) {
	rul__mol_print_readform (WM_GET_OBJ_MOD_RB(wmo), ios);
	rul__ios_printf (ios, "~");
     }
  }
  rul__mol_print_readform (WM_GET_OBJ_MOD_RULE(wmo), ios);
  rul__ios_printf (ios, "] (");

  if (rul__dbg_gl_enable & DBG_M_ENABLE_BKN) {
     /* print the object's class name */
     rul__mol_print_readform (rul__decl_get_class_block_name (
					   WM_GET_OBJ_CLASS (wmo)), ios);
     rul__ios_printf (ios, "~");
  }
  rul__mol_print_readform (WM_GET_OBJ_CLASS_NAME(wmo), ios);

  /* print the name of the rule which last cre'd/spe'd the object */
  rul__ios_printf (ios, " [");
  if (rul__dbg_gl_enable & DBG_M_ENABLE_BKN) {
     if (CRE_RB (wmo)) {
	rul__mol_print_readform (CRE_RB(wmo), ios);
	rul__ios_printf (ios, "~");
     }
  }
  rul__mol_print_readform (CRE_RULE(wmo), ios);
  rul__ios_printf (ios, "]");

  num_attrs = WM_GET_OBJ_NUM_ATTRS(wmo);

  /* for each attribute in the WMO... */
  for (attr_num=DECL_FIRST_USER_ATTR_OFFSET; attr_num<num_attrs; attr_num++) {

    if (!att || att == ATTR_NAME(rclass, attr_num)) {

      /* print the attribute name */
      rul__ios_printf (ios, " ^");
      rul__mol_print_readform(ATTR_NAME(rclass, attr_num), ios);

      /* print the attribute value */
      rul__ios_printf (ios, " ");
      rul__mol_print_readform(ATTR_VALUE(wmo, attr_num), ios);

      rul__ios_printf (ios, " [");
      if (rul__dbg_gl_enable & DBG_M_ENABLE_BKN) {
	 if (ATTR_RB (wmo, attr_num)) {
	    rul__mol_print_readform(ATTR_RB(wmo, attr_num), ios);
	    rul__ios_printf (ios, "~");
	 }
      }
      rul__mol_print_readform(ATTR_RULE(wmo, attr_num), ios);
      rul__ios_printf (ios, "]");
    }
  }
  rul__ios_printf (ios, ")");
}


Boolean
rul__wm_use_wmoform (Object wmo, char *buffer, long max_chars)
{
  long attr_num;
  long num_attrs;
  long str_len;
  Boolean mol_fits_in_buf;
  char *buf;

#define APPEND_CONST_TO_BUF(str) \
  if (max_chars < sizeof(str)) \
    return FALSE; \
  strcpy(buf, str); \
  buf += (sizeof(str) - 1); \
  max_chars -= (sizeof(str) - 1)

#define APPEND_STRING_TO_BUF(str) \
  str_len = strlen(str); \
  if (max_chars < str_len) \
    return FALSE; \
  strcpy(buf, str); \
  buf += str_len; \
  max_chars -= str_len

#define APPEND_MOL_READ_FORM_TO_BUF(mol) \
  mol_fits_in_buf = rul__mol_use_readform(mol, buf, max_chars); \
  if (! mol_fits_in_buf) \
    return FALSE; \
  str_len = strlen(buf); \
  buf += str_len; \
  max_chars -= str_len

#define APPEND_MOL_PRINT_FORM_TO_BUF(mol) \
  mol_fits_in_buf = rul__mol_use_printform(mol, buf, max_chars); \
  if (! mol_fits_in_buf) \
    return FALSE; \
  str_len = strlen(buf); \
  buf += str_len; \
  max_chars -= str_len

  /* set a local variable to be a pointer to the end of the buffer */
  buf = buffer;

  /* make sure the buffer can be terminated */
  if (max_chars < 1)
    return FALSE;

  /* make sure the buffer is terminated */
  buf[0] = '\0';

  if (wmo == NULL)
    return FALSE;

  APPEND_CONST_TO_BUF("(");

  if (rul__dbg_gl_enable & DBG_M_ENABLE_BKN) {
     /* attempt to store the object's block&class name */
     APPEND_MOL_READ_FORM_TO_BUF (rul__decl_get_class_block_name (
					       WM_GET_OBJ_CLASS	(wmo)));
     APPEND_CONST_TO_BUF ("~");
  }
  APPEND_MOL_READ_FORM_TO_BUF (WM_GET_OBJ_CLASS_NAME(wmo));

  APPEND_CONST_TO_BUF(" ^$ID ");

  /* attempt to store the object's instance identifier in the buffer */
  APPEND_MOL_READ_FORM_TO_BUF(WM_GET_OBJ_IDENTIFIER(wmo));

  num_attrs = WM_GET_OBJ_NUM_ATTRS(wmo);

  /* for each attribute in the WMO... */
  for (attr_num=DECL_FIRST_USER_ATTR_OFFSET; attr_num<num_attrs; attr_num++) {

    /* attempt to store the attribute name */
    APPEND_CONST_TO_BUF(" ^");
    APPEND_MOL_READ_FORM_TO_BUF(ATTR_NAME(WM_GET_OBJ_CLASS(wmo),attr_num));

    /* store the attribute value */
    APPEND_CONST_TO_BUF(" ");
    APPEND_MOL_READ_FORM_TO_BUF(ATTR_VALUE(wmo,attr_num));
  }
  APPEND_CONST_TO_BUF(")");

  return TRUE;
}

long
rul__wm_get_wmoform_length (Object wmo)
{
  long attr_num;
  long num_attrs;
  long str_len = 0;

  if (wmo == NULL)
    return 0;

  /* increment for "(" */
  str_len += 1;

  /* add block~ length */
  if (rul__dbg_gl_enable & DBG_M_ENABLE_BKN) {
     str_len += rul__mol_get_readform_length (
			   rul__decl_get_class_block_name (
					    WM_GET_OBJ_CLASS (wmo)));
     str_len += 1;
  }

  /* add class length */
  str_len += rul__mol_get_readform_length (WM_GET_OBJ_CLASS_NAME (wmo));

  /* increment for " ^$ID " */
  str_len += 6;

  /* add id length */
  str_len += rul__mol_get_readform_length (WM_GET_OBJ_IDENTIFIER (wmo));

  num_attrs = WM_GET_OBJ_NUM_ATTRS(wmo);

  /* for each attribute in the WMO... */
  for (attr_num=DECL_FIRST_USER_ATTR_OFFSET; attr_num<num_attrs; attr_num++) {

    /* add length of " ^" */
    str_len += 2;

    /* add length of attribute name */
    str_len += rul__mol_get_readform_length (ATTR_NAME (WM_GET_OBJ_CLASS(wmo),
							attr_num));
    
    /* add length of " " */
    str_len += 1;

    /* add length of attribute value */
    str_len += rul__mol_get_readform_length (ATTR_VALUE (wmo, attr_num));
  }

  /* add length of ")" */
  str_len += 1;

  return str_len;
}


void
rul__wm_print_saveform (Object wmo, IO_Stream ios)
{ /*(DECL-BLOCK~OBJECT-CLASS ^$ID ID ^ATTRIBUTE-1 VALUE-1 ...) */
  long attr_num, num_attrs;

  if (wmo == NULL)
    return;

  rul__ios_printf (ios, "(");

  /* print the object's block&class name */
  rul__mol_print_readform (rul__decl_get_class_block_name (
				  WM_GET_OBJ_CLASS (wmo)), ios);
  rul__ios_printf (ios, "~");
  rul__mol_print_readform (WM_GET_OBJ_CLASS_NAME(wmo), ios);

  /* print the object's instance identifier */
  rul__ios_printf (ios, " ^");
  rul__mol_print_readform (ATTR_NAME (WM_GET_OBJ_CLASS (wmo),
				       DECL_ID_ATTR_OFFSET),
			    ios);
  rul__ios_printf (ios, " ");
  rul__mol_print_readform (WM_GET_OBJ_IDENTIFIER(wmo), ios);

  num_attrs = WM_GET_OBJ_NUM_ATTRS(wmo);

  /* for each attribute in the WMO... */
  for (attr_num = DECL_FIRST_USER_ATTR_OFFSET;
       attr_num < num_attrs;
       attr_num++) {

    /* print the attribute name */
    rul__ios_printf (ios, " ^");
    rul__mol_print_readform(ATTR_NAME(WM_GET_OBJ_CLASS(wmo),attr_num), ios);

    /* print the attribute value */
    rul__ios_printf (ios, " ");
    rul__mol_print_readform(ATTR_VALUE(wmo,attr_num), ios);
  }

  rul__ios_printf (ios, ")");
}



void
rul__wm_hist (
#if __VMS
		Object *wmo, Molecule *att)
#else
		Object wmo, Molecule att)
#endif
  /*
  **   For use directly from the debugger, only.
  **   Object:   #1 1 [RULE] (OBJECT-CLASS [RULE] ^ATTRIBUTE-1 VALUE-1 ...)
  */
{
  rul__ios_printf ( RUL__C_STD_ERR, "\n  Object:  ");
  rul__wm_print_histform (
#if __VMS
		*wmo, *att,
#else
		wmo, att,
#endif
		RUL__C_STD_ERR);
  rul__ios_printf ( RUL__C_STD_ERR, "\n");
}


void
rul__wm_print (
#if __VMS
		Object *wmo)
#else
		Object wmo)
#endif
  /*
  **   For use directly from the debugger, only.
  **   Object:   #1 1 [RULE] (OBJECT-CLASS ^ATTRIBUTE-1 VALUE-1 ...)
  */
{
  rul__ios_printf ( RUL__C_STD_ERR, "\n  Object:  ");
  rul__wm_print_readform (
#if __VMS
		*wmo,
#else
		wmo,
#endif
		RUL__C_STD_ERR);
  rul__ios_printf ( RUL__C_STD_ERR, "\n");
}

