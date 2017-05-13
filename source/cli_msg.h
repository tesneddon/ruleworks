/* cli_msg.h - Command Language Interpreter (Debugger) Message definitions */
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
 *	Also defined in this file are the simple text strings that
 *	are used at various points in the command interpreter.  All
 *	the strings are prefixed with CLI_STR_...
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
 *	01-Dec-1999	CPQ	Releasew ith GPL
 */

/* Command Language Interpreter (RULEWORKS debugger) */

#define CLI_BACK \
  "BACK I Backing up %s cycles (NYI)"

#define CLI_BREAKNOTED \
  "BREAKNOTED I Execution pause by break"

#define CLI_BUILD \
  "BUILD I Building rule %s (NYI)"

#define CLI_CLOSEFILE \
  "CLOSEFILE I Closing file %s"

#define CLI_DISABLE \
  "DISABLE I Disabled %s"

#define CLI_ENABLE \
  "ENABLE I Enabled %s"

#define CLI_EXCISE \
  "EXCISE I Excising rule %s (NYI)"

#define CLI_IDXZERO \
  "IDXZERO W Attribute index of 0 ignored"

#define CLI_INVATTSYN \
  "INVATTUSG E Invalid syntax, attribute %s not compound"

#define CLI_INVBLKNAM \
  "INVBLKNAM E Invalid declaration block %s"

#define CLI_INVCATCH \
  "INVCATCH W Invalid catcher %s"

#define CLI_INVCLSATT \
  "INVCLSATT E Attribute %s not in class %s"

#define CLI_INVBLKCLS \
  "INVCLSNAM E Invalid class %s, not contained in block %s"

#define CLI_INVCLSNAM \
  "INVCLSNAM E Invalid class %s"

#define CLI_AMBCLSNAM \
  "AMBCLSNAM E Ambiguous class %s"

#define CLI_INVWMO \
  "INVWMO E Invalid WMO #%ld for %s"

#define CLI_NOSUCHWMO \
  "NOSUCHWMO E No such WMO %s"

#define CLI_INVIDXPRD \
  "INVIDXPRD E Invalid index and predicate combination"

#define CLI_INVPRDUSG \
  "INVPRDUSG E Invalid predicate usage for COMPOUND"

#define CLI_INVRUNCNF \
  "INVRUNCNT W Invalid run count %f, must be integer"

#define CLI_INVRUNCNT \
  "INVRUNCNT W Invalid run count %s, must be integer"

#define CLI_INVSPECLS \
  "INVSPECLS E Invalid WMO class, %s not subclass of %s"

#define CLI_INVWMOID \
  "INVWMOID E Invalid WMO ID, #%d"

#define CLI_NOENABLE \
  "NOENABLE I Enable %s not implemented"

#define CLI_PAUSE \
  "PAUSE I Pausing after running requested number of rules"

#define CLI_DEBPAUSE \
  "DEBPAUSE I Pausing after DEBUG action or call to rul_debug"

#define CLI_EBREAK \
  "EBREAK I EBREAK encountered; pausing after %s in %s"

#define CLI_RBREAK \
  "RBREAK I RBREAK encountered on rule %s"

#define CLI_REMTIMTAG \
  "REMTIMTAG W REMOVE <timetag> not supported, use #ID"

#define CLI_SETCATCH \
  "SETCATCH I Setting catcher %s after %s cycles"

#define CLI_UNMPAREN \
  "UNMPAREN E Mismatched parentheses"

#define CLI_WBREAK \
  "WBREAK I WBREAK encountered %s"

#define CLI_WMHTIMTAG \
  "WMHTIMTAG W WMHISTORY <timetag> not supported, use #ID"

#define CLI_WMTIMTAG \
  "WMTIMTAG W WM <timetag> not supported, use #ID"

#define CLI_INVMAKID \
  "INVMAKID W Invalid ^$ID specification, translation not started"

#define CLI_INVMAKTID \
  "INVMAKTID W Invalid ^$ID specification, translated id exists"

#define CLI_INVVALTYP \
  "INVVALTYP E Invalid value, %s, for attribute %s"

#define CLI_INVWMENLP \
  "INVWMENLP E Invalid WMO string, missing '('"

#define CLI_INVWMENTH \
  "INVWMENTH E Invalid WMO string, missing '^'"

#define CLI_INVWMEATT \
  "INVWMEATT E Invalid WMO symbol %s, expecting attribute"

#define CLI_INVWMEEOF \
  "INVWMEEOF E Invalid WMO, unexpected EOF encountered"

#define CLI_SYNTAXERR \
    "SYNTAXERR E Syntax error: %s%s%s"

#define CLI_UNEXPEOF \
  "UNEXPEOF E Unexpected end of file%s"

#define CLI_RETVALREQ \
  "RETVALREQ W Return value required"

#define CLI_RETNOTREQ \
  "RETNOTREQ I No return value required, ignored"

#define CLI_RETWRGTYP \
  "RETWRGTYP W Return value is wrong type, expecting %s"

#define CLI_UNMRPAREN \
  "UNMRPAREN W Unexpected right parenthesis ')' ignored"



#define CLI_USAGE_AT  "\
   Usage:     @ filespec\n"
#define CLI_HINT_AT  "\
   Hint:      Specified file must contain only RuleWorks commands.\n"
#define CLI_EXAMP_AT  "\
   Example:   RuleWorks>  @ init.wm\n"


#define CLI_USAGE_ADDSTATE  "\
   Usage:     ADDSTATE  filespec\n"
#define CLI_HINT_ADDSTATE  "\
   Hint:      File must have been created by SAVESTATE action or command.\n"
#define CLI_EXAMP_ADDSTATE  "\
   Example:   RuleWorks>  addstate config.dat\n"


#define CLI_USAGE_AFTER  "\
   Usage:     AFTER  cycles catcher-name\n"
#define CLI_HINT_AFTER  "\
   Hints:     Cycles specifies local firing count, not global;\n\
              catcher must be defined in active entry block.\n"
#define CLI_EXAMP_AFTER  "\
   Example:   RuleWorks>  after 100 too-many-cycles\n"


#define CLI_USAGE_CLOSEFILE  "\
   Usage:     CLOSEFILE  file-id...\n"
#define CLI_HINT_CLOSEFILE  "\
   Hint:      File-id must have been defined by OPENFILE action or command.\n"
#define CLI_EXAMP_CLOSEFILE  "\
   Example:   RuleWorks>  closefile infil outfil\n"


#define CLI_USAGE_COPY  "\
   Usage:     COPY  instance [ {^attribute value}... ]\n"
#define CLI_HINT_COPY  "\
   Hints:     Instance must be constant of type INSTANCE-ID;\n\
              value may not use any expression except COMPOUND function.\n"
#define CLI_EXAMP_COPY  "\
   Example:   RuleWorks>  copy #89 ^list (compound a b c)\n"


#define CLI_USAGE_CS  "\
   Usage:     CS\n"
#define CLI_HINT_CS  "\
   Hint:      No arguments allowed.\n"
#define CLI_EXAMP_CS  "\
   Example:   RuleWorks>  cs\n"


#define CLI_USAGE_DEFAULT  "\
   Usage:     DEFAULT  file-id  {ACCEPT | TRACE | WRITE}\n"
#define CLI_HINT_DEFAULT  "\
   Hints:     File-id must have been defined by OPENFILE action or command.\n\
              Use NIL instead of file-id to mean keyboard or screen\n\
              (standard input or output).\n"
#define CLI_EXAMP_DEFAULT  "\
   Example:   RuleWorks>  default debugfil trace\n"


#define CLI_USAGE_DISABLE  "\
   Usage:     DISABLE  {BLOCK-NAMES | WARNING | WMHISTORY}\n"
#define CLI_HINT_DISABLE  "\
   Hint:      Arguments may be abbreviated to three letters.\n"
#define CLI_EXAMP_DISABLE  "\
   Example:   RuleWorks>  disable war\n"


#define CLI_USAGE_EBREAK  "\
   Usage:     EBREAK  [ON {eblock | *} | OFF {{eblock | number}... | *}]\n"
#define CLI_HINT_EBREAK  "\
   Hint:      Give command with no arguments to see a numbered list \n\
              of current ebreaks.\n"
#define CLI_EXAMP_EBREAK  "\
   Examples:  RuleWorks>  ebreak on my-rule-block\n\
              RuleWorks>  ebreak off 3\n"


#define CLI_USAGE_ENABLE  "\
   Usage:     ENABLE  {BLOCK-NAMES | WARNING | WMHISTORY}\n"
#define CLI_HINT_ENABLE  "\
   Hint:      Arguments may be abbreviated to three letters.\n"
#define CLI_EXAMP_ENABLE  "\
   Example:   RuleWorks>  enable wmh\n"


#define CLI_USAGE_EXIT  "\
   Usage:     EXIT\n"
#define CLI_HINT_EXIT  "\
   Hint:      No arguments allowed; returns immediately to operating system.\n"
#define CLI_EXAMP_EXIT  "\
   Example:   RuleWorks>  exit\n"


#define CLI_USAGE_MAKE  "\
   Usage:     MAKE  class-name [ {^attribute value}... ]\n"
#define CLI_HINT_MAKE  "\
   Hints:     Class must be visible to active entry block;\n\
              value may not use any expression except COMPOUND function.\n"
#define CLI_EXAMP_MAKE  "\
   Example:   RuleWorks>  make my-part ^number 801 ^color-list[3] yellow\n"


#define CLI_USAGE_MATCHES  "\
   Usage:     MATCHES  rule-name...\n"
#define CLI_HINT_MATCHES  "\
   Hint:      Rule(s) must be visible to active entry block\n"
#define CLI_EXAMP_MATCHES  "\
   Example:   RuleWorks>  matches main~a-rule my-block~a-rule\n" 


#define CLI_USAGE_MODIFY  "\
   Usage:     MODIFY  instance [ {^attribute value}... ]\n"
#define CLI_HINT_MODIFY  "\
   Hints:     Instance must be constant of type INSTANCE-ID; \n\
              value may not use any expression except COMPOUND function.\n"
#define CLI_EXAMP_MODIFY  "\
   Example:   RuleWorks>  modify #77 ^color-list[3] green\n"


#define CLI_USAGE_NEXT  "\
   Usage:     NEXT\n"
#define CLI_HINT_NEXT  "\
   Hint:      No arguments allowed.\n"
#define CLI_EXAMP_NEXT  "\
   Example:   RuleWorks>  next\n"


#define CLI_USAGE_OPENFILE  "\
   Usage:     OPENFILE  file-id filespec {IN | OUT | APPEND}\n"
#define CLI_HINT_OPENFILE  "\
   Hint:      You must use OUT or APPEND to open a file used\n\
              for TRACE or WRITE output.\n"
#define CLI_EXAMP_OPENFILE  "\
   Examples:  RuleWorks>  openfile debugfil dbgrules.txt append\n\
              RuleWorks>  openfile infil orders.dat in\n"


#define CLI_USAGE_PPCLASS  "\
   Usage:     PPCLASS  [class-name]\n"
#define CLI_HINT_PPCLASS  "\
   Hint:      Give command with no arguments to see a hierarchy of \n\
              all the visible classes.\n"
#define CLI_EXAMP_PPCLASS  "\
   Example:   RuleWorks>  ppclass my-part\n"


#define CLI_USAGE_PPWM  "\
   Usage:     PPWM  [class-name]\n\
                 [^attr {value | pred value | << value... >>} ]...\n"
#define CLI_HINT_PPWM  "\
   Hints:     Give command with no arguments to see all visible objects.\n\
              Values must be scalar atoms; no expressions allowed.\n"
#define CLI_EXAMP_PPWM  "\
   Example:   RuleWorks>  ppwm my-part ^number << 801 805 809 >>\n"


#define CLI_USAGE_QUIT  "\
   Usage:     QUIT  [return-value | $SUCCESS | $FAILURE]\n"
#define CLI_HINT_QUIT  "\
   Hints:     Optional return value must be a constant;\n\
              returns to operating system.\n"
#define CLI_EXAMP_QUIT  "\
   Example:   RuleWorks>  quit $success\n"


#define CLI_USAGE_RBREAK  "\
   Usage:     RBREAK  [ON {rule | r-group}... |\n\
                       OFF {{rule | r-group | number}... | *}]\n"
#define CLI_HINT_RBREAK  "\
   Hints:     Rules and groups must be visible to active entry block.\n\
              Give command with no arguments to see a numbered list of\n\
              all current rbreaks.\n"
#define CLI_EXAMP_RBREAK  "\
   Example:   RuleWorks>  rbreak on main~my-rule my-block~my-rule\n"


#define CLI_USAGE_REMOVE  "\
   Usage:     REMOVE  {instance-id... | *}\n"
#define CLI_HINT_REMOVE  "\
   Hints:     Instance must be constant of type INSTANCE-ID; * removes\n\
              all visible objects.\n"
#define CLI_EXAMP_REMOVE  "\
   Example:   RuleWorks>  remove #312\n"


#define CLI_USAGE_REMOVE_EVERY  "\
   Usage:     REMOVE-EVERY  class-name\n"
#define CLI_HINT_REMOVE_EVERY  "\
   Hints:     Class must be visible to active entry block; use $ROOT to\n\
              remove all visible objects.\n"
#define CLI_EXAMP_REMOVE_EVERY  "\
   Example:   RuleWorks>  remove-every my-part\n"


#define CLI_USAGE_RESTORESTATE  "\
   Usage:     RESTORESTATE  filespec\n"
#define CLI_HINT_RESTORESTATE  "\
   Hint:      File must have been created by SAVESTATE action or command.\n"
#define CLI_EXAMP_RESTORESTATE  "\
   Example:   RuleWorks>  restorestate config.dat\n"


#define CLI_USAGE_RETURN  "\
   Usage:     RETURN  [value]\n"
#define CLI_HINT_RETURN  "\
   Hints:     Optional value may use COMPOUND function; returns to\n\
              caller, not necessarily operating system.\n"
#define CLI_EXAMP_RETURN  "\
   Example:   RuleWorks>  return 0\n"


#define CLI_USAGE_RUN  "\
   Usage:     RUN  [integer]\n"
#define CLI_HINT_RUN  "\
   Hint:      Optional argument specifies local rule firings, not global.\n"
#define CLI_EXAMP_RUN  "\
   Examples:  RuleWorks>  run 4\n"


#define CLI_USAGE_SAVESTATE  "\
   Usage:     SAVESTATE  filespec\n"
#define CLI_HINT_SAVESTATE  "\
   Hint:      Saves visible objects only.\n"
#define CLI_EXAMP_SAVESTATE  "\
   Example:   RuleWorks> savestate config.dat\n"


#define CLI_USAGE_SPECIALIZE  "\
   Usage:     SPECIALIZE  instance new-class-name [{^attribute value}... ]\n"
#define CLI_HINT_SPECIALIZE  "\
   Hints:     Instance must be constant of type INSTANCE-ID; value may\n\
              not use any expression except COMPOUND function.\n"
#define CLI_EXAMP_SPECIALIZE  "\
   Example:   RuleWorks>  specialize #201 shippable-part ^invoice true\n"


#define CLI_USAGE_TRACE  "\
   Usage:     TRACE  [ {ON | OFF} {trace-type... | *} ]\n\
                 where types are:  ENTRY-BLOCK, RULE, RULE-GROUP, WM, and CS\n"
#define CLI_HINT_TRACE  "\
   Hint:      ENTRY-BLOCK can be abbreviated as EB, and RULE-GROUP as RG\n"
#define CLI_EXAMP_TRACE  "\
   Examples:  RuleWorks>  trace off *\n\
              RuleWorks>  trace on rule wm\n"


#define CLI_USAGE_WBREAK  "\
   Usage:     WBREAK  [ON pattern | OFF {pattern... | number... | *}]\n"
#define CLI_HINT_WBREAK  "\
   Hints:     Patterns are similar to CEs, but may not include variables\n\
              or any functions except COMPOUND.  Give command with no\n\
              arguments to see numbered list of current wbreaks.\n"
#define CLI_EXAMP_WBREAK  "\
   Examples:  RuleWorks>  wbreak on my-part ^number\n"


#define CLI_USAGE_WM  "\
   Usage:     WM  [instance...]\n"
#define CLI_HINT_WM  "\
   Hint:      Optional argument must be a constant of type INSTANCE-ID.\n"
#define CLI_EXAMP_WM  "\
   Example:   RuleWorks>  wm #201 #312\n"


#define CLI_USAGE_WMHISTORY  "\
   Usage:     WMHISTORY  instance [^attribute]\n"
#define CLI_HINT_WMHISTORY  "\
   Hints:     You must ENABLE WMH and RUN some cycles before using this\n\
               command.  Required argument must be an INSTANCE-ID.\n"
#define CLI_EXAMP_WMHISTORY  "\
   Example:   RuleWorks>  wmh #117 ^number\n"



/*********************  Informal response strings  *********************/


#define CLI_STR_EMPTY_CS \
  "   The conflict set is empty\n"

#define CLI_STR_NO_NEXT \
  "   There is no next rule; the conflict set is empty\n"

#define CLI_STR_EBREAK_NONE		"   No EBREAKs set"
#define CLI_STR_EBREAK_HEADER		"   EBREAKs set on:"
#define CLI_STR_EBREAK_ALL		 "  * (all entry blocks)"

#define CLI_STR_WBREAK_HEADER		"   WBREAKs set on:\n"
#define CLI_STR_WBREAK_NONE		"   No WBREAKs set\n"

#define CLI_STR_RBREAK_HEADER		"   RBREAKs set on:\n"
#define CLI_STR_RBREAK_NONE		"   No RBREAKs set\n"


#define CLI_STR_NOMOREFIRINGS \
"   Note:  This entry block is about to exit; changes to\n\
          working memory cannot cause more rules to fire here.\n"

#define CLI_STR_NOSUCHBLOCK \
	"   There is no visible block named %s\n"
#define CLI_STR_NOSUCHRULE \
	"   There is no visible rule named %s\n"
#define CLI_STR_NOSUCHRULE2 \
	"   There is no visible rule named "
#define CLI_STR_AMBIGRULE \
	"   Ambiguous rule name, %s\n"


#define CLI_STR_DATA_TYP_INT		"integer"
#define CLI_STR_DATA_TYP_FLOAT		"float"
#define CLI_STR_DATA_TYP_OPAQUE		"opaque"
#define CLI_STR_DATA_TYP_SYMBOL		"symbol"
#define CLI_STR_DATA_TYP_ID		"instance-id"
#define CLI_STR_DATA_TYP_COMPOUND	"compound"
#define CLI_STR_DATA_TYP_TABLE		"table"

