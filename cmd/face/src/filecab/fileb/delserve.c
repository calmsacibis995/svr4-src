/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)face:src/filecab/fileb/delserve.c	1.4"

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "wish.h"

main(argc,argv)
int argc;
char *argv[];
{
	FILE *fp, *nfp;
	char *home, *getenv(), *mktemp(), *penv, *fname;
	char *mname, item[BUFSIZ], buf[BUFSIZ]; 
	char hpath[PATHSIZ], spath[PATHSIZ], tpath[PATHSIZ];
	int comp_len, found = 0;

	if (argc < 2) {
		fprintf(stderr,"Arguments invalid\n");
		return FAIL;
	}

	/* Initialize arguments needed to delete installation script */
	mname=argv[1];
	penv=argv[2];

	home=getenv(penv);
	if (strcmp(penv,"HOME") == 0)
		sprintf(spath, "%s/pref/services",home);
	else
		sprintf(spath,"%s/lib/services",home);

	sprintf(item,"`echo 'name=\"%s\"';",mname);
	comp_len=strlen(item);

	/* Update the service file */
	sprintf(tpath, "/tmp/servXXXXXX");
	mktemp(tpath);

	if ((fp=fopen(spath,"r")) == NULL) {
		fprintf(stderr, "Cannot open file %s",spath);
		return FAIL;
	}

	if ((nfp=fopen(tpath,"w+")) == NULL) {
		fprintf(stderr, "Cannot open file %s",tpath);
		return FAIL;
	}

	while(fp && (fgets(buf, sizeof(buf), fp) != NULL)) {
		if ( found )
			fputs(buf,nfp);
		else if( strncmp(buf,item,comp_len) )
			fputs(buf,nfp);
		else {
			found++;
			fname=strtok(buf,"=");
			fname=strtok(NULL,"'");
			fname=strtok(NULL,"'");
			fname=strtok(NULL,"$");
			fname=strtok(NULL,"`");
			sprintf(hpath,"%s%s",home,&fname[strlen(penv)]);
		}
	}

	rewind(nfp);
	fclose(fp);

	if ((fp=fopen(spath,"w")) == NULL) {
		fprintf(stderr, "Cannot open file %s",spath);
		return FAIL;
	}
	while(nfp && (fgets(buf, sizeof(buf), nfp) != NULL))
		fputs(buf,fp);
	fclose(fp);
	fclose(nfp);

	/* if file exists, delete it */
	if (found && (access(hpath, 00) == 0))
		unlink(hpath);
	unlink(tpath);
	
	return(SUCCESS);
}
