#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)libnsl:nsl/nsl.mk	1.14.1.1"
# 
# Network services library
#

ROOT=
USRLIB=$(ROOT)/usr/lib
SHLIB=$(ROOT)/shlib
INC=$(ROOT)/usr/include
CFLAGS= -I$(INC) -O  -Kpic

LIBOBJS= t_accept.o t_bind.o t_connect.o t_error.o t_close.o\
	 t_getinfo.o t_getstate.o t_listen.o t_look.o\
	 t_rcv.o t_rcvconnect.o t_rcvdis.o t_snd.o t_snddis.o\
	 t_unbind.o t_optmgmt.o\
	 t_rcvudata.o t_rcvuderr.o t_sndudata.o t_sndrel.o t_rcvrel.o\
	 t_alloc.o t_free.o t_open.o t_sync.o t_getname.o\
	 _dummy.o _errlst.o _data.o _data2.o _conn_util.o _utility.o \
	 __free_def.o __calloc_def.o __perror_def.o __strlen_def.o\
	 __write_def.o __ioctl_def.o __putmsg_def.o __getmsg_def.o\
	 __errno_def.o __memcpy_def.o __fcntl_def.o __sigset_def.o\
	 __open_def.o __close_def.o __ulimit_def.o __getpmsg_df.o \
	 __putpmsg_df.o __patch.o

OBJS= ../t_accept.o ../t_bind.o ../t_connect.o ../t_error.o ../t_close.o\
	 ../t_getinfo.o ../t_getstate.o ../t_listen.o ../t_look.o\
	 ../t_rcv.o ../t_rcvconnect.o ../t_rcvdis.o ../t_snd.o ../t_snddis.o\
	 ../t_unbind.o ../t_optmgmt.o ../t_rcvudata.o ../_utility.o\
	 ../t_rcvuderr.o ../t_sndudata.o ../t_sndrel.o ../t_rcvrel.o\
	 ../t_alloc.o ../t_free.o ../t_open.o ../t_sync.o ../t_getname.o\
	 ../_dummy.o ../_errlst.o ../_data.o ../_data2.o ../_conn_util.o\
	 ../__free_def.o ../__calloc_def.o ../__perror_def.o ../__strlen_def.o\
	 ../__write_def.o ../__ioctl_def.o ../__putmsg_def.o ../__getmsg_def.o\
	 ../__errno_def.o ../__memcpy_def.o ../__fcntl_def.o ../__sigset_def.o\
	 ../__open_def.o ../__close_def.o ../__ulimit_def.o ../__getpmsg_df.o\
	 ../__putpmsg_df.o ../__patch.o

PLIBOBJS= t_accept.o t_bind.o t_connect.o t_error.o t_close.o\
	 t_getinfo.o t_getstate.o t_listen.o t_look.o\
	 t_rcv.o t_rcvconnect.o t_rcvdis.o t_snd.o t_snddis.o\
	 t_unbind.o t_optmgmt.o\
	 t_rcvudata.o t_rcvuderr.o t_sndudata.o t_sndrel.o t_rcvrel.o\
	 t_alloc.o t_free.o t_open.o t_sync.o t_getname.o\
	 _dummy.o _errlst.o _data.o _data2.o _conn_util.o _utility.o 

INCLUDES=  	$(INC)/sys/param.h\
		$(INC)/sys/types.h\
		$(INC)/sys/errno.h\
		$(INC)/sys/stream.h\
		$(INC)/sys/stropts.h\
		$(INC)/sys/tihdr.h\
		$(INC)/sys/timod.h\
		$(INC)/sys/tiuser.h\
		$(INC)/sys/signal.h\
		./_import.h

all:
	if [ x$(CCSTYPE) = xCOFF ] ; \
	then \
		$(MAKE) -f nsl.mk allcoff ROOT=$(ROOT) ; \
	else \
		$(MAKE) -f nsl.mk allelf ROOT=$(ROOT) ; \
	fi

allelf:       $(INCLUDES) $(PLIBOBJS)

allcoff:      $(INCLUDES) $(LIBOBJS)

install:

clean:
	-rm -f *.o 

clobber:	clean
	-rm -f $(OBJS)

.c.o:
	$(CC) -DNO_IMPORT $(CFLAGS) -c $*.c && cp $(*F).o ..
