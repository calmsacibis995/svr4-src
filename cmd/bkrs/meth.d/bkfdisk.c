/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:meth.d/bkfdisk.c	1.13.2.1"

#include	<limits.h> 	/* get PATH_MAX from here, not stdio */
#include	<sys/types.h>
#include	<stdio.h>
#include	<fcntl.h>
#include	<string.h>
#include	<signal.h>
#include	<backup.h>
#include	<bktypes.h>
#include	<bkrs.h>
#include	<brarc.h>
#include 	"libadmIO.h"
#include	<method.h>
#include	<setjmp.h>
#include	<errno.h>

extern int		bknewvol();
extern struct br_arc	*bld_hdr();
extern int		brlog();
extern int		brsndfname();
extern int		chk_vol_sum();
extern int		close();
extern void		do_history();
extern GFILE		*g_open();
extern int		g_write();
extern int		strfind();
extern void		sum();
extern time_t		time();

extern int	bklevels;

short		reuse_dmname = 0;
char		fname[1025];
char		tocname[] = "";
unsigned	Vol_sum = 0;		/* for -v option */
jmp_buf		env;
m_info_t	*MP;

static int	fsinfo();
static GFILE	*openfd();

static int		bytes_summed = 0;
static GFILE		*Fdisk = NULL;
static media_info_t	IMM;

do_bkfdisk(mp)
m_info_t *mp;
{
	FILE	*pv;
	FILE	*lv;
	char	pvtoc[512];
	char	fsdev[128];
	char	mntpt[256];
	char	fn[256];
	char	slice;
	char	*wrk;
	int	flags;
	int	ret;
	int	isfile;
	int	devlen;
	int	len;
	int	errors = 0;
	short	numfs = 0;

	mp->bkdate = time((long *) 0);
	MP = mp;
	ret = setjmp(env);

	if (ret)  {				  /* error has occurred */
		brlog(" do_bkfdisk ret=%d ", ret);
		mp->blk_count >>= 9;
		return(1);
	}
newarc:
	if ((isfile = bknewvol(mp, fname, &reuse_dmname, &env, &IMM)) < 0) {
		brlog(" bknewvol failed for %s", fname);
		sprintf(ME(mp), "Job ID %s: new volume failed for %s", mp->jobid, fname);
		return(1);
	}
	BEGIN_CRITICAL_REGION;
	if (isfile) {
		Fdisk = g_open(fname, (O_WRONLY|O_CREAT|O_TRUNC), 0644);

		if (Fdisk == NULL) {
			brlog(" openfd: cannot create %s %s ", fname, SE);
			sprintf(ME(mp), "Job ID %s: g_open failed for %s: %s", mp->jobid, fname, SE);
			return(1);
		}
	}
	/* else wait until doing the br_write_hdr to open */
	/* This is done since the header needs to be read */
	/* and verified prior to writing and some devices */
	/* will not allow closing and reopenning the      */
	/* device without changing the media. (e.g. the   */
	/* tapes will write a file mark on close.)        */
	END_CRITICAL_REGION;

	ret = wr_bkrs_hdr();
	if (ret < 0) {
		brlog("unable to write bkrs hdr on archive");
		sprintf(ME(mp), "Job ID %s: write of archive header failed for %s", mp->jobid, fname);
		return(1);
	}
	(void) strcpy(fsdev, mp->ofsdev);
	devlen = strlen(fsdev);

	(void) sprintf(pvtoc, "/usr/sbin/prtvtoc %s", mp->ofsdev);
#ifdef TRACE
	brlog("pvtoc = %s", pvtoc);
#endif
	BEGIN_CRITICAL_REGION;

	pv = popen(pvtoc, "r");

	END_CRITICAL_REGION;

	if (pv == NULL) {
		brlog("cannot prtvtoc of %s", mp->ofsdev);
		sprintf(ME(mp), "Job ID %d: prtvtoc failed for %s", mp->jobid, mp->ofsdev);
		return(1);
	}
	mntpt[0] = 0;

	while(1) {
		BEGIN_CRITICAL_REGION;

		wrk = fgets(pvtoc, 512, pv);

		END_CRITICAL_REGION;

		if (wrk == NULL)
			break;

		len = strlen(pvtoc);

		ret = g_write(Fdisk, pvtoc, len);

		bytes_summed += len;
		sum (pvtoc, (long) len, &Vol_sum);

		mp->blk_count += len;

		if (pvtoc[0] == '*')
			continue;

		(void) sscanf(pvtoc, " %c %*d %d %*d %*d %*d %s",&slice,&flags,mntpt);

		if (flags & 1) {	/* NOT mountable */
			brlog("slice %c NOT mountable", slice);
			(void) sprintf(fn, "dp  %s", fsdev);
			mntpt[0] = NULL;
		}
		else {
			fsdev[devlen-1] = slice;
			++numfs;
			errors += fsinfo(fsdev, mp, slice, mntpt);
			(void) sprintf(fn, "fs  %s  ", fsdev);
			(void) strcat(fn, mntpt);
		}
		if (mp->flags & Vflag) {
			(void) brsndfname(fn);
		}
		mntpt[0] = NULL;
	}
	BEGIN_CRITICAL_REGION;

	ret = pclose(pv);

	END_CRITICAL_REGION;

	if ((ret != 0) && !numfs) {	/* did popen already wait for prtvtoc */
		brlog("prtvtoc exit code 0x%x %s",ret,SE);
		sprintf(ME(mp), "Job ID %s: prtvtoc failed for %s", mp->jobid, mp->ofsdev);
		return(1);
	}
	if (errors) {
		brlog("labelit errors");
		sprintf(ME(mp), "Job ID %s: labelit failed for %s", mp->jobid, mp->ofsdev);
		return(1);
	}
	if (mp->flags & vflag) {
		if (chk_vol_sum(mp, &Fdisk,
				(long) bytes_summed, fname, Vol_sum)) {
			(void) g_close(Fdisk);
			goto newarc;
		}
	}
	(void) g_close(Fdisk);

	if (IMM.cur) {			/* last vol was good */
		if (IMM.first == NULL) 
			IMM.first = IMM.cur;
		if (IMM.last)
			(IMM.last)->next = IMM.cur;
		IMM.last = IMM.cur;
	}
	mp->blk_count += 511;
	mp->blk_count >>= 9;

	do_history(mp, &IMM, -1);

	return(0);
} /* do_bkfdisk() */

static int
fsinfo(dev, mp, slice, mntpt)
char	*dev;
m_info_t *mp;
char	slice;
char	*mntpt;
{
	char	cmd[64];
	char	buf[512];
	char	*mntnm;
	FILE	*la;
	int	len;
	int	ret;
	int	i;

	(void) sprintf(cmd, "/sbin/labelit %s", dev);
#ifdef TRACE
	brlog("labelit command=%s", cmd);
#endif
	BEGIN_CRITICAL_REGION;

	la = popen(cmd, "r");

	END_CRITICAL_REGION;

	if (la == NULL) {
		brlog("cannot read label of %s", dev);
		return(1);
	}
	buf[0] = '#';
	buf[1] = slice;
	buf[2] = NULL;

	BEGIN_CRITICAL_REGION;

	len = strlen(buf);

	while (fgets((buf+len+1), 512-len-1, la) != NULL) {
		buf[len] = ' ';
		len = strlen(buf);
		buf[len-1] = ' ';
		buf[len] = '\n';
		buf[len+1] = NULL;
	}
	if ((i = strfind (buf, "Current fsname: ")) >= 0) {
		int	j = 0;
		char	*label;

		i += 16;
		label = &buf[i];

		while ((j < 9) && (label[j++] != ','));

		label[--j] = NULL;
		brlog("label %s found for slice %c", label, slice);

		mntnm = strrchr (mntpt, '/');

		if (strcmp (mntnm+1, label) != 0) {
			brlog("labelit mismatch for slice %c",slice);
			brlog("label=%s, mntpt=%s, mntnm=%s",label, mntpt, mntnm);
		}
		label[j] = ',';
	}
	(void) pclose(la);

	END_CRITICAL_REGION;

	(void) sprintf(cmd, "/sbin/mkfs -m %s", dev);
#ifdef TRACE
	brlog("mkfs command=%s", cmd);
#endif
	BEGIN_CRITICAL_REGION;

	la = popen(cmd, "r");

	END_CRITICAL_REGION;

	if (la == NULL) {
		brlog("cannot read mkfs -m of %s", dev);
		return(1);
	}
	BEGIN_CRITICAL_REGION;

	if (fgets((buf+len+1), 512-len-1, la) == NULL) {
		brlog("cannot read mkfs output from %s - Hopefully we won't need it!", dev);
	}
	buf[len] = ' ';
	len = strlen(buf);
	buf[len-1] = ' ';
	buf[len] = '\n';
	buf[len+1] = NULL;

	if (fgets(cmd, 512, la) != NULL) {
		brlog("mkfs output from %s more than one line? - Hopefully this won't screw me up!", dev);
	}
	END_CRITICAL_REGION;
	len = strlen(buf);
	ret = g_write(Fdisk, buf, len);

	if (ret != len) {
		brlog("archive write error %s",SE);
		return(1);
	}
	bytes_summed += len;
	sum (buf, (long) len, &Vol_sum);
	mp->blk_count += len;

	BEGIN_CRITICAL_REGION;

	(void) pclose(la);

	END_CRITICAL_REGION;

	return(0);
} /* fsinfo() */


int
wr_bkrs_hdr()
{
	int	i;
	int	hdrsize;
	long	nbytes;
	char	*typstrng;
	struct br_arc	*hdr;
	struct wr_archive_hdr	brhd;
	struct wr_archive_hdr	*b = &brhd;
	struct bld_archive_info	brai;
	struct bld_archive_info	*ai = &brai;

	Vol_sum = 0;		/* for -v option */
	bytes_summed = 0;

	ai->br_method    = MN(MP);		/* method name */
	ai->br_fsname    = OFS(MP);		/* file system name */
	ai->br_dev       = ODEV(MP);		/* backup object device */
	ai->br_date      = MP->bkdate;		/* date-time of backup */
	ai->br_seqno     = 1;			/* sequence num of this vol */
	ai->br_media_cap = MP->blks_per_vol;	/* capacity in 512 byte blks */
	ai->br_blk_est   = 10;			/* num of blks in archive */
	ai->br_flags     = 0;

	if (IMM.cur) {
		ai->br_mname = (IMM.cur)->label;
	}
	else {
		ai->br_mname = NULL;
	}
	hdr = bld_hdr (ai, &hdrsize);

	if (hdr == NULL) {
		brlog("unable to build archive hdr");
		sprintf(ME(MP), "Job ID %s: unable to build archive header", MP->jobid);
		return(-1);
	}
	b->br_hdr = hdr;
	b->br_hdr_len = hdrsize;
	typstrng = "\0";

	if (MP->dtype == IS_DPART) {
		typstrng = "dpart";
		nbytes = (MP->blks_per_vol) << 9;
	}
	else {
		if (MP->dtype == IS_FILE)
			typstrng = "file";
		else if (MP->dtype == IS_DIR)
			typstrng = "dir";
		nbytes = 0;
	}
	i = br_write_hdr(&Fdisk, typstrng, MP->volpromt, b, nbytes, &fname);

	if (i < 0 ) {
		brlog("unable to write bkrs hdr on archive");
		sprintf(ME(MP), "Job ID %s: unable to write archive header to %s", MP->jobid, fname);
		return(-1);
	}
	if (MP->flags & vflag) {
		if (b->br_lab_len) {
			bytes_summed += (b->br_lab_len);
			sum (b->br_labelit_hdr, (long) (b->br_lab_len), &Vol_sum);
		}
		if (MP->dtype != IS_DPART)
			bytes_summed += (long) hdrsize;
			sum ((char *)hdr, (long) hdrsize, &Vol_sum);
	}
	if (MP->dtype == IS_DPART) {
		IMM.bytes_left -= hdrsize;
	}
	else {
		if (MP->blks_per_vol > 0) {  /* o/w go to eof */
			IMM.bytes_left -= bytes_summed;
		}
	}
	return(0);
} /* wr_bkrs_hdr() */
