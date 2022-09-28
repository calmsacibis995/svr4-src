/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:smtp/src/tosmtp.c	1.4.3.1"
#include <stdio.h>
#include "s_string.h"
#include "header.h"
#include <ctype.h>
#include "addrformat.h"

/* imported */
extern int to822();
extern char *sysname_read();
extern char *optarg;
extern int optind;
extern void exit();
extern char *convertaddr();
extern char *UPASROOT;
int debug = 0;

setalarm(limit, message)
	char *message;
{
	/* No-op */
}

main(argc, argv)
	int argc;
	char **argv;
{
	int c;
	char *helohost = 0;
	char *domain = 0;
	int filter = 0;
	int noto = 0;
	int unixf = 0;
	string *cmd = s_new();
	string *sender = s_new();
	string *rcvr = s_new();
	char *dest;
	FILE *fp = stdout;

	while ((c = getopt(argc, argv, "uH:d:fn")) != EOF) {
		switch (c) {
		case 'H':
			helohost = optarg;
			break;
		case 'd':
			domain = optarg;
			break;
		case 'f':
			filter = 1;
			break;
		case 'n':
			noto = 1;
			break;
		case 'u':
			unixf = 1;
			break;
		default:
			usage();
		}
	}

	if(argc-optind < 3)
		usage();

	/* get sender - make sure there's at least one hop */
	if(strchr(argv[optind], '!')==0){
		s_append(sender, sysname_read());
		s_append(sender, "!");
	}
	s_append(sender, argv[optind++]);

	/* get destination address */
	dest = argv[optind++];

	/* get rcvrs */
	if (!filter) {
		s_append(cmd, "exec ");
		s_append(cmd, UPASROOT);
		s_append(cmd, "/smtp ");
		if (helohost) {
			s_append(cmd, "-H ");
			s_append(cmd, helohost);
			s_append(cmd, " ");
		}
		if (domain) {
			s_append(cmd, "-d ");
			s_append(cmd, domain);
			s_append(cmd, " ");
		}
		s_append(cmd, s_to_c(sender)); /* sender */
		s_append(cmd, " ");
		s_append(cmd, dest); /* target host */
	}

	/* recipients */
	for (; optind < argc; optind++) {
		s_append(cmd, " ");
		s_append(cmd, argv[optind]);
		s_append(rcvr, " ");
		s_append(rcvr, convertaddr(argv[optind], (char *)0, PERCENT));
		if(optind+1<argc)
			s_append(rcvr, ",");
	}

	if(!filter) {
		fp = (FILE *)popen(s_to_c(cmd), "w");
		if (fp == NULL)
			exit(1);
	}

	if(unixf) {
		char buf[512];
		/*
		 *  just pass message as is (no conversion)
		 */
		while(fgets(buf, sizeof(buf), stdin) != NULL)
			fputs(buf, fp);
	} else {
		/*
		 *  convert to 822 format
		 */
		to822(fputs, stdin, fp, s_to_c(sender), domain, noto?0:s_to_c(rcvr));
	}

	if (!filter)
		switch(pclose(fp)){
		case 0:
			break;
		case -1:
			fprintf(stderr, "no process to wait for!\n");
			exit(1);
		default:
			exit(1);
		}
	exit(0);
}

usage()
{
	fputs("usage:	tosmtp [-f] [-H helohost] [-d domain] from destination to1 to2 ...\n", stderr);
	exit(1);
}
