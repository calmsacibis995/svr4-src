#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)bkrs:libbrmeth.d/makefile	1.1.3.1"

SRC1=bkipc.c bkstate.c brestimate.c brgetvolume.c brinit.c rsreturn.c brinvlbls.c
SRC2=brlog.c brpath.c brreturn.c brspace.c brcontrol.c brhistory.c brio.c bkgetvol.c
SRC3=brsnddot.c brsndfname.c brtoc.c br_read_hdr.c rsgethdr.c bkhdr.c rstoc.c argv.c
OBJ=$(SRC1:.c=.o) $(SRC2:.c=.o) $(SRC3:.c=.o)

INC=$(ROOT)/usr/include
LIB=$(ROOT)/lib
ULIB=$(ROOT)/usr/lib

IOLIB=../libadmIO.d/libadmIO.a

AR=ar
BKBIN=$(ROOT)/usr/sadm/bkup/bin
OAMBASE=$(ROOT)/usr/sadm/sysadm
SBIN=$(ROOT)/sbin
USBIN=$(ROOT)/usr/sbin
ETCBKUP=$(ROOT)/etc/bkup
CC=$(PFX)cc
LFLAGS=-u
LORDER=$(PFX)lorder
LINT=$(PFX)lint

GENERAL=../hdrs
FLAGS=
CFLAGS=-I $(GENERAL) -I $(INC) $(FLAGS)

PRODUCT=libbrmeth.a

all: $(PRODUCT)

libbrmeth.a: $(IOLIB) $(OBJ)
	$(AR) cr $(@) `$(LORDER) $(OBJ) | tsort`

touch: 
	touch $(SRC1) $(SRC2) $(SRC3)

clean:
	rm -f $(OBJ)

clobber: clean
	rm -f $(PRODUCT)

lintit:
	$(LINT) -u $(LFLAGS) $(CFLAGS) $(SRC1) $(SRC2) $(SRC3)

strip: $(PRODUCT)

install: $(PRODUCT)
	install -f $(ULIB) -m 0644 $(PRODUCT)

bkipc.o: $(INC)/sys/types.h
bkipc.o: $(INC)/sys/stat.h
bkipc.o: $(INC)/dirent.h
bkipc.o: $(INC)/fcntl.h
bkipc.o: $(INC)/stdio.h
bkipc.o: $(INC)/signal.h
bkipc.o: $(INC)/errno.h
bkipc.o: $(INC)/limits.h
bkipc.o: $(GENERAL)/backup.h
bkipc.o: $(GENERAL)/bkmsgs.h
bkstate.o: $(INC)/sys/types.h
bkstate.o: $(INC)/signal.h
bkstate.o: $(INC)/time.h
bkstate.o: $(GENERAL)/backup.h
bkstate.o: $(GENERAL)/bkmsgs.h
bkstate.o: $(INC)/bkrs.h
bkstate.o: $(GENERAL)/bkoper.h
bkstate.o: $(INC)/errno.h
brcontrol.o: $(INC)/signal.h
brcontrol.o: $(INC)/bkrs.h
brestimate.o: $(INC)/sys/types.h
brestimate.o: $(INC)/errno.h
brestimate.o: $(INC)/time.h
brestimate.o: $(INC)/bkrs.h
brestimate.o: $(GENERAL)/backup.h
brestimate.o: $(GENERAL)/bkmsgs.h
brgetvolume.o: $(INC)/bkrs.h
brinit.o: $(INC)/signal.h
brinit.o: $(INC)/sys/types.h
brinit.o: $(INC)/errno.h
brinit.o: $(GENERAL)/backup.h
brinit.o: $(INC)/bkrs.h
brio.o: $(INC)/sys/types.h
brio.o: $(INC)/fcntl.h
brio.o: $(INC)/sys/errno.h
brio.o: $(GENERAL)/backup.h
brio.o: $(GENERAL)/bktypes.h
brio.o: $(INC)/sys/stat.h
brlog.o: $(INC)/sys/types.h
brlog.o: $(INC)/time.h
brlog.o: $(INC)/stdio.h
brlog.o: $(INC)/varargs.h
brlog.o: $(GENERAL)/backup.h
brreturn.o: $(INC)/sys/types.h
brreturn.o: $(INC)/time.h
brreturn.o: $(GENERAL)/backup.h
brreturn.o: $(GENERAL)/bkmsgs.h
brreturn.o: $(INC)/bkrs.h
rsgetvol.o: $(INC)/stdio.h
rsgetvol.o: $(INC)/string.h
rsreturn.o: $(INC)/sys/types.h
rsreturn.o: $(INC)/fcntl.h
rsreturn.o: $(INC)/stdio.h
rsreturn.o: $(INC)/errno.h
rsreturn.o: $(INC)/table.h
rsreturn.o: $(GENERAL)/rsstatus.h
brhistory.o: $(INC)/sys/types.h
brhistory.o: $(INC)/time.h
brhistory.o: $(INC)/bkrs.h
brhistory.o: $(GENERAL)/backup.h
brhistory.o: $(GENERAL)/bkmsgs.h
brhistory.o: $(INC)/errno.h
brpath.o: $(INC)/sys/types.h
brpath.o: $(INC)/stdio.h
brpath.o: $(INC)/time.h
brpath.o: $(GENERAL)/backup.h
brpath.o: $(GENERAL)/restore.h
brpath.o: $(GENERAL)/rsstatus.h
brpath.o: $(INC)/bkrs.h
brpath.o: $(GENERAL)/bkexcept.h
brpath.o: $(INC)/errno.h
brsnddot.o: $(INC)/sys/types.h
brsnddot.o: $(INC)/time.h
brsnddot.o: $(INC)/stdio.h
brsnddot.o: $(INC)/bkrs.h
brsnddot.o: $(GENERAL)/backup.h
brsnddot.o: $(GENERAL)/bkmsgs.h
brsnddot.o: $(INC)/errno.h
brsndfname.o: $(INC)/sys/types.h
brsndfname.o: $(INC)/time.h
brsndfname.o: $(INC)/stdio.h
brsndfname.o: $(INC)/bkrs.h
brsndfname.o: $(GENERAL)/backup.h
brsndfname.o: $(GENERAL)/bkmsgs.h
brsndfname.o: $(INC)/errno.h
rstoc.o: $(INC)/sys/types.h
rstoc.o: $(INC)/time.h
rstoc.o: $(GENERAL)/backup.h
rstoc.o: $(GENERAL)/bkmsgs.h
rstoc.o: $(INC)/bkrs.h
brinvlbls.o: $(INC)/sys/types.h
brinvlbls.o: $(INC)/errno.h
brinvlbls.o: $(INC)/time.h
brinvlbls.o: $(INC)/bkrs.h
brinvlbls.o: $(GENERAL)/backup.h
brinvlbls.o: $(GENERAL)/bkmsgs.h
bkhdr.o: $(INC)/limits.h
bkhdr.o: $(INC)/sys/types.h
bkhdr.o: $(INC)/sys/utsname.h
bkhdr.o: $(INC)/sys/fcntl.h
bkhdr.o: $(INC)/signal.h
bkhdr.o: $(GENERAL)/backup.h
bkhdr.o: $(GENERAL)/brarc.h
bkhdr.o: $(INC)/string.h
bkhdr.o: $(INC)/errno.h
bkhdr.o: $(GENERAL)/bktypes.h
br_read_hdr.o: $(INC)/sys/types.h
br_read_hdr.o: $(GENERAL)/backup.h
br_read_hdr.o: $(INC)/fcntl.h
br_read_hdr.o: $(GENERAL)/brarc.h
br_read_hdr.o: $(GENERAL)/bktypes.h
br_read_hdr.o: $(INC)/sys/errno.h
rsgethdr.o: $(INC)/limits.h
rsgethdr.o: $(INC)/signal.h
rsgethdr.o: $(INC)/fcntl.h
rsgethdr.o: $(INC)/sys/types.h
rsgethdr.o: $(GENERAL)/backup.h
rsgethdr.o: $(GENERAL)/bktypes.h
rsgethdr.o: $(INC)/bkrs.h
rsgethdr.o: $(GENERAL)/brarc.h
rsgethdr.o: $(INC)/devmgmt.h
bkgetvol.o: $(INC)/stdio.h
bkgetvol.o: $(INC)/limits.h
bkgetvol.o: $(INC)/fcntl.h
bkgetvol.o: $(INC)/string.h
bkgetvol.o: $(INC)/devmgmt.h
bkgetvol.o: $(INC)/varargs.h
bkgetvol.o: $(GENERAL)/bktypes.h
bkgetvol.o: $(INC)/bkrs.h
bkgetvol.o: $(GENERAL)/backup.h
bkgetvol.o: $(GENERAL)/brarc.h
bkgetvol.o: $(GENERAL)/libadmIO.h
argv.o: $(INC)/sys/types.h
argv.o: $(INC)/string.h
argv.o:	$(GENERAL)/bktypes.h
brspace.o: $(INC)/sys/types.h
