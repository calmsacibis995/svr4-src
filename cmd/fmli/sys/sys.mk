#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#
# Copyright  (c) 1985 AT&T
#	All Rights Reserved
#
#ident	"@(#)fmli:sys/sys.mk	1.26"
#
INC=$(ROOT)/usr/include
#USRLIB=$(CCSROOT)/usr/ccs/lib
LIBRARY=libsys.a
HEADER1=../inc
CURSES_H=$(INC)
INCLUDE=	-I$(HEADER1) -I$(CURSES_H)

AR=		ar
CFLAGS = 	-O

$(LIBRARY):  \
		$(LIBRARY)(actrec.o) \
		$(LIBRARY)(ar_mfuncs.o) \
		$(LIBRARY)(backslash.o) \
		$(LIBRARY)(chgenv.o) \
		$(LIBRARY)(chgepenv.o) \
		$(LIBRARY)(compile.o) \
		$(LIBRARY)(coproc.o) \
		$(LIBRARY)(copyfile.o) \
		$(LIBRARY)(estrtok.o) \
		$(LIBRARY)(evfuncs.o) \
		$(LIBRARY)(eval.o) \
		$(LIBRARY)(evalstr.o) \
		$(LIBRARY)(exit.o) \
		$(LIBRARY)(expand.o) \
		$(LIBRARY)(expr.o) \
		$(LIBRARY)(filename.o) \
		$(LIBRARY)(genfind.o) \
		$(LIBRARY)(getaltenv.o) \
		$(LIBRARY)(getepenv.o) \
		$(LIBRARY)(grep.o) \
		$(LIBRARY)(io.o) \
		$(LIBRARY)(itoa.o) \
		$(LIBRARY)(memshift.o) \
		$(LIBRARY)(mencmds.o) \
		$(LIBRARY)(cut.o) \
		$(LIBRARY)(nstrcat.o) \
		$(LIBRARY)(onexit.o) \
		$(LIBRARY)(parent.o) \
		$(LIBRARY)(putaltenv.o) \
		$(LIBRARY)(readfile.o) \
		$(LIBRARY)(scrclean.o) \
		$(LIBRARY)(spawn.o) \
		$(LIBRARY)(strappend.o) \
		$(LIBRARY)(strCcmp.o) \
		$(LIBRARY)(stream.o) \
		$(LIBRARY)(strsave.o) \
		$(LIBRARY)(tempfiles.o) \
		$(LIBRARY)(terrmess.o) \
		$(LIBRARY)(terror.o) \
		$(LIBRARY)(test.o) \
		$(LIBRARY)(varappend.o) \
		$(LIBRARY)(varchkapnd.o) \
		$(LIBRARY)(varcreate.o) \
		$(LIBRARY)(vardelete.o) \
		$(LIBRARY)(vargrow.o) \
		$(LIBRARY)(varinsert.o) \
		$(LIBRARY)(varshrink.o) \
		$(LIBRARY)(watch.o)

$(LIBRARY)(actrec.o): $(HEADER1)/actrec.h
$(LIBRARY)(actrec.o): $(HEADER1)/ctl.h
$(LIBRARY)(actrec.o): $(HEADER1)/menudefs.h
$(LIBRARY)(actrec.o): $(HEADER1)/message.h
$(LIBRARY)(actrec.o): $(HEADER1)/moremacros.h
$(LIBRARY)(actrec.o): $(HEADER1)/slk.h
$(LIBRARY)(actrec.o): $(HEADER1)/terror.h
$(LIBRARY)(actrec.o): $(HEADER1)/token.h
$(LIBRARY)(actrec.o): $(HEADER1)/wish.h
$(LIBRARY)(actrec.o): actrec.c

$(LIBRARY)(ar_mfuncs.o): $(HEADER1)/actrec.h
$(LIBRARY)(ar_mfuncs.o): $(HEADER1)/slk.h
$(LIBRARY)(ar_mfuncs.o): $(HEADER1)/token.h
$(LIBRARY)(ar_mfuncs.o): $(HEADER1)/wish.h
$(LIBRARY)(ar_mfuncs.o): ar_mfuncs.c

$(LIBRARY)(backslash.o): $(HEADER1)/wish.h
$(LIBRARY)(backslash.o): backslash.c

$(LIBRARY)(chgenv.o): chgenv.c

$(LIBRARY)(chgepenv.o): $(HEADER1)/sizes.h
$(LIBRARY)(chgepenv.o): chgepenv.c

$(LIBRARY)(compile.o): compile.c

$(LIBRARY)(coproc.o): $(HEADER1)/eval.h
$(LIBRARY)(coproc.o): $(HEADER1)/moremacros.h
$(LIBRARY)(coproc.o): $(HEADER1)/sizes.h
$(LIBRARY)(coproc.o): $(HEADER1)/terror.h
$(LIBRARY)(coproc.o): $(HEADER1)/var_arrays.h
$(LIBRARY)(coproc.o): $(HEADER1)/wish.h
$(LIBRARY)(coproc.o): coproc.c

$(LIBRARY)(copyfile.o): $(HEADER1)/exception.h
$(LIBRARY)(copyfile.o): $(HEADER1)/wish.h
$(LIBRARY)(copyfile.o): copyfile.c

$(LIBRARY)(cut.o): $(HEADER1)/ctl.h
$(LIBRARY)(cut.o): $(HEADER1)/eval.h
$(LIBRARY)(cut.o): $(HEADER1)/message.h
$(LIBRARY)(cut.o): $(HEADER1)/moremacros.h
$(LIBRARY)(cut.o): $(HEADER1)/sizes.h
$(LIBRARY)(cut.o): $(HEADER1)/wish.h
$(LIBRARY)(cut.o): cut.c

$(LIBRARY)(estrtok.o): estrtok.c

$(LIBRARY)(eval.o): $(HEADER1)/eval.h
$(LIBRARY)(eval.o): $(HEADER1)/interrupt.h
$(LIBRARY)(eval.o): $(HEADER1)/message.h
$(LIBRARY)(eval.o): $(HEADER1)/moremacros.h
$(LIBRARY)(eval.o): $(HEADER1)/terror.h
$(LIBRARY)(eval.o): $(HEADER1)/wish.h
$(LIBRARY)(eval.o): eval.c

$(LIBRARY)(evalstr.o): $(HEADER1)/eval.h
$(LIBRARY)(evalstr.o): $(HEADER1)/wish.h
$(LIBRARY)(evalstr.o): evalstr.c

$(LIBRARY)(evfuncs.o): $(HEADER1)/ctl.h
$(LIBRARY)(evfuncs.o): $(HEADER1)/eval.h
$(LIBRARY)(evfuncs.o): $(HEADER1)/interrupt.h
$(LIBRARY)(evfuncs.o): $(HEADER1)/message.h
$(LIBRARY)(evfuncs.o): $(HEADER1)/moremacros.h
$(LIBRARY)(evfuncs.o): $(HEADER1)/retcodes.h
$(LIBRARY)(evfuncs.o): $(HEADER1)/wish.h
$(LIBRARY)(evfuncs.o): $(HEADER1)/sizes.h
$(LIBRARY)(evfuncs.o): evfuncs.c

$(LIBRARY)(exit.o): $(HEADER1)/retcodes.h
$(LIBRARY)(exit.o): $(HEADER1)/var_arrays.h
$(LIBRARY)(exit.o): $(HEADER1)/wish.h
$(LIBRARY)(exit.o): exit.c

$(LIBRARY)(expand.o): $(HEADER1)/moremacros.h
$(LIBRARY)(expand.o): $(HEADER1)/sizes.h
$(LIBRARY)(expand.o): $(HEADER1)/terror.h
$(LIBRARY)(expand.o): $(HEADER1)/wish.h
$(LIBRARY)(expand.o): expand.c

$(LIBRARY)(expr.o): $(HEADER1)/eval.h
$(LIBRARY)(expr.o): $(HEADER1)/wish.h
$(LIBRARY)(expr.o): expr.c

$(LIBRARY)(filename.o): filename.c

$(LIBRARY)(genfind.o): $(HEADER1)/eval.h
$(LIBRARY)(genfind.o): $(HEADER1)/partabdefs.h
$(LIBRARY)(genfind.o): $(HEADER1)/sizes.h
$(LIBRARY)(genfind.o): $(HEADER1)/typetab.h
$(LIBRARY)(genfind.o): $(HEADER1)/var_arrays.h
$(LIBRARY)(genfind.o): $(HEADER1)/wish.h
$(LIBRARY)(genfind.o): genfind.c
	$(CC) $(CFLAGS) $(INCLUDE) -c $<
	$(AR) rv $@ genfind.o
	/bin/rm -f genfind.o

$(LIBRARY)(getaltenv.o): $(HEADER1)/var_arrays.h
$(LIBRARY)(getaltenv.o): $(HEADER1)/wish.h
$(LIBRARY)(getaltenv.o): getaltenv.c

$(LIBRARY)(getepenv.o): $(HEADER1)/moremacros.h
$(LIBRARY)(getepenv.o): $(HEADER1)/sizes.h
$(LIBRARY)(getepenv.o): $(HEADER1)/wish.h
$(LIBRARY)(getepenv.o): getepenv.c

$(LIBRARY)(grep.o): $(HEADER1)/ctl.h
$(LIBRARY)(grep.o): $(HEADER1)/eval.h
$(LIBRARY)(grep.o): $(HEADER1)/message.h
$(LIBRARY)(grep.o): $(HEADER1)/moremacros.h
$(LIBRARY)(grep.o): $(HEADER1)/wish.h
$(LIBRARY)(grep.o): grep.c

$(LIBRARY)(io.o): $(HEADER1)/eval.h
$(LIBRARY)(io.o): $(HEADER1)/moremacros.h
$(LIBRARY)(io.o): $(HEADER1)/terror.h
$(LIBRARY)(io.o): $(HEADER1)/wish.h
$(LIBRARY)(io.o): io.c

$(LIBRARY)(itoa.o): itoa.c

$(LIBRARY)(memshift.o): $(HEADER1)/wish.h
$(LIBRARY)(memshift.o): memshift.c

$(LIBRARY)(mencmds.o): $(HEADER1)/ctl.h
$(LIBRARY)(mencmds.o): $(HEADER1)/eval.h
$(LIBRARY)(mencmds.o): $(HEADER1)/message.h
$(LIBRARY)(mencmds.o): $(HEADER1)/moremacros.h
$(LIBRARY)(mencmds.o): $(HEADER1)/procdefs.h
$(LIBRARY)(mencmds.o): $(HEADER1)/sizes.h
$(LIBRARY)(mencmds.o): $(HEADER1)/terror.h
$(LIBRARY)(mencmds.o): $(HEADER1)/typetab.h
$(LIBRARY)(mencmds.o): $(HEADER1)/wish.h
$(LIBRARY)(mencmds.o): mencmds.c
	$(CC) $(CFLAGS) $(INCLUDE) -c $<
	$(AR) rv $@ mencmds.o
	/bin/rm -f mencmds.o

$(LIBRARY)(nstrcat.o): $(HEADER1)/sizes.h
$(LIBRARY)(nstrcat.o): nstrcat.c

$(LIBRARY)(onexit.o): $(HEADER1)/var_arrays.h
$(LIBRARY)(onexit.o): $(HEADER1)/wish.h
$(LIBRARY)(onexit.o): onexit.c

$(LIBRARY)(parent.o): $(HEADER1)/sizes.h
$(LIBRARY)(parent.o): $(HEADER1)/wish.h
$(LIBRARY)(parent.o): parent.c

$(LIBRARY)(putaltenv.o): $(HEADER1)/moremacros.h
$(LIBRARY)(putaltenv.o): $(HEADER1)/var_arrays.h
$(LIBRARY)(putaltenv.o): $(HEADER1)/wish.h
$(LIBRARY)(putaltenv.o): putaltenv.c

$(LIBRARY)(readfile.o): $(HEADER1)/terror.h
$(LIBRARY)(readfile.o): $(HEADER1)/wish.h
$(LIBRARY)(readfile.o): readfile.c

$(LIBRARY)(scrclean.o): scrclean.c

$(LIBRARY)(spawn.o): $(HEADER1)/moremacros.h
$(LIBRARY)(spawn.o): $(HEADER1)/sizes.h
$(LIBRARY)(spawn.o): $(HEADER1)/wish.h
$(LIBRARY)(spawn.o): spawn.c

$(LIBRARY)(strCcmp.o): strCcmp.c

$(LIBRARY)(strappend.o): $(HEADER1)/var_arrays.h
$(LIBRARY)(strappend.o): $(HEADER1)/wish.h
$(LIBRARY)(strappend.o): strappend.c

$(LIBRARY)(stream.o): $(HEADER1)/token.h
$(LIBRARY)(stream.o): $(HEADER1)/wish.h
$(LIBRARY)(stream.o): stream.c

$(LIBRARY)(strsave.o): $(HEADER1)/wish.h
$(LIBRARY)(strsave.o): strsave.c

$(LIBRARY)(tempfiles.o): $(HEADER1)/moremacros.h
$(LIBRARY)(tempfiles.o): $(HEADER1)/retcodes.h
$(LIBRARY)(tempfiles.o): $(HEADER1)/terror.h
$(LIBRARY)(tempfiles.o): $(HEADER1)/var_arrays.h
$(LIBRARY)(tempfiles.o): $(HEADER1)/wish.h
$(LIBRARY)(tempfiles.o): tempfiles.c

$(LIBRARY)(terrmess.o): $(HEADER1)/terror.h
$(LIBRARY)(terrmess.o): $(HEADER1)/wish.h
$(LIBRARY)(terrmess.o): terrmess.c

$(LIBRARY)(terror.o): $(HEADER1)/message.h
$(LIBRARY)(terror.o): $(HEADER1)/retcodes.h
$(LIBRARY)(terror.o): $(HEADER1)/sizes.h
$(LIBRARY)(terror.o): $(HEADER1)/terror.h
$(LIBRARY)(terror.o): $(HEADER1)/vtdefs.h
$(LIBRARY)(terror.o): $(HEADER1)/wish.h
$(LIBRARY)(terror.o): terror.c

$(LIBRARY)(test.o): $(HEADER1)/wish.h
$(LIBRARY)(test.o): ./test.h
$(LIBRARY)(test.o): test.c

$(LIBRARY)(varappend.o): $(HEADER1)/var_arrays.h
$(LIBRARY)(varappend.o): $(HEADER1)/wish.h
$(LIBRARY)(varappend.o): varappend.c

$(LIBRARY)(varchkapnd.o): $(HEADER1)/var_arrays.h
$(LIBRARY)(varchkapnd.o): $(HEADER1)/wish.h
$(LIBRARY)(varchkapnd.o): varchkapnd.c

$(LIBRARY)(varcreate.o): $(HEADER1)/var_arrays.h
$(LIBRARY)(varcreate.o): $(HEADER1)/wish.h
$(LIBRARY)(varcreate.o): varcreate.c

$(LIBRARY)(vardelete.o): $(HEADER1)/var_arrays.h
$(LIBRARY)(vardelete.o): $(HEADER1)/wish.h
$(LIBRARY)(vardelete.o): vardelete.c

$(LIBRARY)(vargrow.o): $(HEADER1)/var_arrays.h
$(LIBRARY)(vargrow.o): $(HEADER1)/wish.h
$(LIBRARY)(vargrow.o): vargrow.c

$(LIBRARY)(varinsert.o): $(HEADER1)/var_arrays.h
$(LIBRARY)(varinsert.o): $(HEADER1)/wish.h
$(LIBRARY)(varinsert.o): varinsert.c

$(LIBRARY)(varshrink.o): $(HEADER1)/var_arrays.h
$(LIBRARY)(varshrink.o): $(HEADER1)/wish.h
$(LIBRARY)(varshrink.o): varshrink.c

$(LIBRARY)(watch.o): $(HEADER1)/wish.h
$(LIBRARY)(watch.o): watch.c


.c.a:
	$(CC) -c $(CFLAGS) $(INCLUDE) $<
	$(AR) rv $@ $*.o
	/bin/rm -f $*.o

####### Standard makefile targets ########

all:		$(LIBRARY)

install:	all

clean:
		/bin/rm -f *.o

clobber:	clean
		/bin/rm -f $(LIBRARY)

profile:	libprof.a
#
# PROFILING: ---------------------------------------------
#  method 0: ignore the exit and suffer the conseqences
#
VERDICT = libp0

libp0.a:  libsys.a noeggs.o
	/bin/rm -f $@; cp libsys.a $@;
	ar d $@ exit.o
	ar ruv $@ noeggs.o
noeggs.c: exit.c
	sed 's/^exit/_noeggs/' $? >$@
#	
#  method 1: Rename the profiling and unix exits with
#      one extra leading under-bar.
# the library itself may not be profiled, but it doesnt
#   need the "exit" module; "exit" has to be recrafted
#   from csu/mcrt0.s
#
# VERDICT = libp1
libprof.a:	$(VERDICT).a
	/bin/rm -f $@; ln $? $@
#
# mcrt0 is viewmaster's profiling "crt0" file
# 
libp1.a:	libsys.a mcrt0.o vexit.o
	cp libsys.a $@
	ar ruv $@ vexit.o
#
#  machine, libc, environment tools
#
MACH	= u3b
LIBC	= /usr/src/lib/libc
M_LIB	= ${LIBC}/${MACH}
M4	= m4 ${M_LIB}/m4.def	
MCDEF	= ${M_LIB}/mcount.def
#
# turn exit into _exit and _exit into __exit
#
CRT0	= ${M_LIB}/csu/mcrt0.s
MCOUNT	= ${M_LIB}/crt/mcount.s
mcrt0.s: ${CRT0} 
	sed 's/exit/_exit/g' $?|$(M4) $(MCDEF) ->$@
vexit.s: ${M_LIB}/sys/exit.s
	sed 's/exit/_exit/g' $? |$(M4) $(MCDEF) -> $@

#
#  method 2. define a cleanup routine from the 
#   local exit routine and the unix cleanup (stdio/flsbuf.c)
#
libp2.a:	libsys.a v_clean.o
	/bin/rm -f $@; cp libsys.a $@; ar d libsys.a exit.0
	ar ruv libsys.a v_clean.o
#
#  ONLY when L*PROFiling, 
# FACE Cleanup ={ port/stdio/flsbuf.c exit.c }{ ... }.
# where _cleanup => __cleanup
#  &    exit     => _cleanup
#		
v_clean.c:	exit.c ${LIBC}/port/stdio/flsbuf.c sys.mk
	(echo "void _cleanup();" ;\
	 sed -e 's/^exit.*/cleanup(n)/'\
	     -e '/_exit/d' exit.c;\
	 echo "void";\
	 sed -n '/cleanup/,/}/p' ${LIBC}/port/stdio/flsbuf.c;\
	)|\
	 sed -e 's/cleanup/_&/g' >$@
#

.PRECIOUS:	$(LIBRARY)
