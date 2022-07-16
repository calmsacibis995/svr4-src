/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1987, 1988 Microsoft Corporation	*/
/*	  All Rights Reserved	*/

/*	This Module contains Proprietary Information of Microsoft  */
/*	Corporation and should be treated as Confidential.	   */

#ident	"@(#)xx:cmd/fsck/fsck1.c	1.2.1.1"

/*
 *      Derived from XENIX fsck1.c 1.9 87/05/01
 */

#include "fsck.h"
/* #include "../h/std.h" */

char	*lfname =	"lost+found";
char	*checklist =	"/etc/checklist";
FILE	*rcv_out =	NULL;			/*M012*/
char	*recover =	"/dev/recover";		/*M012*/
dev_t	pipedev	= -1;

void (*signal())();

main(argc,argv)
int      argc;
register char    *argv[];                       /*M001*/
{
	register FILE *fp;
	FILE	*append_open();			/*M014*/
	register n;
	register char *p;
 	char *cmdname, filename[50];		/* M011 */
	char *sbrk();
	struct stat statbuf;			/* M011 */
/*      int (*sg)();                            /* M011 */
	void (*sg)();                           /* S5.3 compatibility */

 	cmdname = argv[0];
	sync();
	while(--argc > 0 && **++argv == '-') {
		switch(*++*argv) {
			case 'a':
				aflag++;
				rcv_out = append_open(recover);	/*M014*/
				break;
			case 't':
			case 'T':
				tflag++;
				if(**++argv == '-' || --argc <= 0)
					errexit("Bad -t option\n");
				p = scrfile;
				while(*p++ = **argv)
					(*argv)++;
				break;
			case 's':	/* salvage flag */
				stype(++*argv);
				sflag++;
				break;
			case 'S':	/* conditional salvage */
				stype(++*argv);
				csflag++;
				break;
			case 'n':	/* default no answer flag */
			case 'N':
				nflag++;
				yflag = 0;
				break;
			case 'y':	/* default yes answer flag */
			case 'Y':
				yflag++;
				nflag = 0;
				break;
			case 'c':	/* convert file system */
				cflag++;
				break;
			case 'q':			/* M011 begin */
				qflag++;
				break;
			case 'D':
				Dirc++;
				break;
			case 'F':
			case 'f':
				fast++;
				break;			/* M011 end */
			default:
badswit:
				errexit("%c option?\n",**argv);
		}
	}
	if(nflag && sflag )				/* M011 begin */
		errexit("Incompatible options: -n and -s\n");
	if(nflag && qflag)
		errexit("Incompatible options: -n and -q\n");
	if(sflag && csflag)
		sflag = 0;
	if(csflag) nflag++;				/* M011 end */

	/* Begin M007 */
	if(rrflag && argc != 1)
		errexit("Option -rr requires root device only\n");
	if(rrflag) {
		nflag = 0;
		yflag++;
	}
	/* End M007 */

	/* Begin M008 */
	memsize = MAXDATA;
	while(memsize >= 2*sizeof(BUFAREA) &&
		(membase = sbrk(memsize)) == (char *)-1)
		memsize -= 1024;
	sbrk(-sizeof(int));
	memsize -= sizeof(int);
	/* End M008 */
	if(memsize < 2*sizeof(BUFAREA))
		errexit("Can't get memory\n");
	/* M011 begin */
	for(n = 1; n < NSIG; n++) {
		if(n == SIGCLD || n == SIGPWR)
			continue;
		sg = signal(n, catch);
		if(sg != SIG_DFL)
			signal(n,sg);
	}
	/* Check if standard input is a pipe. If it is, record pipedev so
	 * we won't ever check it */
	if ( fstat( 0, &statbuf) == -1 )
		errexit("Can't fstat standard input\n");
	if ( (statbuf.st_mode & S_IFMT) == S_IFIFO ) pipedev = statbuf.st_dev;
	/* M011 end */
	if(argc) {		/* arg list has file names */
		while(argc-- > 0)
			check(*argv++);
	}
	else {			/* use default checklist */
		errexit("No filesystem specified\n");
	}
	exit(0);
}


/* VARARGS1 */
error(s1,s2,s3,s4)
char *s1;
{
	fprntf(stderr,s1,s2,s3,s4);
}


/* VARARGS1 */
errexit(s1,s2,s3,s4)
char *s1;
{
	error(s1,s2,s3,s4);
	exit(8);
}


check(dev)
char *dev;
{
	register DINODE *dp;
	register n;
	register ino_t *blp;
	ino_t savino;
	daddr_t blk;
	register BUFAREA *bp1, *bp2;

	if(setup(dev) == NO)
		return;


	fprntf(stdout,"** Phase 1 - Check Blocks and Sizes\n");
	pfunc = pass1;
	for(inum = 1; inum <= imax; inum++) {
		if((dp = ginode()) == NULL)
			continue;
		if(ALLOC) {
			lastino = inum;
			if(ftypeok(dp) == NO) {
				fprntf(stderr,"UNKNOWN FILE TYPE I=%u",inum);
				if(dp->di_size)
					fprntf(stderr," (NOT EMPTY)");
				if(reply("CLEAR") == YES) {
					zapino(dp);
					inodirty();
				}
				continue;
			}
			n_files++;
			if(setlncnt(dp->di_nlink) <= 0) {
				if(badlnp < &badlncnt[MAXLNCNT])
					*badlnp++ = inum;
				else {
					fprntf(stderr,"LINK COUNT TABLE OVERFLOW");
					if(reply("CONTINUE") == NO)
						errexit("");
				}
			}
			setstate(DIR ? DSTATE : FSTATE);
			badblk = dupblk = 0;
			filsize = 0;
			maxblk = 0;
			ckinode(dp,ADDR);
			if((n = getstate()) == DSTATE || n == FSTATE)
				sizechk(dp);
		}
		else if(dp->di_mode != 0) {
			fprntf(stderr,"PARTIALLY ALLOCATED INODE I=%u",inum);
			if(dp->di_size)
				fprntf(stderr," (NOT EMPTY)");
			if(reply("CLEAR") == YES) {
				zapino(dp);
				inodirty();
			}
		}
	}


	if(enddup != &duplist[0]) {
		fprntf(stdout,"** Phase 1b - Rescan For More DUPS\n");
		pfunc = pass1b;
		for(inum = 1; inum <= lastino; inum++) {
			if(getstate() != USTATE && (dp = ginode()) != NULL)
				if(ckinode(dp,ADDR) & STOP)
					break;
		}
	}
	if(rawflg) {
		if(inoblk.b_dirty)
			bwrite(&dfile,membase,startib,(int)niblk*XXBSIZE);
		inoblk.b_dirty = 0;
		if(poolhead) {
			clear(membase,niblk*XXBSIZE);
			for(bp1 = poolhead;bp1->b_next;bp1 = bp1->b_next);
			bp2 = &((BUFAREA *)membase)[(niblk*XXBSIZE)/sizeof(BUFAREA)];
			while(--bp2 >= (BUFAREA *)membase) {
				initbarea(bp2);
				bp2->b_next = bp1->b_next;
				bp1->b_next = bp2;
			}
		}
		rawflg = 0;

	}


    if ( !fast )	/* M011 */
    {
	fprntf(stdout,"** Phase 2 - Check Pathnames\n");
	inum = ROOTINO;
	thisname = pathp = pathname;
	pfunc = pass2;
	switch(getstate()) {
		case USTATE:
			errexit("ROOT INODE UNALLOCATED. TERMINATING.\n");
		case FSTATE:
			fprntf(stderr,"ROOT INODE NOT DIRECTORY");
			if(reply("FIX") == NO || (dp = ginode()) == NULL)
				errexit("");
			dp->di_mode &= ~IFMT;
			dp->di_mode |= IFDIR;
			inodirty();
			setstate(DSTATE);
		case DSTATE:
			descend();
			break;
		case CLEAR:
			fprntf(stderr,"DUPS/BAD IN ROOT INODE\n");
			if(reply("CONTINUE") == NO)
				errexit("");
			setstate(DSTATE);
			descend();
	}


	pss2done++;
	fprntf(stdout,"** Phase 3 - Check Connectivity\n");
	for(inum = ROOTINO; inum <= lastino; inum++) {
		if(getstate() == DSTATE) {
			pfunc = findino;
			srchname = "..";
			savino = inum;
			do {
				orphan = inum;
				if((dp = ginode()) == NULL)
					break;
				filsize = dp->di_size;
				parentdir = 0;
				ckinode(dp,DATA);
				if((inum = parentdir) == 0)
					break;
			} while(getstate() == DSTATE);
			inum = orphan;
			if(linkup() == YES) {
				thisname = pathp = pathname;
				*pathp++ = '?';
				pfunc = pass2;
				descend();
			}
			inum = savino;
		}
	}


	fprntf(stdout,"** Phase 4 - Check Reference Counts\n");
	pfunc = pass4;
	for(inum = ROOTINO; inum <= lastino; inum++) {
		switch(getstate()) {
			case FSTATE:
				if(n = getlncnt())
					adjust((short)n);
				else {
					/* M011 begin */
					for(blp = badlncnt;blp < badlnp; blp++)
					    if(*blp == inum) 
					    {   if((dp=ginode()) && dp->di_size)
						{   if((n = linkup()) == NO)
						       clri("UNREF",NO);
						    if (n == REM)
						       clri("UNREF",REM);
						}
						else
							clri("UNREF",YES);
						break;	
					    }
					/* M011 end */
				}
				break;
			case DSTATE:
				clri("UNREF",YES);
				break;
			case CLEAR:
				clri("BAD/DUP",YES);
		}
	}
	if(imax - n_files != superblk.s_tinode) {
		fprntf(stderr,"FREE INODE COUNT WRONG IN SUPERBLK");
		/* M011 begin */
		if (qflag) {
			superblk.s_tinode = imax - n_files;
			sbdirty();
			fprntf(stderr,"\nFIXED\n");
		}
		else if(reply("FIX") == YES) {
			superblk.s_tinode = imax - n_files;
			sbdirty();
		}
		/* M011 end */
	}
	flush(&dfile,&fileblk);


    } /* skip to phase 5 if fast set */ /* M011 */
	fprntf(stdout,"** Phase 5 - Check Free List\n");	/* M010 */
	if(sflag || (csflag && rplyflag == 0)) {
		fprntf(stderr,"(Ignored)\n");
		fixfree = 1;
	}
	else {
		fprntf(stderr,"\n");
		if(freemap)
			copy(blkmap,freemap,(MEMSIZE)bmapsz);
		else {
			for(blk = 0; blk < fmapblk; blk++) {
				bp1 = getblk((BUFAREA *)NULL,blk);
				bp2 = getblk((BUFAREA *)NULL,blk+fmapblk);
				copy(bp1->b_un.b_buf,bp2->b_un.b_buf,XXBSIZE);
				dirty(bp2);
			}
		}
		badblk = dupblk = 0;
		freeblk.df_nfree = superblk.s_nfree;
		for(n = 0; n < NICFREE; n++)
			freeblk.df_free[n] = superblk.s_free[n];
		freechk();
		if(badblk)
			fprntf(stderr,"%d BAD BLKS IN FREE LIST\n",badblk);
		if(dupblk)
			fprntf(stderr,"%d DUP BLKS IN FREE LIST\n",dupblk);
		if(fixfree == 0) {
			if((n_blks+n_free) != (fmax-fmin)) {
				fprntf(stderr,"%ld BLK(S) MISSING\n",
					fmax-fmin-n_blks-n_free);
				fixfree = 1;
			}
			else if(n_free != superblk.s_tfree) {
				fprntf(stderr,"FREE BLK COUNT WRONG IN SUPERBLK");
				/* M011 begin */
				if (qflag) {
				    superblk.s_tfree = n_free;
				    sbdirty();
				    fprntf(stderr,"\nFIXED\n");
				}
				else if(reply("FIX") == YES) {
				    superblk.s_tfree = n_free;
				    sbdirty();
				}
				/* M011 end */
			}
		}
		if(fixfree) {
			fprntf(stderr,"BAD FREE LIST");
			/* M011 begin */
			if(qflag && !sflag) {
				fixfree = 1;
				fprntf(stderr,"\nSALVAGED\n");
			}
			else if(reply("SALVAGE") == NO)
				fixfree = 0;
			/* M011 end */
		}
	}


	if(fixfree) {
		fprntf(stdout,"** Phase 6 - Salvage Free List\n");
		makefree();
		n_free = superblk.s_tfree;
	}


	if (n_blks != SYSTOMUL(n_blks))
		fprntf(stdout,"%ld files %ld blocks %ld free\n",
		    		n_files,(long)SYSTOMUL(n_blks),
					(long)SYSTOMUL(n_free));
	else
		fprntf(stdout,"%ld files %ld blocks %ld free\n",
						    n_files,n_blks,n_free);

	/* we'll write the superblock out if:
	/*      any part of the filsys was modified, or
	/*      If s_clean is not set, and we are not checking the
	/*              mounted root.
	*/

	if((dfile.mod) || ((!hotroot)&&(superblk.s_clean != S_CLEAN))) {
		time(&superblk.s_time);
		superblk.s_clean = S_CLEAN;
		sbdirty();
		dfile.mod = 1;
	}

	ckfini();
	sync();         /* sync writes superblock before i/o buffers */
/*
 *  M017 Notes on remounting the root:
 *   The sync() scall will not write the superblock if the filesystem
 * hasn't changed since the last sync().   Fsck does a sync() at the
 * top of main().  So,  if fsck is the only thing running then this
 * sync() should not overwrite the superblock, written above, with
 * the kernel's version.
 * This is good because otherwise the whole scheme would be doomed.
 */
 		/* M017 rearranged code start vvv  */

	if(!dfile.mod)
		return;

	if(nflag)
		fprntf(stderr,
		     "\n***** FILE SYSTEM NOT MODIFIED, STILL DIRTY *****\n");
	else
		fprntf(stderr,"\n***** FILE SYSTEM WAS MODIFIED *****\n");

	if(hotroot) {
		if(aflag && rcv_out != NULL) {
			fflush(rcv_out);
			fclose(rcv_out);
		}
		if(nflag)
			return;
		fprntf(stderr,
			"\n***** REMOUNTING THE ROOT FILESYSTEM *****\n");
/*              uadmin(A_REMOUNT,  0, 0);/*M016*//*M017*/
	}
 		/* M017 rearranged code stop ^^^  */
}


ckinode(dp,flg)
DINODE *dp;
register flg;
{
	register daddr_t *ap;
	register ret;
	int (*func)();
	register int n;                                 /*M001*/
	daddr_t	iaddrs[NADDR];

	if(Fsver == S_V3) {
		if(SPECIAL)
			return(KEEPON);
	} else {
		if(V7_SPECIAL)				/* M006 */
			return(KEEPON);
	}
	l3tol(iaddrs,dp->di_addr,NADDR);
	/* M011 begin */
	switch(flg) {
		case ADDR:
			func = pfunc;
			break;
		case DATA:
			func = dirscan;
			break;
		case BBLK:
			func = chkblk;
	}
	for(ap = iaddrs; ap < &iaddrs[NADDR-3]; ap++) {
	    if(*ap && (ret = (*func)(*ap,((ap == &iaddrs[0]) ? 1 : 0))) & STOP)
		if(flg != BBLK)
			return(ret);
	}
	for(n = 1; n < 4; n++) {
	    if(*ap && (ret = iblock(*ap,n,flg)) & STOP) {
		if(flg != BBLK)
			return(ret);
	    }
	    ap++;
	}
	/* M011 end */
	return(KEEPON);
}


iblock(blk,ilevel,flg)
daddr_t blk;
register ilevel;
{
	register daddr_t *ap;
	register n;
	int (*func)();
	BUFAREA ib;

	if ( flg == BBLK ) func = chkblk;	/* M011 */
	else if(flg == ADDR) {
		func = pfunc;
		if(((n = (*func)(blk)) & KEEPON) == 0)
			return(n);
	}
	else
		func = dirscan;
	if(outrange(blk))		/* protect thyself */
		return(SKIP);
	initbarea(&ib);
	if(getblk(&ib,blk) == NULL)
		return(SKIP);
	ilevel--;
	for(ap = ib.b_un.b_indir; ap < &ib.b_un.b_indir[NINDIR]; ap++) {
		if(*ap) {
			if(ilevel > 0) {
				n = iblock(*ap,ilevel,flg);
			}
			else
				n = (*func)(*ap,0);
			if((n & STOP) && flg != BBLK )
				return(n);
		}
	}
	return(KEEPON);
}

/* M011 begin */

chkblk(blk,flg)
register daddr_t blk;
{
	register DIRECT *dirp;
	register char *ptr;
	int zerobyte, baddir = 0, dotcnt = 0;

	if(outrange(blk))
		return(SKIP);
	if(getblk(&fileblk, blk) == NULL)
		return(SKIP);
	for(dirp = dirblk; dirp < &dirblk[NDIRECT]; dirp++) 
	{   ptr = dirp->d_name;
	    zerobyte = 0;
	    while(ptr < &dirp->d_name[DIRSIZ] ) 
	    {   if(zerobyte && *ptr) 
		{   baddir++;
		    break;
		}
		if(flg) 
		{   if(ptr == &dirp->d_name[0] && *ptr == '.' 
						    && *(ptr + 1) == '\0') 
		    {   dotcnt++;
			if(inum != dirp->d_ino) 
			{   fprntf(stderr,"NO VALID '.' in DIR I = %u\n",inum);
			    baddir++;
			}
			break;
		    }
		    if(ptr == &dirp->d_name[0] && *ptr == '.' &&
				    *(ptr + 1) == '.' && *(ptr + 2) == '\0') 
		    {
			dotcnt++;
			if(!dirp->d_ino) {
			    fprntf(stderr,"NO VALID '..' in DIR I = %u\n",inum);
			    baddir++;
			}
			break;
		    }
		}
		if(*ptr == '/') 
		{   baddir++;
		    break;
		}
		if(*ptr == '\0') 
		{   if(dirp->d_ino && ptr == &dirp->d_name[0]) 
		    {   baddir++;
			break;
		    }
		    else
			zerobyte++;
		}
		ptr++;
	    }
	}
	if(flg && dotcnt < 2) 
	{   fprntf(stderr,"MISSING '.' or '..' in DIR I = %u\n",inum);
	    fprntf(stderr,"BLK %ld ",blk);
	    pinode();
	    fprntf(stderr,"\nDIR=%s\n\n",pathname);
	    return(YES);
	}
	if(baddir) 
	{   fprntf(stderr,"BAD DIR ENTRY I = %u\n",inum);
	    fprntf(stderr,"BLK %ld ",blk);
	    pinode();
	    fprntf(stderr,"\nDIR=%s\n\n",pathname);
	    return(YES);
	}
	return(KEEPON);
}

/* M011 end */

pass1(blk)
daddr_t blk;
{
	register daddr_t *dlp;

	if(outrange(blk)) {
		blkerr("BAD",blk);
		if(++badblk >= MAXBAD) {
			fprntf(stderr,"EXCESSIVE BAD BLKS I=%u",inum);
			if(reply("CONTINUE") == NO)
				errexit("");
			return(STOP);
		}
		return(SKIP);
	}
	if(getbmap(blk)) {
		blkerr("DUP",blk);
		if(++dupblk >= MAXDUP) {
			fprntf(stderr,"EXCESSIVE DUP BLKS I=%u",inum);
			if(reply("CONTINUE") == NO)
				errexit("");
			return(STOP);
		}
		if(enddup >= &duplist[DUPTBLSIZE]) {
			fprntf(stderr,"DUP TABLE OVERFLOW.");
			if(reply("CONTINUE") == NO)
				errexit("");
			return(STOP);
		}
		for(dlp = duplist; dlp < muldup; dlp++) {
			if(*dlp == blk) {
				*enddup++ = blk;
				break;
			}
		}
		if(dlp >= muldup) {
			*enddup++ = *muldup;
			*muldup++ = blk;
		}
	}
	else {
		n_blks++;
		setbmap(blk);
	}
	filsize++;
	return(KEEPON);
}


pass1b(blk)
daddr_t blk;
{
	register daddr_t *dlp;

	if(outrange(blk))
		return(SKIP);
	for(dlp = duplist; dlp < muldup; dlp++) {
		if(*dlp == blk) {
			blkerr("DUP",blk);
			*dlp = *--muldup;
			*muldup = blk;
			return(muldup == duplist ? STOP : KEEPON);
		}
	}
	return(KEEPON);
}


pass2(dirp)
register DIRECT *dirp;
{
	register char *p;
	register n;
	register DINODE *dp;                            /*M001*/

	if((inum = dirp->d_ino) == 0)
		return(KEEPON);
	thisname = pathp;
	/* M011 begin */
	if((&pathname[MAXPATH] - pathp) < DIRSIZ) 
	{   if((&pathname[MAXPATH] - pathp) < dirlen(DIRSIZ,dirp->d_name)) 
	    {   fprntf(stderr,"DIR pathname too deep\n");
		fprntf(stderr,"Increase MAXPATH and recompile.\n");
		fprntf(stderr,"DIR pathname is <%s>\n",pathname);
		ckfini();
		exit(4);
	    }
	}
	/* M011 end */
	for(p = dirp->d_name; p < &dirp->d_name[DIRSIZ]; )
		if((*pathp++ = *p++) == 0) {
			--pathp;
			break;
		}
	*pathp = 0;
	n = NO;
	if(inum > imax || inum < ROOTINO)
		n = direrr("I OUT OF RANGE");
	else {
	again:
		switch(getstate()) {
			case USTATE:
				n = direrr("UNALLOCATED");
				break;
			case CLEAR:
				if((n = direrr("DUP/BAD")) == YES)
					break;
				if((dp = ginode()) == NULL) {
					break;
				}
				setstate(DIR ? DSTATE : FSTATE);
				goto again;
			case FSTATE:
				declncnt();
				break;
			case DSTATE:
				declncnt();
				descend();
		}
	}
	pathp = thisname;
	if(n == NO)
		return(KEEPON);
	dirp->d_ino = 0;
	return(KEEPON|ALTERD);
}

/* M011 begin */
dirlen ( n, s )		/* returns min of 'n' and length of s */
register int n;
register char *s;
{
	register int i ;
	
	for ( i = 0; i < n; ++i)
	    if ( *s++ == 0 )
		break;
	return( i );
}
/* M011 end */

pass4(blk)
daddr_t blk;
{
	register daddr_t *dlp;

	if(outrange(blk))
		return(SKIP);
	if(getbmap(blk)) {
		for(dlp = duplist; dlp < enddup; dlp++)
			if(*dlp == blk) {
				*dlp = *--enddup;
				return(KEEPON);
			}
		clrbmap(blk);
		n_blks--;
	}
	return(KEEPON);
}


pass5(blk)
daddr_t blk;
{
	if(outrange(blk)) {
		fixfree = 1;
		if(++badblk >= MAXBAD) {
			fprntf(stderr,"EXCESSIVE BAD BLKS IN FREE LIST.");
			if(reply("CONTINUE") == NO)
				errexit("");
			return(STOP);
		}
		return(SKIP);
	}
	if(getfmap(blk)) {
		fixfree = 1;
		if(++dupblk >= DUPTBLSIZE) {
			fprntf(stderr,"EXCESSIVE DUP BLKS IN FREE LIST.");
			if(reply("CONTINUE") == NO)
				errexit("");
			return(STOP);
		}
	}
	else {
		n_free++;
		setfmap(blk);
	}
	return(KEEPON);
}


blkerr(s,blk)
daddr_t blk;
char *s;
{
	fprntf(stderr,"%ld %s I=%u\n",blk,s,inum);
	setstate(CLEAR);	/* mark for possible clearing */
}


descend()
{
	register DINODE *dp;
	register char *savname;
	off_t savsize;

	setstate(FSTATE);
	if((dp = ginode()) == NULL)
		return;
	/* M011 begin */
	if ( Dirc && !pss2done )
		ckinode(dp,BBLK);
	/* M011 end */
	savname = thisname;
	*pathp++ = '/';
	savsize = filsize;
	filsize = dp->di_size;
	ckinode(dp,DATA);
	thisname = savname;
	*--pathp = 0;
	filsize = savsize;
}


dirscan(blk)
daddr_t blk;
{
	register DIRECT *dirp;
	register char *p1, *p2;
	register n;
	DIRECT direntry;

	if(outrange(blk)) {
		filsize -= XXBSIZE;
		return(SKIP);
	}
	for(dirp = dirblk; dirp < &dirblk[NDIRECT] &&
		filsize > 0; dirp++, filsize -= sizeof(DIRECT)) {
		if(getblk(&fileblk,blk) == NULL) {
			filsize -= (&dirblk[NDIRECT]-dirp)*sizeof(DIRECT);
			return(SKIP);
		}
		p1 = &dirp->d_name[DIRSIZ];
		p2 = &direntry.d_name[DIRSIZ];
		while(p1 > (char *)dirp)
			*--p2 = *--p1;
		if((n = (*pfunc)(&direntry)) & ALTERD) {
			if(getblk(&fileblk,blk) != NULL) {
				p1 = &dirp->d_name[DIRSIZ];
				p2 = &direntry.d_name[DIRSIZ];
				while(p1 > (char *)dirp)
					*--p1 = *--p2;
				fbdirty();
			}
			else
				n &= ~ALTERD;
		}
		if(n & STOP)
			return(n);
	}
	return(filsize > 0 ? KEEPON : STOP);
}


direrr(s)
char *s;
{
	register DINODE *dp;
	register n;

	fprntf(stderr,"%s ",s);
	pinode();
	/* M011 begin */
	if((dp = ginode()) != NULL && ftypeok(dp)) 
	{   fprntf(stderr,"\n%s=%s",DIR?"DIR":"FILE",pathname);
	    if(DIR) 
	    {   if(dp->di_size > EMPT) 
		{   if((n = chkempt(dp)) == NO) 
		    {   fprntf(stderr," (NOT EMPTY)\n");
		    }
		    else if(n != SKIP) 
		    {   fprntf(stderr," (EMPTY)");
			if(!nflag) 
			{   fprntf(stderr," -- REMOVED\n");
			    return(YES);
			}
			else
			    fprntf(stderr,"\n");
		    }
		}
		else
		{   fprntf(stderr," (EMPTY)");
		    if(!nflag) 
		    {   fprntf(stderr," -- REMOVED\n");
			return(YES);
		    }
		    else
			fprntf(stderr,"\n");
		}
	    }
	    else if(REG)
		if(!dp->di_size) 
		{   fprntf(stderr," (EMPTY)");
		    if(!nflag) 
		    {   fprntf(stderr," -- REMOVED\n");
			return(YES);
		    }
		    else
			fprntf(stderr,"\n");
		}
	}
	else 
	{   fprntf(stderr,"\nNAME=%s",pathname);
	    if(!dp->di_size) 
	    {   fprntf(stderr," (EMPTY)");
		if(!nflag) 
		{   fprntf(stderr," -- REMOVED\n");
		    return(YES);
		}
		else
		    fprntf(stderr,"\n");
	    }
	    else
		fprntf(stderr," (NOT EMPTY)\n");
	}
	/* M011 end */
	return(reply("REMOVE"));
}


adjust(lcnt)
register short lcnt;
{
	register DINODE *dp;
	register n;

	if((dp = ginode()) == NULL)
		return;
	if(dp->di_nlink == lcnt) {
		/* M011 begin */
		if( (n = linkup()) == NO)
			clri("UNREF",NO);
		if ( n == REM )
			clri("UNREF",REM);
		/* M011 end */
	}
	else {
		fprntf(stderr,"LINK COUNT %s",
			(lfdir==inum)?lfname:(DIR?"DIR":"FILE"));
		pinode();
		fprntf(stderr," COUNT %d SHOULD BE %d",
			dp->di_nlink,dp->di_nlink-lcnt);
		if(reply("ADJUST") == YES) {
			dp->di_nlink -= lcnt;
			inodirty();
		}
	}
}


clri(s,flg)
char *s;
{
	register DINODE *dp;
	register n;		/* M011 */

	if((dp = ginode()) == NULL)
		return;
/* M011  begin */
	if(flg == YES) 
	{   if(!FIFO || !qflag || nflag) 
	    {   fprntf(stderr,"%s %s",s,DIR?"DIR":"FILE");
		pinode();
	    }
	    if(DIR) 
	    {   if(dp->di_size > EMPT) 
		{   if((n = chkempt(dp)) == NO) 
		    {   fprntf(stderr," (NOT EMPTY)\n");
		    }
		    else if(n != SKIP) 
		    {   fprntf(stderr," (EMPTY)");
			if(!nflag) 
			{   fprntf(stderr," -- REMOVED\n");
			    clrinode(dp);
			    return;
			}
			else
			    fprntf(stderr,"\n");
		    }
		}
		else 
		{   fprntf(stderr," (EMPTY)");
		    if(!nflag) 
		    {   fprntf(stderr," -- REMOVED\n");
			clrinode(dp);
			return;
		    }
		    else
			fprntf(stderr,"\n");
		}
	    }
	    if(REG)
	    {	if(!dp->di_size) 
		{   fprntf(stderr," (EMPTY)");
		    if(!nflag) 
		    {   fprntf(stderr," -- REMOVED\n");
			clrinode(dp);
			return;
		    }
		    else
			fprntf(stderr,"\n");
		}
		else
		    fprntf(stderr," (NOT EMPTY)\n");
	    }
	    if (FIFO && !nflag) 
	    {   if(!qflag)
		    fprntf(stderr," -- CLEARED");
		fprntf(stderr,"\n");
		clrinode(dp);
		return;
	    }
	}
	if(flg == REM)	clrinode(dp);
	else if(reply("CLEAR") == YES)
		clrinode(dp);
}


clrinode(dp)		/* quietly clear inode */
register DINODE *dp;
{

	n_files--;
	pfunc = pass4;
	ckinode(dp,ADDR);
	zapino(dp);
	inodirty();
}

chkempt(dp)
register DINODE *dp;
{
	register daddr_t *ap;
	register DIRECT *dirp;
	daddr_t blk[NADDR];
	int size;

	size = minsz(dp->di_size, (NADDR - 3) * XXBSIZE);
	l3tol(blk,dp->di_addr,NADDR);
	for(ap = blk; ap < &blk[NADDR - 3], size > 0; ap++) 
	{   if(*ap) 
	    {   if(outrange(*ap)) 
		{   fprntf(stderr,"chkempt: blk %d out of range\n",*ap);
		    return(SKIP);
		}
		if(getblk(&fileblk,*ap) == NULL) 
		{   fprntf(stderr,"chkempt: Can't find blk %d\n",*ap);
		    return(SKIP);
		}
		for(dirp=dirblk; dirp < &dirblk[NDIRECT],size > 0; dirp++) 
		{   if(dirp->d_name[0] == '.' && (dirp->d_name[1] == '\0' 
			|| (dirp->d_name[1] == '.' && dirp->d_name[2] == '\0'))) 
		    {   size -= sizeof(DIRECT);
			continue;
		    }
		    if(dirp->d_ino)
			return(NO);
		    size -= sizeof(DIRECT);
		}
	    }
	}
	if(size <= 0)	return(YES);
	else	return(NO);
}
/* M011 end */

/*	M012
 * Save all the output of fsck in the recover device
 * if the '-a' flag is set and we successfully opened
 * the recover device.
 */
fprntf(fp, fmt, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, x13, x14)
{
	if(aflag && rcv_out != NULL)
		fprintf(rcv_out, fmt, x1, x2, x3, x4, x5, x6, x7,
					 x8, x9, x10, x11, x12, x13, x14);
	fprintf(fp, fmt, x1, x2, x3, x4, x5, x6, x7,
					 x8, x9, x10, x11, x12, x13, x14);
}

FILE *
append_open(fname)		/*M014*/
char *fname;
{
	int ch;
	long cnt=0;
	FILE *fp;

	if (  (fp=fopen(fname, "r")) == NULL)
		return(NULL);
	while( (ch = getc(fp)) != -1 ) {
		if( (ch==0) || (ch>127) )
			break;
		cnt++;
	}
	if (  (fp=fopen(fname, "a")) == NULL)
		return(NULL);
	fseek(fp, cnt, 0);		/* don't complain */
	return(fp);
}
