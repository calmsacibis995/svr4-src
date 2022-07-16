/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)stitam:wind.h	1.1"
#define W_POPUP		0		/* inside other window		*/
#define W_SON		1		/* adjacent to other window	*/
#define W_NEW		2		/* in new screen place		*/

#define WERR_OK		0		/* no error			*/
#define WERR_TOOBIG	-1
#define WERR_NOMEM	-7		/* if malloc failed		*/

extern char wsigflag;
