/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)libyp:getlist.c	1.3.2.1"

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
#include <stdio.h>
#include "ypsym.h"

extern void free();
extern char *strdup();

/*
 * Add a name to the list
 */
static listofnames *
newname(str)
char *str;
{
	listofnames *it;
	char *copy;

	if (str==NULL) return(NULL);
	copy=strdup(str);
	if (copy==NULL) return(NULL);
	it=(listofnames *) malloc(sizeof(listofnames));
	if (it==NULL){
		free(copy);
		return(NULL);
	}
	it->name=copy;
	it->nextname=NULL;
	return(it);
}

/*
 * Assemble the list of names
 */
listofnames *
names (filename, count)
int count;
char *filename;
{
	listofnames *list;
	listofnames *end;
	listofnames *nname;
	FILE *fyle;
	char line[256];
	char name[256];

	fyle=fopen(filename,"r");
	if (fyle==NULL) {
		free(filename);
		return(NULL);
	}
	list=NULL;
	while (fgets(line,sizeof(line),fyle)) {
		if (line[0]=='#') continue;
		if (line[0]=='\0') continue;
		if (line[0]=='\n') continue;
		nname = newname(line);
		if (nname) {
			if (list==NULL) {
					list = nname;
					end = nname;
			} else {
				end->nextname = nname;
				end = nname;
			}
			count++;
		} else
			fprintf(stderr,"file %s bad malloc %s\n",filename,name);
	}
	fclose(fyle);
	return(list);
}

void
free_listofnames(list)
listofnames *list;
{
	listofnames *next=(listofnames *)NULL;

	for(; list;list=next)
		{
		next=list->nextname;
		if (list->name) free(list->name);
		free((char *)list);	
		}
}


#ifdef MAIN
main(argc,argv)
char **argv;
{
	listofnames *list;
	list=names(argv[1]);
#ifdef DEBUG
	print_listofnames(list);
#endif
	free_listofnames(list);
#ifdef DEBUG
	printf("Done\n");
#endif
}
#endif

#ifdef DEBUG
void
print_listofnames(list)
listofnames *list;
{
	if (list==NULL) printf("NULL\n");
	for(; list;list=list->nextname)
		printf("%s\n",list->name);
}
#endif
