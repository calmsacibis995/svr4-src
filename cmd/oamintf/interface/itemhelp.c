/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oamintf:interface/itemhelp.c	1.1"
#include <stdio.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/types.h>

#define WITHCOLON 2
#define NOCOLON   4
#define NONE      0
#define HELPSIZE  12
#define FGETSIZE  77
#define ABSIZE   8

int	aflag;   /* abstract */
int	gflag;   /* rows for abstract */
int	tflag;   /* title */
int	lflag;   /* rows for text */

char*   helpitem; /* item name for which help is being requested */
char*   helpfile; /* Help file */
char*   helpfile2;
FILE* 	file;


	/*
	 * This C program is to replace a shell file (itemhelp.sh) with 
	 * the same * functionalities. It is intended to improve OA&M interface
	 * performance.
	 */

main(argc, argv)
register argc;
char *argv[];
{

	register	c;
	extern int	optind;

	if ( argc < 2 ){
		printf("Usage: itemhelp [-a]|[-t]|[-g]|[-l] helpfile\n");
		exit(2);
	}
	while((c=getopt(argc, argv, "atgl")) != -1)
		switch(c) {
		case 'a':
			aflag++;
			break;
		case 't':
			tflag++;
			break;
		case 'g':
			gflag++;
			break;
		case 'l':
			lflag++;
			break;
		}
	if((optind > argc)) {
		printf("Usage: itemhelp [-a]|[-t]|[-g]|[-l] helpfile\n");
		exit(2);
	}

	helpitem = argv[optind++];
	helpfile = argv[optind++];

	/* 
	 * helpfile2 is used for main/application. the object_gen should
	 * have taken care of this. However, the object_gen was not designed
	 * to take this into consideration, leaving itemhelp to deal with it.
	 */
	if (argv[optind] != NULL)
		helpfile2 = argv[optind]; 

	/* if helpfile2 does not exists, it is probably not what we need */
	if((file = fopen(helpfile2,"r")) == 0)
		if((file = fopen(helpfile,"r")) == 0){
			printf("Can not open [ %s ]\n",helpfile);
			exit(1);
		}

	if (aflag) {
		printabs();
	}else if(tflag){
		printitle();
	}else if(gflag){
		printarow();
	}else if(lflag){
		printtrow();
	}else{
		printext();
	}
	fclose(file);
	exit();
}
int searchabs()
{
	char strbuf[FGETSIZE];
	int colonabs = 0;
	int justabs = 0;

	/* search to see if there is a item ABSTRACT or just a ABSTRACT */
	strcat(helpitem,":ABSTRACT");
	while ( fgets(strbuf, FGETSIZE, file)!= NULL ){
		if( strncmp(strbuf,"ABSTRACT",ABSIZE) ==0 ){
			justabs = 1;
		}
		if( strncmp(strbuf,helpitem, strlen(helpitem)) ==0 ){
			/* found item ABSTRACT */
			colonabs = 1;
			break;
		}
	}
	if (colonabs)
		return WITHCOLON;
	else if (justabs)
		return NOCOLON;
	else
		return NONE;
}
printabs()
{
	char sptr[FGETSIZE];
	int search;
	int found = 0;

	search = searchabs();

	if (search == NONE)
		exit();
	else if (search == NOCOLON){
		rewind(file);
		while ( fgets(sptr, FGETSIZE, file)!= NULL ){
			if( strncmp(sptr,"ABSTRACT",ABSIZE) ==0 ){
				found = 1;
				break;
			}
		}
	}
	if(found || (search & WITHCOLON)) {
		printf("\n");
		while (fgets(sptr, FGETSIZE, file)!= NULL && isspace(sptr[0]))
			printf("%s",sptr);
	}
}
printarow()
{
	char sptr[FGETSIZE];
	int search;
	int found = 0;
	int count = 0;

	search = searchabs();

	if (search == NONE)
		exit();
	else if (search == NOCOLON){
		rewind(file);
		while ( fgets(sptr, FGETSIZE, file)!= NULL ){
			if( strncmp(sptr,"ABSTRACT",ABSIZE) ==0 ){
				found = 1;
				break;
			}
		}
	}
	if(found || (search & WITHCOLON)) {
		do{
			count++;
		}while (fgets(sptr, FGETSIZE, file)!= NULL && isspace(sptr[0]));
		if(count > HELPSIZE)
			printf("%d",HELPSIZE);
		else
			printf("%d",count);
	}
}
printtrow()
{
	char sptr[FGETSIZE];
	int count = 0;
	int found = 0;
	while ( fgets(sptr, FGETSIZE, file)!= NULL ){
		if( strncmp(sptr,helpitem,strlen(helpitem)) ==0 ){
			found = 1;
			break;
		}
	}
	if(found) {
		do{
			count++;
		}while (fgets(sptr, FGETSIZE, file)!= NULL && isspace(sptr[0]));
		if(count > HELPSIZE)
			printf("%d",HELPSIZE);
		else
			printf("%d",count);
	}
}
printext()
{
	char sptr[FGETSIZE];
	int found = 0;
	while ( fgets(sptr, FGETSIZE, file)!= NULL ){
		if( strncmp(sptr,helpitem,strlen(helpitem)) ==0 ){
			found = 1;
			break;
		}
	}
	if(found) {
		printf("\n");
		while (fgets(sptr, FGETSIZE, file)!= NULL && isspace(sptr[0]))
			printf("%s",sptr);
	}

}
printitle()
{
	char sptr[FGETSIZE];
	int found = 0;
	while ( fgets(sptr, FGETSIZE, file)!= NULL ){
		if( strncmp(sptr,helpitem,strlen(helpitem)) ==0 ){
			printf("Help on %s",&sptr[strlen(helpitem)+1]);
			found = 1;
			break;
		}
	}
	if(!found)
		printf("Help on HELP");
}
