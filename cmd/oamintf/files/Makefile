#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)filemgmt:Makefile	1.1.3.2"
OAMBASE=/usr/sadm/sysadm
DIR = $(ROOT)/bin
INC = $(ROOT)/usr/include
FILEDIR = $(ROOT)$(OAMBASE)/menu/files
BINDIR = $(ROOT)$(OAMBASE)/bin
INS = install
STRIP = strip
SIZE = size

TASKS = check defaults diskuse display fileage filesize identify \
	list make mount unmount

O_CFILES = sysfs

O_SHFILES = Identify adddef admpriv call_fsck checkfilsys checkfs chkrun \
	    ckerrs defdev deffs dev devices devlst dirchk dispdef existsfs \
	    filesearch filsys findtype fsnames fstyp_special fstype \
	    getchoidis getchoimod getchoirem getchois getdefdev \
	    getdeffs getdiskuse getfilesize getfsname getumntdev ifdiskette \
	    indeftab invfstab isoam.ins isoam.read isoam.rem labelfsname \
	    makefs makesys mkfs_bfs mkfs_s5 mkfs_ufs moddef mountfs numblocks \
	    numfiles rmdef rmdisk rmfile ufs_fragsiz unmountfs valdefdev \
	    valdeffs validdev validfs

check = Form.check Form.checkfs Text.ckerrs Text.ckok Text.cknok \
	Text.findtype Help Menu.fsckch

defaults = Form.add Form.add2 Form.display Form.modify \
	   Form.modify2 Form.remove Text.display Text.adddef \
	   Text.add2 Text.moderr Text.modify2 Text.rem \
	   defaults.menu  Help Menu.fsdisplay Menu.fsmodify Menu.fsremove

diskuse = Form.diskuse Text.diskuse Text.duse2 Help

display = Text.dispfstyp Help

fileage = Form.fileage Text.searchnok Text.searchok  Help

filesize = Form.filesize Text.sizenok Text.sizeok  Help

identify = Form.identify Text.identify  Help

list = Text.listfiles  Help

make = Form.bfs Form.bfs2 Form.bfsins Form.erasefs Form.existsfs Form.make \
       Form.make2 Form.makefsys Form.mkinsert Form.mntpt Form.s5 Form.s52 \
       Form.ufs Form.ufs2 Form.ufsins Help Menu.mklst Text.erase Text.make \
       Text.mkr Menu.mkfsch

mount = Form.mount Form.mount2 Form.mount3 \
	Text.findtype Text.mountok Help Menu.mntch

unmount = Form.unmount Form.umountfs Menu.umntdev \
          Text.unmount  Help

O_DFILES = file.menu  Help Menu.fsdevch Menu.fsdevbk

all: $(O_CFILES) $(O_SHFILES) 


$(O_CFILES):
	$(CC) -s -O bin/$(@).c  -o bin/$(@)

$(O_SHFILES):
	cp bin/$(@).sh  bin/$(@)

clean:

clobber:
	( cd bin ; \
	rm -f $(O_SHFILES) ; \
	rm -f $(O_CFILES) \
	)

install: all $(FILEDIR) 
# check
	-if [ ! -d $(FILEDIR)/check ] ;\
	then \
		mkdir -p $(FILEDIR)/check ;\
	fi
	for i in $(check) ;\
	do \
		$(INS) -m 644 -g bin -u bin -f $(FILEDIR)/check check/$$i ;\
	done


# defaults  
	-if [ ! -d $(FILEDIR)/defaults ] ;\
	then \
		mkdir -p $(FILEDIR)/defaults ;\
	fi
	for i in $(defaults) ;\
	do \
		$(INS) -m 644 -g bin -u bin -f $(FILEDIR)/defaults defaults/$$i ;\
	done


#diskuse
	-if [ ! -d $(FILEDIR)/diskuse ] ;\
	then \
		mkdir -p $(FILEDIR)/diskuse ;\
	fi
	for i in $(diskuse) ;\
	do \
		$(INS) -m 644 -g bin -u bin -f $(FILEDIR)/diskuse diskuse/$$i ;\
	done


#display
	-if [ ! -d $(FILEDIR)/display ] ;\
	then \
		mkdir -p $(FILEDIR)/display ;\
	fi
	for i in $(display) ;\
	do \
		$(INS) -m 644 -g bin -u bin -f $(FILEDIR)/display display/$$i ;\
	done


#fileage 
	-if [ ! -d $(FILEDIR)/fileage ] ;\
	then \
		mkdir -p $(FILEDIR)/fileage ;\
	fi
	for i in $(fileage) ;\
	do \
		$(INS) -m 644 -g bin -u bin -f $(FILEDIR)/fileage fileage/$$i ;\
	done


#filesize 
	-if [ ! -d $(FILEDIR)/filesize ] ;\
	then \
		mkdir -p $(FILEDIR)/filesize ;\
	fi
	for i in $(filesize) ;\
	do \
		$(INS) -m 644 -g bin -u bin -f $(FILEDIR)/filesize filesize/$$i ;\
	done


#identify
	-if [ ! -d $(FILEDIR)/identify ] ;\
	then \
		mkdir -p $(FILEDIR)/identify ;\
	fi
	for i in $(identify) ;\
	do \
		$(INS) -m 644 -g bin -u bin -f $(FILEDIR)/identify identify/$$i ;\
	done

#list 
	-if [ ! -d $(FILEDIR)/list ] ;\
	then \
		mkdir -p $(FILEDIR)/list ;\
	fi
	for i in $(list) ;\
	do \
		$(INS) -m 644 -g bin -u bin -f $(FILEDIR)/list list/$$i ;\
	done

#make 
	-if [ ! -d $(FILEDIR)/make ] ;\
	then \
		mkdir -p $(FILEDIR)/make ;\
	fi
	for i in $(make) ;\
	do \
		$(INS) -m 644 -g bin -u bin -f $(FILEDIR)/make make/$$i ;\
	done

#mount 
	-if [ ! -d $(FILEDIR)/mount ] ;\
	then \
		mkdir -p $(FILEDIR)/mount ;\
	fi
	for i in $(mount) ;\
	do \
		$(INS) -m 644 -g bin -u bin -f $(FILEDIR)/mount mount/$$i ;\
	done

#unmount
	-if [ ! -d $(FILEDIR)/unmount ] ;\
	then \
		mkdir -p $(FILEDIR)/unmount ;\
	fi
	for i in $(unmount) ;\
	do \
		$(INS) -m 644 -g bin -u bin -f $(FILEDIR)/unmount unmount/$$i ;\
	done


#main
	for i in $(O_DFILES) ;\
	do \
		$(INS) -m 644 -g bin -u bin -f $(FILEDIR) $$i ;\
	done

	for i in $(O_SHFILES) ;\
	do \
		$(INS) -m 755 -g bin -u bin -f $(BINDIR) bin/$$i ;\
	done

	for i in $(O_CFILES) ;\
	do \
		$(INS) -m 755 -g bin -u bin -f $(BINDIR) bin/$$i ;\
	done

size: all
	$(SIZE)

strip: all
	$(STRIP)


$(FILEDIR):
	-if [ ! -d `dirname $(FILEDIR)` ] ;\
	then \
		mkdir -p `dirname $(FILEDIR)` ;\
	fi
	-if [ ! -d $(FILEDIR) ] ;\
	then \
		mkdir -p $(FILEDIR) ;\
	fi

$(TASKS):
	-if [ ! -d $(FILEDIR)/$(@) ] ;\
	then \
		mkdir -p $(FILEDIR)/$(@) ;\
	fi
