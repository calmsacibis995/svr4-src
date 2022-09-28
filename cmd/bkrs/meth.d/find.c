/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:meth.d/find.c	1.16.2.1"

#include	<limits.h>
#include	<stdio.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<ctype.h>
#include	<fcntl.h>
#include	<setjmp.h>
#include	<method.h>
#include	<backup.h>
#include	<bkrs.h>
#include	<signal.h>
#include	<string.h>
#include	<dirent.h>
#include	<table.h>
#include	<brtoc.h>
#include	<errno.h>

/*
 *	this code is 5.3.1 find modified for backup/restore
 */

#define CHARS   	110      /* ASCII header size minus filename field */
#define EQ(x, y)	(strcmp(x, y)==0)

extern int	br_toc_write();
extern int	brcancel();
extern int	brlog();
extern void	free();
extern int	gmatch();
static int	nmatch();
extern void	*malloc();
extern void	*realloc();
extern int	safe_stat();
extern void	seekdir();
extern char	*strrchr();
extern char	*strcpy();
extern time_t	time();
static int	pad();

extern int	bklevels;

typedef struct {
		int f;
		int per;
		int s;
	} Per_t;

m_info_t	*MP;

int		TC;
jmp_buf		env;

static int		and();
static void		descend();
static int		depth();
static struct anode	*e1();
static struct anode	*e2();
static struct anode	*e3();
static struct anode	*exp();
static int		fi_ctime();
static int		localf();
static struct anode	*mk();
static int		mountf();
static int		mtime();
static int		not();
static char		*nxtarg();
static int		or();
static int		print();
static int		remotef();
static int		type();

static int	Randlast;
static char	*Pathname = NULL;
static int	Pathsize = 0;
static int	entno = 0;

static struct anode {
	int (*F)();
	struct anode *L, *R;
} Node[100];

static int	Nn;  /* number of nodes */
static char    *Fname;
static long    Now;
static int     Argc = 0;
static int     Ai;
static char    *Argv[20];

static int     depthf = 0;

struct stat	Statb;

static long	total_bytes = 0;

/*
 * mount--When mount is specified the search is restricted to the file
 * system containing the directory specified or, in the case no
 * directory is specified, the current directory.  This is intended
 * for use by the administrator when backing up file systems.
 *
 * local--returns true if the file physically resides on the local machine.
 *
 * When find is not used with distributed UNIX
 *
 * mount--works no different.
 *
 * local--always return true.
 */

static char    remote_flag; 	/* on current local file system or remote */
static char    mount_flag;
static dev_t     cur_dev;

#ifdef TRACE
static char 	dbg_cmd[256];
#endif

int
find(mp, tc)
m_info_t	*mp;
int		tc;
{
	struct anode	*exlist;
	char		comment[64];
	int		i;
	register char	*cp;
	register char	*sp = NULL;

	MP = mp;
	TC = tc;

	i = strlen(OFS(MP));

	while (Pathsize < (i+1)) {
		Pathsize += PATH_MAX;
	}
	if ((Pathname = (char *) malloc((unsigned) Pathsize)) == NULL) {
		brlog(" pathname %s - no memory", OFS(MP));
		sprintf(ME(mp), "Job ID %s: out of memory", mp->jobid);
		return(1);
	}
	(void) strcpy(Pathname, OFS(MP));

	if (time(&Now) == (time_t) -1) {
		sprintf(ME(mp), "Job ID %s: cannot determine current time", mp->jobid);
		brlog(" time() %s ", SE);
		free (Pathname);
		return(2);
	}
	if (mp->meth_type == IS_INC) {
#ifdef TRACE

#define A_DAY 86400L	/* a day of seconds */

		brlog("Now=0x%x mp->lastfull=0x%x Now-A_DAY=0x%x",
				Now, mp->lastfull, Now-A_DAY);
#endif
		if (mp->flags & iflag) {		/* exclude inode only chgs */
			Argv[Argc++] = "-mtime";
		}
		else {
			Argv[Argc++] = "(";
			Argv[Argc++] = "-mtime";
			Argv[Argc++] = "-o";
			Argv[Argc++] = "-ctime";
			Argv[Argc++] = ")";
		}
	}
	Argv[Argc++] = "-depth";
	Argv[Argc++] = "-print";

	if (!(mp->flags & rflag)) {
		Argv[Argc++] = "-mount";
	}
	else {
		Argv[Argc++] = "-remote";
	}

#ifdef TRACE
	(void) strcat(dbg_cmd,"find ");

	for (i = 0; i < Argc; i++) {
		(void) strcat(dbg_cmd, Argv[i]);
		(void) strcat(dbg_cmd, " ");
	}
	brlog(" %s ", dbg_cmd);
#endif
	/* parse and compile the arguments */
	if ((i = setjmp(env)) || (!(exlist = exp()))) {
		sprintf(ME(mp), "Job ID %s: cannot find expression list", mp->jobid);
		brlog(" find: parsing error ");
		free(Pathname);
		return(i ? i : 1);
	}
	sp = "\0";

	if (cp = strrchr(Pathname, '/')) {
		sp = cp + 1;
	}
	Fname = *sp? sp: Pathname;

#ifdef TRACE
	brlog("find: safe_stat(%s)", Pathname);
#endif
	if (safe_stat(Pathname, &Statb) < 0) {
		brlog(" find: cannot stat %s ", Pathname);
		sprintf(ME(mp), "Job ID %s: cannot stat %s", mp->jobid, Pathname);
		free (Pathname);
		return(2);
	}
	if ((Statb.st_mode & S_IFMT) != S_IFDIR) {
		brlog(" bad starting directory%s ", OFS(MP) );
		sprintf(ME(mp), "Job ID %s: bad starting directory: %s", mp->jobid, OFS(MP));
		free (Pathname);
		return(2);
	}
	cur_dev = Statb.st_dev;

	if (!(i = setjmp(env))) { /* do the work to find files that match */
		descend((int)(Fname - Pathname), exlist);
	}
	else {					  /* error has occurred */
		brlog(" find: setjmp ret=%d ", i);
		free (Pathname);
		return(i);
	}
	if (total_bytes) {	/* include trailer for estimates */
		total_bytes += pad((CHARS + strlen("TRAILER!!!") + 1), 1);
	}
	(void) sprintf(comment,"PATHLENGTH=%d",Pathsize);

	if (i = br_toc_write(0, NULL, NULL, -1, comment)) {
		brlog("error writing pathlength in table of contents");
		sprintf(ME(mp), "Job ID %s: toc pathlength write error", mp->jobid);
		longjmp(env, BRBADFIND);
	}
	mp->blk_count = ((total_bytes + 511) >> 9);
	free (Pathname);

	return(0);
} /* find() */

/* compile time functions:  priority is  exp()<e1()<e2()<e3()  */

static struct anode
*exp()
{ /* parse ALTERNATION (-o)  */
	register struct anode	*p1;

	p1 = e1() /* get left operand */ ;

	if (EQ(nxtarg(), "-o")) {
		Randlast--;
		return(mk(or, p1, exp()));
	}
	else if (Ai <= Argc) --Ai;

	return(p1);
} /* exp() */

static struct anode
*e1()
{ /* parse CONCATENATION (formerly -a) */
	register struct anode	*p1;
	register char	*a;

	p1 = e2();
	a = nxtarg();

	if (EQ(a, "-a")) {
And:
		Randlast--;
		return(mk(and, p1, e1()));
	} else if (EQ(a, "(") || EQ(a, "!") || (*a=='-' && !EQ(a, "-o"))) {
		--Ai;
		goto And;
	} else if (Ai <= Argc) --Ai;

	return(p1);
} /* e1() */

static struct anode
*e2()
{ /* parse NOT (!) */

	if (Randlast) {
		brlog(" find: operand follows operand ");
		sprintf(ME(MP), "Job ID %s: unexpected operand error in find", MP->jobid);
		longjmp(env, BRBADFIND);
	}
	Randlast++;

	if (EQ(nxtarg(), "!"))
		return(mk(not, e3(), (struct anode *)0));

	else if (Ai <= Argc) --Ai;

	return(e3());
} /* e2() */

static struct anode
*e3()
{ /* parse parens and predicates */
	struct anode	*p1;
	int		i;
	register char	*a;
	register char	*b;
	register char	s;

	a = nxtarg();

	if (EQ(a, "(")) {

		Randlast--;
		p1 = exp();
		a = nxtarg();

		if (!EQ(a, ")")) goto err;

		return(p1);
	}
	else if (EQ(a, "-print")) {
		return(mk(print, (struct anode *)0, (struct anode *)0));
	}
	else if (EQ(a, "-depth")) {
		depthf = 1;
		return(mk(depth, (struct anode *)0, (struct anode *)0));
	}
	else if (EQ(a, "-local")) {
		return(mk(localf, (struct anode *)0, (struct anode *)0));
	}
	else if (EQ(a, "-remote")) {
		remote_flag = 1;
		return(mk(remotef, (struct anode *)0, (struct anode *)0));
	}
	else if (EQ(a, "-mount")) {
		mount_flag = 1;
		return(mk(mountf, (struct anode *)0, (struct anode *)0));
	}
	else if (EQ(a, "-mtime"))
		return mk(mtime, (struct anode *)0, (struct anode *)0);
	else if (EQ(a, "-ctime"))
		return mk(fi_ctime, (struct anode *)0, (struct anode *)0);

	b = nxtarg();
	s = *b;
	if (s=='+') b++;

	if (EQ(a, "-type")) {
		i = s=='d' ? S_IFDIR :
		    s=='b' ? S_IFBLK :
		    s=='c' ? S_IFCHR :
		    s=='p' ? S_IFIFO :
		    s=='f' ? S_IFREG :
		    s=='l' ? S_IFLNK :
		    0;
		return(mk(type, (struct anode *)i, (struct anode *)0));
	}
err:
	brlog(" find: bad option %s", a);
	sprintf(ME(MP), "Job ID %s: bad option: %s", MP->jobid, a);
	longjmp(env, BRBADFIND);

/*NOTREACHED*/
} /* e3() */

static struct anode
*mk(f, l, r)
int	(*f)();
struct anode	*l;
struct anode	*r;
{
	Node[Nn].F = f;
	Node[Nn].L = l;
	Node[Nn].R = r;
	return(&(Node[Nn++]));
} /* mk() */


static char
*nxtarg()
{ /* get next arg from backup list */
	static	strikes = 0;

	if (strikes==3) {
		brlog(" find: incomplete args ", MN(MP));
		return((char *) (-1));
	}
	if (Ai>=Argc) {
		strikes++;
		Ai = Argc + 1;
		return("");
	}
	return(Argv[Ai++]);
} /* nxtarg() */

/* execution time functions */

static int
and(p)
register struct anode	*p;
{
	return(((*p->L->F)(p->L)) && ((*p->R->F)(p->R))?1:0);
} /* and() */

static int
or(p)
register struct anode	*p;
{
	 return(((*p->L->F)(p->L)) || ((*p->R->F)(p->R))?1:0);
} /* or() */

static int
not(p)
register struct anode	*p;
{
	return( !((*p->L->F)(p->L)));
} /* not() */

static int
print()
{
	int	vol = 1;
	int	i;

	if ((MP->blks_per_vol) > 0) {
		vol += (total_bytes >> 9) / (MP->blks_per_vol);
	}
	if (i = br_toc_write(entno++, LTOC(MP) ? &Statb : NULL, Pathname, vol, 0)) {
		brlog("error writing table of contents");
		sprintf(ME(MP), "Job ID %s: toc write error", MP->jobid);
		longjmp(env, BRBADFIND);
	}
	if ((Statb.st_mode&S_IFMT) == S_IFREG || (Statb.st_mode&S_IFMT) == S_IFLNK) {
		total_bytes += (Statb.st_size + pad((CHARS + strlen(Pathname) + 1), 1));
	}
	else {
		total_bytes += pad((CHARS + strlen(Pathname) + 1), 1);
	}
	return(1);
} /* print() */

static int
localf()
{
	return (~Statb.st_dev < 0);
} /* localf() */

static int
remotef()
{
	return((Statb.st_dev < 0) || (Statb.st_dev == cur_dev));
} /* remotef() */

static int
mountf()
{
	return(1);
/*	return(Statb.st_dev == cur_dev);*/
} /* mountf() */

static int
mtime()
{
	return(Statb.st_mtime > MP->lastfull);
} /* mtime() */

static int
fi_ctime()
{
	return(Statb.st_ctime > MP->lastfull);
} /* fi_ctime() */

static int
type(p)
register Per_t *p;
{
	return((Statb.st_mode&S_IFMT)==p->per);
} /* type() */

static int
depth()
{
	return(1);
} /* depth() */

static void
descend(fname_off, exlist)
int 		fname_off;
struct anode	*exlist;
{
	DIR	*dir = NULL;		/* new directory */
	struct dirent	d_ent;
	struct dirent	*cur_dirent = &d_ent;
	int	ret;
	int	efn;
	int	Fname_off;
	int	namlen;
	int	fnamlen;
	long	enter_bytes = total_bytes;
	register char	*c1;
	register long   offset;
	extern int	brstate;

#define FN (Pathname + fname_off)

	if (!strlen(FN)) {
		return;	/* eliminate root error message */
	}
#ifdef TRACE
	brlog("descend: safe_stat(%s)", Pathname);
#endif
	if (safe_stat(Pathname, &Statb) < 0) {
		brlog(" find: stat() failed: %s: %s ", Pathname, SE);
		return;
	}
	if (mount_flag && (Statb.st_dev != cur_dev)) {
#ifdef TRACE
		brlog("descend: file %s st_dev=%lx cur_dev=%lx",Pathname, Statb.st_dev, cur_dev);
#endif
		return;
	}
	if (remote_flag && (!remotef())) {
#ifdef TRACE
		brlog("skipping: remote");
#endif
		return;
	}
/* process the exception list */

	if (MP->meth_type == IS_INC)
		if (MP->ex_count)
			if (nmatch(Pathname)) {
#ifdef TRACE
				brlog("skipping %s due to exception list",
					Pathname);
#endif
				return;
			}

	if (!depthf)
		(*exlist->F)(exlist);
	if ((Statb.st_mode&S_IFMT)!=S_IFDIR) {
		if (depthf) {
			(*exlist->F)(exlist);
		}
		return;
	}
	namlen = strlen(Pathname);

	if (namlen <= 0) {
		brlog(" find: pathname length %d illegal",namlen);
		sprintf(ME(MP), "Job ID %s: pathname length %d illegal", MP->jobid, namlen);
		longjmp(env, BRBADFIND);
	}
	c1 = (char *)(Pathname + namlen);
	if (namlen > 1) {
		if (*(c1-1) == '/') {
			*(--c1) = 0;
			namlen--;
		}
	}
	else {
		--c1;
	}
	efn = (c1 - Pathname);
				 /* loop thru this dir */
#define EFN (Pathname + efn)

	for(offset = 0; ; *EFN = 0) {
		if (dir == NULL) {
			BEGIN_CRITICAL_REGION;

			dir = opendir(Pathname);

			END_CRITICAL_REGION;

			if (dir == NULL) {
				brlog("find: cannot open %s %s ", Pathname, SE);
				break;	/* leave for loop */
			}
			if (offset) {
				 (void) seekdir(dir, (long)offset);
			}
		}
		BEGIN_CRITICAL_REGION;

		errno = 0;
		cur_dirent = readdir(dir);

		END_CRITICAL_REGION;

		if (cur_dirent == NULL) {
			if (errno) {
				brlog(" find: cannot read %s %s ", Pathname, SE);
			}
			break;	/* leave for loop */
		}
		offset = telldir(dir);

		if (brstate == BR_CANCEL) {
			if (ret = brcancel()) {
#ifdef TRACE
				brlog("new_vol brcancel returned %d ", ret);
#endif
				sprintf(ME(MP), "Job ID %s: brcancel returned %d", MP->jobid, ret);
				ret = BRFAILED;
			}
			else {
				ret = BRCANCELED;
			}
			sprintf(ME(MP), "Job ID %s: cancel received", MP->jobid);
			longjmp(env, ret);
		}
		if (!strcmp(cur_dirent->d_name, "."))
			continue;
		if (!strcmp(cur_dirent->d_name, ".."))
			continue;

		c1 = EFN;
		*c1++ = '/';
		*c1 = 0;

		if (!(fnamlen = strlen(cur_dirent->d_name))) {
			continue;
		}
		while ((namlen + fnamlen + 2) > Pathsize) { /* the / + null */
			if ((Pathname = (char *) realloc(Pathname,
				 (unsigned) (Pathsize + PATH_MAX))) == NULL) {
				brlog(" pathname - no memory");
				sprintf(ME(MP), "Job ID %s: out of memory", MP->jobid);
				longjmp (env, BRBADFIND);
			}
			else {
				Pathsize += PATH_MAX;
			}
		}
		(void) strcat(Pathname, cur_dirent->d_name);

		Fname_off = efn + 1;

		if (dir->dd_fd > (_NFILE - 5)) {
			(void) closedir(dir);
			dir = NULL;
		}
		descend(Fname_off, exlist);
	}
	if (dir) {
		(void) closedir (dir);
		dir = NULL;
	}
	*EFN = '\0';

	if (EFN == Pathname)		/* doing root dir */
		(void) strcpy(Pathname, "/");

	if (depthf) {
#ifdef TRACE
	brlog("descend: depthf: safe_stat(%s)", Pathname);
#endif
		if (safe_stat(Pathname, &Statb) < 0) {
			brlog(" find: stat failed: %s: %s ", Pathname, SE);
		}
		if (enter_bytes != total_bytes) { /* do dir if any below */
			(void) print();
		}
		else {
			(*exlist->F)(exlist);
		}
	}
	return;
} /* descend() */

static int
nmatch(name)  				/* based on cpio nmatch */
char	*name;
{
	char		**pat = MP->ex_tab;
	register int	i;

	for (i = 0; i < MP->ex_count; i++, pat++) {
		if ((**pat == '!' && !gmatch(name, *pat+1))
					    || gmatch(name, *pat)) {
			return(1);
		}
	}
	return(0);
} /* nmatch() */

/* this function rounds to the next word boundary
** flag = 0 - return roundup factor
** flag = 1 - return num plus roundup factor
*/

int pad(num, flag)
long num;
int flag;
{
int padval;
	if (num < 0) {
		brlog("bkcpio: negative padcnt %d\n", num);
		return(-1);
	}
	padval = (((num + PAD_VAL) & ~PAD_VAL) - num);

	if(flag)
		return((num+padval));	/* return word aligned total */
	else
		return(padval);		/* return the round factor */
}
	
