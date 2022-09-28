/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oamintf:libintf/iexpr.c	1.4.2.2"
/*LINTLIBRARY*/

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <errno.h>
#include <limits.h>
#include <dirent.h>
#include "intf.h"
#include "menu_io.h"
#include "menutrace.h"
#include "../intf_reloc/oldmenu.h"

extern int	task_id;
extern TASKDESC	*exec_list;
extern TASKDESC	*rn_list;
extern TASKDESC	*thread_strt;
extern TASKDESC	**thread_end;
extern struct menu_item
		*m_itemp;
extern struct menu_file
		*file_str;	/* return from input_menu() */

extern struct menu_item 
		*find_menu();
extern char	*getenv();
extern void	*malloc(),
		exit();
extern int	menu_build();

struct menu_file *input_menu();
void lineage();
TASKDESC * find_first(), *find_node();
int match_cnt();
void clr_marks(), add_thread();
int find_expr();
int mk_mt_node();

/* trace lineage back to root, to find .menu file */
void
lineage(p, bfrp)
TASKDESC  *p;		/* thread to follow back to parent */
char	*bfrp;		/* buffer to write menu hierarchy in for find_menu() */
{
	if(p->parent != (TASKDESC *)0) {
		/* trace back more */
		lineage(p->parent, bfrp);
	}
	/* on the way out of recursion, print name of this task into buffer */
	(void) strcat(bfrp, p->tname);
	(void) strcat(bfrp, ":");
}

/* find a node whose ident matches digits */
TASKDESC *
find_node(rp, ident)
TASKDESC *rp;		/* point to root */
int	ident;		/* node number to match */
{
	TASKDESC *tp, *retval;

	for (tp = rp; tp != (TASKDESC *)0; tp = tp->next) {
		if(tp->ident == ident) return(tp);
		if(tp->child != (TASKDESC *)0) 
			if ((retval=find_node(tp->child, ident)) != (TASKDESC *)0)
				return(retval);
	}
	return((TASKDESC *)0);
}

int
match_cnt(not_o, o)
int	*not_o, *o;
{
	int found;
	TASKDESC *y;

	*not_o = *o = 0;
	/* more than one branch in hierarchy mean not unique */
	for (found = 0, y = thread_strt; y != (TASKDESC *)0; y = y->thread) {
		++found;
		if((*(y->action) == 'O') && (*(y->action + 1) == '\0')) {
			++*o;
		} else {
			++*not_o;
		}
	}
	return(found);
}

void
clr_marks(p1)
TASKDESC *p1;
{
	TASKDESC *y;

	for (y = p1; y != (TASKDESC *)0; y = y -> next) {
		y->mark = 0;
		y->thread = (TASKDESC *)0;
		if(y->child != (TASKDESC *)0) clr_marks(y->child);
	}
}

int
find_expr(p1, seed, first_fit)
TASKDESC *p1;		/* point to current place in hierarchy */
char	*seed;		/* current segment of seed not yet located */
int	first_fit;	/* this level MUST at least partial match */
{
	TASKDESC *y;
	int	partial;	/* true if match is only partial */
	int	found, i;
	char	*cp;

	partial = found = 0;
	/* write out menu hierarchy */
	for (y = p1; y != (TASKDESC *)0; y = y -> next) {
		for (i = 0, cp = seed; *cp != '\0'; ++cp, ++i)
			if(*cp == '/') { partial = 1; break; }
		if((i == strlen(y->tname)) && (strncmp(seed, y->tname, i) == 0)) {
			/* paritial match at this point in hierarchy */
			if(partial) {
				/* if only partial, then was this OLD_SYSADM branch */
				if(strcmp(y->action, "O") == 0) {
					add_thread(y);
					y->mark = 2;
				} else if(strcmp(y->action, "R") == 0) {
					add_thread(y->child);
					y->mark = 2;
				} else {
					/* attempt to match last part of seed */
					found = find_expr(y->child, seed + i + 1, 1);
				}
			} else {
				/* complete match */
				if ( strcmp(y->action, "R") == 0 ) {
					add_thread(y->child);
					return(1);
				} else {
					found = 1;
					add_thread(y);
				}
			}
			if(found) {
				y->mark = 1;
			}
		}
		/* yes, this will repeat matches if found above, but with whole seed */
		if((first_fit == 0) && (y->child != (TASKDESC *)0))
			if(find_expr(y->child, seed, 0) != 0) {
				y->mark = 1;
				found = 1;
			}
	}
	return(found);
}

void
menu_dump(x, lvl)
TASKDESC *x;
int	lvl;
{
	int i;
	TASKDESC *y;

	for (y = x; y != (TASKDESC *)0; y = y -> next) {
		for (i = lvl; i > 0; --i) 
			(void) fprintf(stderr, "   ");
		(void) fprintf(stderr, "%s:\t\t(%s)\n", y->tname, y->action);
		menu_dump(y->child, lvl + 1);
	}
}

/* create a new node */
int
mk_mt_node(nextpp, parent, name)
TASKDESC **nextpp;	
TASKDESC *parent;
char	 *name;
{
	TASKDESC *new;

	if((new = (TASKDESC *)malloc(sizeof(TASKDESC))) == (TASKDESC *)0) {
		return(-1);
	}

	new->child = new->thread = (TASKDESC *)0;
	new->mark = 0;
	new->parent = parent;
	new->next = *nextpp;
	*nextpp = new;
	new->ident = task_id++;
	new->action = (char *)0;
	new->tname = strdup(name);
	return(0);
}

void
add_thread(y)
TASKDESC *y;
{
	TASKDESC *p;

	if(y == (TASKDESC *)0) return;

	if (strcmp(y->action, "R") == 0){
		(void) fprintf(stderr, "assertion failure in add_thread()\n");
		exit(1);
	}

	/* make sure thread only looked at once */
	for (p = thread_strt; p != (TASKDESC *)0; p = p->thread) {
		if(p == y) return;
	}

	*thread_end = y;
	thread_end = &(y->thread);
	y->thread = (TASKDESC *)0;
}
