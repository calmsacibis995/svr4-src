/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:meth.d/do_file.c	1.20.2.1"
#include	<limits.h>
#include	<stdio.h>
#include	<errno.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/mkdev.h>
#include	<signal.h>
#include	<memory.h>
#include	<setjmp.h>
#include	<backup.h>
#include	<bktypes.h>
#include	<bkrs.h>
#include	<fcntl.h>
#include 	"libadmIO.h"
#include	<method.h>
#include	<string.h>
#include	<brarc.h>

#define OFAIL	"open of %s failed  %s  restore incomplete"
#define WFAIL	"write of %s failed  %s  restore incomplete"
#define RFAIL	"archive read error %s for file %s restore incomplete"

#define LINKS	500		/* no. of links allocated per bunch */
#define TRUE	1
#define ASCII	1		/* Cflag setting */
#define NONE	2		/* Cflag setting not verified */
#define M_ASCII "070701"	/* ASCII  magic number */
#define ASC_CNT 14		/* ASCII scanf read count */
#define M_STRLEN 6		/* number bytes in ASCII magic number */
#define CPIOBSZ	4096		/* file read/write */
#define HDRSIZE	(Hdr.h_name - (char *)&Hdr) /* header size minus filename */
				 /* header size minus filename field */
#define CHARS	110		/* ASCII header size minus filename field */
#define EQ(x,y)	(strcmp(x,y)==0)

extern int		brlog();
extern int		chdir();
extern int		chmod();
extern int		chown();
extern int		close();
extern int		create_a_special();
extern void		dorsresult();
extern void		dots();
extern int		dup();
extern int		execl();
extern void		exit();
extern file_rest_t	*file_req();
extern pid_t		fork();
extern void		free();
extern int		get_mnt_info();
extern int		link();    
extern long		lseek();
extern int		makdir();
extern void		*malloc();
extern int		missdir();
extern int		mkdir();
extern GFILE		*new_Input();
extern int		pipe();
extern void		*realloc();
extern char		*result();
extern int		rsresult();
extern int		g_close();
extern int		g_read();
extern int		g_seek();
extern int		safe_stat();
extern int		safe_write();
extern void		set_modes();
extern void		set_time();
extern unsigned		sleep();
extern void		smemcpy();
extern long		ulimit();
extern int		unlink();
extern pid_t		wait();
extern int		pad();

extern char		cpio_name[];
extern jmp_buf		env;
extern m_info_t		*MP;
extern int		bklevels;

long		bytes_left = 0;
short		checksize = 0;

static int		hit;
static Hdr_t		Hdr;
static struct stat	Xstatb;
static unsigned		Bufsize = 5120;		/* default record size */
static char		*Cbuf;
static ushort		A_directory;
static ushort		A_special;
static ushort		Filetype = S_IFMT;
static ushort		Cflag = NONE;
static GFILE		*Input = NULL;

static long	sBlocks;
static long	Blocks;
static long	Longfile;
static long	Longtime;

static char	*Chdr;

static pid_t	cpid;			/* pid of forked child */
static long	filesz;			/* size of current file */
static long	sav_filesz;		/* size of current file */
static char	*Buf;			/* pointer to cpio buffer */
static int	archive_bufsize = CPIOBSZ; /* may change due to g_init call */
static int	ct;
static int	stat_loc;
static pid_t	dead_pid;
static int	child_in[2];
static int	child_out[2];
static int	nfs_len;
static int	ofs_len;
static char	newname[PATH_MAX+1];
static long	maxsiz;
static int	left_todo = 0;

static int	bread();
static int	build_hdr();
static char	*build_name();
static int	build_trailer();
static void	chartobin();
static void	check_dirs_made();
static void	child_gone();
static void	child_exit();
static int	do_wait();
static int	f_qualify();
static int	gatling();
static int	rest_flnk();
static int	gethdr();
static int	getmem();
static int	hdck();
static int	postml();
static int	set_up_child();
static int	skip_file();
static void	synch();
static int	rs_pave();
static int	rstbuf();

struct archive_info	arc_info;

static struct dirs_made	Dhead = { &Dhead, &Dhead, '\0' };

static file_rest_t	Ohead;
static file_rest_t	Otail;

#define NEW_INPUT	new_Input(MP, cpio_name, &bytes_left, &checksize, &arc_info)
#define SM(x)		set_modes(x,Hdr.h_mode,Hdr.h_uid,Hdr.h_gid,Hdr.h_mtime)

do_filerest(mp, argv)
m_info_t	*mp;
unsigned char	*argv[];
{
	char	*rmsg;
	char	*tmp;
	char	buffer[24];
	int	i;
	file_rest_t	*file_base;
	register file_rest_t	*f;


	MP = mp;
	maxsiz = 512 * ulimit(1, 0);
	cpio_name[0] = 0;

	file_base = file_req(mp, argv, &left_todo);

	if (file_base == NULL) {
		sprintf(ME(mp), "Job ID %s: no legal F or D requests", mp->jobid);
		return(1);
	}
	if (left_todo <= 0) {
		goto doresult;
	}
	Input = NEW_INPUT;

	if (Input == NULL) {
		sprintf(ME(mp), "Job ID %s: no archive to read on %s", mp->jobid, cpio_name);
		return(1);
	}
	if (Input->_size > 1) {
		archive_bufsize = Input->_size;
	}
	if (getmem()) {
		if (Input != NULL)
			(void) g_close(Input);
		return(1);
	}
	if (setjmp(env) != 0) {
		MP->blk_count += 511;
		MP->blk_count >>= 9;
		if (Input != NULL)
			(void) g_close(Input);
		return(1);
	}
	while (left_todo && gethdr(&Hdr)) {	/* find cpio hdr on archive */

		if (Cflag == ASCII) {
			(void) memcpy((Chdr+CHARS),Hdr.h_name,Hdr.h_namesize);
			/* need to account for filename pad and data pad */
			filesz = (Hdr.h_filesize + pad(CHARS+Hdr.h_namesize,0) +
				pad(Hdr.h_filesize,0));
		} else
			filesz = Hdr.h_filesize;

		sav_filesz = filesz;

		/* see if this file is going anywhere, if so, how many places */

		Ohead.rest_next = Otail.rest_prev = NULL;
		Ohead.rest_prev = Otail.rest_next = NULL;

		for (i = 0, f = file_base; i < mp->n_names ; i++, f++) {

			if (f->status)		/* nothing to do */
				continue;

			if (f->type) {		/* directory */
				if (strncmp(f->name, Hdr.h_name, f->name_len)) {
					continue;
				}
			}			/* file */
			else if (strcmp(Hdr.h_name, f->name))
				continue;			/* no match */
			else {
				left_todo--;			/* file match */
			}

			if (f_qualify(f)) {
				continue;
			}
			if (Ohead.rest_next == NULL) {
				Ohead.rest_next = f;
				Otail.rest_prev = f;
				f->rest_prev = &Ohead;
				f->rest_next = &Otail;
			}
			else {
				(Otail.rest_prev)->rest_next = f;
				f->rest_prev = Otail.rest_prev;
				f->rest_next = &Otail;
				Otail.rest_prev = f;
			}
			if (!(f->type)) {  
				f->status = F_SUCCESS;	/* fail will change */
				f->rindx = R_SUCCESS;
			}                       
		}
		if ((Ohead.rest_next) == NULL) {
			if (!left_todo)
				break;
			i = skip_file();  /* skip this file */
			if (i)
				break;
		}
		else {
			if (FNAMES(MP)) {
				i = (*(MP->sndfp)) (Hdr.h_name);
			}

			if ((Hdr.h_mode & S_IFMT) == S_IFLNK)
				i=rest_flnk();
			else
				i = gatling();  /* got the file so restore it */

			if (i || (!left_todo))
				break;
		}
		if (A_directory) {
			check_dirs_made();
		}
	}
doresult:

	dorsresult(mp, file_base);

	MP->blk_count += 511;
	MP->blk_count >>= 9;
	brlog("do_file returns blks=%d",MP->blk_count);

	if (Input != NULL)
		(void) g_close(Input);

	return(0);
} /* do_filerest() */

static int
f_qualify(fp)
file_rest_t	*fp;
{
	char	buf1[32];
	char	buf2[32];

	if (fp->ldate < Hdr.h_mtime) { 	/* too new */
		if ( ! (fp->type)) {
			fp->status = F_UNSUCCESS;
			fp->rindx = R_DATE;
		}
		return (1);
	}
	if (maxsiz < filesz) {
		brlog(" %s size %d exceeds ulimit %d", fp->name,filesz,maxsiz);

		if (!(fp->type)) {
			fp->status = F_UNSUCCESS;
			fp->rindx = R_ULIMIT;
		}
		if (!RM(fp)) {
			(void) sprintf(buf1, "%ld", filesz);
			(void) sprintf(buf2, "%ld", maxsiz);
			RM(fp) = result(3, "file %s size %s exceeds ulimit %s",
				Hdr.h_name, buf1, buf2);		
		}
		return (1);
	}
	if (fp->idnum) {
		if (Hdr.h_uid != fp->idnum) {
			if (!(fp->type)) {
				fp->status = F_UNSUCCESS;
				fp->rindx = R_NOTOWN;
			}
			else if (!RM(fp)) {
				(void) sprintf(buf1, "%ld", fp->idnum);
				RM(fp) = result(1,
				"some files not owned by uid %s not restored",
						buf1);
			}
#ifdef TRACE
			brlog("f_qualify: uid %ld not owner of %s(%d)",
				fp->idnum, Hdr.h_name, Hdr.h_uid);
#endif
			return(1);
		}
	}
	return(0);
} /* f_qualify() */

	/* send cpio hdr & file to /usr/bin/cpio child */
static int
send_file(fdout,fdin,hdr,len,padj)
register int	fdout;			/* stdin of child */
int		fdin;			/* stdout and stderr of child */
char		*hdr;			/* cpio hdr */
int		len;			/* length of hdr */
int		padj;			/* write pointer adj (could be neg)*/
{
	register int	ret;
	char	*p;
	int	wcnt;		/* write count */

	ret = safe_write(fdout, hdr, len);

	if (ret != len) {
		brlog(" send_file: write of cpio hdr failed %s", SE);
		ret = do_wait(fdin, 1);
		return(1);
	}

	for(; filesz > 0; filesz -= archive_bufsize){
		ct = filesz > archive_bufsize ? archive_bufsize : filesz;

		errno = 0;

		if (bread(Buf, ct, filesz) == -1) {
			brlog("send_file: i/o error, %s is corrupt",Hdr.h_name);
			ret = do_wait(fdin, 1);
			return(1);
		}
		if (padj){
				/* adjust filesize for write pad */
			p = (Buf+padj);		/* adjust write pointer */
			wcnt = ct+((~padj)+1);  /* adjust write count */
			if((ret = safe_write(fdout, p, wcnt)) != wcnt){
				brlog("send_file: pad write failed %s", SE);
				ret = do_wait(fdin, 1);
				return(1);
			}

			padj = 0;
		}else
			if ((ret = safe_write(fdout, Buf, ct)) != ct) {
				brlog("send_file: write failed %s", SE);
				ret = do_wait(fdin, 1);
				return(1);
			}



	}

	return(0);
} /* send_file() */

do_comprest(mp)
m_info_t	*mp;
{
	int	wrk;
	int	len;
	int	i;
	char	*hdr;
	char	*fname;
	cpio_name[0] = 0;

	MP = mp;
	maxsiz = 512 * ulimit(1, 0);

	/* allocate memory for Cbuf, Chdr, and Buf */
	if (getmem()) {
		return(1);
	}
	i = get_mnt_info(mp);

	if (i < 0) {
		return(1);
	}
	ofs_len = strlen(mp->ofsname);
	nfs_len = strlen(mp->nfsname);

	if (nfs_len)		/* new fs name */
		(void) strcpy(newname, mp->nfsname);

	else if (strlen(mp->nfsdev)) {	/* get fs name mounted on nfsdev */
		if (nfs_len = strlen(mp->nfsdevmnt)) {
			(void) strcpy(newname, mp->nfsdevmnt);
			mp->nfsname = mp->nfsdevmnt;
		}
		else {
			brlog("no fs mounted on %s",mp->nfsdev);
			sprintf(ME(mp), "Job ID %s: no filesystem mounted on %s", mp->jobid, mp->nfsdev);
			return(1);
		}
	}
	if (setjmp(env) != 0) {
		MP->blk_count += 511;
		MP->blk_count >>= 9;
		return(1);
	}
	Input = NEW_INPUT;

	if (Input == NULL) {
		return(1);
	}
	stat_loc = 0xffff;
	dead_pid = 0;
	wrk = set_up_child();

	if (wrk) {
		return(wrk);
	}
	while (gethdr(&Hdr)) {	/* find cpio hdr on archive */
		/* The following two variables are used to control
		** the pad-space following the filename in the ASCII
		** header.
		*/
		int nam_pad = 0; /* name pad size */
		int nnam_pad = 0;  /* new name pad size */
		int data_pad =0; /* data pad size */
		int adjwrtp = 0; /* modify pointer if filename pad changes */


		/* ASCII header is word aligned, therefore variable length
		** fields in the header (i.e. filename and data) must be 
		** padded.
		*/
		if (Cflag == ASCII){
			nam_pad = pad((CHARS + Hdr.h_namesize), 0);
			data_pad = pad(Hdr.h_filesize,0);
		}

		filesz = (Hdr.h_filesize+nam_pad+data_pad);
		sav_filesz = filesz;

		if (Cflag == ASCII) {
			(void) memcpy((Chdr+CHARS),Hdr.h_name,Hdr.h_namesize);
			fname = (Chdr + CHARS);
			hdr = Chdr;
			len = Hdr.h_namesize + CHARS;
		}
		else {
			hdr = (char *) &Hdr;
			len = HDRSIZE + Hdr.h_namesize;
			fname = Hdr.h_name;
		}
		if (maxsiz < filesz) {
			brlog(" %s size %d exceeds ulimit %d\n",
				Hdr.h_name,filesz,maxsiz);
			wrk = skip_file();	/* file too large -skip it */
			if (wrk)
				break;
			else
				continue;
		}
		if (nfs_len) {
			(void) strcpy((newname+nfs_len), (Hdr.h_name+ofs_len));
			len = strlen(newname) + 1; /* include null */
			if (Cflag == ASCII)
				nnam_pad = pad((CHARS+len), 0);
			wrk = build_hdr(newname, &len, &hdr);
			if (wrk)
				break;

			/* Check to see if the new filename has any
			** affect on the padsize in the archive header.
			*/

			if (nnam_pad < nam_pad){
				/* Increment the write pointer */
				adjwrtp = nam_pad - nnam_pad;

			}else if (nnam_pad > nam_pad){
				/* Decrement the write pointer */
				adjwrtp = (nam_pad - nnam_pad);

			}else{ 
				/* Pad value for both files is the same. */
				adjwrtp = 0;
			}
		} else {
			/* filename didn't change */
			adjwrtp = 0;
			nnam_pad = nam_pad;
		}

		if (FNAMES(MP)) {
			wrk = (*(MP->sndfp)) (fname);
		}

		wrk = send_file(child_in[1],child_out[0],hdr,len,adjwrtp);
		if (wrk)
			break;
		mp->blk_count += sav_filesz;
	}
	if (!wrk) {			/* no errors */
		ct = build_trailer(&hdr);
		wrk = send_file(child_in[1],child_out[0],hdr,ct,0);
	}
	(void) close(child_in[1]);	/* Close cpio pipe to send an EOF */

	sav_filesz = 0;
	filesz=0;
	wrk = do_wait(child_out[0], 0);

	MP->blk_count += 511;
	MP->blk_count >>= 9;
	rsresult(MP->jobid, wrk ? BRUNSUCCESS : BRSUCCESS,
			    wrk ? "unsuccessful, see log" : "successful");
	return(wrk);
} /* do_comprest() */

static int
do_wait(fdin, do_kill)
int	fdin;		/* cpio stdout & stderr */
int	do_kill;	/* have error, kill child */
{
	char	line[513];
	int	n;
	pid_t ret;

	if (do_kill) {
		(void) kill(cpid, SIGKILL);
	}
	while ((n = safe_read(fdin, line, 512)) > 0) {
		line[n] = 0;
	}
	if (!dead_pid) {
		ret = wait(&stat_loc);
	}
	if ((ret == cpid) || (dead_pid == cpid)) {
		if ((stat_loc & 0xff) == 0) {
			MP->blk_count += sav_filesz;
			return(0);
		}
		else {
			if (Cflag == ASCII) {
				(void) unlink(Chdr + CHARS);
			}
			else {
				(void) unlink(Hdr.h_name);
			}
			return(1);
		}
	}
	else {
		return(1);
	}
} /* do_wait() */

static int
set_up_child()
{
	int	wrk;
	int	fd;
	int	retry = 0;

	wrk = pipe(child_in);

	if (wrk < 0) {
		brlog(" do_fork can't creat child in %s",SE);
		return(-1);
	}
	wrk = pipe(child_out);

	if (wrk < 0) {
		brlog(" do_fork can't creat child out %s",SE);
		(void) close(child_in[0]);
		(void) close(child_in[1]);
		return(-1);
	}
	(void) sigset(SIGCLD, child_exit);
	(void) sigset(SIGPIPE, child_gone);

	while (((cpid = fork()) == -1) && (errno == EAGAIN) && (retry++ < 12)) {
		(void) sleep(10);
	}
	if (cpid < 0) {
		brlog(" do_fork can't fork %s",SE);
		(void) close(child_in[0]);
		(void) close(child_in[1]);
		(void) close(child_out[0]);
		(void) close(child_out[1]);
		return(-1);
	}
	if (cpid > 0) {			/* parent */
		(void) close(child_in[0]);	/* we're the writer */
		(void) close(child_out[1]);	/* we're the reader */
		return(0);
	}
	else {				/* child */
		(void) close(child_in[1]);	/* we're the reader */
		(void) close(child_out[0]);	/* we're the writer */
		while(((wrk = chdir("/")) < 0) && (errno == EINTR)) {
			continue;
		}
		if (wrk < 0) {
			brlog(" child can't cd to / %s", SE);
			exit(2);
/*NOTREACHED*/
		}
		(void) close(0);

		fd = dup(child_in[0]);

		if (fd != 0) {
			brlog(" child can't dup stdin %s", SE);
			exit(2);
/*NOTREACHED*/
		}
		(void) close(child_in[0]);
		(void) close(1);
		fd = dup(child_out[1]);

		if (fd != 1) {
			brlog(" child can't dup stdout %s", SE);
			exit(2);
/*NOTREACHED*/
		}
		(void) close(child_out[1]);
		(void) close(2);
		fd = dup(1);

		if (fd != 2) {
			brlog(" child can't dup stderr %s", SE);
			exit(3);
/*NOTREACHED*/
		}

		wrk = execl("/usr/bin/cpio","cpio","-idum",(char *)0);

		/* HOPEFULLY NOT REACHED */
		brlog(" child exec of cpio failed %s", SE);
		exit(2);
/*NOTREACHED*/
	}
} /* set_up_child() */

static int
gatling()
{
	int		ret = 0;
	int		i;
	file_rest_t	*wrk;
	register file_rest_t	*fp;
	int first = 1;

	for (fp = Ohead.rest_next; fp != (&Otail); ) {
		fp->rest_name = fp->mall_name = NULL;
		i = rs_pave(fp);		/* create out files */

		if (i < 0) {	/* either already there or failed */
			(fp->rest_prev)->rest_next = fp->rest_next;
			(fp->rest_next)->rest_prev = fp->rest_prev;
			wrk = fp->rest_next;
			fp->rest_next = NULL;

			if (fp->type && fp->mall_name) {
				free(fp->mall_name);
				fp->mall_name = NULL;
			}
			fp = wrk;
		}
		else {
			if (i > (_NFILE - 10) ) {
				(void) close(i);
				fp->rest_fd = -1;
			}
			else {
				fp->rest_fd = i;
			}
			fp = fp->rest_next;
		}
	}
	if ((Ohead.rest_next) == (&Otail)) {
		 /* eliminated all possibilities */

		if (left_todo)
			return(skip_file());
		else
			return(1);
	}


	for (; filesz > 0; filesz -= archive_bufsize) {  /* process archive */

		int padj;

		if (filesz > archive_bufsize)
			ct = archive_bufsize;
		else 
			ct = filesz;
		errno = 0;

		if (bread(Buf, ct, filesz ) == -1) {
			brlog("frestore: i/o error, %s is corrupt",Hdr.h_name);
			ret = 1;
			break;
		}
		for (fp = Ohead.rest_next; fp != (&Otail); fp = fp->rest_next) {
			if (fp->rest_fd < 0) {
				fp->rest_fd = open(fp->rest_name, O_WRONLY);

				if (fp->rest_fd < 0) {
					brlog("open of %s failed %s",
						fp->rest_name, SE);
					if (!RM(fp)) {
						RM(fp) = result(2,OFAIL,
						     fp->rest_name, SE);
					}
					if (! fp->type) {
						fp->status = F_UNSUCCESS;
						fp->rindx = R_INCOMP;
					}
					else if (fp->mall_name) {
						free (fp->mall_name);
						fp->mall_name = NULL;
					}
					(fp->rest_prev)->rest_next = 
								fp->rest_next;
					(fp->rest_next)->rest_prev = 
								fp->rest_prev;
					fp->rest_next = NULL;
					fp = fp->rest_prev;
					continue;
				}
				else {
					(void) lseek(fp->rest_fd,
						 fp->offset, 0);
				}
			}

			/* first write skip passed filename pad */ 

			if (first){
				padj = pad(CHARS+Hdr.h_namesize,0);
				ct -= padj;
				first = 0;
			} else
				padj = 0;
			/* last write we get rid of data pad */

			if (archive_bufsize >= filesz){
				ct -= pad(Hdr.h_filesize,0);
			}

			if ((i = safe_write(fp->rest_fd, (Buf+padj), ct)) != ct) {
				brlog("write to %s failed %s",fp->rest_name,SE);

				if (!RM(fp)) {
					RM(fp) = result(2,WFAIL,
						     fp->rest_name, SE);
				}
				if (! fp->type) {
					fp->status = F_UNSUCCESS;
					fp->rindx = R_INCOMP;
				}
				else if (fp->mall_name) {
					free(fp->mall_name);
					fp->mall_name = NULL;
				}
				(void) close(fp->rest_fd);
				fp->rest_fd = -1;
				(fp->rest_prev)->rest_next = fp->rest_next;
				(fp->rest_next)->rest_prev = fp->rest_prev;
				fp->rest_next = NULL;
				fp = fp->rest_prev;
				continue;
			}
			fp->offset += ct;
			MP->blk_count += ct;

			if (fp->rest_fd > (_NFILE - 10)) {
				(void) close(fp->rest_fd);
				fp->rest_fd = -1;
			}
		}
	}
	if (ret) { 			/* read error */
		i = skip_file();

		for (fp = Ohead.rest_next; fp != NULL; fp = fp->rest_next) {
			if (! (fp->type)) {
				fp->status = F_UNSUCCESS;
				fp->rindx = R_INCOMP; 
			}
			else if (!RM(fp)) {
				RM(fp) = result(2, RFAIL, SE, fp->rest_name);
			}
		}
	}
	for (fp = Ohead.rest_next; fp != (&Otail); fp = fp->rest_next) {
		if (fp->rest_fd >= 0) {
			(void) close(fp->rest_fd);
			fp->rest_fd = -1;
		}
		if (fp->rest_name)
			SM(fp->rest_name);
		if (fp->type) {

			if (fp->mall_name) {
				free (fp->mall_name);
				fp->mall_name = NULL;
			}
			fp->file_count++;  
		}
	}
	return(ret);
} /* gatling() */

/*
 * create output file for restore return < 0 if fail or no further
 * action is required (e.g. is directory or special)
 * return open file descriptor for regular files
 */
static int
rs_pave(fp)
file_rest_t	*fp;
{
	char	*namep;
	int	ans;
	int	i;
	int	fd;
	register char	*np;

	if (fp->type) {			/* directory */
		namep = build_name(fp);
#ifdef TRACE
		brlog("rs_pave dir name=%s",(namep ? namep : "NULL"));
#endif
	}
	else {
		if (fp->rename != NULL) {
			namep = fp->rename;
		}
		else {
			namep = fp->name;
		}
	}
	if (namep == NULL) {
		return(-1);
	}
	fp->rest_name = namep;
	np = namep;

	if (A_directory) {			/* archive hdr is a directory */
		if (safe_stat(namep, &Xstatb) == -1) {

			ans = create_a_dir(namep, fp, &Dhead, (uid_t) Hdr.h_uid,
				(gid_t) Hdr.h_gid);

			if (ans) {
				return(-1);
			}
			SM(namep);
			return(-2);
		}
		else {
			if (fp->type) {		/* request was a directory */
				return(-2);	/* may find some files */
			}
			else {
				fp->status = F_UNSUCCESS;
				fp->rindx = R_EXIST;
				return(-1);
			}
		}
	}				/* end archive hdr is a directory */
/* archive hdr is either a special file or regular file */
	i = check_nondir(namep, fp, Hdr.h_mtime);

	if (i < 0) {
		return(i);
	}
	if (Hdr.h_nlink > 1) {
		if (!postml(namep, np, fp)) {
#ifdef TRACE
			brlog("postml linked namep=%s np=%s",namep,np);
#endif
			return (-2);   /* able to link to prev restored file */
		}
	}
	if (A_special) {

		if ((Hdr.h_mode & Filetype) == S_IFIFO)
			Hdr.h_rdev = 0;

		if (create_a_special(namep,fp,np, (mode_t) Hdr.h_mode,
			(dev_t) Hdr.h_rdev)) {
			return(-1);
		}
		SM(namep);		/* set modes */

		return(-2);
	}
/*  is a regular file */
	i = create_a_reg(namep, np, fp, (mode_t) Hdr.h_mode, &fd);

	if (i < 0) {
		return(i);
	}
	fp->offset = (long) 0;
	return (fd);
} /* rs_pave() */

static void
check_dirs_made()
{
	struct dirs_made	*wrkp;

	for (wrkp = Dhead.next; wrkp != &Dhead; wrkp = wrkp->next) {
		if (strcmp(wrkp->dirname, Hdr.h_name))
			continue;
		SM(wrkp->dirname);
		(wrkp->next)->prev = wrkp->prev;
		(wrkp->prev)->next = wrkp->next;
		free(wrkp);
		break;
	}
} /* check_dirs_made() */

static char *
build_name(fp)
file_rest_t	*fp;
{
	int	len;
	char	*namep;

	if (!(fp->rename_len)) {
		return(Hdr.h_name);
	}
	len = strlen(Hdr.h_name);
	len += fp->rename_len;
	len -= fp->name_len;
	len++;				/* leave room for null */
	namep = (char *) malloc((unsigned)len);

	if (namep == NULL) {
		brlog("build_name: malloc failed on %s",Hdr.h_name);
		return(NULL);
	}

	(void) strcpy(namep, fp->rename);
	(void) strcpy((namep+(fp->rename_len)), (Hdr.h_name+(fp->name_len)));

#ifdef TRACE
	brlog("build_name returns %s len %d",namep,len);
#endif
	fp->mall_name = namep;

	return(namep);
} /* build_name() */

static int
gethdr(hdr)		/* get file headers */
Hdr_t *hdr;
{
	register mode_t	ftype;

	/* The synch routine does the real work of finding the cpio header.
	** It builds the static binary header Hdr as well as the character
	** ASCII header Chdr.  */
	synch();

	/* if we reached the archive TRAILER return 0 */
	if (EQ(hdr->h_name, "TRAILER!!!")) {
		return (0);
	}
	ftype = hdr->h_mode & Filetype;
	A_directory = (ftype == S_IFDIR);
	A_special = (ftype == S_IFBLK) ||  (ftype == S_IFCHR)
					||  (ftype == S_IFIFO);
	return (1);
} /* gethdr() */

/*	Shared by bread(), synch() and rstbuf() */
static int	nleft = 0;   /* unread chars left in Cbuf and expansion buffer*/
static char	*ip;	     /* pointer to next char to be read from Cbuf and */
			     /* expansion buffer*/
static int	filbuf = 0;  /* flag to bread() to fill buffer but transfer */
			     /* no characters				*/

/*
 * synch() searches for headers.  Any Cflag specification by the
 * user is ignored.  Cflag is set appropriately after a good
 * header is found.  It searches for and verifies all headers.
 * I/O errors during examination of any part of the
 * header causes synch() to throw away current data and begin
 * again.  Other errors during examination of any part of the
 * header causes synch() to advance a single byte and continue
 * the examination.
 */
static void
synch()
{
	register char	*magic;
	int		hsize;
	int		offset;
	int		align = 0;
	static int	min = CHARS;
	union {
		char	bite[2];
		ushort	temp;
	} mag;

	hit = NONE;

	if (Cflag == NONE) {
		filbuf = 1;

		if (bread(Chdr, 0, 0) == -1)
			brlog(" do_comp io error %s searching to next hdr", SE);
	}
	magic = ip;
	do {
		while (nleft < min) {
			(void) rstbuf(magic);
/*			if (rstbuf(magic) == -1) ;*/
			magic = ip;
		}
		if (Cflag == ASCII || Cflag == NONE) {
			if (strncmp(magic, M_ASCII, M_STRLEN) == 0) {
				(void) memcpy(Chdr, magic, CHARS);
				chartobin(); /* populate ASCII binary header */
				hit = ASCII;
				hsize = CHARS + Hdr.h_namesize;
			}
		}
		if (Cflag == NONE) {
			mag.bite[0] = magic[0];
			mag.bite[1] = magic[1];

		}
		if (hit == NONE) {
			magic++;
			nleft--;
		}
		else {
			if (hdck() == -1) {
				magic++;
				nleft--;
				hit = NONE;
			}
			else {			/* consider possible alignment byte */
				while (nleft < hsize + align) {
					if (rstbuf(magic) == -1) {
						magic = ip;
						hit = NONE;
						break;
					}
					else 
						magic = ip;
				}
				if (hit == NONE)
					continue;
				if (*(magic + hsize - 1) != '\0') {
					magic++;
					nleft--;
					hit = NONE;
					continue;
				}
			}
		}
	}
	while (hit == NONE);

	if (hit == NONE) {
			brlog(" do_filerest bad cpio header");
			return;
	}	
	if (hit == ASCII) {
		/* copy file name to binary header this wasn't done
		** by chartobin call.
		*/
		(void) memcpy(Hdr.h_name, magic + CHARS, Hdr.h_namesize);
		Cflag = ASCII;
	}
	else { 
		brlog("do_filerest: Cpio header not ASCII|NONE");
		return;
	}

	offset = min + Hdr.h_namesize + align;
	ip = magic + offset;
	nleft -= offset;
} /* synch() */

/*
 * bread() is called by rstbuf() in order to read the next block
 * from the input archive into Cbuf.  bread() is called by main(),
 * case: IN, in order to read in file content to then be written out.
 * In case of read errors then bread attempts a max of 10 times to
 * successfully lseek then read good data.  If 10 consecutive reads
 * fail bread() attempts volume switch.  Upon a successful read()
 * bread() leaves nleft and the pointers ip and p set correctly but no
 * characters are copied and bread() returns -1.  In the case of no I/O
 * errors bread() reads from the input archive to Cbuf and then copies
 * chars to a target buffer unless filbuf is set by rstbuf().
 * d, distance to lseek if I/O error encountered
 * Converted to a multiple of Bufsize
 */
static int
bread(b, c, d)
register char	*b;
register int	c;
long		d;
{
	register char	*p = ip;
	register int	dcr;
	register int	rv;
	int		rc = 0;
	int		delta = 0;
	int		i = 0;
	int		readsize;

	if (filbuf == 1) {	/* fill buffer, memcpy no chars */
		nleft = 0;
		c = 0;
	}
	if (!Cflag) {
		/* round c up to an even number */
		c = (c+1)/2;
		c *= 2;
	}
	while (c || filbuf)  {
		while (nleft == 0 ) {
			while (TRUE) {
				errno = 0;
				if (checksize && (!bytes_left)) {
					if (Input != NULL)
						(void) g_close(Input);

					Input = NEW_INPUT;

					if (Input == NULL) {
						sprintf(ME(MP), "Job ID %s: g_open failed for %s: %s", MP->jobid, cpio_name, SE);
						longjmp(env,2);
					}
				}
				if (checksize) {
					readsize = (bytes_left > Bufsize) ?
						 Bufsize : bytes_left;
				}
				else {
					readsize = Bufsize;
				}
				rv = g_read(Input, Cbuf, readsize);

				if ((rv == 0) || ((rv == -1) && (errno == ENOSPC || errno == ENXIO))) {
					if (Input != NULL)
						(void) g_close(Input);

					Input = NEW_INPUT;

					if (Input == NULL) {
						sprintf(ME(MP), "Job ID %s: g_open failed for %s: %s", MP->jobid, cpio_name, SE);
						longjmp(env,2);
					}
				}
				else {
					break;
				}
			}
			if (rv == -1) {
				int	s;
				long	rvl;

				if (i++ > 10) {
					brlog("read error get new vol %s",SE);
					Input = NEW_INPUT;

					if (Input == NULL) {
						sprintf(ME(MP), "Job ID %s: g_open failed for %s: %s", MP->jobid, cpio_name, SE);
						longjmp(env, 2);
					}
					continue;
				}
				rvl = g_seek(Input, (s = d / Bufsize) == 0 ?
				  (long) Bufsize : (long) (s * Bufsize), 1);

				if (i == 1) {
					if ((rvl != -1) && (d % Bufsize) &&
						(d > Bufsize))
						delta = (d % Bufsize);
						d = 0;
						rc = -1;
				}
				else
					delta = 0;
			}
			else {
				if (DOTS(MP)) {
					dots (rv);
				}
				if (checksize)
					bytes_left -= rv;

				if (rv == Bufsize)
					++Blocks;
				else 				/* short read */
					sBlocks += rv;

				if (rv > delta) {
					nleft = rv - delta;
					p = Cbuf + delta;
				}
				else {
					nleft = 0;
					delta -= rv;
				}
			}
		}
		if (filbuf || i > 0) {
			filbuf = 0;
			break;
		}
		if (nleft <= c)
			dcr = nleft;
		else
			dcr = c;

		(void) memcpy( b, p, dcr );
		c -= dcr;
		d -= dcr;
		b += dcr;
		p += dcr;
		nleft -= dcr;
	}
	ip = p;
	return(rc);
} /* bread() */

/*
 * rstbuf(), reset bread() buffer,  moves incomplete potential
 * headers from Cbuf to an expansion buffer to the left of Cbuf.
 * It then forces bread() to replenish Cbuf.  Rstbuf() returns
 * the value returned by bread() to warn synch() of I/O errors.
 * nleft and ip are updated to reflect the new data available.
 */
static int
rstbuf(ptr)
char *ptr;
{
	int	rc = 0;
	int	eleft;		/* eleft: amt in expansion buffer */

	/*
	 * mv leftover bytes to expansion buffer
	 * force fill of Cbuf
	 */
	smemcpy(Cbuf - nleft, ptr, (unsigned)nleft);
	eleft = nleft;
	filbuf = 1;

	if ((rc = bread(Chdr, 0, 0l)) != -1) {
		nleft += eleft;
		ip -= eleft;
	}
	return(rc);
} /* rstbuf() */

/*
 * hdck() sanity checks the fixed length portion of the cpio header
 * -1 indicates a bad header and 0 indicates a good header
 */
static int
hdck()
{
	if (Hdr.h_nlink < 1 || Hdr.h_filesize < 0 ||
		Hdr.h_namesize <= 0 || Hdr.h_namesize >= PATH_MAX)
		return(-1);
	else
		return(0);
} /* hdck() */

static void
chartobin()		/* ASCII header read */
{
	major_t maj, rmaj;
	minor_t min, rmin;
	int ret;

	if (ret = sscanf( Chdr,
		"%6lx%8lx%8lx%8lx%8lx%8lx%8lx%8lx%8x%8x%8x%8x%8x%8lx",
		&Hdr.h_magic, &Hdr.h_ino, &Hdr.h_mode, &Hdr.h_uid, &Hdr.h_gid,
		&Hdr.h_nlink, &Hdr.h_mtime, &Hdr.h_filesize, &maj, &min, &rmaj,
		&rmin, &Hdr.h_namesize, &Hdr.h_cksum) == ASC_CNT) {

		Hdr.h_dev = makedev(maj, min);
		Hdr.h_rdev = makedev(rmaj, rmin);
	} else
		brlog("chartobin: scanf read failed with ASC_CNT %d\n", ret);
	

} /* chartobin() */

static void
child_exit()
{
	dead_pid = wait(&stat_loc);
	brlog(" child_exit dead_pid=%ld cpid=%ld stat_loc=%x",
					dead_pid, cpid, stat_loc);
} /* child_exit() */

static void
child_gone()
{
	dead_pid = wait(&stat_loc);
	brlog(" SIGPIPE dead_pid=%ld cpid=%ld stat_loc=%x",
				dead_pid, cpid, stat_loc);
} /* child_gone() */

static int
build_trailer(hdr)
char	**hdr;
{
	int	nct = 0;
	char	nm_len[10];
	char	fl_len[12];

#ifdef TRACE
	brlog("build_trailer:");
#endif
	if (Cflag == ASCII) {
#ifdef TRACE
	brlog("build_trailer: ASCII mode");
#endif
		(void) strcpy((Chdr+CHARS), "TRAILER!!!");
		nct = (strlen("TRAILER!!!") + 1);
		(void) sprintf(nm_len, "%.8lx", nct);
		(void) memcpy((Chdr+94), nm_len, 8);	/* offset to h_namesize */
		(void) sprintf(fl_len, "%.8lx", 0);	/* trailer has no data */
		(void) memcpy((Chdr+54), fl_len, 8);	/* offset to h_filesize */
		*hdr = Chdr;
		nct += CHARS;
	}
	else
		brlog("build_trailer: found non-ASCII header %d", Cflag);
		
#ifdef TRACE
	brlog("build_trailer: nct=%d", nct);
#endif
	filesz = 0;

	return(nct);
} /* build_trailer() */

/* this function changes the filename in the archive header Chdr */
static int
build_hdr(newnm, len, hd_pt)
char	*newnm;
int	*len;
char	**hd_pt;
{
	int	align;
	char	nm_len[10];

	if (Cflag != ASCII) {
		align = ((*len % 2) == 0) ? 0 : 1;
		(void) memcpy(Hdr.h_name, newnm, *len);
		if (align)
			Hdr.h_name[*len] = 0;
		Hdr.h_namesize = *len;
		*len += (HDRSIZE + align);
		*hd_pt = (char *) &Hdr;
	}
	else {
		(void) memcpy((Chdr+CHARS), newnm, *len); /* copy filename */
		(void) sprintf(nm_len, "%.8lx", *len); /* size of filename */
		(void) memcpy((Chdr+94), nm_len, 8); /* offset to h_namesize */
		*len += CHARS;
		*hd_pt = Chdr;
	}
	return(0);
} /* build_hdr() */

static int
skip_file()
{
	int	fct;
	int	ret = 0;


	for(; filesz > 0; filesz -= archive_bufsize){
		fct = filesz > archive_bufsize ? archive_bufsize : filesz;
		errno = 0;

		if (bread(Buf, fct, filesz) == -1) {
			brlog(" do_file(skip): i/o error, %s is corrupt",
						Hdr.h_name);
			ret = 1;
			break;
		}
	}
	return(ret);
} /* skip_file() */

/*
 * linking function:
 *                    Postml() checks to see if namep should be
 * linked to np.  If so, postml() removes the independent instance
 * of namep and links namep to np.
 */
static int
postml(namep, np, fp)
register char	*namep;
register char	*np;
file_rest_t	*fp;
{
	register	i;
	static struct mlk {
		dev_t	m_dev;
		ino_t	m_ino;
		char	m_name[2];
	} **mlk = 0;
	register struct mlk	*mlp;
	static unsigned	mlsize = 0;
	static nlink_t	mlinks = 0;
	static	mal_err = 0;
	char		*lnamep;
	int		ans;

	if (!mlk) {
		mlsize = LINKS;
		mlk = (struct mlk **) malloc( mlsize * sizeof(struct mlk));

		if (mlk == NULL) {
			if (!mal_err++)
				brlog("postml: malloc failed - no link info");
			return(1);
		}
			
	}
	else if (mlinks == mlsize) {
		mlsize += LINKS;
		mlk = (struct mlk **)realloc( (void *)mlk,
					mlsize * sizeof(struct mlk));
		if (mlk == NULL) {
			if (!mal_err++)
			    brlog("postml: relloc failed - partial link info");
			return(1);
		}
	}
	for(i = 0; i < mlinks; ++i) {
		mlp = mlk[i];

		if (mlp->m_ino==Hdr.h_ino  &&  mlp->m_dev==Hdr.h_dev) {
			(void) unlink(namep);
			lnamep = mlp->m_name;

/* try linking (only twice) */
			ans = 0;
			do {
				if (link(lnamep, namep) < 0) {
					ans += 1;
				}
				else {
					ans = 0;
					break;
				}
			} while (ans < 2 && missdir(np, fp) == 0);
			if (ans == 1) {
				brlog("postml Cannot create directory for %s", np);
				return(1);
			}
			else if (ans == 2) {
				brlog("postml Cannot link %s & %s", lnamep, np);
				return(1);
			}

			set_time(namep, Hdr.h_mtime, Hdr.h_mtime);
			return(0);
		}
	}
	if (!(mlk[mlinks] = (struct mlk *)malloc(strlen(np) + 2 + sizeof(struct mlk)))) {
		static int first=1;

		if (first)
			brlog("No memory for links (%ld)", mlinks);
		first = 0;
		return(1);
	}
	mlk[mlinks]->m_dev = Hdr.h_dev;
	mlk[mlinks]->m_ino = Hdr.h_ino;
	(void) strcpy(mlk[mlinks]->m_name, np);
	++mlinks;
	return(1);
} /* postml() */

void
smemcpy(to, from, count)
register char		*to;
register char		*from;
register unsigned	count;
{
	if (&to[count] <= from  ||  &from[count] <= to) {
		(void) memcpy(to, from, count);
		return;
	}
	if (to == from)
		return;

	if (to < from)
		while (count--)
			*(to++) = *(from++);
	else {
		to += count;
		from += count;

		while (count--)
			*(--to) = *(--from);
	}
	return;
} /* smemcpy() */

static int
getmem()
{
	int cnt;

	Chdr = (char *) malloc((unsigned) (CHARS+PATH_MAX+8));

	if (Chdr == NULL) {
		sprintf(ME(MP), "Job ID %s: out of memory", MP->jobid);
		brlog("no mem for Chdr size %d",(CHARS+PATH_MAX+4));
		return(1);
	}
	Cbuf = (char *)malloc((Bufsize + CHARS + PATH_MAX + 8));

	if (Cbuf  == NULL) {
		brlog(" do_filerest Cbuf malloc failed %s", SE);
		sprintf(ME(MP), "Job ID %s: out of memory", MP->jobid);
		return(1);
	}
	Cbuf += (CHARS + PATH_MAX + 8);
	Buf = (char *)malloc((unsigned) (archive_bufsize+8));

	if (Buf  == NULL) {
		sprintf(ME(MP), "Job ID %s: out of memory", MP->jobid);
		brlog(" do_filerest Buf malloc failed %s", SE);
		return(1);
	}
	/* clean allocated memory */

	for(cnt=0;cnt<(CHARS+PATH_MAX+8);cnt++)
		*(Chdr+cnt) = '\0';

	for(cnt=0;cnt<(Bufsize + CHARS + PATH_MAX + 8);cnt++)
		*(Cbuf+cnt) = '\0';

	for(cnt=0;cnt<(archive_bufsize+8);cnt++)
		*(Buf+cnt) = '\0';
	Buf += 4;	/* this allows for Buf to be adjusted
			** when archive filename pad changes
			** (see do_comprest)
			*/

	return(0);
} /* getmen() */

/* restore symbolic link */
static int
rest_flnk()
{

int padj;
struct stat statbuf;

	/* symbolic links not larger than 1024 */

	if (bread(Buf, filesz, filesz ) == -1) {
		brlog("rest_flnk: i/o error, %s is corrupt",Hdr.h_name);
		return(-1);
	}

	/* skip passed filename pad */ 
	padj = pad(CHARS+Hdr.h_namesize,0);

	/* symbolic names aren't null terminated */
	memcpy((Buf+Hdr.h_filesize+padj), '\0', 1);

	if ( lstat(Hdr.h_name, &statbuf) == 0)
		unlink(Hdr.h_name);

	if (symlink((Buf+padj), Hdr.h_name) < 0) {
		brlog("couldn't create symbolic link %s failed %s",
			Hdr.h_name ,SE);
		return(-1);

	}
	if (lchown(Hdr.h_name, Hdr.h_uid, Hdr.h_gid) < 0)
		brlog("rest_flnk: couldn't change owner/group of %s",
			Hdr.h_name, SE);

	return(0);

}
