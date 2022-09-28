/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


/* 	Portions Copyright(c) 1988, Sun Microsystems Inc.	*/
/*	All Rights Reserved					*/

#ident	"@(#)find:find.c	4.34.1.3"
/*
 *
 * Rewrite of find program to use nftw(new file tree walk) library function
 * This is intended to be upward compatible to System V release 3.
 * There is one additional feature:
 *	If the last argument to -exec is {} and you specify + rather
 *	than ';', the command will be invoked fewer times with {}
 *	replaced by groups of pathnames. 
 */


#include <stdio.h>
#include <errno.h>
#include <pwd.h>
#include <grp.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/param.h>
#include <ftw.h>
#include <limits.h>
#include <unistd.h>
#include <stdlib.h>
#include <locale.h>

#define A_DAY		(long)(60*60*24)	/* a day full of seconds */
#define BLKSIZ		512
#define round(x,s)	(((x)+(s)-1)&~((s)-1))
#ifndef FTW_SLN
#define FTW_SLN		7
#endif

/*
 * This is the list of operations
 */
enum Command
{
	PRINT, DEPTH, LOCAL, MOUNT, ATIME, MTIME, CTIME, NEWER,
	NAME, USER, GROUP, INUM, SIZE, LINKS, PERM, EXEC, OK, CPIO, NCPIO,
	TYPE, AND, OR, NOT, LPAREN, RPAREN, CSIZE, VARARGS, FOLLOW,
	PRUNE, NOUSER, NOGRP, FSTYPE
};

enum Type
{
	Unary, Id, Num, Str, Exec, Cpio, Op
};

struct Args
{
	char		name[10];
	enum Command	action;
	enum Type	type;
};

/*
 * Except for pathnames, these are the only legal arguments
 */
struct Args commands[] =
{
	"!",		NOT,	Op,
	"(",		LPAREN,	Unary,
	")",		RPAREN,	Unary,
	"-a",		AND,	Op,
	"-atime",	ATIME,	Num,
	"-cpio",	CPIO,	Cpio,
	"-ctime",	CTIME,	Num,
	"-depth",	DEPTH,	Unary,
	"-exec",	EXEC,	Exec,
	"-follow",	FOLLOW, Unary,
	"-group",	GROUP,	Num,
	"-inum",	INUM,	Num,
	"-links",	LINKS,	Num,
	"-local",	LOCAL,	Unary,
	"-mount",	MOUNT,	Unary,
	"-mtime",	MTIME,	Num,
	"-name",	NAME,	Str,
	"-ncpio",	NCPIO,  Cpio,
	"-newer",	NEWER,	Str,
	"-o",		OR,	Op,
	"-ok",		OK,	Exec,
	"-perm",	PERM,	Num,
	"-print",	PRINT,	Unary,
	"-size",	SIZE,	Num,
	"-type",	TYPE,	Num,
	"-xdev",	MOUNT,	Unary,
	"-user",	USER,	Num,
	"-prune",	PRUNE,	Unary,
	"-nouser",	NOUSER,	Unary,
	"-nogroup",	NOGRP,	Unary,
	"-fstype",	FSTYPE,	Str,
	0,
};

union Item
{
	struct Node	*np;
	struct Arglist	*vp;
	time_t		t;
	char		*cp;
	char		**ap;
	long		l;
	int		i;
};

struct Node
{
	struct Node	*next;
	enum Command	action;
	union Item	first;
	union Item	second;
};

/*
 * Prototype variable size arglist buffer
 */

struct Arglist
{
	struct Arglist	*next;
	char		*end;
	char		*nextstr;
	char		**firstvar;
	char		**nextvar;
	char		*arglist[1];
};

static int		walkflags = FTW_CHDIR|FTW_PHYS;
static int		compile();
static int		execute();
static int		doexec();
static char		*getfstype();
static struct Args	*lookup();
static int		ok();
static void		usage();
static struct Arglist	*varargs();

static struct Node	*topnode;
static char		*cpio[] = { "cpio", "-o", 0 };
static char		*ncpio[] = { "cpio", "-oc", 0 };
static char		*cpiol[] = { "cpio", "-oL", 0 };
static char		*ncpiol[] = { "cpio", "-ocL", 0 };
static long		now;
static FILE		*output;
static char		*dummyarg = (char*)-1;
static int		lastval;
static int		varsize;
static struct Arglist	*lastlist;
static char		*cmdname;

FILE	*cmdopen();
int	cmdclose();
char	*getshell();
extern int	exec();
extern int	nftw();
extern struct	group	*getgruid();
extern int	strcmp();
extern int	strlen();
extern time_t	time();
extern int	errno;
extern char	*sys_errlist[];
extern char	**environ;

main(argc, argv)
char *argv[];
{
	register char *cp;
	register int paths,xerr;
	time_t time();

	(void)setlocale(LC_ALL, "");

	cmdname = argv[0];
	if (time(&now) == (time_t) -1) 
	{
		fprintf(stderr, "%s: time() %s\n", cmdname, sys_errlist[errno]);
		exit(1);
	}
	if(argc<3)
	{
		fprintf(stderr, "%s: insufficient number of arguments\n",cmdname);
		usage();
	}
	for(paths=1; cp = argv[paths] ; ++paths)
	{
		if(*cp== '-')
			break;
		else if((*cp=='!' || *cp=='(') && *(cp+1)==0)
			break;
	}
	if(paths == 1) /* no path-list */
		usage();
	output = stdout;
	/* allocate enough space for the compiler */
	topnode = (struct Node*)malloc(argc*sizeof(struct Node));
	compile(argv+paths,topnode);
	while(--paths)
		if (nftw(*++argv, execute, 1000, walkflags))
			fprintf(stderr, "%s: cannot open %s: %s\n",
				cmdname, *argv, sys_errlist[errno]); 
	/* execute any remaining variable length lists */
	while(lastlist)
	{
		if(lastlist->end != lastlist->nextstr)
		{
			*lastlist->nextvar = 0;
			doexec((char*)0,lastlist->arglist);
		}
		lastlist = lastlist->next;
	}
	if(output != stdout)
		exit(cmdclose(output));
	exit(0);
}

/*
 * compile the arguments
 */

static int
compile(argv,np)
char **argv;
register struct Node *np;
{
	register char *b;
	register char **av;
	register struct Node *oldnp = 0;
	struct Args *argp;
	char **com;
	int action = 0;
	int i;
	enum Command wasop = PRINT;
	for(av=argv; *av && (argp=lookup(*av)); av++)
	{
		np->next = 0;
		np->action = argp->action;
		np->second.i = 0; 
		if(argp->type==Op)
		{
			if(wasop==NOT || (wasop && np->action!=NOT))
			{
				fprintf(stderr, "%s: operand follows operand\n", cmdname);
				exit(1);
			}
			if(np->action!=NOT && oldnp==0)
				goto err;
			wasop=argp->action;
		}
		else
		{
			wasop = PRINT;
			if(argp->type != Unary)
			{
				if(!(b = *++av))
				{
					fprintf(stderr, "%s: incomplete statement\n", cmdname);
					exit(1);
				}
				if(argp->type == Num)
				{
					if(*b=='+' || *b=='-')
					{
						np->second.i = *b; 
						b++;
					}
				}
			}
		}
		switch(argp->action)
		{
			case AND:
				continue;
			case NOT:
				break;
			case OR:
				np->first.np = topnode;
				topnode = np;
				oldnp->next = 0;
				break;
			case LPAREN:
			{
				struct Node *save = topnode;
				topnode = np+1;
				i = compile(++av,topnode);
				np->first.np = topnode;
				topnode = save;
				av += i;
				oldnp = np;
				np->next = np+i;
				np += i;
				continue;
			}
			case RPAREN:
				if(oldnp==0)
					goto err;
				oldnp->next = 0;
				return(av-argv);
			case FOLLOW:
				walkflags &= ~FTW_PHYS;
				break;
			case MOUNT:
				walkflags |= FTW_MOUNT;
				break;
			case DEPTH:
				walkflags |= FTW_DEPTH;
				break;
			case LOCAL:
				np->first.l = 0;
				np->second.i = '+';
				break;
			case SIZE:
				if(b[strlen(b)-1]=='c')
					np->action = CSIZE;
			case CTIME:
			case MTIME:
			case ATIME:
			case LINKS:
			case INUM:
				np->first.l = atoi(b);
				break;
			case USER:
			case GROUP:
			{
				struct	passwd	*pw;
				struct	group *gr;
				i = -1;
				if(argp->action == USER)
				{
					if((pw=getpwnam(b))!= 0)
						i = (int)pw->pw_uid;
				}
				else
				{
					if((gr=getgrnam(b))!= 0)
						i = (int)gr->gr_gid;
				}
				if(i == -1)
				{
					if(!gmatch(b, "[0-9][0-9][0-9]*")
					&& !gmatch(b, "[0-9][0-9]")
					&& !gmatch(b, "[0-9]"))
					{ 
						fprintf(stderr,"%s: cannot find %s name\n", cmdname, *av);
						exit(1);
					}
					i = atoi(b);
				}
				np->first.l = i;
				break;
			}
			case EXEC:
			case OK:
				walkflags &= ~FTW_CHDIR;
				np->first.ap = av;
				action++;
				while(1)
				{
					if((b= *++av)==0)
					{
					 	fprintf(stderr, "%s: incomplete statement", cmdname);
						exit(1);
					}
					if(strcmp(b,";")==0)
					{
						*av = 0;
						break;
					}
					else if(strcmp(b,"{}")==0)
						*av = dummyarg;
					else if(strcmp(b,"+")==0 &&
						av[-1]==dummyarg &&
						np->action==EXEC)
					{
						av[-1] = 0;
						np->first.vp = varargs(np->first.ap);
						np->action = VARARGS;
						break;
					}
				}
				break;
			case NAME:
				np->first.cp = b;
				break;
			case PERM:
				for(i=0; *b ; ++b)
				{
					if(*b < '0' || *b >= '8')
					{
						fprintf(stderr, "%s: %s not octal\n",
							cmdname, *av);
						exit(1);
					}
					i <<= 3;
					i = i + (*b - '0');
				}
				np->first.l = i;
				break;
			case TYPE:
				i = *b;
				np->first.l = i=='d' ? S_IFDIR :
				    i=='b' ? S_IFBLK :
				    i=='c' ? S_IFCHR :
#ifdef S_IFIFO
				    i=='p' ? S_IFIFO :
#endif
				    i=='f' ? S_IFREG :
#ifdef S_IFLNK
				    i=='l' ? S_IFLNK :
#endif
				    0;
				break;
			case CPIO:
				if (walkflags & FTW_PHYS)
					com = cpio;
				else
					com = cpiol;
				goto common;
			case NCPIO: 	
			{
				FILE *fd;

				if (walkflags & FTW_PHYS)
					com = ncpio;
				else
					com = ncpiol;
			common:
				/* set up cpio */
				if((fd=fopen(b, "w")) == NULL)
				{
					fprintf(stderr,"%s: cannot create %s\n",
						cmdname, b);
					exit(1);
				}
				np->first.l = (long)cmdopen("cpio",com,"w",fd);
				fclose(fd);
				walkflags |= FTW_DEPTH;
				np->action = CPIO;
			}
			case PRINT:
				action++;
				break;
			case NEWER:
			{
				struct stat statb;
				if(stat(b, &statb) < 0)
				{
					fprintf(stderr,"%s: cannot access %s\n",
						cmdname, b);
					exit(1);
				}
				np->first.l = statb.st_mtime;
				np->second.i = '+';
				break;
			}
			case PRUNE:
			case NOUSER:
			case NOGRP:
				break;
			case FSTYPE:
				np->first.cp = b;
				break;
		}
		oldnp = np++;
		oldnp->next = np;
	}
	if((*av) || (wasop))
		goto err;
	else if (action == 0)
		{
			fprintf(stderr,"%s: no action specified\n",cmdname);
			exit(1);
		}
	oldnp->next = 0;
	return(av-argv);
err:	
	fprintf(stderr,"%s: bad option %s\n",cmdname, *av);
	usage();
}

/*
 * print out a usage message
 */

static void
usage()
{
	fprintf(stderr,"%s: path-list predicate-list\n", cmdname);
	exit(1);
}

/*
 * This is the function that gets executed at each node
 */

static int
execute(name, statb, type, state)
char *name;
struct stat *statb;
struct FTW *state;
{
	register struct Node *np = topnode;
	register int val;
	time_t t;
	register long l;
	char *Fstype;
	int not = 1;

	if(type==FTW_NS)
	{
		fprintf(stderr,"%s: stat() error %s: %s\n",
			cmdname, name, sys_errlist[errno]);
		return(0);
	}
	else if(type==FTW_DNR)
	{
		fprintf(stderr,"%s: cannot read dir %s: %s\n",
			 cmdname, name, sys_errlist[errno]);
		return(0);
	}
	else if(type==FTW_SLN)
	{
		fprintf(stderr,"%s: cannot follow symbolic link %s: %s\n",
			 cmdname, name, sys_errlist[errno]);
		return(0);
	}
	while(np)
	{
		switch(np->action)
		{
			case NOT:
				not = !not;
				np = np->next;
				continue;
			case OR:
			case LPAREN:
			{
				struct Node *save = topnode;
				topnode = np->first.np;
				(void)execute(name,statb,type,state);
				val = lastval;
				topnode = save;
				if(np->action==OR)
				{
					if(val)
						return(0);
					val = 1;
				}
				break;
			}
			case LOCAL:
				l = (long)statb->st_dev;
				goto num;
			case TYPE:
				l = (long)statb->st_mode&S_IFMT;
				goto num;
			case PERM:
				l = (long)statb->st_mode&07777;
				if(np->second.i == '-')
					val = ((l&np->first.l)==np->first.l);
				else
					val = (l == np->first.l);
				break;
			case INUM:
				l = (long)statb->st_ino;
				goto num;
			case NEWER:
				l = statb->st_mtime;
				goto num;
			case ATIME:
				t = statb->st_atime;
				goto days;
			case CTIME:
				t = statb->st_ctime;
				goto days;
			case MTIME:
				t = statb->st_mtime;
			days:
				l = (now-t)/A_DAY;
				goto num;
			case CSIZE:
				l = statb->st_size;
				goto num;
			case SIZE:
				l = round(statb->st_size,BLKSIZ)/BLKSIZ;
				goto num;
			case USER:
				l = (long)statb->st_uid;
				goto num;
			case GROUP:
				l = (long)statb->st_gid;
				goto num;
			case LINKS:
				l = (long)statb->st_nlink;
			num:
				if(np->second.i == '+')
					val = (l > np->first.l);
				else if(np->second.i == '-')
					val = (l < np->first.l);
				else
					val = (l == np->first.l);
				break;
			case OK:
				val = ok(name,np->first.ap);
				break;
			case EXEC:
				val = doexec(name,np->first.ap);
				break;
			case VARARGS:
			{
				register struct Arglist *ap = np->first.vp;
				register char *cp;
				cp = ap->nextstr - (strlen(name)+1);	
				if(cp >= (char*)(ap->nextvar+3))
				{
					/* there is room just copy the name */
					val = 1;
					strcpy(cp,name);
					*ap->nextvar++ = cp;
					ap->nextstr = cp;
				}
				else
				{
					/* no more room, exec command */
					*ap->nextvar++ = name;
					*ap->nextvar = 0;
					val = doexec((char*)0,ap->arglist);
					ap->nextstr = ap->end;
					ap->nextvar = ap->firstvar;
				}
				break;
			}
			case DEPTH:
			case MOUNT:
			case FOLLOW:
				val = 1;
				break;
			case NAME:
			{
				val = gmatch(name+state->base,np->first.cp);
				break;
			}
			case PRUNE:
				if (type == FTW_D)
					state->quit = FTW_PRUNE;
				val = 1;
				break;
			case NOUSER:
			{
				val = ((getpwuid(statb->st_uid))== 0);
				break;
			}
			case NOGRP:
			{
				val = ((getgrgid(statb->st_gid))== 0);
				break;
			}
			case FSTYPE:
			{
				Fstype = getfstype(name+state->base);
				val = ((strcmp(np->first.cp,Fstype))?0:1);
				break;
			}
			case CPIO:
				output = (FILE *)np->first.l;
				fprintf(output, "%s\n", name);
				val = 1;
				break;
			case PRINT:
				fprintf(stdout,"%s\n",name);
				val=1;
				break;
		}
		/* evaluate 'val' and 'not' (exclusive-or)
		 * if no inversion (not == 1), return only when val==0 
		 * (primary not true). Otherwise, invert the primary 
		 * and return when the primary is true. 
		 * 'Lastval' saves the last result (fail or pass) when 
		 * returning back to the calling routine. 
		 */
		if(val^not) {
			lastval = 0;
			return(0);
		}
		lastval = 1;
		not = 1;
		np=np->next;
	}
	return(0);
}

/*
 * code for the -ok option
 */

static int
ok(name,argv)
char *name;
char *argv[];
{
	int c, yes=0;

	fflush(stdout); 	/* to flush possible `-print' */
	(void) fprintf(stderr,"< %s ... %s >?   ",*argv , name);
	fflush(stderr);
	if((c=getchar())=='y')
		yes = 1;
	while(c!='\n')
		if(c==EOF)
			exit(2);
		else
			c = getchar();
	return(yes? doexec(name,argv): 0);
}

/*
 * execute argv with {} replaced by name
 */

static int
doexec(name,argv)
char *name;
register char *argv[];
{
	register char *cp;
	register char **av = argv;
	int r = 0;
	pid_t pid;

	fflush(stdout);		/* to flush possible `-print' */
	if(name)
	{
		while (cp= *av++)
		{
			if(cp==dummyarg)
				av[-1] = name;
		}
	}
	if(pid = fork())
	{
		while(wait(&r) != pid);
	}
	else /*child*/
	{
		execvp(argv[0], argv);
		exit(1);
	}
	return(!r);
}


/*
 *  Table lookup routine
 */
static struct Args*
lookup(word)
register char *word;
{
	register struct Args *argp = commands;
	register int second;
	if(word==0 || *word==0)
		return(0);
	second = word[1];
	while(*argp->name)
	{
		if(second == argp->name[1] && strcmp(word,argp->name)==0)
			return(argp);
		argp++;
	}
	return(0);
}


/*
 * Get space for variable length argument list
 */

static struct Arglist*
varargs(com)
char **com;
{
	register struct Arglist *ap;
	register int n;
	register char **ep;
	if(varsize==0)
	{
		n = 2*sizeof(char**);
		for(ep=environ; *ep; ep++)
			n += (strlen(*ep)+sizeof(char**) + 1);
		varsize = sizeof(struct Arglist)+ARG_MAX-PATH_MAX-n-1;
	}
	ap = (struct Arglist*)malloc(varsize+1);
	ap->end = (char*)ap + varsize;
	ap->nextstr = ap->end;
	ap->nextvar = ap->arglist;
	while( *ap->nextvar++ = *com++);
	ap->nextvar--;
	ap->firstvar = ap->nextvar;
	ap->next = lastlist;
	lastlist = ap;
	return(ap);
}

/*
 * Returns the file system type for the file system "file" lives in.
 */

static char *
getfstype(file)
	char *file;
{
	statvfs_t sbuf;
	char mnttype[FSTYPE];

	if (statvfs(file, &sbuf) != 0) 
	{
		fprintf(stderr,"find: can't statvfs %s: %s\n", file,
		  sys_errlist[errno]);
		return(NULL);
	}

	strcpy(mnttype, sbuf.f_basetype);
	return(mnttype);
}

/*
 * filter command support
 * fork and exec cmd(argv) according to mode:
 *
 *	"r"	with fp as stdin of cmd (default stdin), cmd stdout returned
 *	"w"	with fp as stdout of cmd (default stdout), cmd stdin returned
 */

#define CMDERR	((1<<8)-1)	/* command error exit code		*/
#define MAXCMDS	8		/* max # simultaneous cmdopen()'s	*/

static struct			/* info for each cmdopen()		*/
{
	FILE	*fp;		/* returned by cmdopen()		*/
	pid_t	pid;		/* pid used by cmdopen()		*/
} cmdproc[MAXCMDS];

FILE*
cmdopen(cmd, argv, mode, fp)
char	*cmd;
char	**argv;
char	*mode;
FILE	*fp;
{
	register int	proc;
	register int	cmdfd;
	register int	usrfd;
	int		pio[2];

	switch (*mode)
	{
	case 'r':
		cmdfd = 1;
		usrfd = 0;
		break;
	case 'w':
		cmdfd = 0;
		usrfd = 1;
		break;
	default:
		return(0);
	}
	for (proc = 0; proc < MAXCMDS; proc++)
		if (!cmdproc[proc].fp)
			 break;
	if (proc >= MAXCMDS) return(0);
	if (pipe(pio)) return(0);
	switch (cmdproc[proc].pid = fork())
	{
	case -1:
		return(0);
	case 0:
		if (fp && fileno(fp) != usrfd)
		{
			(void)close(usrfd);
			if (dup2(fileno(fp), usrfd) != usrfd) 
				_exit(CMDERR);
			(void)close(fileno(fp));
		}
		(void)close(cmdfd);
		if (dup2(pio[cmdfd], cmdfd) != cmdfd) 
			_exit(CMDERR);
		(void)close(pio[cmdfd]);
		(void)close(pio[usrfd]);
		execvp(cmd, argv);
		if (errno == ENOEXEC)
		{
			register char	**p;
			char		**v;

			/*
			 * assume cmd is a shell script
			 */

			p = argv;
			while (*p++);
			if (v = (char**)malloc((p - argv + 1) * sizeof(char**)))
			{
				p = v;
				*p++ = cmd;
				if (*argv) argv++;
				while (*p++ = *argv++);
				execv(getshell(), v);
			}
		}
		_exit(CMDERR);
		/*NOTREACHED*/
	default:
		(void)close(pio[cmdfd]);
		return(cmdproc[proc].fp = fdopen(pio[usrfd], mode));
	}
}

/*
 * close a stream opened by cmdopen()
 * -1 returned if cmdopen() had a problem
 * otherwise exit() status of command is returned
 */

int
cmdclose(fp)
FILE	*fp;
{
	register int	i;
	register pid_t	p, pid;
	int		status;

	for (i = 0; i < MAXCMDS; i++)
		if (fp == cmdproc[i].fp) break;
	if (i >= MAXCMDS) 
		return(-1);
	(void)fclose(fp);
	cmdproc[i].fp = 0;
	pid = cmdproc[i].pid;
	while ((p = wait(&status)) != pid && p != (pid_t)-1);
	if (p == pid)
	{
		status = (status >> 8) & CMDERR;
		if (status == CMDERR) 
			status = -1;
	}
	else 
		status = -1;
	return(status);
}

/*
 * return pointer to the full path name of the shell
 *
 * SHELL is read from the environment and must start with /
 *
 * if set-uid or set-gid then the executable and its containing
 * directory must not be writable by the real user
 *
 * /usr/bin/sh is returned by default
 */
extern char	*getenv();
extern char	*strrchr();


char*
getshell()
{
	register char	*s;
	register char	*sh;
	register uid_t	u;
	register int	j;

	if ((sh = getenv("SHELL")) && *sh == '/')
	{
		if (u = getuid())
		{
			if ((u != geteuid() || getgid() != getegid())
			   && !access(sh, 2))
				goto defshell;
			s = strrchr(sh, '/');
			*s = 0;
			j = access(sh, 2);
			*s = '/';
			if (!j) goto defshell;
		}
		return(sh);
	}
 defshell:
	return("/usr/bin/sh");
}
