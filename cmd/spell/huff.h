/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)spell:huff.h	1.3"

extern struct huff {
	long xn;
	int xw;
	long xc;
	long xcq;	/* (c,0) */
	long xcs;	/* c left justified */
	long xqcs;	/* (q-1,c,q) left justified */
	long xv0;
} huffcode;
#define n huffcode.xn
#define w huffcode.xw
#define c huffcode.xc
#define cq huffcode.xcq
#define cs huffcode.xcs
#define qcs huffcode.xqcs
#define v0 huffcode.xv0
