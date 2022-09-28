/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)fsinfo:fsinfo.c	1.2.3.1"

/*
 * 	fsinfo.c
 *
 * 	Print various filesystem information. Avoids stdio to help in
 * 	the first-3B2-restore-floppy space crunch.
 */

#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>
/*
#include <sys/vnode.h>
#include <sys/fs/s5inode.h>
*/
#include <sys/fs/s5ino.h>
#include <sys/fs/s5param.h>
#include <sys/fs/s5filsys.h>
#include <sys/fs/s5dir.h>
#include <fcntl.h>

/*
 * Definitions.
 */
#define	reg	register		/* Convenience */
#define	uint	unsigned int		/* Convenience */
#define	ulong	unsigned long		/* Convenience */
#define	STDERR	2			/* fileno(stderr) */
#define	STDOUT	1			/* fileno(stdout) */

#ifndef i386
#define PHYSBLKSZ 512
#endif

/*
 * External functions.
 */
void	exit();
long	lseek();
char	*strrchr();

/*
 * Internal functions.
 */
int	fsinfo();
void	prn();
void	prs();
char	*syserr();
void	usage();
int	warn();

/*
 * External variables.
 */
extern int	errno;			/* System error code */
extern char	*sys_errlist[];		/* Table of system error messages */
extern int	sys_nerr;		/* Number of sys_errlist entries */

/*
 * Static variables.
 */
static char	*myname;		/* Last qualifier of arg0 */
static short	fflag;			/* Free block count */
static short	iflag;			/* Total inode count */
static short	sflag;			/* File system sanity */
static short	lflag;			/* Number of free blocks */

main(ac, av)
int		ac;
reg char	**av;
{
	reg int		errors = 0;

	if (myname = strrchr(*av, '/'))
		++myname;
	else
		myname = *av;
	while (*++av && **av == '-')
		if (strcmp(*av, "-f") == 0)
			++fflag;
		else if (strcmp(*av, "-i") == 0)
			++iflag;
		else if (strcmp(*av, "-s") == 0)
			++sflag;
		else if (strcmp(*av, "-l") == 0)
			++lflag;
		else
			usage();
	if ((fflag == 0 && iflag == 0 && sflag == 0 && lflag == 0) || *av == 0)
		usage();
	while (*av)
		if (fsinfo(*av++) < 0)
			++errors;
	exit(errors);
}

/*
 * fsinfo()
 *
 * Print the number of 512-byte blocks needed by a given filesystem.
 */
static int
fsinfo(path)
char		*path;
{
	reg daddr_t		bsize;
	reg int			oops;
	reg int			fd;

#ifndef i386
	int			result;
#endif

	auto struct filsys	fs;

	if ((fd = open(path, O_RDONLY)) < 0)
		return (warn(path, syserr()));
	oops = (lseek(fd, SUPERBOFF, 0) < 0
	    || read(fd, (char *) &fs, sizeof(fs)) != sizeof(fs));
	if (oops || fs.s_magic != FsMAGIC)
		return (warn(path, "Not a filesystem"));

#ifdef i386
	switch ((int) fs.s_type) {
#else
	switch (fs.s_type) {
#endif

	case Fs1b:
		bsize = 512;
		break;
	case Fs2b:

#ifndef i386
		result = s5bsize(fd);
		if (result == Fs2b)
			bsize = 1024;
		else if (result == Fs4b)
			bsize = 2048;
		else
			return (warn(path, "Can't determine block size\n\troot inode or root directory may be corrupted"));
#else
		bsize = 1024;
#endif

		break;
	case Fs4b:
		bsize = 2048;
		break;
	default:
		return (warn(path, "Unknown filesystem type"));
	}
	(void) close(fd);
	if (fflag)
		prn(bsize * (fs.s_fsize - fs.s_tfree) / 512);
	if (iflag)
		prn(bsize * (fs.s_isize - 2) / sizeof(struct dinode));
	if (lflag)
		prn( (bsize * fs.s_tfree) / 512 );
	if (sflag)
		if ((fs.s_state + (long)fs.s_time) != FsOKAY) 
			return(-1);
	return (0);
}

/*
 * prn()
 *
 * Print a decimal number with a trailing newline.
 */
static void
prn(number)
reg ulong	number;
{
	reg char	*idx;
	auto char	buf[64];

	idx = buf + sizeof(buf);
	*--idx = '\0';
	*--idx = '\n';
	do {
		*--idx = "0123456789abcdef"[number % 10];
		number /= 10;
	} while (number);
	prs(idx);
}

/*
 * prs()
 *
 * Print a string.
 */
static void
prs(str)
reg char	*str;
{
	(void) write(STDOUT, str, (uint) strlen(str));
}

/*
 * syserr()
 *
 * Return a pointer to a system error message.
 */
static char *
syserr()
{
	return (errno <= 0 ? "No error (?)"
	    : errno < sys_nerr ? sys_errlist[errno]
	    : "Unknown error (!)");
}

/*
 * usage()
 *
 * Print a helpful message and exit.
 */
static void
usage()
{
	static char	before[] = "Usage:\t";

#ifdef i386
	static char	after[] = " [ -f -i -s -l ] filesystem ...\n";
#else
	static char	after[] = " [ -f -i -s ] filesystem ...\n";
#endif

	(void) write(STDERR, before, (uint) strlen(before));
	(void) write(STDERR, myname, (uint) strlen(myname));
	(void) write(STDERR, after, (uint) strlen(after));
	exit(1);
}

/*
 * warn()
 *
 * Print an error message. Always returns -1.
 */
static int
warn(what, why)
reg char	*what;
reg char	*why;
{
	static char	between[] = ": ";
	static char	after[] = "\n";

	(void) write(STDERR, myname, (uint) strlen(myname));
	(void) write(STDERR, between, (uint) strlen(between));
	(void) write(STDERR, what, (uint) strlen(what));
	(void) write(STDERR, between, (uint) strlen(between));
	(void) write(STDERR, why, (uint) strlen(why));
	(void) write(STDERR, after, (uint) strlen(after));
	return (-1);
}

#ifndef i386
/* heuristic function to determine logical block size of System V file system */

s5bsize(fd)
int fd;
{
	int results[3];
	int count;
	long address;
	long offset;
	char *buf;
	struct dinode *inodes;
	struct direct *dirs;
	char * p1;
	char * p2;
	
	results[1] = 0;
	results[2] = 0;

	buf = (char *)malloc(PHYSBLKSZ);

	for (count = 1; count < 3; count++) {

		address = 2048 * count;
		if (lseek(fd, address, 0) != address)
			continue;
		if (read(fd, buf, PHYSBLKSZ) != PHYSBLKSZ)
			continue;
		inodes = (struct dinode *)buf;
		if ((inodes[1].di_mode & S_IFMT) != S_IFDIR)
			continue;
		if (inodes[1].di_nlink < 2)
			continue;
		if ((inodes[1].di_size % sizeof(struct direct)) != 0)
			continue;
	
		p1 = (char *) &address;
		p2 = inodes[1].di_addr;
		*p1++ = 0;
		*p1++ = *p2++;
		*p1++ = *p2++;
		*p1   = *p2;
	
		offset = address << (count + 9);
		if (lseek(fd, offset, 0) != offset)
			continue;
		if (read(fd, buf, PHYSBLKSZ) != PHYSBLKSZ)
			continue;
		dirs = (struct direct *)buf;
		if (dirs[0].d_ino != 2 || dirs[1].d_ino != 2 )
			continue;
		if (strcmp(dirs[0].d_name,".") || strcmp(dirs[1].d_name,".."))
			continue;
		results[count] = 1;
		}
	free(buf);
	
	if(results[1])
		return(Fs2b);
	if(results[2])
		return(Fs4b);
	return(-1);
}
#endif	/* i386 */
