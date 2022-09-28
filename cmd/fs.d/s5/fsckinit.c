/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)s5.cmds:fsckinit.c	1.4.3.1"
/*
 *  These are the initialization functions for fsck.
 */

#include <sys/types.h>
#include <sys/fs/s5param.h>


int	F_NUMTRIPLE;
int	F_BSIZE;
int	F_BSHIFT;
int	F_BMASK ;
int	F_INOPB ;
int	F_NINDIR;
int	F_INOSHIFT;

#if FsTYPE == 1

	init_512()
	{
		F_NUMTRIPLE = 255;
		F_BSIZE = 512;
		F_BSHIFT = 9;
		F_BMASK = 0777;
		F_INOPB = 8;
		F_NINDIR = (512/sizeof(daddr_t));
		F_INOSHIFT = 3;
	}

#elif FsTYPE == 2

	init_1024()
	{
		F_NUMTRIPLE = 31;
		F_BSIZE = 1024;
		F_BSHIFT = 10;
		F_BMASK = 01777;
		F_INOPB = 16;
		F_NINDIR = (1024/sizeof(daddr_t));
		F_INOSHIFT = 4;
	}

#elif FsTYPE == 4

	init_2048()
	{
		F_NUMTRIPLE = 3;
		F_BSIZE = 2048;
		F_BSHIFT = 11;
		F_BMASK = 03777;
		F_INOPB = 32;
		F_NINDIR = 512;
		F_INOSHIFT = 5;
	}

#else
/* #error "FsTYPE is unknown"     USE IN ANSI-COMPILATION SYSTEMS */
#include "FsTYPE is unknown"
#endif
