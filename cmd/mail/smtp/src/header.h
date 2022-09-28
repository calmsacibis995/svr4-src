/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:smtp/src/header.h	1.3.3.1"
/* ident "@(#)header.h	1.2 'attmail mail(1) command'" */
typedef struct {
	char *name;	/* header name */
	int size;	/* size of name */
	string *line;	/* header line */
} header;

extern header hdrs[];		/* important headers */

/* some useful macros */
#define HEADER(s) { s, sizeof(s)-1, (string *)0 }
#define STRCMP(s, p) strncmp((s), (p)->name, (p)->size)
#define HCONTENT(p) (s_to_c(p.line) + p.size)
