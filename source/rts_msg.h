/* rts_msg.h - Run-Time-System Message definitions */
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
 *	%c, numeric (decimal) inserts as %d or %ld.  % needs to be doubled as
 *	%%.  Backslash and quote need to be quoted by preceding with \.
 *	Newline is \n.
 *
 *  MODIFIED BY:
 *	DEC	Digital Equipment Corporation
 *	CPQ	Compaq Computer Corporation
 *
 *  MODIFICATION HISTORY:
 *
 *	 9-Sep-1992	DEC	Initial version
 *	01-Dec-1999	CPQ	Release with GPL
 */

/******************** RTS (run-time-system errors) ************/


#define RTS_ATTNOTCOMP \
  "ATTNOTCOMP W Attribute not compound %s, invalid index %s"

#define RTS_VALNOTCOMP \
  "VALNOTCOMP W Non-compound value %s, attribute %s not changed"

#define RTS_VALNOTSCLR \
  "VALNOTSCLR W Non-scalar value %s, attribute %s not changed"

#define RTS_VALNOTSCX \
  "VALNOTSCX W Non-scalar value %s, attribute %s, index %s not changed"

#define RTS_VALNOTTAB \
  "VALNOTTAB W Non-table value %s, attribute %s not changed"

#define RTS_VALNOTSYM \
  "VALNOTSYM W Non-symbolic value %s, attribute %s not changed"

#define RTS_VALNOTID \
  "VALNOTID W Non-instance value %s, attribute %s not changed"

#define RTS_INVCLSID \
  "INVCLSID W Invalid value class %s, attribute %s not changed"

#define RTS_NOVALWMO \
  "NOVALWMO W No value wmo %s, attribute %s not changed"

#define RTS_VALNOTOPA \
  "VALNOTOPA W Non-opaque value %s, attribute %s not changed"

#define RTS_VALNOTNUM \
  "VALNOTNUM W Non-numeric value %s, attribute %s not changed"

#define RTS_VALNOTINT \
  "VALNOTINT W Non-integer value %s, attribute %s not changed"

#define RTS_VALNOTDBL \
  "VALNOTDBL W Non-float value %s, attribute %s not changed"

#define RTS_BADTABVAL \
  "BADTABVAL W TABTO argument value %ld less than or equal to 0"

#define RTS_BADRJUVAL \
  "BADRJUVAL W RJUST argument value %ld must be between 1 and %d"

#define RTS_SYMTRUNC \
  "SYMTRUNC W Symbol too long, truncated to %s"

#define RTS_FATCAT \
  "FATCAT W Concatenated symbol too long, truncated to %s"

#define RTS_INVSUBSYM \
  "INVSUBSYM W Invalid SUBSYMBOL parameter %s, must be scalar"

#define RTS_INVSYMLEN \
  "INVSUBSYM W Invalid SYMBOL-LENGTH parameter %s, must be scalar"

#define RTS_NOITABVAL \
  "NOITABVAL W Non-integer TABTO value %s"

#define RTS_NOIRJUVAL \
  "NOIRJUVAL W Non-integer RJUST value %s"

#define RTS_NOSUCHWM \
  "NOSUCHWM E No such WMO %s"

#define RTS_NOSUCHID \
  "NOSUCHID E No such instance %s"

#define RTS_INVWMO \
  "INVWMO E Invalid WMO%s"

#define RTS_INVWMOATT \
  "INVWMOATT E Attribute %s not in WMO %s"

#define RTS_INVWMOID \
  "INVWMOID E Invalid instance id %s"

#define RTS_MAKWMEEXI \
  "MAKWMEEXI E Attempted to MAKE a wme with INSTANCE-ID of existing wme %s"

#define RTS_INVCMPIDX \
  "INVCMPIDX W Invalid compound index %s, must be integer > 0"

#define RTS_INVNTHPAR \
  "INVNTHPAR W Invalid GET NTH parameter %s, must be a compound"

#define RTS_INVMAKCLS \
  "INVMAKCLS W Invalid class in MAKE action, $ROOT not allowed"

#define RTS_NOSUCHCLS \
  "NOSUCHCLS W No such class %s"

#define RTS_NOTINSCOPE \
  "NOTINSCOPE W Cannot affect an instance of class, %s, that is not visible in this scope"

#define RTS_INVWMONES \
  "INVWMONES W Invalid attempt to modify instance %s, while it is being modified" 

#define RTS_INVMAXMIN \
  "INVMAXMIN W Type mismatch in MAX/MIN comparison, %s"

/*********************************************************/

#define RTS_INVCVTSH \
  "INVCVTSH W Invalid conversion to SHORT, zero (0) value returned"
/*
 * The size representable by a SHORT is defined by the range between
 * SHRT_MIN and SHRT_MAX in the C header file sys$library:limits.h.
 */

#define RTS_INVCVTBY \
  "INVCVTBY W Invalid conversion to BYTE, zero (0) value returned"
/*
 * The size representable by a BYTE is defined by the range between
 * SCHAR_MIN and SCHAR_MAX in the C header file sys$library:limits.h.
 */

#define RTS_INVCVTIN \
  "INVCVTIN W Invalid conversion to integer atom, negative value returned"
/*
 * The largest value representable  by an integer atom is the same as 
 * LONG_MAX in the C header file sys$library:limits.h.
 */

#define RTS_INVCVTUS \
  " INVCVTUS W Invalid conversion to UNSIGNED-SHORT, zero (0) value returned"
/*
 * The size representable by a UNSIGNED-SHORT is defined by the range between
 * 0 and USHRT_MAX in the C header file sys$library:limits.h.
 */

#define RTS_INVCVTUB \
  "INVCVTUB W Invalid conversion to UNSIGNED-BYTE, zero (0) value returned"
/*
 * The size representable by a UNSIGNED-BYTE is defined by the range between
 * 0 and UCHAR_MAX in the C header file sys$library:limits.h.
 */

#define RTS_INVCVTXXXX \
  "INVCVTXXXX W No conversion defined from %s to %s, zero value returned"
/*
 * There is no way to convert between the types requested.  For example,
 * a symbol cannot be converted to a long.  Likely problems range from
 * completely invalid  uses of external routines to variables bound
 * to unexpected values.  It is unlikely a zero it the correct value
 * to use after this error.
 */

#define RTS_EXTARRLOST \
  "EXTARRLOST W External array data lost during conversion to COMPOUND with shorter length"
/*
 * A Read-Write parameter is bound to a compound that is shorter than 
 * the external array it is passed out to.  On return, additional
 * elements in the external array are ignored.  
 */

#define RTS_CMPELTLOST \
  "CMPELTLOST W Compound value elements lost during conversion to external array with shorter length"
/* 
 * A compound value has been converted to an external array with a length
 * less than the length of the compound value.
 */

#define RTS_NONNUMVAL \
  "NONNUMVAL W Non-numeric value, %s%s"

#define RTS_DIVZERO \
  "DIVZERO W Attempt to divide %s by zero"

#define RTS_INVMODULO \
  "INVMODULO W Attempt to use a float value, %s, as a modulus"

#define RTS_INVMOD \
  "INVMOD W Attempt to take a modulus of a float value, %s"

#define RTS_STRMISMAT \
  "STRMISMAT W Stategy mismatch in blocks %s and %s"

#define RTS_RBREAK \
  "RBREAK I RBREAK encountered on rule %s"

/*
 * method call errors
 */

#define RTS_BADMETARG \
  "BADMETARG W Method %s call recieved bad WHEN argument"

#define RTS_NOCLSMET \
  "NOCLSMET W Method %s not defined for class %s"

#define RTS_METARGCNT \
  "METARGCNT W Method %s call recieved %ld arguments, expected %ld"

#define RTS_METINVARG \
  "METINVARG W Method %s call argument %ld invalid %s"

