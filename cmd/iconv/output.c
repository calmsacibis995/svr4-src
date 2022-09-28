/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)iconv:output.c	1.1.1.1"

#include <stdio.h>
#include "symtab.h"
#include "kbd.h"

extern char *gettxt();
extern int numnode;
extern int nerrors;
extern struct kbd_map maplist[];
extern int curmap;
extern unsigned char curswitch;
extern unsigned char oneone[];
extern int oneflag;
extern int fullflag;
extern int timeflag;

struct kbd_header hdr = { KBD_MAGIC, 0 };
struct kbd_tab *tb = 0;
struct cornode *nodes;

int tsize = 0;
unsigned char *tptr;
unsigned char text[65536];	/* 64 k max text */

extern int optt, optv, optreach;
extern struct node *root;
char *calloc();

buildtbl(val)	/* build one map; don't output yet */
	char *val;
{
	register struct kbd_tab *t;
	register int i;
	register unsigned char *s;

	hdr.h_magic[KBD_HOFF] = KBD_VER;
	hdr.h_magic[KBD_HOFF+1] = '\007';
	if (val == LINKAGE) {
		++(hdr.h_ntabs);
		return;
	}
	t = (struct kbd_tab *) calloc(1, sizeof(struct kbd_tab));
	if (oneflag) {
		t->t_flag |= KBD_ONE;
		maplist[curmap].mapone = (unsigned char *) calloc(1, 256);
		s = maplist[curmap].mapone;
		for (i = 0; i < 256; i++)
			*s++ = oneone[i];
	}
	maplist[curmap].maptab = t;

	++(hdr.h_ntabs);	/* add a table */
	strncpy(t->t_name, maplist[curmap].mapname, 15);
	t->t_nodes = numnode;
	if (fullflag) {
		t->t_min = maplist[curmap].map_min;
		t->t_max = maplist[curmap].map_max;
		t->t_flag |= KBD_FULL;
	}
	else
		t->t_min = t->t_max = 0;
	if (timeflag)
		t->t_flag |= KBD_TIME;
	nodes = (struct cornode *)calloc(numnode, sizeof(struct cornode));
	tsize = 0;
	tptr = text;
	maplist[curmap].maproot = root;
	tbaux(root);	/* build node table */
	/*
	 * add in "error" entry, if any.
	 */
	if (maplist[curmap].maperr) {
		t->t_error = (unsigned short) (tptr - text);
		t->t_flag |= KBD_ERR;
		strcpy(tptr, maplist[curmap].maperr);
		i = strlen(maplist[curmap].maperr) + 1;
		tsize += i;
		tptr += i;
	}
	root = (struct node *) 0;	/* no map yet... */
	maplist[curmap].maptext = (unsigned char *) calloc(1, (tptr - text));
	memcpy(maplist[curmap].maptext, text, (tptr - text));
	t->t_text = tsize;
	maplist[curmap].mapnodes = nodes;
	/*
	 * Report on bytes that can't be generated, if requested.
	 */
	if (optreach)
		reachout(t->t_name, text, (int) tptr - (int) text,
			 maplist[curmap].mapone, nodes, numnode);
	nodes = (struct cornode *) 0;
}

/*
 * Builds the "cornode" part of a table as a contiguous block of
 * cornodes.
 */
tbaux(p)

	struct node *p;
{
	register struct node *q;
	register int i, n;

	q = p;
	while (q) {
		n = q->n_num;
		nodes[n].c_val = q->n_val;
		switch (q->n_flag) {
		case N_CHILD:
			if (q->n_what.n_child->n_flag == N_RESULT) {
				nodes[n].c_flag = ND_RESULT;
				i = strlen(q->n_what.n_child->n_what.n_result) + 1;
/*
 * This "if" is a hack to hoist many-one mapping results into the node.
 * if "strlen(q->n_what.n_child->n_what.n_result) is 1, then set a
 * flag and put the result into nodes[n].c_child directly.
 */
				if (i == 2) { /* strlen is 1 */
					nodes[n].c_child = *(q->n_what.n_child->n_what.n_result);
					nodes[n].c_flag |= ND_INLINE;
					break;
					/* out of switch, finished node. */
				}
				nodes[n].c_child =
					(unsigned short) (tptr - text);
				strcpy(tptr, q->n_what.n_child->n_what.n_result);
				tptr += i;
				tsize += i;
				if (i >= KBDOMAX) {
					fprintf(stderr,
					gettxt("kbdcomp:28", "Error: string too long: %s\n"),
					q->n_what.n_child->n_what.n_result);
					++nerrors;
				}
				if (tsize > (KBDTMAX-10))
					fprintf(stderr,
					gettxt("kbdcomp:29", "Warning: replacement text close to overflowing.\n"));

			}
			else {
if (q->n_flag == N_EMPTY) /* SHOULD NEVER HAPPEN - only ROOT has empties. */
fprintf(stderr, gettxt("kbdcomp:30", "Internal error - child node empty.\n"));
				nodes[n].c_flag = 0; /* q->n_flag; */
				nodes[n].c_child = q->n_node;
				tbaux(q->n_what.n_child);
			}
			break;
		case N_EMPTY:
			nodes[n].c_child = q->n_val;
			nodes[n].c_flag = (ND_RESULT | ND_INLINE);
			break;
		default:
		case N_RESULT:
fprintf(stderr, gettxt("kbdcomp:31", "Internal error: un-hoisted node.\n"));
				break;
		}
		if (! q->n_next)
			nodes[q->n_num].c_flag |= ND_LAST;
		q = q->n_next;
	}
}

/*
 * Output all the maps.
 */

output()

{
	register struct kbd_tab *t;
	register int i;

	if (optt)
		fprintf(stderr, gettxt("kbdcomp:32", "%d table%s:\n"), hdr.h_ntabs,
			(hdr.h_ntabs == 1) ? "" : gettxt("kbdcomp:33", "s"));
	if (! optv)
		write(1, &hdr, sizeof(struct kbd_header));

	for (i = 0; i < hdr.h_ntabs; i++) {
		if (maplist[i].mapname == LINKAGE) {
			t = (struct kbd_tab *) calloc(1, sizeof(struct kbd_tab));
			t->t_flag = KBD_COT;
			t->t_max = maplist[i].map_max;
			if (optt)
				fprintf(stderr, gettxt("kbdcomp:34", "Link \"%s\"\n"), maplist[i].maptext);
			if (! optv) {
				write(1, t, sizeof(struct kbd_tab));
				write(1, maplist[i].maptext, t->t_max);
			}
			continue;
		}
		else if (maplist[i].mapname == EXTERNAL) {
			t = (struct kbd_tab *) calloc(1, sizeof(struct kbd_tab));
			t->t_flag = KBD_ALP;
			t->t_max = maplist[i].map_max;
			if (optt)
				fprintf(stderr, gettxt("kbdcomp:35", "Extern \"%s\"\n"), maplist[i].maptext);
			if (! optv) {
				write(1, t, sizeof(struct kbd_tab));
				write(1, maplist[i].maptext, t->t_max);
			}
			continue;
		}
		t = maplist[i].maptab;
		if (optt) {
			fprintf(stderr, gettxt("kbdcomp:36", "\t%s:\n"), maplist[i].mapname);
#if 0
			fprintf(stderr, gettxt("kbdcomp:37", "\t%s: swtch(%02X) "),
				maplist[i].mapname, t->t_swtch);
#endif
			fprintf(stderr, gettxt("kbdcomp:38", "\t\t%d node%s, %d byte%s text\n"),
				t->t_nodes, (t->t_nodes == 1) ? "" : gettxt("kbdcomp:39", "s"),
				t->t_text, (t->t_text == 1) ? "" : gettxt("kbdcomp:40", "s"));
		}
		if (! optv) {
			write(1, t, sizeof(struct kbd_tab));
			if (t->t_flag & KBD_ONE)
				write(1, maplist[i].mapone, 256);
			write(1, maplist[i].mapnodes,
				t->t_nodes * sizeof(struct cornode));
			write(1, maplist[i].maptext, t->t_text);
		}
	}
}
