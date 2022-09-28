/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)face:src/filecab/fileb/modserv.c	1.3"

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "wish.h"

main(argc,argv)
int argc;
char *argv[];
{
	FILE *fp;
	char *home, *getenv();
	char *term, *mname, *appath, *dir, *penv, *oterm, *oname, *oapath, *odir; 
	char hpath[PATHSIZ], path[PATHSIZ], command[BUFSIZ];

	if (argc < 9) {
		fprintf(stderr,"Arguments invalid");
		return FAIL;
	}

	/* Initialize arguments needed to create installation script */
	term=argv[1];
	mname=argv[2];
	appath=argv[3];
	dir=argv[4];
	penv=argv[5];
	oterm=argv[6];
	oname=argv[7];
	oapath=argv[8];
	odir=argv[9];

	home=getenv(penv);

	if(strcmp(term,oterm) == 0 && strcmp(appath,oapath) == 0 && strcmp(dir,odir) == 0 ){
		sprintf(hpath, "%s/bin/%s.ins",home,mname);
		sprintf(path, "%s/bin/%s.ins",home,oname);

		/* if file exist copy it to old.<name> */
		if (strcmp(mname,oname) != 0)  {
			if (access(path, 00) == 0)  {
				copyfile(path,hpath);
				chmod(hpath, 0755);
	        	}
			sprintf(command, "$VMSYS/bin/delserve \"%s\" \"%s\"",oname,penv);
			system(command);
		}
	}
	else {
		sprintf(hpath, "%s/bin/%s.ins",home,mname);
		sprintf(path, "%s/bin/old.%s.ins",home,mname);
		/* if file exist copy it to old.<name> */
		if (access(hpath, 00) == 0) 
			copyfile(hpath,path);

		if ((fp=fopen(hpath,"w+")) == NULL) {
			fprintf(stderr,"Cannot open file");
			return FAIL;
		}

		/* Create the Shell script the application is going to be used */
		fprintf(fp,"TERM=%s;export TERM\n",term);
		fprintf(fp,"cd %s\n",dir);
		fprintf(fp,"%s\n",appath);
		fclose(fp);
		chmod(hpath, 0755);

	}
	/* Update the User's service file */
	if(strcmp(mname,oname) != 0) {
		if (strcmp(penv, "HOME") == 0)
			sprintf(path, "%s/pref/services",home);
		else
			sprintf(path, "%s/OBJECTS/Menu.programs",home);

		if ((fp=fopen(path,"a")) == NULL) {
			fprintf(stderr, "Cannot open file");
			return FAIL;
		}
		fprintf(fp,"\n");
		fprintf(fp,"name=%s\n",mname);
		if (strcmp(penv, "HOME") == 0)
			fprintf(fp,"action=`run $HOME/bin/%s.ins`\n",mname);
		else
			fprintf(fp,"action=`run $VMSYS/bin/%s.ins`\n",mname);
		fclose(fp);
	}
}




/*
 * copy a file
 */
FILE *
cpfile(from, to)
char	*from;
char	*to;
{
	register int	c;
	register FILE	*src;
	register FILE	*dst;

	if ((src = fopen(from, "r")) == NULL)
		return NULL;
	if ((dst = fopen(to, "w+")) == NULL) {
		fclose(src);
		return NULL;
	}
	while ((c = getc(src)) != EOF)
		putc(c, dst);
	if (ferror(src)) {
		fclose(src);
		fclose(dst);
		unlink(to);
		return NULL;
	}
	fclose(src);
	return dst;
}

copyfile(from, to)
char *from;
char *to;
{
	FILE *fp;

	if (fp = cpfile(from, to)) {
		fclose(fp);
		return(0);
	}
	return(-1);
}
