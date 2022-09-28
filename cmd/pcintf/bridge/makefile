#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)pcintf:bridge/makefile	1.1"
# 
#		 	PC Interface Unix Makefile.
#			@(#)makefile	3.86	LCC Modified: 2/26/90 16:50:05
#
#  WHAT : 	This makefile is desgined to make different versions of PCI for
# 		different systems. 
#
#  HOW  :       By changing the current directory to the system_label which the
#		make was invoked w/. All the source files (*.c) & header files
#		(*.h) may be referenced from common directory (..).
#
#
#				+-------------------------+
#				|    Common Directory :   |
#				| makefile, make.mstr     |
#				|       *.c & *.h         |
#				+-------------------------+
#				  |   |                 |
#			         /    |                  \
#		               /      |       . . .        \
#			     /        |                      \
#		+---------------+ +---------------+       +---------------+ 
#		| *.o & targets | | *.o & Targets | . . . | *.o & Targets |
#		+---------------+ +---------------+       +---------------+ 
#		  udps5.dir	    leth.dir	            lrs232.dir
#
#
#		The function of this primary makefile is to set proper flags
#		for CC, LD, & LIB, and invoke the make again w/ the secondary
#		makefile called make.mstr.

default:	svr4232 svr4eth svr430

# RIDGE 4.2 ETHERNET version
ridge : 
	cd ridge.dir; \
	$(MAKE) -f ../make.mstr \
		"DFLAGS = -DUDP42 -DRIDGE -DSYS5 -DETHNETPCI -DSTDIO_REDIR" \
		"IFLAGS = -I.. -I/usr/ucb.include -I/usr/include -I/usr/include/sys" \
		"LIBFLAGS = -linet" \
		pci

#  Sperry TOWER sys5.2 + 4.1C networking version
udp41c : 
	cd udp41c.dir; \
	$(MAKE) -f ../make.mstr \
		"DFLAGS = -DXLN_BUG -DUDP41C -DSYS5 -DETHNETPCI -DM68000 \
		          -DDEBUG" \
		"IFLAGS = -n -I.. -I/appl/EXOS/include/EXOS \
			  -I/usr/include/NET-5000 -I/usr/include/sys" \
		"LIBFLAGS = -lsocket"  \
		pci

#  Sperry ARETE sys5.2 + 4.1C networking version
arete : 
	cd arete.dir; \
	$(MAKE) -f ../make.mstr \
		"DFLAGS = -DXLN_BUG -DUDP41C -DSYS5 -DETHNETPCI -DM68000 -DDEBUG" \
		"IFLAGS = -I.. -I/usr/include/NET-5000 -I/usr/include/sys" \
		"LIBFLAGS = -lsocket"  \
		pci

# UniPlus+ (tm) sys5.2 + 4.2 networking version
udps5 : 
	cd udps5.dir; \
	$(MAKE) -f ../make.mstr \
		"DFLAGS = -DUNISOFT -DUDP42 -DSYS5 -DETHNETPCI -DSTDIO_REDIR" \
		"IFLAGS = -I.. -I/usr/include -I/usr/include/sys" \
		"LIBFLAGS = -lnet"  \
		pci


# LOCUS Ethernet version
leth :
	cd leth.dir; \
	$(MAKE) -f ../make.mstr \
		"DFLAGS = -DETHNETPCI -DETHLOCUS -DSTDIO_REDIR \
			  -DMAPSVR3BNET -DMAPSVRBCAST -DPARAM_GETS_TYPES" \
		"IFLAGS = -I.. -I/usr/6include/sys -I/usr/6include  \
			= -I/usr/include/sys" \
		"LIBFLAGS = "  \
		"CC = cc" \
		pci


# BERKELEY 4.2 ETHERNET version
udp42 : 
	cd udp42.dir; \
	make -f ../make.mstr \
		"DFLAGS = -DUDP42 -DBERKELEY42 -DETHNETPCI -DDOPRNT41 \
			  -DPARAM_GETS_TYPES -DRESTRICT_USER -DSTDIO_REDIR \
			  -DBERK42FILE " \
		"IFLAGS = -I.. -I../h.42 -I../h.s5 \
			   -I/usr/include/sys" \
		"LIBFLAGS =" \
		pci


# BERKELEY 4.3 ETHERNET version
udp43 : 
	cd udp43.dir; \
	make -f ../make.mstr \
		"DFLAGS = -DUDP42 -DBSD43 -DBERKELEY42 -DETHNETPCI -DDOPRNT41 \
			  -DPARAM_GETS_TYPES -DRESTRICT_USER -DSTDIO_REDIR \
			  -DBERK42FILE" \
		"IFLAGS = -I.. -I../h.42 -I../h.s5 \
			   -I/usr/include/sys" \
		"LIBFLAGS =" \
		pci


# ICT MiniFrame 4.2 ETHERNET version
ict : 
	../ict.tools/brk2ict	# convert berkeley flex names to ICT flex
	rm -rf arpa; mkdir arpa; cp inet.h arpa
	cd ict.dir; \
	$(MAKE) -f ../make.mstr \
		"DFLAGS = -DUDP42 -DSYS5 -DETHNETPCI -DPARAM_GETS_TYPES -DSTDIO_REDIR -DICT" \
		"IFLAGS = -I.. -I/usr/include -I/usr/include/sys" \
		"LIBFLAGS = -lsocket" \
		pci

# ICT Mighty Frame ETHERNET version
ict3 : 
	rm -rf arpa; mkdir arpa; cp inet.h arpa
	cd ict3.dir; \
	$(MAKE) -f ../make.mstr \
		"DFLAGS = -DUDP42 -DSYS5 -DETHNETPCI -DPARAM_GETS_TYPES -DSTDIO_REDIR -DICT" \
		"IFLAGS = -I.. -I/usr/include -I/usr/include/sys" \
		"LIBFLAGS = -lsocket" \
		pci


#3bnet System V RS232 version (record locking)
net3b232:
	cd net3b232.dir; \
	$(MAKE) -f ../make.mstr \
		"DFLAGS = -DSYS5 -DRS232PCI -DBINLP -DSTDIO_REDIR \
			  -DDEBUG -DRLOCK -DATT3B2" \
		"IFLAGS  = -I.. -I/usr/include/sys" \
		"LIBFLAGS =" \
		3brs232

# 3B SYS5 version   (Record locking version)
# Note -DDEBUG must be left in INITFLAGS for the sharectl program to be
# able to dump the shared memory.
net3brl : 
	cd net3brl.dir; \
	$(MAKE) -f ../make.mstr \
		"DFLAGS = -DRD5 -DSYS5 -DETHNETPCI -DETH3BNET -DMAPSVR3BNET -DBINLP \
			  -DSTDIO_REDIR -DRLOCK -DATT3B2" \
		"IFLAGS = -I.. -I/usr/include/sys" \
		"LIBFLAGS =" \
		"RLFLAGS =" \
		"SVRFLAGS = -DRLOCK" \
		"INITFLAGS = -DDEBUG -DRLOCK" \
		3bpcirl

# 3B SYS5 version   (no record locking)
net3b : 
	cd net3b.dir; \
	$(MAKE) -f ../make.mstr \
		"DFLAGS = -DRD5 -DSYS5 -DETHNETPCI -DETH3BNET -DMAPSVR3BNET -DBINLP \
			  -DSTDIO_REDIR -DATT3B2" \
		"IFLAGS = -I.. -I/usr/include/sys" \
		"LIBFLAGS =" \
		3bpci

# Manufactured 3B SYS5 version
man3b : 
	cd man3b.dir; \
	$(MAKE) -f ../make.mstr \
		"DFLAGS = -DRD5 -DSYS5 -DETHNETPCI -DETH3BNET -DMAPSVR3BNET -DBINLP \
			  -DSTDIO_REDIR -DATT3B2" \
		"IFLAGS = -I.. -I/usr/include/sys" \
		"LIBFLAGS =" \
		3bpci


# Magnum System V.3 with CMC/4.2 Networking  (Record locking version)
# Note -DDEBUG must be left in INITFLAGS for the sharectl program to be
# able to dump the shared memory.
M5ethrl : 
	cd M5ethrl.dir; \
	$(MAKE) -f ../make.mstr \
		"DFLAGS = -DM_386 -DSYS5 -DETHNETPCI -DUDP42 -DBINLP \
			  -DSTDIO_REDIR -DRLOCK -DPRIME" \
		"IFLAGS  = -I.. -I/usr/include -I/usr/include/sys" \
		"LIBFLAGS = -lnet" \
		"RLFLAGS =" \
		"SVRFLAGS = -DRLOCK" \
		"INITFLAGS = -DDEBUG -DRLOCK" \
		3bpcirl

# Magnum System V.3 with CMC/4.2 Networking  (no record locking)
M5eth : 
	cd M5eth.dir; \
	$(MAKE) -f ../make.mstr \
		"DFLAGS = -DM_386 -DSYS5 -DETHNETPCI -DUDP42 -DBINLP \
			  -DSTDIO_REDIR" \
		"IFLAGS  = -I.. -I/usr/include -I/usr/include/sys" \
		"LIBFLAGS = -lnet" \
		pci	

# System V RS232 version
s5232 : 
	cd s5232.dir; \
	$(MAKE) -f ../make.mstr \
		"DFLAGS = -DSYS5 -DRS232PCI -DBINLP -DSTDIO_REDIR" \
		"IFLAGS  = -I.. -I/usr/include/sys" \
		"LIBFLAGS =" \
		rs232

# Ridge RS232 version
r5232 : 
	cd r5232.dir; \
	$(MAKE) -f ../make.mstr \
		"DFLAGS = -DRIDGE -DSYS5 -DRS232PCI -DBINLP -DSTDIO_REDIR" \
		"IFLAGS  = -I.. -I/usr/include/sys" \
		"LIBFLAGS =" \
		rs232

# ICT RS232 version
ict232 : 
	cd ict232.dir; \
	$(MAKE) -f ../make.mstr \
		"DFLAGS = -DSYS5 -DRS232PCI -DBINLP -DSTDIO_REDIR -DICT" \
		"IFLAGS  = -I.. -I/usr/include -I/usr/include/sys" \
		"LIBFLAGS =" \
		rs232


# System V.3 MAGNUM RS232 version
M5232 : 
	cd M5232.dir; \
	$(MAKE) -f ../make.mstr \
		"DFLAGS = -DM_386 -DSYS5 -DRS232PCI -DBINLP -DSTDIO_REDIR" \
		"IFLAGS  = -I.. -I/usr/include -I/usr/include/sys" \
		"LIBFLAGS =" \
		rs232


# BERKELEY 4.2/4.3 RS232 version
bsd232 : 
	cd bsd232.dir; \
	make -f ../make.mstr \
		"DFLAGS = -DBERKELEY42 -DRS232PCI -DDOPRNT41  -DBERK42FILE \
			   -DBSD43 -DPARAM_GETS_TYPES -DSTDIO_REDIR" \
		"IFLAGS = -I.. -I../h.42 -I../h.s5 \
			   -I/usr/include/sys" \
		"LIBFLAGS =" \
		rs232

# DEC Ultrix 2.0 RS-232
ult232 : 
	cd ult232.dir; \
	make -f ../make.mstr \
		"DFLAGS = -DULTRIX20 -DBERKELEY42 -DRS232PCI -DDOPRNT41 \
			  -DBSD43 -DPARAM_GETS_TYPES -DSTDIO_REDIR \
			  -DBERK42FILE" \
		"IFLAGS = -I.. -I../h.42 -I../h.s5 \
			   -I/usr/include/sys" \
		"LIBFLAGS =" \
		rs232

# TOWER Ethernet version
tower :
	cd tower.dir; \
	$(MAKE) -f ../make.mstr \
	"DFLAGS = -DSYS5 -DETHNETPCI -DETHLOCUS -DSTDIO_REDIR -DBINLP \
		-DMAPSVR3BNET -DMAPSVRBCAST" \
	"IFLAGS = -n -I.. -I/usr/include/sys -I/usr/include" \
	"LIBFLAGS = " \
	pci

# Motorola 6300 System V
moto :
	cd moto.dir; \
	$(MAKE) -f ../make.mstr \
	"DFLAGS = -DSYS5 -DETHNETPCI -DETHLOCUS -DSTDIO_REDIR -DBINLP \
	-DMAPSVR3BNET -DMAPSVRBCAST -DSYS5IPC" \
	"IFLAGS = -I.. -I/usr/include/sys -I/usr/include" \
	"LIBFLAGS = " pci


# Note -DDEBUG must be left in INITFLAGS for the sharectl program to be
# able to dump the shared memory.

#	"DFLAGS = -LARGE -M3e -UM_I86 -UM_I186 -DM_I286 -DM_KERNEL -DXENIX -DSYS5 -DETHNETPCI -DUDP42 -DBINLP -DUSER64 -DSIGNAL_KNOWN -DFAST_LSEEK -DSTDIO_REDIR -DRD3 -DRLOCK"
#-DSYS5_3 -DBSD43 -DSTREAMNET"


x30e :
	cd loadpci.dir; \
	make -f ../make.mstr \
	"DFLAGS = -M3 -DiAPX386 -DI8086 -DXENIX -DSYS5 -DETHNETPCI -DEXCELAN -DUDP42 -DSIGNAL_KNOWN" \
	"IFLAGS = -I.. -I/usr/include -I/usr/include/sys -I/usr/include/exos" \
	"LDFLAGS = -lx -lsocket" \
	loadpci; \
	mv loadpci loadpci.xe

# SCO XENIX System V Release 2.3.2 for Excelan Ethernet
#	define -DOLD_XENIX if previous to version 2.3
xethe :
	cd xethe.dir; \
	make -f ../make.mstr \
	"DFLAGS = -LARGE -M3 -DiAPX386 -DI8086 -DXENIX -DSYS5 -DETHNETPCI -DEXCELAN -DUDP42 -DBINLP -DUSER64 -DSIGNAL_KNOWN -DFAST_LSEEK -DSTDIO_REDIR -DRD3 -DRLOCK" \
	"IFLAGS = -I.. -I../../pkg_rlock -I/usr/include -I/usr/include/sys -I/usr/include/exos" \
	"SVRFLAGS = -DRLOCK" \
	"INITFLAGS = -DDEBUG -DRLOCK" \
	"LIBFLAGS = -lx -lsocket ../../pkg_rlock/librlock.a ../../pkg_lockset/liblset.a" \
	xenixpci loadpci

# SCO XENIX System V Release 2.3.2 for RS232
#	define -DOLD_XENIX if previous to version 2.3

x232 :
	cd x232.dir; \
	make -f ../make.mstr \
	"DFLAGS = -LARGE -M3 -DiAPX386 -DI8086 -DXENIX -DSYS5 -DRS232PCI -DBINLP -DUSER64 -DSIGNAL_KNOWN -DFAST_LSEEK -DSTDIO_REDIR -DRD3 -DRLOCK" \
	"IFLAGS = -I.. -I../../pkg_rlock -I/usr/include -I/usr/include/sys" \
	"SVRFLAGS = -DRLOCK" \
	"INITFLAGS = -DDEBUG -DRLOCK" \
	"LIBFLAGS = -lx ../../pkg_rlock/librlock.a ../../pkg_lockset/liblset.a" \
	xenixrs

# XENIX System V 

xenix :
	cd xenix.dir; \
	makeL -f ../make.mstr \
	"DFLAGS =  -DRD3 -LARGE -DM_KERNEL -M2el -DXENIX -UM_I86 -UM_I186 \
		-DM_I286" \
	"IFLAGS = -I.. -I/usr/include/sys" \
	"LDFLAGS = -Ml" xenixpci


# XENIX System V RS232 version

xenix232 :
	cd xenix232.dir; \
	make -f ../make.mstr \
	"DFLAGS = -LARGE -M3 -DiAPX386 -DI8086 -DXENIX -DSYS5 -DRS232PCI -DBINLP -DUSER64 -DSIGNAL_KNOWN -DFAST_LSEEK -DSTDIO_REDIR -DRD3 -DRLOCK" \
	"IFLAGS = -I.. -I../../pkg_rlock -I/usr/include -I/usr/include/sys" \
	"SVRFLAGS = -DRLOCK" \
	"INITFLAGS = -DDEBUG -DRLOCK" \
	"LIBFLAGS = -lx ../../pkg_rlock/librlock.a ../../pkg_lockset/liblset.a" \
	xenixrs


# Motorola VME-8000 System V
vme :
	cd vme.dir; \
	$(MAKE) -f ../make.mstr \
	"DFLAGS = -DSYS5 -DETHERNET -DETHNETPCI -DETHLOCUS -DSTDIO_REDIR \
		-DBINLP -DMAPSVR3BNET -DMAPSVRBCAST -DRD5" \
	"IFLAGS = -I.. -I/usr/include/sys -I/usr/include" \
	"LIBFLAGS = " vme

# Sperry 7000 aka CCI 732 ETHERNET version
cci : 
	cd cci.dir; \
	make -f ../make.mstr \
		"DFLAGS = -DUDP42 -DSYS5 -DETHNETPCI -DDOPRNT41 \
			-DPARAM_GETS_TYPES -DSTDIO_REDIR -DCCI -DBERK42FILE" \
		"IFLAGS = -I.. -I../h.42 -I../h.s5 -I/usr/include/sys -I/usr/include" \
		"LIBFLAGS = -lnet -linet" \
		pci

# Sperry 7000 aka CCI 732 RS232 version
cci232 : 
	cd cci232.dir; \
	$(MAKE) -f ../make.mstr \
		"DFLAGS = -DSYS5 -DRS232PCI -DBINLP -DSTDIO_REDIR -DCCI -DBERK42FILE" \
		"IFLAGS  = -I.. -I/usr/include/sys" \
		"LIBFLAGS =" \
		rs232

#  Honeywell SGM sys5.2 + 4.1C networking version
sgm : 
	cd sgm.dir; \
	$(MAKE) -f ../make.mstr \
		"DFLAGS = -DXLN_BUG -DUDP41C -DSYS5 -DETHNETPCI -DM68000 -DDEBUG" \
		"IFLAGS = -I.. -I/usr/EXOS/include  -I/usr/include/sys" \
		"LIBFLAGS = -lsocket"  \
		pci

sgm232 : 
	cd sgm232.dir; \
	$(MAKE) -f ../make.mstr \
		"DFLAGS = -DSYS5 -DRS232PCI -DBINLP -DM68000 -DSTDIO_REDIR" \
		"IFLAGS  = -I.. -I/usr/include/sys" \
		"LIBFLAGS =" \
		rs232

# ICM RS232 version
icm232 : 
	cd icm232.dir; \
	$(MAKE) -f ../make.mstr \
		"DFLAGS = -DGETTY232 -DSYS5 -DRS232PCI -DBINLP -DSTDIO_REDIR" \
		"IFLAGS  = -I.. -I/usr/include/sys" \
		"LIBFLAGS =" \
		rs232

#  Counterpoint System 19K sys5.2 + 4.2 networking version
sys19 : 
	cd sys19.dir; \
	$(MAKE) -f ../make.mstr \
		"DFLAGS = -DSYS19 -DUDP42 -DSYS5 -DETHNETPCI -DM68000 -DDEBUG" \
		"IFLAGS = -I..  -I/usr/include/h -I/usr/include/sys" \
		"LIBFLAGS = -lc"  \
		pci

#  Counterpoint System 19K RS232 version
sys19232 : 
	cd sys19232.dir; \
	$(MAKE) -f ../make.mstr \
		"DFLAGS = -DGETTY232 -DSYS19 -DSYS5 -DRS232PCI -DBINLP -DSTDIO_REDIR" \
		"IFLAGS  = -I.. -I/usr/include/sys" \
		"LIBFLAGS =" \
		rs232
#	NCR TOWER sys 5.2 + 4.1C networking version
ncrtowerrl :
	cd ncrtowerrl.dir; \
	$(MAKE) -f ../make.mstr \
		"DFLAGS = -DXLN_BUG -DUDP41C -DSTDIO_REDIR -DSYS5 -DETHNETPCI \
			  -DM68000 -DRLOCK -DDEBUG -DNCRTOWER" \
		"IFLAGS = -n -I.. -I/appl/EXOS/include/EXOS \
			  -I/usr/include/sys" \
		"LIBFLAGS = -lsocket"  \
		"SVRFLAGS = -DDEBUG -DRLOCK" \
		"INITFLAGS = -DDEBUG -DRLOCK" \
		rltowerpci

ncrtower232 :
	cd ncrtower232.dir; \
	$(MAKE) -f ../make.mstr \
		"DFLAGS = -DSYS5 -DRS232PCI -DBINLP -DRLOCK -DSTDIO_REDIR \
			  -DDEBUG -DM68000 -DNCRTOWER" \
		"IFLAGS  = -I.. -I/usr/include/sys" \
		"SVRFLAGS = -DDEBUG -DRLOCK" \
		"INITFLAGS = -DDEBUG -DRLOCK" \
		"LIBFLAGS = " \
		rltowerrs


# Prime EXL-316 System V.3 with 4.2 Networking  (Record locking version)
# Note -DDEBUG must be left in INITFLAGS for the sharectl program to be
# able to dump the shared memory.
prime : 
	cd prime.dir; \
	$(MAKE) -f ../make.mstr \
		"DFLAGS = -DEXL316 -DEXL316CMC -DSYS5 -DETHNETPCI -DUDP42 -DiAPX386 \
			  -DSTDIO_REDIR -DRD5 -DRLOCK -DBINLP -DUSER64" \
		"IFLAGS  = -I.. -I/usr/include -I/usr/include/sys \
			   -I/usr/include/sys/net -I../../pkg_rlock" \
		"LIBFLAGS = -lnet ../../pkg_rlock/librlock.a ../../pkg_lockset/liblset.a" \
		"RLFLAGS =" \
		"SVRFLAGS = -DRLOCK" \
		"INITFLAGS = -DDEBUG -DRLOCK" \
		rdpci

# System V.3 Prime EXL-316 RS232 version
prime232 : 
	cd prime232.dir; \
	$(MAKE) -f ../make.mstr \
		"DFLAGS = -DEXL316 -DSYS5 -DRS232PCI -DBINLP -DSTDIO_REDIR \
			  -DDEBUG -DRLOCK -DiAPX386 -DRS232_7BIT" \
		"IFLAGS  = -I.. -I/usr/include -I/usr/include/sys -I../../pkg_rlock" \
		"LIBFLAGS = ../../pkg_rlock/librlock.a ../../pkg_lockset/liblset.a" \
		pci232

# Prime EXL-316 System V.3.1+ with 4.2 Networking  (Record locking version)
# Note -DDEBUG must be left in INITFLAGS for the sharectl program to be
# able to dump the shared memory.
# Must compile & link on SVR3.1 or later to get shadow password routines.
prime31 : 
	cd prime31.dir; \
	$(MAKE) -f ../make.mstr \
		"DFLAGS = -DEXL316 -DSYS5 -DETHNETPCI -DUDP42 -DTLI -DiAPX386 \
			  -DSTDIO_REDIR -DRD5 -DRLOCK -DBINLP -DUSER64  \
			  -DSHADOW_PASSWD" \
		"IFLAGS  = -I.. -I/usr/include -I/usr/include/sys \
			   -I/usr/include/sys/net -I../../pkg_rlock" \
		"LIBFLAGS = -lstcp -lnsl_s -lsec ../../pkg_rlock/librlock.a ../../pkg_lockset/liblset.a" \
		"RLFLAGS =" \
		"SVRFLAGS = -DRLOCK" \
		"INITFLAGS = -DDEBUG -DRLOCK" \
		rdpci

# System V.3.1+ Prime EXL-316 RS232 version
# Must compile & link on SVR3.1 or later to get shadow password routines.
prime31_232 : 
	cd prime31_232.dir; \
	$(MAKE) -f ../make.mstr \
		"DFLAGS = -DEXL316 -DSYS5 -DRS232PCI -DBINLP -DSTDIO_REDIR \
			  -DDEBUG -DRLOCK -DiAPX386 -DRS232_7BIT \
			  -DSHADOW_PASSWD" \
		"IFLAGS  = -I.. -I/usr/include -I/usr/include/sys -I../../pkg_rlock" \" \
		"LIBFLAGS =  -lsec ../../rlock/librlock.a ../../lockset/liblset.a" \
		pci232

# Bell Tech System V.3.2 with 4.2 Networking  (Record locking version)
# Note -DDEBUG must be left in INITFLAGS for the sharectl program to be
# able to dump the shared memory.
# Use -DBELLTLI for Bell Tech TLI
belleth: 
	cd belleth.dir; \
	$(MAKE) -f ../make.mstr \
		"DFLAGS = -DSYS5 -DSYS5_3 -DETHNETPCI -DUDP42 -DiAPX386 \
			  -DSTDIO_REDIR -DRD5 -DRLOCK -DBINLP -DUSER64  \
			  -DBSD43 -DSHADOW_PASSWD \
			  -DBROADCAST_OPTION -DTLI -DBELLTLI -DSTREAMNET" \
		"IFLAGS  = -I.. -I/usr/include -I/usr/include/sys \
			   -I/usr/include/sys/net -I../../pkg_rlock" \
		"LIBFLAGS = -lnsl_s -lsec ../../pkg_rlock/librlock.a ../../pkg_lockset/liblset.a -lsocket" \
		"RLFLAGS =" \
		"SVRFLAGS = -DRLOCK" \
		"INITFLAGS = -DRLOCK" \
		rdpci

# System V.3.2 Bell Tech RS232 version
bell232: 
	cd bell232.dir; \
	$(MAKE) -f ../make.mstr \
		"DFLAGS = -DSYS5 -DSYS5_3 -DRS232PCI -DBINLP -DSTDIO_REDIR \
			  -DRLOCK -DiAPX386 \
			  -DSHADOW_PASSWD" \
		"IFLAGS  = -I.. -I/usr/include -I/usr/include/sys -I../../pkg_rlock" \
		"LIBFLAGS =  -lsec ../../pkg_rlock/librlock.a ../../pkg_lockset/liblset.a" \
		pci232

# 3.0 ports - only "loadpci" is ported (in theory).  The servers used are
# generic System V/386
# SCO System 5 version 3.2 with TLI on a 386 machine, ethernet version
scoeth:
	cd scoeth.dir; \
	$(MAKE) -f ../make.mstr \
		"DFLAGS = -DSYS5 -DSYS5_3 -DETHNETPCI -DUDP42 -DiAPX386 -DSTDIO_REDIR -DRD5 -DRLOCK -DBINLP -DUSER64 -DFAST_LSEEK -DTLI -DSTREAMNET -DSecureWare -DBROADCAST_OPTION -DBELLTLI -DDONT_START_GETTY" \
		"IFLAGS  = -I.. -I/usr/include -I/usr/include/sys -I/usr/include/sys/net -I../../pkg_rlock" \
		"LIBFLAGS = -lnsl_s -lprot ../../pkg_rlock/librlock.a ../../pkg_lockset/liblset.a -lsocket -lc_s" \
		"RLFLAGS =" \
		"SVRFLAGS = -DRLOCK" \
		"INITFLAGS = -DRLOCK" \
		rdpci

# SCO System 5 version 3.2 with TLI on a 386 machine, RS232 version
sco232:
	cd sco232.dir; \
	$(MAKE) -f ../make.mstr \
		"DFLAGS = -DSYS5 -DSYS5_3 -DRS232PCI -DBINLP -DSTDIO_REDIR -DFAST_LSEEK -DRS232_7BIT -DRLOCK -DiAPX386 -DSecureWare" \
		"IFLAGS  = -I.. -I/usr/include -I/usr/include/sys -I../../pkg_rlock" \
		"LIBFLAGS =  -lprot ../../pkg_rlock/librlock.a ../../pkg_lockset/liblset.a -lc_s" \
		pci232


# AT&T System 5 version 3.2 on a 386 machine with ethernet MICOM
micometh:
	cd micometh.dir; \
	$(MAKE) -f ../make.mstr \
		"DFLAGS = -DSYS5 -DSYS5_3 -DETHNETPCI -DUDP42 -DiAPX386 \
			  -DSTDIO_REDIR -DRD5 -DRLOCK -DBINLP -DUSER64  \
			  -DSHADOW_PASSWD -DMICOM -Di386\
			  -DSIGNAL_KNOWN -DFAST_LSEEK -DSTREAMNET " \
		"IFLAGS  = -I.. -I/usr/include -I/usr/include/sys \
			   -I/usr/include/sys/net -I../../pkg_rlock" \
		"LIBFLAGS = -lnsl_s -lsec ../../pkg_rlock/librlock.a ../../pkg_lockset/liblset.a -ltcp " \
		"RLFLAGS =" \
		"SVRFLAGS = -DRLOCK" \
		"INITFLAGS = -DRLOCK" \
		rdpci loadpci



# System 5 version 3.2 on a CT 386 machine, ethernet version
ctspceth:
	cd ctspceth.dir; \
	$(MAKE) -f ../make.mstr \
		"DFLAGS = -DSYS5 -DSYS5_3 -DETHNETPCI -DUDP42 -DiAPX386 \
			  -DSTDIO_REDIR -DRD5 -DRLOCK -DBINLP -DUSER64  \
			  -DSHADOW_PASSWD -DCTSPC \
			  -DFAST_LSEEK -DBSD43 -DSTREAMNET" \
		"IFLAGS  = -I.. -I/usr/include -I/usr/include/sys \
			   -I/usr/include/sys/net -I../../pkg_rlock" \
		"LIBFLAGS = -lnsl_s -lsec ../../pkg_rlock/librlock.a ../../pkg_lockset/liblset.a -lsocket -lc_s" \
		"RLFLAGS =" \
		"SVRFLAGS = -DRLOCK" \
		"INITFLAGS = -DRLOCK" \
		rdpci loadpci

# System 5 version 3.2 on a CT 386 machine, RS232 version
ctspc232:
	cd ctspc232.dir; \
	$(MAKE) -f ../make.mstr \
		"DFLAGS = -DSYS5 -DSYS5_3 -DRS232PCI -DBINLP -DSTDIO_REDIR \
			  -DFAST_LSEEK -DRS232_7BIT -DRLOCK -DiAPX386 \
			  -DSHADOW_PASSWD -DCTSPC" \
		"IFLAGS  = -I.. -I/usr/include -I/usr/include/sys -I../../pkg_rlock" \
		"LIBFLAGS =  -lsec ../../pkg_rlock/librlock.a ../../pkg_lockset/liblset.a -lc_s" \
		pci232





# Generic System 5 version 3.2 with TLI on a 386 machine, ethernet version
geneth:
	cd geneth.dir; \
	$(MAKE) -f ../make.mstr \
		"DFLAGS = -DSYS5 -DSYS5_3 -DETHNETPCI -DUDP42 -DiAPX386 \
			  -DSTDIO_REDIR -DRD5 -DRLOCK -DBINLP -DUSER64  \
			  -DSHADOW_PASSWD \
			  -DFAST_LSEEK -DTLI -DSTREAMNET" \
		"IFLAGS  = -I.. -I/usr/include -I/usr/include/sys \
			   -I/usr/include/sys/net -I../../pkg_rlock" \
		"LIBFLAGS = -lnsl_s -lsec ../../pkg_rlock/librlock.a ../../pkg_lockset/liblset.a -lsocket -lc_s" \
		"RLFLAGS =" \
		"SVRFLAGS = -DRLOCK" \
		"INITFLAGS = -DRLOCK" \
		rdpci

# Generic System 5 version 3.2 with TLI on a 386 machine, RS232 version
gen232:
	cd gen232.dir; \
	$(MAKE) -f ../make.mstr \
		"DFLAGS = -DSYS5 -DSYS5_3 -DRS232PCI -DBINLP -DSTDIO_REDIR \
			  -DFAST_LSEEK -DRS232_7BIT -DRLOCK -DiAPX386 \
			  -DSIGNAL_KNOWN -DSHADOW_PASSWD" \
		"IFLAGS  = -I.. -I/usr/include -I/usr/include/sys -I../../pkg_rlock" \
		"LIBFLAGS =  -lsec ../../pkg_rlock/librlock.a ../../pkg_lockset/liblset.a -lc_s" \
		pci232


# Generic System 5 version 3.2 with TLI on a 3B2 400 machine, ethernet version
3b2eth:
	cd 3b2eth.dir; \
	$(MAKE) -f ../make.mstr \
		"DFLAGS = -DSYS5 -DSYS5_3 -DATT3B2 -DETHNETPCI -DUDP42 \
			  -DSTDIO_REDIR -DRD5 -DRLOCK -DBINLP -DUSER64  \
			  -DFAST_LSEEK -DTLI -DSTREAMNET -DSHADOW_PASSWD" \
		"IFLAGS  = -I.. -I/usr/include -I/usr/include/sys \
			   -I/usr/include/sys/net -I../../pkg_rlock" \
		"LIBFLAGS = -lnet -lnsl_s ../../pkg_rlock/librlock.a ../../pkg_lockset/liblset.a " \
		"RLFLAGS =" \
		"SVRFLAGS = -DRLOCK" \
		"INITFLAGS = -DRLOCK" \
		rdpci loadpci

# Generic System 5 version 3.2 with TLI on a 3B2 400 machine, RS232 version
3b2232:
	cd 3b2232.dir; \
	$(MAKE) -f ../make.mstr \
		"DFLAGS = -DSYS5 -DSYS5_3 -DATT3B2 -DRS232PCI -DBINLP \
			  -DSTDIO_REDIR -DFAST_LSEEK -DRS232_7BIT -DRLOCK \
			  -DSHADOW_PASSWD" \
		"IFLAGS  = -I.. -I/usr/include -I/usr/include/sys -I../../pkg_rlock" \
		"LIBFLAGS =  -lnet -lnsl_s ../../pkg_rlock/librlock.a ../../pkg_lockset/liblset.a " \
		pci232


# Generic System 5 version 4.0 with TLI on a 386 machine, ethernet version
svr4eth:
	[ -d $@.dir ] || mkdir $@.dir
	cd svr4eth.dir; \
	$(MAKE) -f ../make.mstr \
		"DFLAGS = -DSYS5 -DSYS5_4 -DETHNETPCI -DUDP42 -DiAPX386 \
			  -DSTDIO_REDIR -DRD5 -DRLOCK -DBINLP -DUSER64  \
			  -DSHADOW_PASSWD -D_STYPES -DFAST_LSEEK	\
			  -DSIGNAL_KNOWN -DBSD43 -DSTREAMNET" \
		"IFLAGS  = -I.. -I$(ROOT)/usr/include \
			   -I$(ROOT)/usr/include/sys/net -I../../pkg_rlock" \
		"LIBFLAGS = -lnsl ../../pkg_rlock/librlock.a ../../pkg_lockset/liblset.a -lsocket" \
		"RLFLAGS =" \
		"SVRFLAGS = -DRLOCK" \
		"INITFLAGS = -DRLOCK" \
		rdpci

# Generic System 5 version 4.0 with TLI on a 386 machine, RS232 version
svr4232:
	[ -d $@.dir ] || mkdir $@.dir
	cd svr4232.dir; \
	$(MAKE) -f ../make.mstr \
		"DFLAGS = -DSYS5 -DSYS5_4 -DRS232PCI -DBINLP -DSTDIO_REDIR \
			  -DFAST_LSEEK -DRS232_7BIT -DRLOCK -DiAPX386 \
			  -DSHADOW_PASSWD -D_STYPES -DSIGNAL_KNOWN" \
		"IFLAGS  = -I.. -I$(ROOT)/usr/include -I../../pkg_rlock" \
		"LIBFLAGS =  ../../pkg_rlock/librlock.a ../../pkg_lockset/liblset.a" \
		pci232


# PCI 3.0 loadpci for AT&T System V Release 4.0
svr430:
	[ -d loadpci.dir ] || mkdir loadpci.dir
	cd loadpci.dir; \
	$(MAKE) -f ../make.mstr \
		"DFLAGS = -DSYS5 -DSYS5_4 -DETHNETPCI -DUDP42 -DiAPX386   \
			  -DSTDIO_REDIR -DRD5 -DRLOCK -DBINLP -DUSER64    \
			  -DBROADCAST_OPTION -D_STYPES -DBSD43 -DSTREAMNET" \
		"IFLAGS  = -I.. -I$(ROOT)/usr/include -I$(ROOT)/usr/include/sys/net " \
		"LIBFLAGS = -lsocket -lnsl" \
		"RLFLAGS =" \
		"SVRFLAGS = " \
		"INITFLAGS = " \
		loadpci; \
		mv loadpci loadpci.svr4



# loadpci for SCO
sco30:
	cd loadpci.dir; \
	$(MAKE) -f ../make.mstr \
		"DFLAGS = -DSYS5 -DSYS5_3 -DETHNETPCI -DUDP42 -DiAPX386 -DSTDIO_REDIR -DRD5 -DRLOCK -DBINLP -DUSER64  -DTLI -DSTREAMNET -DBROADCAST_OPTION -DBELLTLI" \
		"IFLAGS  = -I.. -I/usr/include -I/usr/include/sys -I/usr/include/sys/net " \
		"LIBFLAGS = -lsocket -lnsl_s -lc_s " \
		"RLFLAGS =" \
		"SVRFLAGS = " \
		"INITFLAGS = " \
		loadpci; \
		mv loadpci loadpci.sco


# CT S SERIES , ethernet
ctseth:
	cd ctseth.dir; \
	$(MAKE) -f ../make.mstr \
		"DFLAGS = -DSYS5 -DSYS5_3 -DETHNETPCI -DUDP42 -DM68000 \
			  -DSTDIO_REDIR -DRD5 -DRLOCK -DBINLP -DUSER64  \
			  -DFAST_LSEEK -DBSD43" \
		"IFLAGS  = -I.. -I/usr/include -I/usr/include/sys \
			   -I/usr/include/sys/net -I../../pkg_rlock" \
		"LIBFLAGS = -lnsl_s ../../pkg_rlock/librlock.a ../../pkg_lockset/liblset.a -lsocket -lc_s" \
		"RLFLAGS =" \
		"SVRFLAGS = -DRLOCK" \
		"INITFLAGS = -DRLOCK" \
		rdpci loadpci

# CT S SERIES, rs232
cts232:
	cd cts232.dir; \
	$(MAKE) -f ../make.mstr \
		"DFLAGS = -DSYS5 -DSYS5_3 -DRS232PCI -DBINLP -DSTDIO_REDIR \
			  -DFAST_LSEEK -DRS232_7BIT -DRLOCK -DM68000" \
		"IFLAGS  = -I.. -I/usr/include -I/usr/include/sys -I../../pkg_rlock" \
		"LIBFLAGS =  ../../pkg_rlock/librlock.a ../../pkg_lockset/liblset.a -lc_s" \
		pci232


# loadpci for Bell Tech 
belltech30:
	cd loadpci.dir; \
	$(MAKE) -f ../make.mstr \
		"DFLAGS = -DSYS5 -DSYS5_3 -DETHNETPCI -DUDP42 -DiAPX386 \
			  -DSTDIO_REDIR -DRD5 -DRLOCK -DBINLP -DUSER64  \
			  -DBSD43 -DBROADCAST_OPTION -DBELLTLI \
			  -DTLI -DSTREAMNET" \
		"IFLAGS  = -I.. -I/usr/include -I/usr/include/sys \
			   -I/usr/include/sys/net " \
		"LIBFLAGS = -lsocket -lnsl_s -lc_s " \
		"RLFLAGS =" \
		"SVRFLAGS = " \
		"INITFLAGS = " \
		loadpci; \
		mv loadpci loadpci.bt

# loadpci for Prime
prime30:
	cd loadpci.dir; \
	$(MAKE) -f ../make.mstr \
		"DFLAGS = -DSYS5 -DSYS5_3 -DETHNETPCI -DUDP42 -DiAPX386 \
			  -DSTDIO_REDIR -DRD5 -DRLOCK -DBINLP -DUSER64  \
			  -DEXL316  \
			  -DTLI -DSTREAMNET" \
		"IFLAGS  = -I.. -I/usr/include -I/usr/include/sys \
			   -I/usr/include/sys/net " \
		"LIBFLAGS = -lstcp -lnsl_s -lc_s " \
		"RLFLAGS =" \
		"SVRFLAGS = " \
		"INITFLAGS = " \
		loadpci; \
		mv loadpci loadpci.prime

# DECstation 3100 running Ultrix 3.0 Ethernet
deceth: 
	cd deceth.dir; \
	make -f ../make.mstr \
		"DFLAGS = -DBSD43 -DETHNETPCI -DDOPRNT41 -DBERKELEY42 \
			  -DPARAM_GETS_TYPES -DSTDIO_REDIR -DBERK42FILE \
			  -DUSER64 -DSIGNAL_KNOWN -DRD5 -DUDP42 -DRLOCK -DULTRIX" \
		"PCFLAGS = " \
		"IFLAGS = -I.. \
			  -I/usr/include \
			  -I/usr/include/sys \
			  -I../../pkg_rlock \
			  -I/usr/include/mips" \
		"LIBFLAGS = ../../pkg_rlock/librlock.a \
			../../pkg_lockset/liblset.a" \
		rdpci loadpci 

# DECstation 3100 running Ultrix 3.0 RS232
dec232: 
	cd dec232.dir; \
	make -f ../make.mstr \
		"DFLAGS = -DBSD43 -DBERKELEY42 -DRS232PCI -DDOPRNT41 \
			  -DPARAM_GETS_TYPES -DSTDIO_REDIR -DBERK42FILE \
			  -DUSER64 -DSIGNAL_KNOWN -DRLOCK -DULTRIX" \
		"IFLAGS = -I..\
			-I/usr/include \
			-I/usr/include/sys \
			-I../../pkg_rlock \
			 -I/usr/include/mips" \
		"PCFLAGS = " \
		"LIBFLAGS =  ../../pkg_rlock/librlock.a\
			 ../../pkg_lockset/liblset.a" \
		pci232


# Data General Aviion
dgeth:
	cd dgeth.dir; \
	$(MAKE) -f ../make.mstr \
		"DFLAGS = -DSYS5 -DSYS5_3 -DETHNETPCI -DUDP42 \
			  -DSTDIO_REDIR -DRD5 -DRLOCK -DBINLP -DUSER64  \
			  -DFAST_LSEEK -DBSD43 -DSIGNAL_KNOWN -DBERK42FILE -DDEBUG" \
		"IFLAGS  = -I.. -I/usr/include -I/usr/include/sys \
			   -I/usr/include/sys/net -I../../pkg_rlock" \
		"LIBFLAGS = ../../pkg_rlock/librlock.a ../../pkg_lockset/liblset.a" \
		"RLFLAGS =" \
		"SVRFLAGS = -DRLOCK" \
		"INITFLAGS = -DRLOCK" \
		rdpci loadpci

dg232:
	cd dg232.dir; \
	$(MAKE) -f ../make.mstr \
		"DFLAGS = -DSYS5 -DSYS5_3 -DRS232PCI -DBINLP -DSTDIO_REDIR \
			  -DFAST_LSEEK -DRLOCK -DSIGNAL_KNOWN -DBERK42FILE -DDEBUG \
				-DBSD43 -DRD5" \
		"IFLAGS  = -I.. -I/usr/include -I/usr/include/sys -I../../pkg_rlock" \
		"LIBFLAGS =  ../../pkg_rlock/librlock.a ../../pkg_lockset/liblset.a" \
		pci232
