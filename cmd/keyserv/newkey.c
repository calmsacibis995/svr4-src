/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)keyserv:newkey.c	1.9.2.1"

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*	PROPRIETARY NOTICE (Combined)
*
* This source code is unpublished proprietary information
* constituting, or derived under license from AT&T's UNIX(r) System V.
* In addition, portions of such source code were derived from Berkeley
* 4.3 BSD under license from the Regents of the University of
* California.
*
*
*
*	Copyright Notice
*
* Notice of copyright on this source code product does not indicate
*  publication.
*
*	(c) 1986,1987,1988.1989  Sun Microsystems, Inc
*	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
*          All rights reserved.
*/
#if !defined(lint) && defined(SCCSIDS)
static char sccsid[] = "@(#)newkey.c 1.X 89/8/17 Copyr 1986 Sun Micro";
#endif

/*
 * Copyright (C) 1986, Sun Microsystems, Inc.
 */

/*
 * Administrative tool to add a new user to the publickey database
 */
#include <stdio.h>
#include <rpc/rpc.h>
#include <rpc/key_prot.h>
#ifdef YP
#include <rpcsvc/ypclnt.h>
#include <sys/wait.h>
#include <netdb.h>
#endif	/* YP */
#include <pwd.h>
#include <string.h>
#include <sys/resource.h>

#ifdef YP
#define	MAXMAPNAMELEN 256
#else
#define	YPOP_CHANGE 1			/* change, do not add */
#define	YPOP_INSERT 2			/* add, do not change */
#define	YPOP_DELETE 3			/* delete this entry */
#define	YPOP_STORE  4			/* add, or change */
#define	ERR_ACCESS	1
#define	ERR_MALLOC	2
#define	ERR_READ	3
#define	ERR_WRITE	4
#define	ERR_DBASE	5
#define	ERR_KEY		6
#endif

extern char *getpass();
extern char *malloc();

#ifdef YP
static char *basename();
static char SHELL[] = "/bin/sh";
static char YPDBPATH[]="/var/yp";
static char PKMAP[] = "publickey.byname";
static char UPDATEFILE[] = "updaters";
#else
static char PKFILE[] = "/etc/publickey";
static char *err_string();
#endif	/* YP */

main(argc, argv)
	int argc;
	char *argv[];
{
	char name[MAXNETNAMELEN + 1];
	char public[HEXKEYBYTES + 1];
	char secret[HEXKEYBYTES + 1];
	char crypt1[HEXKEYBYTES + KEYCHECKSUMSIZE + 1];
	char crypt2[HEXKEYBYTES + KEYCHECKSUMSIZE + 1];
	int status;
	char *pass;
	struct passwd *pw;
#ifdef undef
	struct hostent *h;
#endif

	if (argc != 3 || !(strcmp(argv[1], "-u") == 0 ||
		strcmp(argv[1], "-h") == 0)) {
		(void)fprintf(stderr, "usage: %s [-u username]\n",
					argv[0]);
		(void)fprintf(stderr, "usage: %s [-h hostname]\n",
					argv[0]);
		exit(1);
	}
	if (geteuid() != 0) {
		(void)fprintf(stderr, "must be superuser to run %s\n", argv[0]);
		exit(1);
	}

#ifdef YP
	if (chdir(YPDBPATH) < 0) {
		(void)fprintf(stderr, "cannot chdir to ");
		perror(YPDBPATH);
	}
#endif	/* YP */
	if (strcmp(argv[1], "-u") == 0) {
		pw = getpwnam(argv[2]);
		if (pw == NULL) {
			(void)fprintf(stderr, "unknown user: %s\n", argv[2]);
			exit(1);
		}
		(void)user2netname(name, (int)pw->pw_uid, (char *)NULL);
	} else {
#ifdef undef
		h = gethostbyname(argv[2]);
		if (h == NULL) {
			(void)fprintf(stderr, "unknown host: %s\n", argv[1]);
			exit(1);
		}
		(void)host2netname(name, h->h_name, (char *)NULL);
#else
		(void)host2netname(name, argv[2], (char *)NULL);
#endif
	}

	(void)printf("Adding new key for %s.\n", name);
	pass = getpass("New password:");
	genkeys(public, secret, pass);

	memcpy(crypt1, secret, HEXKEYBYTES);
	memcpy(crypt1 + HEXKEYBYTES, secret, KEYCHECKSUMSIZE);
	crypt1[HEXKEYBYTES + KEYCHECKSUMSIZE] = 0;
	xencrypt(crypt1, pass);

	memcpy(crypt2, crypt1, HEXKEYBYTES + KEYCHECKSUMSIZE + 1);
	xdecrypt(crypt2, getpass("Retype password:"));
	if (memcmp(crypt2, crypt2 + HEXKEYBYTES, KEYCHECKSUMSIZE) != 0 ||
		memcmp(crypt2, secret, HEXKEYBYTES) != 0) {
		(void)fprintf(stderr, "Password incorrect.\n");
		exit(1);
	}

#ifdef YP
	(void)printf("Please wait for the database to get updated...\n");
#endif
	if (status = setpublicmap(name, public, crypt1)) {
#ifdef YP
		(void)fprintf(stderr,
		"%s: unable to update yp database (%u): %s\n",
			argv[0], status, yperr_string(status));
#else
		(void)fprintf(stderr,
		"%s: unable to update publickey database (%u): %s\n",
			argv[0], status, err_string(status));
#endif
		exit(1);
	}
	(void)printf("Your new key has been successfully stored away.\n");
	exit(0);
	/* NOTREACHED */
}

/*
 * Set the entry in the public key file
 */
setpublicmap(name, public, secret)
	char *name;
	char *public;
	char *secret;
{
	char pkent[1024];

	(void)sprintf(pkent, "%s:%s", public, secret);
#ifdef YP
	return (mapupdate(name, PKMAP, YPOP_STORE,
		strlen(name), name, strlen(pkent), pkent));
#else
	return (localupdate(name, PKFILE, YPOP_STORE,
		strlen(name), name, strlen(pkent), pkent));
#endif
}

#ifndef YP
/*
 * This returns a pointer to an error message string appropriate
 * to an input error code.  An input value of zero will return
 * a success message.
 */
static char *
err_string(code)
	int code;
{
	char *pmesg;

	switch (code) {
	case 0:
		pmesg = "update operation succeeded";
		break;
	case ERR_KEY:
		pmesg = "no such key in file";
		break;
	case ERR_READ:
		pmesg = "cannot read the database";
		break;
	case ERR_WRITE:
		pmesg = "cannot write to the database";
		break;
	case ERR_DBASE:
		pmesg = "cannot update database";
		break;
	case ERR_ACCESS:
		pmesg = "permission denied";
		break;
	case ERR_MALLOC:
		pmesg = "malloc failed";
		break;
	default:
		pmesg = "unknown error";
		break;
	}
	return (pmesg);
}
#endif
