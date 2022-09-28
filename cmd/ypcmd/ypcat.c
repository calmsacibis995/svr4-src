/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libyp:ypcat.c	1.3.4.1"

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
static	char sccsid[] = "@(#)ypcat.c 1.9 86/07/16 Copyr 1985 Sun Micro";
#endif

/*
 * This is a user command which dumps each entry in a yp data base.  It gets
 * the stuff using the normal ypclnt package; the user doesn't get to choose
 * which server gives him the input.  Usage is:
 * ypcat [-k] [-d domain] map
 * where the -k switch will dump keys followed by a single blank space
 * before the value, and the -d switch can be used to specify a domain other
 * than the default domain.
 * 
 */
#ifdef NULL
#undef NULL
#endif
#define NULL 0
#include <limits.h>
#include <stdio.h>
#include <rpc/rpc.h>
#include "yp_b.h"
#include <rpcsvc/ypclnt.h>
#include <rpcsvc/yp_prot.h>

extern int yp_getalias();
static char domain_alias[YPMAXDOMAIN];    /* nickname for domain names */

static int dumpkeys = FALSE;
static char *domain = NULL;
static char default_domain_name[YPMAXDOMAIN];
static char *map = NULL;
static char nullstring[] = "";
static char err_usage[] =
"Usage:\n\
	ypcat [-k] [-d domainname] [-t] mapname\n";
static char err_bad_args[] =
	"ypcat:  %s argument is bad.\n";
static char err_cant_get_kname[] =
	"ypcat:  can't get %s back from system call.\n";
static char err_null_kname[] =
	"ypcat:  the %s hasn't been set on this machine.\n";
static char err_bad_domainname[] = "domainname";
static char err_cant_bind[] =
	"ypcat:  can't bind to yp server for domain %s.  Reason:  %s.\n";
static char err_first_failed[] =
	"ypcat:  can't get first record from yp.  Reason:  %s.\n";
static char err_next_failed[] =
	"ypcat:  can't get next record from yp.  Reason:  %s.\n";

static void get_command_line_args();
static int callback();
static void one_by_one_all();

extern size_t strlen();
extern int strcmp();
extern int getdomainname();
extern void exit();
extern void free();

/*
 * This is the mainline for the ypcat process.  It pulls whatever arguments
 * have been passed from the command line, and uses defaults for the rest.
 */

main (argc, argv)
	int argc;
	char **argv;
	
{
	int err;
	int fail=0;
	struct ypall_callback cbinfo;
	
	get_command_line_args(argc, argv);

	if (!domain) {
		
		if (!getdomainname(default_domain_name, YPMAXDOMAIN) ) {
			domain = default_domain_name;
		} else {
			(void) fprintf(stderr, err_cant_get_kname, err_bad_domainname);
			exit(1);
		}

		if ((int)strlen(domain) == 0) {
			(void) fprintf(stderr, err_null_kname, err_bad_domainname);
			exit(1);
		}
	}

	/* reads in alias file for filename translation */
	sysvconfig();

	/* get the alias name for the domain */
	if (yp_getalias(domain, domain_alias, NAME_MAX) != 0)
                (void) fprintf(stderr, "domain alias for %s not found\n", domain);

	domain = domain_alias;
	if (err = yp_bind(domain) ) {
		(void) fprintf(stderr, err_cant_bind, domain,
		    yperr_string(err) );
		exit(1);
	}

	cbinfo.foreach = callback;
	cbinfo.data = (char *) &fail;
	err = yp_all(domain, map, &cbinfo);

	if (err == YPERR_VERS) {
		one_by_one_all(domain, map);
	}
	
	exit(fail);
}

/*
 * This does the command line argument processing.
 */
static void
get_command_line_args(argc, argv)
	int argc;
	char **argv;
	
{

	argv++;
	
	while (--argc) {

		if ( (*argv)[0] == '-') {

			switch ((*argv)[1]) {

			case 'k': 
				dumpkeys = TRUE;
				argv++;
				break;
				
			case 'd': 

				if (argc > 1) {
					argv++;
					argc--;
					domain = *argv;
					argv++;

					if ((int)strlen(domain) > YPMAXDOMAIN) {
						(void) fprintf(stderr, err_bad_args,
						    err_bad_domainname);
						exit(1);
					}
					
				} else {
					(void) fprintf(stderr, err_usage);
					exit(1);
				}
				
				break;
				
			default: 
				(void) fprintf(stderr, err_usage);
				exit(1);
			}
			
		} else {
			
			if (!map) {
				map = *argv;
				argv++;
			} else {
				(void) fprintf(stderr, err_usage);
				exit(1);
			}
		}
	}

	if (!map) {
		(void) fprintf(stderr, err_usage);
		exit(1);
	}
}

/*
 * This dumps out the value, optionally the key, and perhaps an error message.
 */
static int
callback(status, key, kl, val, vl, fail)
	int status;
	char *key;
	int kl;
	char *val;
	int vl;
	int *fail;
{
	int e;

	if (status == YP_TRUE) {

		if (dumpkeys)
			(void) printf("%.*s ", kl, key);

		(void) printf("%.*s\n", vl, val);
		return (FALSE);
	} else {

		e = ypprot_err(status);

		if (e != YPERR_NOMORE) {
			(void) fprintf(stderr, "%s\n", yperr_string(e));
			*fail = TRUE;
		}
		
		return (TRUE);
	}
}

/*
 * This cats the map out by using the old one-by-one enumeration interface.
 * As such, it is prey to the old-style problems of rebinding to different
 * servers during the enumeration.
 */
static void
one_by_one_all(domain, map)
char *domain;
char *map;
{
	char *key;
	int keylen;
	char *outkey;
	int outkeylen;
	char *val;
	int vallen;
	int err;

	key = nullstring;
	keylen = 0;
	val = nullstring;
	vallen = 0;
	
	if (err = yp_first(domain, map, &outkey, &outkeylen, &val, &vallen) ) {

		if (err == YPERR_NOMORE) {
			exit(0);
		} else {
			(void) fprintf(stderr, err_first_failed,
			    yperr_string(err) );
			exit(1);
		}
	}

	for (;;) {

		if (dumpkeys) {
			(void) printf("%.*s ", outkeylen, outkey);
		}

		(void) printf("%.*s\n", vallen, val);
		free(val);
		key = outkey;
		keylen = outkeylen;
		
		if (err = yp_next(domain, map, key, keylen, &outkey, &outkeylen,
		    &val, &vallen) ) {

			if (err == YPERR_NOMORE) {
				break;
			} else {
				(void) fprintf(stderr, err_next_failed,
				    yperr_string(err) );
				exit(1);
			}
		}

		free(key);
	}
}
