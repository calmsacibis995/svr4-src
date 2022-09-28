/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 *
 */

#ident	"@(#)fmli:qued/fmacs.h	1.8"

extern chtype getattr();
extern chtype acswreadchar();

/* defines for Substreams */
#define SINGLE	0
#define MULTI	1

/* Miscellaneous defines and macros */
#define Fieldrows	(Cfld->rows)
#define Fieldcols	(Cfld->cols)
#define Flags		(Cfld->flags)
#define Fieldattr	(Cfld->fieldattr)
#define Lastattr	(Cfld->lastattr)
#define Currtype	(Cfld->currtype)
#define Scrollbuf	(Cfld->scrollbuf)
#define Buffoffset	(Cfld->buffoffset)
#define Buffsize	(Cfld->buffsize)
#define Bufflast	(Cfld->bufflast)
#define Value		(Cfld->value)
#define Valptr		(Cfld->valptr)

/* computational macros */
#define LASTCOL		(Cfld->cols - 1)
#define LASTROW		(Cfld->rows - 1)
#define LINEBYTES	(Cfld->cols + 1)
#define FIELDBYTES	(Cfld->rows * (Cfld->cols + 1))


/* field character operation macros */
#define	freadchar(r,c)	wreadchar(r+Cfld->frow,c+Cfld->fcol)
#define	acsreadchar(r,c) acswreadchar(r+Cfld->frow,c+Cfld->fcol)
#define fputchar(x)	wputchar(x, Fieldattr, NULL);

#define UP	0
#define DOWN	1
#define LEFT	2
#define RIGHT	3
