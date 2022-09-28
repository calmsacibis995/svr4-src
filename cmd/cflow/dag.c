/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)cflow:dag.c	1.5.1.2"
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <memory.h>

#ifdef __STDC__
#include <stdlib.h>
#else
extern char *calloc();
#endif

#define	NLINK	8


typedef	struct	node {
	struct	node *n_next;
	struct	node *n_left;
	struct	node *n_right;
	char	*n_name;
	char	*n_data;
	int	n_rcnt;
	int	n_visit;
	int	n_lcnt;
	struct	link *n_link;
} node;

typedef	struct	link {
	struct	link *l_next;
	struct	node *l_node[NLINK];
} link;

static node	*first = NULL;
static node	*last = NULL;
static node	*root = NULL;

#define	isnamec(c)	(isalpha(c)||isdigit(c)||c=='_')
#define	NUL	'\0'
#define BIGINT	32767

static int lines, nodes, links, chars;
static int lineno = 0;
static int aflag = 0;
static int lvmax = BIGINT;
static char stdbuf[BUFSIZ];

static int getnum();
static int getst();
static void addlink();
static node	*getnode();
static char	*copy();
static void reader();
static void dfs();

main(argc, argv)
char *argv[];
{
	extern int optind;
	extern char *optarg;
	register node *np;
	int c;

	setbuf(stdout, stdbuf);
	while ((c = getopt(argc, argv, "ad:")) != EOF)
		switch (c) {
		case 'd':
			if ((lvmax = getnum(optarg, 10)) == 0) {
				lvmax = BIGINT;
				goto argerr;
			}
			break;
		case 'a':
			aflag = 1;
			break;
		argerr:
		default:
			(void)fprintf(stderr, "dag: bad option %c ignored\n", c);
		}
	reader(stdin);
	argc -= optind + 1;
	if (argc <= 1)
		for (np = first; np != NULL; np = np->n_next)
			{
			if (np->n_rcnt == 0)
				dfs(np, 0);
			}
	else
		while (--argc > 0)
			dfs(getnode(*++argv), 0);
	/* NOTREACHED */
}

static int
getnum(p, base)
register int base;
register char *p;
{
	register int n;

	n = 0;
	while (isdigit(*p))
		n = n * base + (*p++ - '0');
	return(n);
}

static void 
reader(fp)
register FILE *fp;
{
	register char *p1, *p2;
	int	c;
	node	*np;
	char	linebuf[BUFSIZ];
	char	*line = linebuf;

	while (getst(&line, fp)) {
		++lines;
		p1 = line;
		while (isspace(*p1))
			++p1;
		if (*p1 == NUL)
			continue;
		p2 = p1;
		while (isnamec(*p2))
			++p2;
		do {
			c = *p2;
			*p2++ = NUL;
		} while (isspace(c));
		switch (c) {

		case '=':
			np = getnode(p1);
			while (isspace(*p2))
				++p2;
			if (np->n_data != NULL) {
				(void)fprintf(stderr, "redefinition of %s\n", np->n_name);
				free(np->n_data);
			}
			np->n_data = copy(p2);
			continue;
		case ':':
			np = getnode(p1);
			while (*(p1 = p2) != NUL) {
				while (isspace(*p1))
					++p1;
				p2 = p1;
				while (isnamec(*p2))
					++p2;
				addlink(np, getnode(p1));
				if (*p2 != NUL)
					*p2++ = NUL;
			}
			continue;
		}
	}
}

static int
getst(ss, fp)
char **ss;
FILE *fp;
{
	register i, c;
	char *s = *ss;
	static int bufsiz = BUFSIZ;

	i = 0;
	while ((c = getc(fp)) != EOF) {
		if (c == '\n') {
			*s = NUL;
			return 1;
		}
		*s++ = (char) c;
		if (++i >= bufsiz) {
			*ss = (char *)realloc((char *)(*ss), (bufsiz += BUFSIZ));
			if (*ss == NULL) {
				(void) fprintf(stderr, "out of heap space\n");
				exit (1);
			}
			s = *ss + i;
		}
			
	}
	return 0;
}

static char *
copy(s)
char	*s;
{
	char	*p;

	p = calloc(sizeof(char), (unsigned)(strlen(s)+2));
	if (p == NULL) {
		(void)fprintf(stderr, "too many characters\n");
		exit(1);
	}
	(void)strcpy(p, s);
	chars += strlen(p);
	return p;
}

static node *
getnode(name)
char	*name;
{
	register i;
	register node *np, **pp;

	pp = &root;
	while ((np = *pp) != NULL) {
		i = strcmp(name, np->n_name);
		if (i > 0)
			pp = &np->n_right;
		else if (i < 0)
			pp = &np->n_left;
		else
			return np;
	}
	*pp = (node *)calloc(sizeof(node), 1);
	if ((np = *pp) == NULL) {
		(void)fprintf(stderr, "too many nodes");
		exit(1);
	}
	++nodes;
	np->n_name = copy(name);
	np->n_data = NULL;
	if (first == NULL)
		first = np;
	if (last != NULL)
		last->n_next = np;
	last = np;
	np->n_next = NULL;
	np->n_left = np->n_right = NULL;
	np->n_rcnt = np->n_lcnt = np->n_visit = 0;
	np->n_link = NULL;
	return np;
}

static void
addlink(np, rp)
node	*np, *rp;
{
	register i, j;
	link	*lp, **pp;
	i = j = 0;
	pp = &np->n_link;
	while ((lp = *pp) != NULL && i < np->n_lcnt) {
		if ((rp == lp->l_node[j]) && !aflag)
			return; 
		if (++j >= NLINK) {
			j = 0;
			pp = &lp->l_next;
		}
		++i;
	}
	if (lp == NULL) {
		*pp = (link *)calloc(sizeof(link), 1);
		if ((lp = *pp) == NULL) {
			(void)fprintf(stderr, "too many links\n");
			exit(1);
		}
		++links;
		lp->l_next = NULL;
	}
	lp->l_node[j] = rp;
	if (np != rp)
		++rp->n_rcnt;
	++np->n_lcnt;
	return;
}

static void 
dfs(np, lv)
register
node *np;
{
	register i, j;
	link *lp;

	if (np == NULL || lv > lvmax)
		return;
	i = 0;
	(void)printf("%d\t",++lineno);
	while (i++ < lv)
		(void)putchar('\t');
	(void)printf("%s: ", np->n_name);
	if (np->n_visit > 0) {
		(void)printf("%d\n", np->n_visit);
		return; 
	}
	if (np->n_data != NULL)
		(void)printf("%s\n", np->n_data);
	else
		(void)printf("<>\n");
	np->n_visit = lineno;
	i = j = 0;
	lp = np->n_link;
	while (i < np->n_lcnt && lp != NULL) {
		dfs(lp->l_node[j], lv+1);
		if (++j >= NLINK) {
			j = 0;
			lp = lp->l_next;
		}
		++i;
	}
}
