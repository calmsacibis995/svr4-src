#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)bkrs:meth.d/makefile	1.20.2.1"

SRC1=incfile.c migration.c fimage.c fdisk.c
OBJ=$(SRC1:.c=.o)

COMMON=bkopt.c cmdargs.c mi_init.c fsarg_check.c find.c open_tc.c \
bld_except.c file_req.c do_file.c mback.c iback.c irest.c nxt_media.c \
last_date.c bk_system.c sum.c bkcpio.c methutil.c \
bkfdisk.c bknewvol.c rsnewvol.c rsfdisk.c doresult.c rsutil.c

COMMONOBJ=$(COMMON:.c=.o)

INCLIST=bkopt.o cmdargs.o mi_init.o fsarg_check.o find.o open_tc.o \
bld_except.o file_req.o do_file.o nxt_media.o last_date.o bk_system.o \
sum.o bkcpio.o methutil.o bknewvol.o rsnewvol.o doresult.o rsutil.o

IMLIST=bkopt.o cmdargs.o mi_init.o fsarg_check.o file_req.o \
find.o iback.o irest.o nxt_media.o bk_system.o sum.o \
methutil.o bknewvol.o rsnewvol.o doresult.o rsutil.o

MIGLIST=bkopt.o cmdargs.o mi_init.o fsarg_check.o \
mback.o nxt_media.o bk_system.o sum.o \
methutil.o bknewvol.o

FDLIST=bkopt.o cmdargs.o mi_init.o bkfdisk.o bknewvol.o methutil.o nxt_media.o\
sum.o rsfdisk.o rsnewvol.o bk_system.o

INS=install
BKLIB=../bklib.d/bklib.a
LIBBR=../libbrmeth.d/libbrmeth.a
IOLIB=../libadmIO.d/libadmIO.a
INC=$(ROOT)/usr/include
LOCAL=hdrs
GENERAL=../hdrs
BKBIN=$(ROOT)/usr/sadm/bkup/bin
OAMBASE=$(ROOT)/usr/sadm/sysadm
SBIN=$(ROOT)/sbin
USBIN=$(ROOT)/usr/sbin
ETCBKUP=$(ROOT)/etc/bkup
METHODDIR=$(ETCBKUP)/method
CC=$(PFX)cc
FLAGS=
CFLAGS=-I $(LOCAL) -I $(GENERAL) -I $(INC) $(FLAGS)
LDFLAGS=$(LDLIBPATH) -s
MAKE=make
STRIP=$(PFX)strip
LINT=$(PFX)lint

PRODUCTS=incfile migration fimage ffile fdp fdisk

all: $(PRODUCTS)

incfile: $(BKLIB) $(IOLIB) $(LIBBR) $(INCLIST) incfile.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(@) incfile.o $(INCLIST) $(BKLIB) $(LIBBR) $(IOLIB) -lTL -lgen -ladm

ffile: incfile
	rm -f ffile
	ln incfile ffile

migration: $(BKLIB) $(IOLIB) $(LIBBR) $(MIGLIST) migration.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(@) migration.o $(MIGLIST) $(BKLIB) $(LIBBR) $(IOLIB) -lTL -lgen -ladm

fimage: $(BKLIB) $(IOLIB) $(LIBBR) $(IMLIST) fimage.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(@) fimage.o $(IMLIST) $(BKLIB) $(LIBBR) $(IOLIB) -lTL -lgen -ladm

fdp: fimage
	rm -f fdp
	ln fimage fdp

fdisk: $(BKLIB) $(IOLIB) $(LIBBR) $(FDLIST) fdisk.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(@) fdisk.o $(FDLIST) $(BKLIB) $(LIBBR) $(IOLIB) -lTL -lgen -ladm

touch: 
	touch $(SRC1) $(COMMON)

clean:
	rm -f $(OBJ) $(COMMONOBJ)

clobber: clean
	rm -f $(PRODUCTS)

lintit:
	$(LINT) $(CFLAGS) $(LFLAGS) $(SRC1) $(COMMON)

strip:	$(PRODUCTS)
	$(STRIP) $(PRODUCTS)

install: $(PRODUCTS)
	$(INS) -f $(METHODDIR) -u root -g sys -m 0700 ffile
	$(INS) -f $(METHODDIR) -u root -g sys -m 0700 incfile
	$(INS) -f $(METHODDIR) -u root -g sys -m 0700 fdp
	$(INS) -f $(METHODDIR) -u root -g sys -m 0700 migration
	$(INS) -f $(METHODDIR) -u root -g sys -m 0700 fimage
	$(INS) -f $(METHODDIR) -u root -g sys -m 0700 fdisk

bk_system.o:	$(INC)/sys/types.h
bk_system.o:	$(INC)/signal.h
bk_system.o:	$(GENERAL)/backup.h
bkcpio.o:	$(INC)/limits.h
bkcpio.o:	$(INC)/stdio.h
bkcpio.o:	$(INC)/sys/types.h
bkcpio.o:	$(INC)/sys/stat.h
bkcpio.o:	$(INC)/ctype.h
bkcpio.o:	$(INC)/errno.h
bkcpio.o:	$(INC)/fcntl.h
bkcpio.o:	$(INC)/setjmp.h
bkcpio.o:	$(LOCAL)/method.h
bkcpio.o:	$(GENERAL)/backup.h
bkcpio.o:	$(GENERAL)/bktypes.h
bkcpio.o:	$(INC)/bkrs.h
bkcpio.o:	$(INC)/signal.h
bkcpio.o:	$(INC)/string.h
bkcpio.o:	$(INC)/table.h
bkcpio.o:	$(GENERAL)/brtoc.h
bkcpio.o:	$(GENERAL)/brarc.h
bkfdisk.o:	$(INC)/limits.h
bkfdisk.o:	$(INC)/sys/types.h
bkfdisk.o:	$(INC)/stdio.h
bkfdisk.o:	$(INC)/fcntl.h
bkfdisk.o:	$(INC)/string.h
bkfdisk.o:	$(INC)/signal.h
bkfdisk.o:	$(GENERAL)/backup.h
bkfdisk.o:	$(GENERAL)/bktypes.h
bkfdisk.o:	$(INC)/bkrs.h
bkfdisk.o:	$(GENERAL)/brarc.h
bkfdisk.o:	$(LOCAL)/method.h
bkfdisk.o:	$(INC)/setjmp.h
bknewvol.o:	$(INC)/limits.h
bknewvol.o:	$(INC)/sys/types.h
bknewvol.o:	$(INC)/stdio.h
bknewvol.o:	$(INC)/string.h
bknewvol.o:	$(INC)/signal.h
bknewvol.o:	$(GENERAL)/backup.h
bknewvol.o:	$(INC)/bkrs.h
bknewvol.o:	$(LOCAL)/method.h
bknewvol.o:	$(INC)/setjmp.h
bkopt.o:	$(INC)/limits.h
bkopt.o:	$(INC)/sys/types.h
bkopt.o:	$(INC)/string.h
bkopt.o:	$(INC)/bkrs.h
bkopt.o:	$(LOCAL)/method.h
bld_except.o:	$(INC)/limits.h
bld_except.o:	$(INC)/sys/types.h
bld_except.o:	$(INC)/stdio.h
bld_except.o:	$(LOCAL)/method.h
bld_except.o:	$(GENERAL)/backup.h
bld_except.o:	$(INC)/signal.h
bld_except.o:	$(INC)/fcntl.h
bld_except.o:	$(INC)/table.h
bld_except.o:	$(GENERAL)/bkexcept.h
bld_except.o:	$(INC)/string.h
cmdargs.o:	$(INC)/limits.h
cmdargs.o:	$(INC)/sys/types.h
cmdargs.o:	$(INC)/stddef.h
cmdargs.o:	$(INC)/string.h
cmdargs.o:	$(INC)/bkrs.h
cmdargs.o:	$(LOCAL)/method.h
do_file.o:	$(INC)/limits.h
do_file.o:	$(INC)/stdio.h
do_file.o:	$(INC)/errno.h
do_file.o:	$(INC)/sys/types.h
do_file.o:	$(INC)/sys/stat.h
do_file.o:	$(INC)/signal.h
do_file.o:	$(INC)/memory.h
do_file.o:	$(INC)/setjmp.h
do_file.o:	$(GENERAL)/backup.h
do_file.o:	$(GENERAL)/bktypes.h
do_file.o:	$(INC)/bkrs.h
do_file.o:	$(INC)/fcntl.h
do_file.o:	$(LOCAL)/method.h
do_file.o:	$(INC)/string.h
do_file.o:	$(GENERAL)/brarc.h
doresult.o:	$(INC)/limits.h
doresult.o:	$(INC)/stdio.h
doresult.o:	$(INC)/sys/types.h
doresult.o:	$(GENERAL)/backup.h
doresult.o:	$(INC)/bkrs.h
doresult.o:	$(LOCAL)/method.h
fdisk.o:	$(INC)/limits.h
fdisk.o:	$(INC)/sys/types.h
fdisk.o:	$(INC)/sys/stat.h
fdisk.o:	$(INC)/stdio.h
fdisk.o:	$(INC)/string.h
fdisk.o:	$(GENERAL)/backup.h
fdisk.o:	$(INC)/bkrs.h
fdisk.o:	$(LOCAL)/method.h
file_req.o:	$(INC)/limits.h
file_req.o:	$(INC)/stdio.h
file_req.o:	$(INC)/sys/types.h
file_req.o:	$(INC)/sys/stat.h
file_req.o:	$(INC)/bkrs.h
file_req.o:	$(LOCAL)/method.h
file_req.o:	$(INC)/varargs.h
migration.o:	$(INC)/limits.h
migration.o:	$(INC)/sys/types.h
migration.o:	$(INC)/stdio.h
migration.o:	$(INC)/string.h
migration.o:	$(GENERAL)/backup.h
migration.o:	$(INC)/bkrs.h
migration.o:	$(LOCAL)/method.h
fimage.o:	$(INC)/limits.h
fimage.o:	$(INC)/sys/types.h
fimage.o:	$(INC)/stdio.h
fimage.o:	$(INC)/string.h
fimage.o:	$(GENERAL)/backup.h
fimage.o:	$(INC)/bkrs.h
fimage.o:	$(LOCAL)/method.h
find.o:		$(INC)/limits.h
find.o:		$(INC)/stdio.h
find.o:		$(INC)/sys/types.h
find.o:		$(INC)/sys/stat.h
find.o:		$(INC)/ctype.h
find.o:		$(INC)/fcntl.h
find.o:		$(INC)/setjmp.h
find.o:		$(LOCAL)/method.h
find.o:		$(GENERAL)/backup.h
find.o:		$(INC)/bkrs.h
find.o:		$(INC)/signal.h
find.o:		$(INC)/string.h
find.o:		$(INC)/dirent.h
find.o:		$(INC)/table.h
find.o:		$(GENERAL)/brtoc.h
find.o:	$(INC)/errno.h
fsarg_check.o:	$(INC)/limits.h
fsarg_check.o:	$(INC)/sys/types.h
fsarg_check.o:	$(INC)/sys/param.h
fsarg_check.o:	$(INC)/sys/statvfs.h
fsarg_check.o:	$(INC)/errno.h
fsarg_check.o:	$(INC)/fcntl.h
fsarg_check.o:	$(INC)/mnttab.h
fsarg_check.o:	$(INC)/string.h
fsarg_check.o:	$(LOCAL)/method.h
fsarg_check.o:	$(GENERAL)/backup.h
fsarg_check.o:	$(INC)/signal.h
mback.o:	$(INC)/limits.h
mback.o:	$(INC)/sys/types.h
mback.o:	$(INC)/stdio.h
mback.o:	$(INC)/errno.h
mback.o:	$(INC)/fcntl.h
mback.o:	$(INC)/signal.h
mback.o:	$(INC)/string.h
mback.o:	$(INC)/setjmp.h
mback.o:	$(INC)/sys/param.h
mback.o:	$(LOCAL)/method.h
mback.o:	$(GENERAL)/backup.h
mback.o:	$(GENERAL)/bktypes.h
mback.o:	$(INC)/bkrs.h
mback.o:	$(GENERAL)/brarc.h
iback.o:	$(INC)/limits.h
iback.o:	$(INC)/sys/types.h
iback.o:	$(INC)/sys/statvfs.h
iback.o:	$(INC)/stdio.h
iback.o:	$(INC)/errno.h
iback.o:	$(INC)/fcntl.h
iback.o:	$(INC)/signal.h
iback.o:	$(INC)/string.h
iback.o:	$(INC)/setjmp.h
iback.o:	$(INC)/sys/param.h
iback.o:	$(LOCAL)/method.h
iback.o:	$(GENERAL)/backup.h
iback.o:	$(GENERAL)/bktypes.h
iback.o:	$(INC)/bkrs.h
iback.o:	$(GENERAL)/brarc.h
incfile.o:	$(INC)/limits.h
incfile.o:	$(INC)/sys/types.h
incfile.o:	$(INC)/string.h
incfile.o:	$(GENERAL)/backup.h
incfile.o:	$(INC)/bkrs.h
incfile.o:	$(LOCAL)/method.h
io.o: $(INC)/sys/types.h
io.o: $(INC)/errno.h
io.o: $(INC)/sys/stat.h
irest.o:	$(INC)/limits.h
irest.o:	$(INC)/sys/types.h
irest.o:	$(INC)/sys/mount.h
irest.o:	$(INC)/sys/fstyp.h
irest.o:	$(INC)/sys/fsid.h
irest.o:	$(INC)/sys/stat.h
irest.o:	$(INC)/stdio.h
irest.o:	$(INC)/fcntl.h
irest.o:	$(INC)/errno.h
irest.o:	$(INC)/signal.h
irest.o:	$(LOCAL)/method.h
irest.o:	$(GENERAL)/backup.h
irest.o:	$(GENERAL)/bktypes.h
irest.o:	$(INC)/string.h
irest.o:	$(GENERAL)/brarc.h
last_date.o:	$(INC)/limits.h
last_date.o:	$(INC)/stdio.h
last_date.o:	$(INC)/sys/types.h
last_date.o:	$(INC)/sys/fcntl.h
last_date.o:	$(LOCAL)/method.h
last_date.o:	$(INC)/table.h
last_date.o:	$(GENERAL)/bkhist.h
last_date.o:	$(INC)/string.h
methutil.o:	$(INC)/limits.h
methutil.o:	$(INC)/stdio.h
methutil.o:	$(INC)/sys/types.h
methutil.o:	$(INC)/setjmp.h
methutil.o:	$(LOCAL)/method.h
methutil.o:	$(GENERAL)/backup.h
methutil.o:	$(INC)/bkrs.h
methutil.o:	$(GENERAL)/brtoc.h
methutil.o:	$(INC)/signal.h
methutil.o:	$(INC)/string.h
methutil.o:	$(INC)/table.h
mi_init.o:	$(INC)/limits.h
mi_init.o:	$(INC)/sys/types.h
mi_init.o:	$(INC)/sys/param.h
mi_init.o:	$(LOCAL)/method.h
nxt_media.o:	$(INC)/limits.h
nxt_media.o:	$(INC)/stdio.h
nxt_media.o:	$(INC)/sys/types.h
nxt_media.o:	$(INC)/setjmp.h
nxt_media.o:	$(LOCAL)/method.h
open_tc.o:	$(INC)/limits.h
open_tc.o:	$(INC)/sys/types.h
open_tc.o:	$(INC)/sys/stat.h
open_tc.o:	$(INC)/stdio.h
open_tc.o:	$(GENERAL)/backup.h
open_tc.o:	$(INC)/fcntl.h
open_tc.o:	$(LOCAL)/method.h
open_tc.o:	$(INC)/signal.h
open_tc.o:	$(INC)/string.h
open_tc.o:	$(INC)/errno.h
rsfdisk.o:	$(INC)/limits.h
rsfdisk.o:	$(INC)/sys/types.h
rsfdisk.o:	$(INC)/stdio.h
rsfdisk.o:	$(INC)/fcntl.h
rsfdisk.o:	$(INC)/string.h
rsfdisk.o:	$(INC)/signal.h
rsfdisk.o:	$(GENERAL)/backup.h
rsfdisk.o:	$(GENERAL)/bktypes.h
rsfdisk.o:	$(INC)/bkrs.h
rsfdisk.o:	$(GENERAL)/brarc.h
rsfdisk.o:	$(LOCAL)/method.h
rsnewvol.o:	$(INC)/limits.h
rsnewvol.o:	$(INC)/sys/types.h
rsnewvol.o:	$(INC)/stdio.h
rsnewvol.o:	$(INC)/string.h
rsnewvol.o:	$(INC)/signal.h
rsnewvol.o:	$(INC)/fcntl.h
rsnewvol.o:	$(GENERAL)/backup.h
rsnewvol.o:	$(GENERAL)/bktypes.h
rsnewvol.o:	$(INC)/bkrs.h
rsnewvol.o:	$(LOCAL)/method.h
rsnewvol.o:	$(GENERAL)/brarc.h
rsutil.o:	$(INC)/limits.h
rsutil.o:	$(INC)/errno.h
rsutil.o:	$(INC)/sys/types.h
rsutil.o:	$(INC)/sys/stat.h
rsutil.o:	$(LOCAL)/method.h
sum.o:		$(INC)/limits.h
sum.o:		$(INC)/sys/types.h
sum.o:		$(INC)/fcntl.h
sum.o:		$(LOCAL)/method.h
sum.o:		$(GENERAL)/bktypes.h
