/****************************************************************************
**                                                                         **
**                           I O S _ P . H                                 **
**                                                                         **
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
**	This file contains the private definitions for the
**	I/O Stream (IOS) subsystem.
**
**
**  MODIFIED BY:
**	DEC	Digital Equipment Corporation
 *	CPQ	Compaq Computer Corporation
**
**  MODIFICATION HISTORY:
**
**	1-Dec-1992	DEC	Initial version
**	01-Dec-1999	CPQ	Release with GPL
*/


/*
**	IO_Stream's have been organized into an object class
**	hierarchy as shown below.  The typedefs in this file fake
**	that hierarchy given the limited facilities available in C.
**
**			IO_Stream
**			 /  |  \
**			/   |   \
**		       /    |    \
**		   File  Unreal  Bizzare
**
*/

#define BASE	3100
/*
**	First level types
*/
#define IOS_FILE		0050
#define IOS_UNREAL	  	0060
#define IOS_BIZZARE		0070
/*
**	Second level types
*/
#define IN_ONLY		0001
#define OUT_ONLY	0002

#define MASK1 0070
#define MASK2 0007

typedef enum {
	IOS__E_IN_FILE = 	(IOS_FILE    | IN_ONLY), 
	IOS__E_OUT_FILE = 	(IOS_FILE    | OUT_ONLY), 
	IOS__E_IN_BIZZARE = 	(IOS_BIZZARE | IN_ONLY),
	IOS__E_OUT_BIZZARE = 	(IOS_BIZZARE | OUT_ONLY),
	IOS__E_UNREAL =		IOS_UNREAL
} IO_Stream_Type;

typedef enum {
	IOS__E_INPUT_ONLY,
	IOS__E_OUTPUT_ONLY
} IO_Direction;

#define is_real_stream(x)		(((int)x->type & MASK1) != IOS_UNREAL)
#define is_not_real_stream(x)		(((int)x->type & MASK1) == IOS_UNREAL)

#define is_file_stream(x)		(((int)x->type & MASK1) == IOS_FILE)
#define is_not_file_stream(x)		(((int)x->type & MASK1) != IOS_FILE)

#define is_input_file_stream(x)		(x->type == IOS__E_IN_FILE)
#define is_output_file_stream(x)	(x->type == IOS__E_OUT_FILE)

#define is_input_stream(x)		(((int)x->type & MASK2) == IN_ONLY)
#define is_not_input_stream(x)		(((int)x->type & MASK2) != IN_ONLY)

#define is_output_stream(x)		(((int)x->type & MASK2) == OUT_ONLY)
#define is_not_output_stream(x)		(((int)x->type & MASK2) != OUT_ONLY)




#define	IO_STREAM_COMMON	Mol_Symbol	name;\
				IO_Stream_Type	type;\
  				long		columns;\
  				long		cur_col

struct io_stream {
	IO_STREAM_COMMON ;
};


#define FILE_STREAM_COMMON	Mol_Symbol	file_name;\
				FILE	       *file_ptr;\
				Boolean		file_opened

#define FILE_OPENED		TRUE
#define FILE_NOT_OPENED		FALSE

typedef struct file_io_stream {
	IO_STREAM_COMMON ;
	FILE_STREAM_COMMON ;
} *File_IO_Stream;


typedef struct input_file_io_stream {
	IO_STREAM_COMMON ;
	FILE_STREAM_COMMON ;
	Boolean		at_eof;
	long		line_number;
	void           *lex_buf;
} *Input_File_IO_Stream;


typedef struct bizzare_io_stream {
	IO_STREAM_COMMON ;
	void           *lex_buf;
	Puts_Function	put_func_ptr;
	void	       *put_func_arg0;
	void	       *put_func_arg1;
	Gets_Function	get_func_ptr;
	void	       *get_func_arg0;
	void	       *get_func_arg1;
} *Bizzare_IO_Stream;

