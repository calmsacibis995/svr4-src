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

#ident	"@(#)xx:cmd/fsck/fsck2.c	1.2.1.1"

/*
 *      Derived from XENIX fsck2.c 1.4 87/05/01
 */
#include <sys/sysmacros.h>
#include "fsck.h"
/* #include "../h/std.h" */

setup(dev)
char *dev;
{
	register n;
	register BUFAREA *bp;
	register MEMSIZE msize;
	register char *mbase;                           /*M001*/
	daddr_t bcnt, nscrblk;
	dev_t rootdev;
	off_t smapsz, lncntsz, totsz;
	struct ustat ustatarea;
	struct stat statarea;

	if(stat("/",&statarea) < 0)
		errexit("Can't stat root\n");
	rootdev = statarea.st_dev;
	if(stat(dev,&statarea) < 0) {
		error("Can't stat %s\n",dev);
		return(NO);
	}
	hotroot = 0;
	rawflg = 0;

	/* Begin M007 */
	if((statarea.st_mode & S_IFMT) == S_IFBLK) {
		if(rootdev == statarea.st_rdev)
			hotroot = 1;
		else {
			if(rrflag) {
				fprntf(stderr,"Option -rr ignored\n");
				rrflag = 0;
			}
			if(ustat(statarea.st_rdev, (struct ustat *)&ustatarea) >= 0 && !nflag) {
				error("Can't clean mounted file system: %s\n",dev);
				return(NO);
			}
		}
		/* M011 begin */
		if ( pipedev == statarea.st_rdev )
		{	error( "%s is pipedev, ignored", dev);
			return(NO);
		}
		/* M011 end */
	}
	else if((statarea.st_mode & S_IFMT) == S_IFCHR)
		rawflg++;
	else {
		if (reply("file is not a block or character device; OK") == NO)
			return(NO);
	}
	/* End M007 */

	if((dfile.rfdes = open(dev,0)) < 0) {
		error("Can't open %s\n",dev);
		return(NO);
	}
	fprntf(stderr,"\n%s",dev);
	/* M011 csflag check */
	if((nflag && !csflag) || (dfile.wfdes = open(dev,1)) < 0) {
		dfile.wfdes = -1;
		fprntf(stderr," (NO WRITE)");
	}
	fprntf(stderr,"\n");
	pss2done = 0;				/* M011 */
	fixfree = 0;
	dfile.mod = 0;
	n_files = n_blks = n_free = 0;
	muldup = enddup = &duplist[0];
	badlnp = &badlncnt[0];
	lfdir = 0;
	rplyflag = 0;
	initbarea(&sblk);
	initbarea(&fileblk);
	initbarea(&inoblk);
	sfile.wfdes = sfile.rfdes = -1;
	rmscr = 0;
	if(getblk(&sblk,SUPERB) == NULL) {
		ckfini();
		return(NO);
	}

#ifdef M_I386
	if (superblk.s_magic == S_S3MAGIC) {
 		Fsver = S_V3;
 	}
 	else {
		fprintf(stderr,"CANNOT CLEAN - NOT A XENIX FILE SYSTEM\n");
 		ckfini();
 		return(NO);
 	}
 	if (cflag) {
 		fprintf(stderr,"FILE SYSTEM %s ALREADY UPDATED\n",dev);
 	}
#else
	/*
	 * M006
	 * Determine the version of the superblock and if necessary
	 * convert the superblock to the version supported by the
	 * current version of the kernel.
	 */
	if((Fsver = versfs(&superblk)) == -1) {
		fprntf(stderr,"UNKNOWN FILE SYSTEM VERSION %u\n",Fsver);
		ckfini();
		return(NO);
	}
	if((Fsver == S_V3) && (cflag)) {
		fprntf(stderr,"FILE SYSTEM %s ALREADY UPDATED\n",dev);
/***		cflag = 0;	***/
	}
	if(Fsver != S_V3) {
		fprntf(stderr,"CLEANING NON SYSTEM 3 FILE SYSTEM\n");
		if(cvtfs(&superblk,Fsver,S_V3) == -1) {
			fprntf(stderr,"TROUBLE IN CONVERTING FILE SYSTEM\n");
			ckfini();
			return(NO);
		}
	}
#endif
	imax = ((ino_t)superblk.s_isize - (SUPERB+1)) * INOPB;
	fmin = (daddr_t)superblk.s_isize;	/* first data blk num */
	fmax = superblk.s_fsize;		/* first invalid blk num */
	if(fmin >= fmax || 
		(imax/INOPB) != ((ino_t)superblk.s_isize-(SUPERB+1))) {
		error("Size check: fsize %ld isize %d\n",
			superblk.s_fsize,superblk.s_isize);
		ckfini();
		return(NO);
	}
	if (superblk.s_fname[0])
	    fprntf(stderr,"File System: %.6s ", superblk.s_fname);
	if (superblk.s_fpack[0])
	    fprntf(stderr,"Volume: %.6s",superblk.s_fpack);
	if (superblk.s_fname[0] || superblk.s_fpack[0])
	    fprntf(stderr,"\n\n");
	bmapsz = roundup(howmany(fmax,BITSPB),sizeof(*lncntp));
	smapsz = roundup(howmany((long)(imax+1),STATEPB),sizeof(*lncntp));
	lncntsz = (long)(imax+1) * sizeof(*lncntp);
	if(bmapsz > smapsz+lncntsz)
		smapsz = bmapsz-lncntsz;
	totsz = bmapsz+smapsz+lncntsz;
	msize = memsize;
	mbase = membase;
	if(rawflg) {
		if(msize < (MEMSIZE)(NINOBLK*XXBSIZE) + 2*sizeof(BUFAREA))
			rawflg = 0;
		else {
			msize -= (MEMSIZE)NINOBLK*XXBSIZE;
			mbase += (MEMSIZE)NINOBLK*XXBSIZE;
			niblk = NINOBLK;
			startib = fmax;
		}
	}
	clear(mbase,msize);
	if((off_t)msize < totsz) {
		bmapsz = roundup(bmapsz,XXBSIZE);
		smapsz = roundup(smapsz,XXBSIZE);
		lncntsz = roundup(lncntsz,XXBSIZE);
		nscrblk = (bmapsz+smapsz+lncntsz)>>BSHIFT;
		if(tflag == 0) {
			fprntf(stderr,"\nNEED SCRATCH FILE (%ld BLKS)\n",nscrblk);
			/* M013 */
			do {
				fprntf(stderr,"ENTER FILENAME:  ");
				if((n = getline(stdin,scrfile,sizeof(scrfile))) == EOF)
					errexit("\n");
			} while(n == 0);
		}
		if(stat(scrfile,&statarea) < 0 ||
			(statarea.st_mode & S_IFMT) == S_IFREG)
			rmscr++;
 		/*
 		 * If we are trying to recover the root file system and
 		 * the user enters a regular file as the scratch file
 		 * rather than a device (eg. /dev/scratch) we panic.
 		 */
 		if (rrflag && ((statarea.st_mode & S_IFMT) == S_IFREG)) {
 			fprintf(stderr,"fsck PANIC: cannot create scratch");
 			fprintf(stderr," file during root recovery!\n");
 			for (;;)                /* hang */
 				hotroot = 0;    /* avoid c81 BUG */
 		}
		if((sfile.wfdes = creat(scrfile,0666)) < 0 ||
			(sfile.rfdes = open(scrfile,0)) < 0) {
			error("Can't create %s\n",scrfile);
			ckfini();
			return(NO);
		}
		bp = &((BUFAREA *)mbase)[(msize/sizeof(BUFAREA))];
		poolhead = NULL;
		while(--bp >= (BUFAREA *)mbase) {
			initbarea(bp);
			bp->b_next = poolhead;
			poolhead = bp;
		}
		bp = poolhead;
		for(bcnt = 0; bcnt < nscrblk; bcnt++) {
			bp->b_bno = bcnt;
			dirty(bp);
			flush(&sfile,bp);
		}
		blkmap = freemap = statemap = (char *) NULL;
		lncntp = (short *) NULL;
		smapblk = bmapsz / XXBSIZE;
		lncntblk = smapblk + smapsz / XXBSIZE;
		fmapblk = smapblk;
	}
	else {
		if(rawflg && (off_t)msize > totsz+XXBSIZE) {
			niblk += (unsigned)((off_t)msize-totsz)>>BSHIFT;
			if(niblk > MAXRAW)
				niblk = MAXRAW;
			msize = memsize - (niblk*XXBSIZE);
			mbase = membase + (niblk*XXBSIZE);
		}
		poolhead = NULL;
		blkmap = mbase;
		statemap = &mbase[(MEMSIZE)bmapsz];
		freemap = statemap;
		lncntp = (short *)&statemap[(MEMSIZE)smapsz];
	}
	return(YES);
}


DINODE *
ginode()
{
	register DINODE *dp;
	register char *mbase;
	daddr_t iblk;

	if(inum > imax)
		return(NULL);
	iblk = itod(inum);
	if(rawflg) {
		mbase = membase;
		if(iblk < startib || iblk >= startib+niblk) {
			if(inoblk.b_dirty)
				bwrite(&dfile,mbase,startib,(int)niblk*XXBSIZE);
			inoblk.b_dirty = 0;
			if(bread(&dfile,mbase,iblk,(int)niblk*XXBSIZE) == NO) {
				startib = fmax;
				return(NULL);
			}
			startib = iblk;
		}
		dp = (DINODE *)&mbase[(unsigned)((iblk-startib)<<BSHIFT)];
	}
	else if(getblk(&inoblk,iblk) != NULL)
		dp = inoblk.b_un.b_dinode;
	else
		return(NULL);
	return(dp + itoo(inum));
}


ftypeok(dp)
DINODE *dp;
{
	switch(dp->di_mode & IFMT) {
		case IFDIR:
		case IFREG:
		case IFBLK:
		case IFCHR:
		case IFIFO:			/* M006 */
#ifdef IFNAM                                    /* M005 */
		case IFNAM:
#endif
			return(YES);
	}

	/*
	 * M006
	 * Multiplexed files were removed from system 3
	 * but are still legal for version 7
	 */
#ifndef M_I386
	if(Fsver != S_V3) {
		switch(dp->di_mode & IFMT) {
			case IFMPC:
			case IFMPB:
				return(YES);
		}
	}
#endif
	return(NO);
}

reply(s)
char *s;
{
	char line[80];

	rplyflag = 1;
	fprntf(stderr,"\n%s? ",s);
	if(nflag || csflag || dfile.wfdes < 0) {
		fprntf(stderr," no\n\n");
		return(NO);
	}
	if(yflag) {
		fprntf(stderr," yes\n\n");
		return(YES);
	}
	if(getline(stdin,line,sizeof(line)) == EOF)
		errexit("\n");
	fprntf(stderr,"\n");
	if(line[0] == 'y' || line[0] == 'Y')
		return(YES);
	else
		return(NO);
}


getline(fp,loc,maxlen)
FILE *fp;
char *loc;
{
	register n;
	register char *p, *lastloc;

	p = loc;
	lastloc = &p[maxlen-1];
	while((n = getc(fp)) != '\n') {
		if(n == EOF)
			return(EOF);
		if(!isspace(n) && p < lastloc)
			*p++ = n;
	}
	*p = 0;
	/*   M012   If we are saving output in the recover
	 * device save what the user types as well.
	 */
	if (aflag && rcv_out != NULL)		/*M012*/
		fprintf(rcv_out,"%s\n", loc);
	return(p - loc);
}


stype(p)
register char *p;
{
	if(*p == 0)
		return;
	if (*(p+1) == 0) {
		if (*p == '3') {
			cylsize = 200;
			stepsize = 5;
			return;
		}
		if (*p == '4') {
			cylsize = 418;
			stepsize = 9;
			return;
		}
	}
	cylsize = atoi(p);
	while(*p && *p != ':')
		p++;
	if(*p)
		p++;
	stepsize = atoi(p);
	if(stepsize <= 0 || stepsize > cylsize ||
	cylsize <= 0 || cylsize > MAXCYL) {
		error("Invalid -s argument, defaults assumed\n");
		cylsize = stepsize = 0;
	}
}


dostate(s,flg)
{
	register char *p;
	register unsigned byte, shift;
	register BUFAREA *bp;

	byte = (inum)/STATEPB;
	shift = LSTATE * ((inum)%STATEPB);
	if(statemap != NULL) {
		bp = NULL;
		p = &statemap[byte];
	}
	else if((bp = getblk((BUFAREA *)NULL,(daddr_t)(smapblk+(byte/XXBSIZE)))) == NULL)
		errexit("Fatal I/O error\n");
	else
		p = &bp->b_un.b_buf[byte%XXBSIZE];
	switch(flg) {
		case 0:
			*p &= ~(SMASK<<(shift));
			*p |= s<<(shift);
			if(bp != NULL)
				dirty(bp);
			return(s);
		case 1:
			return((*p>>(shift)) & SMASK);
	}
	return(USTATE);
}


domap(blk,flg)
daddr_t blk;
{
	register char *p;
	register unsigned n;
	register BUFAREA *bp;
	off_t byte;

	byte = blk >> BITSHIFT;
	n = 1<<((unsigned)(blk & BITMASK));
	if(flg & 04) {
		p = freemap;
		blk = fmapblk;
	}
	else {
		p = blkmap;
		blk = 0;
	}
	if(p != NULL) {
		bp = NULL;
		p += (unsigned)byte;
	}
	else if((bp = getblk((BUFAREA *)NULL,blk+(byte>>BSHIFT))) == NULL)
		errexit("Fatal I/O error\n");
	else
		p = &bp->b_un.b_buf[(unsigned)(byte&BMASK)];
	switch(flg&03) {
		case 0:
			*p |= n;
			break;
		case 1:
			n &= *p;
			bp = NULL;
			break;
		case 2:
			*p &= ~n;
	}
	if(bp != NULL)
		dirty(bp);
	return(n);
}


dolncnt(val,flg)
short val;
{
	register short *sp;
	register BUFAREA *bp;

	if(lncntp != NULL) {
		bp = NULL;
		sp = &lncntp[inum];
	}
	else if((bp = getblk((BUFAREA *)NULL,(daddr_t)(lncntblk+(inum/SPERB)))) == NULL)
		errexit("Fatal I/O error\n");
	else
		sp = &bp->b_un.b_lnks[inum%SPERB];
	switch(flg) {
		case 0:
			*sp = val;
			break;
		case 1:
			bp = NULL;
			break;
		case 2:
			(*sp)--;
	}
	if(bp != NULL)
		dirty(bp);
	return(*sp);
}


BUFAREA *
getblk(bp,blk)
register BUFAREA *bp;
daddr_t blk;
{
	int rc;
	register struct filecntl *fcp;

	if(bp == NULL) {
		bp = search(blk);
		fcp = &sfile;
	}
	else
		fcp = &dfile;
	if(bp->b_bno == blk)
		return(bp);
	flush(fcp,bp);

	if(bread(fcp,bp->b_un.b_buf,blk,XXBSIZE) == NO) {
		bp->b_bno = (daddr_t)-1;
		return(NULL);
	}

	bp->b_bno = blk;
	return(bp);
}


flush(fcp,bp)
struct filecntl *fcp;
register BUFAREA *bp;
{
	if((bp->b_bno == SUPERB) && (cflag))
		bp->b_dirty = 1;

	if(bp->b_dirty) {
#ifndef M_I386
		if((bp->b_bno == SUPERB) && (Fsver != S_V3)) {
			if(cflag) {
				if(reply("CONVERT FILE SYSTEM") == YES) {
					goto write;
				}
			}
			if(cvtfs(&superblk,S_V3,Fsver) == -1) {
				fprntf(stderr,"TROUBLE IN CONVERTING FILE SYSTEM\n");
				return;
			}
		}
#endif
	write:
		bwrite(fcp,bp->b_un.b_buf,bp->b_bno,XXBSIZE);
	}
	bp->b_dirty = 0;
}


rwerr(s,blk)
char *s;
daddr_t blk;
{
	fprntf(stderr,"\nCAN NOT %s: BLK %ld",s,blk);
	if(reply("CONTINUE") == NO)
		errexit("Program terminated\n");
}


sizechk(dp)
register DINODE *dp;
{
	off_t size, nblks;

	size = howmany(dp->di_size,XXBSIZE);
	nblks = size;
	size -= NADDR-3;
	while(size > 0) {
		nblks += howmany(size,NINDIR);
		size--;
		size /= NINDIR;
	}
	if ( !qflag )
	{   if(!FIFO && nblks != filsize) 	/* M012 */
	        fprntf(stderr,"POSSIBLE FILE SIZE ERROR I=%u\n\n",inum);
	    if(DIR && (dp->di_size % sizeof(DIRECT)) != 0) 
	        fprntf(stderr,"DIRECTORY MISALIGNED I=%u\n\n",inum);
	}
}


ckfini()
{
	flush(&dfile,&fileblk);
	flush(&dfile,&sblk);
	flush(&dfile,&inoblk);
	close(dfile.rfdes);
	close(dfile.wfdes);
	close(sfile.rfdes);
	close(sfile.wfdes);
	if(rmscr) {
		unlink(scrfile);
	}
}


pinode()
{
	register DINODE *dp;
	register char *p;
	char uidbuf[200];
	char *ctime();

	fprntf(stderr," I=%u ",inum);
	if((dp = ginode()) == NULL)
		return;
	fprntf(stderr," OWNER=");
	if(getpw((int)dp->di_uid,uidbuf) == 0) {
		for(p = uidbuf; *p != ':'; p++);
		*p = 0;
		fprntf(stderr,"%s ",uidbuf);
	}
	else {
		fprntf(stderr,"%d ",dp->di_uid);
	}
	fprntf(stderr,"MODE=%o\n",dp->di_mode);
	fprntf(stderr,"SIZE=%ld ",dp->di_size);
	p = ctime(&dp->di_mtime);
	fprntf(stderr,"MTIME=%12.12s %4.4s ",p+4,p+20);
}


copy(fp,tp,size)
register char *tp, *fp;
MEMSIZE size;
{
	while(size--)
		*tp++ = *fp++;
}


freechk()
{
	register daddr_t *ap;

	if(freeblk.df_nfree == 0)
		return;
	do {
		if(freeblk.df_nfree <= 0 || freeblk.df_nfree > NICFREE) {
			fprntf(stderr,"BAD FREEBLK COUNT\n");
			fixfree = 1;
			return;
		}
		ap = &freeblk.df_free[freeblk.df_nfree];
		while(--ap > &freeblk.df_free[0]) {
			if(pass5(*ap) == STOP)
				return;
		}
		if(*ap == (daddr_t)0 || pass5(*ap) != KEEPON)
			return;
	} while(getblk(&fileblk,*ap) != NULL);
}


makefree()
{
	register i, cyl, step;
	int j;
	char flg[MAXCYL];
	short addr[MAXCYL];
	daddr_t blk, baseblk;

	superblk.s_nfree = 0;
	superblk.s_flock = 0;
	superblk.s_fmod = 0;
	superblk.s_tfree = 0;
	superblk.s_ninode = 0;
	superblk.s_ilock = 0;
	superblk.s_ronly = 0;
	if(cylsize == 0 || stepsize == 0) {
		step = superblk.s_dinfo[0];
		cyl = superblk.s_dinfo[1];
	}
	else {
		step = stepsize;
		cyl = cylsize;
	}
	if(step > cyl || step <= 0 || cyl <= 0 || cyl > MAXCYL) {
		error("Default free list spacing assumed\n");
		step = STEPSIZE;
		cyl = CYLSIZE;
	}
	superblk.s_dinfo[0] = step;
	superblk.s_dinfo[1] = cyl;
	clear(flg,sizeof(flg));
	i = 0;
	for(j = 0; j < cyl; j++) {
		while(flg[i])
			i = (i + 1) % cyl;
		addr[j] = i + 1;
		flg[i]++;
		i = (i + step) % cyl;
	}
	baseblk = (daddr_t)roundup(fmax,cyl);
	clear((char *)&freeblk,XXBSIZE);
	freeblk.df_nfree++;
	for( ; baseblk > 0; baseblk -= cyl)
		for(i = 0; i < cyl; i++) {
			blk = baseblk - addr[i];
			if(!outrange(blk) && !getbmap(blk)) {
				superblk.s_tfree++;
				if(freeblk.df_nfree >= NICFREE) {
					fbdirty();
					fileblk.b_bno = blk;
					flush(&dfile,&fileblk);
					clear((char *)&freeblk,XXBSIZE);
				}
				freeblk.df_free[freeblk.df_nfree] = blk;
				freeblk.df_nfree++;
			}
		}
	superblk.s_nfree = freeblk.df_nfree;
	for(i = 0; i < NICFREE; i++)
		superblk.s_free[i] = freeblk.df_free[i];
	sbdirty();
}


clear(p,cnt)
register char *p;
register MEMSIZE cnt;                           /*M001*/
{
	while(cnt--)
		*p++ = 0;
}


BUFAREA *
search(blk)
daddr_t blk;
{
	register BUFAREA *pbp, *bp;

	for(bp = (BUFAREA *) &poolhead; bp->b_next; ) {
		pbp = bp;
		bp = pbp->b_next;
		if(bp->b_bno == blk)
			break;
	}
	pbp->b_next = bp->b_next;
	bp->b_next = poolhead;
	poolhead = bp;
	return(bp);
}


findino(dirp)
register DIRECT *dirp;
{
	register char *p1, *p2;

	if(dirp->d_ino == 0)
		return(KEEPON);
	for(p1 = dirp->d_name,p2 = srchname;*p2++ == *p1; p1++) {
		if(*p1 == 0 || p1 == &dirp->d_name[DIRSIZ-1]) {
			if(dirp->d_ino >= ROOTINO && dirp->d_ino <= imax)
				parentdir = dirp->d_ino;
			return(STOP);
		}
	}
	return(KEEPON);
}


mkentry(dirp)
register DIRECT *dirp;
{
	register ino_t in;
	register char *p;

	if(dirp->d_ino)
		return(KEEPON);
	dirp->d_ino = orphan;
	in = orphan;
	/*
	 * M002
	 *	Zero out from end of directory entry, rather than just
	 *	one character position.
	 */
	for (p = &dirp->d_name[DIRSIZ]; p > &dirp->d_name[6]; *--p = '\0')
		;
	while(p > dirp->d_name) {
		*--p = (in % 10) + '0';
		in /= 10;
	}
	return(ALTERD|STOP);
}


chgdd(dirp)
register DIRECT *dirp;
{
	if(dirp->d_name[0] == '.' && dirp->d_name[1] == '.' &&
	dirp->d_name[2] == 0) {
		dirp->d_ino = lfdir;
		return(ALTERD|STOP);
	}
	return(KEEPON);
}


linkup()
{
	register DINODE *dp;
	int lostdir;
	register ino_t pdir;
	register ino_t *blp;
	int n;

	if((dp = ginode()) == NULL)
		return(NO);
	lostdir = DIR;
	pdir = parentdir;
	/* M011 begin */
	if(!FIFO || !qflag || nflag) 
	{   fprntf(stderr,"UNREF %s ",lostdir ? "DIR" : "FILE");
	    pinode();
	}
	if(DIR) 
	{   if(dp->di_size > EMPT) 
	    {   if((n = chkempt(dp)) == NO) 
		{   fprntf(stderr," (NOT EMPTY)");
		    if(!nflag) 
		    {   fprntf(stderr," MUST reconnect\n");
			goto connect;
		    }
		    else
			fprntf(stderr,"\n");
		}
		else if(n != SKIP) 
		{   fprntf(stderr," (EMPTY)");
		    if(!nflag) 
		    {   fprntf(stderr," Cleared\n");
			return(REM);
		    }
		    else
			fprntf(stderr,"\n");
		}
	    }
	    else 
	    {   fprntf(stderr," (EMPTY)");
		if(!nflag) 
		{   fprntf(stderr," Cleared\n");
		    return(REM);
		}
		else
		    fprntf(stderr,"\n");
	    }
	}
	if(REG)
	    if(!dp->di_size) 
	    {   fprntf(stderr," (EMPTY)");
		if(!nflag) 
		{   fprntf(stderr," Cleared\n");
		    return(REM);
		}
		else
		    fprntf(stderr,"\n");
	    }
	    else
		fprntf(stderr," (NOT EMPTY)\n");
	if(FIFO && !nflag) 
	{   if(!qflag)	fprntf(stderr," -- REMOVED");
	    fprntf(stderr,"\n");
	    return(REM);
	}
	if(FIFO && nflag)
		return(NO);
	if(reply("RECONNECT") == NO)
		return(NO);
connect:
	/* M011 end */
	orphan = inum;
	if(lfdir == 0) {
		inum = ROOTINO;
		if((dp = ginode()) == NULL) {
			inum = orphan;
			return(NO);
		}
		pfunc = findino;
		srchname = lfname;
		filsize = dp->di_size;
		parentdir = 0;
		ckinode(dp,DATA);
		inum = orphan;
		if((lfdir = parentdir) == 0) {
			fprntf(stderr,"SORRY. NO lost+found DIRECTORY\n\n");
			return(NO);
		}
	}
	inum = lfdir;
	if((dp = ginode()) == NULL || !DIR || getstate() != FSTATE) {
		inum = orphan;
		fprntf(stderr,"SORRY. NO lost+found DIRECTORY\n\n");
		return(NO);
	}
	if(dp->di_size & BMASK) {
		dp->di_size = roundup(dp->di_size,XXBSIZE);
		inodirty();
	}
	filsize = dp->di_size;
	inum = orphan;
	pfunc = mkentry;
	if((ckinode(dp,DATA) & ALTERD) == 0) {
		fprntf(stderr,"SORRY. NO SPACE IN lost+found DIRECTORY\n\n");
		return(NO);
	}
	declncnt();
	if((dp = ginode()) && !dp->di_nlink) 
	{   dp->di_nlink++;
	    inodirty();
	    setlncnt(getlncnt()+1);
	    if(lostdir) 
	    {   for(blp = badlncnt; blp < badlnp; blp++)
		    if(*blp == inum) 
		    {   *blp = 0L;
			break;
		    }
	    }
	}
	if(lostdir) {
		pfunc = chgdd;
		/* dp = ginode(); M011 */
		filsize = dp->di_size;
		ckinode(dp,DATA);
		inum = lfdir;
		if((dp = ginode()) != NULL) {
			dp->di_nlink++;
			inodirty();
			setlncnt(getlncnt()+1);
		}
		inum = orphan;
		fprntf(stderr,"DIR I=%u CONNECTED. ",orphan);
		fprntf(stderr,"PARENT WAS I=%u\n\n",pdir);
	}
	return(YES);
}


bread(fcp,buf,blk,size)
daddr_t blk;
register struct filecntl *fcp;
register size;
char *buf;
{
	if(lseek(fcp->rfdes,blk<<BSHIFT,0) < 0)
		rwerr("SEEK",blk);
	else if(read(fcp->rfdes,buf,size) == size)
		return(YES);
	rwerr("READ",blk);
	return(NO);
}


bwrite(fcp,buf,blk,size)
daddr_t blk;
register struct filecntl *fcp;
register size;
char *buf;
{
	if(fcp->wfdes < 0)
		return(NO);
	if(lseek(fcp->wfdes,blk<<BSHIFT,0) < 0)
		rwerr("SEEK",blk);
	else if(write(fcp->wfdes,buf,size) == size) {
		fcp->mod = 1;
		return(YES);
	}
	rwerr("WRITE",blk);
	return(NO);
}

void
catch()
{
	ckfini();
	exit(4);
}
