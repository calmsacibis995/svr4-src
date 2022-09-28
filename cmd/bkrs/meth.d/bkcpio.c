/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:meth.d/bkcpio.c	1.16.2.1"

#include	<limits.h> 	/* get PATH_MAX from here, not stdio */
#include	<stdio.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/mkdev.h>
#include 	"libadmIO.h"
#include	<ctype.h>
#include	<errno.h>
#include	<fcntl.h>
#include	<setjmp.h>
#include	<method.h>
#include	<backup.h>
#include	<bktypes.h>
#include	<bkrs.h>
#include	<signal.h>
#include	<string.h>
#include	<table.h>
#include	<brtoc.h>
#include	<brarc.h>

#define BUFSIZE		512
#define CPIOBSZ		4096
#define CHARS		110          /* ASCII header size minus filename field */

#define EQ(x, y)        (strcmp(x, y)==0)

#define MAGIC	0x070701
extern int		atoi();
extern int		bknewvol();
extern struct br_arc	*bld_hdr();
extern void		br_state();
extern int		br_write_hdr();
extern int		brlog();
extern void		brsndfname();
extern int		chk_vol_sum();
extern int		close();
extern void		do_history();
extern void		dots();
extern void		free();
extern void		*malloc();
extern int		g_close();
extern GFILE		*g_open();
extern int		g_write();
extern int		g_read();
extern int		g_seek();
extern int		safe_stat();
extern char		*sbrk();
extern void		smemcpy();   
extern void		sum();
extern void		sync();
extern time_t		time();
extern long		ulimit();
extern int		TLassign();
extern int		TLdelete();
extern int		TLwrite();

extern int	bklevels;
extern int	brstate;

m_info_t	*MP;

int		TC;
jmp_buf		env;
struct stat	Statb;
unsigned	Vol_sum;			/* sum for -v option */
char 		cpio_name[PATH_MAX+1];

static int	attempt_restart();
static void	bintochar();
int		pad();
static void	chk_ulim();
static int	cpio();
static void	write_archive();

static media_info_t	M;

static long	bytes_on_vol;
static long	Cur_ulim;	/* current ulimit for file size */
static GFILE	*ifile = NULL;	/* Input device/file for cpio */
static GFILE	*Cpio = NULL;	/* Output device/file for cpio */
static int	Cflag;
static int	orig_Cct;
static char	*Chdr = NULL;
static char	*Bhdr = NULL;
static short	reuse_dmname = 0;	/* for suspend, reuse current media */
static int	Entryno = 2;
static int	Pathsize = PATH_MAX;
typedef struct {
	long    Blocks;		/* 512 byte blocks successfully to archive */
	long	offset;		/* offset in input file, if any */
	int	tc_entryno;	/* table of contents entry num */
	int	rest_index;	/* number of fill blks at end of archive */
	int	Cct;		/* num chars left in current archive buffer */
	char	*Cp;		/* where to place next char in output buffer */
	char	*org;		/* address of archive output buffer */
	char	*A_src_org;	/* address of data source to write_archive */
	int	size;		/* size of archive output buffer */
	int 	ct;		/* char count of last read */
	int	A_src_siz;	/* num of chars to put in archive */
	long 	fsz;		/* size of file currently being archived */
	short 	restart;	/* if non-zero write_archive failed */
	short	new_vol_ok;	/* filling end of archive, no new vol allowed */
	short 	volnum;		/* volume num of current vol */
	char    Pathname[4];	/* malloced array for current pathname */
} restart_info_t;

static int	Bufsize = 5120;
static long	n512 = 10;
static long	bytes_per_vol;

static restart_info_t	*cur;
static restart_info_t	*lastvol;

static char	*sav_A_src;
static char	*Buf;
static char	*sav_buf;
static char    *Cbuf;

int
do_cpio(mp, tc)
m_info_t	*mp;
int		tc;
{
	int	i;
	int	ret;
	int	tocvol;
	ENTRY	eptr;
	unsigned char	*fn;
	unsigned char	buffer[20];
	register restart_info_t	*ri;
	int cnt;

#ifdef TRACE
brlog("do_cpio start");
#endif
	mp->bkdate = time((long *) 0);
	MP = mp;
	TC = tc;
	M.bytes_left = 0;
	M.first = M.last = M.cur = NULL;
	(void) safe_stat("/", &Statb);		/* fill in Statb */

	if (MP->blks_per_vol > 0) {
		bytes_per_vol = MP->blks_per_vol << 9;
	}
	else {
		bytes_per_vol = LONG_MAX;	/* make it big */
	}
	Cur_ulim = ulimit(1, (long) 0);	/* get current ulimit */

	Buf = (char *) malloc((unsigned) (2 * CPIOBSZ) + (2 * Bufsize));

	if (Buf == NULL) {
		brlog("bkcpio: malloc failed: no memory");
		sprintf(ME(MP), "Job ID %s: out of memory", MP->jobid);
		return(1);
	}
	sav_A_src = (char *) (Buf + CPIOBSZ);
	sav_buf = (char *) (sav_A_src + CPIOBSZ);
	Cbuf = (char *) (sav_buf + Bufsize);

	Cflag++;
	MP->blk_count = 0;

	if ((eptr = TLgetentry(TC)) == NULL) {
		brlog("do_cpio: cannot allocate toc entry");
		sprintf(ME(MP), "Job ID %s: cannot allocate table library entry", MP->jobid);
		return(1);
	}
	i = setjmp(env);

	if (i) {			       	  /* do the work */
		free(Buf);
		if (Chdr)
			free(Chdr);
		if (Bhdr)
			free(Bhdr);
		if (cur)
			free(cur);
		if (lastvol)
			free(lastvol);
		TLfreeentry(TC, eptr);
		brlog(" do_cpio: setjmp ret=%d ", i);

		if (ifile != NULL) {
			(void) g_close(ifile);
			ifile = NULL;
		}
		return(i);
	}
	BEGIN_CRITICAL_REGION;

	ret = TLread(TC, Entryno++, eptr);

	END_CRITICAL_REGION;

	fn = TLgetfield(TC, eptr, TLCOMMENT);

	if (fn) {
		if (!strncmp((char *)fn, "PATHLENGTH=", 11)) {
			Pathsize = atoi((char *) (fn+11));
		}
	}
	else {
		Entryno--;
	}
	Chdr = (char *) malloc((unsigned) (Pathsize + CHARS + 8));

	if (Chdr == NULL) {
		brlog("no memory for cpio Chdr size %d",(Pathsize+CHARS+8));
		longjmp(env, BRBADCPIO);
	}
	Bhdr = (char *) malloc((unsigned) (Pathsize + sizeof(HHdr_t)));

	if (Bhdr == NULL) {
		brlog("no mem for cpio Bhdr size %d",(Pathsize+sizeof(HHdr_t)));
		longjmp(env, BRBADCPIO);
	}
	cur = (restart_info_t *)
			malloc((unsigned) (Pathsize+sizeof(restart_info_t)));
	lastvol = (restart_info_t *)
			malloc((unsigned) (Pathsize+sizeof(restart_info_t)));

	if ((cur == NULL) || (lastvol == NULL)) {
		brlog("no mem for restart info size %d",
					(Pathsize+sizeof(restart_info_t)));
		longjmp(env, BRBADCPIO);
	}
	ri = cur;
	ri->Cct = Bufsize;
	ri->Cp = Cbuf;
	ri->volnum = 0;
	ri->Blocks = 0;
	ri->restart = 0;
	ri->new_vol_ok = 1;
	ri->tc_entryno = Entryno;

	while (1) {
		BEGIN_CRITICAL_REGION;

		ret = TLread(TC, Entryno++, eptr);

		END_CRITICAL_REGION;

		if (ret != TLOK)
			break;

		fn = TLgetfield(TC, eptr, TOC_FNAME);

		if (fn == NULL) { 
			continue;
		}
		(void) strcpy(ri->Pathname, (char *)fn);
		fn = TLgetfield(TC, eptr, TOC_VOL);
		(void) sscanf((char *)fn, "%d", &tocvol);

		if (SAVTOC(mp)) { /* volnum = 0 first time, ok */
			if ((ri->volnum) && (tocvol != (ri->volnum))) {
				(void) sprintf((char *)buffer,"%d",ri->volnum);
				i = TLassign(TC, eptr, TOC_VOL, buffer);

				if (i == TLOK) {
					(void) TLwrite(TC, (Entryno-1), eptr);
					tocvol = ri->volnum;
				}
			}
		}
dostat:
		if (safe_stat(ri->Pathname, &Statb) < 0) {
                        brlog(" do_cpio: cannot stat %s ", ri->Pathname);
			if (SAVTOC(mp)) {
				(void) TLdelete(TC, --Entryno);
			}
                        continue;
		}
		/* clean cpio header buf */

		for(cnt=0;cnt<(Pathsize + CHARS + 8); cnt++)
			*(Chdr+cnt) = '\0';

		(void) cpio();

		if (ri->restart) {
			if (attempt_restart())
				goto dostat;
			else
				goto dotrail;
		}
		if (FNAMES(MP)) {
			(void) brsndfname(ri->Pathname);
		}
	}
	if (Entryno == 3) {
		brlog("no file names in toc");
	}
dotrail:
	(void) strcpy(ri->Pathname, "TRAILER!!!");
	Statb.st_size = 0;

	/* clean cpio header buf */

	for(cnt=0;cnt<(Pathsize + CHARS + 8); cnt++)
		*(Chdr+cnt) = '\0';

	(void) cpio();

	if ((MP->flags & vflag) && !(ri->restart)) {
		if (chk_vol_sum(MP, &Cpio, bytes_on_vol, cpio_name, Vol_sum)) {
			ri->restart = 7;	/* reset in attemp_restart */ 
		}
	}
	if (ri->restart)  {
		if (attempt_restart())
			goto dostat;
		else
			goto dotrail;
	}
	if (M.cur) {			/* last vol was good */
		if (M.first == NULL) 
			M.first = M.cur;
		if (M.last)
			(M.last)->next = M.cur;
		M.last = M.cur;
	}
	free(Buf);
	free(Chdr);
	free(Bhdr);
	free(cur);
	free(lastvol);

	if ((mp->br_type & IS_BTOC) == IS_BTOC) {	/* move toc to media */
		char	*ftmp;
		char	*fname1;
		struct stat	pstat;

		ftmp = strdup(mp->tocfname);
		fname1 = strrchr(ftmp, '/');
		fname1++;
		*fname1 = 'P';

		if (stat(ftmp, &pstat) == 0) {
			(void) unlink(ftmp);
			(void) unlink(mp->tocfname);
			mp->tocfname = NULL;
		}
	}
	do_history(MP, &M, TC);

	(void) TLsync(TC);
	(void) TLfreeentry(TC, eptr);

	if (ifile != NULL) {
		(void) g_close(ifile);
		ifile = NULL;
	}
	if (Cpio != NULL) {
		(void) g_close(Cpio);
		Cpio = NULL;
	}
	return(0);
} /* do_cpio() */

static
int
attempt_restart()
{
	register restart_info_t	*ri = cur;
	register restart_info_t	*sav = lastvol;

	if (ifile != NULL) {
		(void) g_close(ifile);
		ifile = NULL;
	}
#ifdef TRACE
	brlog("ri->Pathname=%s sav->Pathname=%s %d",
			ri->Pathname,sav->Pathname,sav->Blocks);
#endif
	if (ri->Blocks) { /* at least 1 write done */
		*ri = *sav;
		(void) memcpy(ri->A_src_org, sav_A_src, ri->A_src_siz);
		(void) memcpy(ri->org, sav_buf, ri->size);
		(void) strcpy(ri->Pathname, sav->Pathname);
	}
#ifdef TRACE
	brlog("have restart Entryno=%d Nentryno=%d ri_offset=%d",Entryno,ri->tc_entryno,ri->offset);
#endif
	Entryno = ri->tc_entryno;

	if (ri->restart > 4) {

		BEGIN_CRITICAL_REGION;

	        ifile = g_open(ri->Pathname, O_RDONLY, 0644);

		END_CRITICAL_REGION;

       		if (ifile == NULL) {
       			brlog(" bkcpio: cannot restart %s ", ri->Pathname);
			sprintf(ME(MP), "Job ID %s: g_open failed for %s: %s", MP->jobid, ri->Pathname, SE);
			longjmp(env,BRBADCPIO);
       		}
		else {
			if (g_seek(ifile, ri->offset, 0) < 0) {
				brlog(" bkcpio: g_seek failed: cannot restart %s ", ri->Pathname);
				sprintf(ME(MP), "Job ID %s: g_seek failed for %s: %s", MP->jobid, ri->Pathname, SE);
				longjmp(env,BRBADCPIO);
			}
		}
	}
	M.bytes_left = 0;	/* get new_vol */

	if (Cpio != NULL) {
		(void) g_close(Cpio);
		if ((MP->dtype == IS_FILE) || (MP->dtype == IS_DIR)) {
			cpio_name[0] = 0;
		}
		Cpio = NULL;
	}
	if (!reuse_dmname) {			/* io error */
		if (M.cur) {
			free(M.cur);
			M.cur = NULL;
		}
	}
	if (strcmp(ri->Pathname, "TRAILER!!!"))
		return(1);
	else 
		return(0);
} /* attempt_restart() */

static
new_vol(buf, size, A_src, A_src_siz)
char	*buf;
int	size;
char	*A_src;
int	A_src_siz;
{
	long	nbytes;
	char	*typstrng;
	int	isfile = 0;
	int	hdrsize;
	int	i;
	struct br_arc		*hdr;
	struct wr_archive_hdr	brhd;
	struct wr_archive_hdr	*b = &brhd;
	struct bld_archive_info	brai;
	struct bld_archive_info	*ai = &brai;
	register restart_info_t	*ri = cur;
	register restart_info_t	*sav = lastvol;

	M.bytes_left = bytes_per_vol;

	*sav = *ri;
	(void) strcpy(sav->Pathname, ri->Pathname);

	if (ifile != NULL) {
		if ((sav->offset = g_seek(ifile, 0l, 1)) < 0) {
			longjmp(env,BRBADCPIO);
		}
	}
	else {
		sav->offset = 0;
	}
	sav->tc_entryno = Entryno;

#ifdef TRACE
	brlog("new_vol: Pathname=%s offset=%d tc_entryno=%d orig_Cct=%d",
		sav->Pathname,sav->offset,sav->tc_entryno,orig_Cct);
#endif
	sav->Cct = orig_Cct; 
	sav->A_src_org = A_src;
	sav->A_src_siz = A_src_siz;
	sav->org = buf;
	sav->size = size;
#ifdef TRACE
	brlog("new_vol: A_src_org=%x A_src_siz=%d buf=%x size=%d Cbuf=%x Buf=%x",
			A_src,A_src_siz,buf,size,Cbuf,Buf);
#endif
	(void) memcpy(sav_buf, buf, size);
	(void) memcpy(sav_A_src, A_src, A_src_siz);

	if (brstate != BR_PROCEED) {
		br_state(MP, env);
	}
	if (Cpio != NULL) {
		(void) g_close(Cpio);
		Cpio = NULL;
		sync();
	}
	if ((isfile = bknewvol(MP, cpio_name, &reuse_dmname, env, &M)) < 0) {
		longjmp(env,BRBADCPIO);
	}
	BEGIN_CRITICAL_REGION;
	if (isfile) {
		Cpio = g_open(cpio_name, (O_WRONLY|O_CREAT|O_TRUNC), 0644);

		if (Cpio == NULL) {
			brlog(" newvol: cannot create %s %s ", cpio_name, SE);
			sprintf(ME(MP), "Job ID %s: g_open failed for %s: %s", MP->jobid, cpio_name, SE);
			return(-1);
		}
	}
	/* else wait until doing the br_write_hdr to open */
	/* This is done since the header needs to be read */
	/* and verified prior to writing and some devices */
	/* will not allow closing and reopenning the      */
	/* device without changing the media. (e.g. the   */
	/* tapes will write a file mark on close.)        */
	END_CRITICAL_REGION;

	Vol_sum = 0;		/* for -v option */
	bytes_on_vol = 0;
	ri->volnum++;

	ai->br_method = MN(MP);		/* method name */
	ai->br_fsname = OFS(MP);	/* file system name */
	ai->br_dev = ODEV(MP);		/* backup object device */
	ai->br_fstype = FSTYPE(MP);		/* fstype string */
	ai->br_date = MP->bkdate;		/* date-time of backup */
	ai->br_seqno = ri->volnum;	/* sequence num of this vol */
	ai->br_media_cap = MP->blks_per_vol;	/* capacity in 512 byte blks */
	ai->br_blk_est = MP->estimate;		/* num of blks in archive */
	if (MP->br_type & IS_TOC) {		/* moving toc to media */
		ai->br_flags = BR_IS_TOC;
	}
	else {
		ai->br_flags = 0;
	}
	if (M.cur) {
		ai->br_mname = (M.cur)->label;
	}
	else {
		ai->br_mname = NULL;
	}
	hdr = bld_hdr (ai, &hdrsize);

	if (hdr == NULL) {
		brlog("unable to build archive hdr");
		sprintf(ME(MP), "Job ID %s: unable to build archive header", MP->jobid);
		if (Cpio != NULL)
			(void) g_close(Cpio);
		Cpio = NULL;
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
brlog("br_media_cap=%d, blks_per_vol=%d, nbytes=%d", ai->br_media_cap, MP->blks_per_vol, nbytes);
	/* br_write_hdr Reopens Cpio in O_WRONLY mode */
	i = br_write_hdr(&Cpio, typstrng, MP->volpromt, b, nbytes, cpio_name);

	if (i < 0 ) {
		brlog("unable to write bkrs hdr on archive");
		sprintf(ME(MP), "Job ID %s: write of archive header failed for %s: %s", MP->jobid, cpio_name, SE);
		if (Cpio != NULL) {
			(void) g_close(Cpio);
			Cpio = NULL;
		}
		if (ifile != NULL) {
			(void) g_close(ifile);
			ifile = NULL;
		}
		return(-1);
	}
	if (MP->flags & vflag) {
		if (b->br_lab_len) {
			sum (b->br_labelit_hdr, (long) (b->br_lab_len),
							&Vol_sum);
		}
		if (MP->dtype != IS_DPART)
			sum ((char *)hdr, (long) hdrsize, &Vol_sum);
	}
	if (MP->dtype == IS_DPART) {
		M.bytes_left -= hdrsize;
	}
	else {
		bytes_on_vol += (b->br_lab_len + b->br_hdr_len);
		if (MP->blks_per_vol > 0) {  /* o/w go to eof */
			M.bytes_left -= bytes_on_vol;
		}
	}
	return(0);
} /* new_vol() */

static int
cpio()
{
	register restart_info_t	*ri = cur;
	register Hdr_t		*hdr = (Hdr_t *) Bhdr;
	int dpad = 0;	/* initialize data pad count */

        (void) strcpy(hdr->h_name, !strncmp(ri->Pathname, "./", 2) ?
				 (ri->Pathname)+2: ri->Pathname);
        hdr->h_magic = MAGIC;
        hdr->h_namesize = strlen(hdr->h_name) + 1;
        hdr->h_uid = Statb.st_uid;
        hdr->h_gid = Statb.st_gid;
        hdr->h_dev = Statb.st_dev;
        hdr->h_ino = Statb.st_ino;
        hdr->h_mode = Statb.st_mode;
        hdr->h_mtime = Statb.st_mtime;
        hdr->h_nlink = Statb.st_nlink;
	hdr->h_cksum = 0L;

	if ((hdr->h_mode&S_IFMT) == S_IFREG || (hdr->h_mode&S_IFMT) == S_IFLNK)
		hdr->h_filesize = Statb.st_size;
	else
		hdr->h_filesize = 0L;

        hdr->h_rdev = Statb.st_rdev;

        if (Cflag){
                bintochar(Chdr, hdr);
	}

	switch (ri->restart) {
		case 2:
			goto restart2;
/*NOTREACHED*/
			break;
		case 3:
			goto restart3;
/*NOTREACHED*/
			break;
		case 4:
			goto restart4;
/*NOTREACHED*/
			break;
		case 5:
			goto restart5;
/*NOTREACHED*/
			break;
		case 6:
			goto restart6;
/*NOTREACHED*/
			break;
	} /* switch */
        ri->fsz = ((hdr->h_mode & S_IFMT) == S_IFREG ||
		 (hdr->h_mode & S_IFMT) == S_IFLNK) ? Statb.st_size: 0L;

        if (EQ(hdr->h_name, "TRAILER!!!")) {
restart2:
		ri->restart = 2;

                write_archive(Chdr, pad((CHARS + hdr->h_namesize), 1));

		if (ri->restart) {
			return(-1);
		}
		MP->blk_count = ri->Blocks;

		if (Cflag)
		    MP->blk_count +=
			((Bufsize - (ri->Cct) + (BUFSIZE-1)) / BUFSIZE);

		if (M.bytes_left >= (Bufsize - (ri->Cct))) {
			 /* Put TRAILER on curr vol */
			ri->new_vol_ok = 0;
		}
                for (ri->rest_index = 0; (ri->rest_index < 10); ++(ri->rest_index)) {
restart3:
			ri->restart = 3;
                        write_archive(Buf, BUFSIZE);

			if (ri->restart) {
				return(-1);
			}
		}
                return(0);
        }
        if (!hdr->h_filesize) {
restart4:
		ri->restart = 4;

		write_archive(Chdr, pad((CHARS + hdr->h_namesize), 1));

		if (ri->restart) {
			return(-1);
		}
		return(0);
	}
	BEGIN_CRITICAL_REGION;

	ifile = g_open(ri->Pathname, O_RDONLY, 0644);

	END_CRITICAL_REGION;

	if (ifile == NULL) {
		brlog(" bkcpio: cannot copy %s ", hdr->h_name);
                return(-2);
        }
restart5:
	ri->restart = 5;
        write_archive(Chdr, pad((CHARS + hdr->h_namesize), 1));

	if (ri->restart) {
		(void) g_close(ifile);
		ifile = NULL;
		return(-1);
	}		

	dpad = pad(hdr->h_filesize,0);  /* get pad for data */

        for (ri->fsz = hdr->h_filesize; ri->fsz > 0; ri->fsz -= CPIOBSZ) {
                ri->ct = ri->fsz>CPIOBSZ? CPIOBSZ: ri->fsz;

		if ((hdr->h_mode&S_IFMT) == S_IFLNK){
			if (readlink(hdr->h_name, Buf, CPIOBSZ) < 0){

                        	brlog(" bkcpio: cannot read symbolic link %s, %s ", hdr->h_name, SE);
				sprintf(ME(MP), "Job ID %s: readlink failed for %s: %s", MP->jobid, ri->Pathname, SE);
				(void) g_close(ifile);
				ifile = NULL;
				longjmp(env, BRBADCPIO);
			}
		} else if (g_read(ifile, Buf, ri->ct) < 0)  {
			brlog(" bkcpio: cannot read %s %s ", hdr->h_name, SE);
	/*
  	 read failed but hdr claims h_filesize data bytes - thus must write
	 garbage for this file so the archive format is not destroyed
	 or give up --- for now give up, thus the archive will really
 	 contain what we say or the backup will fail
	*/
			sprintf(ME(MP), "Job ID %s: g_read failed for %s: %s", MP->jobid, ri->Pathname, SE);
			(void) g_close(ifile);
			ifile = NULL;
			longjmp(env, BRBADCPIO);
                }
restart6:
		ri->restart = 6;
			/* check for padding on data */

		if (dpad>=1 && ((ri->ct+dpad)<=CPIOBSZ)){
			memcpy((Buf+ri->ct), '\0\0\0\0', dpad);
                	write_archive(Buf, (ri->ct+dpad));
			dpad = 0;
		} else
                	write_archive(Buf, ri->ct);

		if (ri->restart) {
			brlog("restart ri->ct=%d %s\n",ri->ct,ri->Pathname);
			(void) g_close(ifile);
			ifile = NULL;
			return(-1);
		}
        }
	if(dpad>=1){ /* write remaining pad */
		/* couldn't fit the data pad in the 
		** above so write it here 
		*/

		memcpy(Buf, "\0\0\0\0", dpad);
               	write_archive(Buf, dpad);
		dpad = 0;
	}
        (void) g_close(ifile);
	ifile = NULL;
        return(0);
} /* cpio() */

static void
write_archive(A_src, count)
register char	*A_src;
register	count;
{
        register char	*cp;
	int		sav_c;
	char		*sav_p;
	int		ret;
	int		write_size;
	register restart_info_t	*ri = cur;

	cp = ri->Cp;
	orig_Cct = ri->Cct;
	sav_c = count;
	sav_p = A_src;

        while (count--)  {
                if (!(ri->Cct))  {
media_eof:
			if (M.bytes_left <= 0) {
				if (!(ri->new_vol_ok)) {
					break;	/* just filling last blk */
				}
				if (M.cur != NULL) {	/* not first vol */
						/* if Cpio == NULL, assume restart */
					if ((MP->flags & vflag) && (Cpio != NULL)) {
						/* check all but last here */
						if ( chk_vol_sum(MP, &Cpio,
							   bytes_on_vol,
							   cpio_name,
							   Vol_sum) ) { 
							return;
						}
					}
				}
				if (new_vol(Cbuf, Bufsize, sav_p, sav_c) < 0)
					return;
			}
			if (brstate != BR_PROCEED) {	/* suspend or cancel */
				reuse_dmname = 1;
				return;
			}
			write_size = (M.bytes_left < Bufsize) ? M.bytes_left :
						    Bufsize;
			errno = 0;
			ret = g_write(Cpio, Cbuf, write_size);

			if ((ret == -1) &&
				(!(errno == ENOSPC || errno == ENXIO))) {
#ifdef TRACE
				brlog("write error %s",SE);
#endif
				return;		/* change to generic io eof */
			}
			else if (ret <= 0) {
				if (MP->blks_per_vol <= 0) {	
					M.bytes_left = 0;	/* eof */
					goto media_eof;
				}
				else {
#ifdef TRACE
					brlog("bytes_left=%d write_size=%d ret=%d %s",M.bytes_left,write_size,ret,SE);
#endif
					return;
				}
			}
#ifdef TRACE
			if (ret != Bufsize) {
				brlog("write of size %d wrote %d %s",
						write_size,ret,SE);
			}
#endif
			bytes_on_vol += ret;
			if (MP->flags & vflag)
				sum (Cbuf, (long) ret, &Vol_sum);
			if (DOTS(MP)) {
				dots (ret);
			}
			if (MP->blks_per_vol > 0) {  /* o/w go to eof */
				M.bytes_left -= ret;
			}
                       	ri->Cct = ret;
                       	cp = Cbuf + (Bufsize - ret); 
			ri->Blocks += (ret >> 9);
			if (ret < Bufsize) {
				smemcpy(Cbuf, (Cbuf + ret),
					(unsigned) (Bufsize - ret));
			}
			chk_ulim();
                }
                *cp++ = *A_src++;
                --(ri->Cct);
        }
        ri->Cp = cp;
	ri->restart = 0;
} /* write_archive() */

static void
chk_ulim()
{
	if (((cur->Blocks) + n512) >= Cur_ulim) {
		Cur_ulim <<= 1;

		if (ulimit(2, Cur_ulim) < 0) {
			brlog(" bkcpio: ulimit error %s ", SE);
		}
	}
} /* chk_ulim() */

static void
bintochar(Chdrp, hdr)            /* ASCII header write */
register char	*Chdrp;
Hdr_t		*hdr;
{
        (void) sprintf(Chdrp,
		"%.6lx%.8lx%.8lx%.8lx%.8lx%.8lx%.8lx%.8lx%.8lx%.8lx%.8lx%.8lx%.8lx%.8lx%s",
                MAGIC, Statb.st_ino, Statb.st_mode, Statb.st_uid, Statb.st_gid, 
		Statb.st_nlink, Statb.st_mtime, hdr->h_filesize, 
		major(Statb.st_dev), minor(Statb.st_dev), major(Statb.st_rdev),
		minor(Statb.st_rdev), strlen(hdr->h_name)+1, 0, hdr->h_name);


} /* bintochar() */

/* this function rounds to the next word boundary */
int pad(num, flag)
long num;
int flag;
{
int padval;
	if (num < 0) {
		brlog("bkcpio: negative padcnt %d\n", num);
		return(-1);
	}
	padval = (((num + PAD_VAL) & ~PAD_VAL) - num);

	if(flag == 1)
		return((num+padval));	/* return total */
	else
		return(padval);		/* return the word align factor */
}
	
