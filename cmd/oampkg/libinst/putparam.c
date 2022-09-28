/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oampkg:libinst/putparam.c	1.2.4.1"

#include <stdio.h>
#include <string.h>

#define ERR_MEMORY	"memory allocation failure, errno=%d"

extern char	**environ;
extern int	errno;

extern void	*calloc(), 
		*realloc();
extern void	progerr(),
		quit(),
		free();

#define MALSIZ	64

void
putparam(param, value)
char	*param;
char	*value;
{
	char	*pt;
	int	i, n;

	if(environ == NULL) {
		environ = (char **) calloc(MALSIZ, sizeof(char *));
		if(environ == NULL) {
			progerr(ERR_MEMORY, errno);
			quit(99);
		}
	}
	n = strlen(param);
	for(i=0; environ[i]; i++) {
		if(!strncmp(environ[i], param, n) && (environ[i][n] == '=')) {
			free(environ[i]);
			break;
		}
	}

	pt = (char *) calloc(strlen(param)+strlen(value)+2, sizeof(char));
	if(pt == NULL) {
		progerr(ERR_MEMORY, errno);
		quit(99);
	}
	(void) sprintf(pt, "%s=%s", param, value);
	if(environ[i]) {
		environ[i] = pt;
		return;
	}

	environ[i++] = pt;
	if((i % MALSIZ) == 0) {
		environ = (char **) realloc((void *)environ, 
			(i+MALSIZ)*sizeof(char *));
		if(environ == NULL) {
			progerr(ERR_MEMORY, errno);
			quit(1);
		}
	}
	environ[i] = (char *)NULL;
}

