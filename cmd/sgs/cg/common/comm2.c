/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nifg:cg/common/comm2.c	1.12"
#include <string.h>
#include "mfile2.h"
#include <malloc.h>

# ifndef EXIT
# define EXIT exit
# endif

int nerrors = 0;  /* number of errors */

#ifdef STATSOUT
# undef INI_TREESZ
# define INI_TREESZ 1
#endif

#define	CLUSTERSIZE INI_TREESZ		/* size of a cluster */

/* number of expansions before panic */
#ifndef	WATCHDOG
#define WATCHDOG ((1000/CLUSTERSIZE) <= 0 ? 1 : (1000/CLUSTERSIZE))
#endif

/*Number of the lowest node*/
/*Only made nonzero by cg*/
#ifndef	FIRST_NODENO
#define	FIRST_NODENO 0
#endif

struct nNODEcluster {
    struct nNODEcluster * next;
    nNODE nNODEs[CLUSTERSIZE];
};

typedef struct nNODEcluster NCLUST;

static NCLUST basecluster;	/* initial, statically allocated hunk */

#ifndef STATSOUT
static
#endif
	int nclusters = 0;		/* number of clusters dynamically alloc. */
int watchdog;

static nNODE *lastfree;  /* pointer to last free node; (for allocator) */
static NCLUST * lastcluster;		/* last cluster on chain */
static NCLUST * curcluster;		/* cluster currently allocating from */
static void freestr();


void
tinit()
{
	 /* initialize expression tree search */
	register nNODE *p;
	register NCLUST * clp;
	register int nodeno = FIRST_NODENO;

	for (clp = &basecluster; clp; clp = clp->next) {
	    for (p = clp->nNODEs; p < &clp->nNODEs[CLUSTERSIZE]; ++p) {
		p->node.in.op = FREE;
		p->_node_no = nodeno++;
	    }
	    lastcluster = clp;
	}
	curcluster = &basecluster;
	lastfree = &basecluster.nNODEs[CLUSTERSIZE-1];

}

# define TNEXT(p) (p == last_in_clust ? &curcluster->nNODEs[0] : p+1)

NODE *
talloc()
{
	register nNODE *p, *q;
	register nNODE * last_in_clust;
	NCLUST * startcluster = curcluster;
	NCLUST * newclust;
	int nodeno;

	do {
	    last_in_clust = &curcluster->nNODEs[CLUSTERSIZE-1];
	    q = lastfree;
	    p = TNEXT(q);
	    do {
		if( p->node.in.op == FREE ) {
/*		    fprintf(stderr, "## alloc %d %#x\n", node_no(p), p); */
		    p->node.in.strat = 0;
		    p->node.in.name = 0;
		    return((NODE *) (lastfree=p));
		}
		p = TNEXT(p);
	    } while( p != q );

	    /* ran out in current cluster; wrap around */
	    if (!(curcluster = curcluster->next)) curcluster = &basecluster;
	    lastfree = &curcluster->nNODEs[CLUSTERSIZE-1];
	} while ( curcluster != startcluster);

	++nclusters;			/* count a new cluster */
	if (++watchdog > WATCHDOG)
	    cerror( "out of tree space; simplify expression");

/*	fprintf(stderr, "new cluster %d\n", nclusters); */
	/* add new cluster; link on list */
	nodeno = lastcluster->nNODEs[CLUSTERSIZE-1]._node_no + 1;
	newclust = (NCLUST *) malloc(sizeof(NCLUST));
	if (! newclust)
	    cerror("can't allocate nodes");
/*	printf("node malloc() returns %#lx\n", newclust); */
	lastcluster->next = newclust;
	newclust->next = (NCLUST *) 0;
	lastcluster = newclust;
	curcluster = newclust;
	/* initialize new cluster */
	for (p = newclust->nNODEs; p < &newclust->nNODEs[CLUSTERSIZE]; ++p) {
	    p->node.in.op = FREE;
	    p->_node_no = nodeno++;
	}
/*	fprintf(stderr, "## alloc %d %#x\n", node_no(newclust->nNODEs),
**		newclust->nNODEs);
*/
	p = lastfree = newclust->nNODEs;
	p->node.in.strat = 0;
	p->node.in.name = 0;
	return( (NODE *) p );
}
int
tcheck()
{
	 /* ensure that all nodes have been freed */
	register nNODE *p;
	register NCLUST * clp;
	int count = 0;

	if( !nerrors ) {
	    for (clp = &basecluster; clp; clp = clp->next) {
		for (p = clp->nNODEs; p < &clp->nNODEs[CLUSTERSIZE]; ++p) {
		    if( p->node.in.op != FREE )
			count++;
		}
	    }
	}
	else
	    tinit();
	freestr();
	return count;
}
void
tshow()
{
		/*print all nodes that have not been freed*/
	register nNODE *p;
	register NCLUST * clp;

	for (clp = &basecluster; clp; clp = clp->next)
	{
		for (p = clp->nNODEs; p < &clp->nNODEs[CLUSTERSIZE]; ++p)
		{
			if( p->node.in.op != FREE )
			{
				fprintf(outfile,"At %#x: ", (unsigned)node_no(p));
				e222print(0, &p->node,"NODE");
			}
		}
	}
}

#ifndef	NODBG

static void
nt_print()
{
    register NCLUST * clp;

    for (clp = &basecluster; clp; clp = clp->next) {
	printf("nodes	%#lx - %#lx\n",
		    (char *) clp->nNODEs, (char *) &clp->nNODEs[CLUSTERSIZE]);
    }
    printf("%d clusters\n", nclusters);
}
#endif

void  
tfree(p)
register NODE *p; 
{
	/* allow tree fragments to be freed, also */
	if( !p ) return;
	switch( optype( p->tn.op ) )
	{
	case BITYPE:
		tfree( p->in.right );
		/*FALLTHRU*/
	case UTYPE:
		tfree( p->in.left );
	}
	p->in.op = FREE;
}


char	ftitle[100] = "\"\"";	/* title of the file */
extern int	lineno;		/* line number of the input file */


#undef	NTSTRBUF

#ifndef	TSTRSZ
#define TSTRSZ		2048
#endif

static char istrbuf[TSTRSZ];
static char * ini_tstrbuf[INI_NTSTRBUF] = { istrbuf };
static TD_INIT( td_tstr, INI_NTSTRBUF, sizeof(char *),
		TD_ZERO, ini_tstrbuf, "temp strings table");
#define	tstrbuf ((char **)(td_tstr.td_start))
#define	NTSTRBUF (td_tstr.td_allo)
#define	curtstr (td_tstr.td_used)	/* number of FILLED string buffers */
static int tstrused;


/* Reset temporary string space. */
static void
freestr()
{
    curtstr = 0;
    tstrused = 0;
}


char *
tstr( cp )			/* place copy of string into temp storage */
	register char *cp;	/* strings longer than TSTRSZ will break tstr */
{
	register int i = strlen( cp );
	register char *dp;

	if ( tstrused + i >= TSTRSZ )
	{
		/* not enough room in current string buffer */
		if (++curtstr >= NTSTRBUF)	/* need to enlarge tree ptrs */
		    td_enlarge(&td_tstr, 0);
		tstrused = 0;			/* nothing used in this buffer */
		if ( tstrbuf[curtstr] == 0 )	/* allocate one if not there */
		{
			if ( ( dp = (char *) malloc( TSTRSZ ) ) == 0 )
				cerror( "out of memory [tstr()]" );
			tstrbuf[curtstr] = dp;
		}
	}
	strcpy( dp = tstrbuf[curtstr] + tstrused, cp );
	tstrused += i + 1;
	return ( dp );
}

#include "dope.h"
