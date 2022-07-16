/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)master:kmacct/space.c	1.3"

#include "sys/types.h"
#include "sys/kmacct.h"

#define KMARRAY  150		/* KMARRAY is the number of entries in the
				   symbol table */

#define SDEPTH  5		/* SDEPTH is the depth of stack to trace back
                                  (no larger than MAXDEPTH from sys/kmacct.h) */

#define NKMABUF  1000		/* NKMABUF is the number of buffer headers to
				   allocate (one for each buffer that has been
				   allocated and not yet returned). */


int	kmacctflag = 0;
int	nkmasym  = KMARRAY;
int	kmadepth = SDEPTH;
int	nkmabuf  = NKMABUF;
struct	kmasym	kmasymtab[KMARRAY];
int	kmastack[KMARRAY*SDEPTH];
struct	kmabuf	kmabuf[NKMABUF];
