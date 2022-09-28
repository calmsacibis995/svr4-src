/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)libyp:ypmatch.c	1.2.2.1"

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
static  char sccsid[] = "@(#)ypmatch.c 1.8 88/02/08 Copyr 1985 Sun Micro";
#endif

/*
 * This is a user command which looks up the value of a key in a map
 *
 * Usage is:
 *	ypmatch [-d domain] [-k] key [key ...] mname 
 *
 * where:  the -d switch can be used to specify a domain other than the
 * default domain.  mname is a mapname.  The -k switch prints keys as 
 * well as values.
 */

#include <stdio.h>
#include <rpc/rpc.h>
#include <rpcsvc/yp_prot.h>
#include <rpcsvc/ypclnt.h>

static void get_command_line_args();
static void getdomain();
static bool match_list();
static bool match_one();
static void print_one();

extern void exit();
extern int strcmp();
extern unsigned int strlen();
extern int getdomainname();
extern void free();

static int printkeys = FALSE;
static char *domain = NULL;
static char default_domain_name[YPMAXDOMAIN];
static char *map = NULL;
static char **keys = NULL;
static int nkeys;
static char err_usage[] =
"Usage:\n\
	ypmatch [-d domain] [-k] key [key ...] mname\n\
where\n\
	mname may be either a mapname or a nickname for a map\n\
	-k prints keys as well as values.\n";
static char err_bad_args[] =
	"ypmatch:  %s argument is bad.\n";
static char err_cant_get_kname[] =
	"ypmatch:  can't get %s back from system call.\n";
static char err_null_kname[] =
	"ypmatch:  the %s hasn't been set on this machine.\n";
static char err_bad_mapname[] = "mapname";
static char err_bad_domainname[] = "domainname";

/*
 * This is the main line for the ypmatch process.
 */
main(argc, argv)
	char **argv;
{
	get_command_line_args(argc, argv);

	if (!domain) {
		getdomain();
	}

	if (!match_list())
		return(1);
	return(0);
}

/*
 * This does the command line argument processing.
 */
static void
get_command_line_args(argc, argv)
	int argc;
	char **argv;
	
{
		
	if (argc < 2) {
		(void) fprintf(stderr, err_usage);
		exit(1);
	}
	argv++;

	while (--argc > 0 && (*argv)[0] == '-') {

		switch ((*argv)[1]) {

		case 'k':
			printkeys = TRUE;
			break;

		case 'd':

			if (argc > 1) {
				argv++;
				argc--;
				domain = *argv;

				if ((int) strlen(domain) > YPMAXDOMAIN) {
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

		argv++;
	}

	if (argc < 2) {
		(void) fprintf(stderr, err_usage);
		exit(1);
	}

	keys = argv;
	nkeys = argc -1;
	map = argv[argc -1];

	if ((int) strlen(map) > YPMAXMAP) {
		(void) fprintf(stderr, err_bad_args, err_bad_mapname);
		exit(1);
	}
}

/*
 * This gets the local default domainname, and makes sure that it's set
 * to something reasonable.  domain is set here.
 */
static void
getdomain()		
{
	if (!getdomainname(default_domain_name, YPMAXDOMAIN) ) {
		domain = default_domain_name;
	} else {
		(void) fprintf(stderr, err_cant_get_kname, err_bad_domainname);
		exit(1);
	}

	if ((int) strlen(domain) == 0) {
		(void) fprintf(stderr, err_null_kname, err_bad_domainname);
		exit(1);
	}
}

/*
 * This traverses the list of argument keys.
 */
static bool
match_list()
{
	bool error;
	bool errors = FALSE;
	char *val;
	int len;
	int n = 0;

	while (n < nkeys) {
		error = match_one(keys[n], &val, &len);

		if (!error) {
			print_one(keys[n], val, len);
			free(val);
		} else {
			errors = TRUE;
		}

		n++;
	}
	
	return (!errors);
}

/*
 * This fires off a "match" request to any old yp server, using the vanilla
 * yp client interface.  To cover the case in which trailing NULLs are included
 * in the keys, this retrys the match request including the NULL if the key
 * isn't in the map.
 */
static bool
match_one(key, val, len)
	char *key;
	char **val;
	int *len;
{
	int err;
	bool error = FALSE;

	*val = NULL;
	*len = 0;
	err = yp_match(domain, map, key, (int) strlen(key), val, len);
	

	if (err == YPERR_KEY) {
		err = yp_match(domain, map, key, ((int) strlen(key) + 1),
		    val, len);
	}
		
	if (err) {
		(void) fprintf(stderr,
		    "Can't match key %s in map %s.  Reason: %s.\n", key, map,
		    yperr_string(err) );
		error = TRUE;
	}
	
	return (error);
}

/*
 * This prints the value, (and optionally, the key) after first checking that
 * the last char in the value isn't a NULL.  If the last char is a NULL, the
 * \n\0 sequence which the yp client layer has given to us is shuffled back
 * one byte.
 */
static void
print_one(key, val, len)
	char *key;
	char *val;
	int len;
{
	if (printkeys) {
		(void) printf("%s: ", key);
	}

	(void) printf("%.*s\n", len, val);
}
