/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:meth.d/iback.c	1.16.3.1"

#include	<limits.h> 	/* get PATH_MAX from here, not stdio */
#include	<sys/types.h>
#include	<sys/statvfs.h>
#include	<stdio.h>
#include	<errno.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<string.h>
#include	<setjmp.h>
#include	<sys/param.h>
#include	<method.h>
#include	<backup.h>
#include	<bktypes.h>
#include	<bkrs.h>
#include	<brarc.h>
#include	"libadmIO.h"

extern long		atol();      
extern int		bknewvol();
extern struct br_arc	*bld_hdr();
extern int		br_write_hdr();
extern int		brcancel();
extern int		brestimate();
extern int		brlog();
extern int		brreturncode();
extern int		brsuspend();
extern int		chk_vol_sum();
extern int		close();
extern int		do_unmount();
extern void		dots();
extern char		*devattr();
extern void		exit();
static int		fill_vol();
extern void		free();
extern char		*malloc();
extern void		remount();
extern int		g_close();
extern GFILE		*g_open();
extern int		g_read();
extern int		g_write();
extern int		g_seek();
extern int		g_set_up();
extern GFILE		*g_init();
extern int		g_copy();
extern int		g_wrap_up();
extern void		sum();
extern void		sync();
extern time_t		time();
extern long		ulimit();

extern int		bklevels;
extern int		brstate;
extern struct statvfs	Statvfs;

m_info_t	*MP;
jmp_buf		env;
char		fname[PATH_MAX+1];
char		*Buf = NULL;
unsigned	Vol_sum;		/* sum for -v option */
long		bytes_on_vol;		/* bytes on current vol */
short		reuse_dmname = 0;	/* for suspend, reuse current media */
media_info_t	IMM;

static int		attempt_restart();
static long		set_size();
static media_list_t	*new_vol();

static media_list_t	*nmedia;
static media_list_t	dpart_media = {NULL, NULL, 0};

static long	blks_512 = 0;
static long	bytes_to_go;
static int	obufsize = 0;
static GFILE	*ifile = NULL;		/* Input device/file */
static GFILE	*Image = NULL;		/* Output device/file desc */
static long	restart_offset = 0;
static long	restart_bytes_to_go;
static long	restart_bytes_done;
static long	media_size = 0;
static char 	reel = 0;
static char 	reels = 0;
static int	rdstart = 0;
static int	wrstart = 0;

do_image_backup(mp)
m_info_t	*mp;
{
	int	i;

#ifdef TRACE
brlog("do_image_backup");
#endif
	mp->bkdate = time((long *) 0);
	MP  = mp;
	IMM.first = IMM.last = NULL;

	/* Number of 512 byte blocks on the device */
	blks_512 = set_size(mp->ofsname, mp->ofsdev);

	if (blks_512 == 0) {
		return(1);
	}
	if (blks_512 < 0) {
		sprintf(ME(mp), "Job ID %s: no size for %s", mp->jobid, mp->ofsdev);
		return(1);
	}
	if ( MP->dtype == IS_DPART) {
		reels = 1;
	}
	else if (MP->blks_per_vol < 0) {
		reels = 255;
	}
/* 4 below = volcopy hdr, boot blk, super blk, br hdr , (its close,
			its just an estimate remember) */
	else {
		reels = ((blks_512 + (MP->blks_per_vol - 4)) / 
						(MP->blks_per_vol - 4));
	}
	if (MP->flags & (Nflag | Eflag)) {
		brestimate ((MP->blks_per_vol) > 0 ? reels : 0 , blks_512);	
		brlog("estimate %d %s completed",
			(MP->blks_per_vol) > 0 ? reels : blks_512 ,
			(MP->blks_per_vol) > 0 ? "vols" : "blocks" );
		if (MP->flags & Nflag) {
			sprintf(ME(mp), "Job ID %s: estimate completed", mp->jobid);
			brreturncode(BRSUCCESS, 0, ME(MP));
			remount(MP);
			exit(0);
		}
	}
	MP->estimate = blks_512;

	bytes_to_go = blks_512 << 9;

	if (!(MP->flags & qflag)) {
		BEGIN_CRITICAL_REGION;

		ifile = g_open(MP->ofsdev, O_RDONLY, 0);

		END_CRITICAL_REGION;

		if (ifile == NULL) {
			brlog(" cannot open %s\n", MP->ofsdev);
			sprintf(ME(mp), "Job ID %s: g_open of %s failed: %s", mp->jobid, MP->ofsdev, SE);
			return(1);
		}
	}
#ifdef TRACE
	brlog("%s reels=%d", MP->ofsdev, reels);
#endif
	if (i = setjmp(env))  {			  /* error has occurred */
		brlog(" do_backup: setjmp failed: ret=%d ", i);
		MP->blk_count >>= 9;
		if (MP->flags & qflag) {
			if (ifile != NULL) {
				(void) g_wrap_up(ifile);
				ifile = Image = NULL;
			}
		}
		else {
			if (Image != NULL) {
				(void) g_close(Image);
				Image = NULL;
			}
			(void) g_close(ifile);
			ifile = NULL;
		}
		return(i);
	}
	while (bytes_to_go) {
		int	left_over;

		switch(brstate) {

		case BR_CANCEL:
			if (i = brcancel()) {
#ifdef TRACE
				brlog("do_backup brcancel returned %d ", i);
#endif
				i = BRFAILED;
				sprintf(ME(mp), "Job ID %s: brcancel returned %d", mp->jobid, i);
			}
			else {
				i = BRCANCELED;
			}
			sprintf(ME(mp), "Job ID %s: cancel received", mp->jobid);
			longjmp(env, i);

			break;
		case BR_SUSPEND:	/* free dev if any */
			if (i = brsuspend()) {
#ifdef TRACE
				brlog("do_backup brsuspend returned %d", i);
#endif
				sprintf(ME(mp), "Job ID %s: brsuspend returned %d", mp->jobid, i);
				longjmp(env, BRFAILED);
			}
			reuse_dmname = 1;	/* reuse current media */

			if (!attempt_restart()) {
				sprintf(ME(mp), "Job ID %s: brsuspend restart error", mp->jobid);
				longjmp(env, BRFAILED);
			}
		/* FALL THROUGH */
		default:
			nmedia = new_vol();	/* get output vol */

			if (nmedia == NULL) {
				longjmp(env, BRFAILED);
			}
			if ((restart_offset = g_seek(ifile, 0, 1)) < 0) {
				brlog("g_seek failed %s",SE);
				sprintf(ME(mp), "Job ID %s: g_seek failed: %s", mp->jobid, SE);
				longjmp(env, BRFAILED);
			}
			restart_bytes_to_go = bytes_to_go;
			restart_bytes_done = MP->blk_count;

			if (i = fill_vol()) {
				longjmp(env, BRFAILED);
			}
			if (MP->flags & vflag) {
				if (chk_vol_sum(MP, &Image, bytes_on_vol, fname,
						Vol_sum)) {
					reuse_dmname = 0;

					if (!attempt_restart()) {
						sprintf(ME(mp), "Job ID %s: restart due to sum failed", mp->jobid);
						brlog("restart failed");
						longjmp(env, BRFAILED);
					}
				}
			}
		}
	}
	if (IMM.cur) {			/* add last vol */
		if (IMM.first == NULL) 
			IMM.first = IMM.cur;
		if (IMM.last)
			(IMM.last)->next = IMM.cur;
		IMM.last = IMM.cur;
	}
	MP->blk_count >>= 9;

	do_history(MP, &IMM, 0);

	if (MP->flags & qflag) {
		if (ifile != NULL) {
			(void) g_wrap_up(ifile);
			ifile = Image = NULL;
		}
	}
	else {
		if (Image != NULL) {
			(void) g_close(Image);
			Image = NULL;
		}
		(void) g_close(ifile);
		ifile = NULL;
	}
	return(0);
} /* do_image_backup() */

static media_list_t *
new_vol()
{
	media_list_t		*m_info = NULL;
	int			media_bufsize;
	int			isfile = 0;
	int			hdrsize;
	int			i;
	struct wr_archive_hdr	brhd;
	struct wr_archive_hdr	*b = &brhd;
	struct bld_archive_info	brai;
	struct bld_archive_info	*ai = &brai;
	char			*typstrng;
	struct br_arc		*hdr;
	long			nbytes;
	long			Cur_ulim = ulimit(1, 0l);

#ifdef TRACE
brlog("new_vol");
#endif
	if (MP->flags & qflag) {
		if (ifile != NULL) {
			(void) g_wrap_up(ifile);
			ifile = Image = NULL;
		}
	}
	else {
		if (Image != NULL) {
			if ((i = g_close(Image)) < 0) {
				brlog("g_close failed, returned %d", i);
			}
			Image = NULL;
			sync();
		}
	}
	if ((isfile = bknewvol(MP, fname, &reuse_dmname, &env, &IMM)) < 0) {
#ifdef TRACE
brlog("isfile = %d", isfile);
#endif
		return(NULL);
	}
	m_info = IMM.cur;

	if (m_info == NULL) {
		m_info = &dpart_media;
		dpart_media.label = MP->dname;
	}
	if (!(MP->flags & qflag)) {
		BEGIN_CRITICAL_REGION;
		if (isfile) {
			Image = g_open(fname, (O_WRONLY|O_CREAT|O_TRUNC), 0644);

			if (Image == NULL) {
				brlog(" new_vol: cannot create %s %s ", fname, SE);
				sprintf(ME(MP), "Job ID %s: cannot create %s: %s", MP->jobid, fname, SE);
				return(NULL);
			}
		}
		/* else wait until doing the br_write_hdr to open */
		/* This is done since the header needs to be read */
		/* and verified prior to writing and some devices */
		/* will not allow closing and reopenning the      */
		/* device without changing the media. (e.g. the   */
		/* tapes will write a file mark on close.)        */
		END_CRITICAL_REGION;
	}
	media_bufsize = 10*512;

	if (MP->blks_per_vol > 0) {
		media_size = MP->blks_per_vol << 9;
		if (Cur_ulim < MP->blks_per_vol) {
			Cur_ulim = ulimit(2, MP->blks_per_vol);
		}
	}
	else {
		media_size = LONG_MAX;		/* make it big */
	}
	if ((media_bufsize > obufsize) || (Buf == NULL)) {
		if (Buf != NULL) {
			free(Buf);
		}
		Buf = malloc((unsigned) media_bufsize);
		if (Buf == NULL) {
			brlog(" new_vol: cannot malloc %d bytes %s",
				media_bufsize, SE);
			sprintf(ME(MP), "Job ID %s: out of memory", MP->jobid);
			if (MP->flags & qflag) {
				if (ifile != NULL) {
					(void) g_wrap_up(ifile);
					ifile = Image = NULL;
				}
			}
			else {
				if (Image != NULL)
					(void) g_close(Image);
				Image = NULL;
			}
			return(NULL);
		}
	}
	obufsize = media_bufsize;

	Vol_sum = 0;
	bytes_on_vol = 0;

	ai->br_method = MN(MP);		/* method name */
	ai->br_fsname = OFS(MP);	/* file system name */
	ai->br_dev = ODEV(MP);		/* backup object device */
	ai->br_fstype = FSTYPE(MP);		/* fstype string */
	ai->br_date = MP->bkdate;		/* date-time of backup */
	ai->br_seqno = (int) ++reel;	/* sequence num of this vol */
	ai->br_media_cap = MP->blks_per_vol;	/* capacity in 512 byte blks */
	ai->br_blk_est = MP->estimate;		/* num of blks in archive */
	ai->br_flags = 0;

	if (IMM.cur) {
		ai->br_mname = (IMM.cur)->label;
	}
	else {
		ai->br_mname = NULL;
	}
	if ((hdr = bld_hdr(ai, &hdrsize)) == NULL) {
		brlog("unable to build archive hdr");
		sprintf(ME(MP), "Job ID %s: unable to build archive header", MP->jobid);
		if (MP->flags & qflag) {
			if (ifile != NULL) {
				(void) g_wrap_up(ifile);
				ifile = Image = NULL;
			}
		}
		else {
			if (Image != NULL)
				(void) g_close(Image);
			Image = NULL;
		}
		return(NULL);
	}
	b->br_hdr = hdr;
	b->br_hdr_len = hdrsize;
	wrstart += hdrsize;
	typstrng = "\0";

	if (MP->dtype == IS_DPART) {
		if ((blks_512 + (hdrsize >> 9)) > MP->blks_per_vol) {
#ifdef TRACE
brlog("blks_512=%d, hdrsize=%d, blks_per_vol=%d", blks_512, hdrsize, MP->blks_per_vol);
#endif
			brlog("no space for bkrs hdr on partition");
			sprintf(ME(MP), "Job ID %s: no space on archive for header", MP->jobid);
			if (MP->flags & qflag) {
				if (ifile != NULL) {
					(void) g_wrap_up(ifile);
					ifile = Image = NULL;
				}
			}
			else {
				if (Image != NULL)
					(void) g_close(Image);
				Image = NULL;
			}
			return(NULL);
		}
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
	if (MP->flags & qflag) {
		i = br_copy_hdr(&ifile, &Image, typstrng, MP->volpromt, b, nbytes, MP->ofsdev, &fname);
	}
	else {
		i = br_write_hdr(&Image, typstrng, MP->volpromt, b, nbytes, &fname);
	}
	if (i < 0 ) {
		brlog("unable to write bkrs hdr on archive");
		sprintf(ME(MP), "Job ID %s: write of header failed on archive", MP->jobid);
		if (MP->flags & qflag) {
			if (ifile != NULL) {
				(void) g_wrap_up(ifile);
				ifile = Image = NULL;
			}
		}
		else {
			if (Image != NULL)
				(void) g_close(Image);
			Image = NULL;
			if (ifile != NULL)
				(void) g_close(ifile);
			ifile = NULL;
		}
		return(NULL);
	}
	if (MP->flags & qflag) {
		if (g_seek(ifile, rdstart, 0) < 0) {
			brlog("g_seek failed %s",SE);
			ME(MP) = "g_seek failed";
			return(NULL);
		}
	}
	if (MP->flags & vflag) {
		if (b->br_lab_len) {
			sum (b->br_labelit_hdr, (long) (b->br_lab_len),
						&Vol_sum);
		}
		if (MP->dtype != IS_DPART)
			sum ((char *)hdr, (long) hdrsize, &Vol_sum);
	}
	IMM.bytes_left = media_size;

	if (MP->dtype == IS_DPART) {
		IMM.bytes_left -= hdrsize;
	}
	else {
		bytes_on_vol += (b->br_lab_len + b->br_hdr_len);
		if (MP->blks_per_vol > 0) {  /* o/w go to eof */
			IMM.bytes_left -= bytes_on_vol;
		}
	}
	return(m_info);
} /* new_vol() */

static int
fill_vol()
{
	int	size;
	int	rdsize;
	int	bytesout;
	int	i;

	size = (obufsize < bytes_to_go) ? obufsize : bytes_to_go;

	while ((IMM.bytes_left) && size && (brstate == BR_PROCEED) ) {

		rdsize = (size < 0) ? 0 : size;

		if (MP->flags & qflag) {
			bytesout = g_copy(ifile, rdsize, rdstart, wrstart);
			rdstart += bytesout;
			wrstart += bytesout;
		}
		else {
			i = g_read(ifile, Buf, rdsize);

			if (i != rdsize) {
				sprintf(ME(MP), "Job ID %s: g_read failed: %s", MP->jobid, SE);
				brlog(" fill_vol: read failed rdsize %d read %d %s",
						rdsize, i, SE);
				return(1);
			}
			bytesout = g_write(Image, Buf, rdsize);
		}
		if ((bytesout != rdsize) && (!(errno == ENOSPC || errno == ENXIO))) {
#ifdef TRACE
			brlog(" fill_vol: copy failed rdsize %d bytesout %d %s",
					rdsize, bytesout, SE);
#endif
			reuse_dmname = 0;

			if (attempt_restart())
				return(0);
			else {
				sprintf(ME(MP), "Job ID %s: restart after write failed on archive", MP->jobid);
				return(1);
			}
		}
		else if (bytesout <= 0) {
			if (MP->blks_per_vol <= 0) {	

				if (g_seek(ifile, (long) -(size), 1) < 0) {
					brlog("g_seek failed %s",SE);
					sprintf(ME(MP), "Job ID %s: g_seek failed: %s", MP->jobid, SE);
					return(1);
				}
				IMM.bytes_left = 0;	/* eof */
				continue;
			}
			else {
				reuse_dmname = 0;
				if (attempt_restart())
					return(0);
				else {
					sprintf(ME(MP), "Job ID %s: restart after write failed", MP->jobid); 
					return(1);
				}
			}
		}
		bytes_on_vol += bytesout;

		if (MP->flags & vflag) {
			sum (Buf, (long) bytesout, &Vol_sum);
		}
		if (DOTS(MP)) {
			dots (bytesout);
		}
		if ((MP->blks_per_vol) > 0)
			IMM.bytes_left -= bytesout;

		if (bytesout != size) {
			IMM.bytes_left = 0;
		}
		if (bytesout > 0) {
			bytes_to_go -= bytesout;
			MP->blk_count += bytesout;
			if (bytesout != rdsize) {
				int	left_over;

				if (g_seek(ifile, (long) (bytesout - rdsize), 1) < 0) {
					brlog("g_seek failed %s",SE);
					sprintf(ME(MP), "Job ID %s: g_seek failed: %s", MP->jobid, SE);
					return(1);
				}
			}
		}
		size = (obufsize < bytes_to_go) ? obufsize : bytes_to_go;
		if ((IMM.bytes_left > 0) && (size > IMM.bytes_left))
			size = IMM.bytes_left;  /* media is full */
	}
	return(0);
} /* fill_vol() */

static long
set_size(fsname, fsdev)
char	*fsname;
char	*fsdev;
{
	int		i;
	struct statvfs	*s = &Statvfs;
	char		*cap = NULL;
	long		fdp_bytes;

	if (MP->meth_type == IS_IMAGE) {

		if (!((MP->flags) & nflag)) {	/* unmount during backup */
			if (i = do_unmount(MP)) {
				brlog ("unmount of %s from %s failed",
						fsname, fsdev);
				sprintf(ME(MP), "Job ID %s: unmount of %s from %s failed", MP->jobid, fsname, fsdev);
				return((long) 0);
			}
			else {
				MP->mntinfo = DOREMOUNT;
			}
		}
#ifdef TRACE
brlog("set_size: vfs->f_bsize=%d", s->f_bsize);
brlog("set_size: vfs->f_frsize=%d, vfs->f_blocks=%d", s->f_frsize, s->f_blocks);
#endif
		if (s->f_frsize == 0) {
			brlog ("unknown file system parameter (f_frsize==0)");
			sprintf(ME(MP), "Job ID %s: unknown file system parameter (f_frsize==0)", MP->jobid);
			return((long) 0);
		}
		if (s->f_frsize > (u_long)512) {
			return((long) (s->f_blocks * (s->f_frsize/512)));
		}
		else {
			return((long) (s->f_blocks / (512/s->f_frsize)));
		}
	}
	else {				/* fdp */
		if (MP->c_count) {
			return((long) (MP->c_count));
		}
		cap = devattr(fsdev, "capacity");
		if ((cap == NULL) || ((fdp_bytes = atol(cap)) <= 0)) {
			brlog(" %s no block count specified",fsdev);
			sprintf(ME(MP), "Job ID %s: cannot determine block count for %s", MP->jobid, fsdev);
			return((long) 0);
		}
		return(fdp_bytes);
	}
} /* set_size() */

static int
attempt_restart()
{
	if (g_seek(ifile, restart_offset, 0) < 0) {
		brlog("g_seek failed in attempt_restart");
		return(0);
	}
#ifdef TRACE
brlog("restart_offset=%d, restart_bytes_to_go=%d, restart_bytes_done=%d", restart_offset, restart_bytes_to_go, restart_bytes_done);
#endif
	bytes_to_go = restart_bytes_to_go;
	MP->blk_count = restart_bytes_done;
	reel--;
	Vol_sum = 0;
	bytes_on_vol = 0;

	if (!reuse_dmname) {			/* io error */
		if (IMM.cur) {
			free(IMM.cur);
			IMM.cur = NULL;
		}
	}
	return(1);
} /* attempt_restart() */
