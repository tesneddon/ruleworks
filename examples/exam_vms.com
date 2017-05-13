$!
$ arch = f$getsyi("arch_name")
$ if (arch .eqs. "VAX") 
$ then
$	lib :== "rul_rtlv.olb"
$ else 
$	lib :== "rul_rtla.olb"
$ endif
$!
$ rulework advent.rul
$ cc /INCLUDE=[-.include] advent.c
$ link advent.obj, [-.library]'lib/lib
$!
$!
$ rulework count.rul
$ cc /INCLUDE=[-.include] count.c
$ link count.obj, [-.library]'lib/lib
$!
$!
$ rulework tourney.rul
$ cc /INCLUDE=[-.include] tourney.c
$ link tourney.obj, [-.library]'lib/lib
$!
