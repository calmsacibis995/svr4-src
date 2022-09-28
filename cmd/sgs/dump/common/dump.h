/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)dump:common/dump.h	1.2"
#define DATESIZE 60

typedef struct scntab {
	char             *scn_name;
	Elf32_Shdr       *p_shdr;
	Elf_Scn          *p_sd;
} SCNTAB;

#ifdef __STDC__
#define VOID_P void *
#else
#define VOID_P char *
#endif

#define UCHAR_P unsigned char *

#define FAILURE 0
#define SUCCESS 1
