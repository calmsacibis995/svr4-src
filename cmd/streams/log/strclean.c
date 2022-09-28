/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)cmd-streams:log/strclean.c	1.2.2.2"
#include <stdio.h>
#include <fcntl.h>
#include <ftw.h>
#include "sys/types.h"
#include "sys/stat.h"
#include "sys/stropts.h"
#include "sys/strlog.h"

#define MSGSIZE 128
#define NSECDAY (60*60*24)
#define LOGDELAY 2
#define LOGDEFAULT "/var/adm/streams"
#define AGEDEFAULT 3
#define DIRECTORY 040000
#define ACCESS 07

static char prefix[128];
static time_t cutoff;

main(ac, av)
int ac;
char **av;
{
	int age;
	int c, errflg;
	int fd;
	struct stat stbuf;
	struct strbuf ctl, dat;
	struct log_ctl lctl;
	char msg[MSGSIZE];
	char *logname;
	extern char *optarg;
	extern int optind;
	int clean();

	logname = LOGDEFAULT;
	age = AGEDEFAULT;
	errflg = 0;

	while ((c = getopt(ac, av, "d:a:")) != EOF)
		switch (c) {

		case 'd' : 
			if (*optarg == '\0') 
				errflg++;
			else
				logname = optarg;
			break;

		case 'a' :
			if (*optarg == '\0') 
				errflg++;
			else 
				age = atoi(optarg);
			break;
		
		default :
			errflg++;
		}

	if (errflg) {
		fprintf(stderr, "Usage: strclean [-d <logdir>] [-a <age>]\n");
		exit(1);
	}

	if (age < 1) {
		fprintf(stderr, "strclean: <age> must be at least 1\n");
		exit(2);
	}

	if ((stat(logname, &stbuf) < 0) || !(stbuf.st_mode & DIRECTORY)) {
		fprintf(stderr, "strclean: %s not a directory\n", logname);
		exit(3);
	}

	if (access(logname, ACCESS) < 0) {
		fprintf(stderr, "strclean: cannot access directory %s\n", logname);
		exit(4);
	}

	cutoff = time((long*)0) - age * NSECDAY;

	ctl.len = sizeof(struct log_ctl);
	ctl.maxlen = sizeof(struct log_ctl);
	ctl.buf = (caddr_t)&lctl;
	lctl.level = 0;
	lctl.flags = SL_ERROR|SL_NOTIFY;
	dat.buf = msg;
	dat.maxlen = MSGSIZE;
	sprintf(dat.buf, 
		"strclean - removing log files more than %d days old", age);
	dat.len = strlen(dat.buf) + 1;

	if ((fd = open("/dev/log", O_RDWR)) >= 0) {
		putmsg(fd, &ctl, &dat, 0);
		close(fd);
		sleep(LOGDELAY);
	}

	strcpy(prefix, logname);
	strcat(prefix, "/error.");

	ftw(logname, clean, 1);
}

/*
 * clean out all files in the log directory prefixed by 'prefix'
 * and that are older than 'cutoff' (these are globals above).
 */
clean(name, stp, info)
char *name;
struct stat *stp;
int info;
{
	if (info != FTW_F) return(0);

	if (strncmp(name, prefix, strlen(prefix)) == 0) {
	    if (stp->st_mtime < cutoff) {
		if (unlink(name) < 0) 
			fprintf(stderr, "strclean: unable to unlink file %s\n", name);
	    }
	}
	return(0);
}

