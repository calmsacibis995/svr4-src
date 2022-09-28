/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)idcmd:idmkenv.c	1.3.3.1"

/*
 *
 * Idmkenv: This command is part of the Installable Drivers (ID) 
 *	    scheme.  It is called at boot time by an Init State 2 
 *	    entry in /etc/inittab.  It's purpose is to update 
 *	    /etc/idrc.d, /etc/idsd.d, /etc/inittab, and the device 
 *	    nodes in /dev.
 *
 *          A second function of idmkenv is to recover the Kernel
 *          environment when a reconfiguration is aborted due to
 *          losing power or the user hitting RESET.
 *
 *	    Idmkenv had to be recoded in C for System V Release 4.0 
 *	    because a number of the commands it depended on (e.g., 
 *	    chown, chmod, cat, mkdir) were moved from /sbin to /usr/bin.  
 *	    On systems where /usr is a separate file system, idmkenv 
 *	    runs before /usr is mounted and these commands would not 
 *	    be available.
 */

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <varargs.h>
#include "inst.h"

#define	SIZE	1024
#define	MODE	0777

#define RD_ERROR	"Unable to read all contents of %s%s\n"

void mkenv(), recover();
void runcmd();
void build_str(), terminate_str();
void Perror();

char *BIN = "/etc/conf/bin";
char *PACK = "/etc/conf/pack.d";
char *CF = "/etc/conf/cf.d";
char *RC = "/etc/idrc.d";
char *SD = "/etc/idsd.d";
char *LASTDEV_A = "/etc/.last_dev_add";
char *LASTDEV_D = "/etc/.last_dev_del";

char errbuf[2 * MAXPATHLEN + 40];

/* High-level structure of the program consists of setting up the kernel
 * environment and attempting to recover an aborted reconfiguration.
 */

main()
{
	mkenv();
	recover();
	exit(0);
}

void mkenv()
{
	struct stat pstat;
	struct stat fstat;
	struct dirent *dir_entry;
	DIR *dirp;
	struct passwd *pwd;
	struct group  *grp;
	char fullpath[MAXPATHLEN];
	char buf[MAXPATHLEN];
	char *ebuf;
	uid_t uid;
	gid_t gid;


	strcpy(fullpath, EROOT);	/* EROOT=/etc in inst.h */
	strcat(fullpath, "/.new_unix");	/* indicates whether new unix was built */

	if (!stat(fullpath, &pstat)) {
		printf("Setting up new kernel environment\n");
		fflush(stdout);
		runcmd("rm -f /etc/idrc.d/* /etc/idsd.d/* /etc/ps_data");

		/* copy contents of rc.d, sd.d directories onto /etc */

		sprintf(buf, "%s/rc.d", ROOT);
		if ((dirp = opendir(buf)) != NULL) {
			ebuf = buf + strlen(buf);
			*ebuf++ = '/';
			while (dir_entry = readdir(dirp)) {

				/* only link regular files; "." and ".." entries
				 * are ignored */
				if (!stat(dir_entry->d_name, &fstat))
					if ((fstat.st_mode & S_IFMT) != S_IFREG)
						continue;

				/* buf contains the path of the file we want to
				 * link. "fullpath" contains the full path of
				 * the target dir/file.
				 */
				strcpy(ebuf, dir_entry->d_name);
				sprintf(fullpath, "%s/%s", RC, dir_entry->d_name);
				if(link(buf, fullpath)) {
					sprintf(errbuf,"<%s> or <%s>", buf, fullpath);
					Perror(errbuf);
				}
			}

		} else { /* idmkenv shell script did not check for this */
			sprintf(errbuf, "directory <%s>", buf);
			Perror(errbuf);
		}
		closedir(dirp);

		sprintf(buf, "%s/sd.d", ROOT);
		if ((dirp = opendir(buf)) != NULL) {
			ebuf = buf + strlen(buf);
			*ebuf++ = '/';
			while (dir_entry = readdir(dirp)) {

				/* only link regular files; "." and ".." entries
				 * are ignored */
				if (!stat(dir_entry->d_name, &fstat))
					if ((fstat.st_mode & S_IFMT) != S_IFREG)
						continue;

				/* buf contains the path of the file we want to
				 * link. "fullpath" contains the full path of
				 * the target dir/file.
				 */
				strcpy(ebuf, dir_entry->d_name);
				sprintf(fullpath, "%s/%s", SD, dir_entry->d_name);

				if(link(buf, fullpath)) {
					sprintf(errbuf,"<%s> or <%s>", buf, fullpath);
					Perror(errbuf);
				}
			}

		} else { /* idmkenv shell script did not check for this */
			sprintf(errbuf, "directory %s:", buf);
			Perror(errbuf);
		}
		closedir(dirp);

		/* invoke idmknod and idmkunix commands */
		runcmd("%s/idmknod", BIN);
		runcmd("%s/idmkinit", BIN);

		/* get numerical value for owner bin, group bin for chown syscall */

		pwd = getpwnam("bin");
		uid = pwd->pw_uid;

		grp = getgrnam("bin");
		gid = grp->gr_gid;

		sprintf(buf, "%s/inittab", CF);
		if (chown(buf, uid, gid))
			Perror(buf);
		if (chmod(buf, 0444))
			Perror(buf);
		runcmd("mv %s/inittab /etc", CF);
		runcmd("rm -rf /etc/.new_unix /etc/.unix_reconf %s %s", LASTDEV_A,
			LASTDEV_D);
		runcmd("sync");
		chmod("/etc/inittab", 0444);
	}
}

void recover()
{
	register int i;
	struct stat pstat;
	struct stat fstat;
	char fullpath[64];
	unsigned int size;
	char *buf;
	int fd, rsize;

	strcpy(fullpath, EROOT);		/* EROOT=/etc in inst.h */
	strcat(fullpath, "/.unix_reconf");	/* indicates whether recovery is
						   needed */

	if (!stat(fullpath, &pstat)) {
		printf("\n\tRecovering Kernel configuration.\n");
		fflush(stdout);
		if (!stat(LASTDEV_A, &fstat)) {
			size = (unsigned int)fstat.st_size;
			buf = (char *)malloc(size + 1);
			if ((fd = open(LASTDEV_A, O_RDONLY)) == -1) {
				free(buf);
				Perror(LASTDEV_A);
			}
			else {
				rsize = read(fd, buf, size);
				if (rsize != size) 
					fprintf(stderr, RD_ERROR, LASTDEV_A,"");
				buf[size] = '\0';

				build_str(buf);
				runcmd("%s/idinstall -d %s > /dev/null 2>&1",
					BIN, buf);
			}
		}
		else {
			if (!stat(LASTDEV_D, &fstat))
				if ((fstat.st_mode & S_IFMT) == S_IFDIR) {

					if (chdir(LASTDEV_D))
						Perror(LASTDEV_D);
					if(stat("mdevice", &fstat)) {
						sprintf(errbuf, "%s/mdevice", LASTDEV_D);
						Perror(errbuf);
					}
					runcmd("mv mdevice %s/mdevice",CF);

					if(stat("dev", &fstat)) {
						sprintf(errbuf, "%s/dev", LASTDEV_D);
						Perror(errbuf);
					}
					size = (unsigned int)fstat.st_size;
					buf = (char *)malloc(size + 1);
					if ((fd = open("dev", O_RDONLY)) == -1) {
						free(buf);
						Perror(LASTDEV_D);
					}

					rsize = read(fd, buf, size);
					if (rsize != size) 
						fprintf(stderr, RD_ERROR, LASTDEV_D,"/dev");
					buf[size] = '\0';

					terminate_str(buf);

					if (!stat("pack.d", &fstat)) {
						sprintf(fullpath, "%s/%s", PACK, buf);
						if (mkdir(fullpath, MODE))
							Perror(fullpath);
						runcmd("mv pack.d/* %s", fullpath);
					}
					else {
						sprintf(errbuf, "%s/pack.d", LASTDEV_D);
						Perror(errbuf);
					}
				}
		}
		chdir(CF);
		runcmd("rm -f unix config.h conf.c vector.c direct ifile buffers.o");
		runcmd("rm -f conf.o gdt.o linesw.o space.o vector.o sdevice.new");
		runcmd("rm -f /etc/conf/pack.d/*/Space.o");
		runcmd("rm -rf /etc/.unix_reconf %s %s", LASTDEV_A, LASTDEV_D);
	}
	runcmd("telinit q");
}

/* This routine takes a variable number of arguments to pass them to the "system"
 * library routine.
 */

void runcmd(va_alist)
va_dcl
{
	va_list args;
	char *fmt;
	char buf[SIZE];

	va_start(args);
	fmt = va_arg(args, char *);
	va_end(args);
	vsprintf(buf, fmt, args);
	system(buf);
	return;
}

/* This routine builds a string of files (one per line) by substituting all
 * new line characters by black space.
 */

void build_str(s)
char *s;
{
	register i;

	for (i = 0; s[i] != '\0'; i++)
		if (s[i] == '\n')
			s[i] = ' ';
}

/* This routine terminates a string that's assumed to consist of only one
 * one file name (otherwise the mkdir will not work). It substitutes the
 * first white space seen with a null character.
 */

void terminate_str(s)
char *s;
{
	register i;

	for (i = 0; s[i] != '\0'; i++)
		if (s[i] == ' ' || s[i] == '\t' || s[i] == '\n') {
			s[i] = '\0';
			break;
		}
}

/* generic print error routine */

void Perror(s)
char *s;
{
	fprintf(stderr, "idmkenv: ");
	perror(s);
	exit(1);
}
