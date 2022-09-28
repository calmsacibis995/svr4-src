#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)face:src/xx/xx.mk	1.6"

INCLUDE= -I../filecab/inc

MKDIR	=	mkdir

LIBPROC= ../proc/libproc.a

CFLAGS = -O
LDFLAGS = -s
CMDS =	suspend slash

.c.o:
	$(CC) $(CFLAGS) $(INCLUDE) -c $<

.o:
	$(CC) $(LDFLAGS) -o $@ $? $(SHLIBS)

all:	$(CMDS)


clobber clean:
	/bin/rm -f facesuspend slash
	/bin/rm -f *.o


suspend: suspend.o $(LIBPROC)
	$(CC) $(LDFLAGS) -o facesuspend suspend.o $(LIBPROC) $(SHLIBS)

suspend.o: suspend.c ../filecab/inc/wish.h

slash:  slash.o
	$(CC) $(LDFLAGS) -o $@ slash.o $(SHLIBS)

slash.o: slash.c

 
install: all $(VMSYS) $(VMBIN) dir
	:  VMBIN
		cp facesuspend $(VMBIN)/facesuspend
		cp initial.txt $(VMBIN)/initial
		cp slash $(VMBIN)/slash
		cp cmdfile.txt $(VMBIN)/cmdfile
		cp chksys.sh $(VMBIN)/chksys
		cp unix.sh $(VMBIN)/unix
		chmod 755 $(VMBIN)/facesuspend
		chmod 755 $(VMBIN)/slash
		chmod 644 $(VMBIN)/initial
		chmod 644 $(VMBIN)/cmdfile
		chmod 755 $(VMBIN)/chksys
		chmod 755 $(VMBIN)/unix
	:  OBJECTS
		cd ../../OBJECTS; find . -print | cpio -pcvd  $(VMSYS)/OBJECTS
	:  HELP
		cd ../../HELP; find . -print | cpio -pcvd $(VMSYS)/HELP

$(VMSYS):
	-mkdir -p $@ ; chmod 755 $@

$(VMBIN):
	mkdir $@ ; chmod 755 $@

dir:
	-mkdir $(VMSYS)/OBJECTS 
	-mkdir $(VMSYS)/HELP 
	find $(VMSYS)/OBJECTS $(VMSYS)/HELP -type d -print -exec chmod 755 {} \;
