/****************************************************************************
**                                                                         **
**                     O P S _ I O S _ F I L E. C                          **
**                                                                         **
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
****************************************************************************/


/*
**  FACILITY:
**	RULEWORKS run time system and compiler
**
**  ABSTRACT:
**	This file contains the 
**	I/O Stream (IOS) subsystem, to be used elsewhere.
**
**
**  MODIFIED BY:
**	DEC	Digital Equipment Corporation
 *	CPQ	Compaq Computer Corporation
**
**  MODIFICATION HISTORY:
**
**	 1-Dec-1992	DEC	Initial version
**	 9-Apr-1994	DEC	Add stdin/out/err Windows window
**	01-DEc-1999	CPQ	Release with gPL
*/

#include <common.h>
#include <stdarg.h>
#include <mol.h>
#include <msg.h>
#include <ios_msg.h>
#include <rts_msg.h>
#include <ios.h>
#include <ios_p.h>
#include <atom.h>

#define NOT_FOUND -1
#define MAX_INPUT_LINE	5000

static	IO_Stream	ios_SA__std_in = NULL;
static 	IO_Stream	ios_SA__std_out = NULL;
static	IO_Stream	ios_SA__std_err = NULL;

/*  CMP streams  */
static	IO_Stream	ios_SA__listing = NULL;
static	IO_Stream	ios_SA__source = NULL;
static	IO_Stream	ios_SA__object = NULL;
static	IO_Stream	ios_SA__atom = NULL;


static	IO_Stream      *ios_SA__known_ios_vector = NULL;
static	long		ios_SL__vector_length = 0;
static	long		ios_SL__known_count = 0;

static struct io_stream	ios_SS__unreal;
#define NOT_A_REAL_STREAM ((IO_Stream) &ios_SS__unreal)

/* static molecules */
static  Molecule        ios_SM__eof;
static  Molecule        ios_SM__nil;
static  Molecule        ios_SM__comp_nil;
static  Molecule        ios_SM__accept;
static  Molecule        ios_SM__trace;
static  Molecule        ios_SM__write;
static  Molecule        ios_SM__in;
static  Molecule        ios_SM__out;
static  Molecule	ios_SM__err;
static  Molecule        ios_SM__append;
static  Molecule        ios_SM__crlf;
static  Molecule        ios_SM__tabto;
static  Molecule        ios_SM__rjust;

static	Boolean  ios_validate_accept_stream (Mol_Symbol stream_name,
					     Boolean accept_line_p);

static void ios_set_def_in (IO_Stream ios, Mol_Symbol stream_name);
static IO_Stream ios_simple_get_def_in (void);
static void ios_close_def_in (void);

static void ios_set_def_out (IO_Stream ios, Mol_Symbol stream_name);
static IO_Stream ios_simple_get_def_out (void);
static void ios_close_def_out (void);

static void ios_set_def_trace (IO_Stream ios, Mol_Symbol stream_name);
static void ios_close_def_trace (void);



/*******************
**                **
**  IOS_IS_VALID  **
**                **
*******************/

static Boolean ios_is_valid (IO_Stream ios)
{
	return (ios == NOT_A_REAL_STREAM  ||
		 ( ios != NULL    &&
		   ( ios->type == IOS__E_IN_FILE  	||
		     ios->type == IOS__E_OUT_FILE	||
		     ios->type == IOS__E_IN_BIZZARE	||  
		     ios->type == IOS__E_OUT_BIZZARE)));
}



/*********************************
**                              **
**  POSITION_OF_NAME_IN_VECTOR  **
**                              **
*********************************/

static long position_of_name_in_vector (Mol_Symbol stream_name)
{
	long i = 0;
	while (i < ios_SL__known_count) {
	    if (stream_name == ios_SA__known_ios_vector[i]->name) {
		return (i);
	    }
	    i++;
	}
	return (NOT_FOUND);
}



/**********************
**                   **
**  STORE_IN_VECTOR  **
**                   **
**********************/

static void store_in_vector (IO_Stream ios)
{
	if (ios_SA__known_ios_vector == NULL) {

	    /*  Never been allocated  */
	    ios_SL__vector_length = 100;
	    ios_SA__known_ios_vector =
		(IO_Stream *) rul__mem_malloc (
				ios_SL__vector_length * sizeof (IO_Stream));

	} else if (ios_SL__vector_length == ios_SL__known_count) {

	    /*  Alocation wasn't big enough */
	    ios_SL__vector_length *= 3;
	    ios_SA__known_ios_vector = 
		(IO_Stream *) rul__mem_realloc (ios_SA__known_ios_vector,
				ios_SL__vector_length * sizeof (IO_Stream));
	}

	ios_SA__known_ios_vector[ios_SL__known_count] = ios;
	ios_SL__known_count++;
}



/*************************
**                      **
**  REMOVE_FROM_VECTOR  **
**                      **
*************************/

static void remove_from_vector (long index_in_vector)
{
	long i;

	assert (index_in_vector < ios_SL__known_count);
	if (index_in_vector == NOT_FOUND) return;

	i = index_in_vector;
	while (i + 1 < ios_SL__known_count) {
	    ios_SA__known_ios_vector[i] = ios_SA__known_ios_vector[i+1];
	    i++;
	}
	ios_SL__known_count--;
}



/***********************
**                    **
**  MAKE_FILE_STREAM  **
**                    **
***********************/

static	IO_Stream  make_file_stream (
			Mol_Symbol stream_name,
			Mol_Symbol file_name,
			FILE *fptr,
			Boolean file_opened,
			IO_Direction dir)
{
	struct file_io_stream *ios;
	struct input_file_io_stream *in_ios;

	if (dir == IOS__E_OUTPUT_ONLY) {
	    ios = (File_IO_Stream) 
		    rul__mem_malloc (sizeof (struct file_io_stream));
	    ios->type = IOS__E_OUT_FILE;

	} else {
	    in_ios = (Input_File_IO_Stream) 
		    rul__mem_malloc (sizeof (struct input_file_io_stream));
	    ios = (File_IO_Stream) in_ios;
	    ios->type = IOS__E_IN_FILE;
	    in_ios->lex_buf =   NULL;

	    /*  Set the input_file_io_stream specific fields  */
	    in_ios->at_eof = FALSE;
	    in_ios->line_number = 0;
	}

	ios->name =      stream_name;
	ios->file_ptr =  fptr;
	ios->file_name = file_name;
	ios->file_opened = file_opened;
	ios->columns = 80;
	ios->cur_col = 0;

	if (ios->name) rul__mol_incr_uses (ios->name);
	if (ios->file_name) rul__mol_incr_uses (ios->file_name);

	store_in_vector ((IO_Stream) ios);

	return ((IO_Stream) ios);
}



/********************
**                 **
**  RUL__IOS_INIT  **
**                 **
********************/

void  rul__ios_init (void)
{
	Mol_Symbol std_in_file_name;
	Mol_Symbol std_out_file_name;
	Mol_Symbol std_err_file_name;
	Mol_Symbol nil;

	/*  Make IO_Streams for all the standard files  */

	static Boolean ios_inited = FALSE;

	rul__mol_init (); /* Just in case.  This subsystem assumes this */

	if (!ios_inited) {
	    ios_inited = TRUE;

	    ios_SM__eof = rul__mol_make_symbol ("END-OF-FILE");
	    rul__mol_mark_perm (ios_SM__eof);
	    ios_SM__nil = rul__mol_symbol_nil ();
	    rul__mol_mark_perm (ios_SM__nil);
	    ios_SM__comp_nil = rul__mol_make_comp (0);
	    rul__mol_mark_perm (ios_SM__comp_nil);
	    ios_SM__accept = rul__mol_make_symbol ("ACCEPT");
	    rul__mol_mark_perm (ios_SM__accept);
	    ios_SM__trace = rul__mol_make_symbol ("TRACE");
	    rul__mol_mark_perm (ios_SM__trace);
	    ios_SM__write = rul__mol_make_symbol ("WRITE");
	    rul__mol_mark_perm (ios_SM__write);
	    ios_SM__in = rul__mol_make_symbol ("IN");
	    rul__mol_mark_perm (ios_SM__in);
	    ios_SM__out = rul__mol_make_symbol ("OUT");
	    rul__mol_mark_perm (ios_SM__out);
	    ios_SM__err = rul__mol_make_symbol ("ERR");
	    rul__mol_mark_perm (ios_SM__err);
	    ios_SM__append = rul__mol_make_symbol ("APPEND");
	    rul__mol_mark_perm (ios_SM__append);
	    ios_SM__crlf = rul__mol_make_symbol ("$CRLF");
	    rul__mol_mark_perm (ios_SM__crlf);
	    ios_SM__tabto = rul__mol_make_symbol ("$TABTO");
	    rul__mol_mark_perm (ios_SM__tabto);
	    ios_SM__rjust = rul__mol_make_symbol ("$RJUST");
	    rul__mol_mark_perm (ios_SM__rjust);

	    if (ios_SA__std_in == NULL) {
	    	std_in_file_name = rul__mol_make_symbol ("Standard-Input");
		rul__mol_mark_perm (std_in_file_name);
		ios_SA__std_in = 
		        make_file_stream (
					  ios_SM__in,
					  std_in_file_name,
					  stdin,
					  FILE_NOT_OPENED,
					  IOS__E_INPUT_ONLY);
	    }

	    if (ios_SA__std_out == NULL) {
	        std_out_file_name = rul__mol_make_symbol ("Standard-Output");
		rul__mol_mark_perm (std_out_file_name);
 	    	ios_SA__std_out =
			make_file_stream (
					  ios_SM__out,
					  std_out_file_name,
					  stdout,
					  FILE_NOT_OPENED,
					  IOS__E_OUTPUT_ONLY);
	    } 

	    if (ios_SA__std_err == NULL) {
	        std_err_file_name = rul__mol_make_symbol ("Standard-Error");
		rul__mol_mark_perm (std_err_file_name);
	    	ios_SA__std_err =
			make_file_stream (
					  ios_SM__err,
					  std_err_file_name,
					  stderr,
					  FILE_NOT_OPENED,
					  IOS__E_OUTPUT_ONLY);
	    }

	    rul__ios_win_init(); /* If on Windows, set up bizzare streams */

	    /*
	    **  Create the unreal stream, and set the
	    **  default listing and source streams to it.
	    */
	    ios_SS__unreal.name = ios_SM__nil;
	    ios_SS__unreal.type = IOS__E_UNREAL;

	    if (ios_SA__listing == NULL) {
	        ios_SA__listing = NOT_A_REAL_STREAM;
	    }
	    if (ios_SA__source == NULL) {
	        ios_SA__source = NOT_A_REAL_STREAM;
	    }
	    if (ios_SA__object == NULL) {
	        ios_SA__object = NOT_A_REAL_STREAM;
	    }
	    if (ios_SA__atom == NULL) {
	        ios_SA__atom = NOT_A_REAL_STREAM;
	    }

	    /*  Set the defaults for programmer outputs  */
	    ios_set_def_in (NULL, ios_SM__nil);
	    ios_set_def_trace (NULL, ios_SM__nil);
	    ios_set_def_out (NULL, ios_SM__nil);
	}
}




/****************************
**                         **
**  RUL__IOS_OPEN_FILE_RT  **
**                         **
****************************/

IO_Stream  rul__ios_open_file_rt (Mol_Symbol stream_name, 
				  Mol_Symbol file_name,
				  Mol_Symbol acc_mode)
{
  IO_Access_Mode iam;

  if (acc_mode == ios_SM__in)
    iam = IOS__E_IN;
  else if (acc_mode == ios_SM__out)
    iam = IOS__E_OUT;
  else if (acc_mode == ios_SM__append)
    iam = IOS__E_APPEND;
  else {
    rul__msg_print_w_atoms (IOS_INVOPNMOD, 1, acc_mode);
    return NULL;
  }
  return rul__ios_open_file (stream_name, file_name, iam);
}




/*************************
**                      **
**  RUL__IOS_OPEN_FILE  **
**                      **
*************************/

IO_Stream  rul__ios_open_file ( Mol_Symbol stream_name, 
				Mol_Symbol file_name,
				IO_Access_Mode acc_mode)
{
	FILE *fptr;
	char buffer [RUL_C_MAX_SYMBOL_SIZE+1];
	char *access_str = NULL, *access_desc = NULL;
	IO_Direction dir;
	IO_Stream ios;

	assert ((stream_name == NULL  || rul__mol_is_valid (stream_name))  &&
		rul__mol_is_valid (file_name));

	if (stream_name && rul__ios_get_named_stream (stream_name)) {
	    rul__msg_print_w_atoms (IOS_FILEOPEN, 1, stream_name);
	    return (NULL);
	}

	rul__mol_use_printform (file_name, buffer, RUL_C_MAX_SYMBOL_SIZE+1);

	if (acc_mode == IOS__E_IN) {
	    access_str = "r";
	    access_desc = "reading";
	    dir = IOS__E_INPUT_ONLY;
	} else if (acc_mode == IOS__E_OUT) {
	    access_str = "w";
	    access_desc = "writing";
	    dir = IOS__E_OUTPUT_ONLY;
	} else if (acc_mode == IOS__E_APPEND) {
	    access_str = "a";
	    access_desc = "appending";
	    dir = IOS__E_OUTPUT_ONLY;
	} else {
#ifndef NDEBUG
	    rul__ios_printf (RUL__C_STD_ERR,
              "\n Internal Error:  Invalid acc_mode, %d, in rul__ios_open_file",
	      acc_mode);
#endif
	    return (NULL);
	}

	fptr = fopen (buffer, access_str);

	if (fptr) {
	    ios = make_file_stream (stream_name, file_name, fptr, FILE_OPENED,
				    dir);
	} else {
    	    /*  Error:  unable to open file  */
	    rul__msg_print (IOS_CANTOPEN, buffer, access_desc);
	    ios = NULL;
	}

	return (ios);
}




/*************************
**                      **
**  RUL__IOS_IS_OPEN    **
**                      **
*************************/

Mol_Symbol  rul__ios_is_open (Mol_Symbol stream_name)
{
	IO_Stream ios;
	assert ((stream_name == NULL  || rul__mol_is_valid (stream_name)));

	if (stream_name) {
	    ios = rul__ios_get_named_stream (stream_name);
	    if (ios && ios_is_valid (ios) && is_file_stream (ios)) {
	        if (is_input_file_stream (ios))
		    return ios_SM__in;
		else
		    return ios_SM__out;
	    }
	}

	return ios_SM__nil;
}




/***************************
**                        **
**  RUL__IOS_CLOSE_FILES  **
**                        **
***************************/

void  rul__ios_close_files (long mol_count, ...)
{
  va_list ap;
  long arg_index = 1;

  if (mol_count == 0) return;
  
  va_start (ap, mol_count);

  while (arg_index <= mol_count) {
    rul__ios_close_file (va_arg (ap, Mol_Symbol));
    arg_index++;
  }
  va_end (ap);
}

/*
**  RUL___IOS_CLOSE_FILE
*/
static void rul___ios_close_file (long i)
{
  struct file_io_stream *ios;
  struct input_file_io_stream *in_ios;
 
  ios = (struct file_io_stream *) ios_SA__known_ios_vector[i] ;
  if ((IO_Stream)ios == rul___ios_get_atom ()) {
    rul__ios_set_atom_stream (rul___ios_get_def_in ());
  }
  rul__ios_flush ((IO_Stream) ios);
  if (ios->type == IOS__E_IN_BIZZARE)
    rul__ios_close_bizzare (ios->name);
  else {
    if (ios->file_opened)
      fclose (ios->file_ptr);
    if (ios->name != NULL)
      rul__mol_decr_uses (ios->name);
    if (ios->file_name != NULL)
      rul__mol_decr_uses (ios->file_name);
    remove_from_vector (i);
    if (is_input_file_stream (ios)) {
      in_ios = (Input_File_IO_Stream) ios;
      if (in_ios->lex_buf)
	rul__atom_delete_lex_buf (in_ios->lex_buf);
    }
    rul__mem_free (ios);
  }
}



/**************************
**                       **
**  RUL__IOS_CLOSE_FILE  **
**                       **
**************************/

void  rul__ios_close_file (Mol_Symbol stream_name)
{
	long i;
	IO_Stream ios;

	i = position_of_name_in_vector (stream_name);

	if (i != NOT_FOUND) {
	    ios = ios_SA__known_ios_vector[i];

	    /* this will operate on the stream at slot i and then remove it */
	    rul___ios_close_file (i);

	    if (ios == ios_simple_get_def_in())
		ios_close_def_in ();
	    else if (ios == rul___ios_get_def_trace())
		ios_close_def_trace ();
	    else if (ios == ios_simple_get_def_out())
		ios_close_def_out ();

	} else {
	    /*  Error:  no such named stream is open  */
	    rul__msg_print_w_atoms (IOS_NOSUCHFILE, 1, stream_name);
	}
}




/**********************
**                   **
**  RUL__IOS_PRINTF  **
**                   **
**********************/

void  rul__ios_printf (IO_Stream ios, char *format_string, ...)
{
	va_list ap;
	File_IO_Stream fs;
	Bizzare_IO_Stream bs;
	static char vsp_buffer[MAX_INPUT_LINE];

	/* vsnprintf would be better, would not need a 5000 byte buffer */
	/* _vsnprintf is not available for all compilers */
	
	assert (ios_is_valid (ios));

	if (ios == NOT_A_REAL_STREAM) return;

	if (is_input_stream (ios)) {
	    /*  Error:  no output allowed on this stream */
	    if (ios->name) {
		rul__msg_print_w_atoms (IOS_BADOUTFILE, 1, ios->name);
	    } else {
		assert(FALSE /* internal error; attempt to write to stdin */);
	    }
	    return;
	}

	if (ios->type == IOS__E_OUT_FILE) {
	    fs = (File_IO_Stream) ios;
	    if (strchr (format_string, (int) '%') == NULL) {
		/*  the format string included no arguments  */
      /* fprintf (fs->file_ptr, format_string); put faster than print??? */
	        fputs (format_string, fs->file_ptr);
	        if (format_string[strlen(format_string)] == '\n') {
		    rul__ios_flush (ios);
	        }
	    } else {
		va_start (ap, format_string);
		vsprintf (vsp_buffer, format_string, ap);
		va_end (ap);
	        fputs (vsp_buffer, fs->file_ptr);
		if (vsp_buffer[strlen(vsp_buffer)] == '\n') {
		    rul__ios_flush (ios);
		}
	    }

	} else {
	    bs = (Bizzare_IO_Stream) ios;
	    if (bs->put_func_ptr != NULL) {

		va_start (ap, format_string);
		vsprintf (vsp_buffer, format_string, ap);
		rul__ios_ensure_window_open(ios);
		(bs->put_func_ptr)(vsp_buffer,
				   bs->put_func_arg0, bs->put_func_arg1);
		va_end (ap);
	    }
	}
}



/*********************
**                  **
**  RUL__IOS_FGETS  **
**                  **
*********************/

char  *rul__ios_fgets (IO_Stream ios, char *buffer, long max_chars)
{
	Input_File_IO_Stream in_ios;
	Bizzare_IO_Stream bs;

	assert (ios_is_valid (ios));

	if (is_output_stream (ios)) {
	    /*  Error:  no input allowed on this stream */
	    if (ios->name) {
		rul__msg_print_w_atoms (IOS_BADINFILE, 1, ios->name);
	    } else {
		assert(FALSE /* internal error; attempt to read from stdout */);
	    }

	    return (NULL);
	}

	if (ios->type == IOS__E_IN_FILE) {
	    char *tmp;

	    in_ios = (Input_File_IO_Stream) ios;
	    tmp = fgets (buffer, max_chars, in_ios->file_ptr);

	    if (tmp == buffer) {
		if (in_ios->line_number == IOS__C_NOT_A_LINE_NUM) {
		    in_ios->line_number = 1;
		} else {
		    in_ios->line_number++;
		}
		return (buffer);
	    } else {
		in_ios->at_eof = TRUE;
		return (NULL);
	    }

	} else {
	    bs = (Bizzare_IO_Stream) ios;
	    if (bs->get_func_ptr != NULL) {
		rul__ios_ensure_window_open(ios);
	        return ((bs->get_func_ptr)(buffer, max_chars,
					   bs->get_func_arg0,
					   bs->get_func_arg1));
	    }
	}
	return (NULL);
}



/*****************************
**                          **
**  RUL__IOS_STREAM_AT_EOF  **
**                          **
*****************************/

Boolean rul__ios_stream_at_eof (IO_Stream ios)
{
	Input_File_IO_Stream in_ios;

	assert (ios_is_valid (ios));

	if (is_input_file_stream (ios)) {
	    in_ios = (Input_File_IO_Stream) ios;
	    return (in_ios->at_eof);
	}

	assert(FALSE); /* internal error: EOF test on an output file */
	return (FALSE);
}




/********************************
**                             **
**  RUL__IOS_GET_NAMED_STREAM  **
**                             **
********************************/

IO_Stream  rul__ios_get_named_stream (Mol_Symbol stream_name)
{
	struct io_stream *ios;
	long i;

	i = position_of_name_in_vector (stream_name);

	if (i != NOT_FOUND) {
	    ios = ios_SA__known_ios_vector[i] ;
	} else {
	    ios = NULL;
	}
	return (ios);
}



/*********************
**                  **
**  RUL__IOS_WRITE  **
**                  **
*********************/

void  rul__ios_write (long mol_count, ...)
{
  va_list ap;
  IO_Stream ios = NULL;
  Molecule mol, format_mol;
  long arg_index = 1, i, j;
  struct file_io_stream *fs;
  Boolean need_blank = TRUE;
  char buf1[RUL_C_MAX_SYMBOL_SIZE+1];
  char buf2[RUL_C_MAX_SYMBOL_SIZE*2+1];

  if (mol_count == 0)
    return;

  va_start (ap, mol_count);
  mol = va_arg (ap, Molecule);

  ios = rul__ios_get_named_stream (mol);

  if (ios == NULL) {
    ios = rul___ios_get_def_out();
    if (ios == NULL) {
      va_end (ap);
      return;
    }
    arg_index = 0;
  }

  if (is_input_stream (ios)) {
    /*  Error:  no output allowed on this stream */
    if (ios->name) {
      rul__msg_print_w_atoms (IOS_BADOUTFILE, 1, ios->name);
    }
    va_end (ap);
    return;
  }

  while (arg_index < mol_count) {

    if (arg_index)
      mol = va_arg (ap, Molecule);

    arg_index++;

    format_mol = NULL;

    if ((rul__mol_is_compound (mol)) &&
	(rul__mol_get_poly_count (mol) > 0))
      format_mol = rul__mol_get_comp_nth (mol, 1);
    
    if (format_mol == ios_SM__crlf) {
      /* special code for (CRLF) */
      
      rul__ios_print_newline (ios);
      need_blank = FALSE;
    }

    else if (format_mol == ios_SM__tabto) {
      /* special code for (TABTO n) */
	
      format_mol = rul__mol_get_comp_nth (mol, 2);

      if (! rul__mol_is_int_atom (format_mol)) {
	rul__msg_print_w_atoms (RTS_NOITABVAL, 1, format_mol);
      }
      else {

	i = rul__mol_int_atom_value (format_mol) - 1;
	if (i < 0) {
	  rul__msg_print (RTS_BADTABVAL, i);
	}
	else {
	
	  if (i < ios->cur_col)
	    rul__ios_print_newline (ios);
	
	  i -= ios->cur_col;
/*	  while ((ios->cur_col + i + 1) > ios->columns) {
	    rul__ios_print_newline (ios);
	    i -= ios->columns;
	  }
*/	
	  if (i) {
	    rul__ios_printf (ios, "%*s", i, " ");
	    ios->cur_col += i;
	  }
	  need_blank = FALSE;
	}
      }
    }
	
    else if (format_mol == ios_SM__rjust) {
      /* special code for (RJUST n) */
	
      format_mol = rul__mol_get_comp_nth (mol, 2);
	
      if (! rul__mol_is_int_atom (format_mol)) {
	rul__msg_print_w_atoms (RTS_NOIRJUVAL, 1, format_mol);
      }
      else {

	i = rul__mol_int_atom_value (format_mol);
	if (i < 1 || i > RUL_C_MAX_SYMBOL_SIZE) {
	  rul__msg_print (RTS_BADRJUVAL, i, RUL_C_MAX_SYMBOL_SIZE);
	}
	else {
	
	  if (arg_index == mol_count)
	    mol = ios_SM__nil;
	  else {
	    mol = va_arg (ap, Molecule);
	    arg_index++;
	  }
	
	  rul__mol_use_printform (mol, buf1, RUL_C_MAX_SYMBOL_SIZE);
	  sprintf (buf2, "%*s", i, buf1);
	  rul__ios_printf (ios, "%s", buf2);
	  ios->cur_col += strlen (buf2);
	  need_blank = TRUE;
	}
      }
    }
    else {
      
      if (need_blank) {
	rul__ios_printf (ios, " ");
	ios->cur_col += 1;
      }
	
      rul__mol_print_printform (mol, ios);
/* c-vms-axp optimizer bug...
      ios->cur_col += rul__mol_get_printform_length (mol);
*/
      j = rul__mol_get_printform_length (mol) + ios->cur_col;
      ios->cur_col = j;
/* */
      need_blank = TRUE;
    }
  }

  va_end (ap);
#ifndef __VMS
  rul__ios_flush (ios);
#endif
}



/*****************************
**                          **
**  RUL__IOS_CURR_LINE_NUM  **
**                          **
*****************************/

long rul__ios_curr_line_num (IO_Stream ios)
{
	struct input_file_io_stream *in_ios;

	assert (ios_is_valid (ios));

	if (is_input_file_stream(ios)) {
		in_ios = (struct input_file_io_stream *) ios;
		return (in_ios->line_number);
	}
	return (IOS__C_NOT_A_LINE_NUM);
}



/*********************************
**                              **
**  IOS_VALIDATE_ACCEPT_STREAM  **
**                              **
*********************************/

static	Boolean  ios_validate_accept_stream (Mol_Symbol stream_name,
					     Boolean accept_line_p)
{
	IO_Stream   stream;

	if (stream_name == NULL  ||  stream_name == ios_SM__nil) {

	    /*  use default input stream  */
	    stream = rul___ios_get_def_in ();
	    if (stream != NULL) {
		if (rul___ios_get_atom () != stream) {
		    rul__ios_set_atom_stream (stream);
	        }
	        return TRUE;
	    } else {
		return FALSE;
	    }
	}

	stream = rul__ios_get_named_stream (stream_name);

	/*  Already ready !  */
	if (stream == rul___ios_get_atom ()) return TRUE;

	if (stream == NULL  ||  ! ios_is_valid (stream)) {
	    if (accept_line_p) {
		rul__msg_print (IOS_INVACLIOS);
	    } else {
	        rul__msg_print (IOS_INVACCIOS);
	    }
	    return FALSE;
	}

	if (! is_input_stream (stream)) {
	    rul__msg_print_w_atoms (IOS_BADINFILE, 1, stream_name);
	    return FALSE;
	}

	if (stream != rul___ios_get_atom ()) {
	    rul__ios_set_atom_stream (stream);
	}

	assert (rul___ios_get_atom () != NOT_A_REAL_STREAM);
	return TRUE;
}




/***************************
**                        **
**  RUL__IOS_ACCEPT_ATOM  **
**                        **
***************************/

Mol_Atom  rul__ios_accept_atom (Mol_Symbol stream_name)
{
	Token_Type  tok_type;
	Molecule    mol;

	if (ios_validate_accept_stream (stream_name, FALSE) == FALSE) {
	    return ios_SM__nil;
	}

	do {
	    tok_type = rul__atom_get_atom (&mol);
	    if (tok_type == TOK_EOF) return ios_SM__eof;

	} while (tok_type == TOK_EOL);

	return mol;
}



/***************************
**                        **
**  RUL__IOS_ACCEPT_LINE  **
**                        **
***************************/

Mol_Compound  rul__ios_accept_line (Mol_Symbol stream_name,
				    Mol_Compound default_return)
{
	Token_Type  tok_type;
	Molecule    mol, comp_mol;
	long        comp_elmts = 0;

	if (ios_validate_accept_stream (stream_name, TRUE) == FALSE) {
	    return ios_SM__comp_nil;
	}

	tok_type = rul__atom_get_atom (&mol);

	if (tok_type == TOK_EOF) {
	    /* There is no more input to be had  */
	    /* so return (COMPOUND END-OF-FILE)	 */
	    comp_mol = rul__mol_make_comp(1, ios_SM__eof);

	} else {

	    rul__mol_start_tmp_comp (1);
	    while ((tok_type != TOK_EOL) && /*  found the end-of line, or */
		   (tok_type != TOK_EOF)) { /*  there is no more input    */

		/* store the element    */
	      /*  if (tok_type != TOK_LPAREN && tok_type != TOK_RPAREN) */
		    rul__mol_set_tmp_comp_nth(comp_elmts++, mol);

		/* get another atom	    */
		tok_type = rul__atom_get_atom (&mol);
	    }
	    /* finish the temporary compound  */
	    comp_mol = rul__mol_end_tmp_comp ();

	    if (comp_elmts == 0) {	    	/* there was no atoms here */
		if (default_return == NULL) {   /* there was no default */

		    /* return the empty compound */
		    comp_mol = ios_SM__comp_nil;
		} else {

		    /* return the default compound */
		    comp_mol = default_return;
		}
	    }
	}
	return comp_mol;
}



/*****************************
**                          **
**  RUL__IOS_SET_DEFAULT_RT **
**                          **
*****************************/

void  rul__ios_set_default_rt (Mol_Symbol stream_name,  Mol_Symbol mol_cat)
{
  IO_Catagory cat;

  if (mol_cat == ios_SM__accept)
    cat = IOS__E_ACCEPT;
  else if (mol_cat == ios_SM__trace)
    cat = IOS__E_TRACE;
  else if (mol_cat == ios_SM__write)
    cat = IOS__E_WRITE;
  else {
    rul__msg_print_w_atoms (IOS_INVDEFKEY, 1, mol_cat);
    return;
  }
  rul__ios_set_default (stream_name, cat);
}




/***************************
**                        **
**  RUL__IOS_SET_DEFAULT  **
**                        **
***************************/

void  rul__ios_set_default (Mol_Symbol stream_name,  IO_Catagory cat)
{
	IO_Stream ios;
	Input_File_IO_Stream in_ios;
	long i;

	assert (stream_name != NULL);

	if (stream_name == ios_SM__nil) {

            if (cat == IOS__E_ACCEPT) {
		ios_set_def_in (NULL, ios_SM__nil);
            } else if (cat == IOS__E_TRACE) {
		ios_set_def_trace (NULL, ios_SM__nil);
            } else if (cat == IOS__E_WRITE) {
		ios_set_def_out (NULL, ios_SM__nil);
            }

	} else {

	    i = position_of_name_in_vector (stream_name);
	    if (i == NOT_FOUND) {
		/*  Error:  no stream by that name */
		rul__msg_print_w_atoms (IOS_NOSUCHFILE, 1, stream_name);

	    } else {

		/*  Found a valid stream  */
	        ios = ios_SA__known_ios_vector[i] ;
		assert (ios_is_valid(ios));

		if (cat == IOS__E_ACCEPT) {
		    if (is_input_stream(ios)) {
			ios_set_def_in (ios, stream_name);
			rul__ios_set_atom_stream (ios);

	            } else {

		        /*  Error:  not open for input  */
			if (ios->name) {
			    rul__msg_print_w_atoms (IOS_BADACCDEF,1,ios->name);
			}
		    }

		} else if (cat == IOS__E_TRACE) {
		    if (is_output_stream (ios)) {
			ios_set_def_trace (ios, stream_name);

		    } else {

		        /*  Error:  not open for output  */
			if (ios->name) {
			    rul__msg_print_w_atoms (IOS_BADTRADEF,1,ios->name);
			}
		    }
	
		} else if (cat == IOS__E_WRITE) {
		    if (is_output_stream (ios)) {
			ios_set_def_out (ios, stream_name);

		    } else {

		        /*  Error:  not open for output  */
			if (ios->name) {
			    rul__msg_print_w_atoms (IOS_BADWRIDEF,1,ios->name);
			}
		    }
		}
	    }
	}
}




/********************************
**                             **
**  RUL__IOS_FILE_NAME_SYMBOL  **
**                             **
********************************/

Mol_Symbol rul__ios_file_name_symbol (IO_Stream ios)
{
	File_IO_Stream fs;

	assert (ios_is_valid (ios));

	if (is_file_stream(ios)) {
	    fs = (File_IO_Stream) ios;
	    return (fs->file_name);
	}
	return (NULL);
}



/*********************************
**                              **
**  RUL__IOS_STREAM_NAME_SYMBOL **
**                              **
*********************************/

Mol_Symbol rul__ios_stream_name_symbol (IO_Stream ios)
{
	File_IO_Stream fs;

	assert (ios_is_valid (ios));

	if (is_file_stream(ios)) {
	    fs = (File_IO_Stream) ios;
	    return (fs->name);
	}
	return (NULL);
}




/***************************
**                        **
**  RUL__IOS_CURR_COLUMN  **
**                        **
***************************/

long rul__ios_curr_column (IO_Stream ios)
{
  if (ios == NULL) return 0;
  assert (ios_is_valid (ios));

  if (ios == NOT_A_REAL_STREAM) return 0;
  return (ios->cur_col);
}


/*****************************
**                          **
**  RUL__IOS_PRINT_NEWLINE  **
**                          **
*****************************/

void rul__ios_print_newline (IO_Stream ios)
{
  assert (ios_is_valid (ios));

  if (is_output_stream (ios)) {
    rul__ios_printf (ios, "\n");
    ios->cur_col = 0;
    rul__ios_flush (ios);
  }
}


/*********************
**                  **
**  RUL__IOS_FLUSH  **
**                  **
*********************/

void rul__ios_flush (IO_Stream ios)
{
  struct file_io_stream *fs;

  assert (ios_is_valid (ios));

  if (is_output_file_stream(ios)) {
    fs = (struct file_io_stream *) ios;
    fflush (fs->file_ptr);
  }
}



/****************************
**                         **
**  RUL___IOS_GET_STD_OUT  **
**                         **
****************************/

IO_Stream	rul___ios_get_std_out (void)
{
	return (ios_SA__std_out);
}



/***************************
**                        **
**  RUL___IOS_GET_STD_IN  **
**                        **
***************************/

IO_Stream	rul___ios_get_std_in (void)
{
	return (ios_SA__std_in);
}



/****************************
**                         **
**  RUL___IOS_GET_STD_ERR  **
**                         **
****************************/

IO_Stream	rul___ios_get_std_err (void)
{
	return (ios_SA__std_err);
}

/*
**  Default Input Stream:
**
**  The following 2 static variables are manipulated exclusively
**  by the three functions on this page.  By default, the name is
**  set to nil and the ios to NULL, indicating that input should be
**  taken from the standard input.
**
*/
static	Mol_Symbol	ios_SA__def_in_name = NULL;
static	IO_Stream	ios_SA__def_in = NULL;


/*********************
**                  **
**  IOS_SET_DEF_IN  **
**                  **
*********************/

static void ios_set_def_in (IO_Stream ios, Mol_Symbol stream_name)
{
	ios_SA__def_in = ios;

	rul__mol_decr_uses (ios_SA__def_in_name);
	ios_SA__def_in_name = stream_name;
	rul__mol_incr_uses (ios_SA__def_in_name);
}


/***********************
**                    **
**  IOS_CLOSE_DEF_IN  **
**                    **
***********************/

static void ios_close_def_in (void)
{
	ios_SA__def_in = NULL;
}


/****************************
**                         **
**  IOS_SIMPLE_GET_DEF_IN  **
**                         **
****************************/

static IO_Stream ios_simple_get_def_in (void)
{
	return ios_SA__def_in;
}


/***************************
**                        **
**  RUL___IOS_GET_DEF_IN  **
**                        **
***************************/

IO_Stream rul___ios_get_def_in (void)
{
	if (ios_SA__def_in == NULL) {
	    if (ios_SA__def_in_name != ios_SM__nil) {
	        rul__msg_print_w_atoms (IOS_DEFINCLOS, 1,
					ios_SA__def_in_name);
		rul__msg_print (IOS_DEFINRESD);
	        rul__mol_decr_uses (ios_SA__def_in_name);
	        ios_SA__def_in_name = ios_SM__nil;
		return (NULL); 
	    }
	    return (ios_SA__std_in);
	}
	return (ios_SA__def_in);
}

/*
**  Default Trace Stream:
**
**  The following 2 static variables are manipulated exclusively
**  by the three functions on this page.  By default, the name is
**  set to nil and the ios to NULL, indicating that trace output 
**  should be directed to the standard output.
**
*/
static	Mol_Symbol	ios_SA__def_trace_name = NULL;
static	IO_Stream	ios_SA__def_trace = NULL;


/************************
**                     **
**  IOS_SET_DEF_TRACE  **
**                     **
************************/

static void ios_set_def_trace (IO_Stream ios, Mol_Symbol stream_name)
{
	ios_SA__def_trace = ios;

	rul__mol_decr_uses (ios_SA__def_trace_name);
	ios_SA__def_trace_name = stream_name;
	rul__mol_incr_uses (ios_SA__def_trace_name);
}


/**************************
**                       **
**  IOS_CLOSE_DEF_TRACE  **
**                       **
**************************/

static void ios_close_def_trace (void)
{
	ios_set_def_trace (NULL, ios_SM__nil);
}


/******************************
**                           **
**  RUL___IOS_GET_DEF_TRACE  **
**                           **
******************************/

IO_Stream rul___ios_get_def_trace (void)
{
	if (ios_SA__def_trace == NULL) {
	    assert (ios_SA__def_trace_name == ios_SM__nil);
	    return (ios_SA__std_out);
	}
	return (ios_SA__def_trace);
}

/*
**  Default Output Stream:
**
**  The following 2 static variables are manipulated exclusively
**  by the three functions on this page.  By default, the name is
**  set to nil and the ios to NULL, indicating that WRITE output 
**  should be directed to the standard output.
**
*/
static	Mol_Symbol	ios_SA__def_out_name = NULL;
static	IO_Stream	ios_SA__def_out = NULL;


/**********************
**                   **
**  IOS_SET_DEF_OUT  **
**                   **
**********************/

static void ios_set_def_out (IO_Stream ios, Mol_Symbol stream_name)
{
	ios_SA__def_out = ios;

	rul__mol_decr_uses (ios_SA__def_out_name);
	ios_SA__def_out_name = stream_name;
	rul__mol_incr_uses (ios_SA__def_out_name);
}


/************************
**                     **
**  IOS_CLOSE_DEF_OUT  **
**                     **
************************/

static void ios_close_def_out (void)
{
	ios_SA__def_out = NULL;
}


/*****************************
**                          **
**  IOS_SIMPLE_GET_DEF_OUT  **
**                          **
*****************************/

static IO_Stream ios_simple_get_def_out (void)
{
	return ios_SA__def_out;
}

/****************************
**                         **
**  RUL___IOS_GET_DEF_OUT  **
**                         **
****************************/

IO_Stream rul___ios_get_def_out (void)
{
	if (ios_SA__def_out == NULL) {
	    if (ios_SA__def_out_name != ios_SM__nil) {
	        rul__msg_print_w_atoms (IOS_DEFOUTCLOS, 1, 
					ios_SA__def_out_name);
		rul__msg_print (IOS_DEFOUTRESD);
	        rul__mol_decr_uses (ios_SA__def_out_name);
	        ios_SA__def_out_name = ios_SM__nil;
		return (NULL);
	    }
	    return (ios_SA__std_out);
	}
	return (ios_SA__def_out);
}




/****************************
**                         **
**  RUL___IOS_GET_LISTING  **
**                         **
****************************/

IO_Stream	rul___ios_get_listing (void)
{
	return (ios_SA__listing);
}



/***************************
**                        **
**  RUL___IOS_GET_SOURCE  **
**                        **
***************************/

IO_Stream	rul___ios_get_source (void)
{
	return (ios_SA__source);
}



/***************************
**                        **
**  RUL___IOS_GET_OBJECT  **
**                        **
***************************/

IO_Stream	rul___ios_get_object (void)
{
	return (ios_SA__object);
}


/*************************
**                      **
**  RUL___IOS_GET_ATOM  **
**                      **
*************************/

IO_Stream	rul___ios_get_atom (void)
{
	return (ios_SA__atom);
}





/********************************
**                             **
**  RUL__IOS_SET_STDIN_STREAM  **
**                             **
********************************/

void  rul__ios_set_stdin_stream (IO_Stream ios)
{
	assert (ios_is_valid (ios)  &&  is_input_stream (ios));

	ios_SA__std_in = ios;
}



/*********************************
**                              **
**  RUL__IOS_SET_STDOUT_STREAM  **
**                              **
*********************************/

void  rul__ios_set_stdout_stream (IO_Stream ios)
{
	assert (ios_is_valid (ios)  &&  is_output_stream (ios));

	ios_SA__std_out = ios;
}



/*********************************
**                              **
**  RUL__IOS_SET_STDERR_STREAM  **
**                              **
*********************************/

void  rul__ios_set_stderr_stream (IO_Stream ios)
{
	assert (ios_is_valid (ios)  &&  is_output_stream (ios));

	ios_SA__std_err = ios;
}



/**********************************
**                               **
**  RUL__IOS_SET_LISTING_STREAM  **
**                               **
**********************************/

void  rul__ios_set_listing_stream (IO_Stream ios)
{
	assert (ios_is_valid (ios)  &&  is_output_stream (ios));

	ios_SA__listing = ios;
}



/*********************************
**                              **
**  RUL__IOS_SET_SOURCE_STREAM  **
**                              **
*********************************/

void  rul__ios_set_source_stream (IO_Stream ios)
{
  Input_File_IO_Stream in_ios = (Input_File_IO_Stream) ios;

  assert (ios_is_valid (ios)  &&  is_input_stream (ios));

  ios_SA__source = ios;
  if (is_input_file_stream (ios)) {
    if (in_ios->lex_buf == NULL) {
      in_ios->lex_buf = rul__atom_create_lex_buf ();
      rul__atom_restart (in_ios->file_ptr);
    }
    rul__atom_switch_lex_buf (in_ios->lex_buf);
  }
}



/*********************************
**                              **
**  RUL__IOS_SET_OBJECT_STREAM  **
**                              **
*********************************/

void  rul__ios_set_object_stream (IO_Stream ios)
{
	assert (ios_is_valid (ios)  &&  is_output_stream (ios));

	ios_SA__object = ios;
}



/*******************************
**                            **
**  RUL__IOS_SET_ATOM_STREAM  **
**                            **
*******************************/

void  rul__ios_set_atom_stream (IO_Stream ios)
{
  Input_File_IO_Stream in_ios;
  Bizzare_IO_Stream bio_ios;

  assert (ios_is_valid (ios)  &&  is_input_stream (ios));

  if (ios_SA__atom == ios) return;

  ios_SA__atom = ios;
  if (is_input_file_stream (ios)) {

    in_ios = (Input_File_IO_Stream) ios;
    if (in_ios->lex_buf == NULL) {
      in_ios->lex_buf = rul__atom_create_lex_buf ();
      rul__atom_restart (in_ios->file_ptr);
    }
    rul__atom_switch_lex_buf (in_ios->lex_buf);

  } else {
    /* bizzare input stream */
    bio_ios = (Bizzare_IO_Stream) ios;
    if (bio_ios->lex_buf == NULL) {
      bio_ios->lex_buf = rul__atom_create_lex_buf ();
      rul__atom_restart (NULL);
    }
    rul__atom_switch_lex_buf (bio_ios->lex_buf);
  }
}


/***************************
**                        **
**  RUL__IOS_OPEN_BIZZARE **
**                        **
***************************/

IO_Stream rul__ios_open_bizzare(Mol_Symbol stream_name,
				Puts_Function put_func,
				void *pfarg0, void *pfarg1,
				Gets_Function get_func,
				void *gfarg0, void *gfarg1)
{
  Bizzare_IO_Stream ios;
  
  assert (rul__mol_is_valid (stream_name));

  if (rul__ios_get_named_stream (stream_name)) {
    rul__msg_print_w_atoms (IOS_FILEOPEN, 1, stream_name);
    return (NULL);
  }

  ios = (Bizzare_IO_Stream) rul__mem_malloc(sizeof(struct bizzare_io_stream));
  ios->name = stream_name;
  ios->type = IOS__E_IN_BIZZARE;
  ios->lex_buf = NULL;
  ios->put_func_ptr = put_func;
  ios->put_func_arg0 = 0;
  ios->put_func_arg1 = 0;
  ios->get_func_ptr = get_func;
  ios->get_func_arg0 = 0;
  ios->get_func_arg1 = 0;
  if (put_func) {
    ios->put_func_arg0 = pfarg0;
    ios->put_func_arg1 = pfarg1;
    }
  if (get_func) {
    ios->get_func_arg0 = gfarg0;
    ios->get_func_arg1 = gfarg1;
  }
  if (ios->name) rul__mol_incr_uses (ios->name);

  store_in_vector ((IO_Stream) ios);

  return ((IO_Stream) ios);
}

/*
**  RUL__IOS_SET_BIZZARE
** parameters must be passed by reference (allows for no changes)
*/
void rul__ios_set_bizzare_args(IO_Stream ios,
			       void **pfarg0, void **pfarg1,
			       void **gfarg0, void **gfarg1)
{
  Bizzare_IO_Stream bs;

  assert (ios_is_valid (ios));

  if (ios->type == IOS__E_IN_BIZZARE) {

    bs = (Bizzare_IO_Stream) ios;

    if (pfarg0)
      bs->put_func_arg0 = *pfarg0;
    if (pfarg1)
      bs->put_func_arg1 = *pfarg1;
    if (gfarg0)
      bs->get_func_arg0 = *gfarg0;
    if (gfarg1)
      bs->get_func_arg1 = *gfarg1;
  }
}

/*
**  RUL__IOS_CLOSE_BIZZARE
*/
void rul__ios_close_bizzare(Mol_Symbol stream_name)
{
  struct io_stream *ios;
  long i;

  i = position_of_name_in_vector (stream_name);
  
  if (i != NOT_FOUND) {
    
    ios = (struct io_stream *) ios_SA__known_ios_vector[i] ;
    rul__mol_decr_uses (ios->name);
    remove_from_vector (i);
    rul__mem_free (ios);
    
  } else {
    /*  Error:  no such named stream is open  */
    rul__msg_print_w_atoms (IOS_NOSUCHFILE, 1, stream_name);
  }
}

/*
**  RUL__IOS_CLOSE_ALL_FILES
*/
void rul__ios_close_all_files (void)
{
  long i;
  long number_of_files = ios_SL__known_count;

  for (i = number_of_files - 1; i >= 0; i--)
    rul___ios_close_file(i);

  assert (ios_SL__known_count == 0);
}

