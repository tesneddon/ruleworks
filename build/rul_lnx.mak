#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#
# RuleWorks - Rules based application development tool.
#
# Copyright (C) 1999  Compaq Computer Corporation
# Copyright (C) 2017  Endless Software Solutions
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
#       Build script for the RULEWORKS product using MAKE in a LINUX
#		environment.
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
#       USAGE target. #make -f rul_lnx.mak USAGE
#
#       Several macros can also be defined to 'direct' the build.
#
# ENVIRONMENT:
#
# NOTE:
#       Please try to keep each list of source files alphabetical.
#
#--


# ***********
# DEFINITIONS
# ***********

#
#	Product.
#
PRODUCT   = rulework
PREFIX	  = rul
RTLIB     = librulrtl.a

#
#	Environment.
#
SRC   = ../source
BLD	  = ../build
BIN	  = ../bin
EXM   = ../examples
INC   = ../include
LIB   = ../library

#
#	Software Build Tools.
#

# Compiler
cc     = cc
ccout  = -o
cdebug =

cflags = $(cdebug) -c -I$(SRC) -I$(INC) -D__UNIX

# Librarian
libr      = ar -r
libflags  =

# Linker
link      = cc
lkflags   =

#
#	Rules.
#
.c.o :
	$(cc) $(ccout) $@ $(cflags) $(cdebug) $<


#
#	Compiler Object Modules.
#
BOTH_LIB    =	 $(BLD)/rulboth.a
BOTH_OBJS_1 =	 $(BLD)/atomize.o  $(BLD)/btree.o    $(BLD)/decl_cls.o \
		 $(BLD)/decl_dec.o $(BLD)/decl_ent.o $(BLD)/decl_ext.o

BOTH_OBJS_2 =	 $(BLD)/decl_inh.o $(BLD)/dyar.o     $(BLD)/hash.o     \
		 $(BLD)/ios_file.o 

BOTH_OBJS_3 = 	 $(BLD)/list.o     $(BLD)/mem.o      $(BLD)/ml_comps.o \
		 $(BLD)/ml_hash.o  $(BLD)/ml_polys.o $(BLD)/ml_print.o

BOTH_OBJS_4 =	 $(BLD)/ml_query.o $(BLD)/ml_table.o $(BLD)/msg.o

CMP_LIB    =	 $(BLD)/rulcomp.a
CMP_OBJS_1 =	 $(BLD)/c_anodes.o $(BLD)/c_atypes.o $(BLD)/c_cli.o    \
		 $(BLD)/c_conrg.o  $(BLD)/c_cons.o   $(BLD)/c_g_clas.o

CMP_OBJS_2 =	 $(BLD)/c_g_cons.o $(BLD)/c_g_ctch.o $(BLD)/c_g_decl.o \
		 $(BLD)/c_g_entr.o $(BLD)/c_g_lvr.o  $(BLD)/c_g_net.o

CMP_OBJS_3 =	 $(BLD)/c_g_on.o   $(BLD)/c_g_rhs.o  $(BLD)/c_g_rb.o \
		 $(BLD)/c_lvar.o   $(BLD)/c_msg.o    $(BLD)/c_nt_nod.o

CMP_OBJS_4 =	 $(BLD)/c_s_gvar.o $(BLD)/c_s_acts.o $(BLD)/c_s_blks.o \
		 $(BLD)/c_s_bnet.o $(BLD)/c_s_bti.o  $(BLD)/c_s_bval.o

CMP_OBJS_5 =	 $(BLD)/c_s_decl.o $(BLD)/c_s_ext.o  $(BLD)/c_s_on.o  \
		 $(BLD)/c_s_rhs.o  $(BLD)/c_s_rule.o $(BLD)/c_s_sql.o

CMP_OBJS_6 = 	 $(BLD)/c_nt_tst.o $(BLD)/c_parser.o $(BLD)/c_scan.o   \
		 $(BLD)/c_val.o    $(BLD)/c_s_cls.o  $(BLD)/c_s_cons.o

EMIT_LIB   =	 $(BLD)/rulemit.a
EMIT_OBJS  =	 $(BLD)/c_ec_int.o $(BLD)/c_ec_val.o $(BLD)/c_ec_wrp.o

RTS_LIB    =	 $(BLD)/rulrts.a
RTS_OBJS_1 = 	 $(BLD)/beta.o     $(BLD)/cb_com.o   $(BLD)/cs.o       \
		 $(BLD)/cvt_ext.o  $(BLD)/delta.o    $(BLD)/i18n_str.o

RTS_OBJS_2 =	 $(BLD)/ml_arith.o $(BLD)/ml_preds.o $(BLD)/rac.o      \
		 $(BLD)/rbs.o      $(BLD)/ref.o      $(BLD)/states.o

RTS_OBJS_3 =	 $(BLD)/wm_iter.o  $(BLD)/wm_print.o $(BLD)/wm_query.o \
		 $(BLD)/wm_updat.o

DBG_LIB    =	 $(BLD)/ruldbg.a
DBG_OBJS   =	 $(BLD)/cmd_pars.o $(BLD)/dbg.o $(BLD)/rts_scan.o

RTL_LIB    =     $(BLD)/rulrtl.a
RTL_OBJS   =	 $(BLD)/cb_atom.o $(BLD)/cb_wmq.o $(BLD)/cb_wms.o

RT_SCAN    =	 INTERP

RULLIBS    =	 $(RTL_LIB) $(DBG_LIB) $(RTS_LIB) $(BOTH_LIB)

HEADER_FILES	=	$(SRC)/common.h $(SRC)/decl.h $(SRC)/mol.h \
			$(SRC)/rbs.h $(SRC)/cs.h $(SRC)/rac.h $(SRC)/ref.h \
			$(SRC)/beta.h $(SRC)/delta.h $(SRC)/wm.h $(SRC)/ios.h \
			$(SRC)/cvt.h $(SRC)/sql.h $(SRC)/states.h

COPY_HDR_1	=	$(SRC)/common.h + $(SRC)/decl.h + $(SRC)/mol.h
COPY_HDR_2	=	$(SRC)/rbs.h + $(SRC)/cs.h + $(SRC)/rac.h + $(SRC)/ref.h
COPY_HDR_3	=	$(SRC)/beta.h + $(SRC)/delta.h + $(SRC)/wm.h + $(SRC)/ios.h
COPY_HDR_4	=	$(SRC)/cvt.h + $(SRC)/sql.h + $(SRC)/states.h


# ************
# MAIN TARGETS
# ************

#
#	How to use this script.
#

USAGE   :
	@ echo Build script for the RULEWORKS product on Digital UNIX.
	@ echo Usage :
	@ echo "  make -f rul_lnx.mak [/macro=macro=1, ...] <target>"
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
CMP	:	$(BIN)/$(PRODUCT) \
		$(BIN)/$(PRODUCT).1
	@ echo "The RuleWorks Compiler is up-to-date."

#
#	Building the Run Time Library.
#
RTL	:	$(LIB)/$(RTLIB)
	@ echo "The RuleWorks Run Time Library is up-to-date."

#
#	Building the Header Files.
#
HDR	:	$(INC)/rul_gend.h \
		$(INC)/rul_rtl.h \
		$(INC)/rul_rtl.for \
		$(INC)/rul_rtl.pas
	@ echo "The RuleWorks Header Files are up-to-date."

#
#	Building the Examples.
#
EXAM	:	$(EXM)/count.rul \
		$(EXM)/advent.rul \
		$(EXM)/tourney.rul \
		$(EXM)/exam_lnx.mak \
		$(EXM)/exam_unx.mak
	@ echo "The RuleWorks Examples are up-to-date."

#
#	Building a Kit.
#
KIT	:	CMP RTL HDR EXAM
	@ echo "The RuleWorks Kit is up-to-date."


# ****************
# BUILD PROCEDURES
# ****************

$(BIN)/$(PRODUCT) : $(BLD)/c_main.o $(CMP_LIB) $(EMIT_LIB) $(BOTH_LIB)
	$(link) $(lkflags) $(linkdebug) -o $(BIN)/$(PRODUCT) \
	$(BLD)/c_main.o $(CMP_LIB) $(EMIT_LIB) $(BOTH_LIB)


$(BOTH_LIB) :	$(BOTH_OBJS_1) $(BOTH_OBJS_2) $(BOTH_OBJS_3) $(BOTH_OBJS_4)
	$(libr) $@ $(libflags) $(BOTH_OBJS_1)
	$(libr) $@ $(libflags) $(BOTH_OBJS_2)
	$(libr) $@ $(libflags) $(BOTH_OBJS_3)
	$(libr) $@ $(libflags) $(BOTH_OBJS_4)

$(CMP_LIB) :	$(CMP_OBJS_1) $(CMP_OBJS_2) $(CMP_OBJS_3) $(CMP_OBJS_4) $(CMP_OBJS_5) $(CMP_OBJS_6)
	$(libr) $@ $(libflags) $(CMP_OBJS_1)
	$(libr) $@ $(libflags) $(CMP_OBJS_2)
	$(libr) $@ $(libflags) $(CMP_OBJS_3)
	$(libr) $@ $(libflags) $(CMP_OBJS_4)
	$(libr) $@ $(libflags) $(CMP_OBJS_5)
	$(libr) $@ $(libflags) $(CMP_OBJS_6)
	@ echo   ...
	@ echo   ...  Library, $@, is up to date.
	@ echo   ...

$(EMIT_LIB) :	$(EMIT_OBJS)
	$(libr) $@ $(libflags) $(EMIT_OBJS) 
	@ echo   ...
	@ echo   ...  Library, $@, is up to date.
	@ echo   ...

#--------------------------------------------------------------------------
#
#	RuleWorks run-time libraries (plus BOTH_LIB, above)
#
#--------------------------------------------------------------------------


$(RTS_LIB) :	$(RTS_OBJS_1) $(RTS_OBJS_2) $(RTS_OBJS_3)
	$(libr) $@ $(libflags) $(RTS_OBJS_1)
	$(libr) $@ $(libflags) $(RTS_OBJS_2)
	$(libr) $@ $(libflags) $(RTS_OBJS_3)
	@ echo   ...
	@ echo   ...  Library, $@, is up to date.
	@ echo   ...

$(DBG_LIB) :	$(DBG_OBJS)
	$(libr) $@ $(libflags) $(DBG_OBJS)
	@ echo   ...
	@ echo   ...  Library, $@, is up to date.
	@ echo   ...

$(RTL_LIB) :	$(RTL_OBJS)
	$(libr) $@ $(libflags) $(RTL_OBJS)
	@ echo   ...
	@ echo   ...  Library, $@, is up to date.
	@ echo   ...

$(LIB)/$(RTLIB) :	$(RTL_OBJS) $(DBG_OBJS) $(RTS_OBJS_1) $(RTS_OBJS_2) $(RTS_OBJS_3) \
		$(BOTH_OBJS_1) $(BOTH_OBJS_2) $(BOTH_OBJS_3) $(BOTH_OBJS_4)
	$(libr) $@ $(libflags) $(RTL_OBJS)
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

$(INC)/rul_gend.h       :       $(BLD)/gend.h $(BLD)/strip_h
	$(BLD)/strip_h $(BLD)/gend.h $(INC)/rul_gend.h

$(BLD)/gend.h : $(HEADER_FILES)
	cat $(HEADER_FILES) > $(BLD)/gend.h

$(BLD)/strip_h	:	$(BLD)/mem.o $(BLD)/strip_h.o
	$(link) $(lkflags) $(linkdebug) -o $(BLD)/strip_h \
	$(BLD)/mem.o $(BLD)/strip_h.o



# **********************
# COMPILATION PROCEDURES
# **********************
$(BLD)/strip_h.o :	$(BLD)/strip_h.c
	$(cc) $(ccout) $@ $(cflags) $(cdebug) $?

$(BLD)/rts_scan.o :	$(BLD)/rts_scan.c
	$(cc) $(ccout) $@ $(cflags) $(cdebug) -D$(RT_SCAN) $?

$(BLD)/c_scan.o :	$(BLD)/c_scan.c
	$(cc) $(ccout) $@ $(cflags) $(cdebug) $?

$(BLD)/cb_atom.o : $(SRC)/cb_atom.c
	$(cc) $(ccout) $@ $(cflags) $(cdebug) -DLOWERCASE_C_BIND $?

$(BLD)/cb_wms.o : $(SRC)/cb_wms.c
	$(cc) $(ccout) $@ $(cflags) $(cdebug) -DLOWERCASE_C_BIND $?

$(BLD)/cb_wmq.o : $(SRC)/cb_wmq.c
	$(cc) $(ccout) $@ $(cflags) $(cdebug) -DLOWERCASE_C_BIND $?

$(BLD)/atomize.o : $(BLD)/atomize.c
	$(cc) $(ccout) $@ $(cflags) $(cdebug) $?

$(BLD)/btree.o : $(SRC)/btree.c
	$(cc) $(ccout) $@ $(cflags) $(cdebug) $?

$(BLD)/c_main.o : $(SRC)/c_main.c
	$(cc) $(ccout) $@ $(cflags) $(cdebug) $?

$(BLD)/c_conrg.o : $(SRC)/c_conrg.c
	$(cc) $(ccout) $@ $(cflags) $(cdebug) $?

$(BLD)/c_cli.o : $(SRC)/c_cli.c
	$(cc) $(ccout) $@ $(cflags) $(cdebug) $?

$(BLD)/c_cons.o : $(SRC)/c_cons.c
	$(cc) $(ccout) $@ $(cflags) $(cdebug) $?

$(BLD)/cmd_pars.o : $(BLD)/cmd_pars.c
	$(cc) $(ccout) $@ $(cflags) $(cdebug) $?

$(BLD)/cs.o : $(SRC)/cs.c
	$(cc) $(ccout) $@ $(cflags) $(cdebug) $?

$(BLD)/cvt_ext.o : $(SRC)/cvt_ext.c
	$(cc) $(ccout) $@ $(cflags) $(cdebug) $?

$(BLD)/c_anodes.o : $(SRC)/c_anodes.c
	$(cc) $(ccout) $@ $(cflags) $(cdebug) $?

$(BLD)/c_atypes.o : $(SRC)/c_atypes.c
	$(cc) $(ccout) $@ $(cflags) $(cdebug) $?

$(BLD)/c_g_clas.o : $(SRC)/c_g_clas.c
	$(cc) $(ccout) $@ $(cflags) $(cdebug) $?

$(BLD)/c_g_cons.o : $(SRC)/c_g_cons.c
	$(cc) $(ccout) $@ $(cflags) $(cdebug) $?

$(BLD)/c_g_ctch.o : $(SRC)/c_g_ctch.c
	$(cc) $(ccout) $@ $(cflags) $(cdebug) $?

$(BLD)/c_g_decl.o : $(SRC)/c_g_decl.c
	$(cc) $(ccout) $@ $(cflags) $(cdebug) $?

$(BLD)/c_g_entr.o : $(SRC)/c_g_entr.c
	$(cc) $(ccout) $@ $(cflags) $(cdebug) $?

$(BLD)/c_g_lvr.o : $(SRC)/c_g_lvr.c
	$(cc) $(ccout) $@ $(cflags) $(cdebug) $?

$(BLD)/c_g_net.o : $(SRC)/c_g_net.c
	$(cc) $(ccout) $@ $(cflags) $(cdebug) $?

$(BLD)/c_g_on.o : $(SRC)/c_g_on.c
	$(cc) $(ccout) $@ $(cflags) $(cdebug) $?

$(BLD)/c_g_rhs.o : $(SRC)/c_g_rhs.c
	$(cc) $(ccout) $@ $(cflags) $(cdebug) $?

$(BLD)/c_g_rb.o : $(SRC)/c_g_rb.c
	$(cc) $(ccout) $@ $(cflags) $(cdebug) $?

$(BLD)/c_ec_int.o : $(SRC)/c_ec_int.c
	$(cc) $(ccout) $@ $(cflags) $(cdebug) $?

$(BLD)/c_ec_val.o : $(SRC)/c_ec_val.c
	$(cc) $(ccout) $@ $(cflags) $(cdebug) $?

$(BLD)/c_ec_wrp.o : $(SRC)/c_ec_wrp.c
	$(cc) $(ccout) $@ $(cflags) $(cdebug) $?

$(BLD)/c_lvar.o : $(SRC)/c_lvar.c
	$(cc) $(ccout) $@ $(cflags) $(cdebug) $?

$(BLD)/c_msg.o : $(SRC)/c_msg.c
	$(cc) $(ccout) $@ $(cflags) $(cdebug) $?

$(BLD)/c_nt_nod.o : $(SRC)/c_nt_nod.c
	$(cc) $(ccout) $@ $(cflags) $(cdebug) $?

$(BLD)/c_nt_tst.o : $(SRC)/c_nt_tst.c
	$(cc) $(ccout) $@ $(cflags) $(cdebug) $?

$(BLD)/c_parser.o : $(BLD)/c_parser.c
	$(cc) $(ccout) $@ $(cflags) $(cdebug) $?

$(BLD)/c_s_gvar.o : $(SRC)/c_s_gvar.c
	$(cc) $(ccout) $@ $(cflags) $(cdebug) $?

$(BLD)/c_s_acts.o : $(SRC)/c_s_acts.c
	$(cc) $(ccout) $@ $(cflags) $(cdebug) $?

$(BLD)/c_s_blks.o : $(SRC)/c_s_blks.c
	$(cc) $(ccout) $@ $(cflags) $(cdebug) $?

$(BLD)/c_s_bnet.o : $(SRC)/c_s_bnet.c
	$(cc) $(ccout) $@ $(cflags) $(cdebug) $?

$(BLD)/c_s_bti.o : $(SRC)/c_s_bti.c
	$(cc) $(ccout) $@ $(cflags) $(cdebug) $?

$(BLD)/c_s_bval.o : $(SRC)/c_s_bval.c
	$(cc) $(ccout) $@ $(cflags) $(cdebug) $?

$(BLD)/c_s_cls.o : $(SRC)/c_s_cls.c
	$(cc) $(ccout) $@ $(cflags) $(cdebug) $?

$(BLD)/c_s_cons.o : $(SRC)/c_s_cons.c
	$(cc) $(ccout) $@ $(cflags) $(cdebug) $?

$(BLD)/c_s_decl.o : $(SRC)/c_s_decl.c
	$(cc) $(ccout) $@ $(cflags) $(cdebug) $?

$(BLD)/c_s_ext.o : $(SRC)/c_s_ext.c
	$(cc) $(ccout) $@ $(cflags) $(cdebug) $?

$(BLD)/c_s_on.o : $(SRC)/c_s_on.c
	$(cc) $(ccout) $@ $(cflags) $(cdebug) $?

$(BLD)/c_s_rhs.o : $(SRC)/c_s_rhs.c
	$(cc) $(ccout) $@ $(cflags) $(cdebug) $?

$(BLD)/c_s_rule.o : $(SRC)/c_s_rule.c
	$(cc) $(ccout) $@ $(cflags) $(cdebug) $?

$(BLD)/c_s_sql.o : $(SRC)/c_s_sql.c
	$(cc) $(ccout) $@ $(cflags) $(cdebug) $?

$(BLD)/c_val.o : $(SRC)/c_val.c
	$(cc) $(ccout) $@ $(cflags) $(cdebug) $?

$(BLD)/decl_cls.o : $(SRC)/decl_cls.c
	$(cc) $(ccout) $@ $(cflags) $(cdebug) $?

$(BLD)/decl_dec.o : $(SRC)/decl_dec.c
	$(cc) $(ccout) $@ $(cflags) $(cdebug) $?

$(BLD)/decl_ent.o : $(SRC)/decl_ent.c
	$(cc) $(ccout) $@ $(cflags) $(cdebug) $?

$(BLD)/decl_ext.o : $(SRC)/decl_ext.c
	$(cc) $(ccout) $@ $(cflags) $(cdebug) $?

$(BLD)/decl_inh.o : $(SRC)/decl_inh.c
	$(cc) $(ccout) $@ $(cflags) $(cdebug) $?

$(BLD)/dyar.o : $(SRC)/dyar.c
	$(cc) $(ccout) $@ $(cflags) $(cdebug) $?

$(BLD)/hash.o : $(SRC)/hash.c
	$(cc) $(ccout) $@ $(cflags) $(cdebug) $?

$(BLD)/ios_file.o : $(SRC)/ios_file.c
	$(cc) $(ccout) $@ $(cflags) $(cdebug) $?

$(BLD)/ios_win.o : $(SRC)/ios_win.c
	$(cc) $(ccout) $@ $(cflags) $(cdebug) $?

$(BLD)/list.o : $(SRC)/list.c
	$(cc) $(ccout) $@ $(cflags) $(cdebug) $?

$(BLD)/mem.o : $(SRC)/mem.c
	$(cc) $(ccout) $@ $(cflags) $(cdebug) $?

$(BLD)/ml_comps.o : $(SRC)/ml_comps.c
	$(cc) $(ccout) $@ $(cflags) $(cdebug) $?

$(BLD)/ml_hash.o : $(SRC)/ml_hash.c
	$(cc) $(ccout) $@ $(cflags) $(cdebug) $?

$(BLD)/ml_polys.o: $(SRC)/ml_polys.c
	$(cc) $(ccout) $@ $(cflags) $(cdebug) $?

$(BLD)/ml_print.o : $(SRC)/ml_print.c
	$(cc) $(ccout) $@ $(cflags) $(cdebug) $?

$(BLD)/ml_query.o : $(SRC)/ml_query.c
	$(cc) $(ccout) $@ $(cflags) $(cdebug) $?

$(BLD)/ml_table.o : $(SRC)/ml_table.c
	$(cc) $(ccout) $@ $(cflags) $(cdebug) $?

$(BLD)/msg.o: $(SRC)/msg.c
	$(cc) $(ccout) $@ $(cflags) $(cdebug) $?

$(BLD)/beta.o : $(SRC)/beta.c
	$(cc) $(ccout) $@ $(cflags) $(cdebug) $?

$(BLD)/cb_com.o : $(SRC)/cb_com.c
	$(cc) $(ccout) $@ $(cflags) $(cdebug) $?

$(BLD)/dbg.o : $(SRC)/dbg.c
	$(cc) $(ccout) $@ $(cflags) $(cdebug) $?

$(BLD)/delta.o : $(SRC)/delta.c
	$(cc) $(ccout) $@ $(cflags) $(cdebug) $?

$(BLD)/i18n_str.o : $(SRC)/i18n_str.c
	$(cc) $(ccout) $@ $(cflags) $(cdebug) $?

$(BLD)/ml_arith.o : $(SRC)/ml_arith.c
	$(cc) $(ccout) $@ $(cflags) $(cdebug) $?

$(BLD)/ml_preds.o : $(SRC)/ml_preds.c
	$(cc) $(ccout) $@ $(cflags) $(cdebug) $?

$(BLD)/rac.o : $(SRC)/rac.c
	$(cc) $(ccout) $@ $(cflags) $(cdebug) $?

$(BLD)/rbs.o : $(SRC)/rbs.c
	$(cc) $(ccout) $@ $(cflags) $(cdebug) $?

$(BLD)/ref.o : $(SRC)/ref.c
	$(cc) $(ccout) $@ $(cflags) $(cdebug) $?

$(BLD)/states.o : $(SRC)/states.c
	$(cc) $(ccout) $@ $(cflags) $(cdebug) $?

$(BLD)/wm_iter.o : $(SRC)/wm_iter.c
	$(cc) $(ccout) $@ $(cflags) $(cdebug) $?

$(BLD)/wm_print.o : $(SRC)/wm_print.c
	$(cc) $(ccout) $@ $(cflags) $(cdebug) $?

$(BLD)/wm_query.o : $(SRC)/wm_query.c
	$(cc) $(ccout) $@ $(cflags) $(cdebug) $?

$(BLD)/wm_updat.o : $(SRC)/wm_updat.c
	$(cc) $(ccout) $@ $(cflags) $(cdebug) $?

# ******************************
# COMPILING LEX AND YACC SOURCES
# ******************************

$(BLD)/rts_scan.c	:	$(SRC)/scan.lex
	lex -o$@ -i -Pcli_ $?

$(BLD)/c_scan.c		:	$(SRC)/scan.lex
	lex -o$@ -i -Pcmp_ $?
	
$(BLD)/strip_h.c		:	$(SRC)/strip_h.lex
	lex -o$@ -i $?
	
$(BLD)/cmd_pars.c	:	$(SRC)/cmd_pars.y
	yacc $?
	mv y.tab.c $@
	
$(BLD)/c_parser.c	:	$(SRC)/c_parser.y
	yacc $?
	mv y.tab.c $@
	
$(BLD)/atomize.c	:	$(SRC)/ops_atom.lex
	lex -o$@ -i -Patom_ $?
	

#
# Sources which only require renaming.
#
$(BIN)/$(PRODUCT).1	:	$(SRC)/$(PRODUCT).1
	@ cp $? $@
