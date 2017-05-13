$!++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
$!
$! RuleWorks - Rules based application development tool.
$!
$! Copyright (C) 1999  Compaq Computer Corporation
$!
$! This program is free software; you can redistribute it and/or modify it 
$! under the terms of the GNU General Public License as published by the 
$! Free Software Foundation; either version 2 of the License, or any later 
$! version. 
$!
$! This program is distributed in the hope that it will be useful, but 
$! WITHOUT ANY WARRANTY; without even the implied warranty of 
$! MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General 
$! Public License for more details. 
$!
$! You should have received a copy of the GNU General Public License along 
$! with this program; if not, write to the Free Software Foundation, 
$! Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
$!
$! Email: ruleworks@altavista.net
$!
$!
$!+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
$!
$!++
$! FACILITY:
$!       RULEWORKS
$!
$! ABSTRACT:
$!       Build script for the RULEWORKS product.
$!
$! USAGE:
$!
$! ENVIRONMENT:
$!
$! NOTE:
$!       Please try to keep each list of source files alphabetical.
$!
$! ***********
$! DEFINITIONS
$! ***********
$!
$!
$!	Environment.
$!
$ define RUL$SRC   [-.source]
$ define RUL$BLD   [-.build]
$ define RUL$BIN   [-.bin]
$ define RUL$EXM   [-.examples]
$ define RUL$INC   [-.include]
$ define RUL$LBR   [-.library]
$ libPath = f$parse( "[]") - "BUILD].;" + "BIN]"
$ define RUL$LIBRARY 'libPath
$ arch = f$getsyi("arch_name")
$ if (arch .eqs. "VAX") 
$ then
$	lib :== "rul_rtlv.olb"
$ else 
$	lib :== "rul_rtla.olb"
$ endif
$!
$ CFLAGS :== /INCLUDE=(RUL$SRC, RUL$INC)/PREFIX=ALL/DEFINE=(NDEBUG)

$ BOTH_OBJS_1 :==	 RUL$BLD:atomize.obj, RUL$BLD:btree.obj, -
			 RUL$BLD:decl_cls.obj, RUL$BLD:decl_dec.obj, -
			 RUL$BLD:decl_ent.obj, RUL$BLD:decl_ext.obj

$ BOTH_OBJS_2 :==	 RUL$BLD:decl_inh.obj, RUL$BLD:dyar.obj, -
			 RUL$BLD:hash.obj, -
			 RUL$BLD:ios_file.obj

$ BOTH_OBJS_3 :== 	 RUL$BLD:list.obj,     RUL$BLD:mem.obj, -      
			 RUL$BLD:ml_comps.obj, RUL$BLD:ml_hash.obj, -
			 RUL$BLD:ml_polys.obj, RUL$BLD:ml_print.obj

$ BOTH_OBJS_4 :==	 RUL$BLD:ml_query.obj, RUL$BLD:ml_table.obj, -
			 RUL$BLD:msg.obj, RUL$BLD:alloca.obj

$ CMP_OBJS_1 :==	 RUL$BLD:c_anodes.obj, RUL$BLD:c_atypes.obj, -
			 RUL$BLD:c_cli.obj, RUL$BLD:c_conrg.obj, -
			 RUL$BLD:c_cons.obj,   RUL$BLD:c_g_clas.obj

$ CMP_OBJS_2 :==	 RUL$BLD:c_g_cons.obj, RUL$BLD:c_g_ctch.obj, -
			 RUL$BLD:c_g_decl.obj, RUL$BLD:c_g_entr.obj, -
			 RUL$BLD:c_g_lvr.obj,  RUL$BLD:c_g_net.obj

$ CMP_OBJS_3 :==	 RUL$BLD:c_g_on.obj,   RUL$BLD:c_g_rhs.obj, -
			 RUL$BLD:c_g_rb.obj,   RUL$BLD:c_lvar.obj, -
			 RUL$BLD:c_main.obj, -
			 RUL$BLD:c_msg.obj,    RUL$BLD:c_nt_nod.obj

$ CMP_OBJS_4 :==	 RUL$BLD:c_s_gvar.obj, RUL$BLD:c_s_acts.obj, -
			 RUL$BLD:c_s_blks.obj, RUL$BLD:c_s_bnet.obj, -
			 RUL$BLD:c_s_bti.obj,  RUL$BLD:c_s_bval.obj

$ CMP_OBJS_5 :==	 RUL$BLD:c_s_decl.obj, RUL$BLD:c_s_ext.obj, -
			 RUL$BLD:c_s_on.obj,   RUL$BLD:c_s_rhs.obj, -
			 RUL$BLD:c_s_rule.obj, RUL$BLD:c_s_sql.obj

$ CMP_OBJS_6 :== 	 RUL$BLD:c_nt_tst.obj, RUL$BLD:c_parser.obj, -
			 RUL$BLD:c_scan.obj,   RUL$BLD:c_val.obj, -
			 RUL$BLD:c_s_cls.obj,  RUL$BLD:c_s_cons.obj

$ EMIT_OBJS  :==	 RUL$BLD:c_ec_int.obj, RUL$BLD:c_ec_val.obj, -
			 RUL$BLD:c_ec_wrp.obj

$ RTS_OBJS_1 :== 	 RUL$BLD:smg.obj, -
			 RUL$BLD:beta.obj,     RUL$BLD:cb_com.obj, -
			 RUL$BLD:cs.obj,       RUL$BLD:cvt_ext.obj, -
			 RUL$BLD:delta.obj,    RUL$BLD:i18n_str.obj

$ RTS_OBJS_2 :==	 RUL$BLD:ml_arith.obj, RUL$BLD:ml_preds.obj, -
			 RUL$BLD:rac.obj,      RUL$BLD:rbs.obj, -
			 RUL$BLD:ref.obj,      RUL$BLD:states.obj

$ RTS_OBJS_3 :==	 RUL$BLD:wm_iter.obj,  RUL$BLD:wm_print.obj, -
			 RUL$BLD:wm_query.obj, RUL$BLD:wm_updat.obj

$ DBG_OBJS   :==	 RUL$BLD:cmd_pars.obj, RUL$BLD:dbg.obj, -
			 RUL$BLD:rts_scan.obj

$ RTL_OBJS   :== RUL$BLD:cb_atom_vms.obj, RUL$BLD:cb_atom_c.obj, RUL$BLD:cb_atom_up.obj, -
				 RUL$BLD:cb_wmq_vms.obj, RUL$BLD:cb_wmq_c.obj, RUL$BLD:cb_wmq_up.obj, -
			 	 RUL$BLD:cb_wms_vms.obj, RUL$BLD:cb_wms_c.obj, RUL$BLD:cb_wms_up.obj 

$ HEADER_FILES	:==	RUL$SRC:common.h, RUL$SRC:decl.h, RUL$SRC:mol.h, -
			RUL$SRC:rbs.h, RUL$SRC:cs.h, RUL$SRC:rac.h, -
			RUL$SRC:ref.h, -
			RUL$SRC:beta.h, RUL$SRC:delta.h, RUL$SRC:wm.h, -
			RUL$SRC:ios.h, -
			RUL$SRC:cvt.h, RUL$SRC:sql.h, RUL$SRC:states.h


$ set verify

$ if (P1 .eqs. "HDR") then goto HDR

$! **********************
$! COMPILATION PROCEDURES
$! **********************
$!
$! COMMON
$!
$ cc 'CFLAGS /object=RUL$BLD:alloca.obj RUL$SRC:alloca.c
$ cc 'CFLAGS /object=RUL$BLD:atomize.obj RUL$BLD:atomize.c
$ cc 'CFLAGS /object=RUL$BLD:btree.obj RUL$SRC:btree.c
$ cc 'CFLAGS /object=RUL$BLD:decl_cls.obj RUL$SRC:decl_cls.c
$ cc 'CFLAGS /object=RUL$BLD:decl_dec.obj RUL$SRC:decl_dec.c
$ cc 'CFLAGS /object=RUL$BLD:decl_ent.obj RUL$SRC:decl_ent.c
$ cc 'CFLAGS /object=RUL$BLD:decl_ext.obj RUL$SRC:decl_ext.c
$ cc 'CFLAGS /object=RUL$BLD:decl_inh.obj RUL$SRC:decl_inh.c
$ cc 'CFLAGS /object=RUL$BLD:dyar.obj RUL$SRC:dyar.c
$ cc 'CFLAGS /object=RUL$BLD:hash.obj RUL$SRC:hash.c
$ cc 'CFLAGS /object=RUL$BLD:ios_file.obj RUL$SRC:ios_file.c
$ cc 'CFLAGS /object=RUL$BLD:list.obj RUL$SRC:list.c
$ cc 'CFLAGS /object=RUL$BLD:mem.obj RUL$SRC:mem.c
$ cc 'CFLAGS /object=RUL$BLD:ml_comps.obj RUL$SRC:ml_comps.c
$ cc 'CFLAGS /object=RUL$BLD:ml_hash.obj RUL$SRC:ml_hash.c
$ cc 'CFLAGS /object=RUL$BLD:ml_polys.obj RUL$SRC:ml_polys.c
$ cc 'CFLAGS /object=RUL$BLD:ml_print.obj RUL$SRC:ml_print.c
$ cc 'CFLAGS /object=RUL$BLD:ml_query.obj RUL$SRC:ml_query.c
$ cc 'CFLAGS /object=RUL$BLD:ml_table.obj RUL$SRC:ml_table.c
$ cc 'CFLAGS /object=RUL$BLD:msg.obj RUL$SRC:msg.c
$!
$ if (P1 .eqs. "RTL") then goto RTL
$!
$! COMPILER
$!
$ cc 'CFLAGS /object=RUL$BLD:c_anodes.obj RUL$SRC:c_anodes.c
$ cc 'CFLAGS /object=RUL$BLD:c_atypes.obj RUL$SRC:c_atypes.c
$ cc 'CFLAGS /object=RUL$BLD:c_cli.obj RUL$SRC:c_cli.c
$ cc 'CFLAGS /object=RUL$BLD:c_cons.obj RUL$SRC:c_cons.c
$ cc 'CFLAGS /object=RUL$BLD:c_conrg.obj RUL$SRC:c_conrg.c
$ cc 'CFLAGS /object=RUL$BLD:c_g_clas.obj RUL$SRC:c_g_clas.c
$ cc 'CFLAGS /object=RUL$BLD:c_g_cons.obj RUL$SRC:c_g_cons.c
$ cc 'CFLAGS /object=RUL$BLD:c_g_ctch.obj RUL$SRC:c_g_ctch.c
$ cc 'CFLAGS /object=RUL$BLD:c_g_decl.obj RUL$SRC:c_g_decl.c
$ cc 'CFLAGS /object=RUL$BLD:c_g_entr.obj RUL$SRC:c_g_entr.c
$ cc 'CFLAGS /object=RUL$BLD:c_g_lvr.obj RUL$SRC:c_g_lvr.c
$ cc 'CFLAGS /object=RUL$BLD:c_g_net.obj RUL$SRC:c_g_net.c
$ cc 'CFLAGS /object=RUL$BLD:c_g_on.obj RUL$SRC:c_g_on.c
$ cc 'CFLAGS /object=RUL$BLD:c_g_rhs.obj RUL$SRC:c_g_rhs.c
$ cc 'CFLAGS /object=RUL$BLD:c_g_rb.obj RUL$SRC:c_g_rb.c
$ cc 'CFLAGS /object=RUL$BLD:c_lvar.obj RUL$SRC:c_lvar.c
$ cc 'CFLAGS /object=RUL$BLD:c_msg.obj RUL$SRC:c_msg.c
$ cc 'CFLAGS /object=RUL$BLD:c_nt_nod.obj RUL$SRC:c_nt_nod.c
$ cc 'CFLAGS /object=RUL$BLD:c_nt_tst.obj RUL$SRC:c_nt_tst.c
$ cc 'CFLAGS /object=RUL$BLD:c_parser.obj RUL$BLD:c_parser.c
$ cc 'CFLAGS /object=RUL$BLD:c_s_gvar.obj RUL$SRC:c_s_gvar.c
$ cc 'CFLAGS /object=RUL$BLD:c_s_acts.obj RUL$SRC:c_s_acts.c
$ cc 'CFLAGS /object=RUL$BLD:c_s_blks.obj RUL$SRC:c_s_blks.c
$ cc 'CFLAGS /object=RUL$BLD:c_s_bnet.obj RUL$SRC:c_s_bnet.c
$ cc 'CFLAGS /object=RUL$BLD:c_s_bti.obj RUL$SRC:c_s_bti.c
$ cc 'CFLAGS /object=RUL$BLD:c_s_bval.obj RUL$SRC:c_s_bval.c
$ cc 'CFLAGS /object=RUL$BLD:c_s_cls.obj RUL$SRC:c_s_cls.c
$ cc 'CFLAGS /object=RUL$BLD:c_s_cons.obj RUL$SRC:c_s_cons.c
$ cc 'CFLAGS /object=RUL$BLD:c_s_decl.obj RUL$SRC:c_s_decl.c
$ cc 'CFLAGS /object=RUL$BLD:c_s_ext.obj RUL$SRC:c_s_ext.c
$ cc 'CFLAGS /object=RUL$BLD:c_s_on.obj RUL$SRC:c_s_on.c
$ cc 'CFLAGS /object=RUL$BLD:c_s_rhs.obj RUL$SRC:c_s_rhs.c
$ cc 'CFLAGS /object=RUL$BLD:c_s_rule.obj RUL$SRC:c_s_rule.c
$ cc 'CFLAGS /object=RUL$BLD:c_s_sql.obj RUL$SRC:c_s_sql.c
$ cc 'CFLAGS /object=RUL$BLD:c_val.obj RUL$SRC:c_val.c
$ cc 'CFLAGS /object=RUL$BLD:c_scan.obj	RUL$BLD:c_scan.c
$ cc 'CFLAGS /object=RUL$BLD:c_main.obj RUL$SRC:c_main.c
$!
$ cc 'CFLAGS /object=RUL$BLD:c_ec_int.obj RUL$SRC:c_ec_int.c
$ cc 'CFLAGS /object=RUL$BLD:c_ec_val.obj RUL$SRC:c_ec_val.c
$ cc 'CFLAGS /object=RUL$BLD:c_ec_wrp.obj RUL$SRC:c_ec_wrp.c
$!
$ library/create RUL$BLD:opscomp.olb
$ library RUL$BLD:opscomp.olb 'CMP_OBJS_1
$ library RUL$BLD:opscomp.olb 'CMP_OBJS_2
$ library RUL$BLD:opscomp.olb 'CMP_OBJS_3
$ library RUL$BLD:opscomp.olb 'CMP_OBJS_4
$ library RUL$BLD:opscomp.olb 'CMP_OBJS_5
$ library RUL$BLD:opscomp.olb 'CMP_OBJS_6
$ library RUL$BLD:opscomp.olb 'EMIT_OBJS
$ library RUL$BLD:opscomp.olb 'BOTH_OBJS_1
$ library RUL$BLD:opscomp.olb 'BOTH_OBJS_2
$ library RUL$BLD:opscomp.olb 'BOTH_OBJS_3
$ library RUL$BLD:opscomp.olb 'BOTH_OBJS_4
$!
$ link /NOTRACEBACK/USERLIB=PROCESS /exe=RUL$BIN:rulework.exe -
	RUL$BLD:opscomp.olb/lib/inc=c_main
$!
$ COPY RUL$SRC:rule_vms.hlp RUL$BIN:rulework.hlp	
$!
$ COPY RUL$SRC:rule_vms.cld RUL$BIN:rulework.cld	
$ set command RUL$BIN:rulework.cld
$ write sys$output "...  Compiler, $@, is up to date."
$!
$ if (P1 .eqs. "CMP") then exit
$!
$! RTL
$!
$RTL:
$ cc 'CFLAGS /object=RUL$BLD:cb_atom_vms.obj /define=(VMS_BIND) RUL$SRC:cb_atom.c
$ cc 'CFLAGS /object=RUL$BLD:cb_atom_c.obj /define=(LOWERCASE_C_BIND)/NAMES=AS_IS RUL$SRC:cb_atom.c
$ cc 'CFLAGS /object=RUL$BLD:cb_atom_up.obj /define=(UPPERCASE_C_BIND) RUL$SRC:cb_atom.c
$ cc 'CFLAGS /object=RUL$BLD:cb_wms_vms.obj /define=(VMS_BIND) RUL$SRC:cb_wms.c
$ cc 'CFLAGS /object=RUL$BLD:cb_wms_c.obj /define=(LOWERCASE_C_BIND)/NAMES=AS_IS RUL$SRC:cb_wms.c
$ cc 'CFLAGS /object=RUL$BLD:cb_wms_up.obj /define=(UPPERCASE_C_BIND) RUL$SRC:cb_wms.c
$ cc 'CFLAGS /object=RUL$BLD:cb_wmq_vms.obj /define=(VMS_BIND) RUL$SRC:cb_wmq.c
$ cc 'CFLAGS /object=RUL$BLD:cb_wmq_c.obj /define=(LOWERCASE_C_BIND)/NAMES=AS_IS RUL$SRC:cb_wmq.c
$ cc 'CFLAGS /object=RUL$BLD:cb_wmq_up.obj /define=(UPPERCASE_C_BIND) RUL$SRC:cb_wmq.c
$ cc 'CFLAGS /object=RUL$BLD:cmd_pars.obj RUL$BLD:cmd_pars.c
$ cc 'CFLAGS /object=RUL$BLD:cs.obj RUL$SRC:cs.c
$ cc 'CFLAGS /object=RUL$BLD:cvt_ext.obj RUL$SRC:cvt_ext.c
$ cc 'CFLAGS /object=RUL$BLD:beta.obj RUL$SRC:beta.c
$ cc 'CFLAGS /object=RUL$BLD:cb_com.obj RUL$SRC:cb_com.c
$ cc 'CFLAGS /object=RUL$BLD:dbg.obj RUL$SRC:dbg.c
$ cc 'CFLAGS /object=RUL$BLD:delta.obj RUL$SRC:delta.c
$ cc 'CFLAGS /object=RUL$BLD:i18n_str.obj RUL$SRC:i18n_str.c
$ cc 'CFLAGS /object=RUL$BLD:ml_arith.obj RUL$SRC:ml_arith.c
$ cc 'CFLAGS /object=RUL$BLD:ml_preds.obj RUL$SRC:ml_preds.c
$ cc 'CFLAGS /object=RUL$BLD:rac.obj RUL$SRC:rac.c
$ cc 'CFLAGS /object=RUL$BLD:rbs.obj RUL$SRC:rbs.c
$ cc 'CFLAGS /object=RUL$BLD:ref.obj  RUL$SRC:ref.c
$ cc 'CFLAGS /object=RUL$BLD:states.obj  RUL$SRC:states.c
$ cc 'CFLAGS /object=RUL$BLD:wm_iter.obj  RUL$SRC:wm_iter.c
$ cc 'CFLAGS /object=RUL$BLD:wm_print.obj  RUL$SRC:wm_print.c
$ cc 'CFLAGS /object=RUL$BLD:wm_query.obj RUL$SRC:wm_query.c 
$ cc 'CFLAGS /object=RUL$BLD:wm_updat.obj RUL$SRC:wm_updat.c
$ cc 'CFLAGS/DEFINE=(INTERP) /object=RUL$BLD:rts_scan.obj	RUL$BLD:rts_scan.c
$!
$ library/create RUL$LBR:'lib
$ library RUL$LBR:'lib 'BOTH_OBJS_1
$ library RUL$LBR:'lib 'BOTH_OBJS_2
$ library RUL$LBR:'lib 'BOTH_OBJS_3
$ library RUL$LBR:'lib 'BOTH_OBJS_4
$ library RUL$LBR:'lib 'RTS_OBJS_1
$ library RUL$LBR:'lib 'RTS_OBJS_2
$ library RUL$LBR:'lib 'RTS_OBJS_3
$ library RUL$LBR:'lib 'DBG_OBJS
$ library RUL$LBR:'lib/LOG 'RTL_OBJS
$! library RUL$LBR:'lib 'SQL_OBJS
$!
$ if (P1 .eqs. "RTL") then exit
$!
$! Header
$!
$HDR:
$ copy 'HEADER_FILES RUL$BLD:gend.h
$!
$ cc 'CFLAGS /object=RUL$BLD:strip_h.obj RUL$BLD:strip_h.c
$ link /NOTRACEBACK/USERLIB=PROCESS /exe=RUL$BLD:strip_h.exe RUL$BLD:strip_h.obj
$ curPath = f$parse( "[]") - ".;"
$ mkhdr :== $'curPath'strip_h.exe
$ mkhdr RUL$BLD:gend.h RUL$INC:rul_gend.h
$!
$ exit
