/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:smtp/src/regexec.c	1.3.3.1"
#include "regprog.h"

/*
 *	Machine state
 */
#define LISTINCREMENT 8
typedef struct List{
	Inst	*inst;		/* Instruction of the thread */
	Subexp	se;		/* matched subexpressions in this thread */
}List;
static List	*tl, *nl;	/* This list, next list */
static List	*tle, *nle;	/* ends of this and next list */
static List	*list[2];
static List	*liste[2];
static int	listsize = LISTINCREMENT;

static Subexp sempty;		/* empty set of matches */
static int match;		/* true if match is found */

/*
 * Note optimization in addinst:
 * 	*lp must be pending when addinst called; if *l has been looked
 *		at already, the optimization is a bug.
 */
static List *
newthread(lp, ip, sep)
	List *lp;	/* list to add to */
	Inst *ip;	/* instruction to add */
	register Subexp *sep;	/* pointers to subexpressions */
{
	register List *p;

	for(p=lp; p->inst != NULL; p++){
		if(p->inst==ip){
			if((sep)->m[0].sp < p->se.m[0].sp)
				p->se = *sep;
			return NULL;
		}
	}
	p->inst = ip;
	p->se = *sep;
	(++p)->inst = NULL;
	return p;
}

static void
newmatch(mp, ms, sp)
	regsubexp *mp;
	int ms;	
	register Subexp *sp;
{
	register int i;

	if (mp==NULL || ms <=0)
		return;
	if(mp[0].sp==0 || sp->m[0].sp<mp[0].sp ||
	   (sp->m[0].sp==mp[0].sp && sp->m[0].ep>mp[0].ep)) {
		for (i=0; i < ms; i++)
			mp[i] = sp->m[i];
	}
}

extern int
regexec(progp, starts, mp, ms)
	Prog *progp;	/* program to run */
	char *starts;	/* string to run machine on */
	regsubexp *mp;	/* subexpression elements */
	int ms;		/* number of elements pointed to by mp */
{
	register flag=0;
	register Inst *inst;
	register List *tlp;
	register char *s;
	int startchar=progp->startinst->type<OPERATOR? progp->startinst->type : 0;
	int checkstart;

restart:
	match = 0;
	checkstart = startchar;
	sempty.m[0].sp = NULL;
	if (mp!=NULL && ms >0)
		mp[0].sp = mp[0].ep = NULL;
	if (list[0] == NULL) {
		list[0] = (List *)malloc(2*listsize*sizeof(List));
		list[1] = list[0] + listsize;
		liste[0] = list[0] + listsize - 1;
		liste[1] = list[1] + listsize - 1;
		if (list[0] == NULL)
			regerror("list overflow");
	}
	list[0][0].inst = list[1][0].inst = NULL;

	/* Execute machine once for each character, including terminal NUL */
	s=starts;
	do{
		/* fast check for first char */
		if(checkstart && *s!=startchar)
			continue;
		tl=list[flag];
		tle=liste[flag];
		nl=list[flag^=1];
		nle=liste[flag];
		nl->inst=0;
		/* Add first instruction to this list */
		sempty.m[0].sp = s;
		(void)newthread(tl, progp->startinst, &sempty);
		/* Execute machine until this list is empty */
		for(tlp=tl; inst=tlp->inst; tlp++){	/* assignment = */
	Switchstmt:
			switch(inst->type){
			default:	/* regular character */
				if(inst->type == *s){
	Addinst:
					if(newthread(nl, inst->next, &tlp->se)==nle)
						goto realloc;
				}
				break;
			case LBRA:
				tlp->se.m[inst->subid].sp = s;
				inst=inst->next;
				goto Switchstmt;
			case RBRA:
				tlp->se.m[inst->subid].ep = s;
				inst=inst->next;
				goto Switchstmt;
			case ANY:
				goto Addinst;
			case BOL:
				if(s == starts){
					inst=inst->next;
					goto Switchstmt;
				}
				break;
			case EOL:
				if(*s=='\0'){
					inst=inst->next;
					goto Switchstmt;
				}
				break;
			case CCLASS:
				if(((char *)inst->right)[*s/8]&(1<<(*s&07)))
					goto Addinst;
				break;
			case OR:
				/* evaluate right choice later */
				if (newthread(tlp, inst->right, &tlp->se) == tle)
					goto realloc;
				/* efficiency: advance and re-evaluate */
				inst=inst->left;
				goto Switchstmt;
			case END:	/* Match! */
				match = 1;
				tlp->se.m[0].ep = s;
				if (mp != NULL && ms > 0)									newmatch(mp, ms, &tlp->se);
				break;
			}
		}
		checkstart = startchar && nl->inst==NULL;
	}while(*s++);
	return match;
realloc:
	free(list[0]);
	list[0] = NULL;
	listsize += LISTINCREMENT;
	goto restart;
}

