/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_CDUMP_H
#define _SYS_CDUMP_H

#ident	"@(#)head.sys:sys/cdump.h	11.2.7.1"

#define	BLKSZ		512
#define CHDR_OFFSET	1024
#define	CRASHSANITY	"crashdump"
#define	MAXBLKS		1422
#define NVR_OFFSET	512

struct	crash_hdr
{
	char	sanity[10] ;
	int	timestamp ,
		mem_size ,
		seq_num ;
} ;
#ifdef TEST
#undef BLKSZ
#undef CHDR_OFFSET
#undef MAXBLKS
#undef NVR_OFFSET
#undef MAINSTORE
#undef SIZOFMEM
#undef SPMEM
#undef PRINTF
#undef STRCMP
#undef GETS
#undef RNVRAM
#undef FD_ACS

#define BLKSZ		100
#define CHDR_OFFSET	80
#define MAXBLKS		5
#define NVR_OFFSET	20
#define MAINSTORE	mainstore
#define SIZOFMEM	sizofmem
#define SPMEM		(mainstore + 100)
#define PRINTF		printf
#define STRCMP		strcmp
#define GETS		gets

extern	int	mainstore ;
extern	int	sizofmem ;
#endif

#endif	/* _SYS_CDUMP_H */
