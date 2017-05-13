#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#
# RuleWorks - Rules based application development tool.
#
# Copyright (C) 1999  Compaq Computer Corporation
#
# This program is free software; you can redistribute it and/or modify it 
# under the terms of the GNU General Public License as published by the 
# Free Software Foundation; either version 2 of the License, or any later 
# version. 
#
# This program is distributed in the hope that it will be useful, but 
# WITHOUT ANY WARRANTY; without even the implied warranty of 
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General 
# Public License for more details. 
#
# You should have received a copy of the GNU General Public License along 
# with this program; if not, write to the Free Software Foundation, 
# Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
# Email: ruleworks@altavista.net
#
#
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#
#++
# FACILITY:
#       RULEWORKS
#
# ABSTRACT:
#       MAKE script for the RULEWORKS product.
#
# USAGE:
#       At the command line decide which 'target' is to be built.
#       A target is always specified :
#
#               TARGET  :  SOURCE
#
#       This is read as "TARGET depends on SOURCE".
#
#       The main targets in this build script can be examined by building the
#       USAGE target. >nmake -f rul_mw.mak USAGE
#
#       Several macros can also be defined to 'direct' the build.
#
# ENVIRONMENT:
#
# NOTE:
#       Please try to keep each list of source files alphabetical.
#
# AUTHOR: Stephen Bolland
#
# MODIFIED BY:
#       SB      Stephen Bolland
#
# REVISION HISTORY:
#
#       When            Who     Why
#		20-Feb-1998	SB	Created
#		05-Nov-1999 SB	Modified for GPL
#

# ***********
# DEFINITIONS
# ***********
#

#	Product.
#
PRODUCT   = rulework
PREFIX	  = rulm
RTLLIB    = rul_rtlm.lib

#
#	Environment.
#
SRC   = ..\source
BLD	  = ..\build
BIN	  = ..\bin
EXM   = ..\examples
INC   = ..\include
LBR   = ..\library


#
#	Software Build Tools.
#
!include "c:\program files\microsoft visual studio\vc98\include\WIN32.MAK"

# Compiler
cc     = cl
ccout  = /Fo

# -DNDEBUG=1 skip assume's, ifndefs
# -Od  optimize - disable all
# -k-  optimize - turn on/off standard stack frame
# -vi  optimize - turn inline expansion on

cdebug =
!IFDEF DEBUG
cdebug = -Od -Ge -Zi
!ELSE
cdebug = $(cdebug) -DNDEBUG=1 -Ox -Gs
!ENDIF

#
# Default C compile rule
#
# -c   compile only
# -I   include directory
# -G4  for 80486 processor
# -w   display warnings
#

cflags =  $(cflags) $(ccout)$@ -c -I$(SRC) -I$(INC) -DWINDOWS -DWIN32 -DNDEBUG
cflags =  $(cflags) -w
cflags =  $(cflags) -G4

# Librarian
libr      = lib
libflags  =

# Linker
link      = cl

# link options
#
# -Td  Target = 32 bit DLL
# -Tpe  Target = 32 bit .exe
# -ap   Console application
# -c    case sensitive
# -x    no map

lkflags   =  
#-Tpe -aa -c -x -v

#
#	Rules.
#
.c.obj :
	$(cc) $(cflags) $(cdebug) $<


#
#	Compiler Object Modules.
#
BOTH_LIB    =	 $(BLD)\rulboth.lib
BOTH_OBJS_1 =	 $(BLD)\atomize.obj  $(BLD)\btree.obj    $(BLD)\decl_cls.obj \
		 $(BLD)\decl_dec.obj $(BLD)\decl_ent.obj $(BLD)\decl_ext.obj

BOTH_OBJS_2 =	 $(BLD)\decl_inh.obj $(BLD)\dyar.obj     $(BLD)\hash.obj     \
		 $(BLD)\ios_file.obj $(BLD)\ios_win.obj

BOTH_OBJS_3 = 	 $(BLD)\list.obj     $(BLD)\mem.obj      $(BLD)\ml_comps.obj \
		 $(BLD)\ml_hash.obj  $(BLD)\ml_polys.obj $(BLD)\ml_print.obj

BOTH_OBJS_4 =	 $(BLD)\ml_query.obj $(BLD)\ml_table.obj $(BLD)\msg.obj

CMP_LIB    =	 $(BLD)\rulcomp.lib
CMP_OBJS_1 =	 $(BLD)\c_anodes.obj $(BLD)\c_atypes.obj $(BLD)\c_cli.obj    \
		 $(BLD)\c_conrg.obj  $(BLD)\c_cons.obj   $(BLD)\c_g_clas.obj

CMP_OBJS_2 =	 $(BLD)\c_g_cons.obj $(BLD)\c_g_ctch.obj $(BLD)\c_g_decl.obj \
		 $(BLD)\c_g_entr.obj $(BLD)\c_g_lvr.obj  $(BLD)\c_g_net.obj

CMP_OBJS_3 =	 $(BLD)\c_g_on.obj   $(BLD)\c_g_rhs.obj  $(BLD)\c_g_rb.obj \
		 $(BLD)\c_lvar.obj   $(BLD)\c_msg.obj    $(BLD)\c_nt_nod.obj

CMP_OBJS_4 =	 $(BLD)\c_s_gvar.obj $(BLD)\c_s_acts.obj $(BLD)\c_s_blks.obj \
		 $(BLD)\c_s_bnet.obj $(BLD)\c_s_bti.obj  $(BLD)\c_s_bval.obj

CMP_OBJS_5 =	 $(BLD)\c_s_decl.obj $(BLD)\c_s_ext.obj  $(BLD)\c_s_on.obj   \
		 $(BLD)\c_s_rhs.obj  $(BLD)\c_s_rule.obj $(BLD)\c_s_sql.obj

CMP_OBJS_6 = 	 $(BLD)\c_nt_tst.obj $(BLD)\c_parser.obj $(BLD)\c_scan.obj   \
		 $(BLD)\c_val.obj    $(BLD)\c_s_cls.obj  $(BLD)\c_s_cons.obj

EMIT_LIB   =	 $(BLD)\rulemit.lib
EMIT_OBJS  =	 $(BLD)\c_ec_int.obj $(BLD)\c_ec_val.obj $(BLD)\c_ec_wrp.obj

RTS_LIB    =	 $(BLD)\rulrts.lib
RTS_OBJS_1 = 	 $(BLD)\beta.obj     $(BLD)\cb_com.obj   $(BLD)\cs.obj       \
		 $(BLD)\cvt_ext.obj  $(BLD)\delta.obj    $(BLD)\i18n_str.obj

RTS_OBJS_2 =	 $(BLD)\ml_arith.obj $(BLD)\ml_preds.obj $(BLD)\rac.obj      \
		 $(BLD)\rbs.obj      $(BLD)\ref.obj      $(BLD)\states.obj

RTS_OBJS_3 =	 $(BLD)\wm_iter.obj  $(BLD)\wm_print.obj $(BLD)\wm_query.obj \
		 $(BLD)\wm_updat.obj

DBG_LIB    =	 $(BLD)\ruldbg.lib
DBG_OBJS   =	 $(BLD)\cmd_pars.obj $(BLD)\dbg.obj $(BLD)\rts_scan.obj

RTL_LIB    =     $(BLD)\rulrtl.lib
RTL_OBJS   =	 $(BLD)\cb_atom.obj $(BLD)\cb_wmq.obj $(BLD)\cb_wms.obj

RT_SCAN    =	 INTERP

RULLIBS    =	 $(RTL_LIB) $(DBG_LIB) $(RTS_LIB) $(BOTH_LIB)
		
HEADER_FILES	=	$(SRC)\common.h $(SRC)\decl.h $(SRC)\mol.h \
			$(SRC)\rbs.h $(SRC)\cs.h $(SRC)\rac.h $(SRC)\ref.h \
			$(SRC)\beta.h $(SRC)\delta.h $(SRC)\wm.h $(SRC)\ios.h \
			$(SRC)\cvt.h $(SRC)\sql.h $(SRC)\states.h

COPY_HDR_1	=	$(SRC)\common.h + $(SRC)\decl.h + $(SRC)\mol.h
COPY_HDR_2	=	$(SRC)\rbs.h + $(SRC)\cs.h + $(SRC)\rac.h + $(SRC)\ref.h
COPY_HDR_3	=	$(SRC)\beta.h + $(SRC)\delta.h + $(SRC)\wm.h + $(SRC)\ios.h
COPY_HDR_4	=	$(SRC)\cvt.h + $(SRC)\sql.h + $(SRC)\states.h

# ************
# MAIN TARGETS
# ************

#
#	How to use this script.
#

USAGE   :
        @ echo Build script for the RULEWORKS product on Intel Windows.
        @ echo (Microsoft Compiler)
        @ echo Usage :
        @ echo "  nmake -f rul_mw.mak [/macro=macro=1, ...] <target>"
		@ echo "  macro values"
        @ echo "  target values"
        @ echo "    USAGE  - Display this message"
        @ echo "    KIT    - Build the RuleWorks Kit"
        @ echo "    CMP    - Build the RuleWorks Compiler"
        @ echo "    RTL    - Build the RuleWorks Run Time Library"
        @ echo "    HDR    - Build the RuleWorks Header File for Compiled Code"
        @ echo "    EXAM   - Build the RuleWorks Example"


#
#
#	Building the Compiler.
#
CMP	:	$(BIN)\$(PRODUCT).exe \
		$(BIN)\$(PRODUCT).hlp
	@ echo "The RuleWorks Compiler is up-to-date."
#
#	Building the Run Time Library.
#
RTL	:	$(LBR)\$(RTLLIB)
	@ echo "The RuleWorks Run Time Library is up-to-date."
#
#	Building the Header File.
#
HDR	:	$(INC)\rul_gend.h \
		$(INC)\rul_rtl.h $(INC)\rul_rtl.for $(INC)\rul_rtl.pas
	@ echo "The RuleWorks Header Files are up-to-date."

#
#	Building the Examples.
#
EXAM	:	$(EXM)\count.rul \
		$(EXM)\advent.rul \
		$(EXM)\tourney.rul \
		$(EXM)\exam_mw.mak \
		$(EXM)\exam_lnx.mak \
		$(EXM)\exam_unx.mak
		
	@ echo "The RuleWorks Examples are up-to-date."

#
#	Building a Kit.
#
KIT	:	CMP RTL HDR EXAM
	@ echo "The RuleWorks Kit is up-to-date."


# ****************
# BUILD PROCEDURES
# ****************

$(BIN)\$(PRODUCT).exe : $(BLD)\c_main.obj $(CMP_LIB) $(EMIT_LIB) $(BOTH_LIB)
	$(link) /Fe$(BIN)\$(PRODUCT).exe $(guilibs) \
	$(BLD)\c_main.obj $(CMP_LIB) $(EMIT_LIB) $(BOTH_LIB)
	@ echo   ...
	@ echo   ...  Compiler, $@, is up to date.
	@ echo   ...

$(BIN)\$(PRODUCT).hlp	:	$(SRC)\rul_win.hlp
	@ COPY $(SRC)\rul_win.hlp $(BIN)\$(PRODUCT).hlp	

$(BOTH_LIB) :	$(BOTH_OBJS_1) $(BOTH_OBJS_2) $(BOTH_OBJS_3) $(BOTH_OBJS_4)
	$(libr) /out:$@ $(libflags) $(BOTH_OBJS_1)
	$(libr) $@ $(libflags) $(BOTH_OBJS_2)
	$(libr) $@ $(libflags) $(BOTH_OBJS_3)
	$(libr) $@ $(libflags) $(BOTH_OBJS_4)
	@ echo   ...
	@ echo   ...  Library, $@, is up to date.
	@ echo   ...

$(CMP_LIB) :	$(CMP_OBJS_1) $(CMP_OBJS_2) $(CMP_OBJS_3) $(CMP_OBJS_4) $(CMP_OBJS_5) $(CMP_OBJS_6)
	$(libr) /out:$@ $(libflags) $(CMP_OBJS_1)
	$(libr) $@ $(libflags) $(CMP_OBJS_2)
	$(libr) $@ $(libflags) $(CMP_OBJS_3)
	$(libr) $@ $(libflags) $(CMP_OBJS_4)
	$(libr) $@ $(libflags) $(CMP_OBJS_5)
	$(libr) $@ $(libflags) $(CMP_OBJS_6)
	@ echo   ...
	@ echo   ...  Library, $@, is up to date.
	@ echo   ...

$(EMIT_LIB) :	$(EMIT_OBJS)
	$(libr) /out:$@ $(libflags) $(EMIT_OBJS) 
	@ echo   ...
	@ echo   ...  Library, $@, is up to date.
	@ echo   ...

#--------------------------------------------------------------------------
#
#	RuleWorks run-time libraries (plus BOTH_LIB, above)
#
#--------------------------------------------------------------------------


$(RTS_LIB) :	$(RTS_OBJS_1) $(RTS_OBJS_2) $(RTS_OBJS_3)
	$(libr) /out:$@ $(libflags) $(RTS_OBJS_1)
	$(libr) $@ $(libflags) $(RTS_OBJS_2)
	$(libr) $@ $(libflags) $(RTS_OBJS_3)
	@ echo   ...
	@ echo   ...  Library, $@, is up to date.
	@ echo   ...

$(DBG_LIB) :	$(DBG_OBJS)
	$(libr) /out:$@ $(libflags) $(DBG_OBJS)
	@ echo   ...
	@ echo   ...  Library, $@, is up to date.
	@ echo   ...

$(RTL_LIB) :	$(RTL_OBJS)
	$(libr) /out:$@ $(libflags) $(RTL_OBJS)
	@ echo   ...
	@ echo   ...  Library, $@, is up to date.
	@ echo   ...

$(LBR)\$(RTLLIB) :	$(RTL_OBJS) $(DBG_OBJS) $(RTS_OBJS_1) $(RTS_OBJS_2) $(RTS_OBJS_3) \
		$(BOTH_OBJS_1) $(BOTH_OBJS_2) $(BOTH_OBJS_3) $(BOTH_OBJS_4)
	$(libr) /out:$@ $(libflags) $(RTL_OBJS)
	$(libr) $@ $(libflags) $(DBG_OBJS)
	$(libr) $@ $(libflags) $(RTS_OBJS_1)
	$(libr) $@ $(libflags) $(RTS_OBJS_2)
	$(libr) $@ $(libflags) $(RTS_OBJS_3)
	$(libr) $@ $(libflags) $(BOTH_OBJS_1)
	$(libr) $@ $(libflags) $(BOTH_OBJS_2)
	$(libr) $@ $(libflags) $(BOTH_OBJS_3)
	$(libr) $@ $(libflags) $(BOTH_OBJS_4)
	@ echo   ...
	@ echo   ...  Library, $@, is up to date.
	@ echo   ...

$(INC)\rul_gend.h       :       $(BLD)\gend.h $(BLD)\strip_h.exe
        $(BLD)\strip_h.exe $(BLD)\gend.h $(INC)\rul_gend.h

$(BLD)\gend.h : $(HEADER_FILES)
	 copy $(COPY_HDR_1) + $(COPY_HDR_2) + $(COPY_HDR_3) + $(COPY_HDR_4) $(BLD)\gend.h
#	 copy $(COPY_HDR_2) + $(BLD)\gend.h $(BLD)\gend.h
#	 copy $(COPY_HDR_3) + $(BLD)\gend.h $(BLD)\gend.h
#	 copy $(COPY_HDR_4) + $(BLD)\gend.h $(BLD)\gend.h

$(BLD)\strip_h.exe	:	$(BLD)\mem.obj $(BLD)\strip_h.obj
	$(link) /Fe$(BLD)\strip_h.exe $(conlibs) \
	$(BLD)\mem.obj $(BLD)\strip_h.obj


# **********************
# COMPILATION PROCEDURES
# **********************

$(BLD)\strip_h.obj :	$(BLD)\strip_h.c
	$(cc) $(ccout)$@ $(cflags) $(cdebug) $?

$(BLD)\rts_scan.obj :	$(BLD)\rts_scan.c
      $(cc) $(ccout)$@ $(cflags) $(cdebug) /D$(RT_SCAN) $?

$(BLD)\c_scan.obj :	$(BLD)\c_scan.c
	$(cc) $(ccout)$@ $(cflags) $(cdebug) $?

$(BLD)\cb_atom.obj : $(SRC)\cb_atom.c
      $(cc) $(cflags) $(cdebug) /DLOWERCASE_C_BIND $?

$(BLD)\cb_wms.obj : $(SRC)\cb_wms.c
	$(cc) $(cflags) $(cdebug) /DLOWERCASE_C_BIND $?

$(BLD)\cb_wmq.obj : $(SRC)\cb_wmq.c
	$(cc) $(cflags) $(cdebug) /DLOWERCASE_C_BIND $?

$(BLD)\atomize.obj : $(BLD)\atomize.c
	$(cc) $(cflags) $(cdebug) $?

$(BLD)\btree.obj : $(SRC)\btree.c
	$(cc) $(cflags) $(cdebug) $?

$(BLD)\c_main.obj : $(SRC)\c_main.c
	$(cc) $(cflags) $(cdebug) $?

$(BLD)\c_conrg.obj : $(SRC)\c_conrg.c
	$(cc) $(cflags) $(cdebug) $?

$(BLD)\c_cli.obj : $(SRC)\c_cli.c
	$(cc) $(cflags) $(cdebug) $?

$(BLD)\c_cons.obj : $(SRC)\c_cons.c
	$(cc) $(cflags) $(cdebug) $?

$(BLD)\cmd_pars.obj : $(BLD)\cmd_pars.c
	$(cc) $(cflags) $(cdebug) $?

$(BLD)\cs.obj : $(SRC)\cs.c
	$(cc) $(cflags) $(cdebug) $?

$(BLD)\cvt_ext.obj : $(SRC)\cvt_ext.c
	$(cc) $(cflags) $(cdebug) $?

$(BLD)\c_anodes.obj : $(SRC)\c_anodes.c
	$(cc) $(cflags) $(cdebug) $?

$(BLD)\c_atypes.obj : $(SRC)\c_atypes.c
	$(cc) $(cflags) $(cdebug) $?

$(BLD)\c_g_clas.obj : $(SRC)\c_g_clas.c
	$(cc) $(cflags) $(cdebug) $?

$(BLD)\c_g_cons.obj : $(SRC)\c_g_cons.c
	$(cc) $(cflags) $(cdebug) $?

$(BLD)\c_g_ctch.obj : $(SRC)\c_g_ctch.c
	$(cc) $(cflags) $(cdebug) $?

$(BLD)\c_g_decl.obj : $(SRC)\c_g_decl.c
	$(cc) $(cflags) $(cdebug) $?

$(BLD)\c_g_entr.obj : $(SRC)\c_g_entr.c
	$(cc) $(cflags) $(cdebug) $?

$(BLD)\c_g_lvr.obj : $(SRC)\c_g_lvr.c
	$(cc) $(cflags) $(cdebug) $?

$(BLD)\c_g_net.obj : $(SRC)\c_g_net.c
	$(cc) $(cflags) $(cdebug) $?

$(BLD)\c_g_on.obj : $(SRC)\c_g_on.c
	$(cc) $(cflags) $(cdebug) $?

$(BLD)\c_g_rhs.obj : $(SRC)\c_g_rhs.c
	$(cc) $(cflags) $(cdebug) $?

$(BLD)\c_g_rb.obj : $(SRC)\c_g_rb.c
	$(cc) $(cflags) $(cdebug) $?

$(BLD)\c_ec_int.obj : $(SRC)\c_ec_int.c
	$(cc) $(cflags) $(cdebug) $?

$(BLD)\c_ec_val.obj : $(SRC)\c_ec_val.c
	$(cc) $(cflags) $(cdebug) $?

$(BLD)\c_ec_wrp.obj : $(SRC)\c_ec_wrp.c
	$(cc) $(cflags) $(cdebug) $?

$(BLD)\c_lvar.obj : $(SRC)\c_lvar.c
	$(cc) $(cflags) $(cdebug) $?

$(BLD)\c_msg.obj : $(SRC)\c_msg.c
	$(cc) $(cflags) $(cdebug) $?

$(BLD)\c_nt_nod.obj : $(SRC)\c_nt_nod.c
	$(cc) $(cflags) $(cdebug) $?

$(BLD)\c_nt_tst.obj : $(SRC)\c_nt_tst.c
	$(cc) $(cflags) $(cdebug) $?

$(BLD)\c_parser.obj : $(BLD)\c_parser.c
	$(cc) $(cflags) $(cdebug) -Od $?

$(BLD)\c_s_gvar.obj : $(SRC)\c_s_gvar.c
	$(cc) $(cflags) $(cdebug) $?

$(BLD)\c_s_acts.obj : $(SRC)\c_s_acts.c
	$(cc) $(cflags) $(cdebug) $?

$(BLD)\c_s_blks.obj : $(SRC)\c_s_blks.c
	$(cc) $(cflags) $(cdebug) $?

$(BLD)\c_s_bnet.obj : $(SRC)\c_s_bnet.c
	$(cc) $(cflags) $(cdebug) $?

$(BLD)\c_s_bti.obj : $(SRC)\c_s_bti.c
	$(cc) $(cflags) $(cdebug) $?

$(BLD)\c_s_bval.obj : $(SRC)\c_s_bval.c
	$(cc) $(cflags) $(cdebug) $?

$(BLD)\c_s_cls.obj : $(SRC)\c_s_cls.c
	$(cc) $(cflags) $(cdebug) $?

$(BLD)\c_s_cons.obj : $(SRC)\c_s_cons.c
	$(cc) $(cflags) $(cdebug) $?

$(BLD)\c_s_decl.obj : $(SRC)\c_s_decl.c
	$(cc) $(cflags) $(cdebug) $?

$(BLD)\c_s_ext.obj : $(SRC)\c_s_ext.c
	$(cc) $(cflags) $(cdebug) $?

$(BLD)\c_s_on.obj : $(SRC)\c_s_on.c
	$(cc) $(cflags) $(cdebug) $?

$(BLD)\c_s_rhs.obj : $(SRC)\c_s_rhs.c
	$(cc) $(cflags) $(cdebug) $?

$(BLD)\c_s_rule.obj : $(SRC)\c_s_rule.c
	$(cc) $(cflags) $(cdebug) $?

$(BLD)\c_s_sql.obj : $(SRC)\c_s_sql.c
	$(cc) $(cflags) $(cdebug) $?

$(BLD)\c_val.obj : $(SRC)\c_val.c
	$(cc) $(cflags) $(cdebug) $?

$(BLD)\decl_cls.obj : $(SRC)\decl_cls.c
	$(cc) $(cflags) $(cdebug) $?

$(BLD)\decl_dec.obj : $(SRC)\decl_dec.c
	$(cc) $(cflags) $(cdebug) $?

$(BLD)\decl_ent.obj : $(SRC)\decl_ent.c
	$(cc) $(cflags) $(cdebug) $?

$(BLD)\decl_ext.obj : $(SRC)\decl_ext.c
	$(cc) $(cflags) $(cdebug) $?

$(BLD)\decl_inh.obj : $(SRC)\decl_inh.c
	$(cc) $(cflags) $(cdebug) $?

$(BLD)\dyar.obj : $(SRC)\dyar.c
	$(cc) $(cflags) $(cdebug) $?

$(BLD)\hash.obj : $(SRC)\hash.c
	$(cc) $(cflags) $(cdebug) $?

$(BLD)\ios_file.obj : $(SRC)\ios_file.c
	$(cc) $(cflags) $(cdebug) $?

$(BLD)\ios_win.obj : $(SRC)\ios_win.c
	$(cc) $(cflags) $(cdebug) $?

$(BLD)\list.obj : $(SRC)\list.c
	$(cc) $(cflags) $(cdebug) $?

$(BLD)\mem.obj : $(SRC)\mem.c
	$(cc) $(cflags) $(cdebug) $?

$(BLD)\ml_comps.obj : $(SRC)\ml_comps.c
	$(cc) $(cflags) $(cdebug) $?

$(BLD)\ml_hash.obj : $(SRC)\ml_hash.c
	$(cc) $(cflags) $(cdebug) $?

$(BLD)\ml_polys.obj: $(SRC)\ml_polys.c
	$(cc) $(cflags) $(cdebug) $?

$(BLD)\ml_print.obj : $(SRC)\ml_print.c
	$(cc) $(cflags) $(cdebug) $?

$(BLD)\ml_query.obj : $(SRC)\ml_query.c
	$(cc) $(cflags) $(cdebug) $?

$(BLD)\ml_table.obj : $(SRC)\ml_table.c
	$(cc) $(cflags) $(cdebug) $?

$(BLD)\msg.obj: $(SRC)\msg.c
	$(cc) $(cflags) $(cdebug) $?

$(BLD)\beta.obj : $(SRC)\beta.c
	$(cc) $(cflags) $(cdebug) $?

$(BLD)\cb_com.obj : $(SRC)\cb_com.c
	$(cc) $(cflags) $(cdebug) $?

$(BLD)\dbg.obj : $(SRC)\dbg.c
	$(cc) $(cflags) $(cdebug) $?

$(BLD)\delta.obj : $(SRC)\delta.c
	$(cc) $(cflags) $(cdebug) $?

$(BLD)\i18n_str.obj : $(SRC)\i18n_str.c
	$(cc) $(cflags) $(cdebug) $?

$(BLD)\ml_arith.obj : $(SRC)\ml_arith.c
	$(cc) $(cflags) $(cdebug) $?

$(BLD)\ml_preds.obj : $(SRC)\ml_preds.c
	$(cc) $(cflags) $(cdebug) $?

$(BLD)\rac.obj : $(SRC)\rac.c
	$(cc) $(cflags) $(cdebug) $?

$(BLD)\rbs.obj : $(SRC)\rbs.c
	$(cc) $(cflags) $(cdebug) $?

$(BLD)\ref.obj : $(SRC)\ref.c
	$(cc) $(cflags) $(cdebug) $?

$(BLD)\states.obj : $(SRC)\states.c
	$(cc) $(cflags) $(cdebug) $?

$(BLD)\wm_iter.obj : $(SRC)\wm_iter.c
	$(cc) $(cflags) $(cdebug) $?

$(BLD)\wm_print.obj : $(SRC)\wm_print.c
	$(cc) $(cflags) $(cdebug) $?

$(BLD)\wm_query.obj : $(SRC)\wm_query.c
	$(cc) $(cflags) $(cdebug) $?

$(BLD)\wm_updat.obj : $(SRC)\wm_updat.c
	$(cc) $(cflags) $(cdebug) $?

# ******************
# lex AND yacc FILES
# ******************

$(BLD)\c_scan.c	  :	$(SRC)\scan.lex
        @ echo LEX required for build of $@ (UNIX/LINUX)
	@ ABORT

$(BLD)\rts_scan.c :	$(SRC)\scan.lex
        @ echo LEX required for build of $@ (UNIX/LINUX)
	@ ABORT

$(BLD)\atomize.c  :	$(SRC)\ops_atom.lex	
        @ echo LEX required for build of $@ (UNIX/LINUX)
	@ ABORT

$(BLD)\strip_h.c  :	$(SRC)\strip_h.lex	
        @ echo LEX required for build of $@ (UNIX/LINUX)
	@ ABORT

$(BLD)\cmd_pars.c :	$(SRC)\cmd_pars.y
        @ echo YACC required for build of $@ (UNIX/LINUX)
	@ ABORT

$(BLD)\c_parser.c :	$(SRC)\c_parser.y
        @ echo YACC required for build of $@ (UNIX/LINUX)
	@ ABORT
