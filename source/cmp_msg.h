/* cmp_msg.h - compiler Message definitions */
/****************************************************************************
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


/************************** Syntax errors *********************/

#define CMP_NOSOURCEFILE \
    "NOSOURCEFILE E No source file name supplied"

#define CMP_FILNOTFND \
    "FILNOTFND E File not found: %s"

#define CMP_SYNTAXERR \
    "SYNTAXERR E Syntax error: %s%s%s"

/*
 *  WRITERS:  Do not insert a space in "file%s," below.  The text strings to 
 *  be inserted there supply their own leading space.
 */
#define CMP_UNEXPEOF \
    "UNEXPEOF E Unexpected end of file%s"

#define CMP_UNTERMCON \
    "UNTERMCON E Construct not terminated; missing closing parenthesis"

#define CMP_UNTERMCE \
    "UNTERMCE W Unterminated condition element; inserted ')' before '-->'"

#define CMP_UNTERMIGN \
    "UNTERMIGN I Unterminated construct ignored"

#define CMP_NOACTION \
  "NOACTION W No actions found"

#define CMP_INVINSTID \
  "INVINSTID W Invalid instance-id constant in source code; value replaced with #0"

#define CMP_INVOPAQUE \
  "INVOPAQUE W Invalid opaque constant in source code; value replaced with %%x0"




/************************** Block errors **********************/

#define CMP_MISENDBLK \
"MISENDBLK W Missing END-BLOCK for block %s, END-BLOCK inserted"

#define CMP_BADENDBLK \
"BADENDBLK W Block name %s on END-BLOCK does not match current block name %s"

#define CMP_NOTINBLK \
"NOTINBLK E Expecting ENTRY-BLOCK, DECLARATION-BLOCK, or RULE-BLOCK, found %s %s, ignored"

#define CMP_DUPBLOCK \
"DUPBLOCK W Declaration of block %s conflicts with previous block with same name, construct ignored"
/* We could also provide the line number of the other block with extra work. */

#define CMP_DUPGROUP \
"DUPGROUP W Declaration of group %s conflicts with previous construct with same name, construct ignored"

#define CMP_NESGROUP \
"NESGROUP W Declaration of group %s starts before end of group %s, construct ignored"

#define CMP_BADENDGRP \
"BADENDGRP W Group name %s on END-GROUP does not match current group name %s"

#define CMP_DECLIGNORED \
"DECLIGNORED W Declaration %s ignored; all declarations must precede executable constructs"
/* When a declaration construct is found following an executable construct
   in a block, we complain and ignore the declaration. */

#define CMP_INVBLKNAME \
"INVBLKNAME E Invalid block name %s contains invalid characters"
/*
 * Block names are used to identify linkable entities.  They are
 * required to not contain characters that may have special meaning
 * on some operating systems.  Therefore block names must be composed
 * of only letters, numbers and underscores.  (An underscore may not
 * be the first character.)
 */





/************************** Class errors **********************/

#define CMP_DUPCLASS \
    "DUPCLASS W Duplicate declaration of class %s ignored, previous declaration in block %s"

#define CMP_MULDECCLS \
    "MULDECCLS E Muliply declared class %s, declared in blocks %s and %s; both cannot be USEd"

#define CMP_NOPARENT \
    "NOPARENT W Class may not inherit from an undeclared or non-local class, %s; INHERITS-FROM clause ignored"
#define CMP_SELFPARENT \
    "SELFPARENT W Class may not inherit from itself; INHERITS-FROM clause ignored"

#define CMP_DUPATTR \
    "DUPATTR W Duplicate declaration of attribute %s; second occurrence ignored"
#define CMP_MULTDEF \
    "MULTDEF W Multiple DEFAULT clauses for attribute %s, second DEFAULT ignored"
#define CMP_MULTFILL \
    "MULTFILL W Multiple FILL clauses for attribute %s; second FILL ignored"
#define CMP_INHCOMP \
    "INHCOMP W Invalid attempt to redefine inherited attribute %s to be COMPOUND; COMPOUND ignored"
#define CMP_BADCOMPDEF \
    "BADCOMPDEF W Compound default value not allowed for scalar attribute %s; DEFAULT ignored"
#define CMP_BADSCADEF \
    "BADSCADEF W Scalar default value not allowed for compound attribute %s; DEFAULT ignored"
#define CMP_SCALFILL \
    "SCALFILL W Attribute %s is scalar; unexpected FILL value ignored"

#define CMP_INHDOM \
    "INHDOM W Invalid attempt to change domain of inherited attribute %s; new domain ignored"
#define CMP_UNKDOMCLASS \
    "DOMCLASS W Attempt to restrict domain of attribute %s to unknown class, %s; restriction ignored"





/******************** External Routine errors *****************/

#define CMP_EXTDUPDEF \
  "EXTDUPDEF W Ignoring duplicate declaration for routine %s in block %s"
/*
 * An external routine is declared more than once in the same declaration
 * block.  The first declaration is used, subsequent declarations are ignored.
 */

#define CMP_EXTMULDEF \
  "EXTMULDEF W Ignoring routine declaration %s in declaration block %s, using declaration from block %s"
/*
 * An external routine is declared differently in two declaration blocks.
 * The second external routine declaration will be ignored.
 */

#define CMP_EXTDUPALIAS \
  "EXTDUPALIAS W Ignoring duplicate ALIAS-FOR clause for routine %s"
/*
 * An external routine declaration may have only one ALIAS clause.  The
 * first ALIAS clause is used others are ignored.
 */

#define CMP_EXTDUPINP \
  "EXTDUPINP W Ignoring duplicate ACCEPTS clause in routine %s"
/*
 * An external routine declaration may have only one ACCEPTS clause.
 * The first ACCEPTS clause is used others are ignored.
 */

#define CMP_EXTDUPRET \
  "EXTDUPRET W Ignoring duplicate RETURNS clause in routine %s"
/*
 * An external routine declaration may have only one RETURNS clause.
 * The first RETURNS clause is used others are ignored.
 */

#define CMP_EXTINVBYVAL \
  "EXTINVBYVAL W Ignoring declaration of %s containing invalid type to be passed by value"
/*
 * An external routine declaration includes a parameter to be passed by
 * value (a string or array) that is invalid.
 * An array or string may only be passed by reference.
 */

#define CMP_EXTINVARR \
  "EXTINVARR W Ignoring declaration of %s with array of unspecified length"
/* 
 * An array of unspecified length may not be returned from an external routine.
 */

#define CMP_EXTUNDEF \
"EXTUNDEF W Undefined external routine %s"
/*
 * An external routine must be declared with an (EXTERNAL-ROUTINE ...)
 * declaration before a call to the routine is made.
 */

#define CMP_EXTUNDPAR \
"EXTUNDPAR W Undefined routine parameter %s"
/*
 * An external routine parameter must be declared before it is used
 * as an array length parameter.
 */

#define CMP_EXTINVTYP \
"EXTINVTYP W Invalid parameter type (non-integer) used as array size argument %s"
/*
 * An external routine parameter must be an int to be used
 * as an array length parameter.
 */

#define CMP_EXTINVCALL \
"EXTINVCALL W Call to external routine %s uses wrong number of arguments"
/*
 * External routine calls must have the same number of parameters as 
 * was defined in the external routine declaration.
 */

#define CMP_EXTINVNESTING \
"EXTINVNESTING W In external routine %s, conversions between external types may not be portable"
/*
 * An external routine call is nested as a parameter to another external
 * call.  The return type of the nested routine does not match the declared
 * parameter type.  
 */

#define CMP_INVCONVER \
"INVCONVER W Conversions between external types may not be portable"
/*
 * The external type of a parameter does not match the type where
 * an external value is used.  This is likely an invalid parameter
 * value (a double being used where a long is required) that will 
 * result in invalid code.
 */

#define CMP_EXTBADCVT \
"EXTBADCVT W External routine conversion from an %s %s %s to an %s %s %s is undefined"
/*
 * An attempt has been made to relate an RULEWORKS type to an external type for
 * which no conversion is defined.  The user may pass any RULEWORKS type as
 * external type ATOM and then convert it in the external code.
 */

#define CMP_EXTINVRWPAR \
"EXTINVRWPAR W Cannot pass nonvariable argument to read-write parameter; parameter treated as read-only"
/*
 * Read-write parameters to external routines require a variable to write back
 * the value after the call.  If a constant is passed to a routine parameter
 * declared as read-write, the parameter is treated as read-only.
 */

#define CMP_EXTINVLHSARG \
"EXTINVLHS W Cannot call external routine %s with invalid passing mechanism on LHS; read-write parameter treated as read-only"
/*
 * An external routine called from the LHS may not pass arguments
 * BY REFERENCE READ-WRITE.  Read-write parameters will be treated as
 * read-only parameters on the LHS.
 */

#define CMP_EXTNORETURN \
"EXTINVRHS W External routine %s used as value returns no value; return value required"
/*
 * An external routine used as a value must have a return value.
 */




/************************** Entry-Block errors ****************/

/*
 * An entry-block declaration may have only one ACCEPTS clause.
 * The first ACCEPTS clause is used others are ignored.
 */
#define CMP_EBDUPINP \
"EBDUPINP W Ignoring duplicate ACCEPTS clause in entry block %s"

/*
 * If you don't supply an argument name for a parameter to an entry-block;
 * the parameter will be ignored since there is no handle to access it.
 */
#define CMP_EBNOARGNAME \
"EBNOARGNAME W Missing formal parameter name for parameter to entry block %s"

/*
 * An entry-block declaration may have only one RETURNS clause.
 * The first RETURNS clause is used others are ignored.
 */
#define CMP_EBDUPRET \
"EBDUPRET W Ignoring duplicate RETURNS clause in entry block %s"

/*
 * An entry-block declaration may have only one USES clause.
 * The first USES clause is used others are ignored.
 */
#define CMP_EBDUPUSES \
"EBDUPUSES W Ignoring duplicate USES clause in entry block %s"

/*
 * An entry-block declaration may have only one ACTIVATES clause.
 * The first ACTIVATES clause is used others are ignored.
 */
#define CMP_EBDUPACT \
"EBDUPACT W Ignoring duplicate ACTIVATES clause in entry block %s"

/*
 * An entry-block declaration may have only one STRATEGY clause.
 * The first STRATEGY clause is used others are ignored.
 */
#define CMP_EBDUPSTR \
"EBDUPACT W Ignoring duplicate STRATEGY clause in entry block %s"

/*
 * An entry block is declared with a RETURNS clause, but no rule or on- clause
 * contains a RETURN action.
 */
#define CMP_MISRETURN \
"MISRETURN W No RETURN action found in entry block %s"




/************************** Rule errors ***********************/

#define CMP_DUPRULENAME \
    "DUPRULENAME W Duplicate rule or catcher name %s, construct ignored"

#define CMP_UNDECCLASS \
    "UNDECCLASS E Reference to undeclared object class %s"

#define CMP_TESTIGNORED \
    "TESTIGNORED I Predicate test ignored"

#define CMP_ATTRNOTDECL \
    "ATTRNOTDECL W Attribute %s not declared in object class %s"

#define CMP_NOINDEXSCAL \
    "NOINDEXSCAL W Cannot index element of attribute %s (not declared COMPOUND)"

#define CMP_ATTRNOLEN \
    "ATTRNOTCOMP W Cannot bind the length of attribute %s (not declared COMPOUND)"

/*
**  The length predicates ([=], [>], [<], [<=], [>=], and [<>]) 
**  can only be used on compound attributes.
*/
#define CMP_ATTRNOTCOMP \
    "ATTRNOTCOMP W Cannot test the length of attribute %s (not declared COMPOUND)"

#define CMP_CONTADJCOMP \
    "CONTADJCOMP W Cannot search attribute %s (not declared COMPOUND); scalar predicate may be in wrong position"

#define CMP_CONTADJSCA \
    "CONTADJSCA W Attribute %s must be scalar or a compound element to use with a scalar predicate followed by a containment predicate; scalar predicate may be in wrong position"

#define CMP_PREDBADTYPE \
    "PREDBADTYPE W The predicates >, <, >=, <=, =, -=, ~=, and -~= should  be used only with symbolic or numeric attributes; %s is neither"

/*
**  The predicates:  >, <, >=, <=, ~=, = 
**  are all scalar predicates, and can only be used to compare
**  scalar values with scalar value expressions
*/
#define CMP_ATTRNOTSCA \
    "ATTRNOTSCA W Cannot test a compound attribute, %s, with a scalar predicate"

#define CMP_INVINDEXTYP \
    "INVINDEXTYP W Compound attribute index %s must be positive integer"

#define CMP_INVCAINDEX \
    "INVCAINDEX W Compound attribute index %s must be greater than 0"

#define CMP_LENNOTINT \
    "LENNOTINT W Cannot compare length of compound attribute %s to noninteger"

#define CMP_DIFFTYPEMATCH \
    "DIFFTYPEMATCH W Cannot match %s attribute %s with %s value"

#define CMP_NOMODIFYID \
    "NOMODIFYID W Cannot modify attribute ^$ID"

#define CMP_NOMODIFYCLASS \
    "NOMODIFYCLASS W Cannot modify attribute ^$INSTANCE-OF"

#define CMP_WMEMISSING \
    "WMEMISSING W Cannot modify, remove, or copy an object that has been removed"



/******************** lhs errors ******************************/

#define CMP_VARMULBIND \
"VARMULBIND I Variable %s bound in a negated condition and rebound here"
/*
 * It is generally a bad idea (or an accidental error) to use the same
 * variable within a negated condition and then binding it again in a
 * subsequent condition element.
 */

#define CMP_VARDIFTYP \
"VARDIFTYP W Variable %s is bound to different types of attributes"
/*
 * This variable is bound in different places to different shaped
 * attributes (e.g. scalar in one and compound in another).
 */

#define CMP_MATCHNEVER \
"MATCHNEVER W Attribute %s and %s are instance-ids of disjoint classes, and thus can never be identical"
/*
 * Because the attributes are both instance-ids but of disjoint classes,
 * this test, and ergo the entire condition can never match.
 */



/******************** rhs errors ******************************/

#define CMP_VARNEGLHS \
"VARNEGLHS E Variable %s bound only in negated LHS context, not available on RHS"
/*
 * This variable has no current binding.  It was bound on the LHS in
 * a negated context, but that value is not available on the RHS.
 */

#define CMP_VARNOTBOUND \
"VARNOTBOUND E Variable %s has no value binding"
/*
 * This variable has no current binding. (Probable misspelling, or an error in
 * the expression to which the variable was to be bound.)
 */

#define CMP_INVRHSVAL \
"INVRHSACT E Invalid value expression; indicates compiler bug in partially implemented feature"
/*
 * Attempt to use a value expression for which no compilation is
 * defined.  This indicates a compiler bug in a partially implemented
 * feature.
 */

#define CMP_INVRHSACT \
"INVRHSACT W Invalid RHS action; indicates compiler bug in partially implemented feature"
/*
 * Attempt to use a RHS action for which no compilation is
 * defined.  This indicates a compiler bug in a partially implemented
 * feature.
 */

#define CMP_INVSQLACT \
  "INVSQLACT W Invalid SQL action nesting"
/*
 * Attempt to execute an sql function within an sql loop
 */

#define CMP_RETACTEXE \
  "RETACTEXE W RETURN action executed, remaining actions ignored"
/*
 * Actions on rhs after a return [arg] will be ignored
 */


#define CMP_QUTACTEXE \
  "QURACTEXE W QUIT action executed, remaining actions ignored"
/*
 * Actions on rhs after a quit [arg] will be ignored
 */


#define CMP_INVRJUUSA \
  "INVRJUUSA W Invalid RJUST usage, allowed only with WRITE action"

#define CMP_INVTABUSA \
  "INVTABUSA W Invalid TABTO usage, allowed only with WRITE action"

#define CMP_INVCRLUSA \
  "INVCRLUSA W Invalid CRLF usage, allowed only with WRITE action"


/******************** rhs sem action errors *******************/


#define CMP_INVADDPAR \
  "INVADDPAR W Invalid ADDSTATE parameter %s, must be a constant or variable"

#define CMP_INVATPAR \
  "INVATPAR W Invalid @ parameter %s, must be a constant or variable"

#define CMP_INVBINPAR \
  "INVBINPAR W Invalid BIND parameter %s, must be a variable"

#define CMP_NOBUILD \
  "NOBUILD W BUILD not implemented in this version"
  
#define CMP_INVCOPPAR \
  "INVCOPPAR W Invalid COPY parameter %s, must be a variable"

#define CMP_INVCOPVAR \
  "INVCOPVAR W Invalid COPY variable %s, must be an ID"

#define CMP_INVCATSYM \
  "INVCATSYM W Invalid catcher name %s"

#define CMP_INVCATNAM \
  "INVCATNAM W No catcher with name %s"

#define CMP_INVCLOPAR \
  "INVCLOPAR W Invalid CLOSEFILE parameter %s"

#define CMP_INVATTCLS \
  "INVATTCLS W Invalid attribute: %s is not an attribute in class %s"

#define CMP_INVATTSPE \
  "INVATTSPE W Invalid attribute specification %s, must be a constant or variable"

#define CMP_INVDEFFID \
  "INVDEFFID W Invalid DEFAULT file identifier %s, must be a constant or variable"

#define CMP_INVDEFKTP \
  "INVDEFKTP W Invalid DEFAULT keyword %s, must be a constant or variable"

#define CMP_INVDEFKEY \
  "INVDEFKEY W Invalid DEFAULT keyword %s, must be a constant or variable"

#define CMP_INVFORPAR \
  "INVFORPAR W Invalid FOR-EACH parameter %s, must be a variable"

#define CMP_INVLHSFUN \
  "INVLHSFUN E The %s function can not be used on the left-hand side of a rule"

#define CMP_INVGETIDARG \
  "INVGETIDARG W Invalid GET argument: first argument must be an instance-id"

#define CMP_IDARGNOCLASS \
  "IDARGNOCLASS I Unable to verify class of instance-id argument to %s; run-time checks inserted"
#define CMP_IDARGNOVER \
  "IDARGNOVER I Unable to verify argument to %s is an instance-id; run-time checks inserted"

/*	The following are paramaters to the above two messages  */
#define CMP_C_MSG_GET     "GET"
#define CMP_C_MSG_REMOVE  "REMOVE"
#define CMP_C_MSG_MODIFY  "MODIFY"
#define CMP_C_MSG_COPY	  "COPY"
#define CMP_C_MSG_SPECIAL "SPECIALIZE"


#define CMP_INVVARCLS \
  "INVVARCLS W Invalid ^$ID variable %s, no class set"

#define CMP_INVMAKPAR \
  "INVMAKPAR W Invalid MAKE parameter %s, must be a class name"

#define CMP_INVMAKCLS \
  "INVMAKCLS W Invalid MAKE class %s"

#define CMP_INVMODPAR \
  "INVMODPAR W Invalid MODIFY parameter %s, must be a variable"

#define CMP_INVMODVAR \
  "INVMODVAR W Invalid MODIFY variable %s, must be an ID"

#define CMP_INVCLSNAM \
  "INVCLSNAM W Invalid class %s"

#define CMP_INVOPNFID \
  "INVOPNFID W Invalid OPENFILE file identifier %s, must be a constant or variable"

#define CMP_INVOPNFIL \
  "INVOPNFIL W Invalid OPENFILE file name %s, must be a constant or variable"

#define CMP_INVOPNMOD \
  "INVOPNMOD W Invalid OPENFILE mode %s, must be a constant or variable"

#define CMP_INVOPNMDC \
  "INVOPNMDC W Invalid OPENFILE mode %s, must be IN, OUT or APPEND"

#define CMP_INVQUIPAR \
  "INVQUIPAR W Invalid QUIT parameter %s"

#define CMP_INVREMPAR \
  "INVREMPAR W Invalid REMOVE parameter %s"

#define CMP_INVREMVAR \
  "INVREMVAR W Invalid REMOVE variable %s, must be an ID"

#define CMP_INVRMEPAR \
  "INVRMEPAR W Invalid REMOVE-EVERY parameter %s, must be a constant or variable"

#define CMP_INVRESPAR \
  "INVRESPAR W Invalid RESTORESTATE parameter %s, must be a constant or variable"

#define CMP_INVRETURN \
  "INVRETURN W Invalid RETURN, must be in ENTRY-BLOCK"

#define CMP_RETVALREQ \
  "RETVALREQ W Return value required"

#define CMP_RETVALIGN \
  "RETVALIGN W Return value ignored"

#define CMP_INVSAVPAR \
  "INVSAVPAR W Invalid SAVESTATE parameter %s"

#define CMP_INVSPEPAR \
  "INVSPEPAR W Invalid SPECIALIZE parameter %s, must be a variable"

#define CMP_INVSPEVAR \
  "INVSPEVAR W Invalid SPECIALIZE variable %s, must be an ID"

#define CMP_INVSPECLS \
  "INVSPECLS W Invalid SPECIALIZE parameter %s, must be a class name"

#define CMP_INVSPESCS \
  "INVSPESCS W Invalid SPECIALIZE class %s, not a valid subclass"

#define CMP_INVWATPAR \
  "INVWATPAR W Invalid WATCH parameter %s, must be a constant"

#define CMP_INVWRIPAR \
  "INVWRIPAR W Invalid WRITE parameter %s, must be a constant or variable"

#define CMP_INVVALPRED \
  "INVVALPRED W Invalid conditional expression: %s%s%s"
/*
 * The shape and domain of the values in combination with the predicate
 * test was not appropriate.
 */

/******************** rhs sem bti errors **********************/

#define CMP_INVACAFID \
  "INVACAPAR W Invalid ACCEPT-ATOM file-id %s"

#define CMP_INVACLCMP \
  "INVACLCMP W Invalid ACCEPTLINE-COMPOUND default compound %s"

#define CMP_INVACLFID \
  "INVFILEID W Invalid ACCEPTLINE-COMPOUND file-id %s"

#define CMP_NONINTRJU \
  "NONINTRJU W Noninteger RJUST parameter %s"

#define CMP_NONCMPSUB \
  "NONCMPSUB W Noncompound SUBCOMPOUND parameter %s"

#define CMP_NONINTTAB \
  "NONINTTAB W Noninteger TABTO parameter %s"

#define CMP_MISMATCHTYP \
  "MISMATCHTYP W Mismatched predicate and value: to successfully match would require %s but %s is %s"
/*
 * The type of the value found in the predicate test was not appropriate,
 * given the shape and domain of the attribute, and the predicate itself.
 */

#define CMP_INVVALCLASS \
  "INVVALCLASS W Invalid value: attribute %s is restricted to instances of %s"

#define CMP_INVATTRDEF \
  "INVATTRDEF W Invalid DEFAULT value: %s is required but %s is %s; DEFAULT ignored"

#define CMP_INVCOMPDEF \
  "INVCOMPDEF W Invalid DEFAULT value: each element is required to be %s; DEFAULT ignored"

#define CMP_INVATTRFIL \
  "INVATTRFIL W Invalid FILL value: %s is required but %s is %s; FILL ignored"

#define CMP_INVVALUE \
  "INVVALUE W Invalid value type: %s is required but %s is %s"

#define CMP_INVVALTYP \
  "INVVALTYP W Invalid value type: %s requires %s but %s is %s"
/* 
 * A value has been detected by the compiler that is of the wrong
 * type for the current use.  For example the parameter to the LENGTH
 * function must be a compound, a scalar argument to that function
 * will generate this error.
 *
 * The following constants are substituted for the 3rd operand in INVVALTYP
 * depending on the situation.
 */
#define	CMP__C_MSG_ARITH_OPERAND	"arithmetic operand"
#define CMP__C_MSG_ARGUMENT		"argument"
#define CMP__C_MSG_ATTR_INDEX		"attribute index"
#define CMP__C_MSG_ATTRIBUTE		"attribute"


#define CMP_INVCMPIDX \
  "INVCMPIDX W Compound index %s invalid; must be an integer > 0"

#define CMP_INVSUBSYMIDX \
  "INVSSYMIX W SUBSYMBOL index %s invalid; must be an integer > 0"

#define CMP_ZERMAXMIN \
  "ZERMAXMIN W Zero arguments to %s returns NIL"

/******************** rhs sem sql errors **********************/

#define CMP_INVATHPAR \
  "INVATHPAR W Invalid ATTACH parameter %s, must be PATHNAME, FILENAME or a constant"

#define CMP_INVSCOPAR \
  "INVSCOPAR W Invalid ATTACH SCOPE %s, must be a constant (ATTACH | TRANSACTION)"

#define CMP_INVFETPAR \
  "INVATTPAR W Invalid FETCH parameter %s, must be a constant or variable"

#define CMP_INVPATPAR \
  "INVPATPAR W Invalid ATTACH PATHNAME %s, must be a constant"

#define CMP_INVTBLPAR \
  "INVTBLPAR W Invalid DB-TABLE %s, must be a constant"

#define CMP_INVRSEPAR \
  "INVRSEPAR W Invalid DB-RSE %s, must be a constant"

#define CMP_INVFTEPAR \
  "INVFTEPAR W Invalid FETCH-EACH parameter %s, must be a variable"

#define CMP_INVINSPAR \
  "INVINSPAR W Invalid INSERT-FROM-OBJECT parameter %s, must be a variable"

#define CMP_INVINSVAR \
  "INVINSVAR W Invalid INSERT-FROM-OBJECT variable %s, must be an ID"

#define CMP_INVUPDPAR \
  "INVUPDPAR W Invalid UPDATE-FROM-OBJECT parameter %s, must be a variable"

#define CMP_INVUPDVAR \
  "INVUPDVAR W Invalid UPDATE-FROM-OBJECT variable %s, must be an ID"


/**************************  IDX errors  **********************/

#define CMP_INXNOTFND \
"INXNOTFND E USE file %s for declaration block %s not found"
/*
 * There is no usefile for the "used" declaration block.  Declaration
 * Blocks must be compiled before compiling and entry block or rule
 * block that uses it.  Also the declaration block must be "used" from
 * the same directory it was compiled in
 */

#define CMP_INXOLDVER \
"INXOLDVER E USE file version for declaration block %s invalid; recompile the declaration block"
/*
 * Recompile the declaration block.
 */

#define CMP_INXBLKNOTFND \
"INXBLKNOTFND E Declaration block %s not found in USE file; first 8 characters of declaration block name must be unique"
/*
 * Usefile names are constructed from the first 8 characters of 
 * the declaration block name.  Verify that all declaration blocks
 * compiled in the same directory have unique initial 8 characters,
 * Verify that the declaration block has been compiled.
 */

#define CMP_INXINVFMT \
  "INXINVFMT E Invalid data format in USE file %s at line %ld"
/*
 * Although ASCII, the usefiles are not intended to be edited.
 * Editing an usefile will guarantee invalid code generation
 * that may or may not be detectable.  Don't do it.
 */

#define CMP_INXDUPBLK \
"INXDUPBLK W Overwriting USE file %s used by multiple declaration blocks; first 8 characters of declaration block name must be unique"
/*
 * The usefile name is constructed from the first 8 characters of
 * the declaration block name.  An usefile with the same name as the
 * current declaration block has been found that contains declarations
 * for a different declaration block.  That file is being replaced
 * causing future reference to the other declaration block to be undefined.
 * TO ensure unique use filenames the first 8 characters of all
 * declaration block names compiled in the same directory must be different.
 */

#define CMP_INXOPENERR \
  "INXOPENERR E Unable to open USE file %s for write"
/*
 * USEfile cannot be opened.  Check file protection, filename 
 * specification, file quotas, etc.
 */

/**************************  ON errors  **********************/

#define CMP_ONAFTRUL \
  "ONAFTRUL W %s follows a RULE (it must precede all RULEs); construct ignored"

#define CMP_OBSSTARTUP \
  "OBSSTARTUP W STARTUP is obsolescent; use ON-ENTRY instead"

#define CMP_ONBADBLK \
  "ONBADBLK E %s is not enclosed within the scope of an ENTRY block"

#define CMP_ONDUPCLS \
  "ONDUPCLS W %s previously defined within this ENTRY block; construct ignored"



#define CMP_NOCODEGEN \
    "NOCODEGEN I No code generated for construct with errors\n"

#define CMP_INVCONIGN \
    "INVCONIGN I Improperly formed construct ignored\n"

#define CMP_NOOUTPUT \
  "NOOUTPUT I %ld errors encountered; no output file produced"

/*********************** CMP CLI ******************************/

#define CMP_CLIDUPINP \
  "CLIDUPINP E Duplicate input file arguments, %s"

#define CMP_CLIBADNAM \
  "CLIBADNAM E Option name \\%s\\ unrecognized"

#define CMP_CLIBADVAL \
  "CLIBADVAL W Option value \\%s\\ unrecognized"

#define CMP_CLIUNEXOPT \
  "CMPUNEXOPT W Unexpected option value \\%s\\, ignored"

/************************* method specific messages *********************/

#define CMP_INVMETCLS \
  "INVMETCLS W Invalid class %s"

#define CMP_CNFMETDEF \
  "CNFMETDEF W Conflicting class definition %s"

#define CMP_INVMPRCNT \
  "INVMPRCNT W Invalid parameter count"

#define CMP_METDUPPAR \
  "METDUPPAR W Duplicately named parameter %s"

#define CMP_METSHPPAR \
  "METSHPPAR W Invalid shape specified for parameter %s"

#define CMP_METDOMPAR \
  "METDOMPAR W Invalid domain specified for parameter %s"

#define CMP_METCLSPAR \
  "METCLSPAR W Invalid class specified for parameter %s"

#define CMP_METPARCLS \
  "METPARCLS W Attempt to restrict domain of parameter %s to unknown class, %s; restriction ignored"

#define CMP_METPARCNT \
  "METPARCNT W Invalid parameter count in method %s call"

#define CMP_NOCLSMET \
  "NOCLSMET W Method %s not defined for class %s"

#define CMP_INVCALINH \
  "INVCALINH W Invalid CALL-INHERITED usage, only allowed within a method"

#define CMP_NOINHCLS \
  "NOINHCLS W No inheritable classes for class %s"

#define CMP_NOINHMET \
  "NOINHMET W No inherited methods for class %s"

#define CMP_GENMETIGN \
  "GENMETIGN I Generic system method %s definition in block %s ignored"

#define CMP_INVMETNPAR \
  "INVMETNPAR W Invalid method parameter %s, must be a variable bound to ^$ID"

#define CMP_INVMETPAR \
 "INVMETPAR W Invalid method class parameter, must be a variable bound to ^$ID"

