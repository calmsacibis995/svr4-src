/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nfs.cmds:nfs/statd/sm_proc.c	1.2.3.1"
/*
 *  		PROPRIETARY NOTICE (Combined)
 *  
 *  This source code is unpublished proprietary information
 *  constituting, or derived under license from AT&T's Unix(r) System V.
 *  In addition, portions of such source code were derived from Berkeley
 *  4.3 BSD under license from the Regents of the University of
 *  California.
 *  
 *  
 *  
 *  		Copyright Notice 
 *  
 *  Notice of copyright on this source code product does not indicate 
 *  publication.
 *  
 *  	(c) 1986,1987,1988,1989  Sun Microsystems, Inc.
 *  	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
 *  	          All rights reserved.
 */
#include <stdio.h>
#include <rpc/rpc.h>
#include "sm_inter.h"
#include <memory.h>

extern int debug;
int local_state;		/* fake local sm state */
int remote_state = 3; 		/* fake remote sm state for testing */

#define SM_TCP_TIMEOUT 0

struct mon_entry {
	mon id;
	struct mon_entry *prev;
	struct mon_entry *nxt;
};
typedef struct mon_entry mon_entry;
struct mon_entry *monitor_q;
char *malloc();

sm_stat_res *
sm_stat_1(namep)
	sm_name *namep;
{
	static sm_stat_res resp;

	if (debug)
		printf("proc sm_stat: mon_name = %s\n", namep);

	/* fake answer */
	resp.res_stat = stat_fail;
	resp.state = -1;
	return (&resp);
}

sm_stat_res *
sm_mon_1(monp)
	mon *monp;
{
	static sm_stat_res resp;
	mon_id *monidp;
	monidp = &monp->mon_idno;

	if (debug)
		printf("proc sm_mon: mon_name = %s, id = %d\n",
		monidp->mon_name, * ((int * )monp->priv));
	/* store monitor request into monitor_q */
	insert_mon(monp);
	pr_mon();
	resp.res_stat = stat_succ;
	resp.state = local_state;
	return (&resp);
}

sm_stat *
sm_unmon_1(monidp)
	mon_id *monidp;
{
	static sm_stat resp;

	if (debug)
		printf("proc sm_unmon: mon_name = %s, [%s, %d, %d, %d]\n",
		monidp->mon_name, monidp->my_idno.my_name,
		monidp->my_idno.my_prog, monidp->my_idno.my_vers,
		monidp->my_idno.my_proc);
	delete_mon(monidp->mon_name, &monidp->my_idno);
	pr_mon();
	resp.state = local_state;
	return (&resp);
}

sm_stat *
sm_unmon_all_1(myidp)
	my_id *myidp;
{
	static sm_stat resp;

	if (debug)
		printf("proc sm_unmon_all: [%s, %d, %d, %d]\n",
		myidp->my_name,
		myidp->my_prog, myidp->my_vers,
		myidp->my_proc);

	delete_mon((char *)NULL, myidp);
	pr_mon();
	resp.state = local_state;
	return (&resp);
}

void *
sm_simu_crash_1()
{
	if (debug)
		printf("proc sm_simu_crash\n");
	if (monitor_q != (struct mon_entry *)NULL)
		sm_crash();
	return;
}


insert_mon(monp)
	mon *monp;
{
	mon_entry *new, *next;
	my_id *my_idp, *nl_idp;

	if ((new = (mon_entry *) malloc (sizeof (mon_entry))) == 0) {
		perror("rpc.statd: insert_mon: malloc");
		abort();
	}
	(void) memset(new, 0, sizeof (mon_entry));
	new->id = *monp;
	monp->mon_idno.mon_name = (char *)NULL;
	monp->mon_idno.my_idno.my_name = (char *)NULL;

	if (debug)
		printf("add_mon(%x) %s (id=%d)\n",
		new, new->id.mon_idno.mon_name, * ((int * )new->id.priv));

	record_name(new->id.mon_idno.mon_name, 1);
	if (monitor_q == (struct mon_entry *)NULL) {
		new->nxt = new->prev = (mon_entry *)NULL;
		monitor_q = new;
		return;
	}
	else {
		next = monitor_q;
		my_idp = &monp->mon_idno.my_idno;
		while (next != (mon_entry *)NULL)  { /* look for other mon_name */
			if (strcmp(next ->id.mon_idno.mon_name, new->id.mon_idno.mon_name) == 0) { /* found */
				nl_idp = &next->id.mon_idno.my_idno;
				if (strcmp(new->id.mon_idno.my_idno.my_name, nl_idp->my_name) == 0 &&
				my_idp->my_prog == nl_idp->my_prog &&
				my_idp->my_vers == nl_idp->my_vers &&
				my_idp->my_proc == nl_idp->my_proc) {
					/* already exists an identical one */
					free(new->id.mon_idno.mon_name);
					free(new->id.mon_idno.my_idno.my_name);
					free(new);
					return;
				}
				else {
					new->nxt = next->nxt;
					if (next->nxt != (mon_entry *)NULL)
						next->nxt->prev = new;
					next->nxt = new;
					return;
				}
			}
			next = next->nxt;
		}
		/* not found */
		new->nxt = monitor_q;
		if (new->nxt != (mon_entry *)NULL)
			new->nxt->prev = new;
		monitor_q = new;
		return;
	}
}

delete_mon(mon_name, my_idp)
	char *mon_name;
	my_id *my_idp;
{
	struct mon_entry *next, *nl;
	my_id *nl_idp;

	if (mon_name != (char *)NULL)
		record_name(mon_name, 0);

	next = monitor_q;
	while ((nl = next) != (struct mon_entry *)NULL) {
		next = next->nxt;
		if (mon_name == (char *)NULL || (mon_name != (char *)NULL &&
			strcmp(nl ->id.mon_idno.mon_name, mon_name) == 0)) {
			nl_idp = &nl->id.mon_idno.my_idno;
			if (strcmp(my_idp->my_name, nl_idp->my_name) == 0 &&
			my_idp->my_prog == nl_idp->my_prog &&
			my_idp->my_vers == nl_idp->my_vers &&
			my_idp->my_proc == nl_idp->my_proc) {
				/* found */
				if (debug)
					printf("delete_mon(%x): %s\n", nl, mon_name);
				if (nl->prev != (struct mon_entry *)NULL)
					nl->prev->nxt = nl->nxt;
				else {
					monitor_q = nl->nxt;
				}
				if (nl->nxt != (struct mon_entry *)NULL)
					nl->nxt->prev = nl->prev;
				free(nl->id.mon_idno.mon_name);
				free(nl_idp->my_name);
				free(nl);
			}
		} /* end of if mon */
	}
	return;
}

send_notice(mon_name, state)
	char *mon_name;
	int state;
{
	struct mon_entry *next;

	next = monitor_q;
	while (next != (struct mon_entry *)NULL) {
		if (strcmp(next->id.mon_idno.mon_name, mon_name) == 0) {
			if (statd_call_lockd(&next->id, state) == -1) {
				if (debug && mon_name)
					printf("problem with notifying %s failure, give up\n", mon_name);
			}
		}
		next = next->nxt;
	}
}

crash_notice()
{
	struct mon_entry *next;

	remote_state ++;
	next = monitor_q;
	while (next != (struct mon_entry *)NULL) {
		if (statd_call_lockd(&next->id, remote_state) == -1) {
			/* rpc problem */
		}
		next = next->nxt;
	}
}

recovery_notice()
{
	struct mon_entry *next;

	remote_state = remote_state +2;
	next = monitor_q;
	while (next != (struct mon_entry *)NULL) {
		if (statd_call_lockd(&next->id, remote_state) == -1) {
			/* rpc problem */
		}
		next = next->nxt;
	}
}

statd_call_lockd(monp, state)
	mon *monp;
	int state;
{
	struct status stat;
	my_id *my_idp;
	char *mon_name;
	int i, err;

	mon_name = monp->mon_idno.mon_name;
	my_idp = &monp->mon_idno.my_idno;
	(void) memset (&stat, 0, sizeof (struct status));
	stat.mon_name = mon_name; /* may be dangerous */
	stat.state = state;
	for (i = 0; i < 16; i++) {
		stat.priv[i] = monp->priv[i];
	}
	if (debug)
		printf("statd_call_lockd: %s state = %d\n",
		stat.mon_name, stat.state);
	if ((err = rpc_call(my_idp->my_name, my_idp->my_prog, my_idp->my_vers,
		my_idp->my_proc, xdr_status, &stat, xdr_void, NULL,
		"visible", SM_TCP_TIMEOUT, 0)) != (int) RPC_SUCCESS &&
		err != (int) RPC_TIMEDOUT ) {
		fprintf(stderr, "rpc.statd:  cannot contact local lockd on %s status change, give up!\n", mon_name);
		return (-1);
		}
	else
		return (0);
}

pr_mon()
{
	mon_entry *nl;

	if (!debug)
		return;
	if (monitor_q == (struct mon_entry *)NULL) {
		printf("*****monitor_q = NULL\n");
		return;
	}

	nl= monitor_q;
	printf("*****monitor_q: ");
	while (nl != (mon_entry *)NULL) {
		printf("(%x), ", nl);
		nl = nl->nxt;
	}
	printf("\n");
}
