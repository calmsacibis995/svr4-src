#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)fs.cmds:fs.d.mk	1.31.4.1"

#  /usr/src/cmd/fs.d is the directory of all generic commands
#  whose executable reside in $(INSDIR).
#  Fstype specific commands are in subdirectories under fs.d
#  named by fstype (ex: the generic mount is in this directory and
#  built by this makefile, but the s5 specific mount in in ./s5/mount.c,
#  built by ./s5/s5.mk)

TESTDIR = .
SYMLINK = :
INSDIR = $(ROOT)/sbin
INSDIR2= $(ROOT)/usr/sbin
BINDIR = $(ROOT)/usr/bin
OLDBINDIR = $(ROOT)/bin
OLDDIR = $(ROOT)/etc
INS = install
INC = $(ROOT)/usr/include
INCSYS = $(ROOT)/usr/include
CFLAGS = -O -I$(INC) -I$(INCSYS)
LDFLAGS = -s

all: all_fstyp df umount mount switch fsck volcopy ff

clean: clean_fstyp
	rm -f *.o

clobber: clobber_fstyp clobber_mount clobber_switch clobber_fsck \
         clobber_volcopy clobber_umount clobber_df  clobber_ff
	rm -f *.o

install: install_fstyp install_mount install_umount  install_df \
	install_switch install_fsck install_volcopy  install_ff

#
#  This is to build all the fstype specific commands
#

all_fstyp:
	@for i in *;\
	do\
	    if [ -d $$i -a -f $$i/$$i.mk ]; \
		then \
		cd  $$i;\
		$(MAKE) -f $$i.mk "MAKE=$(MAKE)" "AS=$(AS)" "CC=$(CC)" "LD=$(LD)"  "INC=$(INC)" "INCSYS=$(INCSYS)" "INS=$(INS)" "SYMLINK=$(SYMLINK)" "SHLIBS=$(SHLIBS)" "ROOTLIBS=$(ROOTLIBS)" "ROOT=$(ROOT)" all ; \
		cd .. ; \
	    fi;\
	done

install_fstyp:
	@for i in *;\
	do\
	    if [ -d $$i -a -f $$i/$$i.mk ]; \
		then \
		cd $$i;\
		$(MAKE) -f $$i.mk "MAKE=$(MAKE)" "AS=$(AS)" "CC=$(CC)" "LD=$(LD)"  "INC=$(INC)" "INCSYS=$(INCSYS)" "INS=$(INS)" "SYMLINK=$(SYMLINK)" "SHLIBS=$(SHLIBS)" "ROOTLIBS=$(ROOTLIBS)" "ROOT=$(ROOT)" "INS=$(INS)" install ; \
		cd .. ; \
		fi;\
	done

clean_fstyp:
	@for i in *;\
	do\
	    if [ -d $$i -a -f $$i/$$i.mk ]; \
		then \
			cd $$i;\
			$(MAKE) -f $$i.mk clean; \
			cd .. ; \
		fi;\
	done

clobber_fstyp:
	@for i in *;\
	do\
	    if [ -d $$i -a -f $$i/$$i.mk ]; \
		then \
			cd $$i;\
			$(MAKE) -f $$i.mk clobber; \
			cd .. ; \
		fi;\
	done

#
# This is for the generic mount command
#
mount:	mount.c\
	$(INC)/stdio.h\
	$(INC)/limits.h\
	$(INCSYS)/sys/types.h\
	$(INCSYS)/sys/stat.h\
	$(INCSYS)/sys/statvfs.h\
	$(INCSYS)/sys/errno.h\
	$(INCSYS)/sys/mnttab.h\
	$(INCSYS)/sys/vfstab.h
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(TESTDIR)/mount mount.c $(ROOTLIBS)

clobber_mount:
	rm -f $(TESTDIR)/mount

install_mount: mount
	-rm -f $(OLDDIR)/mount
	-rm -f $(INSDIR2)/mount
	$(INS) -f $(INSDIR) $(TESTDIR)/mount
	$(INS) -f $(INSDIR2) $(TESTDIR)/mount
	$(SYMLINK) $(INSDIR)/mount $(OLDDIR)/mount


#
# This is for the generic umount command
#
umount:	umount.c\
	$(INC)/stdio.h \
	$(INC)/limits.h \
	$(INC)/signal.h \
	$(INC)/unistd.h \
	$(INCSYS)/sys/mnttab.h \
	$(INCSYS)/sys/errno.h \
	$(INCSYS)/sys/types.h \
	$(INCSYS)/sys/stat.h \
	$(INCSYS)/sys/param.h
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(TESTDIR)/umount umount.c $(ROOTLIBS)

clobber_umount:
	rm -f $(TESTDIR)/umount

install_umount: umount
	-rm -f  $(OLDDIR)/umount 
	-rm -f $(INSDIR2)/umount
	$(INS) -f $(INSDIR) $(TESTDIR)/umount
	$(INS) -f $(INSDIR2) $(TESTDIR)/umount
	$(SYMLINK) $(INSDIR)/umount $(OLDDIR)/umount


#
# This is for the generic df command
#

df:	df.c\
	$(INC)/stdio.h\
	$(INC)/limits.h\
	$(INC)/fcntl.h\
	$(INCSYS)/sys/types.h\
	$(INCSYS)/sys/stat.h\
	$(INCSYS)/sys/statvfs.h\
	$(INCSYS)/sys/errno.h\
	$(INCSYS)/sys/mnttab.h\
	$(INCSYS)/sys/vfstab.h
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(TESTDIR)/df df.c $(ROOTLIBS)

clobber_df:
	rm -f $(TESTDIR)/df

install_df: df
	-rm -f $(OLDBINDIR)/df
	-rm -f $(BINDIR)/df
	$(INS) -f $(INSDIR) $(TESTDIR)/df
	$(INS) -f $(INSDIR2) $(TESTDIR)/df
	$(SYMLINK) $(INSDIR)/df $(BINDIR)/df

#
#  This is the switchout for ff, ncheck and any command which accepts
#  multiple specials;
#
ff:	ff.c\
	$(INC)/stdio.h\
	$(INC)/limits.h\
	$(INC)/string.h\
	$(INCSYS)/sys/fstyp.h\
	$(INCSYS)/sys/errno.h\
	$(INCSYS)/sys/vfstab.h
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(TESTDIR)/ff ff.c $(ROOTLIBS)

clobber_ff:
	rm -f $(TESTDIR)/ff

install_ff: ff 
	-rm -f $(INSDIR2)/ff
	-rm -f $(OLDDIR)/ff
	$(INS) -f $(INSDIR2) $(TESTDIR)/ff
	$(SYMLINK) $(INSDIR2)/ff $(OLDDIR)/ff
	-rm -f $(INSDIR2)/ncheck
	-rm -f $(OLDDIR)/ncheck
	ln $(INSDIR2)/ff $(INSDIR2)/ncheck
	$(SYMLINK) $(INSDIR2)/ncheck $(OLDDIR)/ncheck

# generic fsck
fsck:	fsck.c\
	$(INC)/stdio.h\
	$(INC)/limits.h\
	$(INCSYS)/sys/errno.h\
	$(INCSYS)/sys/types.h\
	$(INCSYS)/sys/vfstab.h
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(TESTDIR)/fsck fsck.c $(ROOTLIBS)

clobber_fsck:
	rm -f $(TESTDIR)/fsck

install_fsck: fsck
	-rm -f $(OLDDIR)/fsck 
	-rm -f $(INSDIR)/fsck
	-rm -f $(INSDIR2)/fsck
	$(INS) -f $(INSDIR) $(TESTDIR)/fsck
	$(INS) -f $(INSDIR2) $(TESTDIR)/fsck
	$(SYMLINK) $(INSDIR)/fsck $(OLDDIR)/fsck

# generic volcopy
volcopy: volcopy.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(TESTDIR)/$@ $@.c

clobber_volcopy:
	rm -f $(TESTDIR)/volcopy

install_volcopy: volcopy
	-rm -f  $(OLDDIR)/volcopy 
	-rm -f  $(INSDIR2)/volcopy 
	$(INS) -f $(INSDIR2) $(TESTDIR)/volcopy
	$(SYMLINK) $(INSDIR2)/volcopy $(OLDDIR)/volcopy

#
#  This is for the switchout
#
switch:	switchout.c\
	$(INCSYS)/sys/types.h\
	$(INCSYS)/sys/stat.h\
	$(INC)/stdio.h\
	$(INCSYS)/sys/fcntl.h\
	$(INCSYS)/sys/fstyp.h\
	$(INCSYS)/sys/errno.h\
	$(INC)/limits.h
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(TESTDIR)/switch switchout.c $(ROOTLIBS)

clobber_switch:
	rm -f $(TESTDIR)/switch $(TESTDIR)/clri $(TESTDIR)/switchout

install_switch: switch
	-rm -f $(TESTDIR)/clri
	-rm -f $(TESTDIR)/switchout
	-rm -f $(OLDDIR)/clri 
	-rm -f $(INSDIR)/clri
	-rm -f $(INSDIR2)/clri
	ln $(TESTDIR)/switch $(TESTDIR)/switchout
	ln $(TESTDIR)/switch $(TESTDIR)/clri
	$(INS) -f $(INSDIR) $(TESTDIR)/clri
	$(INS) -f $(INSDIR2) $(TESTDIR)/clri
	$(SYMLINK) $(INSDIR)/clri $(OLDDIR)/clri
	-rm -f $(INSDIR2)/switchout
	$(INS) -f $(INSDIR2) $(TESTDIR)/switchout
	-rm -f $(OLDDIR)/fsdb 
	-rm -f $(INSDIR)/fsdb
	-rm -f $(INSDIR2)/fsdb
	ln $(INSDIR)/clri $(INSDIR)/fsdb
	$(INS) -f $(INSDIR2) $(INSDIR)/fsdb
	$(SYMLINK) $(INSDIR)/fsdb  $(OLDDIR)/fsdb
	-rm -f $(OLDDIR)/labelit 
	-rm -f $(INSDIR2)/labelit
	ln $(INSDIR2)/clri $(INSDIR2)/labelit
	$(SYMLINK) $(INSDIR2)/labelit $(OLDDIR)/labelit
	-rm -f $(OLDDIR)/mkfs 
	-rm -f $(INSDIR)/mkfs
	-rm -f $(INSDIR2)/mkfs
	ln $(INSDIR)/clri $(INSDIR)/mkfs
	$(INS) -f $(INSDIR2) $(INSDIR)/mkfs
	$(SYMLINK) $(INSDIR)/mkfs $(OLDDIR)/mkfs
	-rm -f $(OLDDIR)/dcopy 
	-rm -f $(INSDIR2)/dcopy
	ln $(INSDIR2)/clri $(INSDIR2)/dcopy
	$(SYMLINK) $(INSDIR2)/dcopy $(OLDDIR)/dcopy
