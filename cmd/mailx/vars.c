/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mailx:vars.c	1.3.5.1"

#include "rcv.h"

/*
 * mailx -- a modified version of a University of California at Berkeley
 *	mail program
 *
 * Variable handling stuff.
 */

static struct var	*lookup();
static void		vfree();

/*
 * Assign a value to a variable.
 */
void
assign(name, value)
	char name[], value[];
{
	register struct var *vp;
	register int h;

	if (name[0]=='-')
		deassign(name+1);
	else if (name[0]=='n' && name[1]=='o')
		deassign(name+2);
	else {
		h = hash(name);
		vp = lookup(name);
		if (vp == NOVAR) {
			vp = (struct var *) calloc(sizeof *vp, 1);
			vp->v_name = vcopy(name);
			vp->v_link = variables[h];
			variables[h] = vp;
		} else
			vfree(vp->v_value);
		vp->v_value = vcopy(value);
		/*
		 * for efficiency, intercept certain assignments here
		 */
		if (strcmp(name, "prompt")==0)
			prompt = vp->v_value;
		else if (strcmp(name, "debug")==0)
			debug = 1;
		if (debug) fprintf(stderr, "assign(%s)=%s\n", vp->v_name, vp->v_value);
	}
}

deassign(s)
register char *s;
{
	register struct var *vp, *vp2;
	register int h;

	if ((vp2 = lookup(s)) == NOVAR) {
		if (!sourcing) {
			printf("\"%s\": undefined variable\n", s);
			return(1);
		}
		return(0);
	}
	if (debug) fprintf(stderr, "deassign(%s)\n", s);
	if (strcmp(s, "prompt")==0)
		prompt = NOSTR;
	else if (strcmp(s, "debug")==0)
		debug = 0;
	h = hash(s);
	if (vp2 == variables[h]) {
		variables[h] = variables[h]->v_link;
		vfree(vp2->v_name);
		vfree(vp2->v_value);
		free(vp2);
		return(0);
	}
	for (vp = variables[h]; vp->v_link != vp2; vp = vp->v_link)
		;
	vp->v_link = vp2->v_link;
	vfree(vp2->v_name);
	vfree(vp2->v_value);
	free(vp2);
	return(0);
}

/*
 * Free up a variable string.  We do not bother to allocate
 * strings whose value is "" since they are expected to be frequent.
 * Thus, we cannot free same!
 */
static void
vfree(cp)
	register char *cp;
{
	if (!equal(cp, ""))
		free(cp);
}

/*
 * Copy a variable value into permanent (ie, not collected after each
 * command) space.  Do not bother to alloc space for ""
 */

char *
vcopy(str)
	char str[];
{
	register char *top, *cp, *cp2;

	if (equal(str, ""))
		return("");
	if ((top = calloc(strlen(str)+1, 1)) == NULL)
		panic("Out of memory");
	cp = top;
	cp2 = str;
	while (*cp++ = *cp2++)
		;
	return(top);
}

/*
 * Get the value of a variable and return it.
 * Look in the environment if its not available locally.
 */

char *
value(name)
	char name[];
{
	register struct var *vp;
	register char *cp;

	if ((vp = lookup(name)) == NOVAR)
		cp = getenv(name);
	else
		cp = vp->v_value;
	if (debug) fprintf(stderr, "value(%s)=%s\n", name, (cp)?cp:"");
	return(cp);
}

/*
 * Locate a variable and return its variable
 * node.
 */

static struct var *
lookup(name)
	char name[];
{
	register struct var *vp;
	register int h;

	h = hash(name);
	for (vp = variables[h]; vp != NOVAR; vp = vp->v_link)
		if (equal(vp->v_name, name))
			return(vp);
	return(NOVAR);
}

/*
 * Locate a group name and return it.
 */

struct grouphead *
findgroup(name)
	char name[];
{
	register struct grouphead *gh;
	register int h;

	h = hash(name);
	for (gh = groups[h]; gh != NOGRP; gh = gh->g_link)
		if (equal(gh->g_name, name))
			return(gh);
	return(NOGRP);
}

/*
 * Print a group out on stdout
 */
void
printgroup(name)
	char name[];
{
	register struct grouphead *gh;
	register struct mgroup *gp;

	if ((gh = findgroup(name)) == NOGRP) {
		printf("\"%s\": not a group\n", name);
		return;
	}
	printf("%s\t", gh->g_name);
	for (gp = gh->g_list; gp != NOGE; gp = gp->ge_link)
		printf(" %s", gp->ge_name);
	printf("\n");
}

/*
 * Hash the passed string and return an index into
 * the variable or group hash table.
 */

hash(name)
	char name[];
{
	register int h;
	register char *cp;

	for (cp = name, h = 0; *cp; h = (h << 2) + *cp++)
		;
	if (h < 0)
		h = -h;
	if (h < 0)
		h = 0;
	return(h % HSHSIZE);
}
