/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)face:src/filecab/fileb/listserv.c	1.8"

#include <stdio.h>
#include <string.h>
#include "wish.h"

main(argc,argv)
int argc;
char *argv[];
{
	FILE *fp;
	char *home, *getenv(), *label, *name, *penv, *fname;
	char tpath[PATHSIZ], hpath[PATHSIZ], buf[BUFSIZ], path[PATHSIZ], *opt;
	int flag=0, cond=0, dos=0;
	int app_type();

	penv=argv[argc-1];
	while(--argc > 0 && (*++argv)[0] == '-')
		for(opt=argv[0]+1; *opt != '\0'; opt++)
		switch(*opt) {
			case 'd':
				flag=1;
				break;
			case 'l': /* used to create the rmenu */
				flag=2;
				break;
			case 'm':
				flag=3;
				break;
			case 'p':
				dos=1;
				break;
			default:
				break;
		}
	home=getenv(penv);

	if (strcmp(penv,"HOME") == 0) {
		sprintf(hpath, "%s/pref/services",home);
		sprintf(tpath,"$VMSYS/OBJECTS/%s",dos?"dos":"programs");
	}
	else {
		sprintf(hpath, "%s/lib/services",home);
		sprintf(tpath,"$OBJ_DIR");
	}

	if ((fp=fopen(hpath,"r")) == NULL) {
		printf("init=`message No Programs Installed`false\n");
		exit(FAIL);
	}

	while(fp && (fgets(buf,BUFSIZ,fp) != NULL)) {
		if (*buf == '\n' || *buf == '#' )
			continue;

		label = strtok(buf,"=");

		if (! strcmp(label,"name")) {
			name=strtok(NULL,"\n");
			sprintf(path,"%s/bin/%s.ins",home,name);
		} else if (! strcmp(label,"`echo 'name")) {
			name=strtok(NULL,"'");
			fname=strtok(NULL,"=");
			fname=strtok(NULL,"$");
			if (! strncmp(fname,"OPEN",4))
				continue;
			fname=strtok(NULL,"`");
			sprintf(path,"%s%s",home,&fname[strlen(penv)]);
		} else
			continue;
		if ( access(path,00)==0 && app_type(path,dos) ) {
			cond=1;
			if (flag == 2)  {
				printf("%s\n",name);
				continue;
			}
			printf("name=%s\n",name);
			printf("lininfo=\"%s\"\n",path);
			if (flag == 1 )
				printf("action=OPEN TEXT %s/Text.conf %s \"$LININFO\" \"%s\" `getfrm`\n",tpath,name,penv);
			else if (flag == 3 )
				printf("action=OPEN FORM %s/Form.mod %s \"$LININFO\" \"%s\" `getfrm`\n",tpath,name,penv);
			else 
				printf("action=`run %s%s`nop\n",dos?"-n ":"",path);
		}
	}
	if (!cond) {
		if ( dos )
			printf("init=`message No MS-DOS Programs Installed`false\n");
		else
			printf("init=`message No Programs Installed`false\n");
		exit(FAIL);
	}
	exit(SUCCESS);
}

int
app_type(path,dos)
char *path;
int dos;
{
	FILE *fp;
	char buf[BUFSIZ];
	int retval;

	retval = dos?FALSE:TRUE;

	if ((fp=fopen(path,"r")) == NULL)
		return(retval);

	while(fp && (fgets(buf,BUFSIZ,fp) != NULL)) {
		if ( *buf != '#' )
			continue;

		if (! strcmp(buf,"#dos\n")) {
			retval = dos?TRUE:FALSE;
			break;
		}

		if (! strcmp(buf,"#unix\n")) {
			retval = dos?FALSE:TRUE;
			break;
		}
	}

	(void)fclose(fp);

	return(retval);
}
