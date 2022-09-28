/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)as:common/codeout.h	1.5"

#define alloc_buffer(buf_type) (buf_type *) calloc(BUFSIZ, sizeof(buf_type))

#define FULL(buf) (buf) == BUFSIZ - 1

#define EMPTY(buf) (buf) == -1

#define ENTER_DATA(buffer, type, size, align)  \
		data->d_buf = (VOID *) (buffer); \
		data->d_type = (type);   \
		data->d_size = (size) ;  \
		data->d_align = (align); \
		data->d_version = EV_CURRENT; 

/*	codebuf passes info from pass1 to pass2
 *
 *	AMASK	action mask, 64 actions max
 *	EFLAG	interpret csym as expression pointer
 */

typedef struct {				/* user defined section data */
		long		cvalue;		/* value to generate */
		symbol		*csym;		/* point to sym or expr */
		unsigned short	cline;		/* newline number */
		unsigned short	errline;	/* line no for error messages */
		unsigned char	caction;	/* addressing mode action routine*/
		unsigned char	cnbits;		/* number of bits to generate */
} codebuf;

#define CB_AMASK	0x3f
#define CB_EFLAG	0x40

#ifndef NCODE
#	define NCODE	1000
#endif


typedef struct CODEBOX	CODEBOX;
struct	CODEBOX
{
	codebuf	*c_end;		/* next free c_buf[] entry */
	CODEBOX	*c_link;
	codebuf	c_buf[NCODE];
};


extern CODEBOX	*Codebox;
extern codebuf	*Code;
	

typedef struct symentag {
	long stindex;
	long fcnlen;
	long fwdindex;
	struct symentag *stnext;
} stent;

typedef struct {
	long relval;
	char *relname;
	BYTE reltype;
	unsigned short lineno;	/* line number of reference for error message */
} prelent;





