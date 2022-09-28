/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)idcmd:idmknod.c	1.3.2.1"

/* This program reads /etc/conf/node.d/* and inserts nodes in /dev.
 * The format of each file is:
 *	Field 1		- name of device entry in mdevice.
 *			  Must be from 1 to 8 chars.
 *			  The first char must be a letter.
 *			  The others can be letters, digits, underscores.
 *	Field 2		- name of node to be inserted.
 *			  Must be from 1 to 14 chars.
 *			  The first char must be a letter.
 *			  The others can be letters, digits, underscores.
 *	Field 3		- b or c (block or character).
 *	Field 4		- minor device number.
 *			  Must be between 0 and 255 for old-style devices and
 *			  between 0 and MAXMINOR (tunable) for new-style
 *			  devices.
 * The command line options are:
 *	-o directory	- the installation directory (/dev).
 *	-i directory	- the directory containing mdevice (cf.d).
 *	-e directory	- the directory containing the node files (node.d).
 *	-s 		- suppress removing nodes from /dev.
 *	-#		- print diagnostics.
 * Nodes that are not listed in mdevice as required (r) are removed
 * from /dev before the new nodes are inserted.
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <sys/mkdev.h>
#include "inst.h"

#define	TRUE	1
#define	FALSE	0

/* check if char 'c' is in string 'str' */
#define INSTRING(str, c)	(strchr(str, c) == NULL ? 0 : 1)

/* modes */
#define	CHAR	0020000
#define	DIRECT	0040000
#define BLOCK	0060000
#define DTYPE	0170000
#define PROT	0666

/* directories */
#define	ENVIRON		0
#define OUTPUT		1
#define FULL_PATH	2

/* error messages */
#define USAGE	"Usage: idmknod [-i dir] [-o dir] [-e dir] [-s]\n"
#define	STAT	"can not stat %s/%s"
#define	CHDIR	"can not chdir to %s"
#define SPECIAL	"%s: must contain 'c' or 'b'"
#define MKNOD	"%s: can not make node. Errno = %d"
#define MKDIR	"%s: can not make subdirectory. Errno = %d"
#define CHOWN	"%s: can not chown. Errno = %d"
#define CHMOD1	"%s: can not chmod. Errno = %d"
#define CHMOD	"%s: can not chmod 666. Errno = %d"
#define	OPEN	"%s: can not open for mode '%s'"
#define UNKNOWN "%s: unknown device"
#define GETINST	"can not locate or open 'mdevice'"
#define ERASE	"can not erase node '%s'"
#define	EXPECT	"Not expecting character"
#define LENGTH	"Field %d exceeds %d character(s)"
#define INCOMP	"Incomplete line\nline:\t%s"
#define NOMATCH "%s: type of node does not match the driver type"
#define GETMAJOR "Error encountered getting major number(s) for %s mdevice entry"
#define WTYPE "Wrong device type specification <%s>; must be single letter"
#define WSYNTAX "Wrong syntax for third field <%s>; single letter or letter:num"
#define IVOFFSET "Invalid offset in third field <%s>; it cannot be a negative value"
#define OUTRANGE "%s major offset produces major number out of range"
#define BADMINOR "%s: node file for driver <%s> contains invalid minor number"

/* analyzer return codes */
#define	BLANK	0
#define GOOD	1
#define BAD	2

/* buffer size for reading in device names */
#define DEVSIZE	15

/* directories */
char current[80];		/* current directory */
char envirmnt[80];		/* path name of node.d directory */
char output[80];		/* path name of '/dev' directory */

/* flags */
int iflag;			/* 'mdevice' directory specified */
int oflag;			/* 'dev' directory specified */
int eflag;			/* 'node.d' directory specified */
int sflag;			/* suppress flag specified */
int debug;			/* debug flag */
int errors;			/* number of errors */

FILE *sd;
FILE *open1();
DIR  *open2();
char errbuf[100];		/* hold error messages */
extern char *optarg;		/* used by getopt */
extern char pathinst[];		/* dir containing mdevice - used by getinst */

main(argc, argv)
int argc;
char *argv[];
{
	char buf[100];
	int c;

	while ((c = getopt(argc, argv, "i:o:e:s#?")) != EOF)
		switch (c) {
		case 'e':	/* contains node files */
			strcpy(envirmnt, optarg);
			eflag++;
			break;
		case 'o':	/* /dev directory */
			strcpy(output, optarg);
			oflag++;
			break;
		case 'i':	/* contains mdevice */
			strcpy(pathinst, optarg);
			iflag++;
			break;
		case 's':
			sflag++;
			break;
		case '#':
			debug++;
			break;
		case '?':
			sprintf(errbuf, USAGE);
			error(1);
		}

	sprintf (buf, "%s/cf.d/sdevice", ROOT);
	if ((sd = fopen(buf, "r")) == NULL) {
		perror(buf);
		exit(1);
	}

	/* get current directory */
	getcwd(current, 80);

	/* get full path name */
	getpath(oflag, output, "/dev");
	sprintf(buf, "%s/node.d", ROOT);
	getpath(eflag, envirmnt, buf);
	getpath(iflag, pathinst, "");

	if (debug) {
		fprintf(stderr, "debug:\tpathinst=%s\n\toutput=%s\n\tenvirmnt=%s\n",
			pathinst, output, envirmnt);
		fprintf(stderr, "\tcurrent=%s\n", current);
	}

	if (!sflag)
		deverase();		/* erase nodes */
	getfile();			/* get next node file and install nodes */
	exit(errors);
}

/* dev erase nodes that are not required or automatically installed */

deverase()
{
	erase (output);
}

/* erase is recursive. It looks at /dev, and subdirecories therein. */

erase(rdir)
char *rdir;
{
	char buf[80];
	char cmd[80];
	struct stat sbuf;
	static rcnt = 0;

	struct dirent *direntp;
	DIR *dp;

	rcnt++;
	if (debug)
		fprintf(stderr, "debug: chdir to %s, depth= %d\n", rdir,rcnt);
	if(chdir(rdir) < 0){
		sprintf(errbuf, CHDIR, rdir);
		error(1);
	}
	dp = open2(".");
	while (direntp = readdir (dp)) {
		if (direntp->d_ino == 0 || direntp->d_name[0] == '.')
			continue;
		if (debug)
			fprintf(stderr, "debug: node='%s'\n", direntp->d_name);
		/* stat node */
		sprintf(buf, "%s/%s", ".", direntp->d_name);
		if (stat(buf, &sbuf) == -1) {
			sprintf(errbuf, STAT, rdir, direntp->d_name);
			error(0);
		}
		/* check if directory (subdirectory of /dev) */
		if ( (sbuf.st_mode & DTYPE) == DIRECT ){
			if (debug) {
				fprintf(stderr,"debug: file mode 0x%x\n",sbuf.st_mode);
				fprintf(stderr,"debug: subdir %s\n", direntp->d_name);
			}
			erase(direntp->d_name);	/* RECURSION */
			continue;
		}

		/* check if character or block special node */
		if ( ((sbuf.st_mode & DTYPE) != BLOCK) &&
			((sbuf.st_mode & DTYPE) != CHAR) )
				continue;

		/* if mdevice lists as required or automatically installed
		 * device, do not erase node */
		if(debug) {
			fprintf(stderr,"sbuf.st_rdev is 0x%x\n",sbuf.st_rdev);
			fprintf(stderr,"major(sbuf.st_rdev) is 0x%x\n",major(sbuf.st_rdev));
		}
		if (required(major(sbuf.st_rdev), (sbuf.st_mode & BLOCK) == BLOCK))
			continue;

		/* erase node */
		if (debug)
			fprintf(stderr,"debug: unlinking %s\n",buf);
		if (unlink(buf) == -1) {
			if (errno != ENOSYS) {
				sprintf(errbuf, ERASE, direntp->d_name);
				error(0);
			}
		}
	}
	closedir(dp);
	if(chdir("..") < 0){
		sprintf(errbuf, CHDIR, "..");
		error(1);
	}
	/* Try to rmdir subdirectories; don't report failure */
	if (rcnt-- > 1){
		sprintf(cmd, "rmdir %s", rdir);
		if (debug)
			fprintf(stderr,"debug: trying to %s\n",cmd);
		strcat(cmd, " 2>/dev/null");
		system (cmd);
	}
}



/* get the next file from environment directory (/etc/conf/node.d) */

getfile()
{
	struct dirent *direntp;
	DIR *dp;

	dp = open2(envirmnt);
	while (direntp = readdir (dp)) {
		if (debug)
			fprintf(stderr, "debug: file='%s'\n", direntp->d_name);
		if (direntp->d_ino == 0 || direntp->d_name[0] == '.')
			continue;

		if (chk_required(direntp->d_name)) 
			install(direntp->d_name);
	}
	closedir(dp);
}

int gettype_maj();

/* install new nodes */

install(file)
char *file;
{
	char buf[100];			/* full path name of node */
	char dev[DEVSIZE];		/* device name */
	char node[61];			/* node name */
	char type;			/* c for char, b for block */
	char type_array[20];		/* third field read as string */
	int minor;			/* minor number of node */
	int major;			/* major number of node */

	int ownerid, groupid, perm;
	char buf1[255];	
	int permflg = 0;

	int offset;			/* offset into multiple majors range */
	int mode;			/* mode for mknod system call */
	int l;				/* temp	*/
	int slash;			/* path has a subdirectory */
	FILE *np;			/* pointer to node file */
	char clndev[DEVSIZE];		/* clone device name */
	int i;				/* ret val from sscanf and loop index */
	int retval;			/* return value from gettype_maj */

	np = open1(file, "r", ENVIRON);
	while (fgets(buf, 99, np) != NULL) {


		i=sscanf(buf, "%s %s %s %s %d %d %o",
			buf1, buf1, buf1, buf1, &ownerid, &groupid, &perm);

		if (i == 7) permflg = 1;	
		else permflg = 0;


		i=sscanf(buf, "%14s %60s %19s %d", dev, node, type_array, &minor);
		if (i<4){
			sscanf(buf,"%14s %60s %19s %14s", dev, node, type_array, clndev);
			if (debug)
				fprintf(stderr, "debug: clone case: %s\n", clndev);
			if ((retval = gettype_maj(type_array, &type, &offset)) == 0)
				continue;
			/* special case for streams: clone device needs major
				dev number for its monor number */
			if ((minor = getmajor(clndev, type == 'b', offset)) == -1)
				continue;
		}

		else
			if ((retval = gettype_maj(type_array, &type, &offset)) == 0)
				continue;
		if (debug)
			fprintf(stderr, "debug: dev='%s' node='%s' type='%c' minor=%d,offset=%d\n",
				dev, node, type, minor, offset);

		if (type != 'c' && type != 'b') {
			sprintf(errbuf, SPECIAL, node);
			error(0);
			continue;
		}
                /* check that the type specified matches mdevice */
                if (checktype(dev, type ) <= 0) {
                        sprintf(errbuf, NOMATCH, node);
                        error(0);
                        continue;
                }

		/* find the major number of the device */
		if ((major = getmajor(dev, type == 'b', offset)) == -1)
			continue;

		mode = (type == 'b' ? BLOCK : CHAR) | PROT;

		slash=0;
		for (i=0; node[i] != '\0'; i++)
			if (node[i] == '/')
				slash++;
		if (slash)
			mkdirs(node);
		sprintf(buf, "%s/%s", output, node);
		if (debug)
			fprintf(stderr, "debug: mknod %s %c 0x%x\n",
				node, type, makedev(major,minor));
		if (mknod(buf, mode, makedev(major, minor)) == -1)
		        if (errno == EINVAL) {
		        	sprintf(errbuf, BADMINOR, node, dev);
				error(0);
				continue;
			} else {
				sprintf(errbuf, MKNOD, node, errno);
				error(0);
			}

		if (permflg) {
			if (chown(buf, ownerid, groupid) == -1) {
				sprintf(errbuf, CHOWN, node, errno);
				error(0);
			}
			if (chmod(buf, perm) == -1) {
				sprintf(errbuf, CHMOD1, node, errno);
				error(0);
			}
		} else if (chmod(buf, 0666) == -1) {
			sprintf(errbuf, CHMOD, node, errno);
			error(0);
		}
	}
	fclose(np);
}


/* open a directory */

DIR *
open2(directory)
char *directory;
{
	DIR *dp;

	if (debug)
		fprintf(stderr, "debug: open directory '%s' for mode 'r'\n",
			directory);

	if ((dp = opendir(directory)) == NULL) {
		sprintf(errbuf, OPEN, directory, "r");
		error(1);
	}
	return(dp);
}

/* open a file */

FILE *
open1(file, mode, dir)
char *file, *mode;
int dir;
{
	FILE *fp;
	char *p;
	char path[80];

	switch (dir) {
	case ENVIRON:
		sprintf(path, "%s/%s", envirmnt, file);
		p = path;
		break;
	case OUTPUT:
		sprintf(path, "%s/%s", output, file);
		p = path;
		break;
	case FULL_PATH:
		p = file;
		break;
	}

	if (debug)
		fprintf(stderr, "debug: open '%s' for mode '%s'\n",
			p, mode);

	if ((fp = fopen(p, mode)) == NULL) {
		sprintf(errbuf, OPEN, file, mode);
		error(1);
	}
	return(fp);
}



error(xit)
int xit;
{
	fprintf(stderr, "idmknod: %s\n", errbuf);
	errors++;
	if (xit)
		exit(errors);
}

/* Check that the device type is the same as listed in the node file. */

checktype(dev, type)
char *dev;
int type;
{
	struct mdev mdev;

	switch (getinst(BDEV, dev, &mdev)) {
	case -1:	/* can not open file */
		sprintf(errbuf, GETINST);
		error(1);

	case -2:	/* error in mdevice entry */
		sprintf(errbuf, GETMAJOR, mdev.device);
		error(1);

	case 0:		/* could not find device */
		sprintf(errbuf, UNKNOWN, dev);
		error(0);
		return(-1);

	default:	/* found device */
	/* check that type, either b or c, is also specified in mdev.type */
		return(INSTRING(mdev.type, type)); 
	}
}

/* get the major number for a device from mdevice */

getmajor(dev, type, offset)
char *dev;
int type, offset;
{
	struct mdev mdev;

	switch (getinst(BDEV, dev, &mdev)) {
	case -1:	/* can not open file */
		sprintf(errbuf, GETINST);
		error(1);

	case -2:	/* error getting majors in mdevice entry */
		sprintf(errbuf, GETMAJOR, mdev.device);
		error(1);

	case 0:		/* could not find device */
		sprintf(errbuf, UNKNOWN, dev);
		error(0);
		return(-1);

	default:	/* found device */
		if(INSTRING(mdev.type, 'M')) {
			if (type && ((mdev.blk_start + offset) > mdev.blk_end ||
			   mdev.blk_start + offset < mdev.blk_start)) {
				sprintf(errbuf, OUTRANGE, "block");
				error(0);
				return(-1);
			}
			if (!type && ((mdev.char_start + offset) > mdev.char_end ||
			   mdev.char_start + offset < mdev.char_start)) {
				sprintf(errbuf, OUTRANGE, "char");
				error(0);
				return(-1);
			}

			return(type ? (mdev.blk_start + offset) :
				(mdev.char_start + offset));
		}
		else
			/* for this case, offset is always zero */
			return(type ? mdev.blk : mdev.chr);
	}
}



/* check if required or automatically installed device */

required(maj, blk)
int maj, blk;
{
	register i;
	struct mdev mdev;
	short howmany;

	if (getinst(BDEV, RESET, 0) == -1) {
		sprintf(errbuf, GETINST);
		error(1);
	}
	while (getinst(BDEV, NEXT, &mdev) != 0) {
		if (INSTRING(mdev.type, 'M')) {
			if (INSTRING(mdev.type, 'b') && blk) {
				howmany = mdev.blk_end - mdev.blk_start + 1;
				for (i = 0; i < howmany; i++) {
					if ((mdev.blk_start + i) == maj)
						if (strpbrk(mdev.type, "ar") != NULL)
							return(1);
				}
			}

			if (INSTRING(mdev.type, 'c') && !blk) {
				howmany = mdev.char_end - mdev.char_start + 1;
				for (i = 0; i < howmany; i++) {
					if ((mdev.char_start + i) == maj)
						if (strpbrk(mdev.type, "ar") != NULL)
							return(1);
				}
			}
		}

		else {
			if (((mdev.blk == maj) && INSTRING(mdev.type, 'b') && blk)
				|| ((mdev.chr == maj) && INSTRING(mdev.type, 'c') && !blk)) {
				if (strpbrk(mdev.type, "ar") != NULL)
					return(1);
				return(0);
			}
		}
	}
	return(0);
}

/* parse the third field of the node file */

int gettype_maj(mstring, type, offset)
char *mstring;
char *type;
int *offset;
{
	register char *p;
	char savestring[20];
	int colon = 0;
	int letter_cnt = 0;

	strcpy(savestring, mstring);
	for (p = mstring; *p != 0; p++) {
		if(isalpha(*p) && *p != ':')
			letter_cnt++;
		if (letter_cnt > 1) {
			sprintf(errbuf, WTYPE, savestring);
			error(0);
			return(0);
		}

		if (!isalpha(*p) && !isdigit(*p) && (*p != ':')) {
			sprintf(errbuf, WSYNTAX, savestring);
			error(0);
			return(0);
		}

		if (*p == ':') {
			*p++ = 0;
			colon++;
			break;
		}
	}

	if (!isalpha(*mstring) || (colon && !isdigit(*p))) {
		if (colon && (*p == '-')) {
			sprintf(errbuf, IVOFFSET, savestring);
			error(0);
			return(0);
		}
		else {
			sprintf(errbuf, WSYNTAX, savestring);
			error(0);
			return(0);
		}
	}

	*type = *mstring;
	*offset = atoi(p);

	if (!colon)
		*offset = 0;

	return(1);
}


/* construct full path name */

getpath(flag, buf, def)
int flag;
char *buf, *def;
{
	switch (flag) {
	case 0:
		strcpy(buf, def);
		break;
	case 1:
		if (chdir(buf) != 0) {
			sprintf(errbuf, CHDIR, buf);
			error(1);
		}
		getcwd(buf, 80);
		chdir(current);
		break;
	}
}
mkdirs(dpath)
char *dpath;
{

	char tpath[61], fpath[80];
	int i;

	for (i=0; dpath[i] != '\0'; i++){
		if (dpath[i] != '/')
			tpath[i]=dpath[i];
		else{
			if (i<1)
				sprintf(errbuf, "Error making subdirectory");
			tpath[i] = '\0';
			sprintf(fpath, "%s/%s", output, tpath);
			if (debug)
				fprintf(stderr,"making subdirectory %s\n",fpath);
			if (mkdir(fpath, 0755) != 0)
				if (errno != EEXIST){
					sprintf(errbuf, MKDIR, tpath, errno);
					error(1);
				}
			tpath[i]=dpath[i];
		}
	}
}



chk_required(file) 
char *file;
{
	char	buf[255], junk[255];
	char 	buftmp[256];
	char	ch;
	{

		rewind(sd);	

		while (fgets(buftmp, 255, sd) != NULL) {
			sscanf(buftmp, "%s %c %s\n", buf, &ch, junk);
			if (! strcmp(buf, file)) {
				if (ch == 'N') return (FALSE);
				else return (TRUE);
			}
		}
		return (TRUE);
	}
}
