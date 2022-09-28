/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oamintf:interface/ie_build.c	1.3.1.2"

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <errno.h>
#include <limits.h>
#include <dirent.h>
#include "intf.h"
#include "print_err.h"
#include "errors.h"
#include "userdefs.h"
#include "sysadm.h"
#include "menu_io.h"
#include "menutrace.h"
#include "../intf_reloc/oldmenu.h"


extern char	*getenv(),
		*menutok();
extern int	ismenu(),
		match_cnt(),
		find_expr(),
		mk_mt_node();
extern void	exit(),
		print_err(),
		lineage(),
		clr_marks();

#define PL_LEN (sizeof(PHOLDER)-1)
#define TMP_NM "/tmp/tmp.menu"

int task_id = 0;
TASKDESC *root = (TASKDESC *)0;
TASKDESC *thread_strt = (TASKDESC *)0;
TASKDESC **thread_end = &thread_strt;

FILE *iefp;			/* internal express DB file pointer */


static void xseed_proc();
static void old_trace();
static void iexpr_write();
static int menu_build();
static int menu_trace(), r3_desc();

main(argc, argv)
int	argc;
char	*argv[];
{
	char fn1[PATH_MAX], fn2[PATH_MAX];
	char *oambase;

	if((oambase = getenv(OAMBASE)) == NULL)
		oambase = "/usr/sadm/sysadm";

	(void) sprintf(fn1, "%s/%s%s", oambase, MAIN_PATH, I_EXPR_PTH);
	if (menu_build("/usr/sadm/sysadm/menu") != 0)
		exit(1);

	thread_strt = (TASKDESC *)0;
	(void) find_expr(root, OLD_SYS, 0);

	if(thread_strt != (TASKDESC *)0) {
		thread_strt->action = "O";
		old_trace(thread_strt, "/usr/admin/menu");
	}

	/* re-create internal express mode file "i_expr" */
	if((iefp = fopen(fn1, "w")) == (FILE *)0) {
		print_err(NOT_OPEN, "ie_build", fn1);
		exit(1);
	}
	/* write out menu hierarchy */
	iexpr_write(root->child, 0);
	(void) fflush(iefp);
	/* read and process express mode seed file */
	(void) sprintf(fn2, "%s/%s%s", oambase, MAIN_PATH, EXPR_PTH);
	xseed_proc(fn2);
	(void) fclose(iefp);

	exit(0);
	/*NOTREACHED*/
}


static void
xseed_proc(fn)
char	*fn;
{
	FILE *xseedp;			/* express seed file pointer */
	char *eof, inbfr[200];		/* input processing */
	char *xname, *cmdstr, *link;	/* express file fields */
	int new_cnt, old_cnt;
	TASKDESC *p;
	char path_lineage[PATH_MAX];	/* path to an action */

	if((xseedp = fopen(fn, "r")) == (FILE *)0) {
		print_err(NO_EXPR, "ie_build");
	}

	/* for each record */
	do {
		if((eof = fgets(inbfr, 200, xseedp)) == NULL) continue;
		if(inbfr[0] == '#') continue;	/* comment */
		for (eof = inbfr; *eof != '\0'; ++eof) if(*eof == '\n') { *eof = '\0'; }
		if((xname = menutok(inbfr)) == NULL) continue;
		if((cmdstr = menutok(NULL)) == NULL) continue;
		link = menutok(NULL); /*optional*/;

		switch (*cmdstr) {

		case 'E':	/* exec string */
			(void) fprintf(iefp, "e\t0\t%s\t%s\n", xname, link);
			break;

		case 'P':	/* placeholder */
			(void) fprintf(iefp, "p\t0\t%s\n", xname);
			break;

		case 'L':
			/* process link information */
			clr_marks(root);
			thread_strt = (TASKDESC *)0;
			thread_end = &thread_strt;
			if(find_expr(root, strdup(link), 1) == 0) {
				print_err(EXPR_SYNTAX, "ie_build", xname, link);
				break;
			}
			(void) match_cnt(&new_cnt, &old_cnt);
			if(new_cnt != 1) {
				print_err(EXPR_SYNTAX, "ie_build", xname, link);
				(void) fprintf(stderr, " not unique menu path\n");
				for (p = thread_strt; p != (TASKDESC *)0; p = p->thread) {
					if(*(p->action) == 'O') continue;
					path_lineage[0] = '\0';
					lineage(p, path_lineage);
					(void) fprintf(stderr, "\t%s\n", path_lineage);
				}
			} else {
				/* find  non-old_sysadm match and write */
				for (p = thread_strt; (p != (TASKDESC *)0) &&
					(*p->action != 'O'); p = p->thread);
				(void) fprintf(iefp, "r\t%d\t%s\n", thread_strt->ident, xname);
			}
			break;

		default:
			/* syntax error */
			print_err(EXPR_SYNTAX, "ie_build", xname, link);
			break;

		}/*endswitch*/
	} while(eof != NULL);
	(void) fclose(xseedp);
}


static void
iexpr_write(p, indx)
TASKDESC *p;
int	indx;	/* index of parent node */
{
	TASKDESC *y;

	/* write out menu hierarchy */
	for (y = p; y != (TASKDESC *)0; y = y -> next) {
		if(*y->action == 'O') {
		  (void) fprintf(iefp, "%d\t%d\t%s\tO\n", y->ident, indx, y->tname);
		} else {
		  (void) fprintf(iefp, "%d\t%d\t%s\n", y->ident, indx, y->tname);
		}
		if(y->child != (TASKDESC *)0)
			iexpr_write(y->child, y->ident);
	}

}


extern int
menu_build(base)
char *base;		/* base directory */
{
	/* set up first/root node in hierarchy */
	root = (TASKDESC *)0;
	(void) mk_mt_node(&root, (TASKDESC *)0, "main");
	root->action = "main.menu";
	root->ident = 0;
	if(menu_trace(root, base) != 0) return(-1);
	return (0);
}

static int
menu_trace(tsk_nodp, dir)
TASKDESC *tsk_nodp;
char	*dir;
{
	char *tasknm, *n_action;	/* .menu field pointers */
	char *menu_dir;			/* path to add to */
	char newdir[PATH_MAX];
	struct menu_line *ptr_menu;	/* pointer to first line of menu item */
	struct menu_line m_line;	/* first line of menu item */
	TASKDESC *tp;
	char *x;

	extern char *read_item();

	ptr_menu = &m_line;		/* init pointer */

	/* ignore ifplaceholder: no access through main.menu */
	if(strncmp(tsk_nodp->action, PHOLDER, PL_LEN) == 0) return(0);

	/* if"*.menu" file, then process sub-menu */
	if(ismenu(tsk_nodp->action)) {
		FILE *mfilep;
		char *instr;

		/* open .menu file */
		(void) sprintf(newdir, "%s/%s", dir, tsk_nodp->action);
		if((mfilep = fopen(newdir, "r")) == (FILE *)0) {
			print_err(NOT_FOUND, "ie_build", newdir);
			return(-1);
		}
		menu_dir = strrchr(newdir, '/');
		++menu_dir;
		*menu_dir = '\0';

		/* read past header info to first menu item */
		while((instr = read_item(ptr_menu, mfilep, FULL_LINE)) != NULL) {
			if((*instr == '\n') || (*instr == '#')) continue;
		
			x = instr + strlen(instr) - 1;
			if(*x == '\n')  *x = '\0';
			/* get task, discard description, and get action field */
			if((tasknm  = menutok(instr)) == NULL) continue;
			if(menutok(NULL) == NULL) continue;
			if((n_action  = menutok(NULL)) == NULL) continue;

			/* create a child to this node in the hierarchy */
			if(mk_mt_node(&(tsk_nodp->child), tsk_nodp, tasknm) != 0) return(-1);
			tsk_nodp->child->action = strdup(n_action);

			/* handle OLD SYSADM case */
			if(strncmp(n_action, OLD_SYSADM, sizeof(OLD_SYSADM) - 1) == 0) {
				/* indicate OLD_SYSADM branch */
				tsk_nodp->child->action = "O";
			}
		}

		(void) fclose(mfilep);

		/* trace all children */
		for (tp = tsk_nodp->child; tp != (TASKDESC *)0; tp = tp -> next) {
			/* look at child menu/action */
			if(menu_trace(tp, newdir) != 0) {
				return (-1);
			}
		}
	}
	return (0);
}


static void
old_trace(tsk_nodp, dir)
TASKDESC *tsk_nodp;
char	*dir;
{
	TASKDESC *tp;
	DIR	*dirp;
	struct stat	xstat;
	struct dirent *dep;
	char	b1[PATH_MAX], *endp;

	if(tsk_nodp == (TASKDESC *)0) return;
	if((dirp = opendir(dir)) == (DIR *)0) {
		/* not a directory */
		return;
	}
	(void) strcpy(b1, dir);
	for (endp = b1; *endp != '\0'; ++endp) ;
	*endp++ = '/';  *endp = '\0';
	while ((dep = readdir(dirp)) != (struct dirent *)0) {
		/* ignore ".", ".." and "DESC" (description) entries */
		if(*dep->d_name == '.') continue;
		if(strcmp(dep->d_name, "DESC") == 0) continue;

		(void) strcpy(endp, dep->d_name);
		(void) stat(b1, &xstat);
		/* ifa sub-dir, only process ifdir has a DESC file */
		if(xstat.st_mode & S_IFDIR) {
			(void) strcat(endp, "/DESC");
			if(stat(b1, &xstat) == -1) continue;
		} else {
			/* ifno proper R3 descr., do not process */
			if(r3_desc(b1) == -1) continue; 
		}
		if(mk_mt_node(&(tsk_nodp->child), tsk_nodp, dep->d_name) != 0)
			return;
		tsk_nodp->child->action = "O";
	}
	(void) closedir(dirp);

	/* trace all child dirs */
	for (tp = tsk_nodp->child; tp != (TASKDESC *)0; tp = tp -> next) {
		/* look at child menu/action */
		(void) strcpy(endp, tp->tname);
		old_trace(tp, b1);
	}
	return;
}


static int
r3_desc(fn)
char	*fn;
{
	FILE	*fp;
	char	b[300];

	if((fp = fopen(fn, "r")) == (FILE *)0) {
		return(-1);
	}
	while (fgets(b, 300, fp) != (char *)0) {
		if(strncmp(b, MENUHDR, sizeof(MENUHDR) - 1) == 0) {
			(void) fclose(fp);
			return(0);
		}
	}
	(void) fclose(fp);
	return(-1);
}
