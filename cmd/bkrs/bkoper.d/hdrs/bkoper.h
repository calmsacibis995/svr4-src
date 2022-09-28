/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:bkoper.d/hdrs/bkoper.h	1.3.2.1"

/* SOME COMMON DEFINES */
#define	SKIPWS(p)	if( p ) while( *p == ' ' || *p == '\t' ) p++
#define	SEEKWS(p)	if( p ) while( *p && *p != ' ' && *p != '\t' && *p != '\n' )\ p++
#define	BSIZE	512
#define	VALID_USER(u)	(uname_to_uid(u) != -1)

/* List defines */
#define	L_EMPTY	l_empty()
#define	L_DOT	(L_EMPTY? 0: ldot->number)
#define	L_HEAD	l_head()
#define	L_DOLLAR	l_tail()

/* A doubly-linked list of methods waiting for operators */
typedef struct bko_list_s {
	struct bko_list_s *forward, *backward;
	int	number;
	char *jobid;
	char *tag;
	uid_t	uid;
	int flags;
	char *oname;
	char *odevice;
	char *starttime;
	char *status;
	char *dgroup;
	char *ddevice;
	char *dchar;
	char *dmname;
} bko_list_t;

/* Possible values for flags */
#define	BKO_OVERRIDE	0x1	/* operator is allowed to override label */
#define	BKO_DONE	0x2

/* Count BKO_DONE entries in l_find() or not */
#define	BKO_LOGICAL	0
#define	BKO_ABSOLUTE	1
