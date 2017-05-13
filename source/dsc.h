/****************************************************************************
**                                                                         **
**                               D S C . H                                 **
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
 * FACILITY:
 *	RULEWORKS Run Time System
 *
 * ABSTRACT:
 *	This header file contains portable definitions from descrip.h
 *	normally only avaliable on VMS systems
 *
 * MODIFIED BY:
 *	DEC	Digital Equipment Corporation
**	CPQ	Compaq Computer Corporation
 *
 * MODIFICATION HISTORY:
 *
 *	11-Mar-1993	DEC	Initial version
 *	01-Dec-1999	CPQ	Release with GPL
 */


#ifdef __VMS
#include <descrip.h>
/*
 * Since the dollar sign character is not allowed on some non VMS platforms,
 * it cannot ever be used in the source code, but the following definitions
 * will allow the symbols without $ to be changed to their counterparts
 * with $ in descrip.h on VMS platforms.
 */
#define dsc_w_length dsc$w_length
#define dsc_b_dtype dsc$b_dtype
#define dsc_b_class dsc$b_class
#define dsc_a_pointer dsc$a_pointer
#define DSC_K_DTYPE_T DSC$K_DTYPE_T
#define DSC_K_CLASS_S DSC$K_CLASS_S

#else
/*
 * Non VMS string descriptor definitions
 */
struct  dsc_descriptor
{
        unsigned short  dsc_w_length;   /* specific to descriptor class;
					 * typically a 16-bit (unsigned)
					 * length */
        unsigned char   dsc_b_dtype;    /* data type code */
        unsigned char   dsc_b_class;    /* descriptor class code */
        char            *dsc_a_pointer; /* address of first byte of data
					 * element */
};

#define DSC_K_DTYPE_T   14              /* character string;  a single
					 * 8-bit character or a sequence
					 * of characters */
#define DSC_K_CLASS_S   1               /* fixed-length descriptor */

#endif



