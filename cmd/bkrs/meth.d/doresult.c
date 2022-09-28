/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:meth.d/doresult.c	1.3.2.1"

#include	<limits.h>
#include	<stdio.h>
#include	<sys/types.h>
#include	<backup.h>
#include	<bkrs.h>
#include	<method.h>

#define R_ULIMIT	0

extern int	rsresult();
extern char	*result();

static char	*results[] = {
			"ulimit",
			"no file name",
			"no uid",
			"not found",
			"successful",
			"restore incomplete  (see rslog)",
			"file found: archive date too recent",
			"file exists",
			"not owner",
			"cannot write target",
			"target directory nonexistent"};

dorsresult(mp, file_base)
m_info_t	*mp;
file_rest_t	*file_base;
{
	char	*tmp;
	char	*rmsg;
	char	buffer[24];
	register int	i;
	register file_rest_t	*f;

	for (i = 0, f = file_base; i < mp->n_names ; i++, f++) {
		if (f->type == 0) {		/* file */
#ifdef TRACE
			brlog("call rsresult for jobid=%s status=%d rindx=%d - %s -",f->jobid,f->status,f->rindx,RM(f) ? RM(f) : results[RI(f)]);
#endif
			(void) rsresult(f->jobid, (f->status ==  F_SUCCESS) ?
						BRSUCCESS : BRUNSUCCESS, 
					RM(f) ? RM(f) : results[RI(f)]);
		}
		else {				/* directory */
			tmp = NULL;
			rmsg = RFC(f) ? "successful" : "no files restored";

			if (RFC(f)) {
				(void) sprintf(buffer, "%d", RFC(f));

				if (RM(f)) {
					tmp = result(2,
					  "successful %s file(s) restored - %s",
						buffer, RM(f));
				}
				else {
					tmp = result(1,
					  "successful %s files restored",
						buffer);
				}
			}
			else if (RM(f)) {
				tmp = result(1,
				  "no files restored - %s", RM(f));
			}
			if (tmp)
				rmsg = tmp;
#ifdef TRACE
			brlog("call rsresult jobid=%s dir fcount=%d rmsg=%s",
						f->jobid,f->file_count,rmsg);
#endif
			(void) rsresult(f->jobid,
			    RFC(f) ? BRSUCCESS : BRUNSUCCESS,
				rmsg);
		}
	}
} /* dorsresult() */
