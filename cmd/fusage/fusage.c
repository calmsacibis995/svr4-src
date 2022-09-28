/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)fusage:fusage.c	1.15.11.1"



#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <sys/mnttab.h>
#include <sys/utsname.h>
#include <nserve.h>
#include <ctype.h>
#include <sys/param.h>
#include <sys/fcntl.h>
#include <sys/nserve.h>
#include <sys/rf_sys.h>
#include <nlist.h>
#include <sys/vfs.h>
#include <errno.h>

#define	ROOTVFS		"rootvfs"
#define OS		"/stand/unix"
#define KMEM		"/dev/kmem"
#define ADVTAB		"/etc/dfs/sharetab"
#define	REMOTE		"/etc/dfs/fstypes"
#define	FSTYPE_MAX	8
#define	REMOTE_MAX	64

#ifndef SZ_PATH
#define SZ_PATH		128
#endif

struct	nlist	nl[]={
	{ROOTVFS},
	{""}
};

struct advlst {
	char resrc[SZ_RES+1];
	char dir[SZ_PATH+1];
} *advlst;

char 		*malloc();
extern 	int 	errno;
extern	char	*sys_errlist[];

main(argc, argv)
	int	argc;
	char	*argv[];
{
	char	*cmdname;
	char 	str[SZ_RES+SZ_MACH+SZ_PATH+20];
	long	fs_blksize;
	FILE 	*atab;
	FILE	*mtab;
	int 	nadv = 0;	/* number of advertised resources */
	int	clientsum;
	int	advsum;
	int	local;
#ifdef	i386
	int	fswant=0;
#else
	int	fswant;
#endif
	int	ii;
	int	jj;
	int	exitcode = 0;
	struct client 	*clientp;
	struct statfs	fsi;
	struct utsname	myname;
	struct mnttab 	mnttab;

	long	getcount();
	void	prdat();
	
	cmdname = argv[0];		/* get command name from argv list */
	for (ii = 0; ii < argc; ii++) {
		if (argv[ii][0] == '-') {
			fprintf(stderr, "Usage: %s [mounted file system]\n",
			  argv[0]);
			fprintf(stderr, "          [advertised resource]\n");
			fprintf(stderr,
			  "          [mounted block special device]\n");
			exit(1);
		}
	}

	/*
	 * Determine RFS status.  If it is installed, begin the RFS things
	 * to do.
	 */
	if ((rfsys(RF_RUNSTATE) != -1 || errno != ENOPKG) &&
	  (atab = fopen(ADVTAB, "r")) != NULL) {
		int max_clients;

		/*
		 * we are not going to store the complete line for each entry
		 * in advtab.  Only the resource name and the path name
		 * are important here (client names are unnecessary).
		 */
		while (getline(str, sizeof(str), atab) != 0) {
			nadv++;
		}
		if ((advlst =
		  (struct advlst *)malloc(nadv * sizeof(struct advlst))) == 0) {
			fprintf(stderr,
			  "fusage: cannot get memory for advtab\n");
			exit(1);
		}
		rewind(atab);
		/*
		 * load advlst from advtab
		 */
		ii = 0;
		while ((getline(str, sizeof(str), atab) != 0) && (ii < nadv)) {
			loadadvl(str, &advlst[ii++]);
		}
		/* 
		 * find out what NSRMOUNT is and allocate memory for struct
		 * client accordingly.
		 */
		if ((max_clients = rfsys(RF_TUNEABLE, T_NSRMOUNT)) <= 0) {
			perror (cmdname);
			exit(1);
		}
		if ((clientp = (struct client *)malloc(max_clients *
		  sizeof(struct client))) == NULL ) {
			fprintf(stderr, "%s: memory allocation failed\n",
			  cmdname);
			exit(1);
		}
	}
	uname(&myname);
	printf("\nFILE USAGE REPORT FOR %.8s\n\n", myname.nodename);
	if ((mtab = fopen(MNTTAB, "r")) == NULL) {
		fprintf(stderr,"fusage: cannot open %s", MNTTAB);
		perror("open");
		exit(1);
	}
	
	/* 
	 * Process each entry in the mnttab.  If the entry matches a requested
	 * name, or all are being reported (default), print its data.  If the
	 * entry names a file system containing one or more advertised resources
	 * that are requested, print their data relative to this entry.
	 */
	while (getmntent(mtab, &mnttab) != -1) {
		if (mnttab.mnt_special == NULL || mnttab.mnt_mountp == NULL)
			continue;
		if (remote(mnttab.mnt_fstype))
			continue;
		if (shouldprint(argc, argv, mnttab.mnt_special,
		  mnttab.mnt_mountp)) {
			printf("\n\t%-15s      %s\n", mnttab.mnt_special, 
			  mnttab.mnt_special);
			fswant = 1;
		} else {
			fswant = 0;
		}
		if (statfs(mnttab.mnt_mountp, &fsi, sizeof(struct statfs), 0)
		  < 0) {
			fs_blksize = 1024;  /* per file system basis */
			printf("forced fs_blksize\n");
		} else {
			fs_blksize = fsi.f_bsize;
		}

		advsum = 0;
		for (ii = 0; ii < nadv; ii++) {
			int		n_clients;
			struct client 	*cl_p;

			if (isinfs(mnttab.mnt_mountp, advlst[ii].dir) &&
			  shouldprint(argc, argv, advlst[ii].resrc,
			  advlst[ii].dir)) {
				printf("\n\t%15s", advlst[ii].resrc);

				/* get client list of this resource  */
				if ((n_clients = rfsys(RF_CLIENTS,
				  advlst[ii].resrc, clientp)) < 0) {
					fprintf(stderr,
					  "%s: can't find client list: %s/n",
					  cmdname, sys_errlist[errno]);
					exit(1);
				}
				
				if (n_clients == 0) {
					printf(" (%s) ...no clients\n",
					  advlst[ii].dir);
					continue;
				}
				printf("      %s\n", advlst[ii].dir);
				for (jj = clientsum = 0, cl_p = clientp;
				  jj < n_clients; jj++, cl_p++) {
					prdat(cl_p->cl_node, cl_p->cl_bcount,
					  fs_blksize);
					clientsum += cl_p->cl_bcount;
				}
				prdat("Sub Total", clientsum, fs_blksize);
				advsum += clientsum;
			}
		}
		if (fswant) {
			printf("\n\t%15s      %s\n", "", mnttab.mnt_mountp);
			if ((local = getcount(mnttab.mnt_mountp, cmdname))
			  != -1) {
				prdat(myname.nodename, local, fs_blksize);
			}
			if (advsum) {
				prdat("Clients", advsum, fs_blksize);
				prdat("TOTAL", local + advsum, fs_blksize);
			}
		}
	}
	for (ii = 1; ii < argc; ii++) {
		if (argv[ii][0] != '\0') {
			exitcode = 2;
			fprintf(stderr,"'%s' not found\n", argv[ii]);
		}
	}
	exit(exitcode);
}

/*
 * Should the file system/resource named by dir and special be printed?
 */
int
shouldprint(argc, argv, dir, special)
	int	argc;
	char	*argv[];
	char	*dir;
	char	*special;
{
	int	found;
	int	i;

	found = 0;
	if (argc == 1) {
		return 1;	/* the default is "print everything" */
	}
	for (i = 0; i < argc; i++) {
		if (!strcmp(dir, argv[i]) || !strcmp(special, argv[i])) {
			argv[i][0] = '\0';	/* done with this arg */
			found++;		/* continue scan to find */
		}				/* duplicate requests */
	}
	return found;
}

void
prdat(s, n, bsize)
	char	*s;
	int	n;
#ifdef i386
	long	bsize;
#else
	short	bsize;
#endif
{
	(void)printf("\t\t\t%15s %10d KB\n", s, n * bsize / 1024);
}

/*
 * Is 'advdir' within mountp?
 */
int
isinfs(mountp, advdir)
	char		*mountp;
	char		*advdir;
{
	struct stat	mpstat;
	struct stat	advstat;

	stat(mountp, &mpstat);
	stat(advdir, &advstat);
	if (advstat.st_dev == mpstat.st_dev) {
		return 1;
	}
	return 0;
}

/*
 * Read up to len characters from the file fp and toss remaining characters
 * up to a newline.  If the last line is not terminated in a '\n', returns 0;
 */ 
int
getline(str, len, fp)
	char	*str;
	int	len;
	FILE	*fp;
{
	int	c;
	int	i = 1;
	char	*s = str; 

	for ( ; ; ) {
		switch (c = getc(fp)) {
		case EOF:
			*s = '\0';
			return 0;
		case '\n':
			*s = '\0';
			return 1;
		default:
			if (i < len) {
				*s++ = (char)c;
			}
			i++;
		}
	}
}

loadadvl(s, advx)
	char		*s;
	struct advlst	*advx;
{
	int		i = 0;

	while (isspace(*s)) {
		s++;
	}
	while (!isspace(*s) && (i < SZ_RES)) {
		advx->dir[i++] = *s++;
	}
	advx->dir[i] = '\0';

	while (isspace(*s)) {
		s++;
	}
	for (i = 0; !isspace(*s) && i < SZ_MACH + SZ_PATH + 1; i++) {
		advx->resrc[i] = *s++;
	}
	advx->resrc[i] = '\0';
}

long
getcount(fs_name, cmdname)
	char		*fs_name;
	char		*cmdname;
{
	struct stat	statb;
	vfs_t		vfs_buf;
	vfs_t		*next_vfs;
	vfs_t		*rvfs;
	int		memfd;
	static int	nl_done = 0;
	static long	root_pos;

	if (stat(fs_name, &statb) == -1) {
		fprintf(stderr, "%s: stat failed\n", cmdname);
		perror(fs_name);
		return -1;
	}
	
	/* open KMEM */
	if ((memfd = open(KMEM, O_RDONLY)) == -1) {
		fprintf(stderr, "%s: open %s error\n", cmdname, KMEM);
		perror(KMEM);
		return -1;
	}
	if (nl_done == 0) {
		if (nlist(OS, nl) == -1) {
			fprintf(stderr, "%s: cannot find ROOTVFS \n", cmdname);
			perror("nlist");
			return -1;
		}
		nl_done++;
		root_pos = nl[0].n_value;
	}

	if (rread(memfd, root_pos, &rvfs, sizeof(struct vfs *)) == -1) {
		fprintf(stderr, "%s: failed to read rootvfs \n", cmdname);
		return -1;
	}
	next_vfs = rvfs;
	while (next_vfs) {
		if (rread(memfd, next_vfs, &vfs_buf,
		  sizeof(struct vfs)) == -1) {
			fprintf(stderr, "%s: cannot read next vfs \n", cmdname);
			close(memfd);
			return -1;
		}
		/* check if this is the same device */
		if (vfs_buf.vfs_dev == statb.st_dev) {
			close(memfd);
			return vfs_buf.vfs_bcount;
		} else {
			next_vfs = vfs_buf.vfs_next;
		}
	}

	/*
	 * not found in vfs list
	 */
	fprintf(stderr, "%s: %s not found in kernel\n", cmdname, fs_name);
	close(memfd);
	return -1;
}
		
int
rread(device, position, buffer, count)
	char	*buffer;
	int	count;
	int	device;
	long	position;
{
	int	rval;
	
	if (lseek(device, position, 0) == (long)-1) {
		fprintf(stderr, "Seek error in %s\n", KMEM);
		return -1;
	}
	if ((rval = read(device, buffer, (unsigned)count)) == -1) {
		fprintf(stderr, "Read error in %s\n", KMEM);
		return -1;
	}
	return rval;
}

/*
 * Returns 1 if fstype is a remote file system type, 0 otherwise.
 */
int
remote(fstype)
	char		*fstype;
{
	char		buf[BUFSIZ];
	char		*fs;
	static int	first_call = 1;
 	static FILE	*fp;

	extern char	*strtok();

	if (first_call) {
		if ((fp = fopen(REMOTE, "r")) == NULL) {
			fprintf(stderr, "Unable to open %s\n", REMOTE);
			return 0;
		} else {
			first_call = 0;
		}
	} else if (fp != NULL) {
		rewind(fp);
	} else {
		return 0;	/* open of REMOTE had previously failed */
	}
	if (fstype == NULL || strlen(fstype) > FSTYPE_MAX) {
		return	0;	/* not a remote */
	}
	while (fgets(buf, sizeof(buf), fp) != NULL) {
		fs = strtok(buf, " \t");
		if (!strcmp(fstype, fs)) {
			return	1;	/* is a remote fs */
		}
	}
	return	0;	/* not a remote */
}
