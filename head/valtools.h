/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oamhdrs:valtools.h	1.2"

struct _choice_ {
	char *token;
	char *text;
	struct _choice_ *next;
};

struct _menu_ {
	char	*label;
	int	attr;
	short	longest;
	short	nchoices;
	struct _choice_ 
		*choice;
	char	**invis;
};

typedef struct _menu_ CKMENU;

#define P_ABSOLUTE	0x0001
#define P_RELATIVE	0x0002
#define P_EXIST		0x0004
#define P_NEXIST	0x0008
#define P_REG		0x0010
#define P_DIR		0x0020
#define P_BLK		0x0040
#define P_CHR		0x0080
#define P_NONZERO	0x0100
#define P_READ		0x0200
#define P_WRITE		0x0400
#define P_EXEC		0x0800
#define P_CREAT		0x1000

#define CKUNNUM		0x01
#define CKALPHA		0x02
#define CKONEFLAG	0x04
