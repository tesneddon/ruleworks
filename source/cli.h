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

typedef enum debug_values {
  CLI_DEBUG_NO,
  CLI_DEBUG_MAYBE,
  CLI_DEBUG_YES
} Debug_Values;

typedef enum names_values {
  CLI_NAMES_UPPER,
  CLI_NAMES_LOWER
} Names_Values;

typedef enum optimize_values {
  CLI_OPTIMIZE_SPACE,
  CLI_OPTIMIZE_REINVOCATION
} Optimize_Values;

Debug_Values rul__cli_debug_option (void);

Boolean rul__cli_listing_flag (void);

char *rul__cli_listing_option (void);

Boolean rul__cli_error_flag (void);

char *rul__cli_error_option (void);

Boolean rul__cli_usedir_flag (void);

char *rul__cli_usedir_option (void);

Names_Values rul__cli_names_option (void);

Optimize_Values rul__cli_optimize_option (void);

char *rul__cli_output_option (void);

char *rul__cli_input_option (void);

Boolean rul__cli_quiet_option (void);

Boolean
rul__cli_parse (int argc, char *argv[]);

#define CMP_HELP_MESSAGE_1 \
  "\nUsage:    rulework file [ option... ] \n"

#define CMP_HELP_MESSAGE_2 \
  "\nOptions:  -debug[={yes,maybe,no}]\n" \
  "          -error=file\n" \
  "          -optimize[={space,reinvocation}]\n" \
  "          -output=file\n" \
  "          -quiet\n" \
  "          -usedirectory=directory\n\n"


