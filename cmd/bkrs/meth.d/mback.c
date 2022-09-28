/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:meth.d/mback.c	1.6.4.1"

#include	<limits.h> 	/* get PATH_MAX from here, not stdio */
#include	<sys/types.h>
#include	<sys/statfs.h>
#include	<sys/stat.h>
#include	<stdio.h>
#include	<errno.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<string.h>
#include	<setjmp.h>
#include	<sys/param.h>
#include 	"libadmIO.h"
#include	<method.h>
#include	<backup.h>
#include	<bktypes.h>
#include	<bkrs.h>
#include	<brarc.h>

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
extern char		*devattr();
extern int		do_unmount();
extern void		dots();
extern void		exit();
extern void		free();
extern char		*malloc();
extern int		g_close();
extern GFILE		*g_open();
extern int		g_read();
extern int		g_write();
extern int		g_seek();
extern void		sum();
extern void		sync();
extern void		remount();
extern time_t		time();
extern long		ulimit();

extern int		bklevels;
extern int		brstate;

jmp_buf		env;
m_info_t	*MP;
char		fname[PATH_MAX+1];
char		*Buf = NULL;
short		reuse_dmname = 0;	/* for suspend, reuse current media */
unsigned	Vol_sum;		/* sum for -v option */
long		bytes_on_vol;		/* bytes on current vol */
media_info_t	IMM;

static int	attempt_restart();
static int	fill_vol();
static long	set_size();

static media_list_t	*new_vol();

static media_list_t	*nmedia;
static media_list_t	dpart_media = {NULL, NULL, 0};

static long	blks_512 = 0;
static long	bytes_to_go;
static int	obufsize = 0;
static GFILE	*ifile = NULL;	/* Input device/file */
static GFILE	*Image = NULL;	/* Output device/file desc */
static long	restart_offset = 0;
static long	restart_bytes_to_go;
static long	restart_bytes_done;
static long	media_size = 0;
static char	reel = 0;
static char	reels = 0;

static struct archive_info	Global_ai;

do_migration(mp, toc)
m_info_t	*mp;
int		toc;
{
	int	i;
	int	ofd;
	long	partsz = 0;
	char	*t;

#ifdef TRACE
brlog(" do_migration: begin toc=%d", toc);
#endif
	mp->bkdate = time((long *) 0);
	MP  = mp;
	IMM.first = IMM.last = NULL;

	BEGIN_CRITICAL_REGION;

        ifile = g_open(MP->ofsdev, O_RDONLY, 0);

	END_CRITICAL_REGION;

        if (ifile == NULL) {
                brlog(" cannot open %s", MP->ofsdev);
		sprintf(ME(mp), "Job ID %s: g_open of %s failed: %s", mp->jobid, MP->ofsdev, SE);
                return(1);
        }
	t = devattr(MP->ofsdev, "type");

	if (!strcmp(t, "dpart")) { /* Is a partition */
		t = devattr(MP->ofsdev, "capacity");
		partsz = atol(t);
	}
	if (toc) {
		struct stat	st;
	
#ifdef TRACE
                brlog("safe_stat of %s", mp->tocfname);
#endif
		if (safe_stat(mp->tocfname, &st) < 0) {
			brlog(" cannot stat %s", MP->ofsdev);
			sprintf(ME(mp), "Job ID %s: cannot stat %s", mp->jobid, MP->ofsdev);
			return(1);
		}
#ifdef TRACE
                brlog("st.st_size = %d", st.st_size);
#endif
		blks_512 = (st.st_size + 511) >> 9;
		bytes_to_go = st.st_size;
	}
	else {
#ifdef TRACE
		brlog("MIGRATION: read header dev=%s, partsz=%d", mp->ofsdev, partsz);
#endif
		if (br_read_hdr(&ifile, &Global_ai, (partsz ? (partsz-1)<<9 : partsz),
			MP->ofsdev) <= 0) {
			brlog(" cannot read header from %s", MP->ofsdev);
			sprintf(ME(mp), "Job ID %s: cannot read header from %s", mp->jobid, MP->ofsdev);
			return(1);
		}
		blks_512 = Global_ai.br_blk_est;
		bytes_to_go = blks_512 << 9;
	}
	if (blks_512 == 0) {
#ifdef TRACE
		brlog("MIGRATION: blks_512 == 0, return(1)");
#endif
		return(1);
	}
	if (blks_512 < 0) {
		sprintf(ME(mp), "Job ID %s: no size for backup object", mp->jobid);
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
#ifdef TRACE
	brlog("%s reels=%d", MP->ofsdev, reels);
#endif
	if (i = setjmp(env)) {	/* error has occurred */
		brlog(" do_migration: setjmp failed: ret=%d ", i);
		MP->blk_count >>= 9;

		if (Image != NULL) {
			(void) g_close(Image);
			Image = NULL;
		}
		(void) g_close(ifile);
		ifile = NULL;
		return(i);
	}
	while (bytes_to_go) {
		int	left_over;
#ifdef TRACE
	brlog("do_migration: bytes_to_go=%d, brstate=%d", bytes_to_go, brstate);
#endif
		switch(brstate) {

		case BR_CANCEL:
			if (i = brcancel()) {
#ifdef TRACE
				brlog("do_migration brcancel returned %d ", i);
#endif
				i = BRFAILED;
				sprintf(ME(mp), "Job ID %s: brcancel returned %d", mp->jobid, i);
			}
			else {
				i = BRCANCELED;
			}
			sprintf(ME(mp), "Job ID %s:  received cancel", mp->jobid);
			longjmp(env, i);
			break;
		case BR_SUSPEND:	/* free dev if any */
			if (i = brsuspend()) {
#ifdef TRACE
				brlog("do_migration brsuspend returned %d", i);
#endif
				sprintf(ME(mp), "Job ID %s:  brsuspend returned %d", mp->jobid, i);
				longjmp(env, BRFAILED);
			}
			reuse_dmname = 1;	/* reuse current media */

			if (!attempt_restart()) {
				sprintf(ME(mp), "Job ID %s:  brsuspend restart error", mp->jobid);
				longjmp(env, BRFAILED);
			}
		/* FALL THROUGH */
		default:
			if ((nmedia = new_vol()) == NULL) { /* get output vol */
				longjmp(env, BRFAILED);
			}
			if ((restart_offset = g_seek(ifile, 0, 1)) < 0) {
				brlog("g_seek failed %s",SE);
				sprintf(ME(mp), "Job ID %s:  g_seek failed: %s", mp->jobid, SE);
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
	/* Assign the original device name info to the migration */
	MP->ofsname = Global_ai.br_fsname;
	MP->ofsdev = Global_ai.br_dev;
	MP->blk_count >>= 9;

	if (Image != NULL) {
		(void) g_close(Image);
		Image = NULL;
	}
	(void) g_close(ifile);
	ifile = NULL;
	return(0);
} /* do_migration() */

static media_list_t *
new_vol()
{
	media_list_t	*m_info = NULL;
	int		media_bufsize;
	int		isfile = 0;
	int		hdrsize;
	int		i;
	char		*typstrng;
	struct br_arc	*hdr;
	long		nbytes;
	long		Cur_ulim = ulimit(1, 0l);
	struct wr_archive_hdr	brhd;
	struct wr_archive_hdr	*b = &brhd;
	struct bld_archive_info	brai;
	struct bld_archive_info	*ai = &brai;

#ifdef TRACE
brlog(" new_vol: begin");
#endif
	if (Image != NULL) {
		(void) g_close(Image);
		Image = NULL;
		sync();
	}
	if ((isfile = bknewvol(MP, fname, &reuse_dmname, &env, &IMM)) < 0) {
		return(NULL);
	}
	if ((m_info = IMM.cur) == NULL) {
		m_info = &dpart_media;
		dpart_media.label = MP->dname;
	}
	BEGIN_CRITICAL_REGION;

	if (isfile) {
		Image = g_open(fname, (O_WRONLY|O_CREAT|O_TRUNC), 0644);

		if (Image == NULL) {
			brlog(" new_vol: cannot create %s %s ", fname, SE);
			sprintf(ME(MP), "Job ID %s: g_open of %s failed: %s", MP->jobid, fname, SE);
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

			if (Image != NULL)
				(void) g_close(Image);
			Image = NULL;
			return(NULL);
		}
	}
	obufsize = media_bufsize;

	Vol_sum = 0;
	bytes_on_vol = 0;

	ai->br_method = Global_ai.br_method;
	ai->br_fsname = Global_ai.br_fsname;
	ai->br_dev = Global_ai.br_dev;
	ai->br_date = Global_ai.br_date;
	MP->bkdate = ai->br_date;
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
	hdr = bld_hdr (ai, &hdrsize);

	if (hdr == NULL) {
		brlog("unable to build archive hdr");
		sprintf(ME(MP), "Job ID %s: unable to build archive header", MP->jobid);

		if (Image != NULL)
			(void) g_close(Image);
		Image = NULL;
		return(NULL);
	}
	b->br_hdr = hdr;
	b->br_hdr_len = hdrsize;
	typstrng = "\0";

	if (MP->dtype == IS_DPART) {
		if ((blks_512 + (hdrsize >> 9)) > MP->blks_per_vol) {
			brlog("no space for bkrs hdr on partition");
			sprintf(ME(MP), "Job ID %s: no space for bkrs hdr on archive", MP->jobid);

			if (Image != NULL)
				(void) g_close(Image);
			Image = NULL;
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
	i = br_write_hdr(&Image, typstrng, MP->volpromt, b, nbytes, &fname);

	if (i < 0 ) {
		brlog("unable to write bkrs hdr on archive");
		sprintf(ME(MP), "Job ID %s: unable to write bkrs hdr on archive", MP->jobid);
		if (Image != NULL)
			(void) g_close(Image);
		Image = NULL;
		if (ifile != NULL)
			(void) g_close(ifile);
		ifile = NULL;
		return(NULL);
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
#ifdef TRACE
brlog(" new_vol: end IMM.bytes_left=%d", IMM.bytes_left);
#endif
	return(m_info);
} /* new_vol() */

static int
fill_vol()
{
	int	size;
	int	i;
	int	rdsize;
	int	bytesout;
#ifdef TRACE
brlog(" fill_vol: begin obufsize=%d, bytes_to_go=%d", obufsize, bytes_to_go);
#endif
	size = (obufsize < bytes_to_go) ? obufsize : bytes_to_go;
#ifdef TRACE
brlog(" fill_vol: size=%d, IMM.bytes_left=%d", size, IMM.bytes_left);
#endif
	while ((IMM.bytes_left) && size && (brstate == BR_PROCEED)) {

		rdsize = (size < 0) ? 0 : size;
		i = g_read(ifile, Buf, rdsize);

		if (i != rdsize) {
			sprintf(ME(MP), "Job ID %s: g_read failed: %s", MP->jobid, SE);
			brlog(" fill_vol: read failed rdsize %d read %d %s",
					rdsize, i, SE);
			return(1);
		}
		bytesout = g_write(Image, Buf, size);

		if ((bytesout == -1) && (!(errno == ENOSPC || errno == ENXIO))) {
#ifdef TRACE
			brlog("write error %s",SE);
#endif
			reuse_dmname = 0;

			if (attempt_restart())
				return(0);
			else {
				sprintf(ME(MP), "Job ID %s: restart after write error failed", MP->jobid);
				return(1);
			}
		}
		else if (bytesout <= 0) {
			if (MP->blks_per_vol <= 0) {	
				int	left_over;

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
					sprintf(ME(MP), "Job ID %s: restart after write error failed", MP->jobid);
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
#ifdef TRACE
brlog(" fill_vol: end size=%d, IMM.bytes_left=%d", size, IMM.bytes_left);
#endif
	return(0);
} /* fill_vol() */

static int
attempt_restart()
{
	if (g_seek(ifile, restart_offset, 0) < 0) {
		return (0);
	}
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
