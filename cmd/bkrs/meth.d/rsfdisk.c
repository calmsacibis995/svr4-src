/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:meth.d/rsfdisk.c	1.12.2.1"

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
#include	<errno.h>

#define VSTR		"volume \""
#define MAXSLICE	16

#define NEW_INPUT new_Input(MP, arc_name, &bytes_left, &checksize, &arc_info)

extern int	brlog();
extern int	brsndfname();
extern void	*malloc();
extern GFILE	*new_Input();
extern int	safe_write();
extern int	strfind();

extern int	bklevels;

m_info_t	*MP;

long			bytes_left = 0;
short			checksize = 0;
struct archive_info	arc_info;

static int	fsadd();
static int	send_to_fmthard();
static int	wait_on_fmthard();
static int	do_mkfs();
static void	getvolname();

static char	arc_name[PATH_MAX+1];

static char	fname[513];
static char	cmd[520];
static char	*volname = NULL;	/* fmthard vol name if any */
static int	totslices = 0;
static int	nfilsys = 0;		/* num file systems on this dev */
static int	ndpart = 0;		/* num non fs partitions */
static int	nmntpt = 0;		/* num mount points */
static char	*fsdata[MAXSLICE];	/* nfilsys pointers to fs info */
static char	*mpt[MAXSLICE];		/* nmntpt pointers to mntpt name */
static char	dpart_slices[MAXSLICE];	/* ndpart slice chars */
static char	ronly[MAXSLICE];	/* read only fs chars */

static GFILE	*arc = NULL;		/* archive device/file */
static FILE	*ar;			/* archive fd */
static int	fmtfnum = -1;		/* fmthard file num */
static FILE	*fmtfd;			/* popen fd to fmthard */

do_rsfdisk(mp)
m_info_t	*mp;
{
	int	isfile;
	int	len;
	int	readnum;
	int	ret;
	char	*dmname = NULL;
	char	rec[520];
	char	*buf;
	int	errors = 0;
	int	rserrors = 0;

	MP = mp;

	if (mp->nfsdev && strlen(mp->nfsdev))
		mp->ofsdev = mp->nfsdev;

	arc = NEW_INPUT;

	if (arc == NULL) {
		sprintf(ME(mp), "Job ID %s: g_open of %s failed: %s", mp->jobid, fname, SE);
		brlog(" do_rsfdisk open of archive %s failed %s", fname, SE);
		return(1);
	}
	ar = fdopen(arc->_file, "r");

	for (len = 1, readnum = 0, ret = 1; 1 ; readnum++) {
		BEGIN_CRITICAL_REGION;

		buf = fgets(rec, 512, ar);

		END_CRITICAL_REGION;

		if (!buf) {
			break;
		}
		len = strlen(rec);

		if (rec[0] == '*') {
			if (!readnum) {
				getvolname(rec, len);
			}
			continue;
		}
		if (rec[0] == '#') {
			if (ret = fsadd(rec, len))
				break;

			continue;
		}
		if (++totslices > MAXSLICE) {
			brlog("Too many slices: %d slices?", totslices);
			ret = 1;
			break;
		}
		if (ret = send_to_fmthard(rec, len))
			break;
	}
	if (ret) {
		(void) wait_on_fmthard();
		sprintf(ME(mp), "Job ID %s: archive format in error", mp->jobid);
		brlog("archive format in error");
		return(1);
	}
	if (ret = wait_on_fmthard()) {
		sprintf(ME(mp), "Job ID %s: fmthard failed", mp->jobid);
		brlog("fmthard failed pclose returned 0x%x", ret);
		return(1);
	}
	if (nmntpt != nfilsys) {
		brlog("%d mnt points, %d fs slices in archive",nmntpt,nfilsys);
	}
	for (ret = 0; ret < ndpart; ret++) {
		int	i;

		(void) sprintf(rec, "/bin/restore -P %s", mp->ofsdev);
		len = strlen(rec);
		rec[len - 1] = dpart_slices[ret];
#ifdef TRACE
		brlog("%s",rec);
#endif
		if ((i = bk_system(cmd)) != 0) {
			brlog("restore for %s failed ret=0x%x",mp->ofsdev,i);
			rserrors++;
		}
	}
	if (rserrors) {
		brlog("%d data partition restore errors", rserrors);
	}
	for(ret = 0; ret < nfilsys; ret++) {
		errors += do_mkfs(ret);
	}
	if (errors) {
		brlog("%d file system errors", errors);
	}
	if (errors || rserrors) {
		sprintf(ME(mp), "Job ID %s: not all partitions restored", mp->jobid);
		return(1);
	}
	return(0);
} /* do_rsfdisk() */

static int
send_to_fmthard(rec, length)
char	*rec;
int	length;
{
	int	ret;
	int	i;
	int	tag;
	int	flag;
	char	vstring[260];
	char	mntpt[260];
	char	slice;
	char	*c;

	if (fmtfd == NULL) {
		(void) sprintf(cmd, "/sbin/fmthard -s - ");

		if (volname) {
			(void) strcat(cmd, "-n ");
			(void) strcat(cmd, volname);
			(void) strcat(cmd, " ");
		}
		(void) strcat(cmd, MP->ofsdev);
#ifdef TRACE
		brlog("popen cmd=%s", cmd);
#endif
		if ((fmtfd = popen(cmd, "w")) == NULL) {
			brlog("unable to execute %s",cmd);
			return(1);
		}
/* THIS CAUSED A PROBLEM WITH DE-ANSI COMPILER	*/
/*#if #lint(on)					*/
/*#define fileno(x) (x)->_file			*/
/*#endif					*/
		fmtfnum = fileno(fmtfd);
	}
	mntpt[0] = 0;
	(void) sscanf(rec, " %c %d %d %*d %*d %*d %s",&slice,&tag,&flag,mntpt);
	brlog("slice=%c, tag=%d, flag=%d, mntpt=%s",slice,tag,flag,mntpt);

	if (MP->flags & Vflag) {
		i = strlen(MP->ofsdev);
		(void) sprintf(vstring, "%s %s", MP->ofsdev, mntpt);
		vstring[i-1] = slice;
		brsndfname(vstring);
	}
	if (mntpt[0]) {
		i = strlen(mntpt);
		c = (char *) malloc((unsigned) (i + 1));

		if (c == NULL) {
			brlog("no memory for volume name");
			return(1);
		}
		ronly[nmntpt] = (flag&10) ? 1 : 0;
		mpt[nmntpt++] = c;
		(void) strcpy(c, mntpt);
	}
	ret = safe_write(fmtfnum, rec, length);

	if (ret != length) {
		brlog("write error to fmthard - %s",SE);
		return(1);
	}
	if (!(flag&1) || mntpt[0])	/* mountable fs */
		return(0);

	dpart_slices[ndpart++] = slice;

	return(0);
} /* send_to_fmthard() */

static int
wait_on_fmthard()
{
	if (fmtfd == NULL) {
		brlog("fmthard stdin not established");
		return(-1);
	}
	return (pclose(fmtfd));
} /* wait_on_fmthard() */

static void
getvolname(rec, length)
char	*rec;
int	length;
{
	int	offset;
	int	end;

	offset = strfind(rec, VSTR);

	if (offset < 0)
		return;

	offset += strlen(VSTR);

	end = strfind((rec + offset), "\"");

	if (end < 0)
		return;

	if ((end + offset) > length)
		return;

	volname = (char *) malloc((unsigned) (end + 1));

	if (volname == NULL) {
		brlog("no memory for volume name");
		return;
	}
	(void) strncpy(volname, (rec + offset), end);

	*(volname + end) = 0;
} /* getvolname() */

static int
fsadd(rec, length)
char	*rec;
int	length;
{
	char	*fsd;

	fsd = (char *) malloc((unsigned) (length + 1));

	if (fsd == NULL) {
		brlog("no memory for fsdata");
		return(1);
	}
	fsdata[nfilsys++] = fsd;

	(void) strcpy(fsd, rec);

	return(0);
} /* fsadd() */

static int
do_mkfs(idx)
int	idx;
{
	int	ret;
	char	slice;
	char	*c;
	char	fstype[20];
	char	fsname[20];
	char	volname[20];
	char	cmd[520];
	char	special[256];
	int	blocks;
	int	inodes;
	int	len;
	char	*fi = fsdata[idx];

	if (fi[0] != '#')
		return(1);

	while((c = strchr(fi, ',')) != NULL)
		*c = ' ';

	slice = fi[1];

	(void) sscanf(&(fi[3]), " Current fsname: %s Current volname: %s Blocks: %d Inodes: %d", fsname, volname, &blocks, &inodes);

	(void) sscanf(&(fi[3]), "mkfs -F %s", fstype);
	(void) strcpy(special, MP->ofsdev);
	len = strlen(special);
	special[len-1] = slice;

	(void) sprintf(cmd, "/sbin/mkfs -F %s %s %d:%d", fstype, special, blocks, inodes); 
	brlog("cmd=%s", cmd);

	if ((ret = bk_system(cmd)) != 0) {
		brlog("mkfs for %s failed ret=0x%x",special,ret);
		return(1);
	}
	(void) sprintf(cmd,
		 "/sbin/labelit %s %s %s", special, fsname, volname);
	brlog("cmd=%s",cmd);

	if ((ret = bk_system(cmd)) != 0) {
		brlog("labelit for %s failed ret=0x%x",special,ret);
		return(1);
	}
	if (mpt[idx]) {
		(void) sprintf(cmd, "%s %s %s", ronly[idx] ? "/sbin/mount -r" : "/sbin/mount",
						special, mpt[idx]);
		brlog("cmd=%s", cmd);

		if ((ret = bk_system(cmd)) != 0) {
			brlog("mount -n  for %s failed ret=0x%x",special,ret);
			return(1);
		}
	}
	(void) sprintf(cmd,"/bin/restore -S %s", special);
	brlog("cmd=%s", cmd);

	if ((ret = bk_system(cmd)) != 0) {
		brlog("restore for %s failed ret=0x%x",special,ret);
		return(1);
	}
	return(0);
} /* do_mkfs() */
