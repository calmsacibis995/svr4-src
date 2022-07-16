/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/getopt.c	1.23"
/*	3.0 SID #	1.2	*/
/*LINTLIBRARY*/
#ifdef __STDC__
	#pragma weak getopt = _getopt
#endif
#include "synonyms.h"
#include <unistd.h>
#include <string.h>
#define NULL	0
#define EOF	(-1)
#define ERR(s, c)	if(opterr){\
	char errbuf[2];\
	errbuf[0] = c; errbuf[1] = '\n';\
	(void) write(2, argv[0], (unsigned)strlen(argv[0]));\
	(void) write(2, s, sizeof(s) - 1);\
	(void) write(2, errbuf, 2);}

/*
 * If building the regular library, pick up the defintions from this file
 * If building the shared library, pick up definitions from opt_data.c 
 */

extern int opterr, optind, optopt;
extern char *optarg;
int _sp = 1;

int
getopt(argc, argv, opts)
int	argc;
char	*const *argv, *opts;
{
	register char c;
	register char *cp;

	if(_sp == 1)
		if(optind >= argc ||
		   argv[optind][0] != '-' || argv[optind][1] == '\0')
			return(EOF);
		else if(strcmp(argv[optind], "--") == NULL) {
			optind++;
			return(EOF);
		}
	optopt = c = (unsigned char)argv[optind][_sp];
	if(c == ':' || (cp=strchr(opts, c)) == NULL) {
		ERR(": illegal option -- ",c);
		if(argv[optind][++_sp] == '\0') {
			optind++;
			_sp = 1;
		}
		return('?');
	}
	if(*++cp == ':') {
		if(argv[optind][_sp+1] != '\0')
			optarg = &argv[optind++][_sp+1];
		else if(++optind >= argc) {
			ERR(": option requires an argument -- ",c);
			_sp = 1;
			return('?');
		} else
			optarg = argv[optind++];
		_sp = 1;
	} else {
		if(argv[optind][++_sp] == '\0') {
			_sp = 1;
			optind++;
		}
		optarg = NULL;
	}
	return(c);
}
