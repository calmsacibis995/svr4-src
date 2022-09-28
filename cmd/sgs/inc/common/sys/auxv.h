/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _AUXV_H
#define _AUXV_H
#ident	"@(#)sgs-inc:common/sys/auxv.h	1.1"

typedef struct
{
       int     a_type;
       union {
               long    a_val;
#ifdef __STDC__
               void    *a_ptr;
#else
               char    *a_ptr;
#endif
               void    (*a_fcn)();
       } a_un;
} auxv_t;

#define AT_NULL		0
#define AT_IGNORE	1
#define AT_EXECFD	2
#define AT_PHDR		3	/* &phdr[0] */
#define AT_PHENT	4	/* sizeof(phdr[0]) */
#define AT_PHNUM	5	/* # phdr entries */
#define AT_PAGESZ	6	/* getpagesize(2) */
#define AT_BASE		7	/* ld.so base addr */
#define AT_FLAGS	8	/* processor flags */
#define AT_ENTRY	9	/* a.out entry point */

#endif
