/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)fmli:sys/putaltenv.c	1.5"

#include	<stdio.h>
#include	<string.h>
#include	"wish.h"
#include	"var_arrays.h"
#include	"moremacros.h"

void
copyaltenv(an_env, another)
char	**an_env;
register char	***another;
{
	register int	i;
	int	lcv;

	lcv = array_len(an_env);
	for (i = 0; i < lcv; i++)
		putaltenv(another, an_env[i]);
}

/* LES: never called
void
dumpenv(fp, the_env)
FILE	*fp;
register char	**the_env;
{
	int	i;
	int	lcv;

	lcv = array_len(the_env);
	for (i = 0; i < lcv; i++) {
		fputs(the_env[i], fp);
		putc('\n', fp);
	}
}
*/
		 
int
delaltenv(the_env, name)
register char	***the_env;
char	*name;
{
	register int	i;

	if ((i = findaltenv(*the_env, name)) != FAIL) {
		free((*the_env)[i]);
		*the_env = (char **) array_delete(*the_env, i);
		return SUCCESS;
	}
	return FAIL;
}

int
putaltenv(the_env, str)
register char	***the_env;
char	*str;
{
	register int	i;
	register char	*p;
	char	*hold;

	hold = strsave(str);
	if ((p = strchr(hold, '=')) == NULL) {
		if (hold) /* ehr3 */
			free(hold); 
		return(FAIL);
	}
	*p = '\0';
	delaltenv(the_env, hold);
	if ((i = findaltenv(*the_env, hold)) != FAIL) {
		*p = '=';
		if ((*the_env)[i]) /* ehr3 */
			free((*the_env)[i]);
		(*the_env)[i] = hold;
	}
	else {
		*p = '=';
		var_append(char *, *the_env, &hold);
	}
	return SUCCESS;
}

/* versions to work on Altenv */

extern char **Altenv;

/* LES: replace with MACROS
copyAltenv(an_env)
char **an_env;
{
	copyaltenv(an_env, &Altenv);
}

int
delAltenv(name)
char *name;
{
	return(delaltenv(&Altenv, name));
}

int
putAltenv(str)
char *str;
{
	return(putaltenv(&Altenv, str));
}
*/
