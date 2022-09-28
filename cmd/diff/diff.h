/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)diff:diff.h	1.3"

/*******************************************************************

		PROPRIETARY NOTICE (Combined)

This source code is unpublished proprietary information
constituting, or derived under license from AT&T's UNIX(r) System V.
In addition, portions of such source code were derived from Berkeley
4.3 BSD under license from the Regents of the University of
California.



		Copyright Notice 

Notice of copyright on this source code product does not indicate 
publication.

	(c) 1986,1987,1988,1989  Sun Microsystems, Inc
	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
	          All rights reserved.
********************************************************************/ 

/*
 * diff - common declarations
 */


void done();

/*
 * Output format options
 */
int	opt;

#define	D_NORMAL	0	/* Normal output */
#define	D_EDIT		-1	/* Editor script out */
#define	D_REVERSE	1	/* Reverse editor script */
#define	D_CONTEXT	2	/* Diff with context */
#define	D_IFDEF		3	/* Diff with merged #ifdef's */
#define	D_NREVERSE	4	/* Reverse ed script with numbered
				   lines and no trailing . */

/* 
 * Constant declarations 
 */
#define	HALFMASK	0xf

#define	prints(s)	fputs(s,stdout)

#define	MAX_CONTEXT	128

/*
 * diff - directory comparison
 */
#define	d_flags	d_ino

#define	ONLY	1		/* Only in this directory */
#define	SAME	2		/* Both places and same */
#define	DIFFER	4		/* Both places and different */
#define	DIRECT	8		/* Directory */

struct dir {
	u_long	d_ino;
	short	d_reclen;
	short	d_namlen;
	char	*d_entry;
};


/* 
 * type definitions 
 */

struct cand {
	int x;
	int y;
	int pred;
} cand;

struct line {
	int serial;
	int value;
} *file[2], line;

/*
 * The following struct is used to record change information when
 * doing a "context" diff.  (see routine "change" to understand the
 * highly mneumonic field names)
 */
struct context_vec {
	int	a;	/* start line in old file */
	int	b;	/* end line in old file */
	int	c;	/* start line in new file */
	int	d;	/* end line in new file */
};


/*
 * Algorithm related options
 */
int bflag=0;
int tflag=0;
int wflag=0;
int iflag=0;
int rflag=0;
int lflag=0;
int sflag=0;
int hflag=0;

/*
 * Variables for D_IFDEF option.
 */
int wantelses=0;	/* used with D_IFDEF */
char *ifdef1, *ifdef2;  /* hold the ifdef strings */
char *endifname;
int inifdef=0;

/*
 * Variables for -C (-c) context option.
 */
int context=0;		/* number of lines specfied with the C flag */

char *empty = "";	/* the empty string */

char **diffargv;	/* keep track of argv for diffdir */

char chrtran[256];	/* array of all characters, used with i flag */

char start[256];	/* specify where to start, used with -S */

FILE *input[2];		/* two input files */
int  len[2];
struct line *sfile[2];  /*shortened by pruning common prefix and suffix*/
int  slen[2];

struct stat stb0, stb1;

/*
 * Input file names.
 * With diffdir, file1 and file2 are allocated BUFSIZ space,
 * and padded with a '/', and then efile0 and efile1 point after
 * the '/'.
 */
char	*file1, *file2, *efile1, *efile2;
struct	stat stb1, stb2;
char	*file1, *file2, *efile1, *efile2;

char pr[] = "/usr/bin/pr";
char diff[] = "/usr/bin/diff";
char diffh[] = "/usr/lib/diffh";
int status = 2;
int anychange = 0;

struct	context_vec	*context_vec_start,
			*context_vec_end,
			*context_vec_ptr;

char tempfile[2][16];	/* used when comparing against std input
			   or char special devices */
int whichtemp;
char *dummy;		/*used in resetting storage search ptr*/
