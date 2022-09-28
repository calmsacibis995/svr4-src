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

#ident	"@(#)fmli:inc/winp.h	1.10"

#define MAXSUBS	5

/*	For possible "undo" operation enhancement 
typedef struct _lastop {
	token tok;
	int oprow;
	int opcol;
	int count;
	char *str;
} undo;
*/

typedef struct _inp {
	int frow;		/* first row of field within window */
	int fcol;		/* first column of field within window */
	int rows;		/* number of rows in field */
	int cols;		/* number of cols in field */
	int currow;		/* current row within field */
	int curcol;		/* current column within field */
	int flags;		/* see field flags below */
	chtype   fieldattr;	/* highlight attribute (i.e., underlined)    */
	chtype   lastattr;	/* attribute of last char written from value *
				 * string so broken writes can continue ok   */
	int      currtype;	/* used to indicate proper (sub)stream       */ 
	chtype  *scrollbuf;	/* buffer of scrolled lines                  */
	unsigned buffoffset;	/* 1st (leftmost)  char visible in field is  *
				 * at scrollbuf + buffoffset                 */
	unsigned buffsize;	/* number of chars that fit in scroll buffer */
	unsigned bufflast;	/* number of valid chars/line in scroll buffer */
	char *value;		/* present field value (since last sync)    */
	char *valptr;		/* current offset into the value string;    * 
				 * used primarily for scrolling fields to   *
				 * point to that part of the value string   *
				 * that has not been part of the visible    *
				 * scroll window, (i.e., not yet part of    *
				 * scrollbuf)				    */
} ifield;

/* field flags */
#define	I_NOEDIT	0x0001
#define I_CHANGED	0x0002
#define I_NOPAGE	0x0004
#define I_STRIPLEAD	0x0008
#define I_INSERT	0x0010
#define I_FULLWIN	0x0020
#define I_WRAP		0x0040
#define I_BLANK		0x0080
#define I_FANCY		0x0100	/* probably unused */
#define I_SCROLL	0x0200
#define I_FILL		0x0400
#define I_INVISIBLE	0x0800
#define I_NOSHOW	0x1000
#define I_TEXT		0x2000	/* a text objext */
#define I_AUTOADV       0x4000	/* autoadvance enabled */
#define	I_CHANGEABLE	~(I_CHANGED)

extern ifield *Cfld;	/* current field */
