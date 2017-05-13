CCFLAGS =	-I../include


RUL =		../bin/rulework
RULFLAGS =	
RUL_LIB =	../library/librulrtl.a


all :	advent tourney count
	@ echo All Done

.SUFFIXES : 	.rul

.rul.c	:
	$(RUL) $(RULFLAGS) $<

#------------------------------------------------------------

advent :	advent.c
	$(CC) -o $@  advent.c $(CCFLAGS) $(RUL_LIB)

advent.c :	advent.rul

#------------------------------------------------------------

count :		count.c
	$(CC) -o $@  count.c $(CCFLAGS) $(RUL_LIB)

count.c :	count.rul

#------------------------------------------------------------

tourney :	tourney.c
	$(CC) -o $@  tourney.c $(CCFLAGS) $(RUL_LIB)

tourney.c :	tourney.rul
