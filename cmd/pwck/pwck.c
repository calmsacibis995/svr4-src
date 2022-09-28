/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)pwck:pwck.c	1.9"

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/signal.h>
#include	<sys/sysmacros.h>
#include	<sys/stat.h>
#include	<stdio.h>
#include	<ctype.h>
#define	ERROR1	"Too many/few fields"
#define ERROR2	"Bad character(s) in logname"
#define ERROR2a "First char in logname not lower case alpha"
#define ERROR2b "Logname field NULL"
#define ERROR3	"Logname too long/short"
#define ERROR4	"Invalid UID"
#define ERROR5	"Invalid GID"
#define ERROR6	"Login directory not found"
#define ERROR6a	"Login directory null"
#define	ERROR7	"Optional shell file not found"

int eflag, code=0;
int badc;
char buf[512];

main(argc,argv)

int argc;
char **argv;

{
	int delim[512];
	char logbuf[80];
	FILE *fptr;
	int error();
	struct	stat obuf;
	uid_t uid;
	gid_t gid;
	int len;
	register int i, j, colons;
	char *pw_file;

	if(argc == 1) pw_file="/etc/passwd";
	else pw_file=argv[1];

	if((fptr=fopen(pw_file,"r"))==NULL) {
		fprintf(stderr,"cannot open %s\n",pw_file);
		exit(1);
	}

	while(fgets(buf,512,fptr)!=NULL) {

		colons=0;
		badc=0;
		uid=gid=0;
		eflag=0;

	/*  Check number of fields */

		for(i=0 ; buf[i]!=NULL; i++) {
			if(buf[i]==':') {
				delim[colons]=i;
				++colons;
			}
		delim[6]=i;
		delim[7]=NULL;
		}
		if(colons != 6) {
			error(ERROR1);
			continue;
		}

	/*  Check that first character is alpha and rest alphanumeric  */

		if(!(islower(buf[0]))) {
			error(ERROR2a);
		}
		if(buf[0] == ':') {
			error(ERROR2b);
		}
		for(i=0; buf[i]!=':'; i++) {
			if(islower(buf[i]));
			else if(isdigit(buf[i]));
			else ++badc;
		}
		if(badc > 0) {
			error(ERROR2);
		}

	/*  Check for valid number of characters in logname  */

		if(i <= 0  ||  i > 8) {
			error(ERROR3);
		}

	/*  Check that UID is numeric and <= MAXUID  */

		len = (delim[2]-delim[1])-1;
		if ( (len > 5) || (len < 1) ) {
			error(ERROR4);
		}
		else {
		    for (i=(delim[1]+1); i < delim[2]; i++) {
			if(!(isdigit(buf[i]))) {
				error(ERROR4);
				break;
			}
			uid = uid*10 + (uid_t)((buf[i])-'0');
		    }
		    if(uid > MAXUID  ||  uid < 0) {
			error(ERROR4);
		    }
		}

	/*  Check that GID is numeric and <= MAXUID  */

		len = (delim[3]-delim[2])-1;
		if ( (len > 5) || (len < 1) ) {
			error(ERROR5);
		}
		else {
		    for(i=(delim[2]+1); i < delim[3]; i++) {
			if(!(isdigit(buf[i]))) {
				error(ERROR5);
				break;
			}
			gid = gid*10 + (gid_t)((buf[i])-'0');
		    }
		    if(gid > MAXUID  ||  gid < 0) {
			error(ERROR5);
		    }
		}

	/*  Stat initial working directory  */

		for(j=0, i=(delim[4]+1); i<delim[5]; j++, i++) {
			logbuf[j]=buf[i];
		}
		if((stat(logbuf,&obuf)) == -1) {
			error(ERROR6);
		}
		if(logbuf[0] == NULL) { /* Currently OS translates */
			error(ERROR6a);   /*  "/" for NULL field */
		}
		for(j=0;j<80;j++) logbuf[j]=NULL;

	/*  Stat of program to use as shell  */

		if((buf[(delim[5]+1)]) != '\n') {
			for(j=0, i=(delim[5]+1); i<delim[6]; j++, i++) {
				logbuf[j]=buf[i];
			}
			if(strcmp(logbuf,"*") == 0)  {  /* subsystem login */
				continue;
			}
			if((stat(logbuf,&obuf)) == -1) {
				error(ERROR7);
			}
			for(j=0;j<80;j++) logbuf[j]=NULL;
		}
	}
	fclose(fptr);
	exit(code);
}
/*  Error printing routine  */

error(msg)

char *msg;
{
	if(!(eflag)) {
		fprintf(stderr,"\n%s",buf);
		code = 1;
		++eflag;
	}
	if(!(badc)) {
	fprintf(stderr,"\t%s\n",msg);
	return;
	}
	else {
	fprintf(stderr,"\t%d %s\n",badc,msg);
	badc=0;
	return;
	}
}
