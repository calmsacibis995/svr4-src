/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pcintf:bridge/pci_types.h	1.1"
/* SCCSID(@(#)pci_types.h	3.36	LCC);	/* Modified: 16:32:35 2/26/90 */

/*****************************************************************************


	Copyright (c) 1984 Locus Computing Corporation.
	All rights reserved.
	This is an unpublished work containing CONFIDENTIAL INFORMATION
	that is the property of Locus Computing Corporation.
	Any unauthorized use, duplication or disclosure is prohibited.

*****************************************************************************/

#ifndef PCI_TYPES_H
#define PCI_TYPES_H

/* why is this used?  why isn't it MAX_OUTPUT?  It's only used in p_read.c. */
/* -- hjb 04/25/85 */
#ifdef TCA
#define MAX_DATA	4096
#else
#define MAX_DATA	1024
#endif

#include	<log.h>

#ifndef	PARAM_GETS_TYPES
#	ifdef MICOM
#		include <interlan/il_types.h>
#	else
#		include	<sys/types.h>
#	endif
#endif	/* ~PARAM_GETS_TYPES */
#include	<sys/param.h>

#if defined(OLD_XENIX)
typedef int size_t;	/* recent Sys V releases have this in <sys/types.h> */
#endif	/* OLD_XENIX */

#if	!defined(BERKELEY42) && !defined(ICM3216) && !defined(ATT3B2)
#define	u_char	unsigned char
#define u_short unsigned short
#endif	/* !BERKELEY42 && !ICM3216 */

#ifdef ICM3216
#include	<inet.h>
#endif	/* ICM3216 */

#if defined(RIDGE) || defined(SYS19)
#define u_long  unsigned long
#endif  /* RIDGE */

#ifdef EXL316
#define uchar  unsigned char
#endif  /* EXL316 */

#include <stdio.h>

#ifndef CONST_H
#include "const.h"
#endif  /* CONST_H */

#if defined(UDP42) || defined(UDP41C)
#    if defined(SYS19) || defined(EXL316CMC)
#        include <socket.h>
#    else
#    	ifdef MICOM
#			include <interlan/socket.h>
#    	else
#      		include <sys/socket.h>
#		endif	/* MICOM */
#    endif	/* SYS19 || EXL316CMC */
#    ifdef TLI
#        include <sys/stream.h>
#        include <sys/tiuser.h>
#        include <sys/tihdr.h>  
#    endif /* TLI */
#    if	defined(CTIX) || defined(EXL316CMC)
#        include	<in.h>
#        include	<netdb.h>
#    else
#ifdef ICM3216
#        include <in.h>
#else
#	ifdef MICOM
#		include <interlan/in.h>
#	else
#		ifndef ATT3B2
#			include <netinet/in.h>
#		else
#			include <sys/in.h>
#		endif	/* ATT3b2 */
#	endif /* MICOM */
#endif	/* ICM3216 */

#if defined(EXCELAN)
#	include <sys/soioctl.h>

/* Excelan LAN WorkPlace XENIX/386 doesn't include netdb.h routines */

struct hostent {		/* only a few fields are needed:	*/
	char	*h_name;	/* 	official host name		*/
	int	 h_length;	/* 	length of host address		*/
	char	*h_addr;	/* 	host address			*/
};

#elif !defined(BELLTECH)
#	ifdef MICOM
#		include <interlan/netdb.h>
#	else
#		include <netdb.h>
#	endif	/* MICOM */
#endif	/* EXCELAN */
#    endif	/* CTIX || EXL316CMC */
#    ifdef UDP42
#       ifdef EXL316CMC
#           include <inet.h>
#       else
#	    if !defined(MICOM) && !defined(EXCELAN)
#		    include <arpa/inet.h>
#	    endif	/* !MICOM && !EXCELAN */
#       endif	/* EXL316CMC */
#        ifdef	BSD43
#            include <net/if.h>
#        endif	/* BSD43 */
#    endif /* UDP42 */

#    define SZNADDR	16	/* size of an internet address */
#    define SZHADDR	4	/* size of an internet host address */
#    define SZHNAME	16	/* Size of site name in nameAddr */
			/* SZHADDR + SZHNAME == 20 ... Or Else! */
#else	/* !UDP42 && !UDP41C */
#    define	SZNADDR	6	/* Size of an ethernet address */
#    define	SZHADDR	6	/* Size of an ethernet host address */
#    define	SZHNAME	14	/* Size of site name in nameAddr */
			/* SZHADDR + SZHNAME == 20 ... Or Else! */
#endif  /* UDP42 || UDP41C */

/*
   Flags to logPacket telling it what to print
*/
#define	LOGNETHEADER	0x0001L
#define	LOGNHHEAD	0x0002L
#define	LOGPREHEADER	0x0004L
#define	LOGHEADER	0x0008L
#define	LOGASCII	0x0010L
#define	LOGDBYTES	0x0020L
#define	LOGHBYTES	0x0040L
#define	LOGDSHORTS	0x0080L
#define	LOGHSHORTS	0x0100L
#define	LOGDLONGS	0x0200L
#define	LOGHLONGS	0x0400L
#define	LOGQUIET	0x8000L

#define LOG_NET         (LOGNETHEADER|LOGHEADER)
#define LOG_SOME        (LOGHEADER|LOGASCII)
#define	LOG_HDRS	(LOGNETHEADER|LOGNHHEAD|LOGPREHEADER|LOGHEADER)
#define	LOG_ALL		(LOGNETHEADER|LOGHEADER|LOGPREHEADER|LOGNHHEAD|LOGASCII)

#define	is_dot(a)	((strcmp(a, ".")) == 0)
#define	is_dot_dot(a)	((strcmp(a, "..")) == 0)

#ifdef HIDDEN_FILES
#define IS_HIDDEN(fileP) (*(fileP) == '.' && *(fileP+1) != '\0' && strcmp(fileP,".."))
#define UNHIDE(fileP)	(++fileP)
#endif /* HIDDEN_FILES */

/* this serial-number stuff was previously under ifdef ETHNETPCI */
/* it is now used in point-to-point copy protection */
#define SERIALSIZE  16 
#define	SERIALSTR	((2*SERIALSIZE)+1)
/*
   serialEq: True if both serial numbers are equal
*/
#define	serialEq(ser1, ser2)		(memcmp(ser1, ser2, SERIALSIZE) == 0)

/*
   serialCpy: Copy serSrc to serDst
*/
#define	serialCpy(serDst, serSrc)	memcpy(serDst, serSrc, SERIALSIZE)

/*
   serialClr: Zero out a serial number
*/
#define	serialClr(addr)			memset(addr, 0, SERIALSIZE)

#define RD_SEQ_MAX   65535           /* RD: Max sequence number b4 wraparound*/
#define TCB_UK        255            /* RD: "tcbn uknown" flag               */
#define RD_VERSION      1            /* RD: current version of rel. del.     */
#define RD_ACK         10            /* RD: reliable delivery emulation ack  */
#define RD_DATA        20            /* RD: reliable delivery emulation data */
#define RD_PB          30            /* RD: reliable delivery piggyback a/d  */
#define RD_INITTERM    40            /* RD: initialize termout               */

/*---------------------------------------------- E T H E R N E T -----------*/
#ifdef ETHNETPCI
#define MAX_INPUT_PCI	(1024+ETH_IFUDGE)	/* the +ETH_FUDGEI is because of 3b5 */
#define	MAX_OUTPUT	(1024+ETH_OFUDGE)	/* the +ETH_FUDGEO is because of 3b5 */

#if defined(UDP42) || defined(UDP41C)
#define	NI2SIZE		0
#else	
#define	NI2SIZE		(sizeof(struct ni2))
#endif  /* UDP42 || UDP41C */

#define	HEADER		NI2SIZE + sizeof(struct preheader) + \
				sizeof(struct header)

/*
   inAddrEq: True if both network addresses are equal
*/
#define	inAddrEq(addr1, addr2)		(memcmp(addr1, addr2, SZHADDR) == 0)


/*
   inAddrCpy: Copy addrSrc to addrDst
*/
#define	inAddrCpy(addrDst, addrSrc)	memcpy(addrDst, addrSrc, SZHADDR)


/*
   inAddrClr: Zero out a network address
*/
#define	inAddrClr(addr)			memset(addr, 0, SZHADDR)

/*
   nAddrEq: True if both network addresses are equal
*/
#define	nAddrEq(addr1, addr2)		(memcmp((char *)(addr1)+4, (char *)(addr2)+4, SZHADDR) == 0)


/*
   nAddrCpy: Copy addrSrc to addrDst
*/
#define	nAddrCpy(addrDst, addrSrc)	memcpy(addrDst, addrSrc, SZNADDR)


/*
   nAddrClr: Zero out a network address
*/
#define	nAddrClr(addr)			memset(addr, 0, SZNADDR)



#if defined(UDP42) || defined(UDP41C)

typedef struct  ni2u {
	union {
		char ni2uc[SZNADDR];
		struct sockaddr_in ni2us;
	} ni2un;
} ni2u;

#else

typedef unsigned char	ni2u[SZNADDR];

#endif  /* UDP42 || UDP41C */

struct ni2 {
	ni2u ni2dstu;
	ni2u ni2srcu;
	unsigned char 	type[2];
#if !defined(UDP42) && !defined(UDP41C)
	char 	head[SZNADDR]; 	/* This is used in 1 place! */
#endif  /* !UDP42 && !UDP41C */
};

#if defined(UDP42) || defined(UDP41C)
#define	dst	ni2dstu.ni2un.ni2uc
#define	src	ni2srcu.ni2un.ni2uc
#define	dst_sin	ni2dstu.ni2un.ni2us
#define	src_sin	ni2srcu.ni2un.ni2us
#else
#define	dst	ni2dstu
#define	src	ni2srcu
#endif  /* UDP42 || UDP41C */


/*
   Flags to netOpen() telling what to do with the last 3 bytes
   of the virtual port address.  NO_PHYS says to use the last 3
   bytes of the local physical address and NO_XERO says to zero
   the last three bytes.  NO_MASK gives the pure port number
   from the portNum argument of netOpen().
   The port opened will be the one asked for unless it is zero, in which
   case the system will choose the port.
*/
#if defined(UDP42) || defined(UDP41C)
#define	PCI_MAPSVR_PORT		125
#define	PCI_CONSVR_PORT		127
#else
#define	PCIPORTOFF	5
#endif  /* UDP42 || UDP41C */

#define MAPPORT_3BNET		0
#ifdef	ETH3BNET
#define	NO_ZERO			0x80
#define	NO_PMASK		0x7f
#define	PCI_CONSVR_PORT		0
#define	PCI_MAPSVR_PORT		(0 | NO_ZERO)
#endif	/* ETH3BNET */
#ifdef	ETHLOCUS
#define	PCI_CONSVR_PORT		5
#define	PCI_MAPSVR_PORT		6
#endif	/* ETHLOCUS */

#endif /* ETHNETPCI */
/*---------------------------------------------- R S 2 3 2 -----------------*/
#ifdef RS232PCI

#define MAX_INPUT_PCI	256
#define	MAX_OUTPUT	1024
#define	HEADER		sizeof(struct rs232) + sizeof(struct preheader) + \
				sizeof(struct header)



struct rs232 {
	unsigned char	syn;		/* Start of frame field */
	unsigned char	null;		/* NULL character field	*/
	unsigned short 	chks;		/* Beginging with f_cnt */
	unsigned short	f_cnt;		/* Byte count for frame */
	char		pad[14];	/* Pad to fill */
};

struct ni2 { int notreferenced; }; /* This structure must be be defined. */
				   /* It is used as a dummy in rs232 case */
#define	dst	notreferenced
#define	src	notreferenced

#endif /* RS232PCI */
#ifdef TCA

#define MAX_INPUT_PCI	4096
#define	MAX_OUTPUT	4096
#define	HEADER		sizeof(struct tca) + sizeof(struct preheader) + \
				sizeof(struct header)

struct tca {
	char		pad[20];	/* Pad to fill */
};

struct ni2 { int notreferenced; }; /* This structure must be be defined. */
				   /* It is used as a dummy in TCA case */
#define	dst	notreferenced
#define	src	notreferenced

#endif /* TCA */
/*--------------------------------J A N U S * S U N A J --------------------*/
#ifdef JANUS

/*#define MAX_INPUT_PCI       (MEMBUFSIZE - HEADER)   /* */
/*#define MAX_OUTPUT      (MEMBUFSIZE - HEADER)   /* */
#define MAX_INPUT_PCI       1024   /* */
#define MAX_OUTPUT      1024   /* */
#define HEADER          (sizeof(struct ni2) + sizeof(struct preheader) + \
				sizeof(struct header))

/* This structure must be be defined. The packets exchanged over the */
/* low memory buffer, have room reserved at the front for the ethernet */
/* header. This ethernet header is not used, except to take up space */
/* Therefore this structure must be defined */
struct ni2 {
	unsigned char	dst[SZNADDR];			/* First 3 fields are */
	unsigned char 	src[SZNADDR];			/* standard ethernet */
	unsigned char 	type[2];			/* header. */
	unsigned char	head[SZNADDR];		/* This is used in 1 place! */
};
#endif /* JANUS */

/*--------------------------------------------------------------------------*/
/*			Packet Preheader				*/


struct preheader {
	unsigned char	select;		/* Demultiplexor */
	unsigned char	reset;		/* Resets sequence numbers */
	char		padding[2];
};


/*			Protocol Header Structure		    */


struct header {
	unsigned char	stat;		/* Transmission code field  */
	unsigned char	seq;		/* Frame sequence number    */
	unsigned char	req;		/* Frame REQUEST field      */
	unsigned char	res;		/* Frame RESPONSE field     */
	unsigned short	fdsc;		/* UNIX file descriptor     */
	unsigned short	b_cnt;		/* Byte count in request    */
	unsigned short	t_cnt;		/* Byte count of text field */
	unsigned short	mode;		/* R/W mode for file access */
					/* Also share access modes
					   for record locking       */
	unsigned short	date;		/* Date of file creation    */
	unsigned short	time;		/* Time of file creation    */
	unsigned short	pid;		/* Process id associated w/call */
	unsigned short	inode;		/* Inode of file for consistency */
	long		f_size;		/* File size 		    */
	long		offset;		/* Byte offset into file    */
	long		pattern;	/* Internal byte ordering   */
	unsigned char	attr;		/* Search attributes	    */
	unsigned char	drvNum;		/* Drive number */
	unsigned short	versNum;	/* Version number to consvr */
	char		padding[4];	/* Header padding	    */
};

/*
 * Reliable delivery structures
 */

struct emhead {                    /* reliable delivery header in text area  */
        unsigned short  dnum;      /* sequence number of data                */
	unsigned short  anum;      /* sequence number of ack                 */
	unsigned char 	tcbn;      /* transmission control block num from pc */
	unsigned char   version;   /* version of reliable delivery           */
	unsigned char   code;      /* command, RD_ACK or RD_DATA             */
	unsigned char   options;   /* option flags, undefined at date        */
        unsigned short  strsiz;    /* stream size to give to termout         */
        unsigned short  dummy;     /* dummy word 	         	     */
};

struct  ackcontrol {               /* dos server to termout control         */
	unsigned char   tcbn;      /* task control bolock number of message */
	unsigned char   code;      /* command: RD_ACK of RD_INITTERM        */
	unsigned short  num;       /* sequence number of ack                */
	unsigned short  ssiz;      /* requested maximum stream size         */
	unsigned short  dum;       /* dummy for 3b2 sizeof bug		    */
};

struct rd_shared_mem {		   /* shared memory segment                 */
	unsigned short	FrameExpected;	/* Frame number expected            */
	unsigned short	kick_ack;	/* dtermines whether we ack or not  */
};


/*			Overall Packet Structure		    	*/

/*
   Input packet format.
*/
struct input {
#ifdef ETHNETPCI
	struct ni2		net;		/* Ethernet header	*/
#endif /* ETHNETPCI */
#ifdef RS232PCI
	struct rs232		rs232;		/* RS-232 header	*/
#endif /* RS232PCI */
#ifdef JANUS
	struct ni2              netnotused;     /* Ethernet header(empty) */
#endif /* JANUS */
#ifdef	TCA
	struct tca		netnotused;	/* Placeholder */
#endif	/* TCA */
	struct preheader	pre;		/* Pre-header		*/
	struct header 		hdr;		/* Header 		*/
	char			text[MAX_INPUT_PCI];/* Text 		*/
};


/*
   Output packet format.
*/
struct output {
#ifdef ETHNETPCI
	struct ni2		net;		/* Ethernet header	*/
#endif /* ETHNETPCI */
#ifdef RS232PCI
	struct rs232		rs232;		/* RS-232 header	*/
#endif /* RS232PCI */
#ifdef JANUS
	struct ni2              netnotused;     /* Ethernet header(empty) */
#endif /* JANUS */
#ifdef	TCA
	struct tca		netnotused;	/* Placeholder */
#endif	/* TCA */
	struct preheader	pre;		/* Pre-header		*/
	struct header 		hdr;		/* Header 		*/
	char			text[MAX_OUTPUT];/* Text 		*/
#if defined(RS232PCI) && defined(RS232_7BIT)
	char			pad[(MAX_OUTPUT+HEADER+6)/7 + 1];
#endif
};


/*
 * Search First/Search Next output packet.
 */
#ifndef	XENIX	/* to avoid run_time problems with assigns */

struct sio {
#ifdef ETHNETPCI
	struct ni2		net;		/* Ethernet header	*/
#endif /* ETHNETPCI */
#ifdef RS232PCI
	struct rs232		rs232;		/* RS-232 header	*/
#endif /* RS232PCI */
#ifdef JANUS
	struct ni2              netnotused;     /* Ethernet header(empty) */
#endif /* JANUS */
#ifdef	TCA
	struct tca		netnotused;	/* Placeholder */
#endif	/* TCA */
	struct preheader	pre;		/* Pre-header		*/
	struct header 		hdr;		/* Header 		*/
	char			text[MAX_PATH];	/* Text 		*/
#if defined(RS232PCI) && defined(RS232_7BIT)
	char			pad[(MAX_PATH+HEADER+6)/7 + 1];
#endif
};
#endif	/* !XENIX */


#ifdef	LOCUS
/*
   Temporary LOCUS site table structure.
*/
struct	nconfig {
	char	n_uname[8];		/* Symbolic name of host */
	char	n_phynid[8];		/* Physical network ID */
};
#endif  /* LOCUS */


/* 
 * nameAddr: A host name/address pair
 */

struct	nameAddr {
	char	name[SZHNAME];		/* Symbolic host name */
	char	address[SZHADDR];	/* Host's network address */
};



/*
   struct pcNex: Connection table entry.  One is used for
			each currently active server/PC pair.
*/

struct pcNex {
	char	pcAddr[SZNADDR];		/* Destination address of connection */
	char	pcAddr2[SZNADDR];		/* Source of copy violator           */	
	char	pcSerial[16];		/* serial number of pc software	     */
	short	probeCount;		/* Set when daemon receives PROBE msg */
	int	svrPid;			/* Process id of PC Interface server */
	int	emPid;			/* Process id of Emulation login */
	short	indict;			/* Set when there may be a violation */
	char	emPty[20];		/* Name of login pty */
};

/*
 *  Structure of text area on connect request.
 */

struct	connect_text {
	char	serial_num[16];		/* encrypted form of serial num */
#ifdef	VERSION_MATCHING	/* EVE - 7/2/87 */
	short	vers_major;		/* major version number */
	short	vers_minor;		/* minor version number */
	short	vers_submin;		/* sub-minor version number */
#endif /* VERSON_MATCHING */
};

/*		End of Network Stuff		*/

#ifdef  RLOCK      /* record locking */
typedef unsigned short	word;

#define ENOSHARE  (-6)		/* Used in error.c and ofiletops.c */

/*
   Record locking values of vFile.flags

   CAUTION:
	   !!These flag bits share vFile.flags with the VF_ bits below!!
*/
#define FF_RDONLY	0		/* Read only open */
#define FF_WRONLY	1		/* Write only open */
#define FF_RDWR		2		/* Read/write open */
#define FF_SPOOL	0x0008		/* Append mode file (spool file) */
					/* Trick: same as O_APPEND! */
#define FF_REOPEN	(03 | FF_SPOOL) /* Use these bits during re-opens */
#define FF_READ		0x0800		/* Fid is readable */
#define FF_WRITE	0x0400		/* Fid is writable */
#define FF_NOLOCK	0x0200		/* Record locking not allowed */
#define FF_INUSE	0x8000		/* Table entry is in use */

/*
    A pre-cast NIL pointer
*/
#define NIL_FIDINFO	((struct vFile *) 0)

#endif  /* RLOCK */

/*
 * Virtual file cache structure.
 */

struct vFile {
	short	flags,			/* Cache entry flags - see beow */
		uDesc,			/* Actual UNIX descriptor */
		dosPid;			/* Process id of calling program */
	ino_t 	iNum;			/* Inode number of file */
	char	*pathName;		/* Full path name of file */
	long	lastUse,		/* Time stamp (used in vfile only) */
		vdostime,		/* emulates the dos time stamp */
		rwPtr;			/* Location of read/write pointer */
#ifdef  RLOCK  /* if record locking */
	short	shareIndex;		/* index to global open file table */
	short	shareMode;		/* DOS file-sharing open mode */
	long    uniqueID;               /* unique file id */
	word    sessID;			/* dossvr pid */
	int	lockCount;		/* count of locks on this file */
#endif  /* RLOCK */
};

/*
   values for vFile.flags:

   CAUTION:
	   !!These flag bits share vFile.flags with the FF_ bits above!!
*/
#define	VF_OMODE	0x0003		/* Unix open modes are kept here */
#define	VF_CERTAIN	0x0004		/* Don't upgrade open mode */
#define	VF_UNLINK	0x0008		/* Temp file, unlink on process exit */
#define	VF_DIRTY	0x0010		/* Contents of file were changed */
#define	VF_INACTV	0x0020		/* Cache entry is inactive */
#define	VF_PRESET	0x0040		/* Never inactivate this cache entry */
#define	VF_EPHEM	0x0080		/* Entry is discardable */
#define	VF_STKYFCB	0x0100		/* Non-abandonable FCB entry */
#define	VF_INUSE	0x8000		/* Cache entry is in use */

#define	VF_INIT		0x0000		/* Initial cache entry flag values */

/*
 * Multiple directory search context structure.
 */

struct dircontext {
	char	*pathname;		/* Pathname of directory under search */
#ifndef	XENIX 	
	struct sio	*buf_ptr;	/* Pointer to read-ahead buffer */
#else
	struct output	*buf_ptr;	/* Pointer to read-ahead buffer */
#endif	/* XENIX */
	long	rdwr_ptr;		/* Current pointer into directory */
	int		attr;			/* MS-DOS search attribute */
	int		mode;			/* DOS search mode (MAPPPED/UNMAPPED) */
	int		pid;			/* Process id of calling program */
	char	pattern[MAX_PATTERN];	/* MS-DOS file name/search pattern */
};

extern char
	*fnQualify();

#ifdef	MULT_DRIVE

extern char
	wDirs[NDRIVE][MAX_CWD];

#define	CurDir	wDirs[drvNum]

#else	/* ~MULT_DRIVE */

extern	char
	cwd[];			/* Contains current working directory string */

#define	CurDir	cwd

#endif	/* MULT_DRIVE */

/**** Structure to hold tempfile listed in wrong directories ****/
struct temp_slot
{
	ino_t	s_ino;
	int	s_dev;
	int	s_flags;
	char	fname[10];
};

#define	PROBE_TIMEOUT	5	/* Max missed probes before disconnect */


#ifdef UDP42
/* This structure is used to define the local and broadcast address of */
/* each interface for which the host is configured		       */


typedef struct netIntFace {
	struct in_addr	localAddr;
	struct in_addr	broadAddr;
	struct in_addr	subnetMask;
} netIntFace;

#define	MAX_NET_INTFACE	10	/* Max # of configured interfaces */

#endif /* UDP42 */

#endif /* !PCI_TYPES_H */
