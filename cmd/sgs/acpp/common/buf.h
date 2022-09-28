/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident "@(#)acpp:common/buf.h	1.19"
/* buf.h - input buffer interfaces */

/* mark in the input buffer that signifies "end of data" */
#define	BF_SENTINEL	'\0'

/* for now - belongs in cpp.h(token.h?), along with tk_chtypes ? */
#define CH_ALPHANUM_DOT	0x01	/* alphanumeric or '.': a-z, A-Z, 0-9, '.' */
#define CH_PLUS_MINUS	0x02	/* '+' or '-' */
#define CH_INT_SUFFIX	0x04	/* integer constant suffix: 'l', 'L', 'u', 'U' */
#define CH_WHITE	0x08	/* white-space */
#define CH_OCTAL	0x10	/* octal digit: 0-7 */
#define CH_HEX		0x20	/* hex digit: 0-9a-f,A-F */
#define CH_DECIMAL	0x40	/* a decimal digit: 0-9 */
#define CH_IDENT	0x80	/* a letter, underscore or decimal digit */

#ifdef __STDC__
typedef enum mode_ {	/* "modes" of buffer handling and tokenization */
	B_text =	0x0100,	/* normal text, write to output if possible */
	B_tokens =	0x0200,	/* normal text, return all Tokens	*/
	B_invocation =	0x0400,	/* return next line of multi-line invocation */
	B_macroname =	0x0800,	/* return next line of possible invocation */
	B_comment =	0x1000	/* get next line of multi-line comment	*/
} Mode;
#else	/* 2.1 has limited enumeration functionality: can't assign enum member to int */
typedef int Mode;
#	define	B_text		0x0100
#	define	B_tokens	0x0200
#	define	B_invocation	0x0400
#	define	B_macroname	0x0800
#	define	B_comment	0x1000
#endif

extern	long	bf_cur;
extern	long	bf_eod;
extern	long	bf_prev;
extern long	bf_lineno;		/* current line number */
extern	char *	bf_ptr;
extern	int	bf_newlines;

extern	unsigned int	bf_curline(/* void */ );
extern	void	bf_init( /* void	*/);
extern	Token*	bf_tokenize( /* Mode, Token * */);	
