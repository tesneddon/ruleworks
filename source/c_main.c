/****************************************************************************
**                                                                         **
**                        C M P _ M A I N . C                              **
**                                    
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
**                                                                         **
****************************************************************************/


/*
 * FACILITY:
 *	RULEWORKS compiler
 *
 * ABSTRACT:
 *	This is the "main" module for the compiler.  It will need to
 *	be made much more sophisticated at some point, but later.
 *
 * MODIFIED BY:
 *	DEC	Digital Equipment Corporation
 *	CPQ	Compaq Computer Corporation
 *
 * MODIFICATION HISTORY:
 *
 *	16-Dec-1992	DEC	Initial version
 *	22-Apr-1994	DEC	Return a value from main() to please compilers
 *	01-Sep-1997	DEC	Modify to allow VER_EXPIRE_YEAR to be expressed
 *			        as yyyy and not yy.
 *					- Check VER_EXPIRE_YEAR > 1970 not > 93 & < 110
 *			        - Subtract 1900 from VER_EXPIRE_YEAR 
 *	01-Sep-1997	DEC	Move the call to licence checking 
 *					(cmp_identify_version) before opening files
 *					(set_up_file_streams).
 *	23-Feb-1998	DEC	EXIT on version expiry added. 
 *	01-Dec-1999	CPQ	Release with GPL
*/

#include <common.h>
#include <time.h>
#include <cmp_comm.h>
#include <mol.h>
#include <ast.h>
#include <ios.h>
#include <sem.h>
#include <gen.h>
#include <msg.h>
#include <cmp_msg.h>
#include <emit.h>
#include <conrg.h>
#include <cons.h>
#include <cli.h>
#include <ver_msg.h>

typedef struct tm *Date_Time;

/*
 * External function declarations
 */
int rul__cmp_yyparse();

/*
 * Forward function declarations
 */
static IO_Stream set_up_file_streams (void);
static void cmp_identify_version (void);

#ifndef NDEBUG
static void print_syntax_tree (void);
#endif

/*
 * Static variables
 */
static Ast_Node 	SA_current_ast_root = NULL;


#ifdef __BORLANDC__
	extern unsigned _stklen = 20000U;
#endif




/***********
**        **
**  MAIN  **
**        **
***********/

int
main (int argc, char **argv)
	/*?*
	**	This is a very simplified compiler top level.
	**	It will eventually need to be replaced with
	**	something more sophisticated.
	*/
{
	IO_Stream src_ios;
	Boolean success;
	int parse_failed;
	char *delete_logical, *construct;
	long errs;

	extern void  rul__decl_destroy_decl_blocks (void);
	extern void  rul__parser_done_with_construct (void);
	extern char *rul__parser_get_construct_str (void);

	rul__ios_init ();

	if (argc == 1) {
	    rul__ios_printf (RUL__C_STD_OUT, CMP_VERSION);
	    rul__ios_printf (RUL__C_STD_OUT, CMP_COPYRIGHT);
#if CMP_INTERNAL
	    rul__ios_printf (RUL__C_STD_OUT, CMP_INTERNAL_ONLY);
#endif
	    rul__ios_printf (RUL__C_STD_OUT, CMP_HELP_MESSAGE_1);
	    exit (EXIT_SUCCESS);
	}
	else if (argc == 2 && strlen (argv[1]) == 2) {
	  if (!strcmp (argv[1], "-h") ||
	      !strcmp (argv[1], "-H") ||
	      !strcmp (argv[1], "-?") ||
	      !strcmp (argv[1], "/h") ||
	      !strcmp (argv[1], "/H") ||
	      !strcmp (argv[1], "/?")) {
	    rul__ios_printf (RUL__C_STD_OUT, CMP_VERSION);
	    rul__ios_printf (RUL__C_STD_OUT, CMP_COPYRIGHT);
#if CMP_INTERNAL
	    rul__ios_printf (RUL__C_STD_OUT, CMP_INTERNAL_ONLY);
#endif
	    rul__ios_printf (RUL__C_STD_OUT, CMP_HELP_MESSAGE_1);
	    rul__ios_printf (RUL__C_STD_OUT, CMP_HELP_MESSAGE_2);
	    exit (EXIT_SUCCESS);
	  }
	}

	/* Deal with command line arguments  */
	if (! rul__cli_parse(argc, argv)) {
	    exit (EXIT_FAILURE);
	}

	cmp_identify_version ();

	src_ios = set_up_file_streams ();

	rul__emit_prologue();

	while (! rul__ios_stream_at_eof (src_ios)) {

	    SA_current_ast_root = NULL;
	    parse_failed = rul__cmp_yyparse ();

	    if (! parse_failed   &&   SA_current_ast_root != NULL) {

#ifndef NDEBUG
		print_syntax_tree ();
#endif
	        success = rul__sem_check_construct (SA_current_ast_root);
		if (success) {
		    rul__gen_constructs_code (SA_current_ast_root);
		} else {
		    rul__msg_print (CMP_NOCODEGEN);
		}
		rul__ast_free (SA_current_ast_root);
	    }
	    if (parse_failed) {
		construct = rul__parser_get_construct_str ();
		if (construct != NULL  &&  construct[0] != '\0') {
		    rul__msg_print (CMP_INVCONIGN);
		}
	    }
	    rul__parser_done_with_construct ();
	    /*?*  memory leak of ast's whenever parse_failed  */ 
	}

	rul__sem_prev_block_closed (NULL);

	errs = rul__msg_get_severe_errors ();
	errs /= 2; /* the msg_get_message_format is called twice per */
	if (errs)
	  rul__msg_print (CMP_NOOUTPUT, errs);

	rul__ios_close_all_files ();
	rul__decl_destroy_decl_blocks ();
	rul__conrg_clear_all_blocks ();
	rul__cons_clear ();
	rul__class_clear ();
	rul__sem_clear_construct_name ();
#ifndef NDEBUG
/*	rul__mol_print_remaining (); */
#endif

	if (errs) {
#ifndef NDEBUG
	  delete_logical = getenv ("TIN_DEBUG_NODELETE");
	  if (!delete_logical || delete_logical[0] == '\0') {
#endif
	    rul__gen_cleanup_usefiles (TRUE);
	    remove (rul__cli_output_option ());
#ifndef NDEBUG
	  }
#endif
	  exit (EXIT_FAILURE);
	}
	
	rul__gen_cleanup_usefiles (FALSE);
	return EXIT_SUCCESS;	/* Same effect as exit(EXIT_SUCCESS) */
}





/**********************************
**                               **
**  RUL__MAIN_SET_CUR_CONSTRUCT  **
**                               **
**********************************/

void rul__main_set_cur_construct (Ast_Node top_level_node)
{
	SA_current_ast_root = top_level_node;
}




/**************************
**                       **
**  SET_UP_FILE_STREAMS  **
**                       **
**************************/

static IO_Stream set_up_file_streams ()
{
	IO_Stream src_ios, obj_ios, err_ios;
	Mol_Symbol src_file, obj_file, err_file;

	src_file = rul__mol_make_symbol (rul__cli_input_option());
	src_ios = rul__ios_open_file (
			src_file,			/* stream name */
			src_file, 			/* file name */
			IOS__E_IN);
	if (src_ios == NULL) exit (EXIT_FAILURE);
	rul__ios_set_source_stream (src_ios);

	/*  Now open the object file stream  */
	obj_file = rul__mol_make_symbol (rul__cli_output_option());
	obj_ios = rul__ios_open_file (
			obj_file,			/* stream name */
			obj_file, 			/* file name */
			IOS__E_OUT);
	if (obj_ios == NULL) exit (EXIT_FAILURE);
	rul__ios_set_object_stream (obj_ios);

	/*  Now open the error file stream  */
	if (rul__cli_error_flag ()) {
	  err_file = rul__mol_make_symbol (rul__cli_error_option());
	  err_ios = rul__ios_open_file (
			err_file,			/* stream name */
			err_file,			/* file name */
			IOS__E_OUT);
	  if (err_ios == NULL) exit (EXIT_FAILURE);
	  rul__mol_decr_uses (err_file);
	  rul__ios_set_stderr_stream (err_ios);
	}

	rul__mol_decr_uses (src_file);
	rul__mol_decr_uses (obj_file);
	return (src_ios);
}




/***************************
**                        **
**  CMP_IDENTIFY_VERSION  **
**                        **
***************************/

static void cmp_identify_version (void)
{
	time_t expire_time, cur_time, ret_time;
	Date_Time expire_date;
	double delta_time;

#if VER_EXPIRES
	/*  First check if this version has expired  */

	ret_time = time (&cur_time);

	assert (VER_EXPIRE_MONTH > 0  &&  VER_EXPIRE_MONTH < 13);
	assert (VER_EXPIRE_DAY > 0 && VER_EXPIRE_DAY < 32);
	assert (VER_EXPIRE_YEAR > 1970);

	expire_date = (Date_Time) rul__mem_calloc (1, sizeof(struct tm));
	expire_date->tm_mon  = VER_EXPIRE_MONTH - 1; 
				/*  Month number - 1 (e.g. september = 8) */
	expire_date->tm_mday = VER_EXPIRE_DAY;
	expire_date->tm_year = (VER_EXPIRE_YEAR - 1900);
			/* Year number - 1900 (e.g. 1970 = 70, 2000 = 100) */
	expire_time = mktime (expire_date);
	delta_time = difftime (expire_time, cur_time);
	rul__mem_free (expire_date);

	if (delta_time < 0.0) { 
	  rul__ios_printf (RUL__C_STD_ERR, CMP_VERSION); 
	  rul__msg_print (VER_LICEXPIRED); 
	  exit (EXIT_FAILURE);
	}
#endif

	/*  Then print the ID messages, unless /quiet was specified  */

	if (! rul__cli_quiet_option()) {
	    rul__ios_printf (RUL__C_STD_OUT, CMP_VERSION);
	    rul__ios_printf (RUL__C_STD_OUT, CMP_COPYRIGHT);
#if CMP_INTERNAL
	    rul__ios_printf (RUL__C_STD_OUT, CMP_INTERNAL_ONLY);
#endif
	}
}

#ifndef NDEBUG



/************************
**                     **
**  PRINT_SYNTAX_TREE  **
**                     **
************************/

static void print_syntax_tree (void)
{
	char *env_val;
	extern void rul___ast_print_node (Ast_Node node, long depth);

	env_val = getenv ("TIN_DEBUG_AST");
	if (env_val  &&  env_val[0] != '\0') {

	    if (strcmp(env_val,"min") == 0 || strcmp(env_val,"MIN") == 0) {

		rul___ast_print_node (SA_current_ast_root, 1);

	    } else {
#ifdef __VMS
	        rul__ast_print (&SA_current_ast_root);
#else
	        rul__ast_print (SA_current_ast_root);
#endif
	    }
	}
}

#endif /* ifndef NDEBUG */
