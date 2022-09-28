/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:hdrs/bkerrors.h	1.6.2.1"

/* This file contains backup service global error returns */
#define	BKSUCCESS	0
#define	BKERRBASE	-100
#define	BKINTERNAL	(BKERRBASE - 1)
#define	BKNOTSPAWNED	(BKERRBASE - 2)
#define	BKBADREGTAB	(BKERRBASE - 3)
#define	BKBADARGS	(BKERRBASE - 4)
#define	BKMETHODDONE	(BKERRBASE - 5)
#define	BKTOOMANYARGS	(BKERRBASE - 6)
#define	BKNOMEMORY	(BKERRBASE - 7)
#define	BKNOFILE	(BKERRBASE - 8)
#define	BKERREXIT	(BKERRBASE - 9)
#define	BKNOTALLOWED	(BKERRBASE - 10)
#define	BKBADMESSAGE	(BKERRBASE - 11)
#define	BKBUSY	(BKERRBASE - 12)
#define	BKEXIST	(BKERRBASE - 13)
#define	BKNONE	(BKERRBASE - 14)
#define	BKFILE	(BKERRBASE - 15)
#define	BKBADID	(BKERRBASE - 16)
#define	BKCANCELLED	(BKERRBASE - 17)
#define BKBADREAD	(BKERRBASE - 18)
#define BKBADFIELD	(BKERRBASE - 19)
#define BKBADVALUE	(BKERRBASE - 20)
#define BKNORSMSG	(BKERRBASE - 21)
#define BKBADSEARCH	(BKERRBASE - 22)
#define BKBADDGROUP	(BKERRBASE - 23)
#define	BKBADTABLE	(BKERRBASE - 24)
#define BKBADWRITE	(BKERRBASE - 25)
#define BKBADASSIGN	(BKERRBASE - 26)
