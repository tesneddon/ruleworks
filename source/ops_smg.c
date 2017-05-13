/* VMS SMG input for RULEWORKS */
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
**	RULEWORKS compiler
**
**  ABSTRACT:
**	Scanner (tokenizer) implementation
**
**  MODIFIED BY:
**	DEC	Digital Equipment Corporation
**	CPQ	Compaq Computer Corporation
**
**  MODIFICATION HISTORY:
**
**	 9-Dec-1992	DEC	Initial version 
**	01-Dec-1999	CPQ	Release with GPL
*/

/* Application include files */

#include <common.h>
#include <descrip.h>
#include <smgdef.h>
#include <ssdef.h>
#include <rmsdef.h>
#include <devdef.h>
#include <jpidef.h>
#include <dvidef.h>
#include <starlet.h>		/* For system service routine declarations */
#include <smg$routines.h>	/* For smg routine declarations */

/*
 **  structure definitions
 */

struct RUL_SMG {
  int		kbid;
  int           key_tbl;
};

struct RUL_KEY {
  char *name;
  char *ifstate;
  int	attr;
  char *eqiv;
  char *state;
};

struct	ITMLST {		/* Item list structure definition */
  short int      itm$w_buflen;
  short int      itm$w_itmcod;
  void          *itm$a_bufadr;
  long          *itm$a_retlen;
};

static $DESCRIPTOR(desc1, "");
static $DESCRIPTOR(descs0, "");
static $DESCRIPTOR(descs1, "");
static $DESCRIPTOR(descs2, "");
static $DESCRIPTOR(descs3, "");

#define $DESCR(string) \
  (desc1.dsc$w_length = (sizeof string) - 1,\
   desc1.dsc$a_pointer = string,\
   &desc1)

#define $DESCRS0(string) \
  (descs0.dsc$w_length = strlen(string),\
   descs0.dsc$a_pointer = string,\
   &descs0)

#define $DESCRS1(string) \
  (descs1.dsc$w_length = strlen(string),\
   descs1.dsc$a_pointer = string,\
   &descs1)

#define $DESCRS2(string) \
  (descs2.dsc$w_length = strlen(string),\
   descs2.dsc$a_pointer = string,\
   &descs2)

#define $DESCRS3(string) \
  (descs3.dsc$w_length = strlen(string),\
   descs3.dsc$a_pointer = string,\
   &descs3)

#define SET_KEY(i,p1,p2,p3,p4,p5) \
  keys[i].name = p1; \
  keys[i].ifstate = p2; \
  keys[i].attr = p3; \
  keys[i].eqiv = p4; \
  keys[i].state = p5;

/*
 **	Local Storage
 */

static Boolean smg_init = 0;
static struct ITMLST itm1[2];
static struct ITMLST itm2[3];
static struct RUL_KEY keys[30];

static struct RUL_SMG rul_smg;



/************************************************************************
 **
 **  ROUTINE: rul__smg_init
 **
 ************************************************************************/
Boolean rul__smg_init(void)
{
  long                  i;
  long                 recall_size = 40;
  long                 smg_input;
  static unsigned long mode = 0;
  static unsigned long devdepend = 0;
  static unsigned long devchar = 0;

  if (!smg_init) {
    smg_init = TRUE;

    itm1[0].itm$w_buflen = 4;
    itm1[0].itm$w_itmcod = JPI$_MODE;
    itm1[0].itm$a_bufadr = &mode;
    itm1[0].itm$a_retlen = 0;

    itm1[1].itm$w_buflen = 0;
    itm1[1].itm$w_itmcod = 0;
    itm1[1].itm$a_bufadr = 0;
    itm1[1].itm$a_retlen = 0;

    itm2[0].itm$w_buflen = 4;
    itm2[0].itm$w_itmcod = DVI$_DEVCHAR;
    itm2[0].itm$a_bufadr = &devchar;
    itm2[0].itm$a_retlen = 0;

    itm2[1].itm$w_buflen = 4;
    itm2[1].itm$w_itmcod = DVI$_DEVDEPEND;
    itm2[1].itm$a_bufadr = &devdepend;
    itm2[1].itm$a_retlen = 0;

    itm2[2].itm$w_buflen = 0;
    itm2[2].itm$w_itmcod = 0;
    itm2[2].itm$a_bufadr = 0;
    itm2[2].itm$a_retlen = 0;

    SET_KEY( 0,"CTRLZ",      "DEFAULT",SMG$M_KEY_TERMINATE,"","");
    SET_KEY( 1,"PF1",        "DEFAULT",SMG$M_KEY_NOECHO,   "","GOLD");
    SET_KEY( 2,"PF1",        "GOLD",   SMG$M_KEY_NOECHO,   "","DEFAULT");
    SET_KEY( 3,"PF2",        "DEFAULT",SMG$M_KEY_TERMINATE,"","");
    SET_KEY( 4,"PF3",        "DEFAULT",SMG$M_KEY_TERMINATE,"","");
    SET_KEY( 5,"PF4",        "DEFAULT",SMG$M_KEY_TERMINATE,"","");
    SET_KEY( 6,"KP0",        "DEFAULT",SMG$M_KEY_TERMINATE,"RUN 1","");
    SET_KEY( 7,"KP1",        "DEFAULT",SMG$M_KEY_TERMINATE,"","");
    SET_KEY( 8,"KP2",        "DEFAULT",SMG$M_KEY_TERMINATE,"","");
    SET_KEY( 9,"KP3",        "DEFAULT",SMG$M_KEY_TERMINATE,"","");
    SET_KEY(10,"KP4",        "DEFAULT",SMG$M_KEY_TERMINATE,"","");
    SET_KEY(11,"KP5",        "DEFAULT",SMG$M_KEY_TERMINATE,"","");
    SET_KEY(12,"KP6",        "DEFAULT",SMG$M_KEY_TERMINATE,"","");
    SET_KEY(13,"KP7",        "DEFAULT",SMG$M_KEY_TERMINATE,"","");
    SET_KEY(14,"KP8",        "DEFAULT",SMG$M_KEY_TERMINATE,"","");
    SET_KEY(15,"KP9",        "DEFAULT",SMG$M_KEY_TERMINATE,"","");
    SET_KEY(16,"PERIOD",     "DEFAULT",SMG$M_KEY_TERMINATE,"NEXT", "");
    SET_KEY(17,"COMMA",      "DEFAULT",SMG$M_KEY_TERMINATE,"RUN",  "");
    SET_KEY(18,"ENTER",      "DEFAULT",SMG$M_KEY_TERMINATE,"","");
    SET_KEY(19,"MINUS",      "DEFAULT",SMG$M_KEY_TERMINATE,"","");
    SET_KEY(20,"FIND",       "DEFAULT",SMG$M_KEY_TERMINATE,"","");
    SET_KEY(21,"INSERT_HERE","DEFAULT",SMG$M_KEY_TERMINATE,"","");
    SET_KEY(22,"REMOVE",     "DEFAULT",SMG$M_KEY_TERMINATE,"","");
    SET_KEY(23,"SELECT",     "DEFAULT",SMG$M_KEY_TERMINATE,"","");
    SET_KEY(24,"PREV_SCREEN","DEFAULT",SMG$M_KEY_TERMINATE,"","");
    SET_KEY(25,"NEXT_SCREEN","DEFAULT",SMG$M_KEY_TERMINATE,"","");
    SET_KEY(26,"HELP",       "DEFAULT",SMG$M_KEY_TERMINATE,"","");
    SET_KEY(27,"",           "",       0,                  "","");
  }

  smg_input = 0;
    
  if (sys$getjpiw(0, 0, 0, itm1, 0, 0, 0) == SS$_NORMAL) {
    if (mode == JPI$K_INTERACTIVE) {
      if (sys$getdviw(0, 0, $DESCR("SYS$INPUT"), itm2,
		      0, 0, 0, 0) == SS$_NORMAL) {
        if (devchar & DEV$M_TRM)
          smg_input = ((devdepend >> 24) & 255);
      }
    }
  }

  if (smg_input) {
    if (smg$create_virtual_keyboard(&rul_smg.kbid, 0, 0,
	0, &recall_size) != SS$_NORMAL)
      smg_input = 0;
    else if (smg$create_key_table(&rul_smg.key_tbl) != SS$_NORMAL)
      smg_input = 0;
  }

  if (smg_input) {
    for (i = 0;  strlen(keys[i].name); i++) {
      smg$add_key_def(&rul_smg.key_tbl,
		      $DESCRS0(keys[i].name),
		      $DESCRS1(keys[i].ifstate),
		      &keys[i].attr,
		      $DESCRS2(keys[i].eqiv),
		      $DESCRS3(keys[i].state));
    }
  }
  return (smg_input != 0);
}



/************************************************************************
 **
 **  ROUTINE: rul__smg_input
 **
 ************************************************************************/
char *rul__smg_input(buf, maxlen, prompt, cmd_cont)
  char      *buf;
  unsigned   maxlen;
  char      *prompt;
  Boolean    cmd_cont;    
{
  static   char         *smg_buf;
  static   long          smg_buf_len;
  unsigned int           return_value = 0;
  unsigned long          status;
  unsigned short         len = 0;
  unsigned short         tcode = 0;
  struct dsc$descriptor  data;
  struct dsc$descriptor  real_prompt;
  static   char         *prompt_str;
  static   long          prompt_str_len;

  if (smg_buf_len < maxlen) {
    smg_buf_len = maxlen + 40;
    smg_buf = rul__mem_realloc (smg_buf, smg_buf_len);
  }

  data.dsc$w_length = maxlen;
  data.dsc$b_dtype = DSC$K_DTYPE_T;
  data.dsc$b_class = DSC$K_CLASS_S;
  data.dsc$a_pointer = smg_buf;

  if (cmd_cont) {
    if (prompt_str_len <= (strlen (prompt) + 2)) {
      prompt_str_len = strlen (prompt) + 2;
      prompt_str = rul__mem_realloc (prompt_str, prompt_str_len);
    }
    strcpy(prompt_str, "_");
    strcat(prompt_str, prompt);
    real_prompt.dsc$w_length = strlen(prompt_str);
    real_prompt.dsc$b_dtype = DSC$K_DTYPE_T;
    real_prompt.dsc$b_class = DSC$K_CLASS_S;
    real_prompt.dsc$a_pointer = prompt_str;
  }
  else {
    real_prompt.dsc$w_length = strlen(prompt);
    real_prompt.dsc$b_dtype = DSC$K_DTYPE_T;
    real_prompt.dsc$b_class = DSC$K_CLASS_S;
    real_prompt.dsc$a_pointer = prompt;
  }

  status = smg$read_composed_line(&rul_smg.kbid,   /* key board id   */
				  &rul_smg.key_tbl,/* key table id   */
				  &data,           /* result string  */
				  &real_prompt,    /* prompt string  */
				  &len,            /* result length  */
				  0,               /* display id     */
				  0,               /* flags          */
				  0,               /* initial string */
				  0,               /* timeout        */
				  0,               /* rendition-set  */
				  0,               /* rendition-comp */
				  &tcode);         /* word term code */

  buf[0] = '\0';
  if (status == RMS$_EOF || tcode == SMG$K_TRM_CTRLZ)
    buf[0] = '\0';
  else if (status & 1) {
    strncpy (buf, smg_buf, len);
    buf[len++] = '\n';
    buf[len] = '\0';
    return_value = len;
  }
  else
    return_value = -1;

  if (return_value < 0)
    return NULL;
  else
    return (buf);
}
