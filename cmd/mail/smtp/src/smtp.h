/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:smtp/src/smtp.h	1.4.3.1"
/* smtp constants and the like */

/* tunable constants */
#define MAXSTR 10240			/* maximum string length */
#define NAMSIZ MAXSTR			/* max file name length */

typedef struct namelist namelist;
struct namelist {
	namelist *next;
	char *name;
};

/* spooling constants */
#define SMTP "/smtp"
