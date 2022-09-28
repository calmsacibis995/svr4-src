/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)cof2elf:common/cof2elf.c	1.12"


/*	This program converts COFF input to ELF output.
 *	Libelf does the real work.
 *
 *	cof2elf [options] file ...
 *	-i		Ignore warnings and proceed
 *	-Qy		Add ident information to output file
 *	-Qn		Don't add ident information
 *	-q		Quiet: suppress information messages
 *	-s dir		Save coff files in 'dir'
 *	-V		print version information
 *
 *	When saving files, the program waits until just before conversion
 *	to make the copy.  Files that aren't converted don't get saved.
 *	This does two things.  First, it doesn't make extra copies of
 *	unchanged files.  Second, and more importantly, it doesn't overwrite
 *	a saved copy of a file with a second copy of the translated file.
 *	Consider the following:
 *
 *		cof2elf -s sv *.o	# something goes wrong halfway
 *		cof2elf -s sv *.o
 *
 *	If the second command saved all files, it would clobber the files
 *	saved the first time.
 *
 *	The program uses a tmp directory and must be careful about using
 *	full path names where appropriate and being in the right
 *	directory for some operations.  The main process doesn't change
 *	directories, but its children do.
 *
 *	Libelf updates a file in place, but the update creates a window
 *	when the file is neither the original coff nor complete elf.
 *	Consequently, the program copies the original to a temp place,
 *	updates the temp copy, and then renames the temp back to the
 *	original.  The temp copy must be in the same file system to
 *	make link(2) [or rename(2)] work, so this creates the temp
 *	copy in the same directory as the original file.
 */


#include <stdio.h>
#include <fcntl.h>
#include <string.h>

#if uts
#define MAXPATH BUFSIZ
#else
#include <limits.h>
#define MAXPATH PATH_MAX
#endif

#include <filehdr.h>
#include <string.h>
#include "sgs.h"
#include "libelf.h"
#include "ccstypes.h"
#include <sys/stat.h>
#include <signal.h>

#ifdef __STDC__
#	include <stdarg.h>
#else
#	include <varargs.h>
#endif


#undef const
#undef _
#ifdef __STDC__
#	define _(a)	a
#	define void_t	void
#else
#	define _(a)	()
#	define const
#	define void_t	char
#endif


typedef struct
{
	int	t_fd;
	char	*t_tmp;
	char	*t_usr;
	int	t_window;
} Temp;


extern int	close();
extern char	*getcwd();
extern char	*getenv();
extern void_t	*malloc();
extern int	mkdir();
extern char	*mktemp();
extern int	system();

static char	*basename	_((const char *));
static int	cksave		_((char *));
static void	die		_((int));
static int	doit		_((int, char *));
static void	error		_((int, const char *, ...));
static int	mkident		_((Elf *));
static void	praction	_((int));
static int	setaddr		_((Elf *));
static int	siginit		_((void));
static void	temp_done	_((Temp *, int));
static int	temp_make	_((Temp *, char *));


static struct
{
	const char	*lib,
			*file;
} printnm = { 0, "<no name>" };

static int		addident;	/* -Qy */
static char		*arcmd;
static size_t		arcmdsz;
static const char	*argv0;		/* pointer to our program name */
static char		*cwd;		/* full path of current dir */
static size_t		cwdsz;
static int		errflag;
static int		ignore;
static const char	*systmp;	/* system temporary directory */
static Temp		temp;		/* temp copy of user file */
static char		*tmpdir;	/* private tmp directory */
static char		*tmpf;		/* private tmp file */
static size_t		tmpsz;		/* tmpdirsz == tmpfsz */
static int		quiet_flag;
static const char	XX[] = "XXXXXXXXXX";


main(argc, argv)
	int	argc; 
	char	*argv[];
{
	int	vflag = 0;

	extern char *optarg;
	extern int optind;
	int optchar;
	extern int getopt();

	argv0 = basename(argv[0]);
	if (elf_version(EV_CURRENT) == EV_NONE)
	{
		error(0, "Libelf mismatch, %s\n", elf_errmsg(-1));
		exit(1);
	}
	if ((cwd = getcwd(NULL, MAXPATH)) == 0)
	{
		error(0, "Can't get current directory path name\n");
		exit(1);
	}
	cwdsz = strlen(cwd);
	if ((systmp = getenv ("TMPDIR")) == 0)
		systmp = P_tmpdir;
	if ((tmpdir = malloc(strlen(systmp) + 3 + sizeof(XX))) == 0
	|| (tmpf = malloc(strlen(systmp) + 3 + sizeof(XX))) == 0)
	{
		error(0, "Can't allocate temp file names\n");
		exit(1);
	}
	sprintf(tmpdir, "%s/a%s", systmp, XX);
	sprintf(tmpf, "%s/b%s", systmp, XX);
	if (*mktemp(tmpdir) == '\0'
	|| *mktemp(tmpf) == '\0'
	|| siginit() != 0
	|| mkdir(tmpdir, (mode_t)0700) != 0)
	{
		error(0, "Can't make temporary files (%s)\n", systmp);
		exit(1);
	}
	tmpsz = strlen(tmpdir);

	if ((arcmd = malloc(strlen(SGS) + 3)) == 0)
	{
		error(0, "Can't allocate AR command name\n");
		die(1);
	}
	strcpy(arcmd, SGS);
	strcat(arcmd, "ar");
	arcmdsz = strlen(arcmd);

	while ((optchar = getopt(argc, argv, "iQ:qs:V")) != -1) {
		switch(optchar) {
		case 'i':
			ignore = 1;
			break;

		case 'Q':
			if (strcmp(optarg, "y") == 0
			|| strcmp(optarg, "Y") == 0)
				addident = 1;
			else if (strcmp(optarg, "n") != 0
			&& strcmp(optarg, "N") != 0)
				error(0, "Invalid argument to -Q (%s)\n", optarg);
			break;

		case 'q':
			quiet_flag = 1;
			break;

		case 's':
			if (cksave(optarg) < 0)
				die(1);
			break;

		case 'V':
			vflag = 1;
			fprintf(stderr, "%s: %s %s\n",
				argv0, CPL_PKG, CPL_REL);
			break;

		default:
		case '?':
			errflag = 1;
			break;
		}
	}

	if (optind >= argc || errflag)
	{
		if (errflag == 0 && vflag)
			die(0);
		fprintf(stderr, "Usage: %s %s file ...\n",
			argv0,
			"[-s directory] [-Q{yn}] [-iqV]");
		die(1);
	}

	for(; optind < argc; optind++)
	{
		int	work;

		printnm.lib = 0;
		printnm.file = argv[optind];
		work = -1;
		if (temp_make(&temp, argv[optind]) >= 0)
		{
			work = doit(temp.t_fd, temp.t_tmp);
			temp_done(&temp, work);
		}
		praction(work);
	}
	die(errflag != 0);
	/*NOTREACHED*/
}


static void
die(j)
{
	signal(SIGINT, SIG_IGN);
	signal(SIGHUP, SIG_IGN);
	signal(SIGTERM, SIG_IGN);

	/*	If a signal interrupted the temp file rename,
	 *	print warning, do NOT remove file
	 */

	if (temp.t_window)
	{
		error(1, "Interrupt during rename: File might be named %s\n",
			temp.t_tmp);
		temp.t_tmp = 0;
	}

	if (fork() == 0)
	{
		/*	temp.t_tmp can be null; just be sure it is last
		 */

		execlp("rm", "rm", "-fr", tmpdir, tmpf, temp.t_tmp, (char *)0);
	}
	exit(j);
}


static int
doit(fd, fullnm)		/* -1 error, 0 no change, 1 xlate */
	int		fd;
	char		*fullnm;
{
	FILE		*ar_names;
	Elf		*elf;
	Elf32_Ehdr	*ehdr;
	char		cmdbuf[5*MAXPATH];
	char		rdbuf[MAXPATH];
	int		work = -1;

	(void)elf_errno();
	elf = 0;
	elf = elf_begin(fd, ELF_C_RDWR, elf);
	if (elf == 0 || ignore == 0 && elf_errmsg(0) != 0)
	{
		error(1, "Libelf %s\n", elf_errmsg(-1));
		goto done;
	}
	if (elf_errmsg(0) != 0)
	{
		--errflag;
		error(1, "Warning: Partial translation possible, %s\n",
			elf_errmsg(-1));
	}
	switch (elf_kind(elf))
	{
	default:
		work = 0;
		break;

	case ELF_K_COFF:
		if ((ehdr = elf32_getehdr(elf)) == 0)
		{
			error(1, "%s\n", elf_errmsg(-1));
			break;
		}
		if (ehdr->e_type != ET_REL)
		{
			error(1, "Not relocatable file\n");
			break;
		}

		if (cksave((char *)0) < 0)
			break;
		if (mkident(elf) < 0 || setaddr(elf) < 0)
			break;
		(void)elf_errno();
		if (elf_update(elf, ELF_C_WRITE) == -1
		|| ignore == 0 && elf_errmsg(0) != 0)
		{
			error(1, "Translation failed, %s\n", elf_errmsg(-1));
			break;
		}
		if (elf_errmsg(0) != 0)
		{
			--errflag;
			error(1, "Warning: Partial translation possible, %s\n",
				elf_errmsg(-1));
		}
		work = 1;
		break;

	case ELF_K_AR:
		if (printnm.lib != 0)
		{
			--errflag;
			error(1, "Warning: Can't translate nested archive\n");
			work = 0;
			break;
		}

		/*	Figure maximum size of system() argument
		 *	[50 includes a fudge factor]
		 */

		if (strlen(fullnm) + 2*tmpsz + arcmdsz + 50 > sizeof(cmdbuf))
		{
			error(1, "AR command too big (>%d bytes)\n", sizeof(cmdbuf));
			goto ar_bad;
		}
		sprintf(cmdbuf, "%s -t %s >%s", arcmd, fullnm, tmpf);
		if (system(cmdbuf) != 0)
		{
			error(1, "%s failed (table of contents)\n", arcmd);
			goto ar_bad;
		}
		sprintf(cmdbuf, "cd %s; %s -x %s", tmpdir, arcmd, fullnm);
		if (system(cmdbuf) != 0)
		{
			error(1, "%s failed (extracting files)\n", arcmd);
			goto ar_bad;
		}
		work = 0;
		ar_names = fopen(tmpf, "r");
		while (fgets(rdbuf, sizeof(rdbuf), ar_names) != NULL)
		{
			int	mfd;
			char	*p;
			int	j;

			p = rdbuf + strlen(rdbuf) - 1;
			if (*p != '\n'
			|| tmpsz + strlen(rdbuf) + 3 > sizeof(cmdbuf))
			{
				error(1, "Archive member name too long (%s...)\n",
					rdbuf);
				work = -1;
				break;
			}
			*p = '\0';
			p = (char *)printnm.lib;
			printnm.lib = printnm.file;
			printnm.file = rdbuf;
			sprintf(cmdbuf, "%s/%s", tmpdir, rdbuf);
			if ((mfd = open(cmdbuf, O_RDWR)) >= 0)
			{
				j = doit(mfd, cmdbuf);
				(void)close(mfd);
			}
			else
			{
				j = -1;
				error(1, "Can't open member for writing\n");
			}
			praction(j);
			printnm.file = printnm.lib;
			printnm.lib = p;
			if (j < 0)
			{
				work = -1;
				break;
			}
			work |= j;
		}
		fclose (ar_names);
	ar_bad:;
		if (work == 0)
			;
		else if (work > 0 && cksave((char *)0) == 0)
		{
			sprintf(cmdbuf, "cd %s; %s -rc %s `cat %s`",
				tmpdir, arcmd, fullnm, tmpf);
			if (system(cmdbuf) != 0)
			{
				error(1, "%s failed (rebuilding archive)\n",
					arcmd);
				die(1);
			}
		}
		else
			work = -1;
		sprintf(cmdbuf, "cd %s; rm -fr *", tmpdir);
		if (system(cmdbuf) != 0)
		{
			error(0, "Can't remove temp files (%s)\n", tmpdir);
			die(1);
		}
		break;
	}
done:
	(void)elf_end(elf);
	return work;
}


static char *
basename(name)			/* strip directories */
	const char *name;
{
	register const char *s = strrchr(name, '/');

	return (char *)(s ? ++s : name);
}


static int
cksave(save)
	char			*save;
{
	static struct stat	sstat;
	static char		*svnm = 0;
	struct stat		tstat;
	const char		*fmt = "cp %s %s";
	char			buf[3*MAXPATH];
	int			j;
	char			sv;
	char			*slash;
	const char		*tdir;

	if (save != 0)
	{
		if (svnm != 0)
		{
			error(0, "Only 1 save directory allowed\n");
			return -1;
		}
		if (save[0] == '/')
			svnm = save;
		else
		{
			if ((svnm = malloc(strlen(save) + cwdsz + 3)) == 0)
			{
				error(0, "No memory for save directory name\n");
				return -1;
			}
			sprintf(svnm, "%s/%s", cwd, save);
		}
		if (stat(svnm, &sstat) != 0
		|| (sstat.st_mode & S_IFMT) != S_IFDIR)
		{
			error(0, "No save directory (%s)\n", svnm);
			return -1;
		}
		return 0;
	}
	if (svnm == 0 || printnm.lib != 0)
		return 0;
	sv = '\0';
	if ((slash = strrchr(printnm.file, '/')) == 0)
		tdir = ".";
	else
	{
		tdir = printnm.file;
		sv = *++slash;
		*slash = '\0';
	}
	j = stat(tdir, &tstat);
	if (sv != '\0')
		*slash = sv;
	if (j == 0
	&& tstat.st_dev == sstat.st_dev
	&& tstat.st_ino == sstat.st_ino)
	{
		error(1, "Can't translate file in save directory\n");
		return -1;
	}
	if (strlen(svnm) + strlen(printnm.file) + strlen(fmt) >= sizeof(buf))
	{
		error(1, "Save command too big (>%d bytes)\n", sizeof(buf));
		return -1;
	}
	sprintf(buf, fmt, printnm.file, svnm);
	if (system(buf) != 0)
	{
		error(1, "Can't save original in %s\n", svnm);
		return -1;
	}
	return 0;
}


/*VARARGS1*/
#ifdef __STDC__
static void
error(int prnm, const char *format, ...)
{
	va_list	args;

	va_start(args, format);
	fprintf(stderr, "%s: ", argv0);
	if (prnm != 0)
	{
		if (printnm.lib)
			fprintf(stderr, "%s(%s): ", printnm.lib, printnm.file);
		else
			fprintf(stderr, "%s: ", printnm.file);
	}
	vfprintf(stderr, format, args);
	++errflag;
	va_end(args);
	return;
}
#else
static void
error(va_alist)
	va_dcl
{
	va_list	args;
	char	*format;
	int	prnm;

	va_start(args);
	prnm = va_arg(args, int);
	format = va_arg(args, char *);
	fprintf(stderr, "%s: ", argv0);
	if (prnm != 0)
	{
		if (printnm.lib)
			fprintf(stderr, "%s(%s): ", printnm.lib, printnm.file);
		else
			fprintf(stderr, "%s: ", printnm.file);
	}
	vfprintf(stderr, format, args);
	++errflag;
	va_end(args);
	return;
}
#endif


static int
mkident(elf)
	Elf		*elf;
{
	Elf_Scn		*scn;
	char		*buf;
	char		*comment = ".comment";
	const char	*format = "%s: %s %s";
	Elf_Data	*data, *strd;
	const char	*msg;
	char		*name;
	Elf32_Ehdr	*eh;
	Elf32_Shdr	*sh;
	size_t		ndx;
	size_t		sz;

	if (addident == 0)
		return 0;
	(void)elf_errno();
	if ((eh = elf32_getehdr(elf)) == 0)
	{
bad:
		msg = elf_errmsg(-1);
badmsg:
		error(1, "Can't add -Qy info to .comment section, %s\n", msg);
		return -1;
	}

	/*	4 is colon, 2 blanks, null
	 */

	sz = strlen(argv0) + strlen(CPL_PKG) + strlen(CPL_REL) + 4;
	if ((buf = malloc(sz)) == 0)
	{
		msg = "No memory for -Qy info";
		goto badmsg;
	}
	ndx = eh->e_shstrndx;
	scn = 0;
	while ((scn = elf_nextscn(elf, scn)) != 0)
	{
		if ((sh = elf32_getshdr(scn)) == 0)
			goto bad;
		name = elf_strptr(elf, ndx, sh->sh_name);
		if (sh->sh_type != SHT_PROGBITS
		|| name == 0 || strcmp(name, comment) != 0)
			continue;
		if ((data = elf_newdata(scn)) == 0)
			goto bad;
		data->d_buf = buf;
		sprintf(data->d_buf, format, argv0, CPL_PKG, CPL_REL);
		data->d_size = sz;
		return 0;
	}
	if (elf_errmsg(0) != 0
	|| (scn = elf_newscn(elf)) == 0
	|| (sh = elf32_getshdr(scn)) == 0
	|| (data = elf_newdata(scn)) == 0
	|| (strd = elf_newdata(elf_getscn(elf, ndx))) == 0)
		goto bad;
	data->d_buf = buf;
	strd->d_buf = comment;
	strd->d_size = strlen(comment) + 1;
	if (elf_update(elf, ELF_C_NULL) == -1)
		goto bad;
	sh->sh_name = strd->d_off;
	sh->sh_type = SHT_PROGBITS;
	sprintf(data->d_buf, format, argv0, CPL_PKG, CPL_REL);
	data->d_size = sz;
	return 0;
}


static void
praction(work)
	int	work;
{
	const char	*action;

	if (quiet_flag)
		return;
	if (work == 0)
		action = "Unchanged";
	else if (work > 0)
		action = "Translated";
	else
		action = "Failed";
	if (printnm.lib == 0)
		printf("%s %s\n", printnm.file, action);
	else
		printf("%s(%s) %s\n", printnm.lib, printnm.file, action);
	return;
}


static int
setaddr(elf)
	Elf		*elf;
{
	Elf_Scn		*scn;
	Elf32_Shdr	*sh;

	(void)elf_errno();
	scn = 0;
	while ((scn = elf_nextscn(elf, scn)) != 0)
	{
		if ((sh = elf32_getshdr(scn)) == 0)
			return -1;
		sh->sh_addr = 0;
	}
	if (elf_errmsg(0) != 0)
	{
		error(1, "Can't reset section addresses, %s\n", elf_errmsg(-1));
		return -1;
	}
	return 0;
}


static int
siginit()
{
	if (signal(SIGINT, SIG_IGN) != SIG_IGN)
		signal(SIGINT, die);
	if (signal(SIGTERM, SIG_IGN) != SIG_IGN)
		signal(SIGTERM, die);
	if (signal(SIGHUP, SIG_IGN) != SIG_IGN)
		signal(SIGHUP, die);
	return 0;
}


static void
temp_done(t, work)
	Temp		*t;
	int		work;
{
	char		*p;

	(void)close(t->t_fd);
	if (work > 0)
	{
		/*	rename(2) would be nice here, but it doesn't
		 *	exist everywhere.
		 */

		t->t_window = 1;
		(void)unlink(t->t_usr);
		if (link(t->t_tmp, t->t_usr) == 0)
			(void)unlink(t->t_tmp);
		else
			error(1, "Can't rename work file (%s) to original name\n",
				t->t_tmp);
		t->t_window = 0;
	}
	else
		(void)unlink(t->t_tmp);
	p = t->t_tmp;		/* avoid dangling t->t_tmp pointer */
	t->t_tmp = 0;
	free(p);
}


static int
temp_make(t, name)		/* full path temp file suitable for rename(2) */
	Temp		*t;
	char		*name;
{
	char		*p;
	char		cmdbuf[3*MAXPATH];
	char		*tmp;

	/*	make safe guess at final path size
	 */

	if ((tmp = malloc(cwdsz + strlen(name) + sizeof(XX) + 3)) == 0)
	{
		error(1, "Can't allocate memory for path name\n");
		return -1;
	}
	if (name[0] == '/')
		strcpy(tmp, name);
	else
		sprintf(tmp, "%s/%s", cwd, name);
	if ((p = strrchr(tmp, '/')) == 0)
	{
		error(0, "System error: no '/' in directory (%s)\n", cwd);
		die(1);
	}
	*(p + 1) = '\0';
	strcat(tmp, XX);
	if (*mktemp(tmp) == '\0')
	{
		error(1, "Can't create working file name\n");
		free(tmp);
		return -1;
	}

	/*	The following code embeds the sprintf to get the
	 *	same error handling for name too big or system() failure.
	 */

	if (strlen(name) + strlen(tmp) + 10 > sizeof(cmdbuf)
	|| (sprintf(cmdbuf, "cp %s %s", name, tmp), system(cmdbuf) != 0))
	{
		error(1, "Can't make working copy of file\n");
		free(tmp);
		return -1;
	}
	if ((t->t_fd = open(tmp, O_RDWR)) < 0)
	{
		error(1, "Can't open working file for read/write\n");
		(void)unlink(tmp);
		free(tmp);
		return -1;
	}
	t->t_tmp = tmp;
	t->t_usr = name;
	t->t_window = 0;
	return 0;
}
