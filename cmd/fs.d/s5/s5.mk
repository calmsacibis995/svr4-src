#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)s5.cmds:s5.mk	1.2.4.1"

#  /usr/src/cmd/lib/fs/s5 is the directory of all s5 specific commands
#  whose executable reside in $(INSDIR1) and $(INSDIR2).

TESTDIR = .
INSDIR1 = $(ROOT)/usr/lib/fs/s5
INSDIR2 = $(ROOT)/etc/fs/s5
INS = install
CFLAGS = -O -I$(INC) -I$(INCSYS)
LDFLAGS = -s

LIBELF =
#
#	Libelf is required for mkfs if ELF_BOOT is defined.  This would allow
#	mkfs to open, parse, and load ELF boot strap code to the disk.  That
#	is, if the name of an ELF boot file was given as the first line of
#	a proto file.
#
# LIBELF = -lelf

INC = $(ROOT)/usr/include
INCSYS = $(ROOT)/usr/include
THISFILE= s5.mk

all:	mount clri df  ff fsck  fsdb labelit mkfs ncheck volcopy dcopy

clean:
	rm -f *.o

clobber: clean		clobber_mount \
	clobber_clri	clobber_dcopy\
	clobber_df	clobber_fsck\
	clobber_fsdb    clobber_ff \
	clobber_labelit clobber_mkfs \
	clobber_ncheck clobber_volcopy

install: install_mount install_ncheck \
	install_clri 	install_dcopy\
	install_df	install_fsck\
	install_fsdb    install_ff \
	install_labelit install_mkfs \
	install_volcopy

#
#  This is to build s5 specific mount command
#

mount:	mount.c\
	$(INC)/stdio.h\
	$(INCSYS)/sys/signal.h\
	$(INC)/unistd.h\
	$(INCSYS)/sys/errno.h\
	$(INCSYS)/sys/mnttab.h\
	$(INCSYS)/sys/mount.h\
	$(INCSYS)/sys/types.h\
	$(INCSYS)/sys/statvfs.h
	$(CC) $(CFLAGS) -o $(TESTDIR)/mount mount.c $(LDFLAGS) $(ROOTLIBS)

install_mount: mount
	@if [ ! -d $(ROOT)/usr/lib/fs ]; \
		then \
		mkdir $(ROOT)/usr/lib/fs; \
		fi;
	@if [ ! -d $(INSDIR1) ]; \
		then \
		mkdir $(INSDIR1); \
		fi;
	@if [ ! -d $(ROOT)/etc/fs ]; \
		then \
		mkdir $(ROOT)/etc/fs; \
		fi;
	@if [ ! -d $(INSDIR2) ]; \
		then \
		mkdir $(INSDIR2); \
		fi;
	$(INS) -f $(INSDIR1) -m 0555 -u bin -g bin $(TESTDIR)/mount
	$(INS) -f $(INSDIR2) -m 0555 -u bin -g bin $(TESTDIR)/mount

clobber_mount:	clean
	rm -f $(TESTDIR)/mount

#
#  This is to build s5 specific clri command
#

clri:	$(INCSYS)/sys/types.h \
	$(INCSYS)/sys/param.h \
	$(INCSYS)/sys/stat.h \
	$(INCSYS)/sys/fs/s5param.h \
	$(INCSYS)/sys/fs/s5ino.h \
	$(INCSYS)/sys/fs/s5dir.h \
	$(INCSYS)/sys/fs/s5filsys.h
	$(CC) $(CFLAGS) -o $(TESTDIR)/clri clri.c $(LDFLAGS) $(ROOTLIBS)

install_clri: clri
	@if [ ! -d $(ROOT)/usr/lib/fs ]; \
		then \
		mkdir $(ROOT)/usr/lib/fs; \
		fi;
	@if [ ! -d $(INSDIR1) ]; \
		then \
		mkdir $(INSDIR1); \
		fi;
	@if [ ! -d $(ROOT)/etc/fs ]; \
		then \
		mkdir $(ROOT)/etc/fs; \
		fi;
	@if [ ! -d $(INSDIR2) ]; \
		then \
		mkdir $(INSDIR2); \
		fi;
	$(INS) -f $(INSDIR1) -m 0555 -u bin -g bin $(TESTDIR)/clri
	$(INS) -f $(INSDIR2) -m 0555 -u bin -g bin $(TESTDIR)/clri

clobber_clri:
	rm -f $(TESTDIR)/clri


#
#  This is to build the s5 specific df command
#

df: 	$(INCSYS)/sys/param.h $(INCSYS)/sys/types.h \
	$(INCSYS)/sys/mnttab.h $(INCSYS)/sys/stat.h \
	$(INCSYS)/sys/statfs.h $(INCSYS)/sys/errno.h \
	$(INCSYS)/sys/vnode.h $(INCSYS)/sys/fs/s5param.h \
	$(INCSYS)/sys/fs/s5filsys.h $(INCSYS)/sys/fs/s5fblk.h \
	$(INCSYS)/sys/fs/s5dir.h $(INCSYS)/sys/fs/s5ino.h \
	$(INC)/ustat.h $(INC)/setjmp.h \
	$(INC)/string.h
	$(CC) $(CFLAGS) -o $(TESTDIR)/df df.c $(LDFLAGS) $(ROOTLIBS)

install_df: df
	@if [ ! -d $(ROOT)/usr/lib/fs ]; \
		then \
		mkdir $(ROOT)/usr/lib/fs; \
		fi;
	@if [ ! -d $(INSDIR1) ]; \
		then \
		mkdir $(INSDIR1); \
		fi;
	@if [ ! -d $(ROOT)/etc/fs ]; \
		then \
		mkdir $(ROOT)/etc/fs; \
		fi;
	@if [ ! -d $(INSDIR2) ]; \
		then \
		mkdir $(INSDIR2); \
		fi;
	$(INS) -f $(INSDIR1) -m 0555 -u bin -g bin $(TESTDIR)/df
	$(INS) -f $(INSDIR2) -m 0555 -u bin -g bin $(TESTDIR)/df

clobber_df:
	rm -f $(TESTDIR)/df


#
# This is s5 specific fsck
#

fsck: fsck.o fsck2.o fsck3.o fsck4.o
	$(CC) $(LDFLAGS) -o fsck fsck.o fsck2.o fsck3.o fsck4.o $(ROOTLIBS)

fsck2.o: $(INCSYS)/sys/types.h\
	$(INCSYS)/sys/fs/s5param.h\
	fsckinit.c\
	$(FRC)
	$(CC) $(CFLAGS) -DFsTYPE=1 -c fsckinit.c
	mv fsckinit.o fsck2.o

fsck3.o: $(INCSYS)/sys/types.h\
	$(INCSYS)/sys/fs/s5param.h\
	fsckinit.c\
	$(FRC)
	$(CC) $(CFLAGS) -DFsTYPE=2 -c fsckinit.c
	mv fsckinit.o fsck3.o

fsck4.o: $(INCSYS)/sys/types.h\
	$(INCSYS)/sys/fs/s5param.h\
	fsckinit.c\
	$(FRC)
	$(CC) $(CFLAGS) -DFsTYPE=4 -c fsckinit.c
	mv fsckinit.o fsck4.o

fsck.o: $(INC)/stdio.h\
	$(INC)/ctype.h\
	$(INC)/string.h \
	$(INCSYS)/sys/fcntl.h\
	$(INC)/stand.h\
	$(INCSYS)/sys/param.h\
	$(INCSYS)/sys/types.h\
	$(INC)/signal.h\
	$(INCSYS)/sys/uadmin.h\
	$(INCSYS)/sys/vnode.h\
	$(INCSYS)/sys/fs/s5param.h\
	$(INCSYS)/sys/fs/s5ino.h\
	$(INCSYS)/sys/fs/s5inode.h\
	$(INCSYS)/sys/fs/s5filsys.h\
	$(INCSYS)/sys/fs/s5dir.h\
	$(INCSYS)/sys/fs/s5fblk.h\
	$(INCSYS)/sys/stat.h\
	$(INCSYS)/sys/ustat.h\
	$(INCSYS)/sys/sysi86.h\
	fsck.c\
	$(FRC)
	$(CC) $(CFLAGS) -DFsTYPE=4 -Di386 -c fsck.c 
#  VERY IMPORTANT: fsck.c MUST be compiled with FsTYPE set to the
#  largest blocksize to allocate the biggest buffer possible.


install_fsck: fsck
	@if [ ! -d $(ROOT)/usr/lib/fs ]; \
		then \
		mkdir $(ROOT)/usr/lib/fs; \
		fi;
	@if [ ! -d $(INSDIR1) ]; \
		then \
		mkdir $(INSDIR1); \
		fi;
	@if [ ! -d $(ROOT)/etc/fs ]; \
		then \
		mkdir $(ROOT)/etc/fs; \
		fi;
	@if [ ! -d $(INSDIR2) ]; \
		then \
		mkdir $(INSDIR2); \
		fi;
	$(INS) -f $(INSDIR1) -m 0555 -u bin -g bin $(TESTDIR)/fsck
	$(INS) -f $(INSDIR2) -m 0555 -u bin -g bin $(TESTDIR)/fsck

FRC :

clobber_fsck:
	rm -f $(TESTDIR)/fsck

lint:
	$(LINT) -I$(INC) -I$(INCSYS) -DFsTYPE=1 fsck2.c 
	$(LINT) -I$(INC) -I$(INCSYS) -DFsTYPE=2 fsck3.c 
	$(LINT) -I$(INC) -I$(INCSYS) -DFsTYPE=4 -Di386 fsck.c 


#
#  This is to build s5 specific fsdb command
#


fsdb: fsdb.o
	$(CC) fsdb.o -o fsdb $(LDFLAGS) $(ROOTLIBS)

fsdb.o: $(INCSYS)/sys/param.h\
	$(INC)/signal.h\
	$(INC)/time.h\
	$(INCSYS)/sys/types.h\
	$(INCSYS)/sys/sysmacros.h\
	$(INCSYS)/sys/vnode.h\
	$(INCSYS)/sys/fs/s5param.h\
	$(INCSYS)/sys/fs/s5ino.h\
	$(INCSYS)/sys/fs/s5inode.h\
	$(INCSYS)/sys/fs/s5dir.h\
	$(INC)/stdio.h\
	$(INC)/setjmp.h\
	$(INCSYS)/sys/fs/s5filsys.h

install_fsdb: fsdb
	@if [ ! -d $(ROOT)/usr/lib/fs ]; \
		then \
		mkdir $(ROOT)/usr/lib/fs; \
		fi;
	@if [ ! -d $(INSDIR1) ]; \
		then \
		mkdir $(INSDIR1); \
		fi;
	@if [ ! -d $(ROOT)/etc/fs ]; \
		then \
		mkdir $(ROOT)/etc/fs; \
		fi;
	@if [ ! -d $(INSDIR2) ]; \
		then \
		mkdir $(INSDIR2); \
		fi;
	$(INS) -f $(INSDIR1) -m 0555 -u bin -g bin $(TESTDIR)/fsdb
	$(INS) -f $(INSDIR2) -m 0555 -u bin -g bin $(TESTDIR)/fsdb

clobber_fsdb:
	rm -f $(TESTDIR)/fsdb

#
#  This is to build s5 specific labelit command
#

labelit: $(INC)/stdio.h $(INCSYS)/sys/param.h \
	$(INC)/signal.h $(INCSYS)/sys/signal.h \
	$(INCSYS)/sys/types.h $(INCSYS)/sys/sysmacros.h \
	$(INCSYS)/sys/filsys.h 
	$(CC) $(CFLAGS) -o $(TESTDIR)/labelit labelit.c $(LDFLAGS) $(SHLIBS)

clobber_labelit:
	rm -f $(TESTDIR)/labelit

install_labelit: labelit
	@if [ ! -d $(ROOT)/usr/lib/fs ]; \
		then \
		mkdir $(ROOT)/usr/lib/fs; \
		fi;
	@if [ ! -d $(INSDIR1) ]; \
		then \
		mkdir $(INSDIR1); \
		fi;
	$(INS) -f $(INSDIR1) -m 0555 -u bin -g bin $(TESTDIR)/labelit


#
#  This is to build s5 specific mkfs command
#

mkfs:   $(INC)/stdio.h  \
	$(INC)/a.out.h \
	$(INC)/signal.h  \
	$(INCSYS)/sys/types.h \
	$(INCSYS)/sys/fs/s5macros.h  \
	$(INCSYS)/sys/vnode.h \
	$(INCSYS)/sys/param.h  \
	$(INCSYS)/sys/fs/s5param.h \
	$(INCSYS)/sys/fs/s5fblk.h  \
	$(INCSYS)/sys/fs/s5dir.h \
	$(INCSYS)/sys/fs/s5filsys.h  \
	$(INCSYS)/sys/fs/s5ino.h \
	$(INCSYS)/sys/stat.h  \
	$(INCSYS)/sys/fs/s5inode.h
	$(CC) $(LDFLAGS) $(CFLAGS) $(IFLAG) -o $(TESTDIR)/mkfs mkfs.c $(LIBELF) $(ROOTLIBS)

install_mkfs: mkfs
	@if [ ! -d $(ROOT)/usr/lib/fs ]; \
		then \
		mkdir $(ROOT)/usr/lib/fs; \
		fi;
	@if [ ! -d $(INSDIR1) ]; \
		then \
		mkdir $(INSDIR1); \
		fi;
	@if [ ! -d $(ROOT)/etc/fs ]; \
		then \
		mkdir $(ROOT)/etc/fs; \
		fi;
	@if [ ! -d $(INSDIR2) ]; \
		then \
		mkdir $(INSDIR2); \
		fi;
	$(INS) -f $(INSDIR1) -m 0555 -u bin -g bin $(TESTDIR)/mkfs 
	$(INS) -f $(INSDIR2) -m 0555 -u bin -g bin $(TESTDIR)/mkfs 

clobber_mkfs: 
	rm -f $(TESTDIR)/mkfs 


#
#  This is to build s5 specific ncheck command
#

ncheck: ncheck.c 
	if [ x$(IFLAG) != x-i ]  ; then \
	$(CC) $(LDFLAGS) -Dsmall=-1 $(CFLAGS)  $(IFLAG) -o $(TESTDIR)/ncheck ncheck.c $(SHLIBS) ; \
	else $(CC) $(LDFLAGS) $(CFLAGS) $(IFLAG) -o $(TESTDIR)/ncheck ncheck.c $(SHLIBS) ; \
	fi

install_ncheck: ncheck
	@if [ ! -d $(ROOT)/usr/lib/fs ]; \
		then \
		mkdir $(ROOT)/usr/lib/fs; \
		fi;
	@if [ ! -d $(INSDIR1) ]; \
		then \
		mkdir $(INSDIR1); \
		fi;
	$(INS) -f $(INSDIR1) -m 0555 -u bin -g bin $(TESTDIR)/ncheck

clobber_ncheck: clean
	rm -f $(TESTDIR)/ncheck


#
# This is to build the s5 specific volcopy
#



volcopy:	 $(INCSYS)/sys/param.h \
		 $(INCSYS)/sys/fs/s5param.h \
		 $(INC)/signal.h \
		 $(INCSYS)/sys/signal.h \
		 $(INCSYS)/sys/types.h \
		 $(INCSYS)/sys/sysmacros.h \
		 $(INCSYS)/sys/filsys.h \
		 $(INCSYS)/sys/fs/s5filsys.h \
		 $(INCSYS)/sys/stat.h \
		 $(INC)/stdio.h \
		 $(INC)/archives.h \
		 $(INC)/libgenIO.h \
		 volcopy.h \
		 volcopy.c
		$(CC) $(CFLAGS) -o $(TESTDIR)/volcopy volcopy.c -lgenIO $(LDFLAGS) $(SHLIBS)


install_volcopy: volcopy 
	@if [ ! -d $(ROOT)/usr/lib/fs ]; \
		then \
		mkdir $(ROOT)/usr/lib/fs; \
		fi;
	@if [ ! -d $(INSDIR1) ]; \
		then \
		mkdir $(INSDIR1); \
		fi;
	$(INS) -f $(INSDIR1) -m 0555 -u bin -g bin $(TESTDIR)/volcopy

clobber_volcopy: clean
	rm -f $(TESTDIR)/volcopy



#
# This is to build the s5 specific ff 
#



ff:		 $(INCSYS)/sys/param.h \
		 $(INCSYS)/sys/types.h \
		 $(INCSYS)/sys/fcntl.h \
		 $(INCSYS)/sys/vnode.h \
		 $(INCSYS)/sys/fs/s5param.h \
		 $(INCSYS)/sys/fs/s5filsys.h \
		 $(INCSYS)/sys/fs/s5ino.h \
		 $(INCSYS)/sys/fs/s5inode.h \
		 $(INCSYS)/sys/fs/s5dir.h \
		 $(INCSYS)/sys/stat.h \
		 $(INC)/stdio.h \
		 $(INC)/pwd.h \
		 $(INC)/malloc.h 
		$(CC) $(CFLAGS) -o $(TESTDIR)/ff ff.c  $(LDFLAGS) $(SHLIBS)


install_ff: ff 
	@if [ ! -d $(ROOT)/usr/lib/fs ]; \
		then \
		mkdir $(ROOT)/usr/lib/fs; \
		fi;
	@if [ ! -d $(INSDIR1) ]; \
		then \
		mkdir $(INSDIR1); \
		fi;
	$(INS) -f $(INSDIR1) -m 0555 -u bin -g bin $(TESTDIR)/ff

clobber_ff: clean
	rm -f $(TESTDIR)/ff


# 
#  This is to build s5 specific dcopy command 
# 

dcopy:	$(INCSYS)/sys/signal.h\
	$(INCSYS)/sys/fcntl.h\
	$(INCSYS)/sys/param.h\
	$(INCSYS)/sys/types.h\
	$(INCSYS)/sys/vnode.h\
	$(INCSYS)/sys/stat.h \
	$(INCSYS)/sys/fs/s5param.h\
	$(INCSYS)/sys/fs/s5filsys.h\
	$(INCSYS)/sys/fs/s5ino.h\
	$(INCSYS)/sys/fs/s5inode.h\
	$(INCSYS)/sys/fs/s5dir.h\
	$(INCSYS)/sys/fs/s5fblk.h\
	$(INC)/stdio.h
	$(CC) $(CFLAGS) -o $(TESTDIR)/dcopy dcopy.c $(LDFLAGS) $(SHLIBS)

install_dcopy: dcopy
	@if [ ! -d $(ROOT)/usr/lib/fs ]; \
		then \
		mkdir $(ROOT)/usr/lib/fs; \
		fi;
	@if [ ! -d $(INSDIR1) ]; \
		then \
		mkdir $(INSDIR1); \
		fi;
	$(INS) -f $(INSDIR1) -m 0555 -u bin -g bin $(TESTDIR)/dcopy

clobber_dcopy:
	rm -f $(TESTDIR)/dcopy
