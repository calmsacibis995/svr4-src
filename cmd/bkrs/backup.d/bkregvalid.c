/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:backup.d/bkregvalid.c	1.3.2.1"

#include	<stdio.h>
#include	<fcntl.h>
#include	<table.h>
#include	<string.h>
#include	<backup.h>
#include	<sys/types.h>
#include	<bkmsgs.h>
#include	<errors.h>
#include	<bkerrors.h>
#include	<bkreg.h>

extern int errno;
extern char *sys_errlist[];
extern char *errmsgs[];
#define SE sys_errlist[errno]

static int en_parse();
static int breg_compare();

extern void free();
extern void *malloc();
extern void twalk();
extern int atoi();
extern void bkr_init();
extern unsigned char *p_weekday1();
extern int bkr_and();
extern void bkr_set();
extern void *tfind();

struct breg {
	unsigned char *tag;
	unsigned char **depend;
	int  priority;
};
static int nbreg = 0;

static struct breg *base;
static int breg_avl = 0;

struct fsreg {
	unsigned char *oname;
	unsigned char *odev;
	bkrotate_t rot;
};

typedef enum { preorder, postorder, endorder, leaf } VISIT;

static struct fsreg *fsbase;
static int fsreg_avl = 0;

static char **tag_rootp = NULL;
static char **fs_rootp = NULL;

static int tid;
static ENTRY eptr;
static int fatalerr = 0;
void bkerror();
static unsigned char *en_getfield();
static unsigned char *toremove;
static int nrem, nstart;

int
bkregvalid(table)
char *table;			/* pathname to bkreg to check */
{
	static void filltable(), check_exist(), rem_dep(), check_dep();
	unsigned char *tag;
	int rc, i, period, curr_day, curr_week, entryno;
	int TLopen(), get_period(), get_rotate_start();
	TLdesc_t descr;

	(void) strncpy( (char *) &descr, "", sizeof( TLdesc_t ) );

	if( (rc = TLopen( &tid, table, &descr, O_RDONLY )) != TLOK ) {
		bkerror( stderr, ERROR17, table , SE );
		free(base);
		free(fsbase);
		return(BKBADREGTAB);
	}
	if( !(eptr = TLgetentry( tid ) ) ) {
		(void) TLclose(tid);
		return( BKNOMEMORY );
	}

	for(entryno = 1, rc = TLOK ;rc == TLOK ; entryno++ ) {
		rc = TLread( tid, entryno, eptr );
		if(rc == TLOK) {
			tag = (unsigned char *)TLgetfield( tid, eptr, R_TAG );
			if(!tag || !(*tag)) {
				continue;
			}
			nbreg++;
		}
	}

	/* Read the rotation period from the bkreg table */

	if( (i = get_period( tid , &period) ) ) {
		bkerror( stderr, ERROR18 );
		period = WK_PER_YR;
		fatalerr++;
	}
	else {
		if(period < 1) {
			bkerror( stderr, ERROR19 );
			period = 1;
			fatalerr++;
		}
		else if (period > WK_PER_YR) {
			bkerror( stderr, ERROR20, period, WK_PER_YR, WK_PER_YR);
			period = WK_PER_YR;
			fatalerr++;
		}
	}

	if( get_rotate_start( tid, period, &curr_week, &curr_day ) ) {
		bkerror( stderr, ERROR21);
		fatalerr++;
	} 

	if(!nbreg) {
		bkerror(stderr, ERROR31);
		fatalerr++;
	}
	else {
		if( (base = ((struct breg *) 
			malloc (nbreg * sizeof(struct breg)))) == NULL ) {
			return(BKNOMEMORY);
		}
		if( (fsbase = ((struct fsreg *) 
			malloc (nbreg * sizeof(struct fsreg)))) == NULL ) {
			free (base);
			return(BKNOMEMORY);
		}

		filltable();

		if(tag_rootp) {
			twalk( (void *) tag_rootp, check_exist);
			for(nstart=-1, nrem=0; nstart != nrem; ) {
				nstart = nrem;
				twalk((void *)tag_rootp, rem_dep);
			}
			twalk( (void *) tag_rootp, check_dep);
		}
	}

	(void) TLfreeentry(tid, eptr);
	(void) TLclose(tid);
	free(base);
	free(fsbase);

	return(fatalerr);

}



static void
filltable()
{
	int rc;
	int entryno = 1;

	for( ; ; entryno++ ) {

		rc = TLread( tid, entryno, eptr );

		switch( rc ) {

		case TLBADENTRY:
			/* hit the end of the table */
			break;

		case TLOK:
			/* Check to see if this entry satisfies the criteria */
			if( !en_parse( tid, eptr ) )  {
				fatalerr++;
			}
			continue;

		default:
			bkerror(stderr, ERROR27, rc, SE);
			fatalerr++;
			break;
		}
		break;
	}
}



static int
en_parse( tid, eptr )
int tid;
ENTRY	eptr;
{
	struct breg *wrkb , **ent;
	struct fsreg *wrkf = fsbase + fsreg_avl, **fent;
	static int breg_compare(), fsreg_compare();
	bkrotate_t rot;
	int pri;
	unsigned char **s_to_argv(), *tsearch(), *msg, *bkr_print();
	unsigned char *priority, *week, *day, *tag, *depend, **dep = NULL;
	unsigned char *ptr;

	tag = (unsigned char *)en_getfield( tid, eptr, R_TAG );
	if(!tag || !(*tag)) {
		 return( TRUE );
	}

	week = (unsigned char *)TLgetfield( tid, eptr, R_WEEK );
	day = (unsigned char *)TLgetfield( tid, eptr, R_DAY );

	if( !week && !day ) {
		bkerror( stderr, ERROR22, tag);
		return( FALSE );
	}

	bkr_init( rot );

	if( !(ptr = p_weekday1( week, day, rot )) || *ptr != '\0' ) {
		bkerror( stderr, ERROR22, tag);
		return( FALSE );
	}

	priority = (unsigned char *)en_getfield( tid, eptr, R_PRIORITY );
	pri = atoi( (char *) priority );
	if((pri < 0) || (pri > 100)) {
		pri = DEFAULT_PRIORITY;
		bkerror(stderr, ERROR23, tag, priority, pri);
	}

	depend = (unsigned char *)en_getfield( tid, eptr, R_DEPEND );
	if(depend) {
		dep = s_to_argv(depend, ", ");
	}

	wrkb = base + breg_avl;
	wrkb->tag = tag;
	wrkb->depend = dep;
	wrkb->priority = pri;

	ent = (struct breg **)
		tsearch( (char *) wrkb, (char **) &tag_rootp, breg_compare);
	if(*ent != wrkb) {
		bkerror(stderr,  ERROR24, tag);
		return(FALSE);
	}
	else {
		breg_avl++;
	}

	wrkf = fsbase + fsreg_avl;
	wrkf->oname = (unsigned char *)en_getfield( tid, eptr, R_ONAME );
	wrkf->odev = (unsigned char *)en_getfield( tid, eptr, R_ODEVICE );
	bkr_init( wrkf->rot );
	if( !p_weekday1( week, day, wrkf->rot ) ) {
		bkerror(stderr, ERROR25, tag, week, day);
		return( FALSE );
	}

	fent = (struct fsreg **)
		tsearch((char *)wrkf, (char **) &fs_rootp, fsreg_compare);
	if(*fent != wrkf) {	/* already have oname,odev pair */
		bkr_init(rot);
		if( bkr_and(rot, (*fent)->rot, wrkf->rot)) {
			msg = bkr_print(rot);
			bkerror(stderr, ERROR26, tag, wrkf->oname,
					wrkf->odev, msg);
			bkr_set((*fent)->rot, wrkf->rot);
			return(FALSE);
		}
		else {
			bkr_set((*fent)->rot, wrkf->rot);
		}
	
	}
	else {
		fsreg_avl++;
	}

	return( TRUE );
}



static void
check_exist( bp, order, level)
struct breg **bp;
VISIT order;
int level;
{
	struct breg bwrk;
	static int breg_compare();
	unsigned char **tg;

	if(!((order == preorder) || (order == leaf))) {
		return;
	}

	tg = (*bp)->depend;

	for(; (tg && (*tg)); tg++) {
		if(!(**tg))
			continue;
		bwrk.tag = *tg;
		if(! tfind((char *) &bwrk, (void **) &tag_rootp, breg_compare)) {
			bkerror(stderr, ERROR28, (*bp)->tag, *tg);
			**tg = '\0';
		}
	}
}



static void
rem_dep( bp, order, level)
struct breg **bp;
VISIT order;
int level;
{
	static void rem_one();
	unsigned char **tg;

	if(!((order == preorder) || (order == leaf))) {
		return;
	}

	tg = (*bp)->depend;

	for(; (tg && (*tg)); tg++) {
		if(**tg) {	/* dependency still exists */
			return;
		}
	}

	toremove = (*bp)->tag;
	twalk( (void *) tag_rootp, rem_one);

}




static void
rem_one( bp, order, level)
struct breg **bp;
VISIT order;
int level;
{
	unsigned char **tg;

	if(!((order == preorder) || (order == leaf))) {
		return;
	}

	tg = (*bp)->depend;

	for(; (tg && (*tg)); tg++) {
		if(!strcmp((char *)toremove, (char *) (*tg))) {
			**tg = '\0';
			nrem++;
		}
	}

}




static void
check_dep( bp, order, level)
struct breg **bp;
VISIT order;
int level;
{
	unsigned char **tg;
	short first = 1;
	char ms[513];

	if(!((order == preorder) || (order == leaf))) {
		return;
	}

	tg = (*bp)->depend;

	for(; (tg && (*tg)); tg++) {
		if(! (**tg) )
			continue;
		if(first) {
			first = 0;
			(void) sprintf(ms, errmsgs[ERROR29], (*bp)->tag );
		}
		(void) strcat (ms, (char *) (*tg) );
		(void) strcat (ms, " ");
	}
	if(!first) {
		bkerror(stderr, ERROR30, ms);
		fatalerr++;
	}
}


static
unsigned char *
en_getfield( tid, eptr, fieldname )
int tid;
ENTRY eptr;
unsigned char *fieldname;
{
	register unsigned char *field;

	field = TLgetfield( tid, eptr, fieldname );
	if( !field || !*field ) 
		return( (unsigned char *)NULL );

	return( (unsigned char *)strdup( (char *)field ) );
}



static int
breg_compare(a, b)
struct breg *a, *b;
{
	return(strcmp((char *)((struct breg *)a)->tag,
				(char *)((struct breg *)b)->tag));
}


static int
fsreg_compare(c, d)
struct fsreg *c, *d;
{
	int ret;

	ret = strcmp((char *)(((struct fsreg *)c)->oname),
				((char *)((struct fsreg *)d)->oname)); 

	if(ret)
		return(ret);

	return(strcmp((char *)(((struct fsreg *)c)->odev),
				((char *)((struct fsreg *)d)->odev)));
}
