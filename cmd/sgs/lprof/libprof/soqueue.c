/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)lprof:libprof/soqueue.c	1.4"

#include "hidelibc.h"		/* uses "_" to hide libc functions */
#include "hidelibelf.h"		/* uses "_" to hide libelf functions */

/*
*	File: soqueue.c
*	Date: 1/16/88
*	Desc: Code that manipulates the queue of shared objects (SO) for
*	the profiler.  Each entry in the queue contains information about
*	one SO.  This information is used to collect profiling information
*	about the SO.
*/
#include <stdio.h>
#include <string.h>
#include <link.h>

#include "debug.h"
#include "symint.h"
#include "profopt.h"

/*
*	Local definitions.
*/

typedef struct link_map	LinkMap_s;
typedef struct r_debug	Debug_s;

typedef struct queue_node {
	TYPE_BASEAD		qn_basead;
	char			*qn_pathname_p;
	char			*qn_cfilename_p;
	char			qn_dmpd;
	struct queue_node	*qn_next_p;
} QUEUE_NODE;


#define STD_DBNAME		"_r_debug"
#define TRUE			1
#define FALSE			0


/*
*	Local routines.
*/
static int chk_path();
static int chk_base();
static QUEUE_NODE * search_queue();
static LinkMap_s * find_linkentry();
static LinkMap_s * get_lmap();


/*
*	File scope variables.
*/
static LinkMap_s	*linkmap_p;
static char		init_flag = TRUE;
static QUEUE_NODE	*queue_anchor_p = NULL;

/*
*	_CAstartSO	- Initialize a shared object (called from pcrti).
*
*	- Check if initialization is required (done only once).
*	- Read the path name for shared object of given base address.
*	- If the SO is not already in the queue, build a new entry
*	and attach it to the beginning of the queue.
*
*	Warning: The base address is ALWAYS assigned to the (new/old)
*	queue node after the namesearch is complete.  If a node already
*	exists for the given path, the base address may have changed.
*/
_CAstartSO(base)
TYPE_BASEAD	base;
{
	LinkMap_s 	*link_p;
	QUEUE_NODE	*qn_p;

	DEBUG_LOC("soqueue.c: _CAstartSO");
	DEBUG(printf("base address from pcrtn = %x\n",base));
	if (init_flag) {
		init_flag = FALSE;
		if ((linkmap_p = get_lmap()) == NULL) {
			_err_exit("Cannot find link map.");
		}
	}
	if ((link_p = find_linkentry(base)) == NULL) {
		_err_exit("Cannot find linkmap entry shared object.");
	}
	DEBUG(printf("path of shared object: %s\n", link_p->l_name));
	if (!(qn_p = search_queue((void *) link_p->l_name, chk_path))) {
		DEBUG(printf("path not found in queue - calling malloc\n"));
		qn_p = (QUEUE_NODE *) _Malloc(1, sizeof(*qn_p));
		qn_p->qn_pathname_p = link_p->l_name;
		qn_p->qn_cfilename_p = NULL;
		qn_p->qn_next_p = queue_anchor_p;
		qn_p->qn_dmpd = FALSE;
		queue_anchor_p = qn_p;
	}
	DEBUG(printf("true base address from pcrtn = %x\n",link_p->l_addr));
	qn_p->qn_basead = link_p->l_addr;
	DEBUG_LOC("soqueue.c: leaving _CAstartSO");
}

/*
*	_CAstopSO	- Cleanup after a shared object (called from pcrtn).
*
*	- Key off the base address to find the SO in the queue.
*	- Initialize "opts" for the call to _CAdump.
*	- If the filename exists, then merge.  Otherwise create a new name.
*	- Call _CAdump.
*/
_CAstopSO(base)
TYPE_BASEAD	base;
{
	LinkMap_s 	*link_p;
	char 		*path;
	QUEUE_NODE	*qn_p;
	struct options	opts;

	DEBUG_LOC("soqueue.c: _CAstopSO");
	if ((link_p = find_linkentry(base)) == NULL) {
		_err_exit("Cannot find linkmap entry shared object.");
	}
	if (!(qn_p = search_queue((void *) &(link_p->l_addr), chk_base))) {
		DEBUG_LOC("soqueue.c: leaving _CAstopSO - base not found");
		return;
	}
	if (qn_p->qn_dmpd) {
		DEBUG_LOC("soqueue.c: leaving _CAstopSO - already dumped");
		return;
	}

	opts.noprofile = 0;
	opts.dir = 0;
	opts.msg = 0;
	opts.pid = 0;

	if (qn_p->qn_cfilename_p) {
		opts.merge = 1;
	} else {
		opts.merge = 0;
		if ((qn_p->qn_cfilename_p = tempnam(NULL, "lp")) == NULL) {
			_err_exit("Unable to create temp name.");
		}
	}
	opts.file = qn_p->qn_cfilename_p;

	qn_p->qn_dmpd = TRUE;
	_CAdump(2, qn_p->qn_pathname_p, &opts);
	DEBUG_LOC("soqueue.c: leaving _CAstopSO - dump complete");
}

/*
*	_CAget_base_address - return base address for shared object.
*
*	- Search for the given path name in SO queue.
*	- If found, return base address of object.  Otherwise return NULL.
*/
TYPE_BASEAD
_CAget_base_address(name)
char *name;
{
	QUEUE_NODE	*qn_p;

	if (qn_p = search_queue((void *) name, chk_path)) {
		return(qn_p->qn_basead);
	}
	return(NULL);
}

_CAnewdump()
{
	/*
	*	WFM 12/7/88
	*	"inside_dump" solves the problem of the infinite loop caused
	*	by exit calling dump which, upon fatal error, calls exit.
	*	Note that the flag ("inside_dump") must be reset before
	*	we return.
	*/
	static inside_dump = 0;

	/*
	*	WFM 03/24/89
	*	_prof_check_match is used to verify that the information
	*	in memory is the same as the information in the file.
	*	See dump.c for more information.
	*/
	extern int _prof_check_match;

	if (!inside_dump) {
		inside_dump = 1;
		_prof_check_match = 0;
		_CApredump();
		_prof_check_match = 1;
		_CAdump(0);
		inside_dump = 0;
	}
}

/*
*	_CApredump	- Precursor to _CAdump call for a.out.
*
*	- For each entry in the queue, call _CAproc_opts to move
*	the temporary file to a final resting place (including
*	consideration for PROFOPTS, except for filename).
*/
static char *str_dumping = "Dumping profiling data from process '%s' . . .\n";

_CApredump()
{
	extern char	*_CAproc;	/* name of this UNIX process */
	QUEUE_NODE	*qn_p, *t_p;
	struct options	opts;

	_CAgetprofopts(&opts);
	if (opts.noprofile) {
		return;
	}
	opts.file = NULL;

	if (opts.msg == TRUE) {
		fprintf(stderr, str_dumping, _CAproc);
	}

	qn_p = queue_anchor_p;
	while (qn_p) {
		if (!qn_p->qn_dmpd) {
			_CAstopSO(qn_p->qn_basead);
		}
		_CAproc_opts(qn_p->qn_cfilename_p, qn_p->qn_pathname_p, &opts);

		t_p = qn_p;
		queue_anchor_p = qn_p = qn_p->qn_next_p;
		free(t_p);
	}
}

/*
*	Functions used to call search_queue.
*/
static int
chk_path(qn_p, path)
QUEUE_NODE	*qn_p;
void		*path;
{ return(strcmp(qn_p->qn_pathname_p, (char *) path) == NULL); }

static int
chk_base(qn_p, base_p)
QUEUE_NODE	*qn_p;
void		*base_p;
{ return(qn_p->qn_basead == * ((TYPE_BASEAD *) base_p)); }


/*
*	search_queue	- Search queue for given entry (via "chk_*").
*/
static QUEUE_NODE *
search_queue(arg_p, fcn_p)
void	*arg_p;
int	(*fcn_p)();
{
	QUEUE_NODE	*qn_p;

	qn_p = queue_anchor_p;
	while (qn_p) {
		if ((*fcn_p)(qn_p, arg_p)) {
			break;
		}
		qn_p = qn_p->qn_next_p;
	}
	return(qn_p);
}

/*
*	find_linkentry - Search for link entry for a given base address.
*
*	- We assume that linkmap_p is *not* NULL.
*	- Search the debug link map for the entry with the given base address.
*	- Return a pointer to the link entry (NULL, if not found).
*
*	Note: Because the base provided to this routine may not match
*	the base seen in the table, we search the table for the greatest
*	lower bound of the given base.  We assume that this one is the
*	one we want.
*/
static LinkMap_s *
find_linkentry(base)
TYPE_BASEAD	base;
{
	LinkMap_s	*lm_p = linkmap_p;
	LinkMap_s	*rvlm_p = linkmap_p;

	while (lm_p) {
		DEBUG(printf("name = %s, address = %x\n"
			, lm_p->l_name
			, lm_p->l_addr
		));

		if (rvlm_p->l_addr < lm_p->l_addr && lm_p->l_addr <= base) {
			rvlm_p = lm_p;
		}
		lm_p = lm_p->l_next;
	}
	return(rvlm_p->l_addr <= base ? rvlm_p : NULL);
}

/*
 *	get_lmap -- Get the address of the link_map for the debug section.
 * 
 *	- Search the _DYNAMIC section structures for the DT_DEBUG entry.
 *	- If the debug entry is found then return the address of its link map.
 *	- Otherwise return NULL.
 *
 *	Note: _DYNAMIC is referenced in the main routine and stored in
 *	_prof_dynamic_ptr.  This is done to allow this code to be compiled
 *	as a shared object.  Without this arrangement (if we were to
 *	reference _DYNAMIC directly), we would get the wrong dynamic section.
 */
static LinkMap_s *
get_lmap()
{
	extern Elf32_Dyn *_prof_dynamic_ptr;

	Elf32_Dyn	*dyn_p;
	Debug_s		*deb_p = NULL;

	if (dyn_p = _prof_dynamic_ptr) {
		while (dyn_p->d_tag != DT_NULL) {
			if (dyn_p->d_tag == DT_DEBUG) {
				deb_p = (Debug_s *) dyn_p->d_un.d_ptr;
				break;
			}
			dyn_p++;
		}
	}

	DEBUG(printf("address of debug entry = 0x%x\n", deb_p));
	DEBUG(printf("address of link map = 0x%x\n", deb_p->r_map));

	return(deb_p ? deb_p->r_map : NULL);
}

