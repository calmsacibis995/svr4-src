/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*ident	"@(#)libelf:common/nplist.c	1.1"*/
/*
 * C++ Demangler Source Code
 * @(#)master	1.5
 * 7/27/88 13:54:37
 */
#include "elf_dem.h"

struct name_pair {
	char *s1,*s2;
};

struct name_pair nplist[100] = {
	{"lt","<"},   {"ls","<<"},  {"dv","/"}, 
	{"gt",">"},   {"rs",">>"},  {"md","%"}, 
	{"le","<="},  {"ml","*"},   {"pl","+"}, 
	{"ge",">="},  {"ad","&"},   {"mi","-"}, 
	{"ne","!="},  {"or","|"},   {"er","^"}, 
	{"aa","&&"},  {"oo","||"},  {"as","="}, 
	{"apl","+="}, {"ami","-="}, {"amu","*="}, 
	{"adv","/="}, {"amd","%="}, {"aad","&="}, 
	{"aor","|="},{"aer","^="}, {"als","<<="},
	{"ars",">>="},{"pp","++"},  {"mm","--"},
	{"vc","[]"},  {"cl","()"},  {"rf","->"},
	{"eq","=="},  {"co","~"},   {"nt","!"},
	{"nw"," new"},{"dl"," delete"}, {0,0} };

/* This routine demangles an overloaded operator function */
char *
findop(c,oplen)
char *c;
int *oplen;
{
	register int i,opl;
	for(opl=0; c[opl] && c[opl] != '_'; opl++)
		;
	*oplen = opl;
	for(i=0; nplist[i].s1; i++) {
		if(strncmp(nplist[i].s1,c,opl) == 0)
			return nplist[i].s2;
	}
	return 0;
}
