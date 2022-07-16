/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libpkg:dstream.c	1.9.12.2"

/*LINTLIBRARY*/

#include <stdio.h>
#include <string.h>
#include <signal.h>
#ifndef PRESVR4
#include <stdlib.h>
#endif
#include <sys/types.h>
#include <sys/statfs.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mkdev.h>
#ifndef PRESVR4
#include <sys/statvfs.h>
#endif
#include <fcntl.h>
#ifdef u3b2
#include <sys/sys3b.h>
#endif
#include <libgenIO.h>

static
struct stat	orig_st_buf;	/* Stat structure of original file (3B2/CTC) */
extern int	errno;
extern FILE	*epopen();
extern int	pkgnmchk(),
		epclose(),
		esystem(),
		getvol(),
		ioctl(),
		write(),
		read(),
		close(),
		ckvolseq(),
		atoi();
extern void	ecleanup(),
		cleanup(),
		rpterr(),
		progerr(), 
		logerr(),
		free();
extern char	*devattr();

#define CMDSIZ	512
#define LSIZE	128
#ifndef PRESVR4
#define DDPROC		"/usr/bin/dd"
#define CPIOPROC	"/usr/bin/cpio"
#else
#define DDPROC		"/bin/dd"
#define CPIOPROC	"/bin/cpio"
#endif

#define ERR_UNPACK	"attempt to process datastream failed"
#define ERR_DSTREAMSEQ	"datastream sequence corruption"
#define ERR_TRANSFER    "unable to complete package transfer"
#define MSG_MEM		"no memory"
#define MSG_CMDFAIL	"- process <%s> failed, exit code %d"
#define MSG_TOC		"- bad format in datastream table-of-contents"
#define MSG_EMPTY	"- datastream table-of-contents appears to be empty"
#define MSG_POPEN	"- popen of <%s> failed, errno=%d"
#define MSG_OPEN	"- open of <%s> failed, errno=%d"
#define MSG_PCLOSE	"- pclose of <%s> failed, errno=%d"
#define MSG_PKGNAME	"- invalid package name in datastream table-of-contents"
#define MSG_NOPKG	"- package <%s> not in datastream"
#define MSG_STATFS	"- unable to stat filesystem, errno=%d"
#define MSG_NOSPACE	"- not enough tmp space, %d free blocks required"

struct dstoc {
	int	cnt;
	char	pkg[16];
	int	nparts;
	long	maxsiz;
	char    volnos[128];
	struct dstoc *next;
} *ds_head, *ds_toc;

#define	ds_nparts	ds_toc->nparts
#define	ds_maxsiz	ds_toc->maxsiz
	
int	ds_totread;	/* total number of parts read */
int	ds_fd = -1;
int	ds_curpartcnt = -1;
int	ds_next();
int	ds_ginit();
int	ds_close();

static FILE	*ds_pp;
static int	ds_realfd = -1;	/* file descriptor for real device */
static int	ds_read;	/* number of parts read for current package */
static int	ds_volno;	/* volume number of current volume */
static int	ds_volcnt;	/* total number of volumes */
static char	ds_volnos[128];	/* parts/volume info */
static char	*ds_device;
static int	ds_volpart;	/* number of parts read in current volume, including skipped parts */
static int	ds_bufsize;
static int	ds_skippart;	/* number of parts skipped in current volume */
static int	ds_getnextvol(), ds_skip();

void
ds_order(list)
char *list[];
{
	struct dstoc *toc_pt;
	register int j, n;
	char	*pt;

	toc_pt = ds_head;
	n = 0;
	while(toc_pt) {
		for(j=n; list[j]; j++) {
			if(!strcmp(list[j], toc_pt->pkg)) {
				/* just swap places in the array */
				pt = list[n];
				list[n++] = list[j];
				list[j] = pt;
			}
		}
		toc_pt = toc_pt->next;
	}
}

static char *pds_header;
static char *ds_header;
static int ds_headsize;

static char *
ds_gets(buf, size)
char *buf;
int size;
{
	int length;
	char *nextp;

	nextp = strchr(pds_header, '\n');
	if(nextp == NULL) {
		length = strlen(pds_header);
		if(length > size)
			return 0;
		if((ds_header = (char *)realloc(ds_header, ds_headsize + 512)) == NULL) 
			return 0;
		if(read(ds_fd, ds_header + ds_headsize, 512) < 512)
			return 0;
		ds_headsize += 512;
		nextp = strchr(pds_header, '\n');
		if(nextp == NULL)
			return 0;
		*nextp = '\0';
		if(length + (int)strlen(pds_header) > size)
			return 0;
		(void)strncpy(buf + length, pds_header, strlen(pds_header));
		buf[length + strlen(pds_header)] = '\0';
		pds_header = nextp + 1;
		return buf;
	}
	*nextp = '\0';
	if((int)strlen(pds_header) > size)
		return 0;
	(void)strncpy(buf, pds_header, strlen(pds_header));
	buf[strlen(pds_header)] = '\0';
	pds_header = nextp + 1;
	return buf;
}

/*
 * function to determine if media is datastream or mounted 
 * floppy
 */
int
ds_readbuf(device)
char *device;
{
	char buf[512];

	if(ds_fd >= 0)
		(void)close(ds_fd);
	if((ds_fd = open(device, 0)) >= 0 
	  	&& read(ds_fd, buf, 512) == 512
	  	&& strncmp(buf, "# PaCkAgE DaTaStReAm", 20) == 0) {
		if((ds_header = (char *)calloc(512, 1)) == NULL) {
			progerr(ERR_UNPACK);
			logerr(MSG_MEM);
			(void)ds_close(0);
			return 0;
		}
		memcpy(ds_header, buf, 512);
		ds_headsize = 512;

		if(ds_ginit(device) < 0) {
			progerr(ERR_UNPACK);
			logerr(MSG_OPEN, device, errno);
			(void)ds_close(0);
			return 0;
		}
		return 1;
	} else if(ds_fd >= 0) {
		(void)close(ds_fd);
		ds_fd = -1;
	}
	return 0;
}

/*
 * Determine how many additional volumes are needed for current package.
 * Note: a 0 will occur as first volume number when the package begins
 * on the next volume.
 */
static int
ds_volsum(toc)
struct dstoc *toc;
{
	int curpartcnt, volcnt;
	char volnos[128], tmpvol[128];
	if(toc->volnos[0]) {
		int index, sum;
		sscanf(toc->volnos, "%d %[ 0-9]", &curpartcnt, volnos);
		volcnt = 0;
		sum = curpartcnt;
		while(sum < toc->nparts && sscanf(volnos, "%d %[ 0-9]", &index, tmpvol) >= 1) {
			(void)strcpy(volnos, tmpvol);
			volcnt++;
			sum += index;
		}
		/* side effect - set number of parts read on current volume */
		ds_volpart = index;
		return volcnt;
	}
	ds_volpart += toc->nparts;
	return 0;
}

/* initialize ds_curpartcnt and ds_volnos */
static void
ds_pkginit()
{
	if(ds_toc->volnos[0])
		sscanf(ds_toc->volnos, "%d %[ 0-9]", &ds_curpartcnt, ds_volnos);
	else
		ds_curpartcnt = -1;
}

/* functions to pass current package info to exec'ed program */
void
ds_putinfo(buf)
char *buf;
{
	(void)sprintf(buf, "%d %d %d %d %d %d %d %d %d %d %s", ds_fd, ds_realfd, ds_volcnt, ds_volno, ds_totread, ds_volpart, ds_skippart, ds_bufsize, ds_toc->nparts, ds_toc->maxsiz, ds_toc->volnos);
}

int
ds_getinfo(string)
char *string;
{
	ds_toc = (struct dstoc *)calloc(1, sizeof(struct dstoc));
	(void)sscanf(string, "%d %d %d %d %d %d %d %d %d %d %[ 0-9]", &ds_fd, &ds_realfd, &ds_volcnt, &ds_volno, &ds_totread, &ds_volpart, &ds_skippart, &ds_bufsize, &ds_toc->nparts, &ds_toc->maxsiz, ds_toc->volnos);
	ds_pkginit();
	return ds_toc->nparts;
}

int
ds_init(device, pkg, norewind)
char	*device;
char	**pkg;
char	*norewind;
{
	struct dstoc *tail, *toc_pt; 
	char	*ret;
	char	cmd[CMDSIZ];
	char	line[LSIZE+1];
	int	i, n, count = 0;

	if(!ds_header) {
		if(ds_fd >= 0)
			(void)ds_close(0);
		/* always start with rewind device */
		if((ds_fd = open(device, 0)) < 0) {
			progerr(ERR_UNPACK);
			logerr(MSG_OPEN, device, errno);
			return -1;
		}
		if((ds_header = (char *)calloc(512, 1)) == NULL) {
			progerr(ERR_UNPACK);
			logerr(MSG_MEM);
			return -1;
		}
		if(ds_ginit(device) < 0) {
			(void)ds_close(0);
			progerr(ERR_UNPACK);
			logerr(MSG_OPEN, device, errno);
			return -1;
		}
		if(read(ds_fd, ds_header, 512) != 512) {
			rpterr();
			progerr(ERR_UNPACK);
			logerr(MSG_TOC);
			(void)ds_close(0);
			return -1;
		}

		while(strncmp(ds_header, "# PaCkAgE DaTaStReAm", 20) != 0) {
			if(!norewind || count++ > 10) {
				progerr(ERR_UNPACK);
				logerr(MSG_TOC);
				(void)ds_close(0);
				return -1;
			}
			if(count > 1)
				while(read(ds_fd, ds_header, 512) > 0)
					;
			(void)ds_close(0);
			if((ds_fd = open(norewind, 0)) < 0) {
				progerr(ERR_UNPACK);
				logerr(MSG_OPEN, device, errno);
				return -1;
			}
			if(ds_ginit(device) < 0) {
				(void)ds_close(0);
				progerr(ERR_UNPACK);
				logerr(MSG_OPEN, device, errno);
				return -1;
			}
			if(read(ds_fd, ds_header, 512) != 512) {
				rpterr();
				progerr(ERR_UNPACK);
				logerr(MSG_TOC);
				(void)ds_close(0);
				return -1;
			}
		}
		/*
		 * remember rewind device for ds_close to rewind at
		 * close
		 */
		if(count >= 1) 
			ds_device = device;
		ds_headsize = 512;

	}
	pds_header = ds_header;
	/* read datastream table of contents */
	ds_head = tail = (struct dstoc *)0;
	ds_volcnt = 1;
	
	while(ret=ds_gets(line, LSIZE)) {
		if(strcmp(line, "# end of header") == 0)
			break;
		if(!line[0] || line[0] == '#')
			continue;
		toc_pt = (struct dstoc *) calloc(1, sizeof(struct dstoc));
		if(!toc_pt) {
			progerr(ERR_UNPACK);
			logerr(MSG_MEM);
			ecleanup();
			return(-1);
		}
		if(sscanf(line, "%14s %d %d %[ 0-9]", toc_pt->pkg, &toc_pt->nparts, 
		&toc_pt->maxsiz, toc_pt->volnos) < 3) {
			progerr(ERR_UNPACK);
			logerr(MSG_TOC);
			free(toc_pt);
			ecleanup();
			return(-1);
		}
		if(tail) {
			tail->next = toc_pt;
			tail = toc_pt;
		} else
			ds_head = tail = toc_pt;
		ds_volcnt += ds_volsum(toc_pt);
	}
	if(!ret) {
		progerr(ERR_UNPACK);
		logerr(MSG_TOC);
		return -1;
	}
	sighold(SIGINT);
	sigrelse(SIGINT);
	if(!ds_head) {
		progerr(ERR_UNPACK);
		logerr(MSG_EMPTY);
		return(-1);
	}
	/* this could break, thanks to cpio command limit */
#ifndef PRESVR4
	(void) sprintf(cmd, "%s -icdumD -C 512 ", CPIOPROC);
#else
	(void) sprintf(cmd, "%s -icdum -C 512 ", CPIOPROC);
#endif
	for(i=0; pkg[i]; i++) {
		if(!strcmp(pkg[i], "all"))
			continue;
		strcat(cmd, pkg[i]);
		strcat(cmd, "'/*' ");
	}

	if(n = esystem(cmd, ds_fd, -1)) {
		rpterr();
		progerr(ERR_UNPACK);
		logerr(MSG_CMDFAIL, cmd, n);
		return(-1);
	}

	ds_toc = ds_head;
	ds_totread = 0;
	ds_volno = 1;
	return(0);
}

ds_findpkg(device, pkg)
char	*device, *pkg;
{
	char	*pkglist[2];
	int	nskip, ods_volpart;

	if(ds_head == NULL) {
		pkglist[0] = pkg;
		pkglist[1] = NULL;
		if(ds_init(device, pkglist))
			return(-1);
	}

	if(!pkg || pkgnmchk(pkg, "all", 0)) {
		progerr(ERR_UNPACK);
		logerr(MSG_PKGNAME);
		return(-1);
	}
		
	nskip = 0;
	ds_volno = 1;
	ds_volpart = 0;
	ds_toc = ds_head;
	while(ds_toc) {
		if(!strcmp(ds_toc->pkg, pkg))
			break;
		nskip += ds_toc->nparts; 
		ds_volno += ds_volsum(ds_toc);
		ds_toc = ds_toc->next;
	}
	if(!ds_toc) {
		progerr(ERR_UNPACK);
		logerr(MSG_NOPKG, pkg);
		return(-1);
	}

	ds_pkginit();
	ds_skippart = 0;
	if(ds_curpartcnt > 0) {
		ods_volpart = ds_volpart;
		/* skip past archives belonging to last package on current volume */
		if(ds_volpart > 0 && ds_getnextvol(device))
			return -1;
		ds_totread = nskip - ods_volpart;
		if(ds_skip(device, ods_volpart))
			return -1;
	} else if(ds_curpartcnt < 0) {
		if(ds_skip(device, nskip - ds_totread))
			return -1;
	} else
		ds_totread = nskip;
	ds_read = 0;
	return(ds_nparts);
}

/*
 * Get datastream part
 * Call for first part should be preceded by
 * call to ds_findpkg
 */

ds_getpkg(device, n, dstdir) 
char	*device;
int	n;
char	*dstdir;
{
	struct statfs buf;

	if(ds_read >= ds_nparts)
		return(2);

	if(ds_read == n)
		return(0);
	else if((ds_read > n) || (n > ds_nparts))
		return(2);

	if(ds_maxsiz > 0) {
		if(statfs(".", &buf, sizeof(buf), 0)) {
			progerr(ERR_UNPACK);
			logerr(MSG_STATFS, errno);
			return(-1);
		}
		if((ds_maxsiz + 50) > ((double)buf.f_bfree * (buf.f_bsize / 512))) {
			progerr(ERR_UNPACK);
			logerr(MSG_NOSPACE, ds_maxsiz+50);
			return(-1);
		}
	}
	return ds_next(device, dstdir);
}

static int
ds_getnextvol(device)
char *device;
{
	char prompt[128];
	int n;

	if(ds_close(0))
		return -1;
	(void)sprintf(prompt, "Insert %%v %d of %d into %%p", ds_volno, ds_volcnt);
	if(n = getvol(device, NULL, NULL, prompt))
		return n;
	if((ds_fd = open(device, 0)) < 0)
		return -1;
	if(ds_ginit(device) < 0) {
		(void)ds_close(0);
		return -1;
	}
	ds_volpart = 0;
	return 0;
}

/*
 * called by ds_findpkg to skip past archives for unwanted packages 
 * in current volume 
 */
static int
ds_skip(device, nskip)
char *device;
int	nskip;
{
	char	cmd[CMDSIZ];
	int	n, onskip = nskip;
	
	while(nskip--) {
		/* skip this one */
#ifndef PRESVR4
		(void) sprintf(cmd, "%s -ictD -C 512 > /dev/null", CPIOPROC);
#else
		(void) sprintf(cmd, "%s -ict -C 512 > /dev/null", CPIOPROC);
#endif
		if(n = esystem(cmd, ds_fd, -1)) {
			rpterr();
			progerr(ERR_UNPACK);
			logerr(MSG_CMDFAIL, cmd, n);
			nskip = onskip;
			if(ds_volno == 1 || ds_volpart > 0)
				return n;
			if(n = ds_getnextvol(device))
				return n;
		}
	}
	ds_totread += onskip;
	ds_volpart = onskip;
	ds_skippart = onskip;
	return(0);
}

/* skip to end of package if necessary */
void ds_skiptoend(device)
char *device;
{
	if(ds_read < ds_nparts && ds_curpartcnt < 0)
		(void)ds_skip(device, ds_nparts - ds_read);
}


int
ds_next(device, instdir)
char *device;
char *instdir; /* current directory where we are spooling package */
{
	char	cmd[CMDSIZ], tmpvol[128];
	int	nparts, n, index;

	while(1) {
		if(ds_read + 1 > ds_curpartcnt && ds_curpartcnt >= 0) {
			ds_volno++;
			if(n = ds_getnextvol(device))
				return n;
			(void)sscanf(ds_volnos, "%d %[ 0-9]", &index, tmpvol);
			(void)strcpy(ds_volnos, tmpvol);
			ds_curpartcnt += index;
		}
#ifndef PRESVR4
		(void) sprintf(cmd, "%s -icdumD -C 512", CPIOPROC);
#else
		(void) sprintf(cmd, "%s -icdum -C 512", CPIOPROC);
#endif
		if(n = esystem(cmd, ds_fd, -1)) {
			rpterr();
			progerr(ERR_UNPACK);
			logerr(MSG_CMDFAIL, cmd, n);
		}
		if(ds_read == 0)
			nparts = 0;
		else
			nparts = ds_toc->nparts;
		if(n || (n = ckvolseq(instdir, ds_read + 1, nparts))) {
			if(ds_volno == 1 || ds_volpart > ds_skippart) 
				return -1;
				
			if(n = ds_getnextvol(device))
				return n;
			continue;
		}
		ds_read++;
		ds_totread++;
		ds_volpart++;
	
		return(0);
	}
}

/*
 * ds_ginit: Determine the device being accessed, set the buffer size,
 * and perform any device specific initialization.  For the 3B2,
 * a device with major number of 17 (0x11) is an internal hard disk,
 * unless the minor number is 128 (0x80) in which case it is an internal
 * floppy disk.  Otherwise, get the system configuration
 * table and check it by comparing slot numbers to major numbers.
 * For the special case of the 3B2 CTC several unusual things must be done.
 * To enable
 * streaming mode on the CTC, the file descriptor must be closed, re-opened
 * (with O_RDWR and O_CTSPECIAL flags set), the STREAMON ioctl(2) command
 * issued, and the file descriptor re-re-opened either read-only or write_only.
 */

static char ds_ctcflg;

int
ds_ginit(device)
char *device;
{
	int oflag, i, size;
#ifdef u3b2
	major_t maj;
	minor_t min;
	int nflag, count, size;
	struct s3bconf *buffer;
	struct s3bc *table;
	struct stat st_buf;
	int devtype;
	char buf[512];
#endif /* u3b2 */
	char *pbufsize, cmd[CMDSIZ];
	int fd2, fd;

	if((pbufsize = devattr(device, "bufsize")) != NULL) {
		ds_bufsize = atoi(pbufsize);
		(void)free(pbufsize);
	} else
		ds_bufsize = 512;
	oflag = fcntl(ds_fd, F_GETFL, 0);
#ifdef u3b2
	devtype = G_NO_DEV;
	if (fstat(ds_fd, &st_buf) == -1)
		return(-1);
	if (!S_ISCHR(st_buf.st_mode) && !S_ISBLK(st_buf.st_mode))
		goto lab;

	/* We'll have to add a remote attribute to stat but this 
	** should work for now.
	*/
	else if (st_buf.st_dev & 0x8000)	/* if remote  rdev */
		goto lab;

	maj = major(st_buf.st_rdev);
	min = minor(st_buf.st_rdev);
	if (maj == 0x11) { /* internal hard or floppy disk */
		if (min & 0x80)
			devtype = G_3B2_FD; /* internal floppy disk */
		else
			devtype = G_3B2_HD; /* internal hard disk */
	} else {
		if (sys3b(S3BCONF, (struct s3bconf *)&count, sizeof(count)) == -1)
			return(-1);
		size = sizeof(int) + (count * sizeof(struct s3bconf));
		buffer = (struct s3bconf *)malloc((unsigned)size);
		if (sys3b(S3BCONF, buffer, size) == -1)
			return(-1);
		table = (struct s3bc *)((char *)buffer + sizeof(int));
		for (i = 0; i < count; i++) {
			if (maj == (int)table->board) {
				if (!strncmp(table->name, "CTC", 3)) {
					devtype = G_3B2_CTC;
					break;
				} else if (!strncmp(table->name, "TAPE", 4)) {
					devtype = G_TAPE;
					break;
				}
				/* other possible devices can go here */
			}
			table++;
		}
	}
	switch (devtype) {
		case G_3B2_CTC:	/* do special CTC initialization */
			ds_bufsize = pbufsize ? ds_bufsize : 15872;
			if (fstat(ds_fd, &orig_st_buf) < 0) {
				ds_bufsize = -1;
				break;
			}
			nflag = (O_RDWR | O_CTSPECIAL);
			(void)close(ds_fd);
			if ((ds_fd = open(device, nflag, 0666)) != -1) {
				if (ioctl(ds_fd, STREAMON) != -1) {
					(void)close(ds_fd);
					nflag = (oflag == O_WRONLY) ? O_WRONLY : O_RDONLY;
					if ((ds_fd = open(device, nflag, 0666)) == -1) {
						rpterr();
						progerr(ERR_TRANSFER);
						logerr(MSG_OPEN, device, errno);
						return -1;
					}
					ds_bufsize = 15872;
				}
			} else
				ds_bufsize = -1;
			if(oflag == O_RDONLY && ds_header && ds_totread == 0)
				/* Have already read in first block of header */
				read(ds_fd, buf, 512); 
			ds_ctcflg = 1;
			
			break;
		case G_NO_DEV:
		case G_3B2_HD:
		case G_3B2_FD:
		case G_TAPE:
		case G_SCSI_HD: /* not developed yet */
		case G_SCSI_FD:
		case G_SCSI_9T:
		case G_SCSI_Q24:
		case G_SCSI_Q120:
		case G_386_HD:
		case G_386_FD:
		case G_386_Q24:
			ds_bufsize = pbufsize ? ds_bufsize : 512;
			break;
		default:
			ds_bufsize = -1;
			errno = ENODEV;
	} /* devtype */
lab:
#endif /*u3b2*/
	if(ds_bufsize > 512) {
		if(oflag & O_WRONLY)
			fd = 1;
		else
			fd = 0;
		fd2 = fcntl(fd, F_DUPFD, fd);
		(void)close(fd);
		fcntl(ds_fd, F_DUPFD, fd);
		if(fd)
			sprintf(cmd, "%s obs=%d 2>/dev/null", DDPROC, ds_bufsize);
		else
			sprintf(cmd, "%s ibs=%d 2>/dev/null", DDPROC, ds_bufsize);
		if((ds_pp = popen(cmd, fd ? "w" : "r")) == NULL) {
			progerr(ERR_TRANSFER);
			logerr(MSG_POPEN, cmd, errno);
			return -1;
		}
		(void)close(fd);
		fcntl(fd2, F_DUPFD, fd);
		(void)close(fd2);
		ds_realfd = ds_fd;
		ds_fd = fileno(ds_pp);
	}
	return(ds_bufsize);
}

int 
ds_close(pkgendflg)
int pkgendflg;
{
	int n, ret = 0;

#ifdef u3b2
	int cnt, mode;
	char *ptr;
	if(ds_pp && ds_ctcflg) {
		ds_ctcflg = 0;
		if((mode = fcntl(ds_realfd, F_GETFL, 0)) < 0) {
			ret = -1;
		}
		else if(mode & O_WRONLY) {
		/* 
		 * pipe to dd write process, 
		 * make sure one more buffer 
		 * gets written out 
		 */
			if ((ptr = calloc(512, 1)) == NULL) {
				ret = -1;
			/* pad to bufsize */
			} else {
				cnt = ds_bufsize;
				while(cnt > 0) {
					if((n = write(ds_fd, ptr, 512)) < 0) {
						ret = -1;
						break;
					}
					cnt -= n;
				}
				(void)free(ptr);
			}
		}
	}
#endif / *u3b2 */
	if(pkgendflg) {
		if(ds_header)
			(void)free(ds_header);
		ds_header = (char *)NULL;
		ds_totread = 0;
	}

	if(ds_pp) {
		(void)pclose(ds_pp);
		ds_pp = 0;
		(void)close(ds_realfd);
		ds_realfd = -1;
		ds_fd = -1;
	} else if(ds_fd >= 0) {
		(void)close(ds_fd);
		ds_fd = -1;
	}

	if(ds_device) {
		/* rewind device */
		if((n = open(ds_device, 0)) >= 0)
			(void)close(n);
		ds_device = NULL;
	}
	return ret;
}
