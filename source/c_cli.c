/*
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

*/


#include <common.h>
#include <cli.h>
#include <ios.h>
#include <msg.h>
#include <cmp_msg.h>
#include <ver_msg.h>

#define CLI_MAX_ARG_SIZE 1024
#define CLI_MAX_FILE_NAME_SIZE 256

#define strneql(str1,str2,len) (strncmp(str1,str2,len) == 0)

static char *program;

#define CLI_VALUE_DELIMS "=:"

#ifdef __UNIX
#define CLI_PREFIX_CHARS "-"
#else
#define CLI_PREFIX_CHARS "-/"
#endif

#define CLI_DEBUG_DEFAULT CLI_DEBUG_NO
static char debug_flag = FALSE;
static Debug_Values debug_option = CLI_DEBUG_DEFAULT;

#define CLI_QUIET_DEFAULT FALSE
static char quiet_flag = FALSE;
static Boolean quiet_option = CLI_QUIET_DEFAULT;

#define CLI_LISTING_FILE_EXT ".lis"
static char listing_flag = FALSE;
static char listing_option[CLI_MAX_FILE_NAME_SIZE];

#define CLI_ERROR_FILE_EXT ".err"
static char error_flag = FALSE;
static char error_option[CLI_MAX_FILE_NAME_SIZE];

#define CLI_USEDIR_FILE_EXT RUL__C_DECL_FILE_EXT
static char usedir_flag = FALSE;
static char usedir_option[CLI_MAX_FILE_NAME_SIZE];

#define CLI_NAMES_DEFAULT CLI_NAMES_UPPER
static Names_Values names_option = CLI_NAMES_DEFAULT;

#define CLI_OPTIMIZE_DEFAULT CLI_OPTIMIZE_SPACE
static Optimize_Values optimize_option = CLI_OPTIMIZE_DEFAULT;

#define CLI_OUTPUT_FILE_EXT ".c"
static char output_flag = FALSE;
static char output_option[CLI_MAX_FILE_NAME_SIZE];

#define CLI_INPUT_FILE_EXT RUL__C_DEF_IN_FILE_EXT
static Boolean input_flag = FALSE;
static char input_option[CLI_MAX_FILE_NAME_SIZE];

#ifdef __VMS
#define CLI_VALUE_NO "NO"
#define CLI_VALUE_YES "YES"
#define CLI_VALUE_MAYBE "MAYBE"
#define CLI_VALUE_UPPER "UPPER"
#define CLI_VALUE_LOWER "LOWER"
#define CLI_VALUE_SPACE "SPACE"
#define CLI_VALUE_REINV "REINVOCATION"
#else
#define CLI_VALUE_NO "no"
#define CLI_VALUE_YES "yes"
#define CLI_VALUE_MAYBE "maybe"
#define CLI_VALUE_UPPER "upper"
#define CLI_VALUE_LOWER "lower"
#define CLI_VALUE_SPACE "space"
#define CLI_VALUE_REINV "reinvocation"
#endif

#ifdef __VMS
/* Not used on VMS (but could be ":]>"). */
#else
#ifdef __UNIX
#define FILE_DELIM_CHARS "/"	/* e.g., /dir/foo.rul or ../foo.rul */
#else  /* Not VMS or Unix, assume DOS */
#define FILE_DELIM_CHARS ":\\"	/* e.g., C:\DIR\FOO.RUL or C:FOO.RUL */
#endif /* __UNIX */
#endif /* __VMS */


/***********************************************************************/

Debug_Values
rul__cli_debug_option (void)
{
   return debug_option;
}

Boolean
rul__cli_error_flag (void)
{
   return error_flag;
}

char *
rul__cli_error_option (void)
{
   return error_option;
}

char *
rul__cli_input_option (void)
{
  return input_option;
}

Boolean
rul__cli_listing_flag (void)
{
   return listing_flag;
}

char *
rul__cli_listing_option (void)
{
   return listing_option;
}

Names_Values
rul__cli_names_option (void)
{
   return names_option;
}

Boolean
rul__cli_usedir_flag (void)
{
   return usedir_flag;
}

char *
rul__cli_usedir_option (void)
{
   return usedir_option;
}

Optimize_Values
rul__cli_optimize_option (void)
{
  return optimize_option;
}

Boolean
rul__cli_output_flag (void)
{
  return output_flag;
}

char *
rul__cli_output_option (void)
{
  return output_option;
}

Boolean
rul__cli_quiet_option (void)
{
   return quiet_option;
}

static Debug_Values
cli_debug_option (const char *option_value,
		  const int option_value_size)
{
   Debug_Values dbg_opt_val;

   if (option_value_size == 0)
     dbg_opt_val = CLI_DEBUG_YES;
   else if (strneql (option_value, CLI_VALUE_NO, option_value_size))
     dbg_opt_val = CLI_DEBUG_NO;
   else if (strneql (option_value, CLI_VALUE_MAYBE, option_value_size))
     dbg_opt_val = CLI_DEBUG_MAYBE;
   else if (strneql (option_value, CLI_VALUE_YES, option_value_size))
     dbg_opt_val = CLI_DEBUG_YES;
   else {
      rul__msg_print (CMP_CLIBADVAL, option_value);
      dbg_opt_val = CLI_DEBUG_DEFAULT;
   }
   return dbg_opt_val;
}

static Names_Values
cli_names_option (const char *option_value,
		  const int option_value_size)
{
   Names_Values nam_opt_val;

   if (option_value_size == 0)
     nam_opt_val = CLI_NAMES_LOWER;
   else if (strneql (option_value, CLI_VALUE_UPPER, option_value_size))
     nam_opt_val = CLI_NAMES_UPPER;
   else if (strneql (option_value, CLI_VALUE_LOWER, option_value_size))
     nam_opt_val = CLI_NAMES_LOWER;
   else {
      rul__msg_print (CMP_CLIBADVAL, option_value);
      nam_opt_val = CLI_NAMES_DEFAULT;
   }

   return nam_opt_val;
}

static Optimize_Values
cli_optimize_option (const char *option_value,
		     const int option_value_size)
{
   Optimize_Values opt_opt_val;

   if (option_value_size == 0)
     opt_opt_val = CLI_OPTIMIZE_REINVOCATION;
   else if (strneql (option_value, CLI_VALUE_SPACE, option_value_size))
     opt_opt_val = CLI_OPTIMIZE_SPACE;
   else if (strneql (option_value, CLI_VALUE_REINV, option_value_size))
     opt_opt_val = CLI_OPTIMIZE_REINVOCATION;
   else {
      rul__msg_print (CMP_CLIBADVAL, option_value);
      opt_opt_val = CLI_OPTIMIZE_DEFAULT;
   }

   return opt_opt_val;
}

/***********************************************************************/

#ifdef __VMS
#include <starlet.h>
#include <lib$routines.h>
#include <rmsdef.h>
#include <climsgdef.h>
#include <fscndef.h>
#include <dsc.h>
#include <string.h>

struct	ITM2LST {		/* Item list structure definition */
   short int      itm$w_buflen;
   short int      itm$w_itmcod;
   char          *itm$a_bufadr;
};

static  char   buf[CLI_MAX_ARG_SIZE+1];
static  String dsc1 = { CLI_MAX_ARG_SIZE, DSC_K_DTYPE_T, DSC_K_CLASS_S, buf };

extern long CLI$PRESENT (String *);
extern long CLI$GET_VALUE (String *, String *, short *);



/************************************************************************
 **
 **  ROUTINE: cli_check_parameter
 **
 **  Checks the command line to see if the specified parameter is present
 **
 ************************************************************************/
static Boolean
cli_option_present (char *param)
{
   long    status;
   String  dsc = { 0, DSC_K_DTYPE_T, DSC_K_CLASS_S, NULL };
   
   dsc.dsc_w_length = strlen (param);
   dsc.dsc_a_pointer = param;

   status = CLI$PRESENT (&dsc);
   if ((status == CLI$_PRESENT) || (status == CLI$_LOCPRES))
     return TRUE;
  
   return FALSE;
}



/************************************************************************
 **
 **  ROUTINE: cli_check_qualifier()
 **
 **	Checks the command line to see if the specified qualifier
 **     is present and negated.
 **
 ************************************************************************/
static long
cli_check_qualifier (char *param)
{
   long    status;
   String  dsc = { 0, DSC_K_DTYPE_T, DSC_K_CLASS_S, NULL };
  
   dsc.dsc_w_length = strlen (param);
   dsc.dsc_a_pointer = param;

   status = CLI$PRESENT (&dsc);
   if ((status == CLI$_NEGATED) || (status == CLI$_LOCNEG))
     return -1;
   else if ((status == CLI$_PRESENT) || (status == CLI$_LOCPRES))
     return TRUE;

   return FALSE;
}



/************************************************************************
 **
 **  ROUTINE: cli_get_str_val
 **
 **	    Get the named parameters string value via CLI$GET_VALUE
 **
 ************************************************************************/
static char *
cli_get_str_val (char *param)
{
   long    status;
   short   tmplen;
   String  dsc = { 0, DSC_K_DTYPE_T, DSC_K_CLASS_S, NULL };
  
   dsc.dsc_w_length = strlen (param);
   dsc.dsc_a_pointer = param;

   status = CLI$GET_VALUE (&dsc, &dsc1, &tmplen);
   if ((status == SS$_NORMAL) ||
       (status == CLI$_COMMA) ||
       (status == CLI$_CONCAT)) {
      buf[tmplen] = 0;
      return buf;
   }

   return NULL;
}



/************************************************************************
 **
 **  ROUTINE: cli_get_int_val
 **
 **	    Get the named parameters integer value via CLI$GET_VALUE
 **
 ************************************************************************/
static long
cli_get_int_val (char *param)
{
   long    status;
   short   tmplen;
   char    buf[RUL_C_MAX_SYMBOL_SIZE+1];
   String  dsc = { 0, DSC_K_DTYPE_T, DSC_K_CLASS_S, NULL };
   String  dsc1 = { 0, DSC_K_DTYPE_T, DSC_K_CLASS_S, NULL };
  
   dsc.dsc_w_length = strlen (param);
   dsc.dsc_a_pointer = param;

   dsc.dsc_w_length = RUL_C_MAX_SYMBOL_SIZE;
   dsc1.dsc_a_pointer = buf;

   status = CLI$GET_VALUE (&dsc, &dsc1, &tmplen);
   if ((status == SS$_NORMAL) ||
       (status == CLI$_COMMA) ||
       (status == CLI$_CONCAT))
     buf[tmplen] = 0;
   else
     buf[0] = 0;

   return (atoi (buf));
}



/************************************************************************
 **
 **  cli_filescan()
 **
 **     scan a file spec and return the root dir and name(and type)
 **     
 ************************************************************************/
static void
cli_filescan (char *spec, char *nod, char *dev, char *rot,
	      char *dir, char *nam, char *typ)
{
   unsigned long   fldflags = 0;
   struct ITM2LST  itm[] = { {0, FSCN$_NODE,      0},
			     {0, FSCN$_DEVICE,    0},
			     {0, FSCN$_ROOT,      0},
			     {0, FSCN$_DIRECTORY, 0},
			     {0, FSCN$_NAME,      0},
			     {0, FSCN$_TYPE,      0},
			     {0, 0,               0} };
   String dsc = { 0, DSC_K_DTYPE_T, DSC_K_CLASS_S, NULL };

   dsc.dsc_w_length = strlen (spec);
   dsc.dsc_a_pointer = spec;

   sys$filescan (&dsc, itm, &fldflags);

   if (nod) {
      if (fldflags & FSCN$M_NODE && itm[0].itm$w_buflen) {
	 if (itm[0].itm$w_buflen > CLI_MAX_FILE_NAME_SIZE)
	   itm[0].itm$w_buflen = CLI_MAX_FILE_NAME_SIZE;
	 strncpy (nod, (char *) itm[0].itm$a_bufadr, itm[0].itm$w_buflen);
	 nod[itm[0].itm$w_buflen] = 0;
      }
      else
	*nod = 0;
   }
   
   if (dev) {
      if (fldflags & FSCN$M_DEVICE && itm[1].itm$w_buflen) {
	 if (itm[1].itm$w_buflen > CLI_MAX_FILE_NAME_SIZE)
	   itm[1].itm$w_buflen = CLI_MAX_FILE_NAME_SIZE;
	 strncpy (dev, (char *) itm[1].itm$a_bufadr, itm[1].itm$w_buflen);
	 dev[itm[1].itm$w_buflen] = 0;
      }
      else
	*dev = 0;
   }
   
   if (rot) {
      if (fldflags & FSCN$M_ROOT && itm[2].itm$w_buflen) {
	 if (itm[2].itm$w_buflen > CLI_MAX_FILE_NAME_SIZE)
	   itm[2].itm$w_buflen = CLI_MAX_FILE_NAME_SIZE;
	 strncpy (rot, (char *) itm[2].itm$a_bufadr, itm[2].itm$w_buflen);
	 rot[itm[2].itm$w_buflen] = 0;
      }
      else
	*rot = 0;
   }
   
   if (dir) {
      if (fldflags & FSCN$M_DIRECTORY && itm[3].itm$w_buflen) {
	 if (itm[3].itm$w_buflen > CLI_MAX_FILE_NAME_SIZE)
	   itm[3].itm$w_buflen = CLI_MAX_FILE_NAME_SIZE;
	 strncpy (dir, (char *) itm[3].itm$a_bufadr, itm[3].itm$w_buflen);
	 dir[itm[3].itm$w_buflen] = 0;
      }
      else
	*dir = 0;
   }
   
   if (nam) {
      if (fldflags & FSCN$M_NAME && itm[4].itm$w_buflen) {
	 if (itm[4].itm$w_buflen > 39)
	   itm[4].itm$w_buflen = 39;
	 strncpy (nam, (char *) itm[4].itm$a_bufadr, itm[4].itm$w_buflen);
	 nam[itm[4].itm$w_buflen] = 0;
      }
      else
	*nam = 0;
   }
   
   if (typ) {
      if (fldflags & FSCN$M_TYPE && itm[5].itm$w_buflen) {
	 if (itm[5].itm$w_buflen > 39)
	   itm[5].itm$w_buflen = 39;
	 strncpy (typ, (char *) itm[5].itm$a_bufadr, itm[5].itm$w_buflen);
	 typ[itm[5].itm$w_buflen] = 0;
      }
      else
	*typ = 0;
   }
}

/************************************************************************
 **
 **  cli_locate_file
 **
 **  This function determines whether a file can be located using 
 **  the LIB$FIND_FILE routine.
 **
 ************************************************************************/
static long
cli_locate_file (char *file_name, char *default_name, char *result)
{
   long     status;
   long     ctx = 0;
   String   outdsc = { 0, DSC_K_DTYPE_T, DSC$K_CLASS_D, NULL };
   String   namdsc = { 0, DSC_K_DTYPE_T, DSC_K_CLASS_S, NULL };
   String   defdsc = { 0, DSC_K_DTYPE_T, DSC_K_CLASS_S, NULL };
  
   /*
    **  Determine if the file is available 
    */
  
   namdsc.dsc_w_length = strlen (file_name);
   namdsc.dsc_a_pointer = file_name;

   defdsc.dsc_w_length = strlen (default_name);
   defdsc.dsc_a_pointer = default_name;

   status = lib$find_file (&namdsc,
			   &outdsc,
			   &ctx,
			   &defdsc,
			   0,0,
			   &1);
   lib$find_file_end (&ctx);
  
   strncpy (result, (char *) outdsc.dsc_a_pointer, outdsc.dsc_w_length);
   result[outdsc.dsc_w_length] = '\0';
   lib$sfree1_dd (&outdsc);
   
   if (status == RMS$_NORMAL)
     return TRUE;
  
   if (status == RMS$_FNF)
     rul__msg_print (CMP_FILNOTFND, result);
   return FALSE;
}

static void
cli_create_default_file (char *target_file_spec,
			 char *source_file_spec,
			 char *target_file_ext)
{
   char dev[RUL_C_MAX_SYMBOL_SIZE];
   char dir[RUL_C_MAX_SYMBOL_SIZE];
   char nam[RUL_C_MAX_SYMBOL_SIZE];
   char ext[RUL_C_MAX_SYMBOL_SIZE];

   cli_filescan (source_file_spec, NULL, dev, NULL, dir, nam, ext);
   strcpy (target_file_spec, nam);
   strcat (target_file_spec, target_file_ext);
}

static void
cli_add_file_ext (char *file_spec, char *file_ext)
{
   char dev[RUL_C_MAX_SYMBOL_SIZE];
   char dir[RUL_C_MAX_SYMBOL_SIZE];
   char nam[RUL_C_MAX_SYMBOL_SIZE];
   char ext[RUL_C_MAX_SYMBOL_SIZE];

   cli_filescan (file_spec, NULL, dev, NULL, dir, nam, ext);
   if (!strlen (ext)) {
      strcpy (file_spec, dev);
      strcat (file_spec, dir);
      strcat (file_spec, nam);
      strcat (file_spec, file_ext);
   }
}

Boolean
rul__cli_parse (int argc, char *argv[])
{
   char    *arg; /* used to cycle through argv */
   char     option_name[CLI_MAX_ARG_SIZE];
   Boolean  status;
   char     default_file[CLI_MAX_ARG_SIZE] = "SYS$DISK:[]";
   char     dev[RUL_C_MAX_SYMBOL_SIZE];
   char     dir[RUL_C_MAX_SYMBOL_SIZE];
   
   /* get the input file first...*/
   arg = cli_get_str_val ("INPUT");
   strcpy (input_option, arg);
   strcat (default_file, RUL__C_DEF_IN_FILE_EXT);
   status = cli_locate_file (input_option, default_file, input_option);
   if (!status) {
     return FALSE;
   }

   /* check for debug qualifier */
   debug_flag = cli_check_qualifier ("DEBUG");
   if (debug_flag > 0) {
      arg = cli_get_str_val ("DEBUG");
      if (arg)
	debug_option = cli_debug_option (arg, strlen (arg));
      else
	debug_option = CLI_DEBUG_YES;
   }
   else if (debug_flag < 0)
     debug_option = CLI_DEBUG_NO;

   /* check for error file */
   if (error_flag = cli_option_present ("ERROR_FILE")) {
      arg = cli_get_str_val ("ERROR_FILE");
      if (arg)
	strcpy (error_option, arg);

      if (error_option[0] == '\0')
	cli_create_default_file (error_option, input_option,
				 CLI_ERROR_FILE_EXT);
      else
	cli_add_file_ext (error_option, CLI_ERROR_FILE_EXT);
   }

   /* check for listing file */
   listing_flag = cli_check_qualifier ("LIST_FILE");
   if (listing_flag < 0)
     listing_flag = FALSE;

   if (listing_flag > 0) {
      arg = cli_get_str_val ("LIST_FILE");
      if (arg)
	strcpy (listing_option, arg);

      if (listing_option[0] == '\0')
	cli_create_default_file (listing_option, input_option,
				 CLI_LISTING_FILE_EXT);
      else
	cli_add_file_ext (listing_option, CLI_LISTING_FILE_EXT);
   }

   /* check for names qualifier */
   if (cli_option_present ("NAMES")) {
      arg = cli_get_str_val ("NAMES");
      names_option = cli_names_option (arg, strlen(arg));
   }

   /* check for names optimize */
   if (cli_option_present ("OPTIMIZE")) {
      arg = cli_get_str_val ("OPTIMIZE");
      if (arg)
	optimize_option = cli_optimize_option (arg, strlen(arg));
   }

   /* check for output */
   output_flag = cli_check_qualifier ("OUTPUT_FILE");
   if (output_flag < 0)
     output_flag = FALSE;
   else
     output_flag = TRUE;

   if (output_flag) {
      arg = cli_get_str_val ("OUTPUT_FILE");
      if (arg)
	strcpy (output_option, arg);

      if (output_option[0] == '\0')
	cli_create_default_file (output_option, input_option,
				 CLI_OUTPUT_FILE_EXT);
      else
	cli_add_file_ext (output_option, CLI_OUTPUT_FILE_EXT);
   }

   /* check for usedirectory */
   if (usedir_flag = cli_option_present ("USEDIRECTORY")) {
      arg = cli_get_str_val ("USEDIRECTORY");
      cli_filescan (arg, NULL, dev, NULL, dir, NULL, NULL);
      strcpy (usedir_option, dev);
      strcat (usedir_option, dir);
   }

   /* check for quiet */
   quiet_flag = cli_check_qualifier ("QUIET");
   if (quiet_flag > 0)
     quiet_option = TRUE;
   else if (quiet_flag < 0)
     quiet_option = FALSE;

   return TRUE;
}

#else /* __VMS  */

/*****************************************************************/

static Boolean
cli_option_present (const char *actual_opt_name,
		    const int actual_opt_size,
		    const char *expected_opt_name,
		    const int expected_opt_size)
{
   register Boolean option_found = FALSE;

   if ((actual_opt_size > 0) && (actual_opt_size <= expected_opt_size))
     option_found = strneql (actual_opt_name, expected_opt_name,
			     MIN(actual_opt_size, expected_opt_size));

   return option_found;
}

static void
cli_create_default_file (char *target_file_spec,
			 const char *source_file_spec,
			 const char *target_file_ext)
/*
 * Return filename (without path) with new extension.
 */
{
   char *token;			/* Result of strtok() (NULL when finished) */
   char *prev_token;		/* Points to last token when finished */
   char *dot_posn;

   /*
    * strtok() writes on its first parameter, so we need to copy source spec.
    */
   strcpy(target_file_spec, source_file_spec);

   /*
    * Call strtok() to return tokens one at a time until there are none left.
    * We save the last token and use it as the filename.
    * Note that strtok() isn't reentrant.  Change to strtok_r() if this is bad.
    */
   prev_token = target_file_spec;
   token = strtok(target_file_spec, FILE_DELIM_CHARS);
   while (token != NULL) {
       prev_token = token;
       token = strtok(NULL, FILE_DELIM_CHARS);
   }

   /*
    * Copy filename (or name and extension) into target.  Can't use strcpy()
    * since strings may overlap.
    */
   memmove(target_file_spec, prev_token, strlen(prev_token) + 1);

   dot_posn = strrchr(target_file_spec,'.');
   if (dot_posn == NULL)
     strcat(target_file_spec,target_file_ext);
   else
     strcpy(dot_posn,target_file_ext);
}

Boolean
rul__cli_parse (int argc, char *argv[])
{
   char *arg; /* used to cycle through argv */
   int arg_size;
   char option_name[CLI_MAX_ARG_SIZE];
   int option_name_size;
   char *option_value;
   int option_value_size;
   Boolean status;


   program = argv[0];

   while (argc > 1) {
      /* 1st arg. is name of compiler executable; others? */
      /* parse args from right to left */
      arg = argv[--argc];
      
      /* non-input arg must begin with prefix  */
      if (strchr(CLI_PREFIX_CHARS,arg[0]) == NULL)
	
	if (!input_flag) {
	   /* we haven't seen an input arg yet; so, this is it */
	   input_flag = TRUE;
	   strcpy(input_option,arg);
	   if (strrchr (input_option, '.') == NULL)
	     strcat (input_option, CLI_INPUT_FILE_EXT);
	}
	else {
	   /* arg w/o prefix char masquerading as input arg */
	   rul__msg_print (CMP_CLIDUPINP, arg);
	   return FALSE;
	}
      else {
	 /* argument begins with prefix character */
	 arg_size = strlen(arg+1);
	 /*find value delim*/
	 option_name_size = strcspn (arg+1, CLI_VALUE_DELIMS);
	 if (option_name_size < arg_size) {
	    option_value = arg + option_name_size + 2;
	    option_value_size = arg_size - option_name_size - 1;
	 }
	 else {
	    option_name_size = arg_size;
	    option_value = NULL;
	    option_value_size = 0;
	 }
	 /* make copy of the opt name part of the arg (skipping prefix char) */
	 strncpy(option_name,arg+1,option_name_size);
	 if (debug_flag =
	     cli_option_present(option_name,option_name_size,
				"debug",sizeof("debug") - 1))
	   debug_option = cli_debug_option(option_value,option_value_size);
	 else if (quiet_flag =
		  cli_option_present(option_name,option_name_size,
				     "quiet",sizeof("quiet") - 1))
	   {
	      quiet_flag = TRUE;
	      quiet_option = TRUE;
	      if (option_value_size != 0)
		rul__msg_print (CMP_CLIUNEXOPT, option_value);
	   }
	 else if (listing_flag = cli_option_present (option_name,
						     option_name_size,
						     "listing",
						     sizeof("listing") - 1))
	   strncpy (listing_option, option_value, option_value_size);
	 else if (error_flag = cli_option_present (option_name,
						   option_name_size,
						   "error",
						   sizeof("error") - 1))
	   strncpy (error_option, option_value, option_value_size);
	 else if (usedir_flag = cli_option_present (option_name,
						    option_name_size,
						    "usedirectory",
						    sizeof("usedirectory")-1))
	   strncpy (usedir_option, option_value, option_value_size);
	 else if (cli_option_present (option_name,
				      option_name_size,
				      "names",
				      sizeof("names") - 1))
	   names_option = cli_names_option (option_value, option_value_size);
	 else if (cli_option_present (option_name,
				      option_name_size,
				      "optimize",
				      sizeof("optimize") - 1))
	   optimize_option = cli_optimize_option (option_value,
						  option_value_size);
	 else if (output_flag = cli_option_present (option_name,
						    option_name_size,
						    "output",
						    sizeof("output") - 1))
	   strncpy (output_option, option_value, option_value_size);
	 else {
	    /* no option names matched! */
	    rul__msg_print (CMP_CLIBADNAM, arg);
	    return FALSE;
	 } /* no option names matched! */
      } /* arg began with prefix char */
   } /* no more args to process */
   
   if (!input_flag) {
      rul__msg_print (CMP_NOSOURCEFILE);
      status = FALSE;
   }
   else {
      status = TRUE;
      if (listing_flag && (listing_option[0] == '\0'))
	cli_create_default_file (listing_option,
				 input_option,
				 CLI_LISTING_FILE_EXT);
      if (error_flag && (error_option[0] == '\0'))
	cli_create_default_file (error_option,
				 input_option,
				 CLI_ERROR_FILE_EXT);
      if (output_option[0] == '\0')
	cli_create_default_file (output_option,
				 input_option,
				 CLI_OUTPUT_FILE_EXT);
   }
   return status;
}

#endif /* not __VMS */

