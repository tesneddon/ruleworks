
!include <ntwin32.mak>

RUL 		= ..\bin\rulework.exe		# Command name of RuleWorks compiler
RULRTL 		= ..\library\rul_rtlm.lib	# Syntax for including run-time library in link
RULFLAGS	= -q

cflags		= $(cflags) /I..\include


all :	advent.exe tourney.exe count.exe
	@ echo All Done

.SUFFIXES : 	.rul

.rul.c	:
	$(RUL) $(RULFLAGS) $<

#------------------------------------------------------------

advent.exe :	advent.obj
	$(link) /OUT:$@ $** $(RULRTL) $(guilibs)

advent.obj :	advent.c
	$(cc) /c $(cflags) /Fo$@  $** 

advent.c :	advent.rul

#------------------------------------------------------------

count.exe :	count.obj
	$(link) /OUT:$@ $** $(RULRTL) $(guilibs)

count.obj :	count.c
	$(cc) /c $(cflags) /Fo$@  $** 

count.c :	count.rul

#------------------------------------------------------------

tourney.exe :	tourney.obj
	$(link) /OUT:$@ $** $(RULRTL) $(guilibs)

tourney.obj :	tourney.c
	$(cc) /c $(cflags) /Fo$@  $** 

tourney.c :	tourney.rul
