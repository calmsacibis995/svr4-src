#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1988 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)libc-port:gen/makefile	1.17.11.12"
#
# makefile for libc/port/gen
#
#

.SUFFIXES: .p .P .A
CC=cc
CFLAGS= -O
PROF=
NONPROF=
PIC=
ABI=
ROOT=
INC=$(ROOT)/usr/include
INCSYS=$(ROOT)/usr/include
INCLIBC= ../../inc
DEFLIST=
SDEFLIST=
INCLIST=-I$(INCLIBC)
AWK=awk

POBJECTS=$(OBJECTS:.o=.p)

OBJECTS=\
_ctype.o      _loc_data.o   _locale.o     _set_tab.o    \
_shrd_data.o  _xftw.o       \
a64l.o        abort.o       abs.o         \
assert.o      atexit.o      atof.o        atoi.o        \
atol.o        bsearch.o	    calloc.o      \
cat_init.o    catclose.o    catgets.o     catopen.o     \
cfgetispeed.o cfgetospeed.o cfree.o       cfsetispeed.o cfsetospeed.o	\
cftime.o      clock.o       closedir.o    \
colldata.o    crypt.o	\
ctime.o	      ctype.o	    \
ctypefcns.o   div.o         drand48.o	\
dup2.o	      ecvt.o        errlst.o      \
execvp.o      fattach.o     fdetach.o	\
ffs.o         fmtmsg.o	    \
frexp.o	      ftok.o        ftw.o         gcvt.o	\
getcwd.o      getdate.o     getdate_data.o              \
getdate_gd.o  getenv.o      getgrent.o    \
getgrnam.o    gethz.o       getlogin.o    getmntent.o   \
getopt.o      getpw.o       getpwent.o    getpwnam.o	\
getsetitimer.o		  \
getspent.o    getsubopt.o   gettimeofday.o	\
gettxt.o      getut.o	    getutx.o      getvfsent.o   \
hrtcftime.o   hrtctime.o    hrtnewres.o   hrtstrftime.o	\
hsearch.o     initgroups.o  insque.o	  isastream.o	\
isatty.o      l3.o          l64a.o        lckpwdf.o	\
ldexp.o       lfind.o	    localeconv.o  lsearch.o     \
malloc.o      \
mbtowc.o      mbstowcs.o    memalign.o    memccpy.o	\
memchr.o      memcmp.o      memcpy.o      memmove.o	\
memset.o      mkdev.o       mkfifo.o      mktemp.o	\
mlock.o	      mlockall.o    \
modf.o        modff.o       mon.o         msync.o	\
munlock.o     munlockall.o  new_list.o    nftw.o	  \
nl_langinfo.o opendir.o     \
old_tolower.o old_toupper.o opt_data.o    \
perror.o      priocntl.o    psiginfo.o    psignal.o	\
pt.o          putenv.o      putpwent.o    \
qsort.o       raise.o       rand.o        readdir.o	\
realpath.o    rename.o      rewinddir.o   seekdir.o	\
select.o      setlocale.o   \
sigflag.o     siglist.o     \
sigsend.o     sigsetops.o   sleep.o	\
ssignal.o     str2id.o      str2sig.o     \
strcat.o      strchr.o      strcmp.o      strcpy.o	\
strcspn.o     strdup.o      strerror.o    strftime.o	\
strlen.o      strncat.o     strncmp.o     \
strncpy.o     strpbrk.o     strrchr.o     strspn.o	\
strstr.o      strtod.o      strtok.o      strtol.o	\
strtoul.o     strxfrm.o	    swab.o	  swapctl.o	\
sysconf.o     syslog.o      \
tcdrain.o     tcflow.o      tcflush.o     tcgetattr.o	\
tcgetpgrp.o   tcgetsid.o    tcsendbreak.o tcsetattr.o	\
tcsetpgrp.o   tell.o        telldir.o     tfind.o	\
time_comm.o   time_data.o   time_gdata.o  \
tolower.o     toupper.o     truncate.o	\
tsearch.o     ttyname.o	    ttyslot.o     valloc.o	\
values-Xt.o   values-Xa.o   values-Xc.o   \
waitpid.o     wctomb.o      wcstombs.o

PIC_OBJECTS=\
_ctype.P      _loc_data.P   _locale.P     _set_tab.P    _xftw.P       \
a64l.P        abort.P       abs.P         assert.P      atexit.P      \
atof.P        atoi.P        atol.P        bsearch.P     calloc.P      \
cat_init.P    catclose.P    catgets.P     catopen.P     \
cfgetispeed.P cfgetospeed.P cfsetispeed.P cfsetospeed.P	\
clock.P       closedir.P    ctime.P       \
ctypefcns.P   div.P         drand48.P     \
dup2.P        ecvt.P        execvp.P      \
fattach.P     fdetach.P     ffs.P         fmtmsg.P      \
frexp.P       ftok.P        ftw.P         getcwd.P      \
getdate.P     getdate_gd.P  getenv.P      getgrent.P    getgrnam.P    \
gethz.P       getlogin.P    getopt.P      getpwent.P    \
getpwnam.P    getsubopt.P   gettimeofday.P gettxt.P     getut.P       \
hsearch.P     import_def.P  initgroups.P  isatty.P      isastream.P   \
l64a.P        ldexp.P       lfind.P       localeconv.P  lsearch.P     \
malloc.P      mbstowcs.P    mbtowc.P      memccpy.P     \
memchr.P      memcmp.P      memcpy.P      memmove.P     \
memset.P      mkdev.P       mkfifo.P      mktemp.P      \
mlock.P       modf.P        mon.P         msync.P       munlock.P     \
nftw.P        nl_langinfo.P \
opendir.P     opt_data.P    perror.P      \
priocntl.P    pt.P          putenv.P      \
putpwent.P    qsort.P       \
raise.P       rand.P        readdir.P     rename.P      \
rewinddir.P   seekdir.P     select.P      setlocale.P   sigsend.P     \
sigsetops.P   sleep.P       strcat.P      strchr.P      \
strcmp.P      strcpy.P      strcspn.P     strdup.P      \
strerror.P    strftime.P    strlen.P      strncat.P     \
strncmp.P     strncpy.P     strpbrk.P     strrchr.P     \
strspn.P      strstr.P      strtod.P      strtok.P      \
strtol.P      strtoul.P     strxfrm.P     swab.P        \
swapctl.P     sysconf.P     syslog.P      \
tcdrain.P     tcflow.P      tcflush.P     tcgetattr.P	\
tcgetpgrp.P   tcgetsid.P    tcsendbreak.P tcsetattr.P	\
tcsetpgrp.P   tell.P        telldir.P     \
tfind.P       time_comm.P   time_gdata.P  tolower.P     \
toupper.P     truncate.P    tsearch.P     ttyname.P     \
ttyslot.P     waitpid.P     wcstombs.P    wctomb.P

ABI_OBJECTS=$(PIC_OBJECTS:.P=.A)

nonprof: $(OBJECTS)

prof: $(POBJECTS)

pic: $(PIC_OBJECTS)

abi: $(ABI_OBJECTS)

_ctype.o _ctype.p atof.o atof.p \
	ctype.o ctype.p ctypefcns.o ctypefcns.p \
	getpw.o getpw.p mbtowc.o mbtowc.p \
	strftime.o strftime.p \
	strtol.o strtol.p strtoul.o strtoul.p \
	wctomb.o wctomb.p: $(INC)/ctype.h
ftw.o ftw.p _xftw.o _xftw.p \
	getut.o getut.p getutx.o getutx.p strtol.o strtol.p \
	strtoul.o strtoul.p: $(INC)/errno.h $(INCSYS)/sys/errno.h
ftw.o ftw.p _xftw.o _xftw.p: $(INC)/ftw.h
strxfrm.o strxfrm.p: $(INC)/stdio.h $(INC)/locale.h $(INC)/stddef.h \
	$(INC)/stdlib.h $(INC)/string.h $(INCSYS)/sys/types.h $(INCSYS)/sys/stat.h
syslog.o syslog.p: $(INCSYS)/sys/types.h $(INC)/unistd.h $(INCSYS)/sys/stropts.h \
		$(INCSYS)/sys/strlog.h $(INCSYS)/sys/syslog.h $(INC)/signal.h \
		$(INC)/fcntl.h $(INC)/string.h $(INC)/varargs.h
localeconv.o localeconv.p: $(INC)/stdio.h $(INC)/locale.h $(INC)/stddef.h \
	$(INC)/stdlib.h $(INC)/string.h $(INCSYS)/sys/types.h $(INCSYS)/sys/stat.h \
	$(INC)/limits.h
dup2.o dup2.p _set_tab.o _set_tab.p ctype.o ctype.p \
	strftime.o strftime.p: $(INC)/fcntl.h
getgrent.o getgrent.p \
	getgrnam.o getgrnam.p: $(INC)/grp.h
dup2.o dup2.p strtol.o strtol.p strtoul.o strtoul.p: $(INC)/limits.h
mon.o mon.p: $(INC)/mon.h $(INCSYS)/sys/types.h $(INC)/stdlib.h $(INC)/stdio.h \
		$(INC)/fcntl.h $(INC)/string.h
modf.o modf.p modff.o modff.p raise.o raise.p: $(INCSYS)/sys/types.h \
		$(INC)/signal.h $(INC)/unistd.h
ecvt.o ecvt.p frexp.o frexp.p ldexp.o ldexp.p \
	modf.o modf.p: $(INC)/nan.h
getpwent.o getpwent.p getpwnam.o getpwnam.p \
	putpwent.o putpwent.p: $(INC)/search.h
tsearch.o tsearch.p tfind.o tfind.p: $(INC)/search.h
abort.o abort.p sleep.o sleep.p \
	ssignal.o ssignal.p: $(INC)/signal.h \
		$(INCSYS)/sys/signal.h
_locale.o _locale.p \
	_set_tab.o _set_tab.p \
	drand48.o drand48.p getcwd.o getcwd.p \
	getgrent.o getgrent.p getopt.o getopt.p \
	getpw.o getpw.p getpwent.o getpwent.p \
	getut.o getut.p getutx.o getutx.p \
	hsearch.o hsearch.p \
	memmove.o memmove.p \
	putpwent.o putpwent.p \
	setlocale.o setlocale.p \
	strerror.o strerror.p \
	strftime.o strftime.p \
	strstr.o strstr.p: $(INC)/string.h
strdup.o strdup.p: $(INC)/string.h
ftw.o ftw.p _xftw.o _xftw.p ttyname.o ttyname.p: $(INCSYS)/sys/fs/s5dir.h
execvp.o execvp.p getcwd.o getcwd.p: $(INCSYS)/sys/errno.h
clock.o clock.p getpwent.o getpwent.p: $(INCSYS)/sys/param.h
ftok.o ftok.p ftw.o ftw.p _xftw.o _xftw.p getut.o getut.p getutx.o getutx.p \
	opendir.o opendir.p strftime.o strftime.p	\
	ttyname.o ttyname.p: $(INCSYS)/sys/stat.h
ftok.o ftok.p: $(INCSYS)/sys/types.h $(INCSYS)/sys/mkdev.h
mktemp.o mktemp.p: $(INCSYS)/sys/types.h $(INC)/unistd.h $(INC)/string.h
isatty.o isatty.p: $(INCSYS)/sys/termio.h
clock.o clock.p: $(INCSYS)/sys/times.h
ctime.o ctime.p strftime.o strftime.p: $(INC)/time.h
getlogin.o getlogin.p getut.o getut.p getutx.o getutx.p \
	ttyslot.o ttyslot.p: $(INC)/utmp.h
atof.o atof.p ecvt.o ecvt.p l64a.o l64a.p modff.o modff.p\
	ldexp.o ldexp.p modf.o modf.p strtod.o strtod.p: $(INC)/values.h
closedir.o closedir.p opendir.o opendir.p       \
	readdir.o readdir.p rewinddir.o rewinddir.p seekdir.o seekdir.p \
	telldir.o telldir.p: $(INCSYS)/sys/dirent.h
abs.o atof.o atoi.o atol.o calloc.o ecvt.o \
	frexp.o gcvt.o getenv.o getopt.o \
	isatty.o ldexp.o malloc.o mktemp.o :$(INCLIBC)/shlib.h
_locale.o _locale.p div.o div.p mbtowc.o mbtowc.p \
	mbstowcs.o mbstowcs.p strftime.o strftime.p \
	wctomb.o wctomb.p wcstombs.o wcstombs.p: $(INC)/stdlib.h
strftime.o strftime.p: $(INCSYS)/sys/types.h
strftime.o strftime.p \
	strstr.o strstr.p: $(INC)/stddef.h
_set_tab.o _set_tab.p: $(INC)/unistd.h
_loc_data.o _loc_data.p \
_locale.o _locale.p _set_tab.o _set_tab.p \
setlocale.o setlocale.p strftime.o strftime.p \
	_shrd_data.o _shrd_data.p: $(INC)/locale.h
mbtowc.o mbtowc.p wctomb.o wctomb.p: _wchar.h

strxfrm.o strxfrm.p colldata.o colldata.p: colldata.h

cfsetispeed.o cfsetispeed.p: $(INCSYS)/sys/termios.h

cfsetospeed.o cfsetospeed.p: $(INCSYS)/sys/termios.h

cfgetispeed.o cfgetispeed.p: $(INCSYS)/sys/termios.h

cfgetospeed.o cfgetospeed.p: $(INCSYS)/sys/termios.h

getmntent.o getmntent.p: $(INC)/stdio.h $(INCSYS)/sys/types.h $(INCSYS)/sys/stat.h\
	$(INCSYS)/sys/mnttab.h

getspent.o getspent.p: $(INCSYS)/sys/param.h $(INCSYS)/sys/types.h\
	$(INC)/stdio.h $(INC)/shadow.h $(INC)/string.h

getsubopt.o getsubopt.p: $(INC)/string.h $(INC)/stdio.h

getvfsent.o getvfsent.p: $(INC)/stdio.h $(INCSYS)/sys/types.h $(INCSYS)/sys/stat.h\
	$(INCSYS)/sys/vfstab.h

fmtmsg.o fmtmsg.p:  $(INC)/fmtmsg.h $(INC)/stdio.h $(INC)/string.h\
	$(INC)/fcntl.h $(INC)/errno.h

gettxt.o gettxt.p:  $(INC)/ctype.h $(INC)/string.h $(INC)/locale.h\
	$(INC)/fcntl.h $(INCSYS)/sys/types.h $(INCSYS)/sys/file.h\
	$(INCSYS)/sys/mman.h $(INCSYS)/sys/stat.h

hrtctime.o: $(INC)/time.h $(INC)/stdio.h $(INCSYS)/sys/types.h\
	$(INCSYS)/sys/dl.h $(INCSYS)/sys/evecb.h $(INCSYS)/sys/hrtcntl.h

hrtcftime.o: $(INC)/time.h $(INC)/limits.h $(INC)/stdio.h $(INCSYS)/sys/types.h\
	$(INCSYS)/sys/dl.h $(INCSYS)/sys/evecb.h $(INCSYS)/sys/hrtcntl.h

hrtnewres.o: $(INCSYS)/sys/types.h\
	$(INCSYS)/sys/dl.h $(INCSYS)/sys/evecb.h $(INCSYS)/sys/hrtcntl.h

hrtstrftime.o: $(INC)/fcntl.h $(INC)/time.h $(INC)/locale.h\
	$(INC)/ctype.h $(INC)/stddef.h $(INCSYS)/sys/types.h $(INCSYS)/sys/stat.h\
	$(INCSYS)/sys/dl.h $(INCSYS)/sys/evecb.h $(INCSYS)/sys/hrtcntl.h


initgroups.o initgroups.p: $(INC)/stdio.h $(INC)/grp.h

isastream.o isastream.p: $(INC)/stdio.h $(INCSYS)/sys/fcntl.h

lckpwdf.o lckpwdf.p: $(INC)/stdio.h $(INC)/signal.h $(INC)/fcntl.h

mkfifo.o mkfifo.p:

msync.o msync.p: $(INCSYS)/sys/types.h $(INCSYS)/sys/mman.h

nftw.o nftw.p: $(INCSYS)/sys/types.h $(INCSYS)/sys/stat.h $(INC)/dirent.h\
	$(INC)/errno.h  $(INC)/limits.h $(INC)/ftw.h

psignal.o psignal.p: $(INC)/signal.h

psiginfo.o psiginfo.p: $(INC)/signal.h $(INC)/siginfo.h

select.o select.p: $(INC)/values.h $(INC)/errno.h $(INCSYS)/sys/types.h \
	$(INCSYS)/sys/time.h $(INCSYS)/sys/poll.h

sigflag.o sigflag.p: $(INC)/signal.h

siglist.o siglist.p: $(INC)/signal.h

sigsend.o sigsend.p: $(INCSYS)/sys/types.h $(INCSYS)/sys/procset.h

str2id.o str2id.p: $(INCSYS)/sys/types.h $(INCSYS)/sys/procset.h

siglist.o siglist.p: $(INC)/signal.h

str2sig.o str2sig.p: $(INC)/signal.h

fattach.o fattach.p: $(INCSYS)/sys/errno.h $(INC)/stdio.h $(INCSYS)/sys/mount.h\
	 $(INCSYS)/sys/types.h $(INCSYS)/sys/vnode.h $(INCSYS)/sys/fs/namenode.h

fdetach.o fdetach.p: $(INCSYS)/sys/fcntl.h $(INC)/stdio.h

tcdrain.o tcdrain.p: $(INCSYS)/sys/termios.h

tcflow.o tcflow.p: $(INCSYS)/sys/termios.h $(INC)/errno.h

tcflush.o tcflush.p: $(INCSYS)/sys/termios.h $(INC)/errno.h

tcgetattr.o tcgetattr.p: $(INCSYS)/sys/termios.h

tcgetpgrp.o tcgetpgrp.p: $(INCSYS)/sys/types.h $(INCSYS)/sys/termios.h

tcgetsid.o tcgetsid.p: $(INC)/errno.h $(INCSYS)/sys/types.h $(INCSYS)/sys/termios.h

tcsendbreak.o tcsendbreak.p: $(INCSYS)/sys/termios.h

tcsetattr.o tcsetattr.p: $(INCSYS)/sys/termios.h $(INC)/errno.h

tcsetpgrp.o tcsetpgrp.p: $(INCSYS)/sys/types.h $(INCSYS)/sys/termios.h

truncate.o truncate.p: $(INCSYS)/sys/types.h $(INC)/fcntl.h $(INC)/stdio.h

ttyname.o ttyname.p: $(INC)/stdio.h $(INCSYS)/sys/types.h $ $(INC)/dirent.h\
	$(INCSYS)/sys/stat.h $(INC)/ctype.h $(INC)/string.h

waitpid.o waitpid.p: $(INCSYS)/sys/types.h $(INCSYS)/sys/procset.h $(INCSYS)/sys/wait.h\
	$(INC)/errno.h

swapctl.o swapctl.p: $(INCSYS)/sys/uadmin.h

sysconf.o sysconf.p: $(INCSYS)/sys/types.h $(INC)/unistd.h $(INCSYS)/sys/sysconfig.h\
	$(INCSYS)/sys/errno.h $(INC)/limits.h $(INC)/time.h

getsetitimer.o getsetitimer.p: $(INC)/stddef.h $(INC)/memory.h $(INC)/errno.h \
	$(INCSYS)/sys/types.h $(INCSYS)/sys/evecb.h $(INCSYS)/sys/hrtcntl.h \
	$(INCSYS)/sys/time.h $(INCSYS)/sys/param.h

gettimeofday.o gettimeofday.p: $(INC)/stddef.h $(INC)/errno.h $(INCSYS)/sys/types.h \
	$(INCSYS)/sys/time.h

mlock.o mlock.p: $(INCSYS)/sys/types.h $(INCSYS)/sys/mman.h

mlockall.o mlockall.p: $(INCSYS)/sys/types.h $(INCSYS)/sys/mman.h

munlock.o munlock.p: $(INCSYS)/sys/types.h $(INCSYS)/sys/mman.h

munlockall.o munlockall.p: $(INCSYS)/sys/types.h $(INCSYS)/sys/mman.h

.c.o .c.p .c.P .c.A:
	$(NONPROF)@echo $*.c:
	$(NONPROF)$(CC) $(DEFLIST) $(SDEFLIST) $(INCLIST) $(CFLAGS) -c $*.c
	$(PROF)@echo $*.c:
	$(PROF)$(CC) $(DEFLIST) $(INCLIST) $(CFLAGS) -c -p $*.c   && mv $(*F).o $*.p
	$(PIC)@echo $*.c:
	$(PIC)$(CC) $(DEFLIST) $(SDEFLIST) $(INCLIST) $(CFLAGS) -DDSHLIB -Kpic -c $*.c && mv $(*F).o $*.P
	$(ABI)@echo $*.c:
	$(ABI)$(CC) $(DEFLIST) $(SDEFLIST) $(INCLIST) $(CFLAGS) -DABI -DDSHLIB -Kpic -c $*.c && mv $(*F).o $*.A

new_list.c errlst.c: errlist errlist.awk
	$(AWK) -f errlist.awk <errlist
