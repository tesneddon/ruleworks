/****************************************************************************
**                                                                         **
**                    C M P _ A S T _ T Y P E S . C                        **
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
 *	This module provides the mapping from AST node types into strings
 *	for printing parse trees.
 *
 * MODIFIED BY:
 *	DEC	Digital Equipment Corporation
 *	CPQ	Compaq Computer Corporation
 *
 * MODIFICATION HISTORY:
 *
 *	8-Dec-1992	DEC	Initial version
 *	1-Dec-1999	CPQ	Release with GPL
 */

#include <common.h>
#include <cmp_comm.h>
#include <ast.h>



/********************************
 **                            **
 **  RUL___AST_PRED_TO_SYMBOL  **
 **                            **
 *******************************/

char *rul__ast_pred_to_symbol (Pred_Type predicate)
     /* Return symbol form of values for AST__E_PRED_TEST. */
{
   static char *pred_strings[] = {
      " ? ",
      "",
      "=",
      "~=",
      "-~=",
      "<=>",
      "<->",
      "<>",
      "-=",
      "<",
      "<=",
      ">",
      ">=",
      "[+]",
      "[-]",
      "[<=]",
      "[<>]",
      "[<]",
      "[=]",
      "[>=]",
      "[>]",
      "[]"
     };
   
   if ((int) predicate < 22)
     return pred_strings[predicate];
   else
     return " ? ";
}


/*************************************
**                                  **
**  RUL___AST_VALUE_TYPE_TO_STRING  **
**                                  **
*************************************/

char *rul___ast_value_type_to_string (Ast_Value_Type type)
{
   register char *string = "? Unknown Ast_Value_Type ?";

   switch (type) {
    case  AST__E_TYPE_NULL:
      string = "No Value";
      break;

    case  AST__E_TYPE_MOL:
      string = "Molecule";
      break;

    case  AST__E_TYPE_OPAQUE:
      string = "Opaque";
      break;

    case  AST__E_TYPE_PRED:
      string = "Predicate";
      break;

    case  AST__E_TYPE_NET_NODE:
      string = "Net_Node";
      break;

    case  AST__E_TYPE_NET_TEST:
      string = "Net_Test";
      break;

    case  AST__E_TYPE_VALUE:
      string = "Value";
      break;

    case  AST__E_TYPE_ASCIZ:
      string = "char*";
      break;

    case  AST__E_TYPE_EXT_TYPE:
      string = "Ext_Type";
      break;

    case  AST__E_TYPE_EXT_MECH:
      string = "Ext_Mech";
      break;

    case  AST__E_TYPE_CARDINALITY:
      string = "Cardinality";
      break;

    case  AST__E_TYPE_STRATEGY:
      string = "Strategy";
      break;
   }
   
   return string;
}


/*******************************
**                            **
**  RUL___AST_TYPE_TO_STRING  **
**                            **
*******************************/

char *rul___ast_type_to_string (Ast_Node_Type type)
{
   register char *string = "? Unknown Ast_Node_Type ?";

   switch (type) {

    case  AST__E_INVALID:
      string = "*INVALID*";
      break;
      
    case  AST__E_ENTRY_BLOCK:
      string = "ENTRY-BLOCK";
      break;
      
    case  AST__E_DECL_BLOCK:
      string = "DECLARATION-BLOCK";
      break;
      
    case  AST__E_RULE_BLOCK:
      string = "RULE-BLOCK";
      break;
      
    case  AST__E_END_BLOCK:
      string = "END-BLOCK";
      break;
      
    case  AST__E_OBJ_CLASS:
      string = "OBJECT-CLASS";
      break;
      
    case  AST__E_EXT_RT_DECL:
      string = "EXTERNAL-ROUTINE";
      break;
      
    case  AST__E_METHOD:
      string = "METHOD";
      break;
      
    case  AST__E_GENERIC_METHOD:
      string = "GENERIC-METHOD";
      break;
      
    case  AST__E_RULE:
      string = "RULE";
      break;
      
    case  AST__E_CATCH:
      string = "CATCH";
      break;
      
    case  AST__E_RULE_GROUP:
      string = "RULE-GROUP";
      break;
      
    case  AST__E_END_GROUP:
      string = "END-GROUP";
      break;
      
    case  AST__E_ON_ENTRY:
      string = "ON-ENTRY";
      break;
      
    case  AST__E_ON_EVERY:
      string = "ON-EVERY";
      break;
      
    case  AST__E_ON_EMPTY:
      string = "ON-EMPTY";
      break;
      
    case  AST__E_ON_EXIT:
      string = "ON-EXIT";
      break;
      
    case  AST__E_USES:
      string = "USES";
      break;
      
    case  AST__E_ACTIVATES:
      string = "ACTIVATES";
      break;
      
    case  AST__E_STRATEGY:
      string = "STRATEGY";
      break;
      
    case  AST__E_RULE_LHS:
      string = "RULE-LHS";
      break;
      
    case  AST__E_RULE_RHS:
      string = "RULE-RHS";
      break;
      
    case  AST__E_METHOD_RHS:
      string = "METHOD-RHS";
      break;
      
    case  AST__E_POS_CE:
      string = "POS-CE";
      break;
      
    case  AST__E_NEG_CE:
      string = "NEG-CE";
      break;
      
    case  AST__E_CE_DISJ:
      string = "CE-DISJ";
      break;
      
    case  AST__E_CE_NEG_CONJ:
      string = "CE-NEG-CONJ";
      break;
      
    case  AST__E_CE:
      string = "CE";
      break;
      
    case  AST__E_ATTR_TEST:
      string = "ATTR-TEST";
      break;
      
    case  AST__E_ATTR_EXPR:
      string = "ATTR-EXPR";
      break;
      
    case  AST__E_ATTR_INDEX:
      string = "ATTR-INDEX";
      break;
      
    case  AST__E_PRED_TEST:
      string = "PRED-TEST";
      break;
      
    case  AST__E_LHS_VAR_BIND:
      string = "LHS-VAR-BIND";
      break;
      
    case  AST__E_VALUE_EXPR:
      string = "VALUE-EXPR";
      break;
      
    case  AST__E_VALUE_DISJ:
      string = "VALUE-DISJ";
      break;
      
    case  AST__E_CONSTANT:
      string = "CONSTANT";
      break;
      
    case  AST__E_VARIABLE:
      string = "VARIABLE";
      break;
      
    case  AST__E_QUOTED_VAR:
      string = "QUOTED-VAR";
      break;
      
    case  AST__E_DOLLAR_LAST:
      string = "$LAST";
      break;
      
    case  AST__E_COND_EXPR:
      string = "COND-EXPR";
      break;
      
    case  AST__E_ARITH_EXPR:
      string = "ARITH-EXPR";
      break;
      
    case  AST__E_OPR_PLUS:
      string = "Plus (+)";
      break;
      
    case  AST__E_OPR_MINUS:
      string = "Minus (-)";
      break;
      
    case  AST__E_OPR_TIMES:
      string = "Times (*)";
      break;
      
    case  AST__E_OPR_DIVIDE:
      string = "Divide (/)";
      break;
      
    case  AST__E_OPR_MODULUS:
      string = "Modulus (\\)";
      break;
      
    case  AST__E_OPR_UMINUS:
      string = "Unary-minus (-)";
      break;
      
    case  AST__E_OPR_AND:
      string = "AND";
      break;
      
    case  AST__E_OPR_NOT:
      string = "NOT";
      break;
      
    case  AST__E_OPR_OR:
      string = "OR";
      break;
      
    case  AST__E_OBJ_ATTRS:
      string = "OBJ-ATTRS";
      break;
      
    case  AST__E_OBJ_CLASS_INH:
      string = "OBJ-CLASS-INH";
      break;
      
    case  AST__E_ATTRIBUTE:
      string = "ATTRIBUTE";
      break;
      
    case  AST__E_ATTR_COMPOUND:
      string = "ATTR-COMPOUND";
      break;
      
    case  AST__E_ATTR_DOMAIN:
      string = "ATTR-DOMAIN";
      break;
      
    case  AST__E_ATTR_CLASS:
      string = "ATTR-CLASS";
      break;
      
    case  AST__E_ATTR_DEFAULT:
      string = "ATTR-DEFAULT";
      break;
      
    case  AST__E_ATTR_COMPOUND_DEFAULT:
      string = "ATTR-COMPOUND-DEFAULT";
      break;
      
    case  AST__E_ATTR_COMPOUND_FILL:
      string = "ATTR-COMPOUND-FILL";
      break;
      
    case  AST__E_EXT_RT_CALL:
      string = "EXTERNAL-ROUTINE-CALL";
      break;
      
    case  AST__E_ALIAS_DECL:
      string = "ALIAS-DECL";
      break;
      
    case  AST__E_ACCEPTS_DECL:
      string = "ACCEPTS-DECL";
      break;
      
    case  AST__E_ACCEPTS_PARAM:
      string = "ACCEPTS-PARAM";
      break;
      
    case  AST__E_METHOD_PARAM:
      string = "METHOD-PARAM";
      break;
      
    case  AST__E_METHOD_WHEN:
      string = "METHOD-WHEN";
      break;
      
    case  AST__E_RETURNS_DECL:
      string = "RETURNS-DECL";
      break;
      
    case  AST__E_EXT_TYPE:
      string = "EXT-TYPE";
      break;
      
    case  AST__E_EXT_LEN:
      string = "EXT-LEN";
      break;
      
    case  AST__E_EXT_MECH:
      string = "EXT-MECH";
      break;
      
    case  AST__E_RHS_ACTION_TERM:
      string = "RHS-ACTION-TERM";
      break;
      
    case  AST__E_ADDSTATE_ACTION:
      string = "ADDSTATE";
      break;
      
    case  AST__E_AFTER_ACTION:
      string = "AFTER";
      break;
      
    case  AST__E_AT_ACTION:
      string = "AT (@)";
      break;
      
    case  AST__E_BIND_ACTION:
      string = "BIND";
      break;
      
    case  AST__E_BUILD_ACTION:
      string = "BUILD";
      break;
      
    case  AST__E_CALL_INHERITED:
      string = "CALL-INHERITED";
      break;
      
    case  AST__E_CLOSEFILE_ACTION:
      string = "CLOSEFILE";
      break;
      
    case  AST__E_COPY_ACTION:
      string = "COPY";
      break;
      
    case  AST__E_DEBUG_ACTION:
      string = "DEBUG";
      break;
      
    case  AST__E_DEFAULT_ACTION:
      string = "DEFAULT";
      break;
      
    case  AST__E_FOR_EACH_ACTION:
      string = "FOR-EACH";
      break;
      
    case  AST__E_IF_ACTION:
      string = "IF";
      break;
      
    case  AST__E_IF_ELSE:
      string = "IF-ELSE";
      break;
      
    case  AST__E_IF_THEN:
      string = "IF-THEN";
      break;
      
    case  AST__E_MAKE_ACTION:
      string = "MAKE";
      break;
      
    case  AST__E_MODIFY_ACTION:
      string = "MODIFY";
      break;
      
    case  AST__E_OPENFILE_ACTION:
      string = "OPENFILE";
      break;
      
    case  AST__E_QUIT_ACTION:
      string = "QUIT";
      break;
      
    case  AST__E_REMOVE_ACTION:
      string = "REMOVE";
      break;
      
    case  AST__E_REMOVE_EVERY_ACTION:
      string = "REMOVE-EVERY";
      break;
      
    case  AST__E_RESTORESTATE_ACTION:
      string = "RESTORESTATE";
      break;
      
    case  AST__E_RETURN_ACTION:
      string = "RETURN";
      break;
      
    case  AST__E_SAVESTATE_ACTION:
      string = "SAVESTATE";
      break;
      
    case  AST__E_SPECIALIZE_ACTION:
      string = "SPECIALIZE";
      break;
      
    case  AST__E_TRACE_ACTION:
      string = "TRACE";
      break;
      
    case  AST__E_WHILE_ACTION:
      string = "WHILE";
      break;
      
    case  AST__E_WHILE_DO:
      string = "DO";
      break;
      
    case  AST__E_WRITE_ACTION:
      string = "WRITE";
      break;
      
    case  AST__E_SQL_ATTACH_ACTION:
      string = "SQL-ATTACH";
      break;
      
    case  AST__E_SQL_COMMIT_ACTION:
      string = "SQL-COMMIT";
      break;
      
    case  AST__E_SQL_DELETE_ACTION:
      string = "SQL-DELETE";
      break;
      
    case  AST__E_SQL_DETACH_ACTION:
      string = "SQL-DETACH";
      break;
      
    case  AST__E_SQL_FETCH_EACH_ACTION:
      string = "SQL-FETCH-EACH";
      break;
      
    case  AST__E_SQL_FETCH_AS_OBJECT_ACTION:
      string = "SQL-FETCH-AS-OBJECT";
      break;
      
    case  AST__E_SQL_INSERT_ACTION:
      string = "SQL-INSERT";
      break;
      
    case  AST__E_SQL_INSERT_FROM_OBJECT_ACTION:
      string = "SQL-INSERT-FROM-OBJECT";
      break;
      
    case  AST__E_SQL_ROLLBACK_ACTION:
      string = "SQL-ROLLBACK";
      break;
      
    case  AST__E_SQL_START_ACTION:
      string = "SQL-START";
      break;
      
    case  AST__E_SQL_UPDATE_ACTION:
      string = "SQL-UPDATE";
      break;
      
    case  AST__E_SQL_UPDATE_FROM_OBJECT_ACTION:
      string = "SQL-UPDATE-FROM-OBJECT";
      break;
      
    case  AST__E_SQL_PATHNAME:
      string = "SQL-PATHNAME";
      break;
      
    case  AST__E_SQL_RSE:
      string = "SQL-RSE";
      break;
      
    case  AST__E_SQL_VARS:
      string = "SQL-VARS";
      break;
      
    case  AST__E_SQL_ACTIONS:
      string = "SQL-ACTIONS";
      break;
      
    case  AST__E_BTI_RT_CALL:
      string = "ROUTINE-CALL";
      break;
      
    case  AST__E_BTI_RHS_CALL:
      string = "RHS-CALL";
      break;
      
    case  AST__E_BTI_ACCEPT_ATOM:
      string = "ACCEPT-ATOM";
      break;
      
    case  AST__E_BTI_ACCEPTLINE_COMPOUND:
      string = "ACCEPTLINE-COMPOUND";
      break;
      
    case  AST__E_BTI_COMPOUND:
      string = "COMPOUND";
      break;
      
    case  AST__E_BTI_CONCAT:
      string = "CONCAT";
      break;
      
    case  AST__E_BTI_CRLF:
      string = "CRLF";
      break;
      
    case  AST__E_BTI_EVERY:
      string = "EVERY";
      break;
      
    case  AST__E_BTI_FLOAT:
      string = "FLOAT";
      break;
      
    case  AST__E_BTI_GENATOM:
      string = "GENATOM";
      break;
      
    case  AST__E_BTI_GENINT:
      string = "GENINT";
      break;
      
    case  AST__E_BTI_GET:
      string = "GET";
      break;
      
    case  AST__E_BTI_INTEGER:
      string = "INTEGER";
      break;
      
    case  AST__E_BTI_IS_OPEN:
      string = "IS-OPEN";
      break;
      
    case  AST__E_BTI_LENGTH:
      string = "LENGTH";
      break;
      
    case  AST__E_BTI_MAX:
      string = "MAX";
      break;
      
    case  AST__E_BTI_MIN:
      string = "MIN";
      break;
      
    case  AST__E_BTI_NTH:
      string = "NTH";
      break;
      
    case  AST__E_BTI_POSITION:
      string = "POSITION";
      break;
      
    case  AST__E_BTI_RJUST:
      string = "RJUST";
      break;
      
    case  AST__E_BTI_SUBCOMPOUND:
      string = "SUBCOMPOUND";
      break;
      
    case  AST__E_BTI_SUBSYMBOL:
      string = "SUBSYMBOL";
      break;
      
    case  AST__E_BTI_SYMBOL:
      string = "SYMBOL";
      break;
      
    case  AST__E_BTI_SYMBOL_LENGTH:
      string = "SYMBOL-LENGTH";
      break;
      
    case  AST__E_BTI_TABTO:
      string = "TABTO";
      break;
      
    default:
      string = "!!! UnCased AST TYPE in 'type_to_string'!!!";
      break;
   }
   return string;
}




/******************************
 **                           **
 **  RUL__AST_PRED_TO_STRING  **
 **                           **
 ******************************/

char *rul__ast_pred_to_string (Pred_Type predicate)
     /* Return string form of values for AST__E_PRED_TEST. */
{
   static char *pred_strings[] = {
      "? Invalid predicate ?",
      "EQ",
      "EQUAL",
      "APPROX_EQUAL",
      "NOT_APPROX_EQUAL",
      "SAME_TYPE",
      "DIFF_TYPE",
      "NOT_EQ",
      "NOT_EQUAL",
      "LESS",
      "LESS_EQUAL",
      "GREATER",
      "GREATER_EQUAL",
      "CONTAINS",
      "DOES_NOT_CONTAIN",
      "LENGTH_LESS_EQUAL",
      "LENGTH_NOT_EQUAL",
      "LENGTH_LESS",
      "LENGTH_EQUAL",
      "LENGTH_GREATER_EQUAL",
      "LENGTH_GREATER",
      "INDEX_VALID"
     };
   
   if ((int) predicate < 22)
     return pred_strings[predicate];
   else
     return "? Unknown predicate ?";
}

