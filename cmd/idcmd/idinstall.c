/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)idcmd:idinstall.c	1.3.1.1"

/*
 *       The idinstall command is called by a driver software package's
 *       (DSP) Install script and	its function is	to install, remove
 *       or update a DSP.	The command syntax is as follows:
 *
 *	       idinstall -[adug] -[msnirhcl] [-R dir] dev_name
 *			     |	     |	        |       |
 *			  action    DSP	     rootdir  internal device name
 *				component(*)
 *
 *	       -a  Add the DSP components
 *	       -d  Remove the DSP components
 *	       -u  Update the DSP components
 *	       -g  Get the DSP components (print to std	out, except Master)
 *	       -e  ignore error checking on free space (default on -g)
 *	       -k  Do not remove component from current directory on -a & -g
 *
 *	       -m Master component
 *	       -s System component
 *	       -o Driver.o component
 *	       -p Space.c component
 *	       -t Stubs.c component
 *	       -n Node (special	file) component
 *	       -i Inittab component
 *	       -r Device Initialization	(rc) component
 *	       -h Device shutdown component
 *             -c Mfsys component: file system type config. data
 *             -l Sfsys component: file system type local system data
 *
 *             -R directory: use this directory instead of /etc/conf
 *
 *
 *	       (*) If no component is specified	the default is all.
 *
 * exit 0 - success
 *	1 - error
 */

#include <stdio.h>
#include <filehdr.h>
#include <ctype.h>
#include <string.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ustat.h>

/* operations */
#define	MASTER		0x1
#define	SYSTEM  	0x2
#define NODE		0x4
#define INIT		0x8
#define	RCFILE		0x10
#define	SHUTDOWN	0x20
#define	MFSYS		0x40
#define	SFSYS		0x80
#define	DRIVER		0x100
#define	SPACE		0x200
#define	STUBS		0x400
#define	ALL		0x7ff

/* error messages */
#define USAGE		"Usage:  idinstall -[adug] [-e] [-k] -[msnirhclopt] [-R directory] dev_name"
#define NOLINK		"Cannot link <%s> to <%s>: '%s'"


#define TRUE	1
#define FALSE	0
#define SIZE 	512
char fbuf[SIZE];

char device[15];	/* device name */
char ppath[40];		/* relative device driver package directory */
char fullpath[40];	/* complete path to device driver package directory */
char root[512];		/* root directory containing cf and packages */

#define	CONF		"/etc/conf"
static char *confdirs[] = {
		"sdevice.d",
		"node.d",
		"init.d",
		"mfsys.d",
		"sfsys.d",
		"rc.d",
		"sd.d"
};

int debug=0;		/* debug flag */
int madedir=0;		/* Flag to remove partial install on error */

char actflag=0;		/* must have one of a, d, u, g */
char aflag=0;		/* -a flag specified, Add the component(s) */
char dflag=0;		/* -d flag specified, Delete the component(s) */
char uflag=0;		/* -u flag specified, Update the component(s) */
char gflag=0;		/* -g flag specified, Get (pr to stdout) component(s) */
char eflag=0;		/* -e flag specified, disable error check for space */
char kflag=0;		/* -k flag specified, do not remove local file */
char Rflag=0;		/* -R flag specified, use argument as ID root directory */

int partflag=0;		/* component flag; none means all */
char cmdline[80];

struct stat pstat;
extern void exit();
extern char *optarg;
extern int optind;
extern int errno;
extern char *sys_errlist[];

main(argc, argv)
int argc;
char *argv[];
{
	int	isthere, m;

	while ((m = getopt(argc, argv, "?#adugekmsnirhlcoptR:")) != EOF)
		switch (m) {
		case 'a':
			aflag++; 
			actflag++;
			break;
		case 'd':
			dflag++; 
			actflag++;
			break;
		case 'u':
			uflag++; 
			actflag++;
			break;
		case 'g':
			gflag++; 
			actflag++;
			break;

		case 'e':
			eflag++; 
			break;

		case 'k':
			kflag++; 
			break;

		case 'R':
			Rflag++;
			strcpy(root, optarg);
			break;
			
		case 'm':
			partflag |= MASTER;
			break;
		case 's':
			partflag |= SYSTEM;
			break;
		case 'o':
			partflag |= DRIVER;
			break;
		case 'p':
			partflag |= SPACE;
			break;
		case 't':
			partflag |= STUBS;
			break;
		case 'n':
			partflag |= NODE;
			break;
		case 'i':
			partflag |= INIT;
			break;
		case 'r':
			partflag |= RCFILE;
			break;
		case 'h':
			partflag |= SHUTDOWN;
			break;
		case 'l':
			partflag |= SFSYS;
			break;
		case 'c':
			partflag |= MFSYS;
			break;
		case '#':
			debug++;
			break;
		case '?':
			fprintf(stderr, "%s\n", USAGE);
			exit(1);
		}

	if (actflag != 1){
		fprintf(stderr,"Must have one of -a, -d, -u, -g options.\n"); 
		error(USAGE);
	}
	if (argc == optind){
		fprintf(stderr,"Must specify a device name.\n"); 
		error(USAGE);
	}
	sprintf(device, "%s", argv[optind]);

	if (*device == '\0'){
		fprintf(stderr,"Must specify a device name.\n"); 
		error(USAGE);
	}

	if (!(gflag || eflag))
		if (system ("/etc/conf/bin/idspace"))
			error("Insufficient disk space to reconfigure.");

	if (partflag == 0)
		partflag=ALL;

	strcpy(ppath, "/pack.d/");	/* dir. for package ( rel to CONF) */
	strcat(ppath, device);
	if (Rflag)
		strcpy(fullpath, root);		/* user specified */
	else
		strcpy(fullpath, CONF);		/* full path to directory */
	strcat(fullpath, ppath);

	if(debug){
		printf ("ppath = %s, fullpath = %s\n", ppath, fullpath);
		printf ("parts= %x, act= %x, dev= %s\n", partflag, actflag, device);
	}

	if (stat(fullpath,&pstat)){
		if (errno != ENOENT)
			error ("Cannot stat device driver directory.");
		isthere = FALSE;
	}
	else
		isthere = TRUE;
	if (aflag){
		if (partflag == ALL || (partflag & (DRIVER|SPACE|STUBS))){
			if (isthere)
				error("Device package already exists.");
			if (debug)
				printf ("making %s\n", fullpath);
			if (mkdir(fullpath, 0755) != 0)
				error("Cannot make the driver package directory.");
			mksave();
			madedir++;
		}
	}
	if (uflag | gflag){
		if(!isthere)
			error("Cannot open driver package directory.");
	}
	if (uflag)
		mksave();

	if (aflag | uflag){
		if (aflag && (partflag == ALL) ){
			if (stat("Driver.o",&pstat))
				error("Local directory does not contain a Driver object (Driver.o) file.");
			if (stat("Master",&pstat))
				error("Local directory does not contain a Master file.");
			if (stat("System",&pstat))
				error("Local directory does not contain a System file.");
		}
		if (partflag & DRIVER)
			if (!stat("Driver.o",&pstat)){
				ckmagic();  /* No return if wrong */
				idcp("Driver.o", ppath, "Driver.o", 0);
				idunlink("Driver.o");
			}
		if (partflag & SPACE)
			if (!stat("Space.c",&pstat)){
				idcp("Space.c", ppath,"space.c", 0);
				idunlink("Space.c");
			}
		if (partflag & STUBS)
			if (!stat("Stubs.c",&pstat)){
				idcp("Stubs.c", ppath,"stubs.c", 0);
				idunlink("Stubs.c");
			}
		if (partflag & MASTER){
			if (!stat("Master",&pstat)){
				if (idmast(aflag ? "-a" : "-u", 0)) {
					if (idmast("-a" , 0))
						error("Cannot add driver Master entry.");
				}
				else
					idunlink("Master");
			}
		}
		if (partflag & SYSTEM)
			if (!stat("System",&pstat)){
				idcp("System", "/sdevice.d", device, 0);
				idunlink("System");
			}
		if (partflag & NODE)
			if (!stat("Node",&pstat)){
				idcp("Node", "/node.d",device, 0);
				idunlink("Node");
			}
		if (partflag & INIT)
			if (!stat("Init",&pstat)){
				idcp("Init", "/init.d", device, 0);
				idunlink("Init");
			}
		if (partflag & RCFILE)
			if (!stat("Rc",&pstat)){
				idcp("Rc", "/rc.d", device, 0);
				idunlink("Rc");
			}
		if (partflag & SHUTDOWN)
			if (!stat("Shutdown",&pstat)){
				idcp("Shutdown", "/sd.d", device, 0);
				idunlink("Shutdown");
			}
		if (partflag & MFSYS)
			if (!stat("Mfsys",&pstat)){
				idcp("Mfsys", "/mfsys.d", device, 0);
				idunlink("Mfsys");
			}
		if (partflag & SFSYS)
			if (!stat("Sfsys",&pstat)){
				idcp("Sfsys", "/sfsys.d", device, 0);
				idunlink("Sfsys");
			}
	}
	if (dflag){
		if (partflag == ALL){
			mksave();
			rmpack(1,1);	/* Save and be quiet on err */
			exit(0);
		}
		if (partflag & MASTER){
			if (idmast ("-d",0))
				error("Cannot remove driver Master entry.");
		}
		if (partflag & SYSTEM)
			idrm("sdevice.d", (char *)0, 0);
		if (partflag & DRIVER)
			idrm("pack.d", "Driver.o", 0);
		if (partflag & SPACE)
			idrm("pack.d", "space.c", 0);
		if (partflag & STUBS)
			idrm("pack.d", "stubs.c", 0);
		if (partflag & NODE)
			idrm("node.d", (char *)0, 0);
		if (partflag & INIT)
			idrm("init.d", (char *)0, 0);
		if (partflag & RCFILE)
			idrm("rc.d", (char *)0, 0);
		if (partflag & SHUTDOWN)
			idrm("sd.d", (char *)0, 0);
		if (partflag & MFSYS)
			idrm("mfsys.d", (char *)0, 0);
		if (partflag & SFSYS)
			idrm("sfsys.d", (char *)0, 0);
	}
	if (gflag){
		if (partflag == ALL){
			fprintf(stderr,"Must have one of -s, -n, -i, -r, -h -c, -l -o, -p, -t\noptions when using -g.\n"); 
			error(USAGE);
		}
		if ( (partflag & MASTER) || (partflag & DRIVER) ){
			fprintf(stderr,"No -m or -o option with -g.\n"); 
			error(USAGE);
		}
		if (partflag & SPACE)
			idcat("pack.d", "space.c");
		if (partflag & STUBS)
			idcat("pack.d", "stubs.c");
		if (partflag & SYSTEM)
			idcat("sdevice.d", (char *)0);
		if (partflag & NODE)
			idcat("node.d", (char *)0);
		if (partflag & INIT)
			idcat("init.d", (char *)0);
		if (partflag & RCFILE)
			idcat("rc.d", (char *)0);
		if (partflag & SHUTDOWN)
			idcat("sd.d", (char *)0);
		if (partflag & MFSYS)
			idcat("mfsys.d", (char *)0);
		if (partflag & SFSYS)
			idcat("sfsys.d", (char *)0);
	}
	exit(0);
}
	
rmpack(savflg,qu)
int savflg;
int qu;
{
	int i;
	char rmdir[256];

	if (debug)
		printf("Removing device driver directory and its contents.\n");
	sprintf(rmdir, "rm -rf %s", fullpath);
	if (system(rmdir))
		if(!qu){
			fprintf(stderr, "idinstall: Cannot remove driver package directory\n");
			exit(1);
		}
	idmast ("-d",qu);
	for (i=0; i<(sizeof(confdirs)/sizeof(char *)); i++){
		strcpy(rmdir, confdirs[i]);
		idrm(rmdir, (char *)0, savflg);
	}
}
	

idrm(dirname,filname,savflg) 
char *dirname;
char *filname;
int savflg;
{
	char delfile[60], savfile[40];
	char errbuf[80];

	if (Rflag)
		strcpy(delfile, root);
	else
		strcpy(delfile, CONF);
	strcat(delfile, "/");
	strcat(delfile, dirname);
	strcat(delfile, "/");
	strcat(delfile, device);
	if (filname != (char *)0 ){
		strcat(delfile, "/");
		strcat(delfile, filname);
	}
	if (debug)
		printf ("removing %s\n", delfile);
	if (savflg && !stat(delfile, &pstat)){
		strcpy(savfile, "/etc/.last_dev_del/");
		strcat(savfile, dirname);
		strcat(savfile, "/");
		strcat(savfile, device);
		if (link(delfile, savfile) < 0) {
			sprintf(errbuf, NOLINK, delfile, savfile, sys_errlist[errno]);
			error(errbuf);
		}
	}
	unlink(delfile);
}

idcat(dirname, filname)
char *dirname;
char *filname;
{
	char catfile[60];

	if (Rflag)
		strcpy(catfile, root);
	else
		strcpy(catfile, CONF);
	strcat(catfile, "/");
	strcat(catfile, dirname);
	strcat(catfile, "/");
	strcat(catfile, device);
	if (filname != (char *)0 ){
		strcat(catfile, "/");
		strcat(catfile, filname);
	}
	strcpy(cmdline, "cat -s ");
	strcat(cmdline, catfile);
	if (system (cmdline)){
		fprintf(stderr, "idinstall: Cannot find driver component.\n");
		exit(1);
	}
}


/* print error message */

error(msg)
char *msg;
{
	fprintf(stderr, "idinstall: %s\n", msg);
	if (madedir){
		rmpack(0,1);	/* don't save, and be quiet */
		unlink ("/etc/.last_dev_add");
	}
	exit(1);
}

idcp(src,destd,destf, pkf)
char *src, *destd, *destf;
int pkf;
{
	char err[80];
	char tpath[80], spath[60];
	int from, to, ct;

	if (Rflag)
		strcpy(tpath, root);
	else
		strcpy(tpath,CONF);
	strcat(tpath,destd);
	strcat(tpath, "/");
	strcat(tpath,destf);
	if(uflag){
		strcpy(spath,"/etc/.last_dev_del");
		if(pkf)
			strcat(spath,"/pack.d");
		else {
			strcat(spath,destd);
			mkdir(spath, 0755);
		}
		strcat(spath, "/");
		strcat(spath,destf);
		if ( !stat(tpath, &pstat)) {
			if (link(tpath,spath) < 0) {
				sprintf(err, NOLINK, tpath, spath, sys_errlist[errno]);
				error(err);
			}
		}
		unlink(tpath);
	}
	if(debug)
		printf("copying %s\n", tpath);

	if((from = open(src, 0)) < 0)
		error("Cannot copy files - read open failed.");
	if((to = creat (tpath, 0644)) < 0)
		error("Cannot copy files - write open failed.");
	while((ct = read(from, fbuf, SIZE)) != 0)
		if(ct < 0 || write(to, fbuf, ct) != ct)
			error("Cannot copy file - read/write failed.");
	close(to);
	close(from);
}
idmast(idmflg,quiet)
char *idmflg;
int quiet;
{
	register int id, w;
	int wstat;
	char errbuf[80];
	static char pathbuf[80];
	static char option[4];
	static char pathopt[4];
	static char cmdb[]="/etc/conf/bin/idmaster";
	static char *idm[] = {
		cmdb,
		option,
		device,
		NULL
	};
	static char *idmdb[] = {
		cmdb,
		"-#",
		option,
		device,
		NULL
	};
	static char *idmdc[] = {
		cmdb,
		pathopt,
		pathbuf,
		option,
		device,
		NULL
	};
	static char *idmdd[] = {
		cmdb,
		"-#",
		pathopt,
		pathbuf,
		option,
		device,
		NULL
	};

	strcpy(option, idmflg);
	if (Rflag)
		sprintf(pathopt, "-o");
		sprintf(pathbuf, "%s/cf.d", root);

	if (aflag || uflag){ 			/* no device flag if -a or -u */
		if (Rflag) {
			if (debug)
				idmdd[5] = NULL;
			else
				idmdc[4] = NULL;
		}
		else {
			if (debug)
				idmdb[3]=NULL;
			else
				idm[2]=NULL;
		}
	}

	if ((id = fork()) == 0) {
		if(quiet){
			close(1);
			close(2);
		}

		if (Rflag) {
			if (debug) {
				char **p;
	
				printf("execing: ");
				for (p = idmdd; *p != NULL; p++)
					printf("%s ", *p);
				printf("\n");
				execv(idmdd[0], idmdd);
			}
			else
				execv(idmdc[0], idmdc);
		}
		else {
			if (debug){
				char **p;
	
				printf("execing: ");
				for (p = idmdb; *p != NULL; p++)
					printf("%s ", *p);
				printf("\n");
				execv(idmdb[0], idmdb);
			}
			else
				execv(idm[0], idm);
		}

		sprintf(errbuf, "idmaster exec failure, errno=%d", errno);
		error(errbuf);
	} else if (id  == -1) {
		sprintf(errbuf, "fork failure, errno=%d", errno);
		error(errbuf);
	} else
		w = wait(&wstat);

	if (w == id && wstat == 0)
		return(0);

	return(-1);
}

mksave()
{
	int fd, from, to, ct;
	struct stat dstat;
	char cfile[40];
	char pfile[80];

	if (aflag){
		if (!stat("/etc/.last_dev_del",&pstat))
			system ("rm -rf /etc/.last_dev_del > /dev/null 2>&1");
		if (debug)
			printf ("making /etc/.last_dev_add\n");
		if ((fd=creat("/etc/.last_dev_add", 0644))<0)
			error("Cannot create recovery files.");
		write(fd, device, sizeof(device));
		close(fd);
	}
	if(dflag | uflag){
		unlink ("/etc/.last_dev_add");
		if (!stat("/etc/.last_dev_del",&pstat))
			system ("rm -rf /etc/.last_dev_del > /dev/null 2>&1");
		if (debug)
			printf ("making /etc/.last_dev_del\n");
		mkdir("/etc/.last_dev_del", 0755);
		if ((fd=creat("/etc/.last_dev_del/dev", 0644))<0)
			error("Cannot create recovery file - /etc/.last_dev_del/dev");
		write(fd, device, sizeof(device));
		close(fd);

		if((from = open("/etc/conf/cf.d/mdevice", 0)) < 0)
			error("Cannot create recovery files - read mdevice.");
		if((to = creat ("/etc/.last_dev_del/mdevice", 0644)) < 0)
			error("Cannot create recovery files - create mdevice.");
		while((ct = read(from, fbuf, SIZE)) != 0)
			if(ct < 0 || write(to, fbuf, ct) != ct)
				error("Cannot create recovery files - copy mdevice.");
		close(to);
		close(from);

		/* create all the other directories that may be needed in case 
		 * of removal or update of a DSP.
		 */

		if (debug)
			printf ("making /etc/.last_dev_del/pack.d\n");
		mkdir("/etc/.last_dev_del/pack.d", 0755);
		if (Rflag)
			strcpy(cfile, root);
		else
			strcpy(cfile, CONF);

		/* make sdevice.d directory, if needed */
		strcpy(pfile, cfile);
		strcat(pfile, "/sdevice.d/");
		strcat(pfile, device);
		if (!stat(pfile, &dstat)) {
			if (debug)
				printf ("making /etc/.last_dev_del/sdevice.d\n");
			mkdir("/etc/.last_dev_del/sdevice.d", 0755);
		}

		/* make node.d directory, if needed */
		strcpy(pfile, cfile);
		strcat(pfile, "/node.d/");
		strcat(pfile, device);
		if (!stat(pfile, &dstat)) {
			if (debug)
				printf ("making /etc/.last_dev_del/node.d\n");
			mkdir("/etc/.last_dev_del/node.d", 0755);
		}

		/* make init.d directory, if needed */
		strcpy(pfile, cfile);
		strcat(pfile, "/init.d/");
		strcat(pfile, device);
		if (!stat(pfile, &dstat)) {
			if (debug)
				printf ("making /etc/.last_dev_del/init.d\n");
			mkdir("/etc/.last_dev_del/init.d", 0755);
		}

		/* make rc.d directory, if needed */
		strcpy(pfile, cfile);
		strcat(pfile, "/rc.d/");
		strcat(pfile, device);
		if (!stat(pfile, &dstat)) {
			if (debug)
				printf ("making /etc/.last_dev_del/rc.d\n");
			mkdir("/etc/.last_dev_del/rc.d", 0755);
		}

		/* make sd.d directory, if needed */
		strcpy(pfile, cfile);
		strcat(pfile, "/sd.d/");
		strcat(pfile, device);
		if (!stat(pfile, &dstat)) {
			if (debug)
				printf ("making /etc/.last_dev_del/sd.d\n");
			mkdir("/etc/.last_dev_del/sd.d", 0755);
		}
		/* make mfsys.d directory, if needed */
		strcpy(pfile, cfile);
		strcat(pfile, "/mfsys.d/");
		strcat(pfile, device);
		if (!stat(pfile, &dstat)) {
			if (debug)
				printf ("making /etc/.last_dev_del/mfsys.d\n");
			mkdir("/etc/.last_dev_del/mfsys.d", 0755);
		}

		/* make sfsys.d directory, if needed */
		strcpy(pfile, cfile);
		strcat(pfile, "/sfsys.d/");
		strcat(pfile, device);
		if (!stat(pfile, &dstat)) {
			if (debug)
				printf ("making /etc/.last_dev_del/sfsys.d\n");
			mkdir("/etc/.last_dev_del/sfsys.d", 0755);
		}
	}
}
ckmagic()  /* No return if wrong */
{
	int fd, ct;
	struct filehdr ex;
	static char cmd[] = "/usr/bin/file Driver.o | grep 386 >/dev/null\n";

	if((fd = open("Driver.o", 0)) < 0)
		error("Cannot open Driver.o.");
	ct = read(fd, &ex, sizeof(ex));
	if(ct < sizeof(ex) )
		error("Cannot read Driver.o header information");
	close(fd);
/*
	if (debug)
		printf("Driver.o magic number = 0%o\n", ex.f_magic);
	if (ex.f_magic != (unsigned short)I386MAGIC)
*/
	if (system(cmd) != 0)
		error("Driver.o is wrong type (not i386 executable)");
	return(0);
}
idunlink(unl)
char *unl;
{
	if(!kflag)
		unlink(unl);
}
