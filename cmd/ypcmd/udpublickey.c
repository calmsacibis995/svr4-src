/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ypcmd:udpublickey.c	1.2.2.1"

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
 * YP updater for public key map
 */
#include <stdio.h>
#include <rpc/rpc.h>
#include <rpcsvc/ypclnt.h>
#include <sys/file.h>

extern char *malloc();

main(argc, argv)
	int argc;
	char *argv[];
{
	unsigned op;
	char name[MAXNETNAMELEN + 1];
	char key[256];
	char data[256];
	char line[256];
	unsigned keylen;
	unsigned datalen;
	FILE *rf;
	FILE *wf;
	char *fname;
	char *tmpname;
	int err;


	if (argc !=  3) {
		exit(YPERR_YPERR);
	}
	fname = argv[1];
	tmpname = malloc(strlen(fname) + 4);
	if (tmpname == NULL) {
		exit(YPERR_YPERR);
	}
	sprintf(tmpname, "%s.tmp", fname);
	
	/*
	 * Get input
	 */
	if (! scanf("%s\n", name)) {
		exit(YPERR_YPERR);	
	}
	if (! scanf("%u\n", &op)) {
		exit(YPERR_YPERR);
	}
	if (! scanf("%u\n", &keylen)) {
		exit(YPERR_YPERR);
	}
	if (! fread(key, keylen, 1, stdin)) {
		exit(YPERR_YPERR);
	}
	key[keylen] = 0;
	if (! scanf("%u\n", &datalen)) {
		exit(YPERR_YPERR);
	}
	if (! fread(data, datalen, 1, stdin)) {
		exit(YPERR_YPERR);
	}
	data[datalen] = 0;

	/*
	 * Check permission
	 */
	if (strcmp(name, key) != 0) {
		exit(YPERR_ACCESS);
	}
	if (strcmp(name, "nobody") == 0) {
		/*
		 * Can't change "nobody"s key.
		 */
		exit(YPERR_ACCESS);
	}

	/*
	 * Open files 
	 */
	rf = fopen(fname, "r");
	if (rf == NULL) {
		exit(YPERR_YPERR);	
	}
	wf = fopen(tmpname, "w");
	if (wf == NULL) {
		exit(YPERR_YPERR);
	}
	err = -1;
	while (fgets(line, sizeof(line), rf)) {
		if (err < 0 && match(line, name)) {
			switch (op) {
			case YPOP_INSERT:
				err = YPERR_KEY;
				break;
			case YPOP_STORE:
			case YPOP_CHANGE:
				fprintf(wf, "%s %s\n", key, data);	
				err = 0;
				break;
			case YPOP_DELETE:
				/* do nothing */
				err = 0;
				break;
			}	
		} else {
			fputs(line, wf);
		}
	}
	if (err < 0) {
		switch (op) {
		case YPOP_CHANGE:	
		case YPOP_DELETE:
			err = YPERR_KEY;
			break;
		case YPOP_INSERT:
		case YPOP_STORE:
			err = 0;	
			fprintf(wf, "%s %s\n", key, data);	
			break;
		}
	}
	fclose(wf);
	fclose(rf);
	if (err == 0) {
		if (rename(tmpname, fname) < 0) {
			exit(YPERR_YPERR);	
		}
	} else {
		if (unlink(tmpname) < 0) {
			exit(YPERR_YPERR);
		}
	}
	if (fork() == 0) {
		close(0); close(1); close(2);
		open("/dev/null", O_RDWR, 0);
		dup(0); dup(0);
		execl("/bin/sh", "sh", "-c", argv[2], NULL);
	}
	exit(err);
	/* NOTREACHED */
}


match(line, name)
	char *line;
	char *name;
{
	int len;

	len = strlen(name);
	return(strncmp(line, name, len) == 0 && 
		(line[len] == ' ' || line[len] == '\t'));
}
