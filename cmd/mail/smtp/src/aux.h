/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:smtp/src/aux.h	1.3.3.1"
/* ident "@(#)aux.h	1.2 'attmail mail(1) command'" */
extern string *abspath();
extern char *sysname_read();
extern char *basename();
extern int delivery_status();
extern void append_match();

/* mailbox types */
#define MF_NORMAL 0
#define MF_PIPE 1
#define MF_FORWARD 2
#define MF_NOMBOX 3
#define MF_NOTMBOX 4
