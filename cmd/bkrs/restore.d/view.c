/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:restore.d/view.c	1.5.2.1"

#include	<fcntl.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<time.h>
#include	<pwd.h>
#include	<grp.h>
#include	<table.h>
#include	<restore.h>
#include	<bkhist.h>
#include	<brtoc.h>

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

#define LOWBITS	0xffff
#define FTBITS	0x7000	/* Mask off all bits except file type bits */
#define FIFO	0x1	/* bit in byte set for FIFO file type */
#define CHAR	0x2	/* bit in byte set for character special file type */
#define DIR	0x4	/* bit in byte set for directory file type */
#define BLK	0x6	/* bit in byte set for block special file type */

/* Field sizes */
#define	DATELEN	25
#define	INODELEN 5
#define MODELEN	11
#define LINKLEN	3
#define SIZELEN	7
#define	UIDLEN 8
#define	GIDLEN 8

static void pr_short(), pr_long();
static char *pr_mode(), *pr_date();
static char *online_toc();
static int pr_toc();
static void pr_one();
static int selectmode();

extern struct group *getgrgid();
extern struct group *getgrnam();
extern struct passwd *getpwuid();
extern struct passwd *getpwnam();
extern char *rss_mfind();
extern int rstm_dfind();
extern int strcmp();
extern char *strdup();
extern char *f_to_fs();
extern int rstm_consistent();
extern int rstm_varch();
extern long strtol();
extern int strncmp();
extern char *strncpy();
extern int atoi();

/* Print out History information about a particular item */
int
view( h_tid, h_entry, rqst )
int h_tid;
ENTRY h_entry;
rs_rqst_t *rqst;
{
	register ndone, h_entryno;
	char *ptr;

	h_entryno = rstm_dceiling( h_tid, rqst->date );

	if( h_entryno < 0 ) return( 0 );

	if( !strcmp( rqst->type, R_DIRECTORY_TYPE )
		|| !strcmp( rqst->type, R_FILE_TYPE ) ) {

		rqst->oname = (char *)strdup( f_to_fs( rqst->object ) );
		
	} else if( !strcmp( rqst->type, R_FILESYS_TYPE ) ) {

		rqst->oname = (char *)strdup( rqst->object );

	} else if( !strcmp( rqst->type, R_PARTITION_TYPE )
		|| !strcmp( rqst->type, R_DISK_TYPE ) ) {

		rqst->odev = (char *)strdup( rqst->object );

	}

	for( ndone = 0; h_entryno > 0; h_entryno-- ) {
		if( TLread( h_tid, h_entryno, h_entry ) != TLOK )
			/* Unable to read an entry -- give up */
			return( ndone );

		if( !(ptr = (char *)TLgetfield( h_tid, h_entry, H_TAG )) || !*ptr )
			/* Ignore comment entries */
			continue;

		if( IS_CANDIDATE( h_tid, h_entry, rqst, ptr ) ) {

			if( !strcmp( rqst->type, R_FILE_TYPE )
				|| !strcmp( rqst->type, R_DIRECTORY_TYPE ) ) {

				switch( pr_toc( h_tid, h_entry, rqst ) ) {

				case 0:
					pr_one( h_tid, h_entry, rqst );
					ndone++;
					break;

				case 1:
					ndone++;
					break;

				default:
					break;
				}

			} else if( !strcmp( rqst->type, R_FILESYS_TYPE )
				|| !strcmp( rqst->type, R_PARTITION_TYPE )
				|| !strcmp( rqst->type, R_DISK_TYPE ) ) {

				pr_one( h_tid, h_entry, rqst );
				ndone++;
			}
		}
	}

	return( ndone );
}

/*
	Print if this file/dir is on this archive.
	returns:
		1: Found and printed.
		0: May be on this archive.
		-1: Not on this archive.
*/
static int
pr_toc( h_tid, h_entry, rqst )
int h_tid;
ENTRY h_entry;
rs_rqst_t *rqst;
{
	int t_tid, rc, t_entryno;
	char *mtime;
	char *tocname;
	ENTRY t_entry;
	TLdesc_t descr;
	TLsearch_t sarray[ 2 ];

	if( !(tocname = online_toc( h_tid, h_entry ) ) )
		return( 0 );

	/* First open the file */
	descr.td_fs = descr.td_eoe = descr.td_comment = (unsigned char)'\0';
	descr.td_format = TOC_ENTRY_F;

	if( (rc = TLopen( &t_tid, tocname, &descr, O_RDONLY, 0644 ) ) != TLOK
		&& rc != TLBADFS && rc != TLDIFFFORMAT ) 
		return( 0 );

	
	sarray[ 0 ].ts_fieldname = TOC_FNAME;
	sarray[ 0 ].ts_pattern = (unsigned char *)rqst->object;
	sarray[ 0 ].ts_operation = (int (*)())TLEQ;
	sarray[ 1 ].ts_fieldname = (unsigned char *)0;

	t_entryno = TLsearch1( t_tid, sarray, TLBEGIN, TLEND, TL_AND );

	if( t_entryno == TLFAILED || t_entryno < 0 )
		return( -1 );

	/* Get an entry element */
	if( !(t_entry = TLgetentry( t_tid )) )
		return( 0 );

	if( TLread( t_tid, t_entryno, t_entry ) != TLOK ) {
		(void) TLfreeentry( t_tid, t_entry );
		return( 0 );
	}

	/* Check to see that this is in long form? */
	mtime = (char *)TLgetfield( t_tid, t_entry, TOC_MTIME ); 
	if( mtime && *mtime )
		/* Assume that this is long format */
		pr_long( t_tid, t_entry, rqst );
	else pr_short( h_tid, h_entry, rqst );

	(void) TLfreeentry( t_tid, t_entry );
	return( 1 );
}

/*
	Print out date into "mon dd hhL:mm" if same year as this.
	Otherwise, print "mon dd yyyy"
*/
static char *
pr_date( date_p )
char *date_p;
{
	static char tbuf[ DATELEN ];
	char ybuf[ DATELEN ];
	time_t date, now;

	if( !*date_p ) {
		tbuf[0] = '\0';
		return( tbuf );
	}

	date = (time_t) strtol( date_p, (char **)0, 16 );
	(void) cftime( tbuf, "%b %d %R %Y", &date );

	now = time( 0 );
	(void) cftime( ybuf, "%Y", &now );
	if( strncmp( ybuf, &tbuf[13], 4 ) == 0 )
		tbuf[12] = '\0';
	else {
		(void) strncpy( &tbuf[7], &tbuf[13], 4 );
		tbuf[11] = '\0';
	}

	return( tbuf );
}

/* Print out short form of file/dir type */
static void
pr_short( h_tid, h_entry, rqst )
int h_tid;
ENTRY h_entry;
rs_rqst_t *rqst;
{
	(void) fprintf( stdout, "%s on archive dated: %s\n", rqst->object,
		pr_date( (char *)TLgetfield( h_tid, h_entry, H_DATE ) ) );
}

/* Print an 'ls -li' listing */
static void
pr_long( t_tid, t_entry, rqst )
int t_tid;
ENTRY t_entry;
rs_rqst_t *rqst;
{
	char *inode, *mode, *nlinks, *fsize, *uid, *gid, *mtime;

	inode = (char *)TLgetfield( t_tid, t_entry, TOC_INODE );
	mode = (char *)TLgetfield( t_tid, t_entry, TOC_MODE );
	nlinks = (char *)TLgetfield( t_tid, t_entry, TOC_NLINK );
	fsize = (char *)TLgetfield( t_tid, t_entry, TOC_SIZE );
	uid = (char *)TLgetfield( t_tid, t_entry, TOC_UID );
	gid = (char *)TLgetfield( t_tid, t_entry, TOC_GID );
	mtime = (char *)TLgetfield( t_tid, t_entry, TOC_MTIME );

	/* Login - must convert from uid.  If not in password file, use uid. */
	if( *uid ) {
		struct passwd *pwp;
		pwp = getpwuid( strtol( uid, (char **)0, 10 ) );
		if ( pwp )
			uid = (char *)pwp->pw_name;
	}

	/* Group - must convert from gid.  If not in password */
	/* file, use gid. */
	if( *gid ) {
		struct group *grp;
		grp = getgrgid( strtol( gid, (char **)0, 10 ) );
		if ( grp )
			gid = (char *)grp->gr_name;
	}

	(void) fprintf( stdout, "%*s %s %*s %*s %*s %*s %s %s\n",
		INODELEN, inode, pr_mode( mode ), LINKLEN, nlinks,
		UIDLEN, (*uid? uid: "root"), GIDLEN, (*gid? gid: "root"),
		SIZELEN, fsize, pr_date( mtime ), rqst->object );
}

/* Print out info from the archive */
static void
pr_one( h_tid, h_entry, rqst )
int h_tid;
ENTRY h_entry;
rs_rqst_t *rqst;
{
	if( !strcmp( rqst->type, R_FILE_TYPE )
		|| !strcmp( rqst->type, R_DIRECTORY_TYPE ) ) {
		(void) fprintf( stdout, "%s may be on archive dated: %s\n",
			rqst->object, pr_date( (char *) TLgetfield( h_tid, h_entry, H_DATE ) ) );

	} else if( !strcmp( rqst->type, R_PARTITION_TYPE )
		|| !strcmp( rqst->type, R_DISK_TYPE ) ) {
		(void) fprintf( stdout, "%s on %s\n", 
			TLgetfield( h_tid, h_entry, H_ODEVICE ),
			pr_date( (char *) TLgetfield( h_tid, h_entry, H_DATE ) ) );

	} else if( !strcmp( rqst->type, R_FILESYS_TYPE ) ) {
		(void) fprintf( stdout, "%s on %s on %s\n",
			TLgetfield( h_tid, h_entry, H_ONAME ),
			TLgetfield( h_tid, h_entry, H_ODEVICE ),
			pr_date( (char *) TLgetfield( h_tid, h_entry, H_DATE ) ) );
	}
}

/* Is this TOC online? */
static char *
online_toc( h_tid, h_entry )
int h_tid;
ENTRY h_entry;
{
	char *toc;
	struct stat buf;

	if( !(toc = (char *)TLgetfield( h_tid, h_entry, H_TOCNAME ) ) || !*toc )
		return( (char *)0 );

	if( !stat( toc, &buf ) && buf.st_size > 0 )
		return( toc );

	return( (char *)0 );
}

static char *
pr_mode( mode_p )
char *mode_p;
{
	/* these arrays are declared static to allow initializations */
	static int	m0[] = { 1, S_IREAD>>0, 'r', '-' };
	static int	m1[] = { 1, S_IWRITE>>0, 'w', '-' };
	static int	m2[] = { 3, S_ISUID|S_IEXEC, 's', S_IEXEC, 'x', S_ISUID, 'S', '-' };
	static int	m3[] = { 1, S_IREAD>>3, 'r', '-' };
	static int	m4[] = { 1, S_IWRITE>>3, 'w', '-' };
	static int	m5[] = { 3, S_ISGID|(S_IEXEC>>3),'s', S_IEXEC>>3,'x', S_ISGID,'l', '-'};
	static int	m6[] = { 1, S_IREAD>>6, 'r', '-' };
	static int	m7[] = { 1, S_IWRITE>>6, 'w', '-' };
	static int	m8[] = { 3, S_ISVTX|(S_IEXEC>>6),'t', S_IEXEC>>6,'x', S_ISVTX,'T', '-'};

	static int  *m[] = { m0, m1, m2, m3, m4, m5, m6, m7, m8};

	register int **mp;
	int ftype, i = 0;
	static char cbuf[ MODELEN + 1 ];
	short aflag;

	if( !mode_p || !*mode_p ) {
		*cbuf = '\0';
		return( cbuf );
	} else aflag = strtol( mode_p, (char **)0, 16 ) & LOWBITS;

	/* Figure out file type */
	ftype = (aflag & FTBITS) >> 12;
	if ( ftype & FIFO )
		cbuf[i] = 'p';
	else if ( ftype & CHAR )
		cbuf[i] = 'c';
	else if ( ftype & DIR )
		cbuf[i] = 'd';
	else if ( ftype & BLK )
		cbuf[i] = 'b';
	else
		cbuf[i] = '-';

	for (mp = &m[0]; mp < &m[sizeof(m)/sizeof(m[0])];)
		cbuf[++i] = (char) selectmode( *mp++, aflag );
	cbuf[++i] = '\0';
	return( cbuf );
}

static int
selectmode(pairp, aflag)
register int *pairp;
short aflag;
{
	register int n;

	n = *pairp++;
	while (n-->0) {
		if((aflag & *pairp) == *pairp) {
			pairp++;
			break;
		}else {
			pairp += 2;
		}
	}
	return( *pairp );
}
