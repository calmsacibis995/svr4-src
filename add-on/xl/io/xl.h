/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)xl:io/xl.h	1.3"

/*	Copyright (c) 1989 Intel Corp.		*/
/*	  All Rights Reserved  	*/
/*
 *	INTEL CORPORATION PROPRIETARY INFORMATION
 *
 *	This software is supplied under the terms of a license
 *	agreement or nondisclosure agreement with Intel Corpo-
 *	ration and may not be copied or disclosed except in
 *	accordance with the terms of that agreement.
 */

/************************************************************************/
/*	Copyright (c) 1988, 1989 ARCHIVE Corporation			*/
/*	This program is an unpublished work fully protected by the	*/
/*	United States Copyright laws and is considered a trade secret	*/
/*	belonging to Archive Corporation.				*/
/************************************************************************/
/*	file: xl.h							*/
/*
/************************************************************************/
/*	  QIC 117 commands						*/
/************************************************************************/
#define	QIC117_RST	01	/* reset				*/
#define	QIC117_RNB	02	/* report next bit			*/
#define	QIC117_PAUS	03	/* pause				*/
#define	QIC117_STS	06	/* status				*/
#define	QIC117_ECD	07	/* report error code			*/
#define	QIC117_FWD	10	/* logical forward			*/
#define	QIC117_BOT	11	/* rewind to begining of tape		*/
#define	QIC117_EOT	12	/* forward to end of tape		*/
#define	QIC117_SEEK	13	/* seek head to track			*/
#define	QIC117_CAL	14	/* rewind, calibrate drive		*/
#define	QIC117_FMD	15	/* format mode				*/
#define	QIC117_WRF	16	/* write reference bursts		*/
#define	QIC117_VMD	17	/* verify mode				*/
#define	QIC117_STOP	18	/* stop tape motion			*/
#define	QIC117_SKPB	25	/* skip n segments back			*/
#define	QIC117_NMD	30	/* normal mode				*/

/************************************************************************/
/*	request buffer structure					*/
/************************************************************************/
struct rb{
	struct	rb *nxt;	/* ptr to next packet	*/
	unchar	fun;		/* function		*/
	unchar	sts;		/* status		*/
	ushort	sgn;		/* segment #		*/
	paddr_t	adr;		/* addr of buffer	*/
	ulong	map;		/* bad sector map	*/
	unchar	hed;		/* fdc head		*/
	unchar	cyl;		/* fdc cylinder		*/
	unchar	sct;		/* base fdc sector	*/
	unchar	trk;		/* tape track		*/
	unchar	tps;		/* segment		*/
	unchar	idc;		/* read id cylinder	*/
	unchar	ids;		/* sct			*/
	unchar	nbk;		/* # blocks		*/
	unchar	erc;		/* error count		*/
	unchar	ers[ 3 ]; 	/* error sectors	*/
	ushort	tbl[ 33 ];	/* parsed segment params*/
};

struct rbq {			/* buf queue structure	*/
	struct	rb *top;
	struct	rb *bot;
};

/************************************************************************/
/* 	Volume table structure (128 bytes)				*/
/************************************************************************/

typedef struct {
	unsigned  char	ident[4]; 	/* Volume entry signature "VTBL"*/
	unsigned  short	data_seg_num;	/* Starting segment number	*/
	unsigned  short	last_seg_num;	/* Ending segment number  	*/
	unsigned  char	op_system[6];	/* OS type "unix"	  	*/
	unsigned  char	p1[43];		/*  Null (offset 14-56)	   	*/
	char		c_seq_num;	/* Multicartridge seq #    	*/
					/*  initial cartridge = 1  	*/
	unsigned  short	p3[34];		/*  Null (offset 58-126)   	*/
	unsigned  short	last_blk_size;	/* Reserved. For UNIX/Xenix 	*/
					/*  last block data size   	*/
} xlvtbl;

/************************************************************************/
/*	xl type defines 						*/
/************************************************************************/

#define	fpchr	unsigned char *
#define	fpwrd	unsigned short *
#define	fplng	unsigned long *

/************************************************************************/
/*	xldefines							*/
/************************************************************************/

#define	RBFRD	0x01		/* rb.fun values			*/
#define	RBFWT	0x02
#define	RBWFD	0x03

#define	XLSRDY	0x01		/* xl6sts equates			*/
#define	XLSEXC	0x02
#define	XLSCIN	0x04
#define	XLSWRP	0x08
#define	XLSCHG	0x10
#define	XLSREF	0x20
#define	XLSBOT	0x40
#define	XLSEOT	0x80
/*				  xlster equates			*/
#define	XLSNEC	0x80		/* nec error				*/
#define	XLSLSB	0x40		/* last status bit != 1			*/
#define	XLSSFT	0x20		/* soft error:, exc, chg, or no cin	*/
#define	XLSNTD	0x10		/* not tape drive			*/
#define	XLSNID	0x08		/* can't read id's			*/

