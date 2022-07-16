/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libpkg:pkgtrans.c	1.15.15.1"

/*LINTLIBRARY*/
#include <stdio.h>
#include <varargs.h>
#include <limits.h>
#ifndef PRESVR4
#include <stdlib.h>
#endif
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <signal.h>
#include <pkginfo.h>
#include <pkgstrct.h>
#include <pkgtrans.h>
#include <pkgdev.h>
#include <devmgmt.h>

extern int	errno;
extern char	*pkgdir;
extern FILE	*epopen();
extern char	**gpkglist(), *devattr(), *fpkginst();
extern void	free(),
		ecleanup(),
		progerr(),
		logerr(),
		rpterr(),
		ds_order();
extern int	getvol(),
		_getvol(),
		rrmdir(),
		mkdir(),
		chdir(),
		access(),
		epclose(),
		ckvolseq(),
		isdir(),
		ds_init(),
		ds_findpkg(),
		ds_getpkg(),
		fpkginfo(),
		esystem(),
		devtype(),
		pkgmount(),
		pkgumount(),
		ds_ginit(),
		ds_readbuf(),
		open(),
		ds_close(),
		write(),
		creat();

#define CMDSIZ	1024

#define PKGINFO	"pkginfo"
#define PKGMAP	"pkgmap"
#define INSTALL	"install"
#define RELOC	"reloc"
#define ROOT	"root"

#ifndef PRESVR4
#define CPIOPROC	"/usr/bin/cpio"
#define LSPROC		"/usr/bin/ls"
#define XARGSPROC	"/usr/bin/xargs"
#define ECHOPROC	"echo"
#else
#define CPIOPROC	"/bin/cpio"
#define LSPROC		"/bin/ls"
#define XARGSPROC	"/usr/bin/xargs"
#define ECHOPROC	"echo"
#endif

#define MSG_TRANSFER	"Transferring <%s> package instance\n"
#define MSG_RENAME 	"\t... instance renamed <%s> on destination\n"
#define MSG_CORRUPT \
	"Volume is corrupt or is not part of the appropriate package."

#define ERR_TRANSFER	"unable to complete package transfer"
#define MSG_SEQUENCE	"- volume is out of sequence"
#define MSG_MEM		"- no memory"
#define MSG_CMDFAIL	"- process <%s> failed, exit code %d"
#define MSG_CMDBIG	"- command line too big - <%s>"
#define MSG_POPEN	"- popen of <%s> failed, errno=%d"
#define MSG_PCLOSE	"- pclose of <%s> failed, errno=%d"
#define MSG_BADDEV	"- invalid or unknown device <%s>"
#define MSG_GETVOL	"- unable to obtain package volume"
#define MSG_NOSIZE 	"- unable to obtain maximum part size from pkgmap"
#define MSG_CHDIR	"- unable to change directory to <%s>"
#define MSG_FSTYP	"- unable to determine filesystem type for <%s>" 
#define MSG_NOTEMP	"- unable to create or use temporary directory <%s>"
#define MSG_SAMEDEV	"- source and destination represent the same device"
#define MSG_NOTMPFIL	"- unable to create or use temporary file <%s>"
#define MSG_NOPKGMAP	"- unable to open pkgmap for <%s>"
#define MSG_BADPKGINFO	"- unable to determine contents of pkginfo file"
#define MSG_NOPKGS	"- no packages were selected from <%s>"
#define MSG_MKDIR	"- unable to make directory <%s>"
#define MSG_NOEXISTS \
	"- package instance <%s> does not exist on source device"
#define MSG_EXISTS \
	"- no permission to overwrite existing path <%s>"
#define MSG_DUPVERS \
	"- identical version of <%s> already exists on destination device"
#define MSG_TWODSTREAM \
	"- both source and destination devices cannot be a datastream"
#define MSG_NOSPACE	"- not enough space on device"
#define MSG_HDRLARGE	"- datastream header line too large"
#define MSG_OPEN	"- open of <%s> failed, errno=%d"

static struct pkgdev srcdev, dstdev;
static char	*tmpdir;
static char	*tmppath;
static char	dstinst[16];
static char 	*ids_name, *ods_name;
static int	ds_volcnt;
static int	ds_volno;
static void	(*func)();
static void	cleanup(), sigtrap();
static int	pkgxfer(), wdsheader(), ckoverwrite();
int		pkgtrans();
extern int	ds_fd; /* open file descriptor for data stream */

char	**xpkg;	/* array of transferred packages */
int	nxpkg;

static	char *allpkg[] = {
	"all",
	NULL
};

int
pkghead(device)
char	*device;
{
	static char	*tmppath;
	char	*pt;
	int	n;

	cleanup();
	if(tmppath) {
		/* remove any previous tmppath stuff */
		rrmdir(tmppath);
		free(tmppath);
		tmppath = NULL;
	}

	if(device == NULL)
		return(0);
	else if((device[0] == '/') && !isdir(device)) {
		pkgdir = device;
		return(0);
	} else if((pt = devattr(device, "pathname")) && !isdir(pt)) {
		pkgdir = pt;
		return(0);
	}

	/* check for datastream */
	if(n=pkgtrans(device, (char *)0, allpkg, PT_SILENT|PT_INFO_ONLY))
		return(n);
		/* pkgtrans has set pkgdir */
	return(0);
}

static char *hdrbuf;
static char *pinput, *nextpinput;

static char *
mgets(buf, size)
char *buf;
int size;
{
	nextpinput = strchr(pinput, '\n');
	if(nextpinput == NULL) 
		return 0;
	*nextpinput = '\0';
	if((int)strlen(pinput) > size)
		return 0;
	(void)strncpy(buf, pinput, strlen(pinput));
	buf[strlen(pinput)] = '\0';
	pinput = nextpinput + 1;
	return buf;
}


/* will return 0, 1, 3, or 99 */
int
pkgtrans(device1, device2, pkg, options)
char	*device1, *device2, **pkg;
int	options;
{
	char	*src, *dst;
	int	errflg, i, n;

	func = signal(SIGINT, sigtrap);

	/* transfer spool to appropriate device */
	if(devtype(device1, &srcdev)) {
		progerr(ERR_TRANSFER);
		logerr(MSG_BADDEV, device1);
		return(1);
	}
	srcdev.rdonly++;


	/* check for datastream */
	ids_name = NULL;
	if(srcdev.bdevice) {
		if(n = _getvol(srcdev.bdevice, NULL, NULL, "Insert %v into %p.", srcdev.norewind)) {
			cleanup();
			if(n == 3)
				return(3);
			progerr(ERR_TRANSFER);
			logerr(MSG_GETVOL);
			return(1);
		}
		if(ds_readbuf(srcdev.cdevice))
			ids_name = srcdev.cdevice;
	}

	if(srcdev.cdevice && !srcdev.bdevice) 
		ids_name = srcdev.cdevice;
	else if(srcdev.pathname) {
		if(access(ids_name, 0) == -1) {
			progerr(ERR_TRANSFER);
			logerr(MSG_GETVOL);
			return(1);
		}
		ids_name = srcdev.pathname;
	}
		
	if(!ids_name && device2 == (char *)0) {
		if(n = pkgmount(&srcdev, NULL, 1, 0, 0)) {
			cleanup();
			return(n);
		}
		else if (srcdev.mount) {
			pkgdir = srcdev.mount;
			return(0);
		}
		return(1);
	}

	if(ids_name && device2 == (char *)0) {
		tmppath = tmpnam(NULL);
		tmppath = strdup(tmppath);
		if(tmppath == NULL) {
			progerr(ERR_TRANSFER);
			logerr(MSG_MEM);
			return(1);
		}
		if(mkdir(tmppath, 0755)) {
			progerr(ERR_TRANSFER);
			logerr(MSG_MKDIR, tmppath);
			return(1);
		}
		device2 = tmppath;
	}

	if(devtype(device2, &dstdev)) {
		progerr(ERR_TRANSFER);
		logerr(MSG_BADDEV, device2);
		return(1);
	}

	if((srcdev.cdevice && dstdev.cdevice) &&
	   !strcmp(srcdev.cdevice, dstdev.cdevice)) {
		progerr(ERR_TRANSFER);
		logerr(MSG_SAMEDEV);
		return(1);
	}

	ods_name = NULL;
	if(dstdev.cdevice && !dstdev.bdevice || dstdev.pathname)
		options |= PT_ODTSTREAM;

	if(options & PT_ODTSTREAM) {
		if(!((ods_name = dstdev.cdevice) || (ods_name = dstdev.pathname))) {
			progerr(ERR_TRANSFER);
			logerr(MSG_BADDEV, device2);
			return(1);
		}
		if(ids_name) {
			progerr(ERR_TRANSFER);
			logerr(MSG_TWODSTREAM);
			return 1;
		}
	}

	if((srcdev.dirname && dstdev.dirname) &&
	!strcmp(srcdev.dirname, dstdev.dirname)) {
		progerr(ERR_TRANSFER);
		logerr(MSG_SAMEDEV);
		return(1);
	}

	if((srcdev.pathname && dstdev.pathname) &&
	!strcmp(srcdev.pathname, dstdev.pathname)) {
		progerr(ERR_TRANSFER);
		logerr(MSG_SAMEDEV);
		return(1);
	}

	if(ids_name) {
		if(srcdev.cdevice && !srcdev.bdevice && 
		(n = _getvol(srcdev.cdevice, NULL, NULL, NULL, srcdev.norewind))) {
			cleanup();
			if(n == 3)
				return(3);
			progerr(ERR_TRANSFER);
			logerr(MSG_GETVOL);
			return(1);
		}
		if(srcdev.dirname = tmpnam(NULL)) 
			tmpdir = srcdev.dirname = strdup(srcdev.dirname);
		if((srcdev.dirname == NULL) || mkdir(srcdev.dirname, 0755) || 
		   chdir(srcdev.dirname)) {
			progerr(ERR_TRANSFER);
			logerr(MSG_NOTEMP, srcdev.dirname);
			cleanup();
			return(1);
		}
		if(ds_init(ids_name, pkg, srcdev.norewind)) {
			cleanup();
			return(1);
		}
	} else if(srcdev.mount) {
		if(n = pkgmount(&srcdev, NULL, 1, 0, 0)) {
			cleanup();
			return(n);
		}
	}

	src = srcdev.dirname;
	dst = dstdev.dirname;

	if(chdir(src)) {
		progerr(ERR_TRANSFER);
		logerr(MSG_CHDIR, src);
		cleanup();
		return(1);
	}

	xpkg = pkg = gpkglist(src, pkg);
	if(!pkg) {
		progerr(ERR_TRANSFER);
		logerr(MSG_NOPKGS, src);
		cleanup();
		return(1);
	}
	for(nxpkg=0; pkg[nxpkg]; )
		nxpkg++; /* count */

	if(ids_name)
		ds_order(pkg); /* order requests */

	if(options & PT_ODTSTREAM) {
		char line[128];

		if(!dstdev.pathname && (n = _getvol(ods_name, NULL, DM_FORMAT, NULL, dstdev.norewind)) ) {
			cleanup();
			if(n == 3)
				return(3);
			progerr(ERR_TRANSFER);
			logerr(MSG_GETVOL);
			return(1);
		} 
		if(wdsheader(src, ods_name, pkg)) {
			cleanup();
			return(1);
		}
		ds_volno = 1; /* number of volumes in datastream */
		pinput = hdrbuf;
		/* skip past first line in header */
		(void)mgets(line, 128);
	}

	errflg = 0;
	
	for(i=0; pkg[i]; i++) {
		if(!(options & PT_ODTSTREAM) && dstdev.mount) {
			if(n = pkgmount(&dstdev, NULL, 0, 0, 1)) {
				cleanup();
				return(n);
			}
		}
		if(errflg = pkgxfer(pkg[i], options)) {
			pkg[i] = NULL;
			if((options & PT_ODTSTREAM) || (errflg != 2))
				break;
		} else if(strcmp(dstinst, pkg[i]))
			pkg[i] = strdup(dstinst);
	}

	if(!(options & PT_ODTSTREAM) && dst)
		pkgdir = strdup(dst);
	cleanup();
	return(errflg);
}


static int
wdsheader(src, device, pkg)
char	*src, *device, **pkg;
{
	FILE	*fp;
	char	path[PATH_MAX], cmd[CMDSIZ];
	int	i, n, nparts, maxpsize;
	int partcnt, totsize;
	int hdrsize = 512;
	char *hp;
	struct stat statbuf;

	if((hdrbuf = (char *)malloc(512)) == NULL) {
		progerr(ERR_TRANSFER);
		logerr(MSG_MEM);
		return(1);
	}	
	
	(void)ds_close(0);
	if(dstdev.pathname)
		ds_fd = creat(device, 0644);
	else
		ds_fd = open(device, 1);
	if(ds_fd < 0) {
		progerr(ERR_TRANSFER);
		logerr(MSG_OPEN, device, errno);
		return(1);
	}	
	if(ds_ginit(device) < 0) {
		progerr(ERR_TRANSFER);
		logerr(MSG_OPEN, device, errno);
		(void)ds_close(0);
		return(1);
	}	
	nparts = maxpsize = 0;
	(void) sprintf(hdrbuf, "# PaCkAgE DaTaStReAm\n");
	hp = hdrbuf + strlen(hdrbuf);
	
	totsize = 0;
	for(i=0; pkg[i]; i++)  {
		(void) sprintf(path, "%s/%s/%s", src, pkg[i], PKGINFO);
		if(stat(path, &statbuf) < 0) {
			progerr(ERR_TRANSFER);
			logerr(MSG_BADPKGINFO);
			ecleanup();
			return(1);
		}
		totsize += statbuf.st_size/512 + 1;
	}

	/*
	 * totsize contains number of blocks used by header plus
	 * extra pkginfo files
	 */
	totsize += i/4 + 1;
	if(dstdev.capacity && totsize > dstdev.capacity) {
		progerr(ERR_TRANSFER);
		logerr(MSG_NOSPACE);
		ecleanup();
		return(1);
	}

	ds_volcnt = 1;		
	for(i=0; pkg[i]; i++) {
		partcnt = 0;
		(void) sprintf(path, "%s/%s/%s", src, pkg[i], PKGMAP);
		if((fp = fopen(path, "r")) == NULL) {
			progerr(ERR_TRANSFER);
			logerr(MSG_NOPKGMAP, pkg[i]);
			sighold(SIGINT);
			sigrelse(SIGINT);
			ecleanup();
			return(1);
		}
		if(fscanf(fp, ":%d%d", &nparts, &maxpsize) != 2) {
			progerr(ERR_TRANSFER);
			logerr(MSG_NOSIZE);
			(void) fclose(fp);
			ecleanup();
			return(1);
		}
		if(dstdev.capacity && maxpsize > dstdev.capacity) {
			progerr(ERR_TRANSFER);
			logerr(MSG_NOSPACE);
			(void) fclose(fp);
			ecleanup();
			return(1);
		}
			

		(void) sprintf(hp, "%s %d %d", pkg[i], nparts, maxpsize);
		totsize += nparts * maxpsize;
		if(dstdev.capacity && dstdev.capacity < totsize) { 
			int lastpartcnt = 0;
			/* if(i != 0) {
				progerr(ERR_TRANSFER);
				logerr(MSG_NOSPACE);
				(void) fclose(fp);
				ecleanup();
				return(1);
			} */
				
			if(totsize)
				totsize -= nparts * maxpsize;
			while(partcnt < nparts) {
				while(totsize <= dstdev.capacity && partcnt <= nparts) {
					totsize +=  maxpsize;
					partcnt++;
				}
				/* partcnt == 0 means skip to next volume */
				if(partcnt)
					partcnt--;
				(void) sprintf(hdrbuf + strlen(hdrbuf), " %d", partcnt - lastpartcnt);
				ds_volcnt++;
				totsize = 0;
				lastpartcnt = partcnt;
			}
			/* first parts/volume number does not count */
			ds_volcnt--;
		}
		(void) sprintf(hdrbuf + strlen(hdrbuf), "\n");

		if(strlen(hp) > (size_t)128) {
			progerr(ERR_TRANSFER);
			logerr(MSG_HDRLARGE);
			(void) fclose(fp);
			ecleanup();
			return(1);
		}

		hp = hdrbuf + strlen(hdrbuf);
		if(strlen(hdrbuf) + 1 > (size_t)hdrsize) {
			
			if((hdrbuf = (char *)realloc(hdrbuf,  hdrsize + 512)) == NULL) {
				progerr(ERR_TRANSFER);
				logerr(MSG_MEM);
				(void) fclose(fp);
				ecleanup();
				return(1);
			}
			(void)memset(hdrbuf + hdrsize, '\0', 512);
			hdrsize += 512;
		}	
		(void) fclose(fp);
	}
	sighold(SIGINT);
	sigrelse(SIGINT);

	if(strlen(hdrbuf) + 17 > (size_t)hdrsize) {
			
		if((hdrbuf = (char *)realloc(hdrbuf, hdrsize + 512)) == NULL) {
			progerr(ERR_TRANSFER);
			logerr(MSG_MEM);
			(void) fclose(fp);
			ecleanup();
			return(1);
		}
		(void)memset(hdrbuf + hdrsize, '\0', 512);
		hdrsize += 512;
	}	
	(void) sprintf(hdrbuf + strlen(hdrbuf), "# end of header\n");
	write(ds_fd, hdrbuf, hdrsize);

	/*
	 * write the first cpio() archive to the datastream
	 * which should contain the pkginfo & pkgmap files
	 * for all packages
	 */
	sprintf(cmd, "%s ", ECHOPROC);
	for(i=0; pkg[i]; i++) {
		if(strlen(cmd) + 70 > (size_t)CMDSIZ) {
			progerr(ERR_TRANSFER);
			logerr(MSG_CMDBIG, cmd);
			cleanup();
			return 1;
		}		
		(void) sprintf(cmd + strlen(cmd), "%s/%s ", pkg[i], PKGINFO);
		(void) sprintf(cmd + strlen(cmd), "%s/%s ", pkg[i], PKGMAP);
	}
	if(strlen(cmd) + 30 > (size_t)CMDSIZ) {
		progerr(ERR_TRANSFER);
		logerr(MSG_CMDBIG, cmd);
		cleanup();
		return 1;
	}		
#ifndef PRESVR4
	(void) sprintf(cmd + strlen(cmd), " | %s %s | %s -ocD -C 512", XARGSPROC, LSPROC, CPIOPROC);
#else
	(void) sprintf(cmd + strlen(cmd), " | %s %s | %s -oc -C 512", XARGSPROC, LSPROC, CPIOPROC);
#endif
	if(n = esystem(cmd, -1, ds_fd)) {
		rpterr();
		progerr(ERR_TRANSFER);
		logerr(MSG_CMDFAIL, cmd, n);
		cleanup();
		return(1);
	}
	return(0);
}

static int
ckoverwrite(dir, inst, options)
char	*dir;
char	*inst;
int	options;
{
	char	path[PATH_MAX];

	(void) sprintf(path, "%s/%s", dir, inst);
	if(access(path, 0) == 0) {
		if(options & PT_OVERWRITE)
			return(rrmdir(path));
		progerr(ERR_TRANSFER);
		logerr(MSG_EXISTS, path);
		return(1);
	}
	return(0);
}

static int
pkgxfer(srcinst, options)
char	*srcinst;
int	options;
{
	struct pkginfo info;
	FILE	*fp, *pp;
	char	*pt, *src, *dst;
	char	dstdir[PATH_MAX],
		temp[PATH_MAX], 
		srcdir[PATH_MAX],
		cmd[CMDSIZ],
		pkgname[16];
	int	i, n, part, nparts, maxpartsize, curpartcnt;
	char	volnos[128], tmpvol[128];

	info.pkginst = NULL; /* required initialization */

	/*
	 * when this routine is entered, the first part of
	 * the package to transfer is already available in
	 * the directory indicated by 'src' --- unless the
	 * source device is a datstream, in which case only
	 * the pkginfo and pkgmap files are available in 'src'
	 */
	src = srcdev.dirname;
	dst = dstdev.dirname;

	if(!(options & PT_SILENT))
		(void) fprintf(stderr, MSG_TRANSFER, srcinst);
	(void) strcpy(dstinst, srcinst);

	if(!(options & PT_ODTSTREAM)) {
		/* destination is a (possibly mounted) directory */
		(void) sprintf(dstdir, "%s/%s", dst, dstinst);

		/*
		 * need to check destination directory to assure
		 * that we will not be duplicating a package which
		 * already resides there (though we are allowed to
		 * overwrite the same version)
		 */
		pkgdir = src;
		if(fpkginfo(&info, srcinst)) {
			progerr(ERR_TRANSFER);
			logerr(MSG_NOEXISTS, srcinst);
			(void) fpkginfo(&info, NULL);
			return(1);
		}
		pkgdir = dst;

		(void) strcpy(temp, srcinst);
		if(pt = strchr(temp, '.'))
			*pt = '\0';
		(void) strcat(temp, ".*");

		if(pt = fpkginst(temp, info.arch, info.version)) {
			/* the same instance already exists, although
			 * its pkgid might be different
			 */
			if(options & PT_OVERWRITE) {
				(void) strcpy(dstinst, pt);
				(void) sprintf(dstdir, "%s/%s", dst, dstinst);
			} else {
				progerr(ERR_TRANSFER);
				logerr(MSG_DUPVERS, srcinst);
				(void) fpkginfo(&info, NULL);
				(void) fpkginst(NULL);
				return(2);
			}
		} else if(options & PT_RENAME) {
			/* 
			 * find next available instance by appending numbers
			 * to the package abbreviation until the instance
			 * does not exist in the destination directory
			 */
			if(pt = strchr(temp, '.'))
				*pt = '\0';
			for(i=2; (access(dstdir, 0) == 0); i++) {
				(void) sprintf(dstinst, "%s.%d", temp, i);
				(void) sprintf(dstdir, "%s/%s", dst, dstinst);
			}
		} else if(options & PT_OVERWRITE) {
			/* we're allowed to overwrite, but there seems
			 * to be no valid package to overwrite, and we are
			 * not allowed to rename the destination, so act
			 * as if we weren't given permission to overwrite
			 * --- this keeps us from removing a destination
			 * instance which is named the same as the source
			 * instance, but really reflects a different pkg!
			 */
			options &= (~PT_OVERWRITE);
		}
		(void) fpkginfo(&info, NULL);
		(void) fpkginst(NULL);

		if(ckoverwrite(dst, dstinst, options))
			return(2);

		if(isdir(dstdir) && mkdir(dstdir, 0755)) {
			progerr(ERR_TRANSFER);
			logerr(MSG_MKDIR, dstdir);
			return(1);
		}
	}

	if(!(options & PT_SILENT) && strcmp(dstinst, srcinst))
		(void) fprintf(stderr, MSG_RENAME, dstinst);

	(void) sprintf(srcdir, "%s/%s", src, srcinst);
	if(chdir(srcdir)) {
		progerr(ERR_TRANSFER);
		logerr(MSG_CHDIR, srcdir);
		return(1);
	}

	if(ids_name) {	/* unpack the datatstream into a directory */
		/*
		 * transfer pkginfo & pkgmap first
		 */
		(void) sprintf(cmd, "%s -pudm %s", CPIOPROC, dstdir);
		if((pp = epopen(cmd, "w")) == NULL) {
			rpterr();
			progerr(ERR_TRANSFER);
			logerr(MSG_POPEN, cmd, errno);
			return(1);
		}
		(void)fprintf(pp, "%s\n%s\n", PKGINFO, PKGMAP);
		sighold(SIGINT);
		if(epclose(pp)) {
			sigrelse(SIGINT);
			rpterr();
			progerr(ERR_TRANSFER);
			logerr(MSG_PCLOSE, cmd, errno);
			return(1);
		}
		sigrelse(SIGINT);

		if(options & PT_INFO_ONLY)
			return(0); /* don't transfer objects */

		if(chdir(dstdir)) {
			progerr(ERR_TRANSFER);
			logerr(MSG_CHDIR, dstdir);
			return(1);
		}

		/*
		 * for each part of the package, use cpio() to
		 * unpack the archive into the destination directory
		 */
		nparts = ds_findpkg(srcdev.cdevice, srcinst);
		if(nparts < 0) {
			progerr(ERR_TRANSFER);
			return(1);
		}
		for(part=1; part <= nparts;) {
			if(ds_getpkg(srcdev.cdevice, part, dstdir)) {
				progerr(ERR_TRANSFER);
				return(1);
			}
			part++;
			if(dstdev.mount) { 
				(void) chdir("/");
				if(pkgumount(&dstdev))
					return(1);
				if(part <= nparts) {
					if(n = pkgmount(&dstdev, NULL, part+1, 
					  nparts, 1))
						return(n);
					if(ckoverwrite(dst, dstinst, options))
						return(1);
					if(isdir(dstdir) && mkdir(dstdir, 0755)) {
						progerr(ERR_TRANSFER);
						logerr(MSG_MKDIR, dstdir);
						return(1);
					}
					/* 
					 * since volume is removable, each part
					 * must contain a duplicate of the 
					 * pkginfo file to properly identify the
					 * volume
					 */
					if(chdir(srcdir)) {
						progerr(ERR_TRANSFER);
						logerr(MSG_CHDIR, srcdir);
						return(1);
					}
					if((pp = epopen(cmd, "w")) == NULL) {
						rpterr();
						progerr(ERR_TRANSFER);
						logerr(MSG_POPEN, cmd, errno);
						return(1);
					}
					(void) fprintf(pp, "pkginfo");
					if(epclose(pp)) {
						rpterr();
						progerr(ERR_TRANSFER);
						logerr(MSG_PCLOSE, cmd, errno);
						return(1);
					}
					if(chdir(dstdir)) {
						progerr(ERR_TRANSFER);
						logerr(MSG_CHDIR, dstdir);
						return(1);
					}
				}
			}
		}
		return(0);
	}

	/* 
	 * read nparts and maxpartsiz from pkgmap
	 */
	if((fp = fopen(PKGMAP, "r")) == NULL) {
		progerr(ERR_TRANSFER);
		logerr(MSG_NOPKGMAP, srcinst);
		return(1);
	}
	nparts = 1;
	if(fscanf(fp, ":%d%d", &nparts, &maxpartsize) != 2) {
		progerr(ERR_TRANSFER);
		logerr(MSG_NOSIZE);
		(void) fclose(fp);
		return(1);
	}
	(void) fclose(fp);

	if(srcdev.mount) {
		if(ckvolseq(srcdir, 1, nparts)) {
			progerr(ERR_TRANSFER);
			logerr(MSG_SEQUENCE);
			return(1);
		}
	}

	/* write each part of this package */
	if(options & PT_ODTSTREAM) {
		char line[128];
		(void)mgets(line, 128);
		curpartcnt = -1;
		if(sscanf(line, "%s %d %d %[ 0-9]", &pkgname, &nparts, &maxpartsize, volnos) == 4) {
			sscanf(volnos, "%d %[ 0-9]", &curpartcnt, tmpvol);
			strcpy(volnos, tmpvol);
		}
	}
		
	for(part=1; part <= nparts; ) {
		if(curpartcnt == 0 && (options & PT_ODTSTREAM)) {
			char prompt[128];
			int index;
			ds_volno++;
			(void)ds_close(0);
			(void)sprintf(prompt, "Insert %%v %d of %d into %%p", ds_volno, ds_volcnt);
			if(n = getvol(ods_name, NULL, DM_FORMAT, prompt)) 
				return n;
			if((ds_fd = open(dstdev.cdevice, 1)) < 0) {
				progerr(ERR_TRANSFER);
				logerr(MSG_OPEN, dstdev.cdevice, errno);
				return 1;
			}
			if(ds_ginit(dstdev.cdevice) < 0) {
				progerr(ERR_TRANSFER);
				logerr(MSG_OPEN, dstdev.cdevice, errno);
				(void)ds_close(0);
				return(1);
			}	
		
			(void)sscanf(volnos, "%d %[ 0-9]", &index, tmpvol);
			(void)strcpy(volnos, tmpvol);
			curpartcnt += index;
		}
			

		if(options & PT_INFO_ONLY)
			nparts = 0;

		if(part == 1) {
			(void) sprintf(cmd, "find %s %s", PKGINFO, PKGMAP);
			if(nparts && (isdir(INSTALL) == 0)) {
				(void) strcat(cmd, " ");
				(void) strcat(cmd, INSTALL);
			}
		} else
			(void) sprintf(cmd, "find %s", PKGINFO);

		if(nparts > 1) {
			(void) sprintf(temp, "%s.%d", RELOC, part);
			if(isdir(temp) == 0) {
				(void) strcat(cmd, " ");
				(void) strcat(cmd, temp);
			}
			(void) sprintf(temp, "%s.%d", ROOT, part);
			if(isdir(temp) == 0) {
				(void) strcat(cmd, " ");
				(void) strcat(cmd, temp);
			}
		} else if(nparts) {
			if(isdir(RELOC) == 0) {
				(void) strcat(cmd, " ");
				(void) strcat(cmd, RELOC);
			}
			if(isdir(ROOT) == 0) {
				(void) strcat(cmd, " ");
				(void) strcat(cmd, ROOT);
			}
		}
		if(options & PT_ODTSTREAM) {
#ifndef PRESVR4
			(void) sprintf(cmd+strlen(cmd), " -print | %s -ocD -C 512",
#else
			(void) sprintf(cmd+strlen(cmd), " -print | %s -oc -C 512",
#endif
				CPIOPROC);
		} else {
			(void) sprintf(cmd+strlen(cmd), " -print | %s -pdum %s",
				CPIOPROC, dstdir);
		}

		if(n = esystem(cmd, -1, (options & PT_ODTSTREAM) ? ds_fd : -1)) {
			rpterr();
			progerr(ERR_TRANSFER);
			logerr(MSG_CMDFAIL, cmd, n);
			return(1);
		}

		part++;
		if(srcdev.mount && (nparts > 1)) {
			/* unmount current source volume */
			(void) chdir("/");
			if(pkgumount(&srcdev))
				return(1);
			/* loop until volume is mounted successfully */
			while(part <= nparts) {
				/* read only */
				if(n = pkgmount(&srcdev, NULL, part, nparts, 1))
					return(n);
				if(chdir(srcdir)) {
					progerr(ERR_TRANSFER);
					logerr(MSG_CORRUPT, srcdir);
					(void) chdir("/");
					pkgumount(&srcdev);
					continue;
				}
				if(ckvolseq(srcdir, part, nparts)) {
					(void) chdir("/");
					pkgumount(&srcdev);
					continue;
				}
				break;
			}
		}
		if(!(options & PT_ODTSTREAM) && dstdev.mount) {
			/* unmount current volume */
			if(pkgumount(&dstdev))
				return(1);
			/* loop until next volume is mounted successfully */
			while(part <= nparts) {
				/* writable */
				if(n = pkgmount(&dstdev, NULL, part, nparts, 1))
					return(n);
				if(ckoverwrite(dst, dstinst, options))
					continue;
				if(isdir(dstdir) && mkdir(dstdir, 0755)) {
					progerr(ERR_TRANSFER);
					logerr(MSG_MKDIR, dstdir);
					continue;
				}
				break;
			}
		}

		if((options & PT_ODTSTREAM) && part <= nparts) {
			if(curpartcnt >= 0 && part > curpartcnt) {
				char prompt[128];
				int index;
				ds_volno++;
				if(ds_close(0))
					return 1;
				(void)sprintf(prompt, "Insert %%v %d of %d into %%p", ds_volno, ds_volcnt);
				if(n = getvol(ods_name, NULL, DM_FORMAT, prompt)) 
					return n;
				if((ds_fd = open(dstdev.cdevice, 1)) < 0) {
					progerr(ERR_TRANSFER);
					logerr(MSG_OPEN, dstdev.cdevice, errno);
					return 1;
				}
				if(ds_ginit(dstdev.cdevice) < 0) {
					progerr(ERR_TRANSFER);
					logerr(MSG_OPEN, dstdev.cdevice, errno);
					(void)ds_close(0);
					return 1;
				}	
			
				(void)sscanf(volnos, "%d %[ 0-9]", &index, tmpvol);
				(void)strcpy(volnos, tmpvol);
				curpartcnt += index;
			}
		}

	}
	return(0);
}

static void
sigtrap(signo)
int	signo;
{

	cleanup();

 	if(tmppath) { 
		rrmdir(tmppath);
		free(tmppath);
		tmppath = NULL;
	}
	if(func && (func != SIG_DFL) && (func != SIG_IGN))
		/* must have been an interrupt handler */
		(*func)(signo);
}

static void
cleanup()
{
	chdir("/");
 	if(tmpdir) { 
		rrmdir(tmpdir);
		free(tmpdir);
		tmpdir = NULL;
	}
	if(srcdev.mount && !ids_name)
		pkgumount(&srcdev);
	if(dstdev.mount && !ods_name)
		pkgumount(&dstdev);
	(void)ds_close(1);
}
