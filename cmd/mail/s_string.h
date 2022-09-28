/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:s_string.h	1.4.3.1"
/* extensible strings */

#ifndef _string_h
#define _string_h
#include <string.h>

typedef struct string {
	char *base;	/* base of string */
	char *end;	/* end of allocated space+1 */
	char *ptr;	/* ptr into string */
} string;

#define s_clone(s)	s_copy((s)->ptr)
#define s_curlen(s)	((s)->ptr - (s)->base)
#define s_dup(s)	s_copy((s)->base)
#define s_getc(s)	(*((s)->ptr++))
#define s_peek(s)	(*((s)->ptr))
#define s_putc(s,c)	(((s)->ptr<(s)->end) ? (*((s)->ptr)++ = (c)) : s_grow((s),(c)))
#define s_reset(s)	((s) ? (*((s)->ptr = (s)->base) = '\0' , (s)) : s_new())
#define s_restart(s)	((s)->ptr = (s)->base , (s))
#define s_skipc(s)	((s)->ptr++)
#define s_space(s)	((s)->end - (s)->base)
#define s_terminate(s)	(((s)->ptr<(s)->end) ? (*(s)->ptr = 0) : (s_grow((s),0), (s)->ptr--, 0))
#define s_to_c(s)	((s)->base)
#define s_ptr_to_c(s)	((s)->ptr)

#ifdef __STDC__
extern string *s_append(string *to, char *from);
extern string *s_array(char *, int len);
extern string *s_copy(char *);
extern void s_free(string*);
extern int s_grow(string *sp, int c);
extern string *s_new(void);
extern string *s_parse(string *from, string *to);
extern char *s_read_line(FILE *fp, string *to);
extern int s_read_to_eof(FILE *fp, string *to);
extern string *s_seq_read(FILE *fp, string *to, int lineortoken);
extern void s_skipwhite(string *from);
extern string *s_tok(string*,char*);
extern int s_tolower(string*);
#else
extern string *s_append();
extern string *s_array();
extern string *s_copy();
extern void s_free();
extern int s_grow();
extern string *s_new();
extern string *s_parse();
extern char *s_read_line();
extern int s_read_to_eof();
extern string *s_seq_read();
extern void s_skipwhite();
extern string *s_tok();
extern int s_tolower();
#endif

/* controlling the action of s_seq_read */
#define TOKEN 0		/* read the next whitespace delimited token */
#define LINE 1		/* read the next logical input line */
#define s_getline(a,b) s_seq_read(a,b,LINE)
#define s_gettoken(a,b) s_seq_read(a,b,TOKEN)

#endif
