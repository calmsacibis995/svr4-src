/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mailx:translate.c	1.2.4.1"
#include "rcv.h"

struct name *
translate(np)
	struct name *np;
{
	struct name	*n, *t, *x;
	void	(*sigint)(), (*sigquit)();
	char	*xl = value("translate");
	char	line[LINESIZE];
	char	postmark[256];
	char	*cmd;
	FILE	*pp;
	int	i;

	if (!xl)
		return np;
	askme = 0;
	postmark[0] = 0;
	i = strlen(xl) + 1;
	for (n = np; n; n = n->n_flink)
		if (! (n->n_type & GDEL))
			i += strlen(n->n_name) + 3;
	cmd = salloc((unsigned)i);
	strcpy(cmd, xl);
	for (n = np; n; n = n->n_flink)
		if (! (n->n_type & GDEL)) {
			strcat(cmd, " \"");
			strcat(cmd, n->n_name);
			strcat(cmd, "\"");
		}
	if ((pp = npopen(cmd, "r")) == NULL) {
		perror(xl);
		senderr++;
		return np;
	}
	sigint = sigset(SIGINT, SIG_IGN);
	sigquit = sigset(SIGQUIT, SIG_IGN);
	fgets(postmark, sizeof postmark, pp);
	if (postmark[0])
		{
		postmark[strlen(postmark)-1] = 0;
		assign("postmark", postmark);
		}
	for (n = np; n; n = n->n_flink) {
		if (n->n_type & GDEL)
			continue;
		if (fgets(line, sizeof line, pp) == NULL)
			break;
		line[strlen(line)-1] = 0;
		if (!strcmp(line, n->n_name))
			continue;
		x = extract(line, n->n_type);
		n->n_type |= GDEL;
		n->n_name = "";
		if (x && !x->n_flink && strpbrk(n->n_full, "(<"))
			x->n_full = splice(x->n_name, n->n_full);
		if (x) {
			t = tailof(x);
			cat(t, n->n_flink);
			n->n_flink = NULL;
			cat(n, x);
			n = t;
		}
	}
	if (getc(pp) == 'y')
		askme++;
	if (npclose(pp) != 0 || n)
		senderr++;
	sigset(SIGINT, sigint);
	sigset(SIGQUIT, sigquit);
	return np;
}
