/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)keyserv:chkey.c	1.7.2.1"

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
#ifndef lint
static char sccsid[] = "@(#)chkey.c 1.X 89/08/20 Copyr 1986 Sun Micro";
#endif
/*
 * Copyright (C) 1986, Sun Microsystems, Inc.
 */

/*
 * Command to change one's public key in the public key database
 */
#include <stdio.h>
#include <rpc/rpc.h>
#include <rpc/key_prot.h>
#ifdef YP
#include <rpcsvc/ypclnt.h>
#else
#define	YPOP_STORE	4
#endif
#include <pwd.h>
#include <string.h>

extern char *getpass();
#define	index strchr
extern char *crypt();
#ifdef YPPASSWD
struct passwd *ypgetpwuid();
#endif

#ifdef YP
static char *domain;
static char PKMAP[] = "publickey.byname";
#else
static char PKFILE[] = "/etc/publickey";
#endif	/* YP */
static char ROOTKEY[] = "/etc/.rootkey";

main(argc, argv)
	int argc;
	char **argv;
{
	char name[MAXNETNAMELEN+1];
	char public[HEXKEYBYTES + 1];
	char secret[HEXKEYBYTES + 1];
	char crypt1[HEXKEYBYTES + KEYCHECKSUMSIZE + 1];
	char crypt2[HEXKEYBYTES + KEYCHECKSUMSIZE + 1];
	int status;
	char *pass;
	struct passwd *pw;
	uid_t uid;
	int force = 0;
	char *self;
#ifdef YP
	char *master;
#endif

	self = argv[0];
	for (argc--, argv++; argc > 0 && **argv == '-'; argc--, argv++) {
		if (argv[0][2] != 0) {
			usage(self);
		}
		switch (argv[0][1]) {
		case 'f':
			force = 1;
			break;
		default:
			usage(self);
		}
	}
	if (argc != 0) {
		usage(self);
	}

#ifdef YP
	(void)yp_get_default_domain(&domain);
	if (yp_master(domain, PKMAP, &master) != 0) {
		(void)fprintf(stderr,
			"can't find master of publickey database\n");
		exit(1);
	}
#endif
	uid = geteuid();
	if (uid == 0) {
		if (host2netname(name, NULL, NULL) == 0) {
			(void)fprintf(stderr,
			"chkey: cannot convert hostname to netname\n");
			exit(1);
		}
	} else {
		if (user2netname(name, uid, NULL) == 0) {
			(void)fprintf(stderr,
			"chkey: cannot convert username to netname\n");
			exit(1);
		}
	}
	(void)printf("Generating new key for %s.\n", name);

	if (!force) {
		if (uid != 0) {
#ifdef YPPASSWD
			pw = ypgetpwuid(uid);
#else
			pw = getpwuid(uid);
#endif
			if (pw == NULL) {
#ifdef YPPASSWD
				(void)fprintf(stderr,
		"No yp password entry found: can't change key.\n");
#else
				(void)fprintf(stderr,
		"No password entry found: can't change key.\n");
#endif
				exit(1);
			}
		} else {
			pw = getpwuid(0);
			if (pw == NULL) {
				(void)fprintf(stderr,
				"No password entry found: can't change key.\n");
				exit(1);
			}
		}
	}
	pass = getpass("Password:");
#ifdef YPPASSWD
	if (!force) {
		if (strcmp(crypt(pass, pw->pw_passwd), pw->pw_passwd) != 0) {
			(void)fprintf(stderr, "Invalid password.\n");
			exit(1);
		}
	}
#else
	force = 1;	/* Make this mandatory */
#endif
	genkeys(public, secret, pass);

	memcpy(crypt1, secret, HEXKEYBYTES);
	memcpy(crypt1 + HEXKEYBYTES, secret, KEYCHECKSUMSIZE);
	crypt1[HEXKEYBYTES + KEYCHECKSUMSIZE] = 0;
	xencrypt(crypt1, pass);

	if (force) {
		memcpy(crypt2, crypt1, HEXKEYBYTES + KEYCHECKSUMSIZE + 1);
		xdecrypt(crypt2, getpass("Retype password:"));
		if (memcmp(crypt2, crypt2 + HEXKEYBYTES, KEYCHECKSUMSIZE) != 0 ||
		    memcmp(crypt2, secret, HEXKEYBYTES) != 0) {
			(void)fprintf(stderr, "Password incorrect.\n");
			exit(1);
		}
	}

#ifdef YP
	(void)printf("Sending key change request to %s...\n", master);
#endif
	status = setpublicmap(name, public, crypt1);
	if (status != 0) {
#ifdef YP
		(void)fprintf(stderr,
		"%s: unable to update yp database (%u): %s\n",
				self, status, yperr_string(status));
#else
		(void)fprintf(stderr,
		"%s: unable to update publickey database\n", self);
#endif
		exit(1);
	}

	if (uid == 0) {
		/*
		 * Root users store their key in /etc/$ROOTKEY so
		 * that they can auto reboot without having to be
		 * around to type a password. Storing this in a file
		 * is rather dubious: it should really be in the EEPROM
		 * so it does not go over the net.
		 */
		int fd;

		fd = open(ROOTKEY, O_WRONLY|O_TRUNC|O_CREAT, 0);
		if (fd < 0) {
			perror(ROOTKEY);
		} else {
			char newline = '\n';

			if (write(fd, secret, strlen(secret)) < 0 ||
			    write(fd, &newline, sizeof (newline)) < 0) {
				(void)fprintf(stderr, "%s: ", ROOTKEY);
				perror("write");
			}
		}
	}

	if (key_setsecret(secret) < 0) {
		(void)printf("Unable to login with new secret key.\n");
		exit(1);
	}
	(void)printf("Done.\n");
	exit(0);
	/* NOTREACHED */
}

usage(name)
	char *name;
{
	(void)fprintf(stderr, "usage: %s [-f]\n", name);
	exit(1);
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
	return (yp_update(domain, PKMAP, YPOP_STORE,
		name, strlen(name), pkent, strlen(pkent)));
#else
	return (localupdate(name, PKFILE, YPOP_STORE,
		strlen(name), name, strlen(pkent), pkent));
#endif
}

#ifdef YPPASSWD
struct passwd *
ypgetpwuid(uid)
	uid_t uid;
{
	char uidstr[10];
	char *val;
	int vallen;
	static struct passwd pw;
	char *p;

	(void)sprintf(uidstr, "%d", uid);
	if (yp_match(domain, "passwd.byuid", uidstr, strlen(uidstr),
			&val, &vallen) != 0) {
		return (NULL);
	}
	p = index(val, ':');
	if (p == NULL) {
		return (NULL);
	}
	pw.pw_passwd = p + 1;
	p = index(pw.pw_passwd, ':');
	if (p == NULL) {
		return (NULL);
	}
	*p = 0;
	return (&pw);
}
#endif	/* YPPASSWD */
