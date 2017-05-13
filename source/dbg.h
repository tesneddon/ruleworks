/****************************************************************************
**                                                                         **
**                             D B G . H      

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



#ifndef IN_RTS_RAC

/* common references for rul_debug (interpreter) */

EXTERNAL long    (*rul__dbg_ga_interp)(Entry_Data eb_data);
EXTERNAL Boolean (*rul__dbg_ga_ebreak)(Mol_Symbol ebname);
EXTERNAL Boolean (*rul__dbg_ga_wbreak)(Object wmo);
EXTERNAL Boolean (*rul__dbg_ga_rbreak)(Conflict_Set_Entry cse);
EXTERNAL long       rul__dbg_gl_break;
EXTERNAL long       rul__dbg_gl_ebreak;
EXTERNAL long       rul__dbg_gl_trace;
EXTERNAL long       rul__dbg_gl_enable;
EXTERNAL Mol_Symbol rul__dbg_ga_group;
EXTERNAL long       rul__rac_rule_firing_cycle;
EXTERNAL Mol_Symbol rul__rac_active_rule_name;

#endif



#define DBG_M_ENABLE_WMH   1
#define DBG_M_ENABLE_CRC   2
#define DBG_M_ENABLE_TIM   4
#define DBG_M_ENABLE_REP   8
#define DBG_M_ENABLE_DBG   16
#define DBG_M_ENABLE_TRA   32
#define DBG_M_ENABLE_BKN   64

#define DBG_M_BREAK        1
#define DBG_M_BREAK_RUN    2
#define DBG_M_BREAK_WMO    4
#define DBG_M_BREAK_RULE   8
#define DBG_M_BREAK_RG     16
#define DBG_M_BREAK_ENTRY  32
#define DBG_M_BREAK_EXIT   64
#define DBG_M_BREAK_EVERY  128
#define DBG_M_BREAK_EMPTY  256
#define DBG_M_BREAK_CRC    1024
#define DBG_M_BREAK_API    2048		/* DEBUG action or rul_debug API */

#define DBG_M_EBREAK_ALL   1
#define DBG_M_EBREAK_NAME  2

#define DBG_M_TRACE_WM    1
#define DBG_M_TRACE_CS    2
#define DBG_M_TRACE_PM    4
#define DBG_M_TRACE_R     8
#define DBG_M_TRACE_RG    16
#define DBG_M_TRACE_EB    32
#define DBG_M_TRACE_ALL   255
                           /*(DBG_M_TRACE_WM || DBG_M_TRACE_CS || \
			     DBG_M_TRACE_PM || DBG_M_TRACE_R  || \
			     DBG_M_TRACE_RG || DBG_M_TRACE_EB) */

