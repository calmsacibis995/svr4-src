/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:meth.d/irest.c	1.18.2.1"

#include	<limits.h> 	/* get PATH_MAX from here, not stdio */
#include	<sys/types.h>
#include	<sys/mount.h>
#include	<sys/fstyp.h>
#include	<sys/fsid.h>
#include	<sys/stat.h>
#include	<ftw.h>
#include	<stdio.h>
#include	<fcntl.h>
#include	<errno.h>
#include	<signal.h>
#include	<method.h>
#include 	"libadmIO.h"
#include	<backup.h>
#include	<string.h>
#include	<brarc.h>
#include	<bktypes.h>

#define SM(x) set_modes(x,Stat.st_mode,Stat.st_uid,Stat.st_gid,Stat.st_mtime)

#define NEW_INPUT new_Input(MP, arc_name, &bytes_left, &checksize, &arc_info)
#define OFAIL	"open of %s failed  %s  restore incomplete"
#define WFAIL	"write of %s failed  %s  restore incomplete"
#define RFAIL	"archive read error %s for file %s restore incomplete"

extern int		brlog();
extern int		close();
extern char		*devattr();
extern char		**devreserv();
extern void		dorsresult();
extern void		dots();
extern file_rest_t	*file_req();
extern char		*get_apart();
extern int		get_dpart();
extern int		get_mnt_info();
extern argv_t		*listdgrp();
extern char		*malloc();
extern GFILE		*new_Input();
extern void		plist();
extern char		*result();
extern int		g_close();
extern GFILE		*g_open();
extern int		g_read();
extern int		g_write();
extern int		safe_stat();

extern int	bklevels;

file_rest_t	Global_f;

m_info_t	*MP;
char		*Buf;
long		bytes_left = 0;
short		checksize = 0;

static int	copy1file();

static int	media_bufsize = 4096;	/* may be changed by g_init */

static long	maxsiz;
static char	arc_name[PATH_MAX+1];

struct archive_info	arc_info;
struct archive_info	*ai = &arc_info;

struct fim_dp {
	char		*dname;
	long		cap;
	struct fim_dp	*next;
};

static struct fim_dp	*dhead = NULL;
static struct fim_dp	*dtail = NULL;
static pid_t		mypid;
static GFILE		*ifd = NULL;		/* Input device/file */
static char		*imdev = (char *) NULL;
static char		*special = (char *) NULL;
static int		again = 1;
static char		mntpt[21] = "";
static int		mntpt_len = 0;
static int		fsname_len = 0;
static int		subdir_len;

do_image_filerest(mp, argv)
m_info_t	*mp;
unsigned char	*argv[];
{
	int		left_todo = 0;
	int		ret;
	int		i;
	file_rest_t	*file_base;
	register file_rest_t	*f;

	MP = mp;
	maxsiz = 512 * ulimit(1, 0);
	mypid = getpid();

#ifdef TRACE
	brlog("do_image_filerest: ");
#endif
	if ((file_base = file_req(mp, argv, &left_todo)) == NULL) {
		sprintf(ME(mp), "Job ID %s: no legal F or D requests", mp->jobid);
		return(1);
	}
	if (left_todo <= 0) {
		goto doresult;
	}
	ai->br_blk_est = (long) -1;

	if ((ifd = NEW_INPUT) == NULL) {
		sprintf(ME(mp), "Job ID %s: g_open of %s failed: %s", mp->jobid, arc_name, SE);
		if (ifd != NULL) {
			(void) g_close(ifd);
			ifd = NULL;
		}
		return(1);
	}
	if (ifd->_size > 1) {
		media_bufsize = ifd->_size;
	}
	if (ai->br_blk_est < 0) {
		sprintf(ME(mp), "Job ID %s: cannot determine needed partition size", mp->jobid);
		if (ifd != NULL) {
			(void) g_close(ifd);
			ifd = NULL;
		}
		return(1);
	}
	mp->fstype = ai->br_fstype;

#ifdef TRACE
	brlog("archive size: %ld fstype=%s",ai->br_blk_est,ai->br_fstype);
#endif
	if (ret = get_dpart(ai->br_blk_est)) {
		if (ifd != NULL) {
			(void) g_close(ifd);
			ifd = NULL;
		}
		return(1);
	}
	fsname_len = strlen(ai->br_fsname);
	mp->nfsdev = special;

	if (MP->dtype == IS_DPART) {
		if (ifd != NULL) {
			(void) g_close(ifd);
			ifd = NULL;
		}
	}
	else {
		if (do_image_comprest(mp, 1)) {
			brlog("image restore failed");
			if (ifd != NULL) {
				(void) g_close(ifd);
				ifd = NULL;
			}
			return(1);
		}
	}
	if (mount_it()) {
		brlog("unable to mount restored image");
		if (ifd != NULL) {
			(void) g_close(ifd);
			ifd = NULL;
		}
		return(1);
	}
	mntpt_len = strlen(mntpt);

	for (i = 0, f = file_base; i < mp->n_names ; i++, f++) {

#ifdef TRACE
		brlog("BEGIN FOR LOOP i=%d", i);
#endif
		if (f->status)		/* nothing to do */
			continue;
		if (f->type) {		/* directory */
			subdir_len = f->name_len - fsname_len;
#ifdef TRACE
			brlog("subdir_len = %d",subdir_len);
#endif
			(void) try_dir(f);
		}
		else { 			/* file */
			(void) try_file(f);
		}
#ifdef TRACE
		brlog("END FOR LOOP i=%d, f->file_count=%d", i, f->file_count);
#endif
	}
doresult:
	dorsresult(mp, file_base);

	brlog("do_image_filerest imdev=%s", imdev);
	if ((imdev) && (MP->dtype != IS_DPART)) {
		ret = devfree(mypid, imdev);
		if (ret) {
			brlog("devfree of %s pid = %d failed errno = %d",
						imdev,mypid,errno);
		}
	}
	brlog("do_image_filerest mntpt=%s", mntpt);
	if (mntpt[0]) {
		(void) umount(mntpt);
		(void) rmdir(mntpt);
	}
	MP->blk_count += 511;
	MP->blk_count >>= 9;
	brlog("do_image_filerest returns blks=%d",MP->blk_count);
	if (ifd != NULL) {
		(void) g_close(ifd);
		ifd = NULL;
	}
	return(0);
} /* do_image_filerest() */

do_image_comprest(mp, seq1_isopen)
m_info_t	*mp;
int		seq1_isopen;
{
	long	vol_bytes_to_go;
	long	bytes_to_go = -1;
	char	*outdev;
	int	result = 0;
	int	ret;
	GFILE	*ofd = NULL;
	int	size;
	int	wrsize;
	int	offset;

	MP = mp;

	ret = get_mnt_info(mp);

#ifdef TRACE
	brlog("do_image_comprest: just after get_mnt_info, ret %d",
	ret);
	brlog("contents of method_info structure mp:");
	brlog("%s %s %s %s br_type %d c_count %d blk_count %d blks_per_vol %d",
	mp->method_name,mp->ofsname,mp->ofsdev,mp->ofslab,mp->br_type,
	mp->c_count,mp->blk_count,mp->blks_per_vol);
#endif
	if (ret < 0) {
		if (ifd != NULL) {
			(void) g_close(ifd);
			ifd = NULL;
		}
		return(1);
	}
	outdev = strlen(mp->nfsdev) ? mp->nfsdev : mp->ofsdev;

#ifdef TRACE
	brlog("do_image_comprest: outdev %s",
	outdev);
#endif
	if (outdev == mp->nfsdev) {
		if (mp->nfsdevmnt) {
			brlog("%s currently mounted on nfsdev %s",
				mp->nfsdevmnt, mp->nfsdev);
			sprintf(ME(mp), "Job ID %s: %s currently mounted on %s", mp->jobid, mp->nfsdevmnt, mp->nfsdev);
			if (ifd != NULL) {
				(void) g_close(ifd);
				ifd = NULL;
			}
			return(1);
		}
	}
	else {
		if (mp->ofsdevmnt) {
			brlog("%s currently mounted on ofsdev %s",
				mp->ofsdevmnt, mp->ofsdev);
			sprintf(ME(mp), "Job ID %s: %s currently mounted on %s", mp->jobid, mp->nfsdevmnt, mp->nfsdev);
			if (ifd != NULL) {
				(void) g_close(ifd);
				ifd = NULL;
			}
			return(1);
		}
	}
	if (mp->meth_type == IS_IMAGE) {
		if (!strlen(outdev)) {
			brlog(" nfsdev and ofsdev both NULL");
			sprintf(ME(mp), "Job ID %s: nfsdev and ofsdev both NULL", mp->jobid);
			if (ifd != NULL) {
				(void) g_close(ifd);
				ifd = NULL;
			}
			return(1);
		}
	}

	BEGIN_CRITICAL_REGION;

	ofd = g_open(outdev, O_WRONLY, 0);

/*  debug  */
#ifdef TRACE
	brlog("do_image_comprest: just after g_open, ofd %d",
	ofd);
#endif
	END_CRITICAL_REGION;

	if (ofd == NULL) {
		brlog(" can't open %s - %s",outdev, SE);
		sprintf(ME(mp), "Job ID %s: g_open of %s failed: %s", mp->jobid, outdev, SE);
		return(1);
	}
	if ((Buf = malloc((unsigned) media_bufsize)) == NULL) {
		brlog(" new_vol: cannot malloc %d bytes %s", media_bufsize, SE);
		sprintf(ME(mp), "Job ID %s: out of memory", mp->jobid);
		if (ifd != NULL) {
			(void) g_close(ifd);
			ifd = NULL;
		}
		if (ofd != NULL) {
			(void) g_close(ofd);
			ofd = NULL;
		}
		return(1);
	}
/*  debug  */
#ifdef TRACE
	brlog("do_image_comprest: just before while bytes_to_go %d result %d",
	bytes_to_go,result);
#endif
	while (bytes_to_go && !result) {
		if (seq1_isopen) {
			seq1_isopen = 0;
			goto isopen;
		}
		if (ifd != NULL) {
			(void) g_close(ifd);
			ifd = NULL;
		}
		if ((ifd = NEW_INPUT) == NULL) {
			result = 1;
			break;
		}
isopen:
		if (ai->br_seqno == 1) {
			if (bytes_to_go < 0)
				bytes_to_go = ai->br_blk_est << 9;
		}
		vol_bytes_to_go = (bytes_to_go < bytes_left) ?
					bytes_to_go : bytes_left;
		ret = 1;

		while ((vol_bytes_to_go > 0) && (ret != 0)) {
			size = (vol_bytes_to_go < media_bufsize) ?
				vol_bytes_to_go : media_bufsize;

			ret = g_read(ifd, Buf, size);

			if (ret != size) {
				if (ret == 0)
					break;
				else if (ret > 0) {
					size = ret;
				}
				else {
					brlog("archive read error %s", SE);
					sprintf(ME(mp), "Job ID %s: g_read failed: %s", mp->jobid, SE);
					result = 1;
					break;
				}
			}
			if (DOTS(MP)) {
				dots (ret);
			}
			offset = 0;
			wrsize = size;
			ret = g_write(ofd, (Buf + offset), wrsize);

			if (ret != wrsize) {
				brlog("%s write error %s",arc_name, SE);
				sprintf(ME(mp), "Job ID %s: g_write failed: %s", mp->jobid, SE);
				result = 1;
				break;
			}
			bytes_to_go -= wrsize;

			if ((MP->blks_per_vol) > 0)
				vol_bytes_to_go -= size;

			mp->blk_count += wrsize;
		}
	}
	if (ofd != NULL) {
		(void) g_close(ofd);
		ofd = NULL;
	}
	if (ifd != NULL) {
		(void) g_close(ifd);
		ifd = NULL;
	}
	(void) free (Buf);
	mp->blk_count >>= 9;
	brlog("do_image_comprest returns blks=%d",MP->blk_count);
	return(result);
} /* do_image_comprest() */

static int
get_dpart(arc_size)
long	arc_size;
{
	int	ngrp;
	argv_t	*grp;

	if (MP->dtype == IS_DPART) {
		imdev = MP->dname;
		goto just_mount;
	}
	grp = listdgrp(IMAGE_BKRS);

	if (grp == (argv_t *) NULL) {
		brlog("listdgrp error: no members of device group %s",
						IMAGE_BKRS);
		sprintf(ME(MP), "Job ID %s: no members of device group %s", MP->jobid, IMAGE_BKRS);
		return(1);
	}
	ngrp = grp_choose((char **)grp, arc_size);

	if (!ngrp) {
		brlog("archive size %d blocks: no partitions large enough in device group %s", arc_size, IMAGE_BKRS);
		sprintf(ME(MP), "Job ID %s: no partitions large enough in device group %s", MP->jobid, IMAGE_BKRS);
		return(1);
	}
	while (((imdev = get_apart()) == NULL) && again) {
		sleep(10);
	}
#ifdef TRACE
	brlog("imdev=%s",imdev);
#endif

just_mount:
	if (!(special = devattr(imdev, "bdevice"))) {
		if (*imdev != '/') {
			brlog("no bdevice for alias %s",imdev);
			return(1);
		}
		else {
			special = imdev;
		}
	}
	return(0);
} /* get_dpart() */

static int
grp_choose(grp, arc_size)
char	**grp;
long	arc_size;
{
	struct fim_dp	*dinfo;
	struct fim_dp	*tmp;
	char		*g;
	char		**wrk;
	char		*cap;
	long		lcap;
	int		num = 0;

	wrk = (char **)grp;

	for (g = *wrk; g; g = *(++wrk)) {
		if (!(cap = devattr(g, "capacity")))
			continue;

		if ((lcap = atol(cap)) < arc_size)
			continue;

		dinfo = (struct fim_dp *) malloc( sizeof(struct fim_dp));

		if (!dinfo) {
			brlog("get_dpart: no memory");
			sprintf(ME(MP), "Job ID %s: out of memory", MP->jobid);
			return(1);
		}
		dinfo->dname = g;
		dinfo->cap = lcap;

		for (tmp = dhead; tmp; tmp = tmp->next) {
			if (lcap > tmp->cap)
				break;
			if (tmp->next == NULL) {
				dtail = dinfo;
				break;
			}
		}
		if (!tmp) {
			dhead = dtail = dinfo;
			dinfo->next = (struct fim_dp *) NULL;
			num++;
		}
		else {
			dinfo->next = tmp->next;
			tmp->next = dinfo;
			num++;
		}
	}
#ifdef TRACE
	plist();
#endif
	return(num);
} /* grp_choose() */

static char *
get_apart()
{
	char	*wrk[2];
	char	**result;
	char	**list[2];
	struct fim_dp	*dinfo;

	wrk[1] = (char *) NULL;
	list[0] = (char **) wrk;
	list[1] = (char **) NULL;

	for (dinfo = dhead, again = 0; dinfo; dinfo = dinfo->next) {
		wrk[0] = dinfo->dname;
		result = devreserv(mypid, list);
#ifdef TRACE
		brlog("result=%x for %s",result,wrk[0]);
#endif
		if (result)
			break;
		else if (errno == EAGAIN)
			again++;
#ifdef TRACE
		else
			brlog("errno=%d for %s",errno,dinfo->dname);
#endif
	}
	if (result) {
		return(dinfo->dname);
	}
	return((char *)NULL);
} /* get_apart() */

static int
mount_it()
{
	int	fstyp;

	sprintf(mntpt, "/.rs%ld", mypid);
	
	if (mknod(mntpt, 040755, 0)) {
		brlog("mknod of %s failed %s",mntpt,SE);
		sprintf(ME(MP), "Job ID %s: mknod of %s failed", MP->jobid, mntpt);
		return(1);
	}
#ifdef TRACE
brlog("sysfs GETFSIND for %s",MP->fstype);
#endif
	fstyp = sysfs(GETFSIND, MP->fstype);

	if (mount(special, mntpt, MS_FSS, fstyp, 0, 0)) {
		brlog("mount of %s on %s failed %s",special,mntpt,SE);
		sprintf(ME(MP), "Job ID %s: mount of %s on %s failed", MP->jobid, special, mntpt);
		return(1);
	}
} /* mount_it() */

static int
try_file(f)
register file_rest_t	*f;
{
	char		fname[PATH_MAX];
	char		*newname;
	char		*np;
	struct stat	Stat;
	struct stat	Nstat;
	ushort		ftype;
	ushort		A_special;
	int		ret;
	int		fd;
	int		nbytes;

	strcpy(fname, mntpt);
	strcat(fname, ((f->name) + fsname_len)); 

	brlog("try_file: fname=%s f->name=%s mntpt=%s",fname,f->name,mntpt);

	if (safe_stat(fname, &Stat)) {		/* not found */
		brlog("ARCHIVE FILE (%s) NOT FOUND!", fname);
		f->status = F_UNSUCCESS;
		f->rindx = R_NOTFOUND;
		return(1);
	}


	if (f->ldate < Stat.st_mtime) { 	/* too new */
		brlog("ARCHIVE FILE (%s) NEWER THAN EXISTING FILE (%s)!", f->name, fname);

		if (f->type == 1) {
			brlog("Directory %s NOT restored, continuing...",fname);
			return (0);
		}
		f->status = F_UNSUCCESS;
		f->rindx = R_DATE;
		return (1);
	}
	if (maxsiz < Stat.st_size) {
		brlog(" %s size %d exceeds ulimit %d", f->name,Stat.st_size,maxsiz);
		f->status = F_UNSUCCESS;
		f->rindx = R_ULIMIT;
		return (1);
	}
	if (f->idnum) {
		if (Stat.st_uid != f->idnum) {
			f->status = F_UNSUCCESS;
			f->rindx = R_NOTOWN;
#ifdef TRACE
			brlog("try_file: uid %ld not owner of %s(%ld)",
				f->idnum, f->name, Stat.st_uid);
#endif
			return(1);
		}
	}
	newname = (f->rename_len) ? f->rename : f->name;
	np = newname;
	brlog("newname=%s",newname);

	f->status = F_SUCCESS;	/* fail will change */
	f->rindx = R_SUCCESS;

	/* if directory exists leave it alone */
	if (S_ISDIR(Stat.st_mode))
		return(0);

	if ((ret = check_nondir(newname, f, Stat.st_mtime)) == -1) {
		return(1);
	}
	ftype = Stat.st_mode & S_IFMT;
	A_special = (ftype == S_IFBLK) ||  (ftype == S_IFCHR)
						||  (ftype == S_IFIFO);

	if (A_special) {
		if ((Stat.st_mode & S_IFMT) == S_IFIFO)
			Stat.st_rdev = 0;
		if (create_a_special(newname,f,np,Stat.st_mode,Stat.st_rdev)) {
			return(1);
		}
		SM(newname);		/* set modes */
		f->file_count++;
#ifdef TRACE
	brlog("f=0x%x, f->file_count=%d", f, f->file_count);
#endif
		return(0);
	}
	if (ret == 0) {
		if (ftype == S_IFDIR) {
			if (ret = mkdir(newname, Stat.st_mode)) {
				brlog("mkdir of %s failed - %s, ret=%d",newname,SE, ret);
				return(1);
			}
			if (ret = chown(newname, Stat.st_uid, Stat.st_gid)) {
				brlog("chown of %s failed  %s, ret=%d", newname, SE, ret); 
				return(1);
			}
		}
		else {
			if (create_a_reg(newname, np, f, Stat.st_mode, &fd) < 0) {
				brlog("create of %s failed  %s", newname, SE); 
				return(1);
			}
			nbytes = copy1file(fname, &fd, f, newname);
			MP->blk_count += nbytes;
		}
	}
	SM(newname);		/* set modes */
	f->file_count++;
#ifdef TRACE
	brlog("f=0x%x, f->file_count=%d", f, f->file_count);
#endif
	return(0);
} /* try_file() */

static int
copy1file(fname, fd, f, oname)
char		*fname;
char		*oname;
int		*fd;
file_rest_t	*f;
{
	char	buf[5120];
	int	rdsize;
	int	wrsize;
	int	nbytes = 0;

	if (f->rest_fd >= 0) {
		close(f->rest_fd);
		f->rest_fd = -1;
	}
	if ((f->rest_fd = open(fname, O_RDONLY)) < 0) {
		brlog("open of %s failed %s", fname, SE);

		if (!RM(f)) {
			RM(f) = result(2,OFAIL, fname, SE);
		}
		if (! f->type) {
			f->status = F_UNSUCCESS;
			f->rindx = R_INCOMP;
		}
		(void) close(*fd);
		*fd = -1;
		return(0);
	}
	while ((rdsize = read(f->rest_fd, buf, 5120)) > 0) {
		wrsize = write(*fd, buf, rdsize);

		if (wrsize != rdsize) {
			brlog("write to %s failed %s",oname,SE);
			if (!RM(f)) {
				RM(f) = result(2,WFAIL, oname, SE);
			}
			if (!f->type) {
				f->status = F_UNSUCCESS;
				f->rindx = R_INCOMP;
			}
		}
		else {
			nbytes += wrsize;
		}
	}
	if (rdsize < 0) {
		if (!RM(f)) {
			RM(f) = result(2, RFAIL, SE, fname);
			f->status = F_UNSUCCESS;
			f->rindx = R_INCOMP;
		}
	}
	(void) close(*fd);
	(void) close(f->rest_fd);
	f->rest_fd = -1;
	*fd = -1;
	return(nbytes);
} /* copy1file() */

static int
try_em(nm, statptr, flags )
char		*nm;
struct stat	*statptr;
int		flags;
{
	char		tmp1_name[PATH_MAX];
	char		tmp2_name[PATH_MAX];
	file_rest_t	f1;

#ifdef TRACE
	brlog ("try_em: nm=%s, Global_f.name=%s", nm, Global_f.name);
#endif
	strcpy(tmp1_name, Global_f.name);
	strcat(tmp1_name, (nm + mntpt_len + subdir_len));
	f1 = Global_f;
	f1.name = tmp1_name;

	if (Global_f.rename_len) {
		strcpy(tmp2_name, Global_f.rename);
		strcat(tmp2_name, (nm + mntpt_len + subdir_len));
		f1.rename = tmp2_name;
	}
	try_file(&f1);
	Global_f.file_count = f1.file_count;
	return (0);
} /* try_em() */

static int
try_dir(f)
register file_rest_t	*f;
{
	char	nm[PATH_MAX];

	Global_f = *f;
#ifdef TRACE
	brlog ("try_dir: f->name=%s", f->name);
#endif
	strcpy(nm, mntpt);
	strcat(nm, (f->name + fsname_len));

	if (nftw(nm, try_em, 10, FTW_PHYS | FTW_MOUNT)) {
		brlog ("Cannot traverse the tree! Wow!");
		f->status = F_UNSUCCESS;
		f->rindx = R_INCOMP;
		return (1);
	}
	f->file_count = Global_f.file_count - 1; /* Don't count the directory */
} /* try_dir() */

#ifdef TRACE
void
plist()
{
	struct fim_dp	*dinfo;

	for (dinfo = dhead; dinfo; dinfo = dinfo->next) {
		brlog("dname=%s cap=%d",dinfo->dname,dinfo->cap);
	}
} /* plist() */
#endif
