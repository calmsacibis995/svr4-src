/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)face:src/filecab/fileb/mnucheck.c	1.2"

#include	<stdio.h>
#include 	<string.h>
#include 	<ctype.h>
#include	"wish.h"


#define FAILURE	4
#define EXISTS  2
#define INVALID 3
#define SUCCESS 0

extern char *optarg;
char	*string=NULL;
char	*name=NULL;

main(argc,argv)
int argc;
char *argv[];
{

	int     ret;
	int 	opt;
	char	hpath[PATHSIZ], path[PATHSIZ], *home, *vmsys, *str, *getenv();
	FILE    *fp, *nfp;

	home=getenv("HOME");
	vmsys=getenv("VMSYS");

	while((opt=getopt(argc,argv,"s:n:")) != EOF)
		switch(opt) {
		case 's':
			string=optarg;
			break;
		case 'n':
			name=optarg;
			break;
		}

	if (string == NULL || strlen(string) == 0)
		return(INVALID);

	str = string;
	for (; *str != '\0'; str++)
		if ( ! isprint(*str))
			return(FAILURE);

	if (strcmp(string,name) == 0)
		return(SUCCESS);

	sprintf(hpath,"%s/pref/services",home);
	sprintf(path, "%s/lib/services",vmsys);

	if(access(hpath, 00) == 0) {
		if ((fp=fopen(hpath, "r")) == NULL) {
			fprintf(stderr, "Cannot open file %s",hpath);
			return(FAILURE);
		}

		if (mread(fp)) {
			fclose(fp);
			return(EXISTS);
		}

	}
	if(access(path, 00) == 0) {
		if ((nfp=fopen(path,"r")) == NULL) {
			fprintf(stderr, "Cannot open file %s",path);
			return(FAILURE);
		}

		if (mread(nfp)) {
			fclose(nfp);
			return(EXISTS);
		}
	}
	fclose(fp);
	fclose(nfp);
	return(SUCCESS);
}

mread(fp)
FILE *fp;
{
	char buf[BUFSIZ], *label, *mname;

	while(fp && (fgets(buf, BUFSIZ,fp) != NULL)) {
		if (*buf == '\n' || *buf == '#' )
			continue;

		label=strtok(buf, "=");
		if (strcmp(label,"`echo 'name") != 0)
			continue;

		mname=strtok(NULL,"\"");
		if (strcmp(mname,string) == 0)
			return(EXISTS);
	}
	return(SUCCESS);
}
