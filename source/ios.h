/****************************************************************************
**                                                                         **
**                               I O S . H                                 **
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
**	This file contains the exported definitions for the
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
**	 9-Apr-1994	DEC	Add Windows
**	01-Dec-1999	CPQ	Release with GPL
*/


#define  IOS__C_NOT_A_LINE_NUM	-1


#define  RUL__C_STD_IN		rul___ios_get_std_in()
#define	 RUL__C_STD_OUT		rul___ios_get_std_out()
#define  RUL__C_STD_ERR		rul___ios_get_std_err()
#define  RUL__C_DEF_IN		rul___ios_get_def_in()
#define  RUL__C_DEF_OUT		rul___ios_get_def_out()
#define  RUL__C_DEF_TRACE	rul___ios_get_def_trace()
#define  RUL__C_LISTING		rul___ios_get_listing()
#define  RUL__C_SOURCE		rul___ios_get_source()
#define  RUL__C_OBJECT		rul___ios_get_object()
#define  RUL__C_ATOM		rul___ios_get_atom()


typedef enum {
	IOS__E_IN = 8172, 
	IOS__E_OUT,
	IOS__E_APPEND
} IO_Access_Mode;

typedef enum {
	IOS__E_ACCEPT = 3111, 
	IOS__E_TRACE,
	IOS__E_WRITE
} IO_Catagory;


typedef void (*Puts_Function)(char *, void *, void *);
typedef char *(*Gets_Function)(char *, long, void *, void *);


/* INCLUDE-IN-GEND.H  *********************************************** */

void  rul__ios_init (void);

IO_Stream  rul__ios_open_file_rt (Mol_Symbol stream_name, 
				  Mol_Symbol file_name,
				  Mol_Symbol acc_mode);

void  rul__ios_close_files (long mol_count, ...);

void  rul__ios_write (long mol_count, ...);

void  rul__ios_set_default_rt (Mol_Symbol stream_name,  Mol_Symbol cat);

Mol_Symbol rul__ios_is_open (Mol_Symbol stream_name);

Mol_Atom  rul__ios_accept_atom (Mol_Symbol stream_name);

Mol_Compound  rul__ios_accept_line (Mol_Symbol stream,
				    Mol_Compound default_return);

/* END-INCLUDE-IN-GEND.H  *********************************************** */

IO_Stream  rul__ios_open_file (Mol_Symbol stream_name, 
			       Mol_Symbol file_name,
			       IO_Access_Mode acc_mode);

void  rul__ios_printf (IO_Stream stream, char *format_string, ...);

char  *rul__ios_fgets (IO_Stream ios, char *buffer, long max_chars);

Boolean rul__ios_stream_at_eof (IO_Stream stream);

IO_Stream  rul__ios_get_named_stream (Mol_Symbol stream_name);

void  rul__ios_close_file (Mol_Symbol stream_name);

void  rul__ios_set_default (Mol_Symbol stream_name,  IO_Catagory cat);

long rul__ios_curr_line_num (IO_Stream stream);

Mol_Symbol rul__ios_file_name_symbol (IO_Stream ios);

Mol_Symbol rul__ios_stream_name_symbol (IO_Stream ios);

long rul__ios_curr_column (IO_Stream ios);

void rul__ios_print_newline (IO_Stream ios);

void rul__ios_flush (IO_Stream ios);




/*
**  These should only be used by a debugger environment
*/
void  rul__ios_set_stdin_stream (IO_Stream stream);
void  rul__ios_set_stdout_stream (IO_Stream stream);
void  rul__ios_set_stderr_stream (IO_Stream stream);


/*
**  The following should only be used from the compiler
*/
void  rul__ios_set_listing_stream (IO_Stream stream);
void  rul__ios_set_source_stream (IO_Stream stream);
void  rul__ios_set_object_stream (IO_Stream stream);
void  rul__ios_set_atom_stream (IO_Stream stream);


/*
**  Use the following only via the macros defined above (e.g. RUL__C_STD_ERR)
*/
IO_Stream	rul___ios_get_std_out (void);
IO_Stream	rul___ios_get_std_in (void);
IO_Stream	rul___ios_get_std_err (void);
IO_Stream	rul___ios_get_def_in (void);
IO_Stream	rul___ios_get_def_out (void);
IO_Stream	rul___ios_get_def_trace (void);
IO_Stream	rul___ios_get_listing (void);
IO_Stream	rul___ios_get_source (void);
IO_Stream	rul___ios_get_object (void);
IO_Stream	rul___ios_get_atom (void);


/*
** Bizzare ios routines
*/

IO_Stream rul__ios_open_bizzare(Mol_Symbol stream_name,
				Puts_Function put_func,
				void *pfarg0, void *pfarg1,
				Gets_Function get_func,
				void *gfarg0, void *gfarg1);

void rul__ios_set_bizzare_args(IO_Stream ios,
			       void **pfarg0, void **pfarg1,
			       void **gfarg0, void **gfarg1);

void rul__ios_close_bizzare(Mol_Symbol stream_name);

void rul__ios_close_all_files (void);

/* Define Windows-only functions, and no-op them if not building for Windows */
#ifdef WINDOWS
void rul__ios_win_init(void);
void rul__ios_ensure_window_open(IO_Stream ios);
#ifdef including_windows_isnt_slow_and_doesnt_conflict_with_our_ATOM
#include <windows.h>		/* For HINSTANCE */
void rul_win_init(HINSTANCE hinstCurrent, HINSTANCE hinstPrevious);
#endif
#else
#define rul__ios_win_init()		 /* Do nothing */
#define rul__ios_ensure_window_open(ios) /* Do nothing */
#endif
