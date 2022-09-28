/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)keyserv:keyserv.c	1.13.3.1"

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

/*
 * Keyserver
 * Store secret keys per uid. Do public key encryption and decryption
 * operations. Generate "random" keys.
 * Do not talk to anything but a local root
 * process on the local transport only
 */

#include <stdio.h>
#include <rpc/rpc.h>
#include <sys/param.h>
#include <sys/file.h>
#include <pwd.h>
#include <rpc/des_crypt.h>
#include <rpc/key_prot.h>

#ifdef KEYSERV_RANDOM
extern long random();
#endif
#ifndef NGROUPS
#define	NGROUPS 16
#endif

extern keystatus pk_setkey();
extern keystatus pk_encrypt();
extern keystatus pk_decrypt();

#ifdef DEBUG
int debugging = 1;
#else
int debugging = 0;
#endif

static void keyprogram();
des_block masterkey;
char *getenv();
static char ROOTKEY[] = "/etc/.rootkey";

main(argc, argv)
	int argc;
	char *argv[];
{
	int nflag = 0;
	extern char *optarg;
	extern int optind;
	int c;

	while ((c = getopt(argc, argv, "ndD")) != -1)
		switch (c) {
		case 'n':
			nflag++;
			break;
		case 'd':
			pk_nodefaultkeys();
			break;
		case 'D':
			debugging = 1;
			break;
		default:
			usage();
		}

	if (optind != argc) {
		usage();
	}

	/*
	 * Initialize
	 */
	(void) umask(066);	/* paranoia */
	if (geteuid() != 0) {
		(void) fprintf(stderr, "%s must be run as root\n", argv[0]);
		exit(1);
	}
	setmodulus(HEXMODULUS);
	getrootkey(&masterkey, nflag);

	if (svc_create_local_service(keyprogram, KEY_PROG, KEY_VERS,
		"netpath", "keyserv") == 0) {
		(void) fprintf(stderr,
			"%s: unable to create service\n", argv[0]);
		exit(1);
	}
	if (!debugging) {
		detachfromtty();
	}
	svc_run();
	abort();
	/* NOTREACHED */
}

/*
 * In the event that we don't get a root password, we try to
 * randomize the master key the best we can
 */
static
randomize(master)
	des_block *master;
{
	int i;
	int seed;
	struct timeval tv;
	int shift;

	seed = 0;
	for (i = 0; i < 1024; i++) {
		(void) gettimeofday(&tv, (struct timezone *) NULL);
		shift = i % 8 * sizeof (int);
		seed ^= (tv.tv_usec << shift) | (tv.tv_usec >> (32 - shift));
	}
#ifdef KEYSERV_RANDOM
	srandom(seed);
	master->key.low = random();
	master->key.high = random();
	srandom(seed);
#else
	/* use stupid dangerous bad rand() */
	srand(seed);
	master->key.low = rand();
	master->key.high = rand();
	srand(seed);
#endif
}

/*
 * Try to get root's secret key, by prompting if terminal is a tty, else trying
 * from standard input.
 * Returns 1 on success.
 */
static
getrootkey(master, prompt)
	des_block *master;
	int prompt;
{
	char *getpass();
	char *passwd;
	char name[MAXNETNAMELEN + 1];
	char secret[HEXKEYBYTES + 1];
	char *crypt();
	char *strrchr();
	int fd;

	if (!prompt) {
		/*
		 * Read secret key out of $ROOTKEY
		 */
		fd = open(ROOTKEY, O_RDONLY, 0);
		if (fd < 0) {
			randomize(master);
			return (0);
		}
		if (read(fd, secret, HEXKEYBYTES) < 0) {
			(void) fprintf(stderr, "Invalid %s\n", ROOTKEY);
			(void) close(fd);
			return (0);
		}
		(void) close(fd);
		secret[HEXKEYBYTES] = 0;
		(void) pk_setkey(0, secret);
		return (1);
	}
	/*
	 * Decrypt yellow pages publickey entry to get secret key
	 */
	passwd = getpass("root password:");
	passwd2des(passwd, master);
	getnetname(name);
	if (!getsecretkey(name, secret, passwd)) {
		(void) fprintf(stderr,
		"Can't find %s's secret key\n", name);
		return (0);
	}
	if (secret[0] == 0) {
		(void) fprintf(stderr,
	"Password does not decrypt secret key for %s\n", name);
		return (0);
	}
	(void) pk_setkey(0, secret);
	/*
	 * Store it for future use in $ROOTKEY, if possible
	 */
	fd = open(ROOTKEY, O_WRONLY|O_TRUNC|O_CREAT, 0);
	if (fd > 0) {
		char newline = '\n';

		write(fd, secret, strlen(secret));
		write(fd, &newline, sizeof (newline));
		close(fd);
	}
	return (1);
}

/*
 * Procedures to implement RPC service
 */
char *
strstatus(status)
	keystatus status;
{
	switch (status) {
	case KEY_SUCCESS:
		return ("KEY_SUCCESS");
	case KEY_NOSECRET:
		return ("KEY_NOSECRET");
	case KEY_UNKNOWN:
		return ("KEY_UNKNOWN");
	case KEY_SYSTEMERR:
		return ("KEY_SYSTEMERR");
	default:
		return ("(bad result code)");
	}
}

keystatus *
key_set_1(uid, key)
	uid_t uid;
	keybuf key;
{
	static keystatus status;

	if (debugging) {
		(void) fprintf(stderr, "set(%d, %.*s) = ", uid,
				sizeof (keybuf), key);
	}
	status = pk_setkey(uid, key);
	if (debugging) {
		(void) fprintf(stderr, "%s\n", strstatus(status));
		(void) fflush(stderr);
	}
	return (&status);
}

cryptkeyres *
key_encrypt_1(uid, arg)
	uid_t uid;
	cryptkeyarg *arg;
{
	static cryptkeyres res;

	if (debugging) {
		(void) fprintf(stderr, "encrypt(%d, %s, %08x%08x) = ", uid,
				arg->remotename, arg->deskey.key.high,
				arg->deskey.key.low);
	}
	res.cryptkeyres_u.deskey = arg->deskey;
	res.status = pk_encrypt(uid, arg->remotename,
				&res.cryptkeyres_u.deskey);
	if (debugging) {
		if (res.status == KEY_SUCCESS) {
			(void) fprintf(stderr, "%08x%08x\n",
					res.cryptkeyres_u.deskey.key.high,
					res.cryptkeyres_u.deskey.key.low);
		} else {
			(void) fprintf(stderr, "%s\n", strstatus(res.status));
		}
		(void) fflush(stderr);
	}
	return (&res);
}

cryptkeyres *
key_decrypt_1(uid, arg)
	uid_t uid;
	cryptkeyarg *arg;
{
	static cryptkeyres res;

	if (debugging) {
		(void) fprintf(stderr, "decrypt(%d, %s, %08x%08x) = ", uid,
				arg->remotename, arg->deskey.key.high,
				arg->deskey.key.low);
	}
	res.cryptkeyres_u.deskey = arg->deskey;
	res.status = pk_decrypt(uid, arg->remotename,
				&res.cryptkeyres_u.deskey);
	if (debugging) {
		if (res.status == KEY_SUCCESS) {
			(void) fprintf(stderr, "%08x%08x\n",
					res.cryptkeyres_u.deskey.key.high,
					res.cryptkeyres_u.deskey.key.low);
		} else {
			(void) fprintf(stderr, "%s\n", strstatus(res.status));
		}
		(void) fflush(stderr);
	}
	return (&res);
}

/* ARGSUSED */
des_block *
key_gen_1()
{
	struct timeval time;
	static des_block keygen;
	static des_block key;

	(void) gettimeofday(&time, (struct timezone *) NULL);
	keygen.key.high += (time.tv_sec ^ time.tv_usec);
	keygen.key.low += (time.tv_sec ^ time.tv_usec);
	ecb_crypt(&masterkey, &keygen, sizeof (keygen), DES_ENCRYPT | DES_HW);
	key = keygen;
	des_setparity(&key);
	if (debugging) {
		(void) fprintf(stderr, "gen() = %08x%08x\n", key.key.high,
					key.key.low);
		(void) fflush(stderr);
	}
	return (&key);
}

getcredres *
key_getcred_1(uid, name)
	uid_t uid;
	netnamestr *name;
{
	static getcredres res;
	static u_int gids[NGROUPS];
	struct unixcred *cred;

	cred = &res.getcredres_u.cred;
	cred->gids.gids_val = gids;
	if (!netname2user(*name, &cred->uid, &cred->gid,
			&cred->gids.gids_len, gids)) {
		res.status = KEY_UNKNOWN;
	} else {
		res.status = KEY_SUCCESS;
	}
	if (debugging) {
		(void) fprintf(stderr, "getcred(%s) = ", *name);
		if (res.status == KEY_SUCCESS) {
			(void) fprintf(stderr, "uid=%d, gid=%d, grouplen=%d\n",
				cred->uid, cred->gid, cred->gids.gids_len);
		} else {
			(void) fprintf(stderr, "%s\n", strstatus(res.status));
		}
		(void) fflush(stderr);
	}
	return (&res);
}

/*
 * RPC boilerplate
 */
static void
keyprogram(rqstp, transp)
	struct svc_req *rqstp;
	SVCXPRT *transp;
{
	union {
		keybuf key_set_1_arg;
		cryptkeyarg key_encrypt_1_arg;
		cryptkeyarg key_decrypt_1_arg;
		des_block key_gen_1_arg;
	} argument;
	char *result;
	bool_t(*xdr_argument)(), (*xdr_result)();
	char *(*local) ();
	uid_t uid;
	int check_auth;

	switch (rqstp->rq_proc) {
	case NULLPROC:
		svc_sendreply(transp, xdr_void, (char *)NULL);
		return;

	case KEY_SET:
		xdr_argument = xdr_keybuf;
		xdr_result = xdr_int;
		local = (char *(*)()) key_set_1;
		check_auth = 1;
		break;

	case KEY_ENCRYPT:
		xdr_argument = xdr_cryptkeyarg;
		xdr_result = xdr_cryptkeyres;
		local = (char *(*)()) key_encrypt_1;
		check_auth = 1;
		break;

	case KEY_DECRYPT:
		xdr_argument = xdr_cryptkeyarg;
		xdr_result = xdr_cryptkeyres;
		local = (char *(*)()) key_decrypt_1;
		check_auth = 1;
		break;

	case KEY_GEN:
		xdr_argument = xdr_void;
		xdr_result = xdr_des_block;
		local = (char *(*)()) key_gen_1;
		check_auth = 0;
		break;

	case KEY_GETCRED:
		xdr_argument = xdr_netnamestr;
		xdr_result = xdr_getcredres;
		local = (char *(*)()) key_getcred_1;
		check_auth = 0;
		break;

	default:
		svcerr_noproc(transp);
		return;
	}
	if (check_auth) {
		if (root_auth(transp, rqstp) == 0) {
			if (debugging) {
				(void) fprintf(stderr,
				"not local privileged process\n");
			}
			svcerr_weakauth(transp);
			return;
		}
		if (rqstp->rq_cred.oa_flavor != AUTH_SYS) {
			if (debugging) {
				(void) fprintf(stderr,
				"not unix authentication\n");
			}
			svcerr_weakauth(transp);
			return;
		}
		uid = ((struct authsys_parms *)rqstp->rq_clntcred)->aup_uid;
	}
	memset((char *) &argument, 0, sizeof (argument));
	if (!svc_getargs(transp, xdr_argument, &argument)) {
		svcerr_decode(transp);
		return;
	}
	result = (*local) (uid, &argument);
	if (!svc_sendreply(transp, xdr_result, (char *) result)) {
		if (debugging)
			(void) fprintf(stderr, "unable to reply\n");
		svcerr_systemerr(transp);
	}
	if (!svc_freeargs(transp, xdr_argument, &argument)) {
		if (debugging)
			(void) fprintf(stderr,
			"unable to free arguments\n");
		exit(1);
	}
	return;
}

static
int root_auth(trans, rqstp)
	SVCXPRT *trans;
	struct svc_req *rqstp;
{
	uid_t uid;

	if (_rpc_get_local_uid(trans, &uid) < 0) {
		if (debugging)
			fprintf(stderr, "_rpc_get_local_uid failed %s %s\n",
				trans->xp_netid, trans->xp_tp);
		return (0);
	}
	if (debugging)
		fprintf(stderr, "local_uid  %d\n", uid);
	if (uid == 0)
		return (1);
	if (rqstp->rq_cred.oa_flavor == AUTH_SYS) {
		if (((uid_t) ((struct authunix_parms *)rqstp->rq_clntcred)->aup_uid)
			== uid) {
			return (1);
		} else {
			if (debugging)
				fprintf(stderr,
			"local_uid  %d mismatches auth %d\n", uid,
((uid_t) ((struct authunix_parms *)rqstp->rq_clntcred)->aup_uid));
			return (0);
		}
	} else {
		if (debugging)
			fprintf(stderr, "Not auth sys\n");
		return (0);
	}
}

static
usage()
{
	(void) fprintf(stderr, "usage: keyserv [-n] [-D] [-d]\n");
	(void) fprintf(stderr, "-d disables the use of default keys\n");
	exit(1);
}
