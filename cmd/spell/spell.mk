#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)spell:spell.mk	1.19"
#	spell make file

#	Note:  In using the -f flag it is assumed that either
#	both the host and the target machines need the -f, or
#	neither needs it.  If one needs it and the other does
#	not, it is assumed that the machine that does not need
#	it will treat it appropriately.

# _SH_ is used by 3B15 View-path environment

ROOT =
OL = $(ROOT)/
SL = $(ROOT)/usr/src/cmd
RDIR = ${SL}/spell
INS = install
REL = current
CSID = -r`gsid spellcode ${REL}`
DSID = -r`gsid spelldata ${REL}`
SHSID = -r`gsid spell.sh ${REL}`
CMPRSID = -r`gsid compress.sh ${REL}`
MKSID = -r`gsid spell.mk ${REL}`
BIN = P108
LIST = opr -ttx -b${BIN}
CINSDIR = ${OL}usr/bin
PINSDIR = ${OL}usr/lib/spell
SINSDIR = ${OL}usr/share/lib/spell
VINSDIR = ${OL}var/adm/
SMFLAG =
CFLAGS = -O $(FFLAGS)
SFILES = spellprog.c spellin.c
DFILES = american british local list extra stop
MAKE = make
SYMLINK = :
DIRS =  $(PINSDIR) $(SINSDIR)

compile all: spell hlista hlistb hstop spellin spellin1 spellprog hashmake \
	hashmk1 hashcheck compress spellhist
	:

spell:	spellprog spell.sh
	cp spell.sh spell

$(DIRS):
	mkdir $@
	$(CH)chmod 555 $@
	$(CH)chgrp bin $@
	$(CH)chown bin $@

compress:  compress.sh
	cp compress.sh compress

spellprog: spellprog.c hash.c hashlook.c huff.c malloc.c
	$(CC) ${SMFLAG} $(CFLAGS) -s spellprog.c hash.c hashlook.c huff.c malloc.c -o spellprog $(PERFLIBS)

spellin1: spellin.c huff.c
	cc ${SMFLAG} $(CFLAGS) -s spellin.c huff.c -o spellin1

spellin: spellin.c huff.c
	$(CC) ${SMFLAG} $(CFLAGS) -s spellin.c huff.c -o spellin $(SHLIBS)

spellhist:
	echo '\c' > spellhist

hashcheck: hashcheck.c hash.c huff.c
	$(CC) ${SMFLAG} $(CFLAGS) -s hashcheck.c hash.c huff.c -o hashcheck $(SHLIBS)

hashmk1: hashmake.c hash.c
	cc ${SMFLAG} $(CFLAGS) -s hashmake.c hash.c -o hashmk1

hashmake: hashmake.c hash.c
	$(CC) ${SMFLAG} $(CFLAGS) -s hashmake.c hash.c -o hashmake $(SHLIBS)

alldata: hlista hlistb hstop
	rm htemp1

htemp1:	list local extra hashmk1
	cat list local extra | $(_SH_) ./hashmk1 >htemp1

hlista: american hashmake hashmk1 spellin spellin1 htemp1
	$(_SH_) ./hashmk1 <american |sort -u - htemp1 >htemp2
	$(_SH_) ./spellin1 `wc htemp2|sed -n 's/\([^ ]\) .*/\1/p' ` <htemp2 >hlista
	rm htemp2

hlistb: british hashmk1 spellin1 htemp1
	$(_SH_) ./hashmk1 <british |sort -u - htemp1 >htemp2
	$(_SH_) ./spellin1 `wc htemp2|sed -n 's/\([^ ]\) .*/\1/p' ` <htemp2 >hlistb
	rm htemp2


hstop:	stop spellin1 hashmk1
	$(_SH_) ./hashmk1 <stop | sort -u >htemp2
	$(_SH_) ./spellin1 `wc htemp2|sed -n 's/\([^ ]\) .*/\1/p' ` <htemp2 >hstop
	rm htemp2

install:  all $(DIRS)
	-rm -f $(PINSDIR)/hstop
	-rm -f $(PINSDIR)/hlistb
	-rm -f $(PINSDIR)/hlista
	$(INS) -f $(SINSDIR) -m 0644 -u bin -g bin hstop
	$(INS) -f $(SINSDIR) -m 0644 -u bin -g bin hlistb
	$(INS) -f $(SINSDIR) -m 0644 -u bin -g bin hlista
	$(INS) -f $(SINSDIR) -m 0555 -u bin -g bin compress
	$(INS) -f $(PINSDIR) -m 0555 -u bin -g bin spellprog
	$(INS) -f $(PINSDIR) -m 0555 -u bin -g bin hashmake
	$(INS) -f $(PINSDIR) -m 0555 -u bin -g bin hashcheck
	$(INS) -f $(PINSDIR) -m 0555 -u bin -g bin spellin
	$(INS) -f $(VINSDIR) -m 0666 -u bin -g bin spellhist
	$(INS) -f $(CINSDIR) -m 0555 -u bin -g bin spell
	-$(SYMLINK) /usr/share/lib/spell/hstop $(PINSDIR)/hstop
	-$(SYMLINK) /usr/share/lib/spell/hlistb $(PINSDIR)/hlistb
	-$(SYMLINK) /usr/share/lib/spell/hlista $(PINSDIR)/hlista
	-$(SYMLINK) /usr/share/lib/spell/compress $(PINSDIR)/compress
inssh:    ;  ${MAKE} -f spell.mk INS="install -f" OL=${OL} spell
inscomp:  ;  ${MAKE} -f spell.mk INS="install -f" OL=${OL} compress
inscode:  ;  ${MAKE} -f spell.mk INS="install -f" spell OL=${OL}
insdata:  ;  ${MAKE} -f spell.mk INS="install -f" alldata OL=${OL}

listing:  ;  pr spell.mk spell.sh compress.sh ${SFILES} ${DFILES} | ${LIST}
listmk:   ;  pr spell.mk | ${LIST}
listsh:	  ;  pr spell.sh | ${LIST}
listcomp: ;  pr compress.sh | ${LIST}
listcode: ;  pr ${SFILES} | ${LIST}
listdata: ;  pr ${DFILES} | ${LIST}

build:  bldmk bldsh bldcomp bldcode blddata
	:
bldcode:  ;  get -p ${CSID} s.spell.src ${REWIRE} | ntar -d ${RDIR} -g
blddata:  ;  get -p ${DSID} s.spell.data | ntar -d ${RDIR} -g
bldsh:	  ;  get -p ${SHSID} s.spell.sh ${REWIRE} > ${RDIR}/spell.sh
bldcomp:  ;  get -p ${CMPRSID} s.compress.sh ${REWIRE} > ${RDIR}/compress.sh
bldmk:    ;  get -p ${MKSID} s.spell.mk > ${RDIR}/spell.mk

edit:	sedit dedit mkedit shedit compedit
	:
sedit:	;  get -p -e s.spell.src | ntar -g
dedit:	;  get -p -e s.spell.data | ntar -g
shedit:	;  get -e s.spell.sh
compedit: ; get -e s.compress.sh

delta:	sdelta ddelta mkdelta shdelta compdelta
	:
sdelta:
	ntar -p ${SFILES} > spell.src
	delta s.spell.src
	rm -f ${SFILES}
ddelta:
	ntar -p ${DFILES} > spell.data
	delta s.spell.data
	rm -f ${DFILES}
shdelta:
	delta s.spell.sh
compdelta: ; delta s.compress.sh

mkedit:	;  get -e s.spell.mk
mkdelta: ; delta s.spell.mk

clean:
	rm -f *.o

clobber: clean shclobber compclobber
	rm -f spell spellprog spellin spellhist hashmake hlist* hstop spellin1 hashmk1
	-rm -f htemp1 htemp2 hashcheck
shclobber: ; rm -f spell
compclobber: ; rm -f compress

delete:	clobber shdelete compdelete
	rm -f ${SFILES} ${DFILES}
shdelete: shclobber
	rm -f spell.sh
compdelete: compclobber
	rm -f compress.sh
