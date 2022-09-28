/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)portmgmt:tty_settings/ttylist.c	1.2.2.1"

/*
 * ttylist.c - a program to check and list the hunt sequences in
 *	       /etc/ttydefs.
 *
 * Usage: cat /etc/ttydefs | cut -d: -f1,5 | ttylist
 */

#include	<stdio.h>
#include	<string.h>

#define	MAXTABLE	200

#define	NOTSEQ		-1
#define	NOLABEL		-2
#define	COMPLETE	-3

struct	tbl {
	char	label[16];
	char	nextlabel[16];
	int	next;
	int	printed;
} tbl[MAXTABLE];

int	Nentry;

main()
{
	int	i,j,ind;
	char	buf[BUFSIZ];
	char	*bp, *tp;

	Nentry = 0;
	while ((bp = gets(buf)) != NULL) {
		if (buf[0] == '#')
			continue;
		while (isspace(*bp)) bp++;
		if (strlen(bp) == 0)
			continue;
		if ((tp = strtok(bp,":")) == NULL) {
			printf("%s :(incorrect format, use <sttydefs -l> to check it)##true\n",buf);
			continue;
		}
		Nentry++;
		(void)strcpy(tbl[Nentry].label,tp);
		if ((tp = strtok(NULL,":")) == NULL) {
			printf("%s :(incorrect format, use <sttydefs -l> to check it)##true\n",buf);
			Nentry--;
			continue;
		}
		(void)strcpy(tbl[Nentry].nextlabel,tp);
		tbl[Nentry].next = 0;
		tbl[Nentry].printed = 0;
	}

	for (i=1; i<=Nentry; i++) {
		if (tbl[i].next == 0) {
			j = i;
			while ((ind=findlabel(j)) > 0) {
				if (ind == i) {
					tbl[j].next = COMPLETE;
					break;
				}
				if (tbl[ind].next > 0)
					break;
				tbl[j].next = ind;
				j = ind;
			}
			if (ind < 0)
				tbl[j].next = ind;
		}
	}
	printtbl();
	
}

int
findlabel(slot) 
int	slot;
{
	int	i;
	for (i=1; i<=Nentry; i++) {
		if (strcmp(tbl[slot].label,tbl[slot].nextlabel) == 0) {
			return(NOTSEQ);
		}
		if (strcmp(tbl[i].label,tbl[slot].nextlabel) == 0) {
			return(i);
		}
	}
	return(NOLABEL);
}

printtbl()
{
	int	i,j;
	for (i=1; i<=Nentry; i++) {
		if (!tbl[i].printed) {
			j = i;
			while (1) {
				printf("%s:%s",tbl[j].label,tbl[j].nextlabel);
				tbl[j].printed = 1;
				if (tbl[j].next > 0) {
					printf("##false\n");
					j = tbl[j].next;
					if (tbl[j].printed)
						break;
				}
				else {
					if (tbl[j].next == NOTSEQ)
					    printf(" (does not sequence)##false\n");
					else if (tbl[j].next == NOLABEL)
					    printf(" (Nextlabel not found)##false\n");
					else 
						printf("##false\n");
					break;
				}
			}
		}
	}
}
