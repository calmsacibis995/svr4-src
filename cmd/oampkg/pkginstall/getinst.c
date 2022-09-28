/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oampkg:pkginstall/getinst.c	1.8.3.1"
#include <stdio.h>
#include <string.h>
#include <valtools.h>
#include <pkginfo.h>
#include "install.h"

extern struct admin 
		adm;
extern char	*pkgarch, 
		*pkgvers,
		*msgtext,
		*pkgabrv;
extern int	opresvr4,
		update,
		maxinst,
		nointeract;

extern CKMENU	*allocmenu();
extern void	free(),
		printmenu(),
		ptext(),
		progerr(),
		quit();
extern int	setitem(),
		pkginfo(),
		ckitem(),
		ckyorn();

#define MSG_UNIQ1	\
"\\nCurrent administration requires that a unique instance \
of the <%s> package be created.  However, the \
maximum number of instances of the package which may be \
supported at one time on the same system has already been met."

#define MSG_NOINTERACT	\
"\\nUnable to determine whether to overwrite an existing \
package instance, or add a new instance."

#define MSG_NEWONLY	\
"\\nA version of the <%s> package is already installed \
on this machine.  Current administration does not allow new instances \
of an existing package to be created, nor existing instances to be overwritten."

#define MSG_SAME	\
"\\nThis appears to be an attempt to install the \
same architecture and version of a package which \
is already installed.  This installation will \
attempt to overwrite this package.\\n"

#define MSG_OVERWRITE	\
"\\nCurrent administration does not allow \
new instances of an existing package to \
be created.  However, the installation \
service was unable to determine which  \
package instance to overwrite."

static char	newinst[15];
static char	*nextinst(), *prompt();

char *
getinst(info, npkgs)
struct pkginfo *info;
int	npkgs;
{
	int	i, samearch, nsamearch;
	char	*inst, *sameinst;

	if(ADM(instance, "newonly") || ADM(instance, "quit")) {
		msgtext = MSG_NEWONLY;
		ptext(stderr, msgtext, pkgabrv);
		quit(4);
	} 

	samearch = nsamearch = 0;
	sameinst  = NULL;
	for(i=0; i < npkgs; i++) {
		if(!strcmp(info[i].arch, pkgarch)) {
			samearch = i;
			nsamearch++;
		   	if(!strcmp(info[i].version, pkgvers))
				sameinst = info[i].pkginst;
		}
	}

	if(sameinst) {
		ptext(stderr, MSG_SAME);
		inst = sameinst; /* can't be overwriting a pre-svr4 package */
		update++;
	} else if(ADM(instance, "overwrite")) {
		if(npkgs == 1) 
			samearch = 0; /* use only package we know about */
		else if(nsamearch != 1) {
			/* 
			 * more than one instance of 
			 * the same ARCH is already 
			 * installed on this machine
			 */
			msgtext = MSG_OVERWRITE;
			ptext(stderr, msgtext);
			quit(4);
		}
		inst = info[samearch].pkginst;
		if(info[samearch].status == PI_PRESVR4)
			opresvr4++; /* overwriting a pre-svr4 package */
		update++;
	} else if(ADM(instance, "unique")) {
		if(maxinst >= npkgs) {
			/* too many instances */
			msgtext = MSG_UNIQ1;
			ptext(stderr, msgtext, pkgabrv);
			quit(4);
		}
		inst = nextinst();
	} else if(nointeract) {
		msgtext = MSG_NOINTERACT;
		ptext(stderr, msgtext);
		quit(5);
	} else {
		inst = prompt(info, npkgs);
		if(!strcmp(inst, "new"))
			inst = nextinst();
		else {
			update++;
			/* see if this instance is presvr4 */
			for(i=0; i < npkgs; i++) {
				if(!strcmp(inst, info[i].pkginst)) {
					if(info[i].status == PI_PRESVR4)
						opresvr4++;
					break;
				}
			}
		}
	}
	return(inst);
}

static char *
nextinst()
{
	struct pkginfo info;
	int	n;

	n = 2; /* requirements say start at 2 */

	info.pkginst = NULL;
	(void) strcpy(newinst, pkgabrv);
	while(pkginfo(&info, newinst, NULL, NULL) == 0)
		(void) sprintf(newinst, "%s.%d", pkgabrv, n++);
	return(newinst);
}

#define PROMPT0 \
"Do you want to overwrite this installed instance"

#define PROMPT1	\
"Do you want to create a new instance of this package"

#define HELP1 \
"The package you are attempting to install already \
exists on this machine.  You may choose to create a \
new instance of this package by answering 'y' to this \
prompt.  If you answer 'n' you will be asked to choose \
one of the instances which is already to be overwritten."

#define HEADER	\
"The following instance(s) of the <%s> \
package are already installed on this machine:"

#define PROMPT2	\
"Enter the identifier for the instance that you want to overwrite"

#define HELP2 \
"The package you are attempting to install already\
exists on this machine.  You may choose to overwrite\
one of the versions which is already installed by\
selecting the appropriate entry from the menu."

static char *
prompt(info, npkgs)
struct pkginfo *info;
int	npkgs;
{
	CKMENU	*menup;
	int	i, n;
	char	ans, *inst;
	char	header[256];
	char	temp[256];
	
	if(maxinst < npkgs) {
		/*
		 * the user may choose to install a completely new
		 * instance of this package
		 */
		if(n = ckyorn(&ans, NULL, NULL, HELP1, PROMPT1))
			quit(n);
		if(ans == 'y')
			return("new");
	}

	(void) sprintf(header, HEADER, pkgabrv);
	menup = allocmenu(header, CKALPHA);

	for(i=0; i < npkgs; i++) {
		(void) sprintf(temp, "%s %s\n(%s) %s", info[i].pkginst,
			info[i].name, info[i].arch, info[i].version);
		if(setitem(menup, temp)) {
			progerr("no memory");
			quit(99);
		}
	}

	if(npkgs == 1) {
		printmenu(menup);
		if(n = ckyorn(&ans, NULL, NULL, NULL, PROMPT0))
			quit(n);
		if(ans != 'y')
			quit(3);
		(void) strcpy(newinst, info[0].pkginst);
	} else {
		if(n = ckitem(menup, &inst, 1, NULL, NULL, HELP2, PROMPT2))
			quit(n);
		(void) strcpy(newinst, inst);
	}
	(void) setitem(menup, 0); /* clear resource usage */
	free(menup); /* clear resource usage */

	return(newinst);
}
